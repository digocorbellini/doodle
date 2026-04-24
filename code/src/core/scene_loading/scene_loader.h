////////////////////////////////////////////////////////////
//
// The SceneLoader handles the loading of scene files into
// the game, the parsing of said scene files, and then
// the passing of the handling of loading assets and 
// entities to their appropriate systems. 
//
////////////////////////////////////////////////////////////

#pragma once
#include "core/components.h"
#include <nlohmann/json.hpp>

// TODO: have to handle the file opening of a scene file with a given name
// (the starting scene must be set in a config file somewhere).
// Then the scene must be split into textures and entities to be 
// parsed and loaded by the resource manager and ECS respectively.

// TODO: Have to figure out how ECS and the resource manager will actually perform
// the JSON parsing to see what exactly we have to send them.


class SceneLoader
{
private:
//	// TODO: place the entity map here and getters
	bool testBool = false;

public:

	bool LoadScene( const char* sceneRef );
	
	// TODO: need some way to load scenes async to avoid blocking game loop to load scene
	//bool LoadSceneAsync( const char* sceneRef );

#define COMPONENT(X) friend void ComponentParser_##X##Parser( const nlohmann::json* jsonEntity, X * componentList, SceneLoader* sceneLoader );
	COMPONENT_LIST
#undef COMPONENT
};