#pragma once
#include "core/system.h"

class RenderingSystem : public System
{
public:
	void OnDrawFrame( RenderWindow* window, EntityIterator entityIterator ) override;
};