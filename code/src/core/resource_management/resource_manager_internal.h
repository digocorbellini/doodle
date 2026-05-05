////////////////////////////////////////////////////////////
//
// Holds declarations for logic that must only be used
// for the resource manager logic.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/types.h"
#include "resource_types.h"

namespace resource_manager_impl
{
	template <typename T>
	struct ResourceHandle
	{
		HashedString handle = INVALID_HASHED_STRING;

		inline bool IsInvalid() const
		{
			return handle == INVALID_HASHED_STRING;
		}
	};

	typedef void ( *ResourceDeletorFunc )( void* );

	struct CachedResource
	{
		void* resourcePtr = nullptr;
		ResourceType type = ResourceType::Invalid;
		HashedString hash = INVALID_HASHED_STRING;
		ResourceDeletorFunc deletorFunc = nullptr;

		void Reset()
		{
			type = ResourceType::Invalid;
			hash = INVALID_HASHED_STRING;
			if ( deletorFunc )
			{
				deletorFunc( resourcePtr );
			}
			resourcePtr = nullptr;
		}

		bool IsEmpty()
		{
			return resourcePtr == nullptr;
		}
	};

	const CachedResource* GetCachedResource( HashedString resourceHashName );

	/// <summary>
	/// Get a pointer to the given resource if it has been loaded.
	/// </summary>
	/// <typeparam name="T">The type of the resource. Will determine
	/// the type of the pointer</typeparam>
	/// <param name="resourceHandle">The handle which is used to identify the resource</param>
	/// <returns>A pointer of type T to the given resource if it exists and has been loaded, 
	/// otherwise returns nullptr.</returns>
	template<typename T>
	T* ResourceManager_GetResource( resource_manager_impl::ResourceHandle<T> resourceHandle )
	{
		// defined function in header due to it being templated
		if ( resourceHandle.IsInvalid() )
		{
			return nullptr;
		}

		const resource_manager_impl::CachedResource* resource = resource_manager_impl::GetCachedResource( resourceHandle.handle.GetHash() );
		if ( !resource )
		{
			return nullptr;
		}

		COM_ASSERT( ResourceTypes_ResourceTypeForType<T>() == resource->type, "%s - resource type requested (%s) is not the same as the actual type of the resource (%s).\n", __FUNCTION__,
					ResourceTypes_GetResourceTypeString( ResourceTypes_ResourceTypeForType<T>() ), ResourceTypes_GetResourceTypeString( resource->type ) );

		return static_cast<T*>( resource->resourcePtr );
	}
}
