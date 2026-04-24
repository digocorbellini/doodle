#include "common/lib/com_assert.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include "common/hashing/hash.h"
#include "core/ecs.h"
#include "scene_component_parsers.h"
#include "scene_loader.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;
using namespace std;

#define SCENE_LOADER_STR OBFUSCATED_STRING( "SceneLoader" )

#define SCENES_DIR_PATH OBFUSCATED_STRING_CONCAT( GAME_DIR_PATH, "scenes/" )
#define ENTITY_TEMPLATES_DIR_PATH OBFUSCATED_STRING_CONCAT( GAME_DIR_PATH, "entity_templates/" )

#define RESOURCES_ARRAY_FIELD OBFUSCATED_STRING( "resources" )
#define RESOURCE_NAME_FIELD OBFUSCATED_STRING( "resourceName" )
#define RESOURCE_TYPE_FIELD OBFUSCATED_STRING( "resourceType" )

#define ENTITIES_ARRAY_FIELD OBFUSCATED_STRING( "entities" )
#define ENTITY_NAME_FIELD OBFUSCATED_STRING( "entityName" )
#define ENTITY_TEMPLATE_FIELD OBFUSCATED_STRING( "template" )

#define COMPONENTS_ARRAY_FIELD OBFUSCATED_STRING( "components" )
#define COMPONENT_NAME_FIELD OBFUSCATED_STRING( "componentName" )
#define COMPONENT_PER_ENTITY_VALUES_ARR_FIELD OBFUSCATED_STRING( "perEntityValues" )
#define COMPONENT_VALUES_FIELD OBFUSCATED_STRING( "values" )
#define COMPONENT_ENTITY_NAME_FIELD OBFUSCATED_STRING( "entityName" )

static constexpr size_t MAX_PATH_LEN = 512;
static constexpr size_t MAX_FULL_PATH_LEN = MAX_PATH_LEN + 64;
static constexpr size_t MAX_ENTITY_NAME_LEN = 256;

struct ParsingEnitity
{
	char entityName[MAX_ENTITY_NAME_LEN] = { 0 };
	char templatePath[MAX_PATH_LEN] = { 0 };
	EntityID entityID = INVALID_ENTITY_ID;

	void Reset()
	{
		memset( entityName, 0, ARRAY_SIZE( entityName ) );
		memset( templatePath, 0, ARRAY_SIZE( templatePath ) );
		entityID = INVALID_ENTITY_ID;
	}
};

// TODO: maybe only have this memory instantiated when scene loading is needed instead of making it global and static all the 
// time? Maybe make it part of the SceneLoader object?
static ParsingEnitity s_entitiesMap[MAX_ENTITIES];

static char s_fullPath[MAX_FULL_PATH_LEN];


// ===========================
// Private Helpers
// ===========================

static void ResetCachedParsingEntities()
{
	for ( size_t i = 0; i < ARRAY_SIZE( s_entitiesMap ); ++i )
	{
		s_entitiesMap[i].Reset();
	}
}


