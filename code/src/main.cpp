#include "common/lib/com_thread.h"
#include "core/ecs.h"
#include "common/platform/net_socket.h"

int main ()
{
    // TODO: have to call all "Init"s for all core systems
    Com_SetMainThreadID();
    
    NetSocket_Init(); 

    ECS_StartGameLoop();

    return 0;
}