////////////////////////////////////////////////////////////
//
// System for handling the physics for 2D entities
//
////////////////////////////////////////////////////////////


#pragma once
#include "core/system.h"

class PhysicsBody2DSystem : public System
{
public:
	void OnPhysicsFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) override;
};

