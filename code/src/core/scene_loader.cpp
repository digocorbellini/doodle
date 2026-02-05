#include "scene_loader.h"
#include <iostream>
#include <fstream>
#include <string>

// TODO: just for testing
#include "common/hashing/hashed_string.h"
#include <inttypes.h>
#include "common/lib/com_print.h"

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

	// TODO: remove this test
	HashedString testHash( "test String!" );
	printf( "Testing: %s = %" PRIu64 "\n", testHash.GetStringForHash(), testHash.GetHash() );

	HashedString testHash2( "test String 2!" );
	printf( "Testing: %s = %" PRIu64 "\n", testHash2.GetStringForHash(), testHash2.GetHash() );

	HashedString testHash3( "test String 3!" );
	printf( "Testing: %s = %" PRIu64 "\n", testHash3.GetStringForHash(), testHash3.GetHash() );

	HashedString testHash4( "test String 3!" );
	printf( "Testing: %s = %" PRIu64 "\n", testHash4.GetStringForHash(), testHash4.GetHash() );


	Com_Printf( "Test int: %i\n", 10 );
	Com_PrintfVerbose( "SceneLoader", "Test num: [%i]", -43 );
	Com_PrintfErrorVerbose( "SceneLoader", "Test num: [%s]", "test");
	Com_PrintfWarningVerbose( "SceneLoader", "Test num: [%.2f]", 12.432152);


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

	// TODO: for now parse all resources as strings and hash them at runtime. Later
	// can have them pre-hashed in the scene file and then load in the
	// hash to string translations separately 

	return false;
}


SceneLoader::~SceneLoader()
{

}