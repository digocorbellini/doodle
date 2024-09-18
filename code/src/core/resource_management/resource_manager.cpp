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

}


ResourceManager::~ResourceManager()
{

}