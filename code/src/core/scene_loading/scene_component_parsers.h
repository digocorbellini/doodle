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

typedef void ( *ComponentParser_ParserFunctPtr ) ( const nlohmann::json* jsonEntity, SceneLoader* sceneLoader );

ComponentParser_ParserFunctPtr ComponentParser_GetParserForType( ComponentType componentType );

#define COMPONENT(X) void ComponentParser_##X##Parser( const nlohmann::json* jsonEntity, X * componentList, SceneLoader* sceneLoader );
COMPONENT_LIST
#undef COMPONENT

// TODO: create wrapper functions around the functions that the user will define which perform the getting of the right component type list from ECS
#define COMPONENT(X) \
inline void ComponentParser_##X##ParsingWrapper( const nlohmann::json* jsonEntity, SceneLoader* sceneLoader ) \
{ \
	X* componentList = ECS_GetComponentList<X>( ComponentType::X ); \
	ComponentParser_##X##Parser( jsonEntity, componentList, sceneLoader ); \
}
COMPONENT_LIST
#undef COMPONENT 

