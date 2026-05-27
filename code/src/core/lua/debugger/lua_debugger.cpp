#include "common/global_defines.h"

#if IS_ENABLED( LUA_DEBUGGER )
#include <array>
#include "common/types.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include "common/lib/com_thread.h"
#include "common/lib/data_structures/fixed_map.h"
#include "common/platform/net_socket.h"
#include "common/platform/pathing_utils.h"
#include "common/platform/platform_thread.h"
#include <lua.hpp>
#include "lua_debugger.h"
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>


enum class DebugStepMode : Byte
{
	None,
	Pause,
	Over,
	In,
	Out,
};

struct DebugExecutionState
{
	bool halted = false;
	DebugStepMode stepMode = DebugStepMode::None;
	int stepDepth = 0;
};

static const char* LUA_DEBUGGER_STR = OBFUSCATED_STRING( "LuaDebugger" );

static const char* DAP_HEADER_CONTENT_LEN_STR = OBFUSCATED_STRING( "Content-Length: " );

static constexpr size_t MAX_SOURCE_PATH_LEN = 512;
static constexpr size_t MAX_DEBUG_FILES = 128;
static constexpr size_t MAX_BREAKPOINTS_PER_FILE = 128;
static constexpr int MAX_FRAME_ID = 1000;
static constexpr size_t MAX_DAP_MESSAGE_SIZE = 64 * 1024;

// TODO: ideally this should be a configurable value
static constexpr int DAP_PORT = 56789;


using namespace std;
using json = nlohmann::json;
using BreakpointsArray = array<size_t, MAX_BREAKPOINTS_PER_FILE>;
using BreakpointMap = FixedMapStringKey<BreakpointsArray, MAX_SOURCE_PATH_LEN, MAX_DEBUG_FILES>;


// Map of script file path to list of active breakpoint line numbers
// Ex: s_breakpoints["C:/doodle/game/scripts/main.lua"] = {10, 20, 35}
// This data is shared among threads so must use mutex to access
static Monitor<BreakpointMap> s_breakpoints;

// Used by lua hook to sleep on until a breakpoint is hit
static Monitor<DebugExecutionState> s_executionState;

static thread s_acceptThread;

// for communicating with editor through DAP
static NetSocketPtr s_listenSocket; // initialized by main thread but only used by accept thread
static Monitor<NetSocketPtr> s_clientSocket;

static atomic<bool> s_isShuttingDown{ false };

// incremented on every DAP message
static Monitor<int> s_dapMessageSequenceCounter;


// ===================
// Private Helpers
// ===================

static void NormalizePathSlashes( char* str, const size_t strSize )
{
	if ( Com_StrEmpty( str ) || strSize == 0 )
	{
		return;
	}

	// normalize backslashes to forward slashes for consistent comparison
	for ( size_t i = 0; i < strSize && str[i] != '\0'; ++i )
	{
		if ( str[i] == '\\' )
		{
			str[i] = '/';
		}
	}
}

// Lua prefixes file-source strings with '@', e.g. "@C:/path/to/main.lua".
// Strip that prefix so paths match what VS Code sends in setBreakpoints.
// Also convert relative path to absolute path if relevant
static bool NormalizeSource( const char* source, char* outBuf, const size_t bufSize )
{
	if ( !outBuf || bufSize == 0 )
	{
		return false;
	}

	if ( Com_StrEmpty( source ) )
	{
		outBuf[0] = '\0';
		return true;
	}

	const char* stripped = ( source[0] == '@' ) ? source + 1 : source;

	if ( !PathingUtils_GetAbsolutePath( stripped, outBuf, bufSize ) )
	{
		COM_ALWAYS_ASSERT( "[%s]: failed to get absolute path for source: '%s'\n", LUA_DEBUGGER_STR, stripped );
		return false;
	}

	NormalizePathSlashes( outBuf, bufSize );

	// make drive name lowercase
	for ( int i = 0; i < bufSize && outBuf[i] != '\0'; ++i )
	{
		char* currChar = &outBuf[i];
		if ( *currChar == ':' )
		{
			break;
		}

		*currChar = Com_CharToLower( *currChar );
	}

	return true;
}


