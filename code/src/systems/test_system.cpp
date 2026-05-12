#include "test_system.h"
#include "common/lib/com_print.h"
#include "core/resource_management/resource_manager.h"
#include "core/scene_loading/scene_loader.h"
#include "common/lib/com_string.h"
#include <SFML/Window/Keyboard.hpp>

using namespace sf;

static TestSystem testSystem;

static NanoSeconds s_printIntervalNs = 1 * 1000 * 1000 * 1000;
static NanoSeconds s_timeElapsedNs = 0;

static bool haveRun = false;

void TestSystem::OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator )
{
	s_timeElapsedNs += deltaTimeNs;
	if ( s_timeElapsedNs > s_printIntervalNs )
	{
		//Com_Printf( "HOWDY GAMER!!! Delta time: %zu, elapsed time %zu\n", deltaTimeNs, s_timeElapsedNs);
		s_timeElapsedNs = 0;
	}

	/*if ( Keyboard::isKeyPressed(Keyboard::A) )
	{
		Com_Printf( "A key pressed!!!!!\n" );
	}*/

	if ( !haveRun )
	{
		//const char* testScenename = OBFUSCATED_STRING( "test_circular_reference_scene.json" );
		const char* testScenename = OBFUSCATED_STRING( "test_scene.json" );
		ECS_QueueSceneLoad( testScenename );

		const HashedString poggiesHash = STATIC_HASHED_STRING( "poggies.png" );
		Com_Printf( "Hash string for hash 'poggines.png': %s\n", poggiesHash.GetStringForHash() );
		ResourceManager_LoadResource( poggiesHash, OBFUSCATED_STRING( "poggies.png" ), ResourceType::Texture );
		ResourceType type = ResourceTypes_ResourceTypeForType<sf::Texture>();
		Com_Printf( "TEST: resource type: %s\n", ResourceTypes_GetResourceTypeString( type ) );

		const HashedString trueHash = STATIC_HASHED_STRING( "test_dir/true.png" );
		if ( ResourceManager_LoadResource( trueHash , OBFUSCATED_STRING("test_dir/true.png"), ResourceType::Texture ) )
		{
			Com_Printf( "LOADED TRUE\n" );
		}
		else
		{
			Com_Printf( "Failed to load true\n" );
		}
		Com_Printf( OBFUSCATED_STRING( "Hash string for hash 'test_dir/true.png': %s\n"), trueHash.GetStringForHash());


		haveRun = true;
	}
}
