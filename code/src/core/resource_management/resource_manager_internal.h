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
}
