#include "scene_loader.h"
#include <iostream>
#include <fstream>
#include <string>

// TODO: just for testing
#include "common/hashing/HashedString.h"
#include <inttypes.h>

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

	// TODO: remove this test
	HashedString testHash( "test String!" );
	printf( "Testing: %s = %" PRIu64 "\n", testHash.GetDebugString(), testHash.GetHash());


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