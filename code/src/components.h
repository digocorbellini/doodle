////////////////////////////////////////////////////////////
//
// This file is meant to contain all components in the game.
// This may become unruly un a larger project, however,
// this engine is meant to be small and lightweight. 
// 
// A component in this engine is simply data represented by 
// structs. This data will be accessed and modified by
// systems which will create behaviours. 
// 
// Entities will only be acted on by a system if they contain
// a certain set of components. This is why the ComponentMask
// is defined in this file. This allows entities to know
// what components make them up and then systems will know
// what entities to act on.
//
////////////////////////////////////////////////////////////

#pragma once

#include <initializer_list>
#include <bitset>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#define MAX_COMPONENTS 64

using namespace std;
using namespace sf;

// Every new component must have a type in this enum
enum class ComponentType : size_t
{
	EntityTransform2D = 0,
	PhysicsBody2D,
	SpriteRenderer2D,
	PlayerController2D,
	Camera2D,

	Invalid = MAX_COMPONENTS
};

/// <summary>
/// A bitmask used to represent a set of active and inactive components
/// </summary>
class ComponentsMask
{
private:
	bitset<MAX_COMPONENTS> mask;

public:

	ComponentsMask() = default;
	ComponentsMask( const ComponentsMask& other );

	/// <summary>
	/// Creates a component mask with all of the given components set.
	/// </summary>
	/// <param name="compList">the list of components to be set</param>
	ComponentsMask( std::initializer_list<ComponentType> compList );

	/// <summary>
	/// Adds the given component to the mask. Returns true if 
	/// successful and false otherwise.
	/// </summary>
	/// <param name="component">the component to be added</param>
	/// <returns>true if successfully added and false othewise</returns>
	bool AddComponent( const ComponentType component );

	/// <summary>
	/// Adds all of the given components to the mask. Returns the 
	/// number of components added starting from left to right.
	/// </summary>
	/// <param name="compList">an initializer list of components to be added</param>
	/// <returns>the number of components added starting from left to right</returns>
	int AddComponents( std::initializer_list<ComponentType> compList );

	/// <summary>
	/// Removes the given component from the mask. Returns true if
	/// successful and false otherwise.
	/// </summary>
	/// <param name="component">the component to be removed</param>
	/// <returns>true if successfully removed and false othewise</returns>
	bool RemoveComponent( const ComponentType component );

	/// <summary>
	/// Removes all of the given components from the mask. Returns the
	/// number of components removed starting from left to right.
	/// </summary>
	/// <param name="compList">an initializer list of components to be removed</param>
	/// <returns>the number of components removed starting from left to right</returns>
	int RemoveComponents( std::initializer_list<ComponentType> compList );

	/// <summary>
	/// See if all of the components set in the given mask are persent
	/// in this mask. 
	/// </summary>
	/// <param name="other">the mask to compare against</param>
	/// <returns>true if the components in the given mask are a
	/// subset of the components of this mask. False otherwise</returns>
	inline bool ContainsMask( const ComponentsMask other )
	{
		return ( mask & other.mask ) == other.mask;
	}
	
	/// <summary>
	/// See if this mask has any components set or not. 
	/// </summary>
	/// <returns>true if this mask has any components set and false otherwise</returns>
	inline bool IsEmpty() 
	{ 
		return !mask.any(); 
	}
	
	inline bool operator==( ComponentsMask const& other ) const
	{
		return mask == other.mask;
	}

	inline bool operator!=( ComponentsMask const& other ) const
	{
		return mask != other.mask;
	}
};


//==========================
// Components
//==========================


struct EntityTransform2D
{
	Vector2f position;
};


struct PhysicsBody2D
{
	Vector2f velocity;
	float gravity;

};


struct SpriteRenderer2D
{
	Sprite sprite;
	Texture texture;
	int renderingOrder;

	bool isXFlipped;
	bool isYFlipped;
};


struct PlayerController2D
{
	float moveSpeed;
	float jumpSpeed;
};


struct Camera2D
{
	EntityTransform2D* targetTransform; // if null, will not try to follow a target and will instead be controlled by its transform
	Vector2f viewOffsetFromPos; // used to center the camera since position is top left corner
	View cameraView;
	bool isMainCam; // If true, this camera will be displayed on the window
};
