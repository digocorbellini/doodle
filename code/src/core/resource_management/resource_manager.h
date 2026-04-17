////////////////////////////////////////////////////////////
//
// The resource manager is meant to handle all memory 
// management related to loading, caching, and freeing 
// game assets such as images, sounds, etc. This will
// also distribute the resources to the systems
// that request them.
// 
////////////////////////////////////////////////////////////

#pragma once
#include "common/hashing/hashed_string.h"
#include "common/lib/com_assert.h"
#include "resource_manager_internal.h"

// TODO: replace this with a config file somehow
#define RELATIVE_ASSETS_PATH "../assets"
#define SCENES_DIR_NAME "scenes"

template <typename T>
struct ResourceHandle
{
	HashedString handle = INVALID_HASHED_STRING;
};



// TODO: need to determine someway to determine where to load from.
// Also need some way to unload and reload assets.
// Idk if we want to be fully tied to "scenes" at all times
// because there might be some assets we want loaded at all times 
// (like loading scenes maybe? or audio that is used between scenes)
//		- maybe have the same thing as unity where they "don't destroy on load"
//			- so it persists between scenes?
// Maybe instead of having a reference to a scene file? Why not only load it once
// and then pass it here and also in the entity parsing.
void ResourceManager_LoadSceneAssets( const char* sceneFileName );


/// <summary>
/// Unload all resources for the currently loaded scene if a currently loaded 
/// scene exists.
/// </summary>
void ResourceManager_UnloadCurrentScene();


/// <summary>
/// Get a pointer to the given resource if it has been loaded.
/// </summary>
/// <typeparam name="T">The type of the resource. Will determine
/// the type of the pointer</typeparam>
/// <param name="resourcehandle">The handle which is used to identify the resource</param>
/// <returns>A pointer of type T to the given resource if it exists and has been loaded, 
/// otherwise returns nullptr.</returns>
template<typename T>
T* ResourceManager_GetResource( ResourceHandle<T> resourcehandle )
{
	// TODO: maybe figure out a way to return a temporary resource that does not allow
	// caching between frames, so systems have to request the resouce using the handle
	// every frame = prevents case where user is using a pointer to a resource that has
	// been freed / moved to another space in memory... or just don't clear the resouce
    // from memory until the scene unloads?

	// defined function in header due to it being templated

	const resource_manager_impl::CachedResource* resource = resource_manager_impl::GetCachedResource( resourcehandle.handle.GetHash() );
	if ( !resource )
	{
		return nullptr;
	}

	COM_ASSERT( ResourceTypes_ResourceTypeForType<T>() == resource->type, "%s - resource type requested (%s) is not the same as the actual type of the resource (%s).\n", __FUNCTION__,
				ResourceTypes_GetResourceTypeString( ResourceTypes_ResourceTypeForType<T>() ), ResourceTypes_GetResourceTypeString( resource->type ) );

	return static_cast<T*>( resource->resourcePtr );
}