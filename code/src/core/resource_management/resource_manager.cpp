#include "resource_manager.h"

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