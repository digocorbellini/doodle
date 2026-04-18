#include "common/types.h"
#include "resource_types.h"
#include <SFML/Graphics.hpp>

using namespace sf;

static const char* s_resourceTypeStrings[] =
{
	"invalid",
#define RESOURCE(X) #X,
	ALL_RESOURCE_TYPES
#undef RESOURCE
};
static_assert( GetUndelyingEnumVal( ResourceType::Count ) == ARRAY_SIZE( s_resourceTypeStrings ),
			   "ResourceTypeStrings and ResourceType length missmatch" );


const char* ResourceTypes_GetResourceTypeString( ResourceType resourceType )
{
	COM_ASSERT( resourceType < ResourceType::Count, "%s - Invalid resource type [%i]\n",
				__FUNCTION__,
				GetUndelyingEnumVal( resourceType ) );

	return s_resourceTypeStrings[GetUndelyingEnumVal( resourceType )];
}


ResourceType ResourceTypes_GetResourceTypeForString( const char* resourceTypeStr )
{
	for ( size_t i = 0; i < GetUndelyingEnumVal( ResourceType::Count ); ++i )
	{
		if ( strcmp( resourceTypeStr, s_resourceTypeStrings[i] ) == 0 )
		{
			return static_cast<ResourceType>( i );
		}
	}

	return ResourceType::Invalid;
}


template<typename T>
ResourceType ResourceTypes_ResourceTypeForType()
{
	COM_ASSERT( false, "%s - Resource type '%s' is not supported. Make sure it is added to resource_types.h/cpp\n", 
				__FUNCTION__,  
				typeid(T).name);
	return ResourceType::Invalid;
}


#define RESOURCE(X) template<>ResourceType ResourceTypes_ResourceTypeForType<X>() { return ResourceType::X; }
ALL_RESOURCE_TYPES
#undef RESOURCE
