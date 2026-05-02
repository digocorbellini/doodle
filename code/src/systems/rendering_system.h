#pragma once
#include "core/system.h"

class RenderingSystem : public System
{
public:
	void OnDrawFrame( sf::RenderWindow* window, EntityIterator entityIterator ) override;
};