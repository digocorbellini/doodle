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

/// <summary>
/// A wrapper around resource pointers which resolves the resource pointer at 
/// access time and prevents users from storing raw pointers ensuring that
/// resources remain valid.
/// </summary>
/// <typeparam name="T">The type of a resource</typeparam>
template <ValidResource T>
class ResourcePtr
{
private:
	resource_manager_impl::ResourceHandle<T> m_handle;

	T* Get() const
	{
		if ( m_handle.IsInvalid() )
		{
			return nullptr;
		}

		return resource_manager_impl::ResourceManager_GetResource<T>( m_handle );
	}

public:
	ResourcePtr() : m_handle( INVALID_HASHED_STRING ) {}
	ResourcePtr( const resource_manager_impl::ResourceHandle<T> handle ) : m_handle( handle ) {}
	ResourcePtr( const HashedString handle ) : m_handle( handle ) {}
	ResourcePtr( const ResourcePtr<T>& other )
	{
		this->m_handle = other.m_handle;
	}

	T* operator->() const
	{
		T* resource = Get();
		COM_ASSERT( resource, "[ScopedResource]: resource '%s' is not loaded\n", m_handle.handle.GetStringForHash() );
		return resource;
	}

	T& operator*() const
	{
		T* resource = Get();
		COM_ASSERT( resource, "[ScopedResource]: resource '%s' is not loaded\n", m_handle.handle.GetStringForHash() );
		return *resource;
	}

	explicit operator bool() const
	{
		if ( m_handle.IsInvalid() )
		{
			return false;
		}

		return Get() != nullptr;
	}

	inline bool ResourceExists()
	{
		return bool();
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