static bool HasBreakpointAtLine( const char* scriptSource, const size_t line )
{
	if ( Com_StrEmpty( scriptSource ) )
	{
		return false;
	}

	return s_breakpoints.Access( 
		[&]( BreakpointMap& map ) -> bool
		{
			const BreakpointsArray* breakpointArrayEntry = map.Get( scriptSource );
			if ( !breakpointArrayEntry )
			{
				return false;
			}

			for ( size_t i = 0; i < breakpointArrayEntry->size(); ++i )
			{
				const size_t currLine = ( *breakpointArrayEntry )[i];
				if ( currLine == 0 )
				{
					// lines are 1s based so 0
					// means we found end of entries
					break;
				}

				if ( currLine == line )
				{
					return true;
				}
			}

			return false;
		}
	);
}


static void SetBreakpoints( const char* scriptSource, const size_t lines[], const size_t lineCount)
{
	if ( Com_StrEmpty( scriptSource ) )
	{
		return;
	}

	if ( !lines )
	{
		return;

	}

	char normalizedSource[MAX_SOURCE_PATH_LEN] = { 0 };
	NormalizeSource( scriptSource, normalizedSource, sizeof( normalizedSource ) );

	s_breakpoints.Access(
		[&]( BreakpointMap& map )
		{
			map.Remove( normalizedSource );

			if ( lineCount == 0 )
			{
				return;
			}

			BreakpointsArray* newArray = map.InsertKey( normalizedSource );
			if ( !newArray )
			{
				Com_PrintfWarningVerbose( LUA_DEBUGGER_STR, "unable to add breakpoints for source '%s' due to map being full", normalizedSource );
				return;
			}

			// clear array
			*newArray = {}; 

			// copy breakpoint lines into array
			for ( size_t i = 0; i < lineCount; ++i )
			{
				if ( i > newArray->max_size() )
				{
					Com_PrintfWarningVerbose( LUA_DEBUGGER_STR, "unable to add all breakpoints for source '%s' due to exceeding max breakpoints per file. %zu >= %zu", normalizedSource, lineCount, newArray->max_size() );
					break;
				}
				(*newArray)[i] = lines[i];
			}
		}
	);
}


static void AcceptLoop()
{
	Com_PrintfVerbose( LUA_DEBUGGER_STR, "waiting for DAP client on port [%i]", DAP_PORT );

	// Block until IDE connects
	NetSocketPtr acceptedSocket = NetSocket_Accept( s_listenSocket );

	if ( s_isShuttingDown.load() )
	{
		return;
	}

	// cache accepted socket (will null socket if connection was not successful)
	s_clientSocket.Modify(
		[&]( NetSocketPtr& sock )
		{
			sock = std::move( acceptedSocket );

			if ( sock )
			{
				Com_PrintfVerbose( LUA_DEBUGGER_STR, "DAP client connected successfully" );
			}
			else
			{
				Com_PrintfErrorVerbose( LUA_DEBUGGER_STR, "failed to accept DAP client connection" );
			}
		}
	);
}


static void StartDAPServer()
{
	s_listenSocket = NetSocket_Listen( DAP_PORT, NetSocketType::TCP );
	if ( !s_listenSocket )
	{
		Com_PrintfErrorVerbose( LUA_DEBUGGER_STR, "failed to start DAP server on port %i", DAP_PORT );
		return;
	}


	// fire accept thread for listening for editor connection
	s_acceptThread = thread( AcceptLoop );
}


static void StopDAPServer()
{
	s_isShuttingDown.store( true );

	// close socket to unblock accept thread 
	s_listenSocket.reset(); 
	if ( s_acceptThread.joinable() )
	{
		s_acceptThread.join();
	}

	s_clientSocket.Modify(
		[&]( NetSocketPtr& sock )
		{
			sock.reset();
		}
	);
}


