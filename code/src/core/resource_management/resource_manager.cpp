#include "resource_manager.h"

// TODO: potentially create a static cache of resources which can be added to and removed from
// as scenes are loaded and unloaded
// Can be structured as so:
// - Create a struct which contains: HashedString (used for hashing), pointer to resource
// - Create a static array of these structs and use this as a linear probing map which you can index
//   into using HashedStrings
// - Dynamically allocate resources using heap and assign those pointers to the struct
//		- Might have to store the resources as void pointers in the struct and then cast them
//		  to the desired type when they are requested using templated resources 

ResourceManager::ResourceManager()
{

}


ResourceManager& ResourceManager::GetInstance()
{
	static ResourceManager instance;
	
	return instance;
}

void ResourceManager::LoadSceneAssets( const char* sceneFileName )
{
	// TODO: sceneFileName is a JSON file which contains the defintion of all of the assets to load?

}


ResourceManager::~ResourceManager()
{

}