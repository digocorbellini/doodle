////////////////////////////////////////////////////////////
//
// This file is meant to contain all declarations for
// all of the resource types the resource manager will
// support. CPP files must be made for each resource type
// definition.
//
////////////////////////////////////////////////////////////

#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

// resource.cpp
class Resource
{
protected:
	char* pathStr;
	bool dontFreeOnLoad;

public:
	Resource() = default;
	Resource( const char* pathStr );

	/// <summary>
	/// Get the path for this resource. This is both a unique
	/// identifier for this resource, and a relative path
	/// used by the resource manager for loading this resource.
	/// 
	/// TODO: ensure this part is true if we switch to custom resource type
	/// The path is relative to the assets directory defined
	/// in the project configs.
	/// </summary>
	/// <returns>Get the path for this resource</returns>
	const char* GetPath();

	/// <summary>
	/// See if this resource should be freed upon
	/// loading a new scene that does not contain this resource.
	/// </summary>
	/// <returns>False if this resource will be freed upon the loading
	/// of a scene that does not contain this resource. True if this
	/// resource will remain in memory until manually freed</returns>
	bool GetDontFreeOnLoad();

	/// <summary>
	/// Set whether this resource should be freed upon
	/// loading a new scene
	/// </summary>
	/// <param name="dontFreeOnLoad">If false, this resource
	/// will be freed when a new scene is loaded that does not
	/// contain this resource. If true, this resource will remain
	/// in memory until manually freed</param>
	void SetDontFreeOnLoad( bool dontFreeOnLoad );

	~Resource();
};


///////////////////////////////////
// Resource Type Declarations
///////////////////////////////////

// resource_texture.cpp
class TextureResource : public Resource
{
private:
	class Texture* texture;

public:
	TextureResource( const char* pathStr );

	// TODO: determine if making the returned texture const + having modifiers in this class is better
	/// <summary>
	/// Get the texture associated with this texture resource.
	/// </summary>
	/// <returns>A pointer to the texture for this resource</returns>
	class Texture* GetTexture();

	~TextureResource();
};