#include "ecs.h"
#include <chrono>
#include <cinttypes>
#include "common/lib/com_print.h"
#include "common/lib/com_assert.h"
#include "core/system.h"

using SteadyClock = std::chrono::steady_clock;

static constexpr uint64_t MAX_ENTITY_QUEUE_SIZE = MAX_ENTITIES / 2;
static constexpr uint64_t MAX_SYSTEMS = 64;

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

	void Reset()
	{
		componentsMask = INVALID_COMPONENTS_MASK;
		state = EntityState::Available;
		isEnabled = false;
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

// List of all systems in the game
static System* s_systems[MAX_SYSTEMS];
static uint64_t s_numSystems;

static SteadyClock::time_point s_lastFrameTime;

//// TODO: maybe handle window logic in a separate file? Maybe can't
//// because it has to be handled by the game loop? Could be its own set
//// of static functions in a window.cpp file?
// TODO: these should be set via a settings/config file maybe? And should be configurable
// during game loop as well
static constexpr uint64_t MAX_WINDOW_TITLE_LENGTH = 64;
static Color s_backgroundColor;
static Vector2i s_windowSize = Vector2i( 1920, 1080 );
static const char s_windowTitle[MAX_WINDOW_TITLE_LENGTH] = "DOODLE";

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


static void ProcessEntityCreationQueue()
{
	for ( int i = 0; i < s_numQueuedAdditions; ++i )
	{
		QueuedEntityAddition* currAddition = &s_entityAdditionQueue[i];
		if ( !IsValidEntityID( currAddition->claimedID ) )
		{
			COM_ALWAYS_ASSERT( "Queued entity creation has invalid entity ID [%" PRIu64 "]. Max entity ID: %" PRIu64 "", currAddition->claimedID, MAX_ENTITY_ID );
			continue;
		}

		Entity* newEntity = &s_entities[currAddition->claimedID];
		if ( newEntity->state != EntityState::Claimed )
		{
			COM_ALWAYS_ASSERT( "Queued entity with claimed ID of [%" PRIu64 "] is attempting to claim an entity without a 'claimed' state. State: [%i]", currAddition->claimedID, GetUndelyingEnumVal( newEntity->state ) );
			continue;
		}

		newEntity->componentsMask = currAddition->componentsMask;
		newEntity->state = EntityState::Assigned;
	}

	s_numQueuedAdditions = 0;
}


static void ProcessEntityDeletionQueue()
{
	for ( int i = 0; i < s_numQueuedDeletions; ++i )
	{
		EntityID currDeletionID = s_entityDeletionQueue[i];
		if ( !IsValidEntityID( currDeletionID ) )
		{
			COM_ALWAYS_ASSERT( "Queued entity deletion has invalid entity ID [%" PRIu64 "]. Max entity ID: %" PRIu64 "", currDeletionID, MAX_ENTITY_ID );
			continue;
		}

		Entity* entity = &s_entities[currDeletionID];
		if ( entity->state != EntityState::Claimed )
		{
			COM_ALWAYS_ASSERT( "Queued entity deletion with claimed ID of [%" PRIu64 "] is attempting to delete an entity without a 'assigned' state. State: [%i]", currDeletionID, GetUndelyingEnumVal( entity->state ) );
			continue;
		}

		entity->Reset();
	}

	s_numQueuedDeletions = 0;
}


// ====================
// Public Functions
// ====================

const EntityID ECS_QueueEntityCreation( const ComponentsMask compMask )
{
	if ( compMask == INVALID_COMPONENTS_MASK )
	{
		return INVALID_ENTITY_ID;
	}

	if ( s_numQueuedAdditions >= MAX_ENTITY_QUEUE_SIZE )
	{
		COM_ALWAYS_ASSERT( "Unable to add new entity due to addition queue being full. Max addition queue size: %" PRIu64 "", MAX_ENTITY_QUEUE_SIZE );
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

	COM_ALWAYS_ASSERT( "Unable to add new entity due to entities array being full. Entities array size: %" PRIu64 "", MAX_ENTITIES );
	return INVALID_ENTITY_ID;
}


bool ECS_QueueEntityRemoval( const EntityID entityID )
{
	if ( s_numQueuedDeletions >= MAX_ENTITY_QUEUE_SIZE )
	{
		COM_ALWAYS_ASSERT( "Unable to queue entity for deletion since queue is full. Max deletion queue size: %" PRIu64 "", MAX_ENTITY_QUEUE_SIZE );
		return false;
	}

	if ( !IsValidEntityID( entityID ) )
	{
		return false;
	}

	// see if entity even exists 
	const Entity* entity = &s_entities[entityID];
	if ( entity->state != EntityState::Assigned )
	{
		return false;
	}

	s_entityDeletionQueue[s_numQueuedDeletions] = entityID;
	++s_numQueuedDeletions;
	return true;
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


const void ECS_RegisterSystem( System* system )
{
	COM_ASSERT( system, "System pointer is null" );

	if ( s_numSystems >= MAX_SYSTEMS )
	{
		COM_ALWAYS_ASSERT( "Unable to register system since systems list is full. Max systems: %" PRIu64 "", MAX_SYSTEMS );
		return;
	}

	s_systems[s_numSystems] = system;
}


// TODO: have to figure out how scene loader will mass unload entities + mass load in entities 
// in respect to game loop. Might have to integrate it somehow into the start of a frame
// (clear entity list + add new list of entities as a whole). Maybe have a 
// load scene wrapper which calls the load scene functions which return a list of entities? 

void ECS_StartGameLoop()
{
	sf::RenderWindow window( sf::VideoMode( s_windowSize.x, s_windowSize.y ), s_windowTitle );

	// initialize clock
	s_lastFrameTime = SteadyClock::now();

	while ( window.isOpen() )
	{
		// get user input
		sf::Event event;
		while ( window.pollEvent( event ) )
		{
			// see if user has closed application window
			if ( event.type == sf::Event::Closed )
				window.close();
		}

		// - TODO: handle scenes AKA initialize new scene if applicable AKA clear entities list and fill in new entities
		
		// add new queued entities
		ProcessEntityCreationQueue();

		// run all systems
		const SteadyClock::time_point currentTime = SteadyClock::now();
		const double deltaTime = std::chrono::duration<double>( currentTime - s_lastFrameTime ).count();

		// run frame start
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrameStart( deltaTime, s_numEntities, &s_components );
		}

		// run frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrame( deltaTime, s_numEntities, &s_components );
		}

		// run frame end
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrameEnd( deltaTime, s_numEntities, &s_components );
		}
		
		// run physics frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnPhysicsFrame( deltaTime, s_numEntities, &s_components );
		}

		// run drawing frame
		window.clear( s_backgroundColor );
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnDrawFrame( &window, s_numEntities, &s_components );
		}
		window.display();


		// delete all queued deletion entities
		ProcessEntityDeletionQueue();

		s_lastFrameTime = SteadyClock::now();
	}
}
