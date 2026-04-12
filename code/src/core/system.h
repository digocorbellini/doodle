////////////////////////////////////////////////////////////
//
// Definition of the virtual "System" class which all 
// systems will inherit from. Systems are meant to be 
// static classes that automatically register themselves
// to the ecs system, which gives them access to some 
// optional methods to hook into game events and loops.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/types.h"
#include "core/ecs.h"
#include <SFML/Graphics.hpp>

class System
{
public:
	System()
	{
		ECS_RegisterSystem( this );
	}

	virtual ~System() = default;

	/// <summary>
	/// Called once per frame at the start of a frame (before the regular frame logic is run)
	/// </summary>
	/// <param name="deltaTime">The time since the last frame start</param>
	/// <param name="numEntities">The number of entities that currently exist</param>
	/// <param name="componenents">All componenets that currently exist</param>
	virtual void OnFrameStart( const double deltaTime, const EntityID numEntities, Components* componenents ) {};

	/// <summary>
	/// Called once per frame 
	/// </summary>
	/// <param name="deltaTime">The time since the last frame start</param>
	/// <param name="numEntities">The number of entities that currently exist</param>
	/// <param name="componenents">All componenets that currently exist</param>
	virtual void OnFrame( const double deltaTime, const EntityID numEntities, Components* componenents ) {};

	/// <summary>
	/// Called once per frame at the end of a frame (after the regular frame logic is run)
	/// </summary>
	/// <param name="deltaTime">The time since the last frame start</param>
	/// <param name="numEntities">The number of entities that currently exist</param>
	/// <param name="componenents">All componenets that currently exist</param>
	virtual void OnFrameEnd( const double deltaTime, const EntityID numEntities, Components* componenents ) {};

	/// <summary>
	/// Called once per frame after all regular logic frames in order to perform physics 
	/// </summary>
	/// <param name="deltaTime">The time since the last frame start</param>
	/// <param name="numEntities">The number of entities that currently exist</param>
	/// <param name="componenents">All componenets that currently exist</param>
	virtual void OnPhysicsFrame( const double deltaTime, const EntityID numEntities, Components* componenents ) {};

	/// <summary>
	/// Called once per frame after all other logic is run in order to draw to the window
	/// </summary>
	/// <param name="deltaTime">The time since the last frame start</param>
	/// <param name="numEntities">The number of entities that currently exist</param>
	/// <param name="componenents">All componenets that currently exist</param>
	virtual void OnDrawFrame( RenderWindow* window, const EntityID numEntities, Components* componenents ) {};
};