#include "common/global_defines.h"
#include "common/lib/com_assert.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include "common/lib/data_structures/fixed_map.h"
#include "common/lib/data_structures/fixed_hash_set.h"
#include "common/hashing/hash.h"
#include "core/ecs.h"
#include "scene_component_parsers.h"
#include "scene_loader.h"
#include <inttypes.h>
#include <iostream>
#include <fstream>

static const char* SCENE_LOADER_STR = OBFUSCATED_STRING( "SceneLoader" );

static const char* SCENES_DIR_PATH = GAME_DIR_PATH "scenes/";
static const char* ENTITY_TEMPLATES_DIR_PATH = GAME_DIR_PATH "entity_templates/";

static const char* RESOURCES_ARRAY_FIELD = OBFUSCATED_STRING( "resources" );
static const char* RESOURCE_NAME_FIELD = OBFUSCATED_STRING( "resourceName" );
static const char* RESOURCE_TYPE_FIELD = OBFUSCATED_STRING( "resourceType" );

static const char* ENTITIES_ARRAY_FIELD = OBFUSCATED_STRING( "entities" );
static const char* ENTITY_NAME_FIELD = OBFUSCATED_STRING( "entityName" );
static const char* ENTITY_TEMPLATE_FIELD = OBFUSCATED_STRING( "template" );

static const char* COMPONENTS_ARRAY_FIELD = OBFUSCATED_STRING( "components" );
static const char* COMPONENT_NAME_FIELD = OBFUSCATED_STRING( "componentName" );
static const char* COMPONENT_PER_ENTITY_VALUES_ARR_FIELD = OBFUSCATED_STRING( "perEntityValues" );
static const char* COMPONENT_VALUES_FIELD = OBFUSCATED_STRING( "values" );
static const char* COMPONENT_ENTITY_NAME_FIELD = OBFUSCATED_STRING( "entityName" );

static const char* TEMPLATE_PARENT_FIELD = OBFUSCATED_STRING( "parentTemplate" );

static constexpr size_t MAX_PATH_LEN = MAX_SCENE_PATH_LEN;
static constexpr size_t MAX_FULL_PATH_LEN = MAX_PATH_LEN + 64;
static constexpr size_t MAX_ENTITY_NAME_LEN = 256;
static constexpr size_t MAX_CACHED_PARSING_TEMPLATES = 64;

using UsedTemplatesSet = FixedHashSetStringKey < MAX_PATH_LEN, MAX_CACHED_PARSING_TEMPLATES>;
using json = nlohmann::json;
using namespace std;

struct ParsingEntity
{
	char templatePath[MAX_PATH_LEN] = { 0 };
	EntityID entityID = INVALID_ENTITY_ID;

	ParsingEntity() = default;

	ParsingEntity( const char* templatePath, const EntityID entityID )
	{
		strcpy_s( this->templatePath, templatePath );
		this->entityID = entityID;
	}

	ParsingEntity( const ParsingEntity& other )
	{
		strcpy_s( templatePath, other.templatePath );
		entityID = other.entityID;
	}

	ParsingEntity& operator=( const ParsingEntity& other )
	{
		strcpy_s( templatePath, other.templatePath );
		entityID = other.entityID;
		return *this;
	}
};

struct ParsingTemplate
{
	json jsonTemplate;
	// TODO: in an ideal world, this dynamic memory shouldn't be used or at the very least its memory should be 
	// freed at the end of the scene loading
	vector<char> fileBuffer; 
};

// TODO: maybe only have this memory instantiated when scene loading is needed instead of making it global and static all the 
// time? Maybe make it part of the SceneLoader object? Or place this in the SceneLoader object and then we only initialize the
// scene loader on the stack so that this memory is freed whenever the scene loader goes out of scope?
static FixedMapStringKey<ParsingEntity, MAX_ENTITY_NAME_LEN, MAX_ENTITIES> s_parsingEntitiesMap;
static FixedMapStringKey<ParsingTemplate, MAX_PATH_LEN, MAX_CACHED_PARSING_TEMPLATES> s_parsingTemplatesMap;

static char s_fullPath[MAX_FULL_PATH_LEN];


// ===========================
// Private Helpers
// ===========================

static void ResetCachedParsingEntities()
{
	s_parsingEntitiesMap.Clear();
}