static bool SendDAPMessage( const json& msg )
{
	const json::string_t msgBody = msg.dump();
	// prepend HTTP-style header to body
	const json::string_t frame = OBFUSCATED_STRING( "Content-Length: " ) + to_string( msgBody.size() ) + "\r\n\r\n" + msgBody;

	return s_clientSocket.Access(
		[&]( const NetSocketPtr& sock )
		{
			if ( !sock )
			{
				Com_PrintfErrorVerbose( LUA_DEBUGGER_STR, "failed to send message due to client socket not having a DAP connection. Message: %s", frame );
				return true;
			}

			const bool success = NetSocket_Send( sock, frame.c_str(), static_cast<int>( frame.size() ) ) >= 0;
			if ( success )
			{
				Com_PrintfVerbose( LUA_DEBUGGER_STR, "successfully sent DAP message: \n\n%s\n", msgBody.c_str() );
			}
			return success;
		}
	);
}


static bool ReadDAPMessage(char* outBuff, const size_t buffSize)
{
	const bool success = s_clientSocket.Access(
		[&]( const NetSocketPtr& sock )
		{
			if ( !sock )
			{
				return false;
			}

			NetSocket_SetNonBlocking( sock, true );

			char headerBuff[512]; // header should never be this large in practice 
			int headerLen = 0;
			int contentLen = 0;
			int bodyBytesAlreadyRead = 0;
			bool headerFound = false;

			// Read in header = find end when find "\r\n\r\n"
			while ( headerLen < static_cast<int>( sizeof( headerBuff ) ) )
			{
				const int bytesRead = NetSocket_Recv( sock, headerBuff + headerLen, sizeof( headerBuff ) - headerLen );

				if ( bytesRead <= 0 )
				{
					NetSocket_SetNonBlocking( sock, false );
					return false;
				}

				headerLen += bytesRead;

				if ( headerLen < 4 )
				{
					continue;
				}

				// see if we have hit "\r\n\r\n"
				for ( int i = 0; i <= headerLen - 4; ++i )
				{
					if ( headerBuff[i] == '\r' && headerBuff[i + 1] == '\n' &&
						 headerBuff[i + 2] == '\r' && headerBuff[i + 3] == '\n' )
					{
						// parse content length from header
						const char* contentLenPos = strstr( headerBuff, DAP_HEADER_CONTENT_LEN_STR );
						if ( !contentLenPos )
						{
							COM_ALWAYS_ASSERT( "[%s] DAP message missing Content-Length header\n", LUA_DEBUGGER_STR );
							NetSocket_SetNonBlocking( sock, false );
							return false;
						}

						contentLen = atoi( contentLenPos + strlen( DAP_HEADER_CONTENT_LEN_STR ) );

						// copy any bytes of body that were already read into out buffer
						const int bodyStart = i + 4;
						bodyBytesAlreadyRead = ( headerLen > bodyStart ) ? headerLen - bodyStart : 0;
						if ( bodyBytesAlreadyRead > 0 )
						{
							memcpy( outBuff, headerBuff + bodyStart, bodyBytesAlreadyRead );
						}

						headerFound = true;
						break;
					}
				}

				if ( headerFound )
				{
					break;
				}
			}


			if ( !headerFound )
			{
				NetSocket_SetNonBlocking( sock, false );
				return false;
			}

			// +1 to account for null terminator
			if ( contentLen + 1 > buffSize )
			{
				COM_ALWAYS_ASSERT( "[%s] DAP message length exceeds given buffer size. %i > %i\n", LUA_DEBUGGER_STR, contentLen + 1, buffSize );
				NetSocket_SetNonBlocking( sock, false );
				return false;
			}

			// read the rest of the body
			int totalBytesRead = bodyBytesAlreadyRead;
			while ( totalBytesRead < contentLen )
			{
				const int bytesRead = NetSocket_Recv( sock, outBuff + totalBytesRead, contentLen - totalBytesRead );

				if ( bytesRead <= 0 )
				{
					NetSocket_SetNonBlocking( sock, false );
					return false;
				}

				totalBytesRead += bytesRead;
			}

			outBuff[totalBytesRead] = '\0';
			NetSocket_SetNonBlocking( sock, false );
			Com_PrintfVerbose( LUA_DEBUGGER_STR, "successfully read DAP message: \n\n%s\n", outBuff );
			return true;
		}
	);

	return success;
}


