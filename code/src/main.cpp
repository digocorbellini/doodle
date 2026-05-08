#include <SFML/Graphics.hpp>
#include "core/ecs.h"

int main ()
{
    // TODO: have to call all "Init"s for all core systems

    ECS_StartGameLoop();

    return 0;
}