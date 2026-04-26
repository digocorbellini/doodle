#include "common/lib/com_print.h"
#include "scene_component_parsers.h"
#include "scene_loader.h"

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
// void ComponentParser_<Component Name>Parser( const nlohmann::json jsonEntity, <Component Type>* componentList, SceneLoader* sceneLoader )
// ==================================

void ComponentParser_EntityTransform2DParser( const nlohmann::json* jsonEntity, EntityTransform2D* componentList, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
	
	Com_Printf( "\tTest bool: %i\n", sceneLoader->testBool );
}


void ComponentParser_PhysicsBody2DParser( const nlohmann::json* jsonEntity, PhysicsBody2D* componentList, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_SpriteRenderer2DParser( const nlohmann::json* jsonEntity, SpriteRenderer2D* componentList, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_PlayerController2DParser( const nlohmann::json* jsonEntity, PlayerController2D* componentList, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}


void ComponentParser_Camera2DParser( const nlohmann::json* jsonEntity, Camera2D* componentList, SceneLoader* sceneLoader )
{
	Com_Printf( "%s - successfully called\n", __FUNCTION__ );
}