static bool SendResponse( const int requestSeq, const char* command, const bool success, const json& body = {} )
{
	const int seq = s_dapMessageSequenceCounter.Access(
		[&]( int& s )
		{
			return s++;
		}
	);

	json msg;
	msg["seq"] = seq;
	msg["type"] = "response";
	msg["request_seq"] = requestSeq;
	msg["command"] = command;
	msg["success"] = success;
	if ( !body.is_null() && !body.empty() )
	{
		msg["body"] = body;
	}

	return SendDAPMessage( msg );
}


static bool SendEvent( const char* event, const json& body = {} )
{
	const int seq = s_dapMessageSequenceCounter.Access(
		[&]( int& s )
		{
			return s++;
		}
	);

	json msg;
	msg["seq"] = seq;
	msg["type"] = "event";
	msg["event"] = event;
	if ( !body.is_null() && !body.empty() )
	{
		msg["body"] = body;
	}

	return SendDAPMessage( msg );
}


static void HandleMessage( const json& msg, lua_State* luaState )
{
	if ( msg.is_discarded() || !msg.contains( "type" ) )
	{
		return;
	}

	if ( msg["type"] != "request" )
	{
		return;
	}

	const json::string_t command = msg["command"];
	const int seq = msg["seq"];

	if ( command == "initialize" )
	{
		// handshake
		json caps;
		caps["supportsConfigurationDoneRequest"] = true;
		caps["supportsTerminateRequest"] = true;
		caps["supportsSuspendDebuggee"] = true;
		SendResponse( seq, command.c_str(), true, caps );
		SendEvent( "initialized" );
	}
	else if ( command == "configurationDone" )
	{
		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "launch" || command == "attach" )
	{
		SendResponse( seq, command.c_str(), true);
	}
	else if ( command == "setBreakpoints" )
	{
		// editor sending breakpoints for one source file
		const json::string_t source = msg["arguments"]["source"]["path"];
		vector<size_t> lines; // TODO: potentially change this for static array?
		json verifiedBreakpoints = json::array();

		if ( msg["arguments"].contains( "breakpoints" ) )
		{
			for ( auto& breakpoint : msg["arguments"]["breakpoints"] )
			{
				const int line = breakpoint["line"];
				COM_ASSERT( line >= 0, "[%]: breakpoint is on a negative line: %i\n", LUA_DEBUGGER_STR, line );
				lines.push_back( static_cast<size_t>( line ) );
				json verifiedBreakpoint;
				verifiedBreakpoint["verified"] = true;
				verifiedBreakpoint["line"] = line;
				verifiedBreakpoints.push_back( verifiedBreakpoint );
			}
		}

		SetBreakpoints( source.c_str(), lines.data(), lines.size() );

		json body;
		body["breakpoints"] = verifiedBreakpoints;
		SendResponse( seq, command.c_str(), true, body );
	}
	else if ( command == "threads" )
	{
		// hard code 1 thread since lua JIT is single threaded
		json thread;
		thread["id"] = 1;
		thread["name"] = "main";
		json body;
		body["threads"] = json::array( { thread } );
		SendResponse( seq, command.c_str(), true, body );
	}
	else if ( command == "continue" )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.halted = false;
				state.stepMode = DebugStepMode::None;
			}
		);

		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "next" )
	{
		s_executionState.Modify(
			[&]( DebugExecutionState& state )
			{
				lua_Debug ar;
				int depth = 0;
				while ( lua_getstack( luaState, depth, &ar ) )
				{
					++depth;
				}

				state.stepDepth = depth;
				state.stepMode = DebugStepMode::Over;
				state.halted = false;
			}
		);

		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "stepIn" )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.halted = false;
				state.stepMode = DebugStepMode::In;
			}
		);

		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "stepOut" )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.halted = false;
				state.stepMode = DebugStepMode::Out;
			}
		);

		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "stackTrace" )
	{
		json frames = json::array();
		lua_Debug ar;
		int level = 0;

		while ( lua_getstack( luaState, level, &ar ) )
		{
			// S = source file, l = current line, n = name
			// AKA get source file, current line number, and the function name
			lua_getinfo( luaState, "Sln", &ar );

			char normalizedPath[MAX_SOURCE_PATH_LEN] = { 0 };
			NormalizeSource( ar.source, normalizedPath, sizeof( normalizedPath ) );

			json frame;
			frame["id"] = level;
			frame["name"] = ar.name ? ar.name : "(unknown)";
			frame["line"] = ar.currentline;
			frame["column"] = 1;

			json src;
			src["path"] = normalizedPath;
			frame["source"] = src;

			frames.push_back( frame );
			++level;
		}

		json body;
		body["stackFrames"] = frames;
		body["totalFrames"] = static_cast<int>( frames.size() );
		SendResponse( seq, command.c_str(), true, body );
	}
	else if ( command == "scopes" )
	{
		const int frameID = msg["arguments"]["frameId"];
		json locals; 
		locals["name"] = "Locals";
		// 0 to MAX_FRAME_ID = frame IDs and MAX_FRAME_ID+ = variable refs
		COM_ASSERT( frameID < MAX_FRAME_ID, "[%s]: frame ID exceeded max frame ID: %i\n", LUA_DEBUGGER_STR, MAX_FRAME_ID );
		locals["variablesReference"] = frameID + MAX_FRAME_ID;
		locals["expensive"] = false;
		
		json body; 
		body["scopes"] = json::array( { locals } );
		SendResponse( seq, command.c_str(), true, body );
	}
	else if ( command == "variables" )
	{
		const int ref = msg["arguments"]["variablesReference"];
		// 0 to MAX_FRAME_ID = frame IDs and MAX_FRAME_ID+ = variable refs
		const int frameID = ref - MAX_FRAME_ID;

		json vars = json::array();
		lua_Debug ar;

		if ( lua_getstack( luaState, frameID, &ar ) )
		{
			int i = 1;
			const char* name;
			while ( ( name = lua_getlocal( luaState, &ar, i++ ) ) != nullptr )
			{
				json var;
				var["name"] = name;
				var["variablesReference"] = 0;

				const int type = lua_type( luaState, -1 );
				switch ( type )
				{
					case LUA_TNUMBER:
						var["value"] = to_string( lua_tonumber( luaState, -1 ) );
						var["type"] = "number";
						break;
					case LUA_TSTRING:
						var["value"] = lua_tostring( luaState, -1 );
						var["type"] = "string";
						break;
					case LUA_TBOOLEAN:
						var["value"] = lua_toboolean( luaState, -1 ) ? "true" : "false";
						var["type"] = "boolean";
						break;
					case LUA_TNIL:
						var["value"] = "nil";
						var["type"] = "nil";
						break;
					default:
						var["value"] = lua_typename( luaState, type );
						var["type"] = lua_typename( luaState, type );
						break;
				}

				lua_pop( luaState, 1 );
				vars.push_back( var );
			}
		}

		json body;
		body["variables"] = vars;
		SendResponse( seq, command.c_str(), true, body );
	}
	else if ( command == "pause" )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.halted = false;
				state.stepMode = DebugStepMode::Pause;
			}
		);

		SendResponse( seq, command.c_str(), true );
	}
	else if ( command == "disconnect" || command == "terminate" )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state)
			{
				state.halted = false;
				state.stepMode = DebugStepMode::None;
			}
		);

		SendResponse( seq, command.c_str(), true );

		// TODO: potentially kick off a new accept loop to wait for another debugger?
	}
	else
	{
		// always respond to unknown commands in order to prevent
		// debugger hang
		SendResponse( seq, command.c_str(), true );
	}
}


