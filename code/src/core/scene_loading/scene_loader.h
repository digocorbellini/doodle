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


class SceneLoader
{
private:
	EntityID GetParsingEntityID( const nlohmann::json::string_t& entityName );

public:

	bool LoadScene( const char* sceneRef );
	
	// TODO: need some way to load scenes async to avoid blocking game loop to load scene
	//bool LoadSceneAsync( const char* sceneRef );

#define COMPONENT(X) friend void ComponentParser_##X##Parser( const nlohmann::json& jsonComponentValues, X * entityComponent, SceneLoader* sceneLoader );
	COMPONENT_LIST
#undef COMPONENT
};