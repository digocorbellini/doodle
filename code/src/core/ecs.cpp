#include "ecs.h"
#include <chrono>
#include <cinttypes>
#include "common/lib/com_print.h"
#include "common/lib/com_assert.h"
#include "core/system.h"
#include <typeindex>

using SteadyClock = std::chrono::steady_clock;

static constexpr EntityID MAX_ENTITIES = 5000;
static constexpr uint64_t MAX_ENTITY_QUEUE_SIZE = MAX_ENTITIES / 2;
static constexpr uint64_t MAX_SYSTEMS = 64;

// Systems will operate on this struct of all components in the game
// Every entity has an entry in every list. Their component is
// accessed by indexing using their entity id.
struct Components
{
	// Have a list for every component type in the game. This
	// allows for componenets of the same type to be near each
	// other in memory since they will likely be accessed
	// together as systems iterate through all entities.
#define COMPONENT(X) X X ## List[MAX_ENTITIES];
	COMPONENT_LIST
#undef COMPONENT
};

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

static Entity s_entities[MAX_ENTITIES];
// TODO: have to figure out if this is even needed since dynamic addition and 
// deletion will make it so that the entities in the list are no longer stored consequtively... might have to change
// what is being passed in the frame functions... maybe just pass MAX_ENTITIES?
// TODO: maybe it would be good to still keep this count and then create an iterator that can be passed to the systems
// that way the iterator can skip giving the systems un-operable entities and it can know when it has reached the end of the 
// list since it can know the number of entities (AKA it avoids having to iterate over the entire list every time). This
// also helps encapsulate the entities list and the systems just have to work with a simple list 
// that gets entity IDs from the iterator.
static EntityID s_numEntities; 
static_assert( ARRAY_SIZE( s_entities ) < MAX_ENTITY_ID, "s_entities array size is >= MAX_ENTITY_ID" );

static Components s_components;
static void* s_componentsList[] =
{
#define COMPONENT(X) s_components. ## X ## List,
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentsList ),
			   "s_componentsList and ComponentType length missmatch" );

// used to map component type enum to component type
static std::type_index s_componentTypeMap[] =
{
#define COMPONENT(X) typeid(X),
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentTypeMap ),
			   "s_componentTypeMap and ComponentType length missmatch" );

static System* s_systems[MAX_SYSTEMS];
static uint64_t s_numSystems;
static SteadyClock::time_point s_lastFrameTime;

static QueuedEntityAddition s_entityAdditionQueue[MAX_ENTITY_QUEUE_SIZE];
static uint64_t s_numQueuedAdditions;

static EntityID s_entityDeletionQueue[MAX_ENTITY_QUEUE_SIZE];
static uint64_t s_numQueuedDeletions;

//// TODO: maybe handle window logic in a separate file? Maybe can't
//// because it has to be handled by the game loop? Could be its own set
//// of static functions in a window.cpp file?
// TODO: these should be set via a settings/config file maybe? And should be configurable
// during game loop as well
static constexpr uint64_t MAX_WINDOW_TITLE_LENGTH = 64;
static Color s_backgroundColor;
static Vector2i s_windowSize = Vector2i( 1920, 1080 );
static const char s_windowTitle[MAX_WINDOW_TITLE_LENGTH] = "DOODLE";

// ====================
// Private Helpers
// ====================

static bool IsValidEntityID( const EntityID id )
{
	return id >= 0 && id < MAX_ENTITIES && id != INVALID_ENTITY_ID;
}


