////////////////////////////////////////////////////////////
//
// This file is meant to contain all major functions that
// relate to operations of the main game loop.
// 
// This includes creating and removing entities, changing
// scenes, etc.
//
////////////////////////////////////////////////////////////

#pragma once
//#include "../components.h"
#include "../components.h"
#include "../common/types.h"

#define MAX_ENTITIES 5000
#define INVALID_ENTITY ULLONG_MAX

// Systems will operate on this struct of all components in the game
// Every entity has an entry in every list. Their component is
// accessed by indexing using their entity id.
struct Components
{
	// Have a list for every component type in the game. This
	// allows for componenets of the same type to be near each
	// other in memory since they will likely be accessed
	// together as systems iterate through all entities.
	EntityTransform2D entityTransforms2D[MAX_ENTITIES];
	PhysicsBody2D physicsBodies2D[MAX_ENTITIES];
	SpriteRenderer2D spriteRenderers2D[MAX_ENTITIES];
	PlayerController2D playerControllers2D[MAX_ENTITIES];
	Camera2D cameras[MAX_ENTITIES];
};


struct Entity
{
	ComponentsMask componentsMask;

	Entity() = default;
	Entity( ComponentsMask mask )
	{
		componentsMask = mask;
	}
};

Components* ECS_GetAllComponents();

Entity_t ECS_AddEntity( ComponentsMask compMask );

bool ECS_RemoveEntity( Entity_t entity );

bool ECS_LoadScene( const char* sceneFileName );

// TODO: figure out how to load and parse a scene/level/thing file
//bool ECS_LoadScene(); or maybe an "init"?

void ECS_StartGameLoop();