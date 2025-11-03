#include "scene_loader.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

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
bool SceneLoader::LoadSceneFile( const char* sceneFileName )
{
	// TODO: maybe take in a JSON?

	ifstream inputFile("../game/scenes/test_scene.json");

	if ( !inputFile.is_open() )
	{
		printf("%s - Failed to open file\n", __FUNCTION__);
		return false;
	}

	string fileContent = "";
	string currLine = "";
	while ( getline( inputFile, currLine ) )
	{
		fileContent += currLine;
		fileContent.push_back( '\n' );
	}
	printf( "File content: %s\n", fileContent.c_str() );

	json j_complete = json::parse( fileContent );
	std::cout << std::setw( 4 ) << j_complete << "\n\n";

	return false;
}


SceneLoader::~SceneLoader()
{

}