static void ProcessEntityCreationQueue()
{
	for ( int i = 0; i < s_numQueuedAdditions; ++i )
	{
		QueuedEntityAddition* currAddition = &s_entityAdditionQueue[i];
		if ( !IsValidEntityID( currAddition->claimedID ) )
		{
			COM_ALWAYS_ASSERT( "Queued entity creation has invalid entity ID [%" PRIu64 "]. Max entities: %" PRIu64 "\n", currAddition->claimedID, MAX_ENTITIES );
			continue;
		}

		Entity* newEntity = &s_entities[currAddition->claimedID];
		if ( newEntity->state != EntityState::Claimed )
		{
			COM_ALWAYS_ASSERT( "Queued entity with claimed ID of [%" PRIu64 "] is attempting to claim an entity without a 'claimed' state. State: [%i]\n", currAddition->claimedID, GetUndelyingEnumVal( newEntity->state ) );
			continue;
		}

		newEntity->componentsMask = currAddition->componentsMask;
		newEntity->state = EntityState::Assigned;
		
		COM_ASSERT( s_numEntities < MAX_ENTITIES, "num entities is exceeding max number of entities:  %" PRIu64 "\n", MAX_ENTITIES );
		++s_numEntities;
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
			COM_ALWAYS_ASSERT( "Queued entity deletion has invalid entity ID [%" PRIu64 "]. Max entity ID: %" PRIu64 "\n", currDeletionID, MAX_ENTITY_ID );
			continue;
		}

		Entity* entity = &s_entities[currDeletionID];
		if ( entity->state != EntityState::Assigned )
		{
			COM_ALWAYS_ASSERT( "Queued entity deletion with claimed ID of [%" PRIu64 "] is attempting to delete an entity without a 'assigned' state. State: [%i]\n", currDeletionID, GetUndelyingEnumVal( entity->state ) );
			continue;
		}

		entity->Reset();

		COM_ASSERT( s_numEntities > 0, "num entities is already 0 but attempting to decrement it.\n");
		--s_numEntities;
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
		COM_ALWAYS_ASSERT( "Unable to add new entity due to addition queue being full. Max addition queue size: %" PRIu64 "\n", MAX_ENTITY_QUEUE_SIZE );
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

	COM_ALWAYS_ASSERT( "Unable to add new entity due to entities array being full. Entities array size: %" PRIu64 "\n", MAX_ENTITIES );
	return INVALID_ENTITY_ID;
}


bool ECS_QueueEntityRemoval( const EntityID entityID )
{
	if ( s_numQueuedDeletions >= MAX_ENTITY_QUEUE_SIZE )
	{
		COM_ALWAYS_ASSERT( "Unable to queue entity for deletion since queue is full. Max deletion queue size: %" PRIu64 "\n", MAX_ENTITY_QUEUE_SIZE );
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


bool ECS_CanOperateOnEntity( const EntityID entityID )
{
	if ( !IsValidEntityID( entityID ) )
	{
		return false;
	}

	const Entity* entity = &s_entities[entityID];
	return entity->CanBeOperatedOn();
}


void ECS_RegisterSystem( System* system )
{
	COM_ASSERT( system, "System pointer is null\n" );

	if ( s_numSystems >= MAX_SYSTEMS )
	{
		COM_ALWAYS_ASSERT( "Unable to register system since systems list is full. Max systems: %" PRIu64 "\n", MAX_SYSTEMS );
		return;
	}

	s_systems[s_numSystems] = system;
	++s_numSystems;
}

template<typename T>
T* ECS_GetComponentList( ComponentType componentType )
{
	if ( !Components_IsComponentValid( componentType ) )
	{
		return nullptr;
	}

	COM_ASSERT( ( typeid( T ) == s_componentTypeMap[GetUndelyingEnumVal( componentType )] ), "Type '%s' does not match expected type '%s' for given component type '%s'\n", typeid( T ).name(), s_componentTypeMap[GetUndelyingEnumVal( componentType )].name(), Components_GetComponentTypeString( componentType ) );

	return static_cast<T*>( s_componentsList[GetUndelyingEnumVal( componentType )] );
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
		// TODO: have to figure out how scene loader will communicate with this ECS system
		
		// add new queued entities
		ProcessEntityCreationQueue();

		// run all systems
		const SteadyClock::time_point currentTime = SteadyClock::now();
		const double deltaTime = std::chrono::duration<double>( currentTime - s_lastFrameTime ).count();

		// run frame start
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrameStart( deltaTime, s_numEntities );
		}

		// run frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrame( deltaTime, s_numEntities );
		}

		// run frame end
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnFrameEnd( deltaTime, s_numEntities );
		}
		
		// run physics frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnPhysicsFrame( deltaTime, s_numEntities );
		}

		// run drawing frame
		window.clear( s_backgroundColor );
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			s_systems[i]->OnDrawFrame( &window, s_numEntities );
		}
		window.display();


		// delete all queued deletion entities
		ProcessEntityDeletionQueue();

		s_lastFrameTime = SteadyClock::now();
	}
}
