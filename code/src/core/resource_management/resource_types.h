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

/// <summary>
/// Get the string for the given resource type
/// </summary>
/// <param name="resourceType">The resource type to get a string for</param>
/// <returns>The string equivalent of the resource type. If none exists, returns nullptr</returns>
const char* ResourceTypes_GetResourceTypeString( ResourceType resourceType );

/// <summary>
/// Get the resource type for the given string.
/// </summary>
/// <param name="resourceTypeStr">The string equivalent of the resource type</param>
/// <returns>The ResourceType enum value which matches the given string. Otherwise ResourceType::Invalid</returns>
ResourceType ResourceTypes_GetResourceTypeForString( const char* resourceTypeStr);

// Each resouce type should define a type overload for this function in resource_types.cpp
template<typename T>
ResourceType ResourceTypes_ResourceTypeForType();
