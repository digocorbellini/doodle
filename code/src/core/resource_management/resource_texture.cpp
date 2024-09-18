#include "resources.h"

TextureResource::TextureResource( const char* pathStr ) 
	: Resource( pathStr )
{
	// TODO: have to figure out how to convert 
	// the pathStr to an actual path that contains
	// the asset. Have to get it based on a config?
	// Maybe I grab it from the resource manager singleton?
	texture = new Texture();
}


sf::Texture* TextureResource::GetTexture()
{
	return texture;
}


TextureResource::~TextureResource()
{
	if( texture )
	{
		delete( texture );
	}
}