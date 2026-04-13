////////////////////////////////////////////////////////////
//
// The SceneLoader handles the loading of scene files into
// the game, the parsing of said scene files, and then
// the passing of the handling of loading assets and 
// entities to their appropriate systems. 
//
////////////////////////////////////////////////////////////

#pragma once
#include <nlohmann/json.hpp>


// TODO: have to handle the file opening of a scene file with a given name
// (the starting scene must be set in a config file somewhere).
// Then the scene must be split into textures and entities to be 
// parsed and loaded by the resource manager and ECS respectively.

// TODO: Have to figure out how ECS and the resource manager will actually perform
// the JSON parsing to see what exactly we have to send them.

// TODO: replace this with a config loaded on start up
#define SCENE_FILE_PATH ""

class SceneLoader
{
private:
	// This class is a singleton
	SceneLoader();

public:

	static SceneLoader& GetInstance();

	/// <summary>
	/// Load the scene file and kickoff the necessary entity and 
	/// </summary>
	/// <param name="sceneFileName"></param>
	/// <returns></returns>
	bool LoadSceneFile( const char* sceneFileName );

	~SceneLoader();

private:

};