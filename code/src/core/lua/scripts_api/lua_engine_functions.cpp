#include "common/lib/com_print.h"
#include <lua.hpp>
#include "lua_function_registration.h"

int LuaFunction_TestFunct( lua_State* luaState )
{
	Com_Printf( "LUA TEST FUNCTION CALLED!!!\n" );
	return 0;
}