static bool CacheParsingEntity( const json::string_t& entityName, const json::string_t& templatePath, const EntityID entityID )
{
	COM_ASSERT( entityName.length() <= MAX_ENTITY_NAME_LEN, "[%s]: %s - entity name exceeds max entity name length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, entityName.length(), MAX_ENTITY_NAME_LEN );
	COM_ASSERT( templatePath.length() <= MAX_PATH_LEN, "[%s]: %s - entity template path exceeds max path length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, templatePath.length(), MAX_PATH_LEN );
	
	if ( entityID == INVALID_ENTITY_ID )
	{
		return false;
	}

	COM_ASSERT( !s_parsingEntitiesMap.IsFull(), "[%s]: %s - unable to cache parsing entity due to map being full. Max entities: %zu\n", SCENE_LOADER_STR, __FUNCTION__, s_parsingEntitiesMap.Capacity() );

	return s_parsingEntitiesMap.Insert( entityName.c_str(), ParsingEntity( templatePath.c_str(), entityID ) );
}


static const ParsingEntity* GetCachedParsingEntity( const json::string_t& entityName )
{
	COM_ASSERT( entityName.length() <= MAX_ENTITY_NAME_LEN, "[%s]: %s - entity name exceeds max entity name length. %zu > %zu\n", SCENE_LOADER_STR, __FUNCTION__, entityName.length(), MAX_ENTITY_NAME_LEN );

	if ( Com_StrEmpty( entityName.c_str() ) )
	{
		return nullptr;
	}

	return s_parsingEntitiesMap[entityName.c_str()];
}


static const char* GetFullPath( const char* pathPrefix, const char* path )
{
	if ( !pathPrefix || !path )
	{
		return nullptr;
	}

	COM_ASSERT( ( strlen( path ) + strlen( pathPrefix ) ) < MAX_FULL_PATH_LEN, "[%s]: given scene path '%s%s' exceeds max scene path length of '%zu'\n", SCENE_LOADER_STR, pathPrefix, path, MAX_FULL_PATH_LEN );

	snprintf( s_fullPath, MAX_FULL_PATH_LEN, "%s%s", pathPrefix, path );
	return s_fullPath;
}


static void ParseAndLoadResources( const json& jsonResourcesArray )
{
	for ( size_t i = 0; i < jsonResourcesArray.size(); ++i )
	{
		const json jsonCurrResource = jsonResourcesArray[i];
		const json::string_t currResourceName = jsonCurrResource[RESOURCE_NAME_FIELD];
		const json::string_t currResourceTypeStr = jsonCurrResource[RESOURCE_TYPE_FIELD];
		const ResourceType currResourceType = ResourceTypes_GetResourceTypeForString( currResourceTypeStr.c_str() );
		if ( currResourceType == ResourceType::Invalid )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "resource '%s' has invalid resource type '%s'", currResourceName.c_str(), currResourceTypeStr.c_str() );
			continue;
		}

		// handles already loaded resources internally
		if ( !ResourceManager_LoadResource( HashedString( currResourceName.c_str(), currResourceName.length() ), currResourceName.c_str(), currResourceType ) )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to load resource '%s' with type '%s'", currResourceName.c_str(), currResourceTypeStr.c_str() );
		}
	}
}


static json* GetTemplateJson( const char* templatePath )
{
	if ( Com_StrEmpty( templatePath ) )
	{
		return nullptr;
	}
	
	// see if cached entry already exists
	ParsingTemplate* jsonEntry = s_parsingTemplatesMap[templatePath];
	if ( jsonEntry )
	{
		return &jsonEntry->jsonTemplate;
	}

	// cache new entry 
	COM_ASSERT( !s_parsingTemplatesMap.IsFull(), "[%s]: %s - unable to cache parsing tempalte due to map being full. Max entities: %zu\n", SCENE_LOADER_STR, __FUNCTION__, s_parsingTemplatesMap.Capacity() );
	ParsingTemplate* newEntry = s_parsingTemplatesMap.InsertKey( templatePath );
	COM_ASSERT( newEntry, "[%s]: %s - attempted to cache duplicate template path: %s\n", SCENE_LOADER_STR, __FUNCTION__, templatePath );

	newEntry->fileBuffer.clear();
	newEntry->jsonTemplate.clear();

	const char* fullTemplatePath = GetFullPath( ENTITY_TEMPLATES_DIR_PATH, templatePath );
	if ( !fullTemplatePath )
	{
		return nullptr;
	}

	ifstream templateFile( fullTemplatePath );
	if ( !templateFile.is_open() )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "Failed to open scene file '%s'", fullTemplatePath );
		return nullptr;
	}

	// get file size
	templateFile.seekg( 0, ios::end );
	const streamsize templateFileSize = templateFile.tellg();
	templateFile.seekg( 0, ios::beg );

	newEntry->fileBuffer.resize( templateFileSize, '\0' );
	templateFile.read( newEntry->fileBuffer.data(), templateFileSize );

	newEntry->jsonTemplate = json::parse( newEntry->fileBuffer );

	return &newEntry->jsonTemplate;
}


