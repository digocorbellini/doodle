////////////////////////////////////////////////////////////
//
// System for handling 2d player controls
//
////////////////////////////////////////////////////////////


#pragma once
#include "core/system.h"

class PlayerController2DSystem : public System
{
public:
	void OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) override;
};