static void HaltExecution( lua_State* luaState, lua_Debug* ar, const char* reason )
{
	// notify VS Code that execution has stopped
	json body;
	body["reason"] = reason;
	body["threadId"] = 1;
	body["allThreadsStopped"] = true;
	SendEvent( "stopped", body );

	s_executionState.Modify(
		[]( DebugExecutionState& state )
		{
			state.halted = true;
		}
	);


	// pump DAP messages while halted
	bool stillHalted = true;
	while ( stillHalted )
	{
		char rawBuff[MAX_DAP_MESSAGE_SIZE];
		if ( ReadDAPMessage( rawBuff, sizeof( rawBuff ) ) )
		{
			const json msg = json::parse( rawBuff, nullptr, false );
			if ( !msg.is_discarded() && !msg.is_null() )
			{
				HandleMessage( msg, luaState );
			}
		}

		stillHalted = s_executionState.Access(
			[]( const DebugExecutionState& state )
			{
				return state.halted;
			}
		);
	}
}


static void LuaHook( lua_State* luaState, lua_Debug* ar )
{
	lua_getinfo( luaState, "Sl", ar );
	 
	char normalizedPath[MAX_SOURCE_PATH_LEN];
	NormalizeSource( ar->source, normalizedPath, sizeof( normalizedPath ) );
	const int line = ar->currentline;

	// read step state 
	const DebugStepMode stepMode = s_executionState.Access(
		[]( const DebugExecutionState& state )
		{
			return state.stepMode;
		}
	);

	const int stepDepth = s_executionState.Access(
		[]( const DebugExecutionState& state )
		{
			return state.stepDepth;
		}
	);

	// pause 
	if( stepMode == DebugStepMode::Pause && ar->event == LUA_HOOKLINE )
	{
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.stepMode = DebugStepMode::None;
			}
		);
		HaltExecution( luaState, ar, "pause" );
		return;
	}

	// step in
	if ( stepMode == DebugStepMode::In && ar->event == LUA_HOOKLINE )
	{
		// halt immediately on next line regardless of depth
		s_executionState.Modify( 
			[]( DebugExecutionState& state )
			{
				state.stepMode = DebugStepMode::None;
			}
		);
		HaltExecution( luaState, ar, "step" );
		return;
	}

	// step over
	if ( stepMode == DebugStepMode::Over && ar->event == LUA_HOOKLINE )
	{
		lua_Debug depthAr;
		int depth = 0;
		while ( lua_getstack( luaState, depth, &depthAr ) )
		{
			++depth;
		}

		// stop execution only when stack return same depth as request
		if ( depth <= stepDepth )
		{
			s_executionState.Modify(
				[]( DebugExecutionState& state )
				{
					state.stepMode = DebugStepMode::None;
				}
			);
			HaltExecution( luaState, ar, "step" );
			return;
		}
	}

	// step out
	if ( stepMode == DebugStepMode::Out && ar->event == LUA_HOOKRET )
	{
		// delay halting execution until after a return, then use 
		// step in to halt
		s_executionState.Modify(
			[]( DebugExecutionState& state )
			{
				state.stepMode = DebugStepMode::In;
			}
		);
		return;
	}

	// breakpoint check
	if ( HasBreakpointAtLine( normalizedPath, line ) )
	{
		HaltExecution( luaState, ar, "breakpoint" );
	}
}


