#include "common/global_defines.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include <lua.hpp>
#include "core/lua/scripts_api/lua_function_registration.h"
#include "lua_system.h"

#define SCRIPTS_PATH GAME_DIR_PATH "scripts/"
#define ENTRY_SCRIPT_PATH SCRIPTS_PATH "main.lua"

static const char* LUA_SYSTEM_STR = OBFUSCATED_STRING( "LuaSystem" );

// declare all lua function libraries
#define LUA_FUNCTION( luaFunctName, cppFunctName ) { #luaFunctName, cppFunctName },
#define LUA_LIBRARY( libraryName, functionMacros ) \
static const luaL_Reg libraryName##Functs[] = { \
	functionMacros \
	{ nullptr, nullptr } \
};
LUA_FUNCTION_LIBRARIES
#undef LUA_LIBRARY
#undef LUA_FUNCTION

static LuaSystem s_luaSystem;
static lua_State* s_luaState;

// =====================
// Private Helpers
// =====================

static bool RunLuaEntryPoint( lua_State* luaState )
{
	if ( luaL_dofile( luaState, ENTRY_SCRIPT_PATH ) != LUA_OK )
	{
		COM_ALWAYS_ASSERT( "[%s]: failed to run lua entry point script '%s'\n", LUA_SYSTEM_STR, ENTRY_SCRIPT_PATH );
		lua_pop( luaState, 1 );
		return false;
	}

	return true;
}


static bool InitLua()
{
	s_luaState = luaL_newstate();
	if ( !s_luaState )
	{
		COM_ALWAYS_ASSERT( "[%s]: failed to initialize lua state\n", LUA_SYSTEM_STR );
		return false;
	}
	luaL_openlibs( s_luaState );

	if ( luaL_dostring( s_luaState, "package.path = package.path .. ';" SCRIPTS_PATH "?.lua'" ) != LUA_OK )
	{
		COM_ALWAYS_ASSERT( "[%s]: failed to set package path to '%s'\n", LUA_SYSTEM_STR, SCRIPTS_PATH );
		lua_pop( s_luaState, 1 );
		return false;
	}

	// register all cpp API functions that are callable from Lua
#define LUA_LIBRARY( libraryName, functionMacros ) luaL_register( s_luaState, #libraryName, libraryName##Functs );
	LUA_FUNCTION_LIBRARIES
#undef LUA_LIBRARY 

	if ( !RunLuaEntryPoint( s_luaState ) )
	{
		return false;
	}

	Com_PrintfVerbose( LUA_SYSTEM_STR, "lua state successfully initialized" );
	return true;
}


// =====================
// System Methods
// =====================

void LuaSystem::OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator )
{
	// TODO: call frame in lua
}


// =========================
// Public Methods
// =========================

LuaSystem::LuaSystem()
{
	InitLua();
}


LuaSystem::~LuaSystem()
{
	lua_close( s_luaState );
}


bool LuaSystem::RestartLua()
{
	Com_PrintfVerbose( LUA_SYSTEM_STR, "restarting lua state" );
	lua_close( s_luaState );
	return InitLua();
}


bool LuaSystem::HotReload()
{
#if IS_ENABLED( DEV_BUILD )
	// clear require cache so all modules are re-executed upon re-running lua entry point
	luaL_dostring( s_luaState, "for k in pairs(package.loaded) do package.loaded[k] = nil end" );

	if ( !RunLuaEntryPoint( s_luaState ) )
	{
		return false;
	}
#endif // #if IS_ENABLED( DEV_BUILD )

	return true;
}


// =========================
// Public Functions
// =========================

const LuaSystem& LuaSystem_GetLuaSystem()
{
	return s_luaSystem;
}