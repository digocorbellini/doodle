#include "ecs.h"
#include <chrono>
#include <cinttypes>
#include "common/lib/com_print.h"
#include "common/lib/com_assert.h"
#include "core/system.h"
#include <typeindex>

using SteadyClock = std::chrono::steady_clock;

static constexpr EntityID MAX_ENTITIES = 5000;
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
	Claimed,        // claimed by queue addition system 
	Assigned,       // entity is assigned an entity ID by ecs system
	MarkedForDelete // entity is still active but will be deleted at the end of the frame
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
		return isEnabled && (state == EntityState::Assigned || state == EntityState::MarkedForDelete);
	}
};

struct QueuedEntityAddition
{
	ComponentsMask componentsMask;
	EntityID claimedID;
};

static Entity s_entities[MAX_ENTITIES];
static EntityID s_numEntities = 0; 
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


static void ProcessEntityCreationAndDeletion()
{
	for ( EntityID entityID = 0; entityID < MAX_ENTITIES; ++entityID )
	{
		Entity* currEntity = &s_entities[entityID];

		if ( currEntity->state == EntityState::Claimed )
		{
			// add entity
			currEntity->state = EntityState::Assigned;
			COM_ASSERT( s_numEntities < MAX_ENTITIES, "num entities is exceeding max number of entities:  %" PRIu64 "\n", MAX_ENTITIES );
			++s_numEntities;
		}
		else if ( currEntity->state == EntityState::MarkedForDelete )
		{
			// delete entity
			currEntity->Reset();
			COM_ASSERT( s_numEntities > 0, "num entities is already 0 but attempting to decrement it.\n" );
			--s_numEntities;
		}
	}
}


// ====================
// Entity Iterator
// ====================

// This class exists so that only the ECS system can decide when to create iterators. 
// This is because the EntityIterator class depends on the s_entities list not changing
// as systems iterate over entities.
class EntityIteratorCreator
{
public:
	static EntityIterator CreateEntityIterator()
	{
		return EntityIterator();
	}

	static EntityIterator CreateInvalidEntityIterator()
	{
		return EntityIterator( INVALID_ENTITY_ID );
	}
};
const static EntityIterator ENTITY_ITERATOR_END = EntityIteratorCreator::CreateInvalidEntityIterator();


static EntityID GetNextValidEntityID( const EntityID startingID )
{
	if ( !IsValidEntityID( startingID ) )
	{
		return INVALID_ENTITY_ID;
	}

	EntityID nextValidEntityID = INVALID_ENTITY_ID;
	for ( EntityID entityIndex = startingID; entityIndex < MAX_ENTITIES; ++entityIndex )
	{
		Entity* currEntity = &s_entities[entityIndex];
		if ( currEntity->CanBeOperatedOn() )
		{
			nextValidEntityID = entityIndex;
			break;
		}
	}
	
	return nextValidEntityID;
}


EntityIterator::EntityIterator()
{
	// no entities to iterate over exist
	if ( s_numEntities == 0 )
	{
		count = 0;
		currEntity = INVALID_ENTITY_ID;
		return;
	}

	// find starting index (AKA first entity that can be operated on)
	currEntity = GetNextValidEntityID( 0 );
	count = 1;
}


EntityIterator::EntityIterator( const EntityID startingID )
{
	if ( !IsValidEntityID( startingID ) )
	{
		currEntity = INVALID_ENTITY_ID;
		count = 0;
		return;
	}

	currEntity = GetNextValidEntityID( startingID );
	count = 1;
}


EntityIterator EntityIterator::operator++()
{
	// see if we have reached end
	if ( currEntity == INVALID_ENTITY_ID || count >= s_numEntities )
	{
		currEntity = INVALID_ENTITY_ID;
		return *this;
	}

	// get next operable entry
	currEntity = GetNextValidEntityID( currEntity + 1 );
	++count;
	return *this;
}


EntityID EntityIterator::operator*() const
{
	return currEntity;
}


bool EntityIterator::operator!=( const EntityIterator& other ) const
{
	return currEntity != other.currEntity;
}


const EntityIterator EntityIterator::end()
{
	return ENTITY_ITERATOR_END;
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

	// clainm first available entity for addition
	for ( EntityID entityID = 0; entityID < MAX_ENTITIES; ++entityID )
	{
		Entity* currEntity = &s_entities[entityID];
		if ( currEntity->state == EntityState::Available )
		{
			currEntity->state = EntityState::Claimed;
			currEntity->componentsMask = compMask;
			currEntity->isEnabled = true;

			return entityID;
		}
	}

	COM_ALWAYS_ASSERT( "Unable to add new entity due to entities array being full. Entities array size: %" PRIu64 "\n", MAX_ENTITIES );
	return INVALID_ENTITY_ID;
}


bool ECS_QueueEntityRemoval( const EntityID entityID )
{
	if ( !IsValidEntityID( entityID ) )
	{
		return false;
	}

	// see if entity can be deleted
	Entity* entity = &s_entities[entityID];
	if ( entity->state != EntityState::Assigned )
	{
		return false;
	}

	entity->state = EntityState::MarkedForDelete;

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
		
		// run all systems
		const SteadyClock::time_point currentTime = SteadyClock::now();
		const double deltaTime = std::chrono::duration<double>( currentTime - s_lastFrameTime ).count();

		// run frame start
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			EntityIterator it = EntityIteratorCreator::CreateEntityIterator();
			s_systems[i]->OnFrameStart( deltaTime, &it );
		}

		// run frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			EntityIterator it = EntityIteratorCreator::CreateEntityIterator();
			s_systems[i]->OnFrame( deltaTime, &it );
		}

		// run frame end
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			EntityIterator it = EntityIteratorCreator::CreateEntityIterator();
			s_systems[i]->OnFrameEnd( deltaTime, &it );
		}
		
		// run physics frame
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			EntityIterator it = EntityIteratorCreator::CreateEntityIterator();
			s_systems[i]->OnPhysicsFrame( deltaTime, &it );
		}

		// run drawing frame
		window.clear( s_backgroundColor );
		for ( uint64_t i = 0; i < s_numSystems; ++i )
		{
			EntityIterator it = EntityIteratorCreator::CreateEntityIterator();
			s_systems[i]->OnDrawFrame( &window, &it );
		}
		window.display();

		// process all entity additions and deletions
		ProcessEntityCreationAndDeletion();

		s_lastFrameTime = SteadyClock::now();
	}
}
