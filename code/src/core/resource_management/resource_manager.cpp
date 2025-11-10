#include "common/lib/com_assert.h"
#include "common/types.h"
#include "resource_manager.h"
#include "resource_types.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

// TODO: potentially create a static cache of resources which can be added to and removed from
// as scenes are loaded and unloaded
// Can be structured as so:
// - Create a struct which contains: HashedString (used for hashing), pointer to resource
// - Create a static array of these structs and use this as a linear probing map which you can index
//   into using HashedStrings
// - Dynamically allocate resources using heap and assign those pointers to the struct
//		- Might have to store the resources as void pointers in the struct and then cast them
//		  to the desired type when they are requested using templated resources 
constexpr size_t MAX_CACHED_RESOURCES = 1 << 19; // ~500,000 resources

struct CachedResource
{
	void* resourcePtr;
	ResourceType type;
};

static CachedResource s_cachedResources[MAX_CACHED_RESOURCES];


// ===========================
// Private Resource Loaders
// ===========================

bool CacheResource( HashedString resourceHashName, void* resource, ResourceType resourceType )
{
	// TODO: implement caching using resourceHashName for indexing

	// TODO: assert of cache is full. Maybe can eventually implement LRU caching instead
	// but will have to make it so that getter will also load in resources if they are no longer
	// loaded so that user can safely assume that their resources are still in memory... but that
	// may not work?? B/c what if a component has a Sprite object which takes in a texture pointer?
	// Is it possible to have Sprite use a pointer to the texture but then have to re-set it 
	// the next frame? Maybe just don't ever unload until it is no longer needed explicitly.
	return true;
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

	return CacheResource( resourceHashName, static_cast<void*>( newTexture ), ResourceType::TEXTURE );
}


// ===========================
// Private Helpers
// ===========================

const CachedResource* GetCachedResource( HashedString resourceHashName )
{
	// TODO: implement getting a resource given a hash
	return nullptr;
}


// ===========================
// Public Functions/Methods
// ===========================

// TODO: do away with the resource manager class and just use functions since we will be working
// with a static resource buffer instead... unless I want multiple resource managers at once
// for loading multiple scenes at once... nah that would get messy
ResourceManager::ResourceManager()
{

}


ResourceManager& ResourceManager::GetInstance()
{
	static ResourceManager instance;
	
	return instance;
}


void ResourceManager::LoadSceneAssets( const char* sceneFileName )
{
	// TODO: sceneFileName is a JSON file which contains the defintion of all of the assets to load?

}


// TODO: maybe figure out a way to return a temporary resource that does not allow
// caching between frames, so systems have to request the resouce using the handle 
// every frame = prevents case where user is using a pointer to a resource that has
// been freed/moved to another space in memory
template<typename T>
T* ResourceManager::GetResource( ResourceHandle<T> resourcehandle )
{
	const CachedResource* resource = GetCachedResource( resourcehandle.handle.GetHash() );
	if ( !resource )
	{
		return nullptr;
	}

	// TODO: maybe this should be dev only? Or asserts as a whole should be dev only
	COM_ASSERT( ResourceTypeForType<T>() == resource->type, "%s - resource type requested (%s) is not the same as the actual type of the resource (%s).\n",
				GetResourceTypeString( ResourceTypeForType<T>() ), GetResourceTypeString( resource->type ) );

	return static_cast<T*>( resource->resourcePtr );
}


ResourceManager::~ResourceManager()
{

}