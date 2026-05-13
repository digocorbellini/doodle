#include "common/lib/math/time_utils.h"
#include "common/lib/math/vector_math.h"
#include "core/components.h"
#include "physicsbody2d_system.h"


// TODO: would be nice to expose this to other systems so that it can be modified and used,
static constexpr float GRAVITY_SCALE = 9.8f; // m/s^2

static const ComponentsMask PHYSICS_SYSTEM_COMP_MASK( { ComponentType::PhysicsBody2D, ComponentType::EntityTransform2D } );
static PhysicsBody2DSystem s_physicsSystem;

using namespace sf;


// =========================
// Private Helpers
// =========================

float newVelocity( const float initialVelocity, const float gravity, const float time )
{
	return initialVelocity + ( gravity * time );
}

// =========================
// System Methods
// =========================

void PhysicsBody2DSystem::OnPhysicsFrame( const NanoSeconds deltaTimeNs, EntityIterator entityIterator )
{
	const float deltaTimeSec = NanoToSeconds<float>( deltaTimeNs );

	// TODO: have to process a given entiies gravity and velocity and apply it to their transform
	// don't have to handle collisions since that will be handled by a separate collision system
	for ( EntityIterator itr = entityIterator; itr != itr.end(); ++itr )
	{
		EntityID currEntity = *itr;

		// skip entities that do not have the required components
		if ( !ECS_GetEntityComponentsMask( currEntity ).ContainsMask( PHYSICS_SYSTEM_COMP_MASK ) )
		{
			continue;
		}

		PhysicsBody2D* currPhysicsBody = ECS_GetEntityComponent<PhysicsBody2D>( currEntity, ComponentType::PhysicsBody2D );
		if ( !currPhysicsBody )
		{
			continue;
		}

		EntityTransform2D* currTransform = ECS_GetEntityComponent<EntityTransform2D>( currEntity, ComponentType::EntityTransform2D );
		if ( !currTransform )
		{
			continue;
		}

		if ( currPhysicsBody->hasGravity )
		{
			currPhysicsBody->velocity.y = newVelocity( currPhysicsBody->velocity.y, GRAVITY_SCALE, deltaTimeSec );
		}

		// apply velocity to transform
		const Vector2f posDelta = Vec2Math::Scale( currPhysicsBody->velocity, deltaTimeSec );
		currTransform->position = Vec2Math::Sum( posDelta, currTransform->position );
	}


	// for collisions, maybe make a quadtree and at the start of every frame only re-calculate
	// the tree partially by removing any entities that have moved and then re-inserting them.
	// Each entity that moved can be removed from tree in o(1) by making each entity hold a pointer
	// to its current node in the tree. Then insertion would be o(log n). To keep track of movement,
	// maybe can keep track of an entitie's last position in the previous frame? 
	// Might have to make a "OnLatePhysicsFrame" so that I can ensure that the collision logic is 
	// always run after the physics logic? Although I'm not certain how important that is.
	// Upon collision, if they have rigid body 2d components, then use the velocity to see their direction
	// of movement, and push them away in that directon until they are no longer colliding? Or make them bounce
	// by reflecting their velovity? Maybe use axis aligned bounding boxes for broad check with potential
	// sweep check if that is enabled on collider (aka generate a new bounding box based on the AABB and
	// the distance traveled between frames in order to detect collisions that occured in between frames 
	// then calculate the time of collision and resolve the collision there) then if a collision is detected
	// we can use the actual collider bounds to check for an overlap (and for the swept collisions
	// we check this overlap with the actual collider at the position that the colliders would have
	// been at the time of collision.
}
