#include "resource_types.h"
#include <SFML/Graphics.hpp>

using namespace sf;

static const char* s_resourceTypeStrings[] =
{
	"invalid",
	"texture"
};
static_assert( GetUndelyingEnumVal( ResourceType::Count ) == ARRAY_SIZE( s_resourceTypeStrings ),
			   "ResourceTypeStrings and ResourceType length missmatch" );


const char* GetResourceTypeString( ResourceType resourceType )
{
	COM_ASSERT( resourceType < ResourceType::Count, "%s - Invalid resource type [%i]\n",
				__FUNCTION__,
				GetUndelyingEnumVal( resourceType ) );

	return s_resourceTypeStrings[GetUndelyingEnumVal( resourceType )];
}


template<typename T>
ResourceType ResourceTypeForType()
{
	COM_ASSERT( false, "%s - Resource type '%s' is not supported. Make sure it is added to resource_types.h/cpp\n", 
				__FUNCTION__,  
				typeid(T).name);
	return ResourceType::Invalid;
}


template<>
ResourceType ResourceTypeForType<Texture>()
{
	return ResourceType::Texture;
}
