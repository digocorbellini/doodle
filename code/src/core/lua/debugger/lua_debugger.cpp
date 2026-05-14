#include "common/global_defines.h"

#if IS_ENABLED( LUA_DEBUGGER )
#include "common/types.h"
#include "common/lib/com_print.h"
#include "common/platform/net_socket.h"
#include "common/platform/platform_thread.h"
#include <lua.hpp>
#include "lua_debugger.h"

//#include <string>
//#include <unordered_map>
//#include <vector>
#include <mutex>
#include <thread>
#include <array>
#include "common/lib/data_structures/fixed_map.h"
#include <string> // TODO: remove this eventually


enum class DebugStepMode : Byte
{
	None,
	Over,
	In,
	Out,
};

using namespace std;

static constexpr size_t MAX_SOURCE_PATH_LEN = 512;
static constexpr size_t MAX_DEBUG_FILES = 128;
static constexpr size_t MAX_BREAKPOINTS_PER_FILE = 128;

// TODO: ideally this should be a configurable value
static constexpr int DAP_PORT = 56789;

// TODO: potentially turn this into a monitor class instead to make it more thread safe
 
// Map of script file path to list of active breakpoint line numbers
// Ex: s_breakpoints["C:/doodle/game/scripts/main.lua"] = {10, 20, 35}
// This data is shared among threads so must use mutex to access
static FixedMapStringKey<array<int, MAX_BREAKPOINTS_PER_FILE>, MAX_SOURCE_PATH_LEN, MAX_DEBUG_FILES> s_breakpoints;
static mutex s_breakpointMutex;

static lua_State* s_luaState = nullptr;
static NetSocketPtr s_listenSocket = nullptr;
static NetSocketPtr s_clientSocket = nullptr;
static thread s_acceptThread;
static mutex s_sendMutex; // TODO: see if I can make a monitor instead?

// should only ever be accessed by main thread (AKA thread safe)
// TODO: is there some way I can verify a thread is the main thread? That way I can make a getter/setter for this value
// and then assert if non-main thread calls this? Also how will I handle other threads like physics and rendering in the
// future if this is only for main thread?
static bool s_halted = false;

// Also only accessed by main thread (TODO: should also make an assert for this)
static DebugStepMode s_debugStepMode = DebugStepMode::None;
static int s_debugStepDepth = 0;

static thread::id s_mainThreadId;

// ===================
// Private Helpers
// ===================

// TODO: replace this with c-str implementation or maybe make this a c-str helper in com_string.h
// Lua prefixes file-source strings with '@', e.g. "@C:/path/to/main.lua".
// Strip that prefix so paths match what VS Code sends in setBreakpoints.
static std::string NormalizeSource( const char* source )
{
	if ( source && source[0] == '@' )
		return std::string( source + 1 );
	return source ? source : "";
}

#endif // #if IS_ENABLED( LUA_DEBUGGER )