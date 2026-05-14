#include "common/lib/com_thread.h"
#include "core/ecs.h"

int main ()
{
    // TODO: have to call all "Init"s for all core systems
    Com_SetMainThreadID();

    ECS_StartGameLoop();

    return 0;
}