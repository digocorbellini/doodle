////////////////////////////////////////////////////////////
//
// The resource manager is meant to handle all memory 
// management related to loading and freeing game assets
// such as images, sounds, etc. 
// 
// Resources are loaded and unloaded by scene.
// 
// The supported resource types are all defined in
// resources.h
//
////////////////////////////////////////////////////////////

#pragma once

// TODO: replace this with a config file somehow
#define RELATIVE_ASSETS_PATH "../assets"
#define SCENES_DIR_NAME "scenes"

// TODO: have to figure out if this is good as a class or if I can do what
// ecs files are doing and just keep this as functions. That way it will be
// easier for systems to work with? Or maybe not? Have to think about workflow
class ResourceManager
{
private:
	// This class is a singleton
	ResourceManager();

public:

	static ResourceManager& GetInstance();

	// TODO: need to determine someway to determine where to load from.
	// Also need some way to unload and reload assets.
	// Idk if we want to be fully tied to "scenes" at all times
	// because there might be some assets we want loaded at all times 
	// (like loading scenes maybe? or audio that is used between scenes)
	//		- maybe have the same thing as unity where they "don't destroy on load"
	//			- so it persists between scenes?
	// Maybe instead of having a reference to a scene file? Why not only load it once
	// and then pass it here and also in the entity parsing.
	void LoadSceneAssets( const char* sceneFileName );

	~ResourceManager();

private:

};