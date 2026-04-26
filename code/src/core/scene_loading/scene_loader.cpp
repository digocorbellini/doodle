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

#define TEMPLATE_PARENT_FIELD OBFUSCATED_STRING( "parentTemplate" )


static constexpr size_t MAX_PATH_LEN = 512;
static constexpr size_t MAX_FULL_PATH_LEN = MAX_PATH_LEN + 64;
static constexpr size_t MAX_ENTITY_NAME_LEN = 256;

struct ParsingEntity
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
// time? Maybe make it part of the SceneLoader object? Or place this in the SceneLoader object and then we only initialize the
// scene loader on the stack so that this memory is freed whenever the scene loader goes out of scope?
static ParsingEntity s_entitiesMap[MAX_ENTITIES];

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
		ParsingEntity* currParsingEntity = &s_entitiesMap[currIndex];
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


static const ParsingEntity* GetCachedParsingEntity( const json::string_t& entityName )
{
	COM_ASSERT( entityName.length() <= MAX_ENTITY_NAME_LEN, "[%s]: %s - entity name exceeds max entity name length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, entityName.length(), MAX_ENTITY_NAME_LEN );

	const uint64_t hash = FNV1A_64_Hash( entityName.c_str(), entityName.length() );
	const size_t arrSize = ARRAY_SIZE( s_entitiesMap );
	const size_t hashIndex = hash % arrSize;
	for ( size_t i = 0; i < arrSize; ++i )
	{
		const size_t currIndex = ( hashIndex + i ) % arrSize;
		const ParsingEntity* currParsingEntity = &s_entitiesMap[currIndex];
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

	COM_ASSERT( ( strlen( scenePath ) + strlen( pathPrefix ) ) < MAX_FULL_PATH_LEN, "[%s]: given scene path '%s%s' exceeds max scene path length of '%zu'\n", SCENE_LOADER_STR, pathPrefix, scenePath, MAX_FULL_PATH_LEN );

	snprintf( s_fullPath, MAX_FULL_PATH_LEN, "%s%s", pathPrefix, scenePath );
	return s_fullPath;
}


static void ParseAndLoadResources( const json& jsonResourcesArray )
{
	for ( size_t i = 0; i < jsonResourcesArray.size(); ++i )
	{
		const json jsonCurrResource = jsonResourcesArray[i];
		const json::string_t currResourceName = jsonCurrResource[RESOURCE_NAME_FIELD.ToStdString()];
		const json::string_t currResourceTypeStr = jsonCurrResource[RESOURCE_TYPE_FIELD.ToStdString()];
		const ResourceType currResourceType = ResourceTypes_GetResourceTypeForString( currResourceTypeStr.c_str() );
		if ( currResourceType == ResourceType::Invalid )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "resource '%s' has invalid resource type '%s'\n", currResourceName.c_str(), currResourceTypeStr.c_str() );
			continue;
		}

		if ( !ResourceManager_LoadResource( HashedString( currResourceName.c_str(), currResourceName.length() ), currResourceName.c_str(), currResourceType ) )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to load resource '%s' with type '%s'\n", currResourceName.c_str(), currResourceTypeStr.c_str() );
		}
	}
}


bool ParseTemplateForComponent( const char* componentName, const ComponentParser_ParserFunctPtr componentParser, const EntityID entityID, const char* templatePath, SceneLoader* sceneLoader )
{
	if ( !componentParser )
	{
		COM_ALWAYS_ASSERT( "[%s]: component parser is null\n", SCENE_LOADER_STR );
		return false;
	}

	if ( !componentName )
	{
		COM_ALWAYS_ASSERT( "[%s]: component name is null\n", SCENE_LOADER_STR );
		return false;
	}

	// see if we have reached leaf template
	if ( Com_StrEmpty( templatePath ) )
	{
		return true;
	}
	
	// TODO: load template file (would be best to cache this somehow so that we don't have to re-open re-used templates for every
	// component and entity that uses this)
	const char* fullTemplatePath = GetFullPath( ENTITY_TEMPLATES_DIR_PATH, templatePath );
	if ( !fullTemplatePath )
	{
		return false;
	}

	ifstream templateFile( fullTemplatePath );
	if ( !templateFile.is_open() )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "Failed to open scene file '%s'\n", fullTemplatePath );
		return false;
	}

	// get file size
	templateFile.seekg( 0, ios::end );
	const streamsize templateFileSize = templateFile.tellg();
	templateFile.seekg( 0, ios::beg );

	// load full file content 
	vector<char> fileBuffer( templateFileSize, 0 );
	templateFile.read( fileBuffer.data(), templateFileSize );

	// parse json
	const json jsonTemplate = json::parse( fileBuffer );

	// process parent template 
	// TODO: have to figure out case where child overrides parent resources so have currently 
	// the resources loaded by the parent will stay in memory and remain unused foever. Maybe template holds reference of overriden
	// resources? OR resource manager can free resources that have no references (AKA keep a reference counter)
	const json::string_t jsonParentTemplatePath = jsonTemplate[TEMPLATE_PARENT_FIELD.ToStdString()];
	if ( jsonParentTemplatePath.length() > 0 )
	{
		if ( !ParseTemplateForComponent( componentName, componentParser, entityID, jsonParentTemplatePath.c_str(), sceneLoader ) );
	}

	// load resources for template
	const json jsonResourcesArray = jsonTemplate[RESOURCES_ARRAY_FIELD.ToStdString()];
	COM_ASSERT( jsonResourcesArray.is_array(), "[%s]: %s: - json field [%s] in file [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, RESOURCES_ARRAY_FIELD, fullTemplatePath );
	ParseAndLoadResources( jsonResourcesArray );

	// load component values (will override parent values if applicable)
	const json jsonComponents = jsonTemplate[COMPONENTS_ARRAY_FIELD.ToStdString()];
	const json jsonCurrComponent = jsonComponents[componentName];
	const json jsonComponentValues = jsonCurrComponent[COMPONENT_VALUES_FIELD.ToStdString()];
	componentParser( entityID, jsonComponentValues, sceneLoader );

	return true;
}

