#include "ecs.h"

// List of all entities in the game
static Entity entities[MAX_ENTITIES];

static Components components;
static Entity_t numEntities;
//
static bool isDebugMode;
//
//// TODO: maybe handle window logic in a separate file? Maybe can't
//// because it has to be handled by the game loop? Could be its own set
//// of static functions in a window.cpp file?
//static Color backgroundColor;
//static Vector2i windowSize;

//TODO: figure out if all json parsing will happen here if if I need too many helpers so 
// another file would be better for defining the static functions + handling all scene loading
// logistic (like keeping track of active scene, async loading, etc).
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
using json = nlohmann::json;
bool ECS_LoadScene( const char* sceneFileName )
{
	//const char* testFile = ".\\..\\assets\\test_scene.json";
	// TODO: have to get executable path when creating relative pathing
	ifstream inputFile( "../game/scenes/test_scene.json" );

	if( !inputFile.is_open() )
	{
		printf( "Failed to open file\n" );
		return false;
	}

	string fileContent = "";
	string currLine = "";
	while( getline( inputFile, currLine ) )
	{
		fileContent += currLine;
		fileContent.push_back( '\n' );
	}
	printf( "File content: %s\n", fileContent.c_str() );
	return true;
}