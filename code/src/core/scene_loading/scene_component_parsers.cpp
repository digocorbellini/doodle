#include "common/lib/com_print.h"
#include "core/resource_management/resource_manager.h"
#include "scene_component_parsers.h"
#include "scene_loader.h"

using json = nlohmann::json;


#define COMPONENT(X) \
void ComponentParser_##X##ParsingWrapper( const EntityID entityID, const nlohmann::json& jsonComponentValues, SceneLoader* sceneLoader ) \
{ \
	if ( !ECS_IsValidEntityID( entityID ) )\
	{\
		return;\
	}\
\
	X* componentList = ECS_GetComponentList<X>( ComponentType::X );\
	if ( !componentList )\
	{\
		return;\
	}\
\
	X* entityComponent = &componentList[entityID];\
	ComponentParser_##X##Parser( jsonComponentValues, entityComponent, sceneLoader );\
}
COMPONENT_LIST
#undef COMPONENT 


static ComponentParser_ParserFunctPtr s_ParserFunctsList[] =
{
#define COMPONENT(X) ComponentParser_##X##ParsingWrapper,
	COMPONENT_LIST
#undef COMPONENT
};
static_assert( ARRAY_SIZE( s_ParserFunctsList ) == GetUndelyingEnumVal(ComponentType::Count) );


// ==================================
// Public Functions
// ==================================

ComponentParser_ParserFunctPtr ComponentParser_GetParserForType( ComponentType componentType )
{
	if ( !Components_IsComponentValid( componentType ) )
	{
		return nullptr;
	}

	return s_ParserFunctsList[GetUndelyingEnumVal( componentType )];
}


// ==================================
// Component Parsers
// 
// Defined using the following naming format:
// void ComponentParser_<Component Name>Parser( const nlohmann::json jsonComponentValues, <Component Type>* entityComponent, SceneLoader* sceneLoader )
// Make sure your parser function handles missing fields 
// ==================================

void ComponentParser_EntityTransform2DParser( const json& jsonComponentValues, EntityTransform2D* entityComponent, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );	
}


void ComponentParser_PhysicsBody2DParser( const json& jsonComponentValues, PhysicsBody2D* entityComponent, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_SpriteRenderer2DParser( const json& jsonComponentValues, SpriteRenderer2D* entityComponent, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_PlayerController2DParser( const json& jsonComponentValues, PlayerController2D* entityComponent, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_Camera2DParser( const json& jsonComponentValues, Camera2D* entityComponent, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}
