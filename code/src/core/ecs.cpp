#include "ecs.h"

// List of all entities in the game
static Entity entities[MAX_ENTITIES];

static Components components;
//static Entity_t numEntities;
//
//static bool isDebugMode;
//
//// TODO: maybe handle window logic in a separate file? Maybe can't
//// because it has to be handled by the game loop? Could be its own set
//// of static functions in a window.cpp file?
//static Color backgroundColor;
//static Vector2i windowSize;