bool ParseEntityForComponent( const char* componentName, const ComponentParser_ParserFunctPtr componentParser, const json& jsonEntity, SceneLoader* sceneLoader )
{
	if ( !componentParser )
	{ 
		COM_ALWAYS_ASSERT( "[%s]: component parser is null\n", SCENE_LOADER_STR );
		return false;
	}

	// get entity name and entity ID
	const json::string_t entityName = jsonEntity[ENTITY_NAME_FIELD.ToStdString()];
	const ParsingEntity* parsingEntity = GetCachedParsingEntity( entityName );
	if ( !parsingEntity )
	{
		return false;
	}

	const EntityID entityID = parsingEntity->entityID;
	if ( entityID == INVALID_ENTITY_ID )
	{
		return false;
	}

	// process template
	const char* templatePath = parsingEntity->templatePath;
	if ( !Com_StrEmpty( templatePath ) )
	{
		if ( !ParseTemplateForComponent( componentName, componentParser, entityID, templatePath, sceneLoader ) )
		{
			return false;
		}
	}

	// load components for entity (will override template values if applicable)
	const json jsonComponentValues = jsonEntity[COMPONENT_VALUES_FIELD.ToStdString()];
	componentParser( entityID, jsonComponentValues, sceneLoader );

	return true;
}


// ===========================
// SceneLoader Methods
// ===========================

EntityID SceneLoader::GetParsingEntityID( const json::string_t& entityName )
{
	const ParsingEntity* parsingEnitity = GetCachedParsingEntity( entityName );
	if ( !parsingEnitity )
	{
		return INVALID_ENTITY_ID;
	}

	return parsingEnitity->entityID;
}


bool SceneLoader::LoadScene( const char* sceneRef )
{
	const char* fullScenePath = GetFullPath( SCENES_DIR_PATH, sceneRef );
	if ( !fullScenePath )
	{
		return false;
	}

	ifstream sceneFile( fullScenePath );
	if ( !sceneFile.is_open() )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "Failed to open scene file '%s'\n", fullScenePath);
		return false;
	}

	// get file size
	sceneFile.seekg( 0, ios::end );
	const streamsize sceneFileSize = sceneFile.tellg();
	sceneFile.seekg( 0, ios::beg );

	// load full file content 
	vector<char> fileBuffer( sceneFileSize, 0 );
	sceneFile.read( fileBuffer.data(), sceneFileSize);

	// parse json
	const json jsonScene = json::parse( fileBuffer );

	// clear all previous scene data
	ECS_DeleteAllEntities();
	ResourceManager_UnloadAllResources();

	// load all resources for the given entity
	const json jsonResourcesArray = jsonScene[RESOURCES_ARRAY_FIELD.ToStdString()];
	COM_ASSERT( jsonResourcesArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, RESOURCES_ARRAY_FIELD );
	ParseAndLoadResources( jsonResourcesArray );

	// initialize all entities in the scene and cache entity parsing metadata 
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

	// initialize components in all entities grouped by component type (since this is friendlier to the ECS design pattern)
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

		ComponentParser_ParserFunctPtr componentTypeParsingFunct = ComponentParser_GetParserForType( currCompType );
		if ( !componentTypeParsingFunct )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to find parsing function for component '%s'", currCompName.c_str() );
			continue;
		}

		const json jsonCurrCompEntitiesList = jsonCurrComp[COMPONENT_PER_ENTITY_VALUES_ARR_FIELD.ToStdString()];
		for ( size_t entityIndex = 0; entityIndex < jsonCurrCompEntitiesList.size(); ++entityIndex )
		{
			const json currEntity = jsonCurrCompEntitiesList[entityIndex];
			if ( !ParseEntityForComponent( currCompName.c_str(), componentTypeParsingFunct, currEntity, this ) )
			{
				const json::string_t currEntityName = currEntity[ENTITY_NAME_FIELD.ToStdString()];
				Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to load component '%s' values for entity '%s'", currCompName.c_str(), currEntityName.c_str() );
			}
		}

	}

	Com_PrintfVerbose( SCENE_LOADER_STR, "Finished loading scene '%s'", sceneRef );
	return true;
}