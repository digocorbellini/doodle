#pragma once
#include "core/system.h"

class TestSystem : public System
{
	void OnFrame( const NanoSeconds deltaTimeNs, const EntityIterator* entityIterator ) override;
};