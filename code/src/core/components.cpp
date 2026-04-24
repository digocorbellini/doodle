#include "components.h"
#include "common/lib/com_array.h"

// TODO: this will eventually be used by the scene parsing logic to know
// what components to add to an entity
static const char* s_componentTypeStrings[] =
{
	// TODO: figure out how to obscure these strings... maybe do it by having static versions of each instead of 
	// current obscure string implementation which is temporary
#define COMPONENT(X) #X,
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentTypeStrings ),
			   "s_componentTypeStrings and ComponentType length missmatch" );

// used to map component type enum to component type
static std::type_index s_componentTypeMap[] =
{
#define COMPONENT(X) typeid(X),
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentTypeMap ),
			   "s_componentTypeMap and ComponentType length missmatch" );

// ==================
// Public Methods
// ==================

bool Components_IsComponentValid( const ComponentType componentType )
{
	return componentType != ComponentType::Invalid && componentType != ComponentType::Count;
}


const char* Components_GetComponentTypeString( const ComponentType componentType )
{
	if ( !Components_IsComponentValid( componentType ) )
	{
		return nullptr;
	}

	return s_componentTypeStrings[GetUndelyingEnumVal( componentType )];
}


ComponentType Components_GetComponentTypeForString( const char* componentStr )
{
	for ( size_t i = 0; i < GetUndelyingEnumVal( ComponentType::Count ); ++i )
	{
		if ( strcmp( componentStr, s_componentTypeStrings[i] ) == 0 )
		{
			return static_cast<ComponentType>( i );
		}
	}

	return ComponentType::Invalid;
}


std::type_index Components_GetComponentTypeIndex( const ComponentType componentType )
{
	if ( !Components_IsComponentValid( componentType ) )
	{
		return typeid(nullptr);
	}

	return s_componentTypeMap[GetUndelyingEnumVal( componentType )];
}


ComponentsMask::ComponentsMask( const ComponentsMask& other )
{
	mask = other.mask;
}


ComponentsMask::ComponentsMask( std::initializer_list<ComponentType> compList )
{
	AddComponents( compList );
}


bool ComponentsMask::AddComponent( const ComponentType component )
{
	if ( !Components_IsComponentValid( component ) )
	{
		return false;
	}

	mask.set(static_cast<size_t>( component ));

	return true;
}


int ComponentsMask::AddComponents( std::initializer_list<ComponentType> compList )
{
	int numAdded = 0;
	for( auto it = compList.begin(); it < compList.end(); ++it )
	{
		if( !AddComponent( *it ) )
		{
			return numAdded;
		}
		numAdded++;
	}

	return numAdded;
}


bool ComponentsMask::RemoveComponent( const ComponentType component )
{
	if( !Components_IsComponentValid( component ) )
	{
		return false;
	}

	mask.reset( static_cast< size_t >( component ) );

	return true;
}


int ComponentsMask::RemoveComponents( std::initializer_list<ComponentType> compList )
{
	int numRemoved = 0;
	for( auto it = compList.begin(); it < compList.end(); ++it )
	{
		if( !RemoveComponent( *it ) )
		{
			return numRemoved;
		}
		numRemoved++;
	}

	return numRemoved;
}