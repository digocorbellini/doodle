////////////////////////////////////////////////////////////
//
// TODO: define this
//
////////////////////////////////////////////////////////////

#pragma once
#include "core/components.h"
#include "core/ecs.h"
#include <nlohmann/json.hpp>

class SceneLoader;

// TODO: potentially make scene loader const everywhere here?

typedef void ( *ComponentParser_ParserFunctPtr ) ( const EntityID entityID, const nlohmann::json& jsonComponentValues, SceneLoader* sceneLoader );

ComponentParser_ParserFunctPtr ComponentParser_GetParserForType( ComponentType componentType );

// wrapper functions around the functions that the user will define which perform the getting of the right component from ECS
#define COMPONENT(X) \
void ComponentParser_##X##ParsingWrapper( const EntityID entityID, const nlohmann::json& jsonComponentValues, SceneLoader* sceneLoader ); \
COMPONENT_LIST
#undef COMPONENT 

// declare the functions which the user will later define for parsing components 
#define COMPONENT(X) void ComponentParser_##X##Parser( const nlohmann::json& jsonComponentValues, X * entityComponent, SceneLoader* sceneLoader );
COMPONENT_LIST
#undef COMPONENT
