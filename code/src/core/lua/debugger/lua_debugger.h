////////////////////////////////////////////////////////////
//
// Handle lua debug hooks and DAP server for lua debugger
//
////////////////////////////////////////////////////////////


#pragma once
#include "common/global_defines.h"

#if IS_ENABLED( LUA_DEBUGGER )
// Call once after LuaL_openlibs to install the debug hook and start DAP serber.
// MUST be from main thread
void LuaDebugger_Init( struct lua_State* luaState );

void LuaDebugger_Frame();

// Called on shutdown
void LuaDebugger_Shutdown();
#endif // #if IS_ENABLED( LUA_DEBUGGER )
