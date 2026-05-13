#include "common/lib/math/vector_math.h"
#include "core/components.h";
#include "playercontroller2d_system.h"
#include <SFML/Window/Keyboard.hpp>


static const ComponentsMask PLAYER_CONTROLLER_COMP_MASK( { ComponentType::PhysicsBody2D, ComponentType::PlayerController2D } );
static PlayerController2DSystem s_playerController2dSystem;

using namespace sf;


// =========================
// System Methods
// =========================

void PlayerController2DSystem::OnFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator )
{
	for ( EntityIterator itr = entityIterator; itr != itr.end(); ++itr )
	{
		EntityID currEntity = *itr;

		// skip entities that do not have the required components
		if ( !ECS_GetEntityComponentsMask( currEntity ).ContainsMask( PLAYER_CONTROLLER_COMP_MASK) )
		{
			continue;
		}

		PhysicsBody2D* currPhysicsBody = ECS_GetEntityComponent<PhysicsBody2D>( currEntity, ComponentType::PhysicsBody2D );
		if ( !currPhysicsBody )
		{
			continue;
		}

		PlayerController2D* currPlayerController = ECS_GetEntityComponent<PlayerController2D>( currEntity, ComponentType::PlayerController2D);
		if ( !currPlayerController)
		{
			continue;
		}
		
		Vector2f movementDirection( 0.0f, 0.0f );

		// TODO: need some alternative form of input handling intead of hard coding keys. Maybe do something similar to unreal where inputs
		// map to actions and then systems listen for action events
		if ( Keyboard::isKeyPressed( Keyboard::A ) )
		{
			// go left
			movementDirection.x -= 1;
		}

		if ( Keyboard::isKeyPressed( Keyboard::D ) )
		{
			// go right
			movementDirection.x += 1;
		}

		if ( !currPlayerController->isPlatformingController )
		{
			if ( Keyboard::isKeyPressed( Keyboard::W ) )
			{
				// go up
				movementDirection.y -= 1;
			}

			if ( Keyboard::isKeyPressed( Keyboard::S ) )
			{
				// go right
				movementDirection.y += 1;
			}
		}

		// apply move speed 
		movementDirection = Vec2Math::Normalize( movementDirection );
		const Vector2f newMoveSpeed = Vec2Math::Scale( movementDirection, currPlayerController->moveSpeed );
		if ( currPlayerController->isPlatformingController )
		{
			currPhysicsBody->velocity.x = newMoveSpeed.x;
		}
		else
		{
			currPhysicsBody->velocity = newMoveSpeed;
		}
	}
}