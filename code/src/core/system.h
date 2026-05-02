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
	/// <param name="deltaTimeNs">The time since the last frame start</param>
	/// <param name="entityIterator">An iterator over all entities in the game</param>
	virtual void OnFrameStart( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) {};

	/// <summary>
	/// Called once per frame 
	/// </summary>
	/// <param name="deltaTimeNs">The time since the last frame start</param>
	/// <param name="entityIterator">An iterator over all entities in the game</param>
	virtual void OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) {};

	/// <summary>
	/// Called once per frame at the end of a frame (after the regular frame logic is run)
	/// </summary>
	/// <param name="deltaTimeNs">The time since the last frame start</param>
	/// <param name="entityIterator">An iterator over all entities in the game</param>
	virtual void OnFrameEnd( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) {};

	/// <summary>
	/// Called once per frame after all regular logic frames in order to perform physics 
	/// </summary>
	/// <param name="deltaTimeNs">The time since the last frame start</param>
	/// <param name="entityIterator">An iterator over all entities in the game</param>
	virtual void OnPhysicsFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator ) {};

	/// <summary>
	/// Called once per frame after all other logic is run in order to draw to the window
	/// </summary>
	/// <param name="deltaTimeNs">The time since the last frame start</param>
	/// <param name="entityIterator">An iterator over all entities in the game</param>
	virtual void OnDrawFrame( sf::RenderWindow* window, EntityIterator entityIterator ) {};
};