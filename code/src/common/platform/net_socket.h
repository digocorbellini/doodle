////////////////////////////////////////////////////////////
//
// Platform agnostic header for a network socket
//
////////////////////////////////////////////////////////////


#pragma once
#include "common/types.h"
#include <memory>

// Each platform defines what this actually contains
namespace net_socket_impl
{
    class NetSocket;
    struct NetSocketDeleter
    {
        void operator()( NetSocket* socket ) const; // implemented by platform specific logic
    };
}
using NetSocketPtr = std::unique_ptr<net_socket_impl::NetSocket, net_socket_impl::NetSocketDeleter>;

enum class NetSocketType : char
{
	TCP,
	UDP
};

enum class NetSocketSendFlags : int
{
    None = 0,
    // add more as needed
};

enum class NetSocketRecvFlags : int
{
    None = 0,
    Peek = 1 << 0,  // inspect data without consuming it
    WaitAll = 1 << 1,  // block until all requested bytes are received
};

// Return codes for send/recv
static constexpr int NET_SOCKET_ERROR   = -1;
static constexpr int NET_SOCKET_BLOCKED = -2; // non-blocking socket had no data ready

// ============================
// Flag Helpers
// ============================

template <typename T>
concept IsNetSocketFlag = std::is_same_v<T, NetSocketSendFlags> || std::is_same_v<T, NetSocketRecvFlags>;

template<IsNetSocketFlag TFlags>
constexpr TFlags NetSocketFlags_Combine( TFlags a, TFlags b )
{
    return static_cast<TFlags>( static_cast<int>( a ) | static_cast<int>( b ) );
}

template<IsNetSocketFlag TFlags>
constexpr bool NetSocketFlags_IsSet( TFlags flags, TFlags flag )
{
    return ( static_cast<int>( flags ) & static_cast<int>( flag ) ) != 0;
}

template<IsNetSocketFlag TFlags>
constexpr TFlags operator|( TFlags a, TFlags b )
{
    return NetSocketFlags_Combine( a, b );
}

// ============================
// Lifecycle
// ============================

// Initialize the networking subsystem. Must be called only once 
// before any networking functions are called
bool NetSocket_Init();

// Shutdown the networking subsystem. Should be called at engine shutdown
void NetSocket_Shutdown();

// ============================
// Socket Setup
// ============================

// Create a listening socket on the given port.
// Will return nullptr on failure.
NetSocketPtr NetSocket_Listen( const int port, const NetSocketType type, const int backlog = 1 );

// Accepts an inncomming connection on a listening socket.
// Blocks until a client connects and returns nullptr on failure.
// NOTE: only applicable for sockets that require connection (Ex: TCP)
NetSocketPtr NetSocket_Accept( const NetSocketPtr& listenSocket );

// ============================
// Data Transfer
// ============================

// Send data. Returns the number of bytes sent or NET_SOCKET_ERROR on failure.
// NOTE: may not send all byes in one call so keep track of the returned number
// of bytes.
int NetSocket_Send( const NetSocketPtr& socket, const void* data, const int size, const NetSocketSendFlags flags = NetSocketSendFlags::None );


// Receive data. Returns bytes received or NET_SOCKET_BLOCKED if no data is available
// in non-blocking mode or NET_SOCKET_ERROR on failure.
// NOTE: may not receive all bytes in one call so can loop over this until all bytes are
// read.
int NetSocket_Recv( const NetSocketPtr& socket, void* outBuff, const int buffSize, const NetSocketRecvFlags flags = NetSocketRecvFlags::None );

// ============================
// Configuration
// ============================

// Toggle non-blocking mode on a socket
void NetSocket_SetNonBlocking( const NetSocketPtr& socket, const bool isNonBLocking );

// Make it so that port can be reused immediately after restart.
void NetSocket_SetReuseAddr( const NetSocketPtr& socket );

// ============================
// Helpers
// ============================

bool NetSocket_IsConnectionless( const NetSocketPtr& socket );

inline bool NetSocket_IsTypeConnectionless( const NetSocketType type )
{
    switch ( type )
    {
        case NetSocketType::TCP:
            return false;
        case NetSocketType::UDP:
            return true;
        default:
            COM_ALWAYS_ASSERT( "Unhandled NetSocketType in IsConnectionless: %i\n", static_cast<int>( GetUndelyingEnumVal( type ) ) );
            return false;
    }
}
