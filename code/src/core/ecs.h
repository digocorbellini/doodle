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
#include <typeindex>

static constexpr EntityID MAX_ENTITIES = 5000;

/// <summary>
/// Iterator class meant to be used for iterating over all entities in the game.
/// Can't be instantiated and instead will be passed through to registered
/// systems during each frame of the game loop.
/// </summary>
class EntityIterator
{
private:
	EntityID currEntity;
	EntityID count;

	EntityIterator();
	EntityIterator( const EntityID startingID );

	friend class EntityIteratorCreator;

public:
	/// <summary>
	/// The entity ID that the iterator is currently pointing to.
	/// </summary>
	/// <returns>The entity ID that the iterator is currently pointing to. Will be INVALID_ENTITY_ID if 
	/// it is pointing to the end of the list.</returns>
	EntityID operator*() const;

	/// <summary>
	/// Go to the next entity 
	/// </summary>
	/// <returns>An EntityIterator pointing to the next entity. Will be equal to end() if
	/// the end of the list of entities has been reached.</returns>
	EntityIterator operator++();

	/// <summary>
	/// Whether two entity iterators are pointing to the same entity ID.
	/// </summary>
	/// <param name="other">The other entity iterator to compare to</param>
	/// <returns>True if the two entity iterators don't equal and false otherwise.</returns>
	bool operator!=( const EntityIterator& other ) const;

	/// <summary>
	/// Get the end of the entity list.
	/// </summary>
	/// <returns>An iterator to the end of the list. The EntityID will be INVALID_ENTITY_ID</returns>
	const EntityIterator end();
};

/// <summary>
/// Queue up the creation of an entity with the componenets in the given mask which will 
/// be added at the end of the frame.
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
/// Delete all entities in the game at once
/// </summary>
/// <returns>True if successful and false otherwise</returns>
bool ECS_DeleteAllEntities();

// TODO: add a comment or make this private ideally
EntityID ECS_AddEntity();

/// <summary>
/// Returns the components mask for the given entity. Will return INVALID_COMPONENTS_MASK
/// if the entity can not be operated on (Ex: does not exist, is disabled, etc).
/// </summary>
/// <param name="entityID">The ID of the desired entity</param>
/// <returns>The component mask for the given entity if it exists and can be operated on, 
/// otherwise returns INVALID_COMPONENTS_MASK</returns>
const ComponentsMask ECS_GetEntityComponentsMask( const EntityID entityID );

// TODO: add a way to modify entity component mask (adding/removing components)
bool ECS_AddComponentToEntity( const EntityID entityID, const ComponentType component );

/// <summary>
/// See if the given entity can be operated on in a system (AKA is valid, is enabled, etc.)
/// </summary>
/// <param name="entityID">The ID of the relevant entity</param>
/// <returns>True if the entity can be operated on in a system. Otherwise, returns false. </returns>
bool ECS_CanOperateOnEntity( const EntityID entityID );

/// <summary>
/// Register the given system so that it may be run during the ECS game loop
/// </summary>
/// <param name="system">A pointer to the system to be registered</param>
void ECS_RegisterSystem( class System* system );

// TODO: comment this
void* ECS_GetComponentListRaw( ComponentType componentType );

/// <summary>
/// Get the component list for the given component type. This list will contain a component
/// for each entry.
/// </summary>
/// <typeparam name="T">The type of the component list. If this does not match the 
/// actual type of the component list, an assert will be hit.</typeparam>
/// <param name="componentType">The component type for the list</param>
/// <returns>The component list for the given component type. If the componentType is invalid, 
/// nullptr is returned.</returns>
template<typename T>
T* ECS_GetComponentList( ComponentType componentType )
{
	COM_ASSERT( typeid( T ) == Components_GetComponentTypeIndex( componentType ), "Type '%s' does not match expected type '%s' for given component type '%s'\n", typeid( T ).name(), Components_GetComponentTypeIndex( componentType ).name(), Components_GetComponentTypeString( componentType ) );
	return static_cast<T*>( ECS_GetComponentListRaw( componentType ) );
}

/// <summary>
/// Start the game loop logic. The loop will handle :
/// - loading and unloading scenes
/// - running all systems to modify entities and componenets
/// - running the rendering logic 
/// - deleting and creating queued entities 
/// </summary>
void ECS_StartGameLoop();
