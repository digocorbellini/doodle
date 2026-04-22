#include <SFML/Graphics.hpp>
#include "core/ecs.h"

// TODO: testing
#include "core/scene_loader.h"

int main ()
{
    // TODO: have to call all "Init"s for all core systems

    ECS_StartGameLoop();

    return 0;
}