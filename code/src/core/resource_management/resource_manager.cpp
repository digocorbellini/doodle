#include "resource_manager.h"
#include <SFML/Graphics.hpp>

#define RESOURCE_DELETOR(T) [](void* ptr){ delete static_cast<T*>(ptr); }

using namespace std;
using namespace sf;
using namespace resource_manager_impl;

constexpr size_t MAX_CACHED_RESOURCES = 1 << 19; // ~500,000 resources

// linear probing hash
static CachedResource s_cachedResources[MAX_CACHED_RESOURCES];

// TODO: replace this with a scene type?
// TODO: also look into the option of having a second loaded scene for
// the ability of loading a second scene in the background?
static const char* s_currentlyLoadedScene = nullptr;

// TODO: have to test all of the caching and unloading and getting functions
// ===========================
// Private Resource Loaders
// ===========================

bool CacheResource( HashedString resourceHashName, void* resource, ResourceType resourceType, ResourceDeletorFunc deletorFunct )
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
				GetResourceTypeString( T ) );
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
	Texture* newTexture = new Texture();
	if ( !newTexture->loadFromFile( resourcePath ) )
	{
		delete newTexture;
		return false;
	}


	return CacheResource( resourceHashName, static_cast<void*>( newTexture ), ResourceType::Texture, RESOURCE_DELETOR( Texture ) );
}


// ===========================
// Private Helpers
// ===========================

bool UnloadResource( HashedString resourceHashName )
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


void UnloadAllResources()
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


// ===========================
// Public Functions
// ===========================

void ResourceMannager_LoadSceneAssets( const char* sceneFileName )
{
	// TODO: sceneFileName is a JSON file which contains the defintion of all of the assets to load?
	// Maybe should be a scene object instead which 
	
}


void ResourceMannager_UnloadCurrentScene()
{
	if ( !s_currentlyLoadedScene )
	{
		return;
	}

	UnloadAllResources();
}
