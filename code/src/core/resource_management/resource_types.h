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

#define ALL_RESOURCE_TYPES \
RESOURCE(Texture)

enum class ResourceType : int
{
	Invalid,

#define RESOURCE(X) X,
	ALL_RESOURCE_TYPES
#undef RESOURCE

	Count
};

const char* ResourceTypes_GetResourceTypeString( ResourceType resourceType );

// Each resouce type should define a type overload for this function in resource_types.cpp
template<typename T>
ResourceType ResourceTypes_ResourceTypeForType();
