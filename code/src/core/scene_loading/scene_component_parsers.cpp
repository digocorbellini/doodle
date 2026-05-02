#include "common/lib/com_print.h"
#include "core/resource_management/resource_manager.h"
#include "scene_component_parsers.h"
#include "scene_loader.h"

using namespace sf;

using json = nlohmann::json;

// ==================================
// Parsing Helper Macros
// ==================================

#define VALUE_FIELD OBFUSCATED_STRING( "val" )


// parsing logic for 1 to 1 mappings of simple data types
#define PARSE_SIMPLE_FIELD_MAPPED( jsonFieldName, structDestName ) \
{\
	const char* fieldNameStr = OBFUSCATED_STRING( #jsonFieldName );\
	if ( jsonComponentValues.contains( fieldNameStr ) )\
	{\
		const json jsonObj = jsonComponentValues[fieldNameStr];\
		const decltype( entityComponent->structDestName ) value = jsonObj[VALUE_FIELD.ToStdString()];\
		entityComponent->structDestName = value;\
	}\
}
#define PARSE_SIMPLE_FIELD( fieldName ) PARSE_SIMPLE_FIELD_MAPPED( fieldName, fieldName )


#define PARSE_VECTOR2F_FIELD_MAPPED( jsonFieldName, structDestName ) \
{\
	const char* fieldNameStr = OBFUSCATED_STRING( #jsonFieldName ); \
	if ( jsonComponentValues.contains( fieldNameStr ) )\
	{\
		const json jsonObj = jsonComponentValues[fieldNameStr]; \
		const json jsonValueArr = jsonObj[VALUE_FIELD.ToStdString()]; \
		if ( jsonValueArr.size() == 2 )\
		{\
			entityComponent->structDestName.x = jsonValueArr[0];\
			entityComponent->structDestName.y = jsonValueArr[1];\
		}\
	}\
}
#define PARSE_VECTOR2F_FIELD( fieldName ) PARSE_VECTOR2F_FIELD_MAPPED( fieldName, fieldName )


#define PARSE_ENTITY_REF_FIELD_MAPPED( jsonFieldName, structDestName ) \
{\
	const char* fieldNameStr = OBFUSCATED_STRING( #jsonFieldName ); \
	if ( jsonComponentValues.contains( fieldNameStr ) )\
	{\
		const json jsonObj = jsonComponentValues[fieldNameStr]; \
		const json::string_t entityName = jsonObj[VALUE_FIELD.ToStdString()]; \
		if ( entityName.length() > 0 )\
		{\
			EntityID entityID = sceneLoader->GetParsingEntityID( entityName ); \
			entityComponent->structDestName = entityID; \
		}\
	}\
}
#define PARSE_ENTITY_REF_FIELD( fieldName ) PARSE_ENTITY_REF_FIELD_MAPPED( fieldName, fieldName )

// ==================================
// Parsing Wrapper Functions
// ==================================

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
// void ComponentParser_<Component Name>Parser( const json& jsonComponentValues, <Component Type>* entityComponent, SceneLoader* sceneLoader )
// Make sure your parser function handles missing fields 
// ==================================

void ComponentParser_EntityTransform2DParser( const json& jsonComponentValues, EntityTransform2D* entityComponent, SceneLoader* sceneLoader )
{
	PARSE_VECTOR2F_FIELD( position );
	PARSE_VECTOR2F_FIELD( scale );
}


void ComponentParser_PhysicsBody2DParser( const json& jsonComponentValues, PhysicsBody2D* entityComponent, SceneLoader* sceneLoader )
{
	PARSE_VECTOR2F_FIELD( velocity );
	PARSE_SIMPLE_FIELD( gravity );
}

void ComponentParser_SpriteRenderer2DParser( const json& jsonComponentValues, SpriteRenderer2D* entityComponent, SceneLoader* sceneLoader )
{
	// TODO: see if I can make a resource parsing helper
	const char* spriteField = OBFUSCATED_STRING( "sprite" );
	if ( jsonComponentValues.contains( spriteField ) )
	{
		const json jsonSprite = jsonComponentValues[spriteField];
		const json::string_t spriteRef = jsonSprite[VALUE_FIELD.ToStdString()];
		if ( spriteRef.length() > 0 )
		{
			entityComponent->sprite.SetTexture( ResourceHandle<Texture>( HashedString( spriteRef.c_str(), spriteRef.length() ) ) );
		}
	}

	PARSE_SIMPLE_FIELD( renderingOrder );
	PARSE_SIMPLE_FIELD( hasCenteredOrigin );
	PARSE_SIMPLE_FIELD( isXFlipped );
	PARSE_SIMPLE_FIELD( isYFlipped );
}


void ComponentParser_PlayerController2DParser( const json& jsonComponentValues, PlayerController2D* entityComponent, SceneLoader* sceneLoader )
{
	PARSE_SIMPLE_FIELD( moveSpeed );
	PARSE_SIMPLE_FIELD( jumpSpeed );
}


void ComponentParser_Camera2DParser( const json& jsonComponentValues, Camera2D* entityComponent, SceneLoader* sceneLoader )
{
	PARSE_ENTITY_REF_FIELD( targetEntity );
	PARSE_VECTOR2F_FIELD( viewOffsetFromPos );
	// TODO: figure out if have to parse "view" 
	PARSE_SIMPLE_FIELD( isMainCam );
}
