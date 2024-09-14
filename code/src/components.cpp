#include "components.h"

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
	if( component == ComponentType::Invalid )
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
	if( component == ComponentType::Invalid )
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