static bool ParseTemplateForComponent( const char* componentName, const ComponentParser_ParserFunctPtr componentParser, const EntityID entityID, const char* templatePath, SceneLoader* sceneLoader, UsedTemplatesSet* usedTemplatesSet )
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

#if IS_ENABLED( DEV_BUILD )
	// see if we are in a circular reference (AKA processing an already processed template)
	if ( usedTemplatesSet )
	{
		if ( usedTemplatesSet->Contains( templatePath ) )
		{
			COM_ALWAYS_ASSERT( "[%s]: encountered a circular reference while processing templates. Duplicate template: '%s'\n", SCENE_LOADER_STR, templatePath );
			return false;
		}
		else
		{
			COM_ASSERT( !usedTemplatesSet->IsFull(), "[%s]: unable to add to used templates set due to it being full. Might need to bump size. Size: %zu\n", SCENE_LOADER_STR, usedTemplatesSet->Capacity() );
			usedTemplatesSet->Insert( templatePath );
		}
	}
#endif // #if IS_ENABLED( DEV_BUILD )

	json* jsonTemplate = GetTemplateJson( templatePath );
	if ( !jsonTemplate )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to get json template for path '%s'", templatePath );
		return false;
	}

	// process parent template 
	// TODO: have to figure out case where child overrides parent resources so have currently 
	// the resources loaded by the parent will stay in memory and remain unused foever. Maybe template holds reference of overriden
	// resources? OR resource manager can free resources that have no references (AKA keep a reference counter)
	const json::string_t jsonParentTemplatePath = (*jsonTemplate)[TEMPLATE_PARENT_FIELD];
	if ( jsonParentTemplatePath.length() > 0 )
	{
		if ( !ParseTemplateForComponent( componentName, componentParser, entityID, jsonParentTemplatePath.c_str(), sceneLoader, usedTemplatesSet ) )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to parse parent '%s' for template '%s'", jsonParentTemplatePath.c_str(), templatePath );
			return false;
		}
	}

	// load resources for template 
	const json jsonResourcesArray = (*jsonTemplate)[RESOURCES_ARRAY_FIELD];
	COM_ASSERT( jsonResourcesArray.is_array(), "[%s]: %s: - json field [%s] in template [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, RESOURCES_ARRAY_FIELD, templatePath );
	ParseAndLoadResources( jsonResourcesArray );

	// load component values if they exist (will override parent values if applicable)
	const json jsonComponents = (*jsonTemplate)[COMPONENTS_ARRAY_FIELD];
	if ( jsonComponents.contains( componentName ) )
	{
		const json jsonCurrComponent = jsonComponents[componentName];
		const json jsonComponentValues = jsonCurrComponent[COMPONENT_VALUES_FIELD];
		componentParser( entityID, jsonComponentValues, sceneLoader );
	}

	return true;
}

