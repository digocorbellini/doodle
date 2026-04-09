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
#include "core/components.h"
#include "common/types.h"

static constexpr EntityID MAX_ENTITIES = 5000;

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

/// <summary>
/// Queue up the creation of an entity with the componenets in the given mask which will 
/// be added at the start of the next frame
/// </summary>
/// <param name="compMask">A mask with the components the entity should have</param>
/// <returns>The ID of the entity to be created. Note that the entity won't be 
/// active for systems to operate on until the next frame. Will return 
/// INVALID_ENTITY_ID if no new assets can be added.</returns>
const EntityID ECS_QueueEntityCreation( const ComponentsMask compMask );

/// <summary>
/// Queue the removal of the given entity at the end of the frame
/// </summary>
/// <param name="entity">The ID of the entity to be removed</param>
/// <returns>True if the entity is found and can be removed at the end of the frame, otherwise
/// returns false</returns>
bool ECS_QueueEntityRemoval( const EntityID entityID );

/// <summary>
/// Returns the components mask for the given entity. Will return INVALID_COMPONENTS_MASK
/// if the entity can not be operated on (Ex: does not exist, is disabled, etc).
/// </summary>
/// <param name="entityID">The ID of the desired entity</param>
/// <returns>The component mask for the given entity if it exists and can be operated on, 
/// otherwise returns INVALID_COMPONENTS_MASK</returns>
const ComponentsMask ECS_GetEntityComponentsMask( const EntityID entityID );

/// <summary>
/// Start the game loop logic. The loop will handle :
/// - creating queued entity creations
/// - running all systems to modify entities and componenets
/// - deleting queued entity deletions
/// - running the rendering logic 
/// </summary>
void ECS_StartGameLoop();
