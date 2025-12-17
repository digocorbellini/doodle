#include "components.h"
#include "common/lib/com_array.h"

static const char* s_componentTypeStrings[] =
{
	"EntityTransform2D",
	"PhysicsBody2D",
	"SpriteRenderer2D",
	"PlayerController2D",
	"Camera2D"
};
static_assert( GetUndelyingEnumVal( ComponentType::Count ) == ARRAY_SIZE( s_componentTypeStrings ),
			   "s_componentTypeStrings and ComponentType length missmatch" );

// ==================
// Private Helpers
// ==================

static bool IsComponentValid( const ComponentType component )
{
	return component != ComponentType::Invalid && component != ComponentType::Count;
}


// ==================
// Public Methods
// ==================

const char* GetComponentTypeString( ComponentType componentType )
{
	if ( !IsComponentValid( componentType ) )
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
	if ( !IsComponentValid( component ) );
	{
		return false;
	}

	mask.set(static_cast<size_t>( component ));

	return true;
}


// TODO: maybe only have this and then make the singular "add component" a private helper?
// That way it is less confusing for users as to which one to use?
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
	if( !IsComponentValid( component ) )
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