#include "components.h"
#include "common/lib/com_array.h"

// TODO: this will eventually be used by the scene parsing logic to know
// what components to add to an entity
static const char* s_componentTypeStrings[] =
{
#define COMPONENT(X) #X,
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentTypeStrings ),
			   "s_componentTypeStrings and ComponentType length missmatch" );

// ==================
// Public Methods
// ==================

bool Components_IsComponentValid( const ComponentType componentType )
{
	return componentType != ComponentType::Invalid && componentType != ComponentType::Count;
}

const char* Components_GetComponentTypeString( ComponentType componentType )
{
	if ( !Components_IsComponentValid( componentType ) )
	{
		return nullptr;
	}

	return s_componentTypeStrings[GetUndelyingEnumVal( componentType )];
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