// ===================
// Public Functions
// ===================

void LuaDebugger_Init( struct lua_State* luaState )
{
	COM_ASSERT_IS_MAIN_THREAD();

	StartDAPServer();
	// set up lua hook callback on 3 events: 
	// - LUA_MASKLINE = fire hook before every line of lua is executed (used by breakpoint checks)
	// - LUA_MASKCALL = fire hook when any function is called (used by step over logic)
	// - LUA_MASKRET = fire hook when any function returns (used by step out logic)
	lua_sethook( luaState, LuaHook, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0 );
	Com_PrintfVerbose( LUA_DEBUGGER_STR, "initialized lua DAP Server on port %d", DAP_PORT );
}


// poll for messages between breakpoints
void LuaDebugger_Frame()
{
	COM_ASSERT_IS_MAIN_THREAD();

	char rawBuff[MAX_DAP_MESSAGE_SIZE];
	if ( !ReadDAPMessage( rawBuff, sizeof( rawBuff ) ) )
	{
		return;
	}

	const json msg = json::parse( rawBuff, nullptr, false );
	if ( !msg.is_discarded() && !msg.is_null() )
	{
		// since this is main thread and not lua hook, don't need to pass in lua state
		HandleMessage( msg, nullptr );
	}
	else
	{
		Com_Printf( "RODRIGO: test\n" );
	}
}


void LuaDebugger_Shutdown()
{
	COM_ASSERT_IS_MAIN_THREAD();

	s_executionState.Modify(
		[]( DebugExecutionState& state )
		{
			state.halted = false;
			state.stepMode = DebugStepMode::None;
		}
	);

	StopDAPServer();
	Com_PrintfVerbose( LUA_DEBUGGER_STR, "lua debugger shut down")
}
#endif // #if IS_ENABLED( LUA_DEBUGGER )