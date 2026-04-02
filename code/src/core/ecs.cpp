#include "ecs.h"
#include <cinttypes>
#include "common/lib/com_print.h"

static constexpr uint64_t MAX_ENTITY_QUEUE_SIZE = MAX_ENTITIES / 2;

enum class EntityState : int
{
	Available = 0,
	Claimed, // claimed by queue addition system
	Assigned // entity is assigned an entity ID by ecs system
};

struct Entity
{
	ComponentsMask componentsMask = INVALID_COMPONENTS_MASK;
	EntityState state = EntityState::Available;
	bool isEnabled = false;

	// TODO: maybe add support for don't delete on load?

	Entity() = default;

	Entity( ComponentsMask mask )
	{
		componentsMask = mask;
	}

	bool CanBeOperatedOn() const
	{
		return isEnabled && state == EntityState::Assigned;
	}
};

struct QueuedEntityAddition
{
	ComponentsMask componentsMask;
	EntityID claimedID;
};

// List of all entities in the game
static Entity s_entities[MAX_ENTITIES];
static EntityID s_numEntities;
static_assert( ARRAY_SIZE( s_entities ) < MAX_ENTITY_ID, "s_entities array size is >= MAX_ENTITY_ID" );

// Struct containing all components for all entities
static Components s_components;

//// TODO: maybe handle window logic in a separate file? Maybe can't
//// because it has to be handled by the game loop? Could be its own set
//// of static functions in a window.cpp file?
// TODO: these should be set via a settings/config file maybe? And should be configurable
// during game loop as well
static Color s_backgroundColor;
static Vector2i s_windowSize;

static QueuedEntityAddition s_entityAdditionQueue[MAX_ENTITY_QUEUE_SIZE];
static uint64_t s_numQueuedAdditions;

static EntityID s_entityDeletionQueue[MAX_ENTITY_QUEUE_SIZE];
static uint64_t s_numQueuedDeletions;

// ====================
// Private Helpers
// ====================

static bool IsValidEntityID( const EntityID id )
{
	return id >= 0 && id <= s_numEntities && id != INVALID_ENTITY_ID;
}


// ====================
// Public Functions
// ====================

const EntityID ECS_QueueEntityCreation( const ComponentsMask compMask )
{
	if ( s_numQueuedAdditions >= MAX_ENTITY_QUEUE_SIZE )
	{
		Com_PrintfWarningVerbose( "ECS", "Unable to add new entity due to addition queue being full. Max addition queue size: %" PRIu64 "", MAX_ENTITY_QUEUE_SIZE );
		return INVALID_ENTITY_ID;
	}

	// find and cache next available ID
	for ( EntityID entityID = 0; entityID < s_numEntities; ++entityID )
	{
		Entity* currEntity = &s_entities[entityID];
		if ( currEntity->state == EntityState::Available )
		{
			currEntity->state = EntityState::Claimed;

			// add to queue
			QueuedEntityAddition* currQueuedAddition = &s_entityAdditionQueue[s_numQueuedAdditions];
			currQueuedAddition->claimedID = entityID;
			currQueuedAddition->componentsMask = compMask;

			return entityID;
		}
	}

	Com_PrintfWarningVerbose( "ECS", "Unable to add new entity due to entities array being full. Entities array size: %" PRIu64 "", MAX_ENTITIES );
	return INVALID_ENTITY_ID;
}


bool ECS_QueueEntityRemoval( const EntityID entityID )
{
	// TODO: have to create and manage a queue for deleting at the end of the current game loop iteration 
	// (AKA current frame)
	return false;
}


const ComponentsMask ECS_GetEntityComponentsMask( const EntityID entityID )
{
	if ( !IsValidEntityID( entityID ) )
	{
		return INVALID_COMPONENTS_MASK;
	}

	const Entity* entity = &s_entities[entityID];
	if ( entity->CanBeOperatedOn() )
	{
		return entity->componentsMask;
	}

	return INVALID_COMPONENTS_MASK;
}


// TODO: have to figure out how scene loader will mass unload entities + mass load in entities 
// in respect to game loop. Might have to integrate it somehow into the start of a frame
// (clear entity list + add new list of entities as a whole). Maybe have a 
// load scene wrapper which calls the load scene functions which return a list of entities? 

void ECS_StartGameLoop()
{
	// TODO: run the following in this order
	// - initialize new scene (AKA clear entities list and fill in new entities)
	// - add new entities
	// - run all systems
	// - delete all queued deletion entities
	// - run rendering 
}
