#include "common/lib/com_print.h"
#include "resource_manager.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;
using namespace resource_manager_impl;

#define RESOURCE_MANAGER_STR "ResourceManager" 
#define RESOURCES_DIR_PATH GAME_DIR_PATH "resources/"

#define RESOURCE_DELETOR(T) [](void* ptr){ delete static_cast<T*>(ptr); }

static constexpr size_t MAX_CACHED_RESOURCES = 1 << 19; // ~500,000 resources

static constexpr size_t MAX_RESOURCE_PATH_LEN = 512;

static char s_fullResourcePath[MAX_RESOURCE_PATH_LEN];

// linear probing hash
static CachedResource s_cachedResources[MAX_CACHED_RESOURCES];

// TODO: replace this with a scene type?
// TODO: also look into the option of having a second loaded scene for
// the ability of loading a second scene in the background?
static const char* s_currentlyLoadedScene = nullptr;

// ===========================
// Private Helpers
// ===========================

static const char* GetFullResourcePath( const char* resourcePath )
{
	if ( !resourcePath )
	{
		return nullptr;
	}

	COM_ASSERT( ( strlen( resourcePath ) + strlen( RESOURCES_DIR_PATH ) ) < MAX_RESOURCE_PATH_LEN, "[%s]: given resource path '%s%s' exceeds max resource path length of '%zu'\n", RESOURCE_MANAGER_STR, RESOURCES_DIR_PATH, resourcePath, MAX_RESOURCE_PATH_LEN );

	snprintf( s_fullResourcePath, MAX_RESOURCE_PATH_LEN, "%s%s", RESOURCES_DIR_PATH, resourcePath );
	return s_fullResourcePath;
}


static bool UnloadResource( HashedString resourceHashName )
{
	const size_t resourceIndex = resourceHashName.GetHash() % MAX_CACHED_RESOURCES;

	// iterate through array starting at hash index until resource for hash is found
	for ( size_t i = 0; i < MAX_CACHED_RESOURCES; ++i )
	{
		const size_t currIndex = ( resourceIndex + i ) % MAX_CACHED_RESOURCES;
		CachedResource* currResource = &s_cachedResources[currIndex];
		if ( currResource->hash == resourceHashName )
		{
			currResource->Reset();
			return true;
		}
	}

	// resource not found
	return false;
}


static void UnloadAllResources()
{
	for ( size_t i = 0; i < MAX_CACHED_RESOURCES; ++i )
	{
		CachedResource* currResource = &s_cachedResources[i];
		if ( !currResource->IsEmpty() )
		{
			currResource->Reset();
		}
	}
}


const CachedResource* resource_manager_impl::GetCachedResource( HashedString resourceHashName )
{
	const size_t resourceIndex = resourceHashName.GetHash() % MAX_CACHED_RESOURCES;

	// iterate through array starting at hash index until resource for hash is found
	for ( size_t i = 0; i < MAX_CACHED_RESOURCES; ++i )
	{
		const size_t currIndex = ( resourceIndex + i ) % MAX_CACHED_RESOURCES;
		CachedResource* currResource = &s_cachedResources[currIndex];
		if ( currResource->hash == resourceHashName )
		{
			return currResource;
		}
	}

	// resource is not cached
	return nullptr;
}


const bool CachedResourceExists( HashedString resourceHashName )
{
	return resource_manager_impl::GetCachedResource( resourceHashName ) != nullptr;
}


// ===========================
// Private Resource Loaders
// ===========================

static bool CacheResource( HashedString resourceHashName, void* resource, ResourceType resourceType, ResourceDeletorFunc deletorFunct )
{
	const size_t newIndex = resourceHashName.GetHash() % MAX_CACHED_RESOURCES;

	// iterate through array starting at hash index until an empty slot is found in cache
	for ( size_t i = 0; i < MAX_CACHED_RESOURCES; ++i )
	{
		const size_t currIndex = ( newIndex + i ) % MAX_CACHED_RESOURCES;
		CachedResource* currResource = &s_cachedResources[currIndex];
		if ( currResource->IsEmpty() )
		{
			currResource->resourcePtr = resource;
			currResource->type = resourceType;
			currResource->hash = resourceHashName;
			currResource->deletorFunc = deletorFunct;
			return true;
		}
		else if (currResource->hash == resourceHashName )
		{
			// resource has already been cached
			Com_PrintfWarningVerbose( RESOURCE_MANAGER_STR, "Attempting to cache resource that is already cached. Resource: %s", resourceHashName.GetStringForHash() );
			return false;
		}
	}

	COM_ASSERT( false, "%s - no empty slot found for caching resource '%s'. Might need to increase resource cache size. Current max: %zu\n",
				__FUNCTION__,
				resourceHashName.GetStringForHash(),
				MAX_CACHED_RESOURCES );

	return false;
}


template <ResourceType T>
bool LoadAndCacheResource( HashedString resourceHashName, const char* resourcePath )
{
	COM_ASSERT( false, "%s - Resource type '%s' does not have its own loading function.\n",
				__FUNCTION__,
				ResourceTypes_GetResourceTypeString( T ) );
	return false;
}


template<>
bool LoadAndCacheResource<ResourceType::Invalid>( HashedString resourceHashName, const char* resourcePath )
{
	COM_ASSERT( false, "%s - 'Invalid' resource type is not supported for loading.\n", __FUNCTION__ );
	return false;
}


template <>
bool LoadAndCacheResource<ResourceType::Texture>( HashedString resourceHashName, const char* resourcePath )
{
	if ( CachedResourceExists( resourceHashName ) )
	{
		return false;
	}

	const char* fullResourcePath = GetFullResourcePath( resourcePath );
	if ( !fullResourcePath )
	{
		return false;
	}

	Texture* newTexture = new Texture();
	if ( !newTexture->loadFromFile( fullResourcePath ) )
	{
		delete newTexture;
		return false;
	}

	return CacheResource( resourceHashName, static_cast<void*>( newTexture ), ResourceType::Texture, RESOURCE_DELETOR( Texture ) );
}


// ===========================
// Public Functions
// ===========================

void ResourceManager_LoadSceneAssets( const char* sceneFileName )
{
	// TODO: sceneFileName is a JSON file which contains the defintion of all of the assets to load?
	// Maybe should be a scene object instead which 
	LoadAndCacheResource<ResourceType::Texture>( HashedString( "poggies.png" ), "poggies.png" );
	
}


void ResourceManager_UnloadCurrentScene()
{
	if ( !s_currentlyLoadedScene )
	{
		return;
	}

	UnloadAllResources();
}
