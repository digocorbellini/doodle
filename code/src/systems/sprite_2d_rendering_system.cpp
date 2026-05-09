#include "core/components.h"
#include "common/lib/data_structures/fixed_skip_list.h"
#include "common/lib/com_print.h"
#include "common/lib/com_string.h"
#include "common/lib/math/vector_math.h"
#include "sprite_2d_rendering_system.h"

using namespace sf;

struct RenderingOrderComp
{
	constexpr bool operator()(const SpriteRenderer2D * left, const SpriteRenderer2D * right) const
	{
		return left->renderingOrder < right->renderingOrder;
	}
};

static const char* RENDERING_SYSTEM_NAME = OBFUSCATED_STRING( "Sprite2DRenderingSystem" );

static const ComponentsMask RENDERING_SYSTEM_COMP_MASK( { ComponentType::SpriteRenderer2D, ComponentType::EntityTransform2D } );
static FixedSkipList<SpriteRenderer2D*, MAX_ENTITIES, 6, RenderingOrderComp> s_sortedRenderingOrder;
static Sprite2DRenderingSystem s_renderingSystem;

// ==================
// Private Helpers
// ==================

void CenterOrigin( SpriteRenderer* spriteRend )
{
	const ResourcePtr<Texture> textureResource = spriteRend->GetTextureResourcePtr();
	spriteRend->SetOrigin( textureResource->getSize().x / 2.0f, textureResource->getSize().y / 2.0f );
}


// ==================
// Public Methods
// ==================

void Sprite2DRenderingSystem::OnDrawFrame( RenderWindow* window, EntityIterator entityIterator )
{
	for ( EntityIterator itr = entityIterator; itr != entityIterator.end(); ++itr )
	{
		EntityID currEntity = *itr;

		// skip entities that do not have the required components 
		if ( !ECS_GetEntityComponentsMask( currEntity ).ContainsMask( RENDERING_SYSTEM_COMP_MASK ) )
		{
			continue;
		}

		SpriteRenderer2D* currRend = ECS_GetEntityComponent<SpriteRenderer2D>( currEntity, ComponentType::SpriteRenderer2D );
		if ( !currRend )
		{
			continue;
		}

		EntityTransform2D* currTransform = ECS_GetEntityComponent<EntityTransform2D>( currEntity, ComponentType::EntityTransform2D );
		if ( !currTransform )
		{
			continue;
		}

		if ( currRend->hasCenteredOrigin )
		{
			CenterOrigin( &currRend->sprite );
		}
		else
		{
			currRend->sprite.SetOrigin( 0, 0 );
		}

		// apply scale and flip sprite X or Y
		const Vector2f flippingVector = Vector2f( currRend->isXFlipped ? -1 : 1, currRend->isYFlipped ? -1 : 1 );
		currRend->sprite.SetScale( Vec2Math::Scale( currTransform->scale, flippingVector ) );

		// apply sprite position in world
		currRend->sprite.SetPosition( currTransform->position );

		// add sprite to rendering queue to draw later
		if ( !s_sortedRenderingOrder.Insert( currRend ) )
		{
			COM_ALWAYS_ASSERT( "[%s]: exceeded rendering order buffer size during draw frame. Max buffer size: %zu\n", RENDERING_SYSTEM_NAME, s_sortedRenderingOrder.Capacity() );
			break;
		}
	}

	// draw sprites following rendering order (higher order means draw later)
	for ( auto itr = s_sortedRenderingOrder.begin(); itr != s_sortedRenderingOrder.end(); ++itr )
	{
		const SpriteRenderer2D* currSpriteRend = *itr;
		window->draw( currSpriteRend->sprite );
	}
	s_sortedRenderingOrder.Clear();
}