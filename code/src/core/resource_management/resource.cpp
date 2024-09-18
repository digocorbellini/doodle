#include "resources.h"
#include "malloc.h"
//#include "string.h"
#include <string>
#include <cassert>

Resource::Resource( const char* pathStr )
{
	dontFreeOnLoad = false;

	this->pathStr = static_cast< char* >( malloc( sizeof( pathStr ) ) );
	// TODO: maybe don't assert here. Instead send a warning? or will have to have
	// null ptr checks everytime pathStr is checked? Or maybe that is ok since we won't be using
	// it internally anyway and we just want to utilize it outside of this class
	assert( this->pathStr );
	strncpy_s( this->pathStr, sizeof( pathStr ), pathStr, sizeof(pathStr) + 1 );
}


const char* Resource::GetPath()
{
	return pathStr;
}


bool Resource::GetDontFreeOnLoad()
{
	return dontFreeOnLoad;
}


void Resource::SetDontFreeOnLoad( bool dontFreeOnLoad )
{
	this->dontFreeOnLoad = dontFreeOnLoad;
}


Resource::~Resource()
{
	if( pathStr )
	{
		free( pathStr );
	}
}