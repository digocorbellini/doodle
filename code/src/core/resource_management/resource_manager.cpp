#include "common/lib/com_assert.h"
#include "common/types.h"
#include "resource_manager.h"
#include "resource_types.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;


constexpr size_t MAX_CACHED_RESOURCES = 1 << 19; // ~500,000 resources

typedef void ( *ResourceDeletorFunc )( void* );
#define RESOURCE_DELETOR(T) [](void* ptr){ delete static_cast<T*>(ptr); }

struct CachedResource
{
	void* resourcePtr = nullptr;
	ResourceType type = ResourceType::INVALID;
	HashedString hash = INVALID_HASHED_STRING;
	ResourceDeletorFunc deletorFunc;

	void Reset()
	{
		type = ResourceType::INVALID;
		hash = INVALID_HASHED_STRING;
		deletorFunc( resourcePtr );
		resourcePtr = nullptr;
	}
};

// linear probing hash
static CachedResource s_cachedResources[MAX_CACHED_RESOURCES];

// TODO: replace this with a scene type?
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
		if ( currResource->hash == INVALID_HASHED_STRING )
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
bool LoadAndCacheResource<ResourceType::INVALID>( HashedString resourceHashName, const char* resourcePath )
{
	COM_ASSERT( false, "%s - 'Invalid' resource type is not supported for loading.\n", __FUNCTION__ );
	return false;
}


template <>
bool LoadAndCacheResource<ResourceType::TEXTURE>( HashedString resourceHashName, const char* resourcePath )
{
	Texture* newTexture = new Texture();
	if ( !newTexture->loadFromFile( resourcePath ) )
	{
		delete newTexture;
		return false;
	}


	return CacheResource( resourceHashName, static_cast<void*>( newTexture ), ResourceType::TEXTURE, RESOURCE_DELETOR( Texture ) );
}


// ===========================
// Private Helpers
// ===========================

bool UnloadResource( HashedString resourceHashName )
{
	const size_t resourceIndex = resourceHashName.GetHash() % MAX_CACHED_RESOURCES;

	// iterate through array starting at hash index until an empty slot is found in cache
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


const CachedResource* GetCachedResource( HashedString resourceHashName )
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
// Public Functions/Methods
// ===========================

void LoadSceneAssets( const char* sceneFileName )
{
	// TODO: sceneFileName is a JSON file which contains the defintion of all of the assets to load?
	// Maybe should be a scene object instead which 

}


// TODO: maybe figure out a way to return a temporary resource that does not allow
// caching between frames, so systems have to request the resouce using the handle 
// every frame = prevents case where user is using a pointer to a resource that has
// been freed/moved to another space in memory
template<typename T>
T* GetResource( ResourceHandle<T> resourcehandle )
{
	const CachedResource* resource = GetCachedResource( resourcehandle.handle.GetHash() );
	if ( !resource )
	{
		return nullptr;
	}

	COM_ASSERT( ResourceTypeForType<T>() == resource->type, "%s - resource type requested (%s) is not the same as the actual type of the resource (%s).\n",
				GetResourceTypeString( ResourceTypeForType<T>() ), GetResourceTypeString( resource->type ) );

	return static_cast<T*>( resource->resourcePtr );
}
