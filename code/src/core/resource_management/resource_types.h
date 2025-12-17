////////////////////////////////////////////////////////////
//
// This file includes all resouce types in the 
// ResourceType enum as well as helper functions for 
// dealing with resource types
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/lib/com_assert.h"
#include "common/lib/com_array.h"
#include "common/types.h"


enum class ResourceType : int
{
	INVALID,
	TEXTURE,

	COUNT
};

const char* GetResourceTypeString( ResourceType resourceType );

// Each resouce type should define a type overload for this function in resource_types.cpp
template<typename T>
ResourceType ResourceTypeForType();
