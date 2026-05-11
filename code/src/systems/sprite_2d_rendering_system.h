////////////////////////////////////////////////////////////
//
// System for handling the drawing of entities with
// sprites
//
////////////////////////////////////////////////////////////


#pragma once
#include "core/system.h"

class Sprite2DRenderingSystem : public System
{
public:
	void OnDrawFrame( sf::RenderWindow* window, EntityIterator entityIterator ) override;
};