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
#include <SFML/Graphics.hpp>

#define ALL_RESOURCE_TYPES \
RESOURCE(Texture, sf::Texture)

enum class ResourceType : int
{
#define RESOURCE(X, Y) X,
	ALL_RESOURCE_TYPES
#undef RESOURCE

	Count,
	Invalid = Count,
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
ResourceType ResourceTypes_GetResourceTypeForString( const char* resourceTypeStr );

// Each resouce type should define a type overload for this function in resource_types.cpp
template<typename T>
ResourceType ResourceTypes_ResourceTypeForType();

// used for compile time checking for valid resource type
template<typename T>
struct IsValidResourceType : std::false_type {};

#define RESOURCE(X, Y) template<> struct IsValidResourceType<Y> : std::true_type {};
ALL_RESOURCE_TYPES
#undef RESOURCE

template<typename T>
concept ValidResource = IsValidResourceType<T>::value;