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


static constexpr size_t MAX_SCENE_PATH_LEN = 512;

class SceneLoader
{
private:
	class EntityManagementToken m_EntityManagementToken;

	EntityID GetParsingEntityID( const nlohmann::json::string_t& entityName );

public:
	SceneLoader() = default;

	/// <summary>
	/// Unload the currently active scene and load the given scene if it exists.
	/// </summary>
	/// <param name="sceneRef">The string referece to the scene to be loaded</param>
	/// <returns>True if the scene is loaded successfully. False otherwise</returns>
	bool LoadScene( const char* sceneRef );
	
	// TODO: need some way to load scenes async to avoid blocking game loop to load scene
	//bool LoadSceneAsync( const char* sceneRef );

#define COMPONENT(X) friend void ComponentParser_##X##Parser( const nlohmann::json& jsonComponentValues, X * entityComponent, SceneLoader* sceneLoader );
	COMPONENT_LIST
#undef COMPONENT
};