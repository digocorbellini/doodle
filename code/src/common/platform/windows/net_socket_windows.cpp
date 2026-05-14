#include "common/global_defines.h"

#if IS_ENABLED( PLATFORM_WINDOWS )
#include "common/lib/com_assert.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include "common/platform/net_socket.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define NET_SOCKET_WINSOCK_VERSION MAKEWORD( 2, 2 )

using namespace net_socket_impl;

static const char* NET_SOCKET_STR = OBFUSCATED_STRING( "NetSocket" );


// wrapper around winsock socket
class net_socket_impl::NetSocket
{
public:
	SOCKET handle = INVALID_SOCKET;
	NetSocketType type;
	
	~NetSocket()
	{
		if ( handle != INVALID_SOCKET )
		{
			closesocket( handle );
		}
	}
};


void NetSocketDeleter::operator()( NetSocket* socket ) const
{
	if ( !socket )
	{
		return;
	}

	delete socket;
}


// ============================
// Private Helpers
// ============================

static int GetWinSocketType( const NetSocketType type )
{
	switch ( type )
	{
		case NetSocketType::TCP:
			return SOCK_STREAM;
		case NetSocketType::UDP:
			return SOCK_DGRAM;
		default:
			COM_ALWAYS_ASSERT( "Unsupported socket type: %i\n", static_cast<int>( GetUndelyingEnumVal( type ) ) );
			return -1;
	}
}

static int GetSocketProtocolForType( const NetSocketType type )
{
	switch ( type )
	{
		case NetSocketType::TCP:
			return IPPROTO_TCP;
		case NetSocketType::UDP:
			return IPPROTO_UDP;
		default:
			COM_ALWAYS_ASSERT( "Unsupported socket type: %i\n", static_cast<int>( GetUndelyingEnumVal( type ) ) );
			return -1;
	}
}

static int TranslateSendFlags( const NetSocketSendFlags flags )
{
	int result = 0;
	// add mappings as you add flags
	return result;
}

static int TranslateRecvFlags( const NetSocketRecvFlags flags )
{
	int result = 0;
	if ( NetSocketFlags_IsSet( flags, NetSocketRecvFlags::Peek ) )
	{
		result |= MSG_PEEK;
	}
		
	if ( NetSocketFlags_IsSet( flags, NetSocketRecvFlags::WaitAll ) )
	{
		result |= MSG_WAITALL;
	}

	return result;
}


// ============================
// Public Functions
// ============================

bool NetSocket_Init()
{
	// initialize winsock library
	WSADATA wsaData;
	int result = WSAStartup( NET_SOCKET_WINSOCK_VERSION, &wsaData );
	if ( result != 0 )
	{
		Com_PrintfErrorVerbose( NET_SOCKET_STR, "Failed to initialize WinSock library with WSAStartup. Result: %d\n", result );
		return false;
	}

	return true;
}


void NetSocket_Shutdown()
{
	WSACleanup();
}


NetSocketPtr NetSocket_Listen( const int port, const NetSocketType type, const int backlog )
{
	const int sockType = GetWinSocketType( type );
	if ( sockType < 0 )
	{
		return nullptr;
	}

	const int protocol = GetSocketProtocolForType( type );
	if ( protocol < 0 )
	{
		return nullptr;
	}

	SOCKET sock = socket( AF_INET, sockType, protocol );
	if ( sock == INVALID_SOCKET )
	{
		Com_PrintfErrorVerbose( NET_SOCKET_STR, "Failed to create socket for port [%i]. Error: %d\n", port, WSAGetLastError() );
		return nullptr;
	}

	// TODO: should figure out if I can avoid dynamic memory here
	NetSocketPtr netSock( new NetSocket );
	if ( !netSock )
	{
		return nullptr;
	}
	netSock->handle = sock;
	netSock->type = type;

	NetSocket_SetReuseAddr( netSock );

	// bind socket to desired port
	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons( static_cast<u_short>( port ) ); // convert to network protocol byte order (always big-endian)
	addr.sin_addr.s_addr = INADDR_ANY; // accept connections from any network 
	if ( bind( sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR )
	{
		Com_PrintfErrorVerbose( NET_SOCKET_STR, "Failed to bind socket to port [%i]. Error: %d\n", port, WSAGetLastError() );
		return nullptr;
	}

	// only TCP needs to listen for socket so UDP can return early
	if ( type == NetSocketType::UDP )
	{
		return netSock;
	}

	// put socket in listening mode (AKA ready to accept connections) with a given backlog of connections before connections
	// are dropped
	if ( listen( sock, backlog ) == SOCKET_ERROR )
	{
		Com_PrintfErrorVerbose( NET_SOCKET_STR, "Failed to listen to socket for port [%i]. Error: %d\n", port, WSAGetLastError() );
		return nullptr;
	}

	return netSock;
}


NetSocketPtr NetSocket_Accept( const NetSocketPtr& listenSocket )
{
	if ( !listenSocket )
	{
		return nullptr;
	}

	if ( NetSocket_IsConnectionless( listenSocket ) )
	{
		return nullptr;
	}

	// blocks until the client connects then returns a new socket 
	// which should be used for the actual data transfer with the client
	// (the passed in listen socket is just for listening for incomming connections)
	SOCKET client = accept( listenSocket->handle, nullptr, nullptr );
	if ( client == INVALID_SOCKET )
	{
		return nullptr;
	}

	// TODO: should figure out if I can avoid dynamic memory here
	NetSocketPtr netSock( new NetSocket );
	if ( !netSock )
	{
		return nullptr;
	}
	netSock->handle = client;
	netSock->type = listenSocket->type;
	return netSock;
}


int NetSocket_Send( const NetSocketPtr& socket, const void* data, const int size, const NetSocketSendFlags flags )
{
	if ( !socket )
	{
		return NET_SOCKET_ERROR;
	}

	const int result = send( socket->handle, reinterpret_cast<const char*>( data ), size, TranslateSendFlags( flags ) );
	return ( result == SOCKET_ERROR ) ? NET_SOCKET_ERROR : result;
}


int NetSocket_Recv( const NetSocketPtr& socket, void* outBuff, const int buffSize, const NetSocketRecvFlags flags )
{
	if ( !socket )
	{
		return NET_SOCKET_ERROR;
	}

	const int result = recv( socket->handle, reinterpret_cast<char*>( outBuff ), buffSize, TranslateRecvFlags( flags ) );
	if ( result == SOCKET_ERROR )
	{
		const int err = WSAGetLastError();
		if ( err == WSAEWOULDBLOCK )
		{
			return NET_SOCKET_BLOCKED;
		}
		return NET_SOCKET_ERROR;
	}

	return result;
}


void NetSocket_SetNonBlocking( const NetSocketPtr& socket, const bool isNonBLocking )
{
	if ( !socket )
	{
		return;
	}

	u_long mode = isNonBLocking ? 1 : 0;
	ioctlsocket( socket->handle, FIONBIO, &mode );
}


void NetSocket_SetReuseAddr( const NetSocketPtr& socket )
{
	if ( !socket )
	{
		return;
	}

	const int opt = 1;
	setsockopt( socket->handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &opt ), sizeof( opt ) );
}


bool NetSocket_IsConnectionless( const NetSocketPtr& socket )
{
	return NetSocket_IsTypeConnectionless( socket->type );
}
#endif // #if IS_ENABLED( PLATFORM_WINDOWS )