static bool CacheParsingEntity( const json::string_t& entityName, const json::string_t& templatePath, const EntityID entityID )
{
	COM_ASSERT( entityName.length() <= MAX_ENTITY_NAME_LEN, "[%s]: %s - entity name exceeds max entity name length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, entityName.length(), MAX_ENTITY_NAME_LEN );
	COM_ASSERT( templatePath.length() <= MAX_PATH_LEN, "[%s]: %s - entity template path exceeds max path length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, templatePath.length(), MAX_PATH_LEN );
	
	if ( entityID == INVALID_ENTITY_ID )
	{
		return false;
	}

	const uint64_t hash = FNV1A_64_Hash( entityName.c_str(), entityName.length() );
	const size_t arrSize = ARRAY_SIZE( s_entitiesMap );
	const size_t hashIndex = hash % arrSize;
	for ( size_t i = 0; i < arrSize; ++i )
	{
		const size_t currIndex = ( hashIndex + i ) % arrSize;
		ParsingEnitity* currParsingEntity = &s_entitiesMap[currIndex];
		if ( currParsingEntity->entityID == INVALID_ENTITY_ID )
		{
			currParsingEntity->entityID = entityID;
			memcpy( currParsingEntity->entityName, entityName.c_str(), MAX_ENTITY_NAME_LEN );
			memcpy( currParsingEntity->templatePath, templatePath.c_str(), MAX_PATH_LEN );

			return true;
		}
	}

	COM_ALWAYS_ASSERT( "[%s]: %s - unable to cache parsing entity due to map being full. Max entities: %zu\n", SCENE_LOADER_STR, __FUNCTION__, MAX_ENTITIES );
	return false;
}


static const ParsingEnitity* GetCachedParsingEntity( const json::string_t& entityName )
{
	COM_ASSERT( entityName.length() <= MAX_ENTITY_NAME_LEN, "[%s]: %s - entity name exceeds max entity name length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, entityName.length(), MAX_ENTITY_NAME_LEN );

	const uint64_t hash = FNV1A_64_Hash( entityName.c_str(), entityName.length() );
	const size_t arrSize = ARRAY_SIZE( s_entitiesMap );
	const size_t hashIndex = hash % arrSize;
	for ( size_t i = 0; i < arrSize; ++i )
	{
		const size_t currIndex = ( hashIndex + i ) % arrSize;
		const ParsingEnitity* currParsingEntity = &s_entitiesMap[currIndex];
		if ( Com_StrEq( currParsingEntity->entityName, entityName.c_str(), MAX_ENTITY_NAME_LEN ) )
		{
			return currParsingEntity;
		}
	}

	return nullptr;
}


static const char* GetFullPath( const char* pathPrefix, const char* scenePath )
{
	if ( !pathPrefix || !scenePath )
	{
		return nullptr;
	}

	COM_ASSERT( ( strlen( scenePath ) + strlen( pathPrefix ) ) < MAX_FULL_PATH_LEN, "[ResourceManager]: given scene path '%s%s' exceeds max scene path length of '%zu'\n", pathPrefix, scenePath, MAX_FULL_PATH_LEN );

	snprintf( s_fullPath, MAX_FULL_PATH_LEN, "%s%s", pathPrefix, scenePath );
	return s_fullPath;
}

// ===========================
// TODO: maybe define the per-component parsers here? 
// ===========================


// ===========================
// SceneLoader Methods
// ===========================

// TODO: might be worth moving scene loading logic to ecs.cpp since I will need to modify the components list anyways...
// unless I expose the components list through a public function somehow but that could be dangerous?

bool SceneLoader::LoadScene( const char* sceneRef )
{
	const char* fullScenePath = GetFullPath( SCENES_DIR_PATH, sceneRef );
	if ( !fullScenePath )
	{
		return false;
	}

	ifstream inputFile( fullScenePath );
	if ( !inputFile.is_open() )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "Failed to open scene file '%s'\n", fullScenePath);
		return false;
	}

	// get file size
	inputFile.seekg( 0, ios::end );
	const streamsize inputFileSize = inputFile.tellg();
	inputFile.seekg( 0, ios::beg );

	// load full file content 
	vector<char> fileBuffer( inputFileSize, 0 );
	inputFile.read( fileBuffer.data(), inputFileSize);

	// parse json
	const json jsonScene = json::parse( fileBuffer );
	
	// TODO: remove this print
	std::cout << std::setw( 4 ) << jsonScene << "\n\n";

	// clear all previous scene data
	ECS_DeleteAllEntities();
	ResourceManager_UnloadAllResources();

	// TODO: loading scene steps:
	// 1) initialize all entities and map entity ID to entity name + keep track of template 
	//    - either keep track of template name so that it can be loaded later or pre-load template in separate map so that
	//      it can be re-used by all entities that need it and then also keep name in entity map so that I know how to get from
	//      entity to template
	//    - potentially handle loading resources here too especially if I am already pre-loading templates so I can already
	//      see what resources need to be loaded
	ResetCachedParsingEntities();
	const json jsonEntitiesArray = jsonScene[ENTITIES_ARRAY_FIELD.ToStdString()];
	COM_ASSERT( jsonEntitiesArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, ENTITIES_ARRAY_FIELD );
	for ( size_t i = 0; i < jsonEntitiesArray.size(); ++i )
	{
		const EntityID newEntityID = ECS_AddEntity();
		if ( newEntityID == INVALID_ENTITY_ID )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to initialize all entities from scene\n" );
			break;
		}

		const json jsonCurrEntity = jsonEntitiesArray[i];
		const json::string_t currEntityName = jsonCurrEntity[ENTITY_NAME_FIELD.ToStdString()];
		const json::string_t currTemplatePath = jsonCurrEntity[ENTITY_TEMPLATE_FIELD.ToStdString()];
		
		CacheParsingEntity( currEntityName, currTemplatePath, newEntityID );
	}

	Com_Printf( "template for no_template_entity: %s\n", GetCachedParsingEntity( "no_template_entity" )->templatePath );
	Com_Printf( "template for template_override_values: %s\n", GetCachedParsingEntity( "template_override_values" )->templatePath );
	Com_Printf( "template for using_template: %s\n", GetCachedParsingEntity( "using_template" )->templatePath );

	Com_Printf( "ID for no_template_entity: %zu\n", GetCachedParsingEntity( "no_template_entity" )->entityID );
	Com_Printf( "ID for template_override_values: %zu\n", GetCachedParsingEntity( "template_override_values" )->entityID );
	Com_Printf( "ID for using_template: %zu\n", GetCachedParsingEntity( "using_template" )->entityID );


	// 2) iterate through list of components, use entity name to get entity ID and potential template
	// values/overrides and then initialize component 
	//    - potentially have logic be:
	//     a) if using template, recurse through all templates and their parents until no parent is used
	//     b) apply parent template values, pop, apply previous parent overrides, pop, etc.
	//         - do this by indexing in to template component list using component name as key (component object migth be empty)
	//         - this means that parent values will be set even if they are overriden... which is ok but maybe inefficient? 
	//             - so maybe as recursing, set values, go to parent, only set unset values, etc. This can also avoid loading
	//				 resources that are never used. The only thing is how can I tell that value has been set or not? Do I have to
	//               make a dirty bit for all values? Maybe in entity ID to template mapping keep track of already set fields somehow?
	//               Maybe as a list of strings? This might be overkill
	//     c) then apply loaded entity overrides (hopefully by re-using the same deserialization function as the 
	//        parent recursion used
	//     d) if no template is used, re-use same deserialization function to set entity's component's values
	const json jsonComponentsArray = jsonScene[COMPONENTS_ARRAY_FIELD.ToStdString()];
	COM_ASSERT( jsonComponentsArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, COMPONENTS_ARRAY_FIELD );
	for ( size_t componentIndex = 0; componentIndex < jsonComponentsArray.size(); ++componentIndex )
	{
		const json jsonCurrComp = jsonComponentsArray[componentIndex];

		const json::string_t currCompName = jsonCurrComp[COMPONENT_NAME_FIELD.ToStdString()];
		const ComponentType currCompType = Components_GetComponentTypeForString( currCompName.c_str() );
		if ( currCompType == ComponentType::Invalid )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "component name '%s' is not a valid component", currCompName.c_str() );
			continue;
		}

		// TODO: somehow call processing function for the given component type and then process the entities recursivelly somehow to get the values from
		// the parents
		const json jsonCurrCompEntitiesList = jsonCurrComp[COMPONENT_PER_ENTITY_VALUES_ARR_FIELD.ToStdString()];
		for ( size_t entityIndex = 0; entityIndex < jsonCurrCompEntitiesList.size(); ++entityIndex )
		{
			const json currEntity = jsonCurrCompEntitiesList[entityIndex];
			// TODO: have to figure out how to handle recursion here and then only call parser when I want to store 
			// component values
			ComponentParser_ParserFunctPtr parsingFunct = ComponentParser_GetParserForType( currCompType );
			parsingFunct( &currEntity, this );
		}

	}

	 
	
	// 3) in loading process, as resources come up, cache them 

	// TODO: for the per component de-serilizers/parsers, maybe pass in a pointer to the component to be set, that way
	// they don't have to care about the entity ID as it recurses through parents.
	//    - Although need to somehow allow access to resource map (for loading resources) and to entities map (for entity name
	//      to entity ID resolving for fields)
	//	  - Ideally don't have to pass in resource map, so maybe the resources can be loaded separately? Maybe during entity 
	//      parsing since I will be able to pre-load all templates as well so might as well pre-load resources too? Then 
	//      components can simply pash a hashed version of the resource name to get the resoruce.
	//			- although this might cause a problem where template has resource defined, but in the actual scene, the 
	//			  resource is overriden so the template resource is never used but is loaded. This is inefficient, so maybe 
	//			  it is best to only load final values? 
	Com_PrintfVerbose( SCENE_LOADER_STR, "Finished loading scene '%s'", sceneRef );
	return true;
}