static bool ParseEntityForComponent( const ComponentType componentType, const char* componentName, const ComponentParser_ParserFunctPtr componentParser, const json& jsonEntity, SceneLoader* sceneLoader )
{
	if ( !componentParser )
	{ 
		COM_ALWAYS_ASSERT( "[%s]: component parser is null\n", SCENE_LOADER_STR );
		return false;
	}

	// get entity name and entity ID
	const json::string_t entityName = jsonEntity[ENTITY_NAME_FIELD];
	const ParsingEntity* parsingEntity = GetCachedParsingEntity( entityName );
	if ( !parsingEntity )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to find parsing data for entity '%s'", parsingEntity );
		return false;
	}

	const EntityID entityID = parsingEntity->entityID;
	if ( entityID == INVALID_ENTITY_ID )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "parsing entity data for entity '%s' has invalid entity ID", parsingEntity );
		return false;
	}

	// process template component values
	const char* templatePath = parsingEntity->templatePath;
	if ( !Com_StrEmpty( templatePath ) )
	{
#if IS_ENABLED( DEV_BUILD )
		// dev only logic used to detect circular references in templates
		UsedTemplatesSet usedTemplatesSet;
		UsedTemplatesSet* usedTemplatesSetPtr = &usedTemplatesSet;
#else // #if IS_ENABLED( DEV_BUILD )
		UsedTemplatesSet* usedTemplatesSetPtr = nullptr;
#endif // #else // #if IS_ENABLED( DEV_BUILD )

		if ( !ParseTemplateForComponent( componentName, componentParser, entityID, templatePath, sceneLoader, usedTemplatesSetPtr ) )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "error encountered while parsing template for component '%s' for entity '%s' with entity ID '%" PRIu64 "'", componentName, parsingEntity, entityID );
			return false;
		}
	}

	// load component values for entity (will override template values if applicable)
	const json jsonComponentValues = jsonEntity[COMPONENT_VALUES_FIELD];
	componentParser( entityID, jsonComponentValues, sceneLoader );

	// add component to entity
	if ( !ECS_AddComponentToEntity( entityID, componentType ) )
	{
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "error encountered while adding component '%s' to entity '%s' with entity ID '%" PRIu64 "'", componentName, parsingEntity, entityID );
		return false;
	}

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
		Com_PrintfErrorVerbose( SCENE_LOADER_STR, "Failed to open scene file '%s'", fullScenePath);
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
	ECS_ClearSceneEntities( m_EntityManagementToken );
	ResourceManager_UnloadAllResources();

	// load all resources for the given entity
	const json jsonResourcesArray = jsonScene[RESOURCES_ARRAY_FIELD];
	COM_ASSERT( jsonResourcesArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, RESOURCES_ARRAY_FIELD );
	ParseAndLoadResources( jsonResourcesArray );

	// initialize all entities in the scene and cache entity parsing metadata 
	ResetCachedParsingEntities();
	const json jsonEntitiesArray = jsonScene[ENTITIES_ARRAY_FIELD];
	COM_ASSERT( jsonEntitiesArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, ENTITIES_ARRAY_FIELD );
	for ( size_t i = 0; i < jsonEntitiesArray.size(); ++i )
	{
		const EntityID newEntityID = ECS_AddSceneEntity( m_EntityManagementToken );
		if ( newEntityID == INVALID_ENTITY_ID )
		{
			Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to initialize all entities from scene" );
			break;
		}

		const json jsonCurrEntity = jsonEntitiesArray[i];
		const json::string_t currEntityName = jsonCurrEntity[ENTITY_NAME_FIELD];
		const json::string_t currTemplatePath = jsonCurrEntity[ENTITY_TEMPLATE_FIELD];
		
		CacheParsingEntity( currEntityName, currTemplatePath, newEntityID );
	}

	// initialize components in all entities grouped by component type (since this is friendlier to the ECS design pattern)
	const json jsonComponentsArray = jsonScene[COMPONENTS_ARRAY_FIELD];
	COM_ASSERT( jsonComponentsArray.is_array(), "[%s]: %s: - json field [%s] is not an array\n", SCENE_LOADER_STR, __FUNCTION__, COMPONENTS_ARRAY_FIELD );
	for ( size_t componentIndex = 0; componentIndex < jsonComponentsArray.size(); ++componentIndex )
	{
		const json jsonCurrComp = jsonComponentsArray[componentIndex];

		const json::string_t currCompName = jsonCurrComp[COMPONENT_NAME_FIELD];
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

		const json jsonCurrCompEntitiesList = jsonCurrComp[COMPONENT_PER_ENTITY_VALUES_ARR_FIELD];
		for ( size_t entityIndex = 0; entityIndex < jsonCurrCompEntitiesList.size(); ++entityIndex )
		{
			const json currEntity = jsonCurrCompEntitiesList[entityIndex];
			if ( !ParseEntityForComponent( currCompType, currCompName.c_str(), componentTypeParsingFunct, currEntity, this ) )
			{
				const json::string_t currEntityName = currEntity[ENTITY_NAME_FIELD];
				Com_PrintfErrorVerbose( SCENE_LOADER_STR, "unable to load component '%s' values for entity '%s'", currCompName.c_str(), currEntityName.c_str() );
			}
		}

	}

	// clear parsing caches
	s_parsingTemplatesMap.Clear();

	Com_PrintfVerbose( SCENE_LOADER_STR, "Finished loading scene '%s'", sceneRef );
	return true;
}