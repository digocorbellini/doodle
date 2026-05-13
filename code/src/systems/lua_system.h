////////////////////////////////////////////////////////////
//
// System for interfacing with Lua scripts
//
////////////////////////////////////////////////////////////


#pragma once
#include "core/system.h"

class LuaSystem : public System
{
protected:
	void OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) override;

public:
	LuaSystem();
	~LuaSystem();

	bool RestartLua();
	bool HotReload();
};

const LuaSystem& LuaSystem_GetLuaSystem();