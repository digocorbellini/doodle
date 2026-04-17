#include "test_system.h"
#include "common/lib/com_print.h"
#include "core/resource_management/resource_manager.h"

static TestSystem testSystem;

static NanoSeconds s_printIntervalNs = 1 * 1000 * 1000 * 1000;
static NanoSeconds s_timeElapsedNs = 0;

static bool haveRun = false;

void TestSystem::OnFrame( const NanoSeconds deltaTimeNs, const EntityIterator* entityIterator )
{
	s_timeElapsedNs += deltaTimeNs;
	if ( s_timeElapsedNs > s_printIntervalNs )
	{
		Com_Printf( "HOWDY GAMER!!! Delta time: %zu, elapsed time %zu\n", deltaTimeNs, s_timeElapsedNs);
		s_timeElapsedNs = 0;
	}

	if ( !haveRun )
	{
		Texture* texture = ResourceManager_GetResource( ResourceHandle<sf::Texture>( HashedString( "poggies.png" ) ) );
		ResourceType type = ResourceTypes_ResourceTypeForType<sf::Texture>();
		Com_Printf( "TEST: resource type: %s\n", ResourceTypes_GetResourceTypeString( type ) );
		haveRun = true;
	}
}
