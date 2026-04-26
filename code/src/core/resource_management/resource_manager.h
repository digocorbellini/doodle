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

template <typename T>
struct ResourceHandle
{
	HashedString handle = INVALID_HASHED_STRING;

	inline bool IsInvalid()
	{
		return handle == INVALID_HASHED_STRING;
	}
};

/// <summary>
/// Load the given asset and store it using the given resourceHashName
/// </summary>
/// <param name="resourceHashName">The hash name for the resource used to retrieve the resource</param>
/// <param name="resourcePath">the path to the resource</param>
/// <param name="resourceType">the type of the resource</param>
/// <returns></returns>
bool ResourceManager_LoadResource( const HashedString resourceHashName, const char* resourcePath, const ResourceType resourceType );

/// <summary>
/// Unload all currently loaded resources.
/// </summary>
void ResourceManager_UnloadAllResources();


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
	// defined function in header due to it being templated

	if ( resourcehandle.IsInvalid() )
	{
		return nullptr;
	}
	// TODO: maybe figure out a way to return a temporary resource that does not allow
	// caching between frames, so systems have to request the resouce using the handle
	// every frame = prevents case where user is using a pointer to a resource that has
	// been freed / moved to another space in memory... or just don't clear the resouce
    // from memory until the scene unloads?

	const resource_manager_impl::CachedResource* resource = resource_manager_impl::GetCachedResource( resourcehandle.handle.GetHash() );
	if ( !resource )
	{
		return nullptr;
	}

	COM_ASSERT( ResourceTypes_ResourceTypeForType<T>() == resource->type, "%s - resource type requested (%s) is not the same as the actual type of the resource (%s).\n", __FUNCTION__,
				ResourceTypes_GetResourceTypeString( ResourceTypes_ResourceTypeForType<T>() ), ResourceTypes_GetResourceTypeString( resource->type ) );

	return static_cast<T*>( resource->resourcePtr );
}