#include "test_system.h"
#include "common/lib/com_print.h"
#include "core/resource_management/resource_manager.h"
#include "core/scene_loading/scene_loader.h"
#include "common/lib/com_string.h"

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

	if ( !haveRun )
	{
		SceneLoader loader;
			
		loader.LoadScene("test_scene.json");

		HashedString poggiesHash( "poggies.png" );
		Com_Printf( "Hash string for hash 'poggines.png': %s\n", poggiesHash.GetStringForHash() );
		ResourceManager_LoadResource( poggiesHash, OBFUSCATED_STRING( "poggies.png" ), ResourceType::Texture );
		Texture* poggiesTexture = ResourceManager_GetResource( ResourceHandle<sf::Texture>( poggiesHash ) );
		ResourceType type = ResourceTypes_ResourceTypeForType<sf::Texture>();
		Com_Printf( "TEST: resource type: %s\n", ResourceTypes_GetResourceTypeString( type ) );

		if ( ResourceManager_LoadResource( HashedString( "test_dir/true.png" ), OBFUSCATED_STRING("test_dir/true.png"), ResourceType::Texture ) )
		{
			Texture* trueTexture = ResourceManager_GetResource( ResourceHandle<sf::Texture>( HashedString( "test_dir/true.png" ) ) );
			Com_Printf( "LOADED TRUE\n" );
		}
		else
		{
			Com_Printf( "Failed to load true\n" );
		}
		Com_Printf( OBFUSCATED_STRING( "Hash string for hash 'test_dir/true.png': %s\n"), HashedString( "test_dir/true.png" ).GetStringForHash());


		haveRun = true;
	}
}
