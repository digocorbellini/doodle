#include "scene_loader.h"

SceneLoader::SceneLoader()
{

}


SceneLoader& SceneLoader::GetInstance()
{
	static SceneLoader instance;

	return instance;
}


// TODO: maybe change this to be able to load a file async? b/c this will replace currently active scene.
// Need to somehow stop all game logic while this is happening b/c will load assets + modify entities.
// ECS must handle this somehow
bool LoadSceneFile( const char* sceneFileName )
{
	return false;
}


SceneLoader::~SceneLoader()
{

}