#include "core/components.h"
#include "rendering_system.h"

using namespace sf;

static const ComponentsMask RENDERING_SYSTEM_COMP_MASK( { ComponentType::SpriteRenderer2D, ComponentType::EntityTransform2D } );

static RenderingSystem s_renderingSystem;


// ==================
// Private Helpers
// ==================

void CenterOrigin( SpriteRenderer* spriteRend )
{
	const Texture* texture = spriteRend->GetTexture();
	spriteRend->SetOrigin( texture->getSize().x / 2, texture->getSize().y / 2 );
}


// ==================
// Public Methods
// ==================

void RenderingSystem::OnDrawFrame( RenderWindow* window, EntityIterator entityIterator )
{
	for ( EntityIterator itr = entityIterator; itr != entityIterator.end(); ++itr )
	{
		EntityID currEntity = *itr;

		// skip entities that do not have the required components 
		if ( ECS_GetEntityComponentsMask( currEntity ) != RENDERING_SYSTEM_COMP_MASK )
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

		// TODO: eventually have to take into account x and y flipping and rendering order but for now lets just get it on the screen somehow
		if ( currRend->hasCenteredOrigin )
		{
			CenterOrigin( &currRend->sprite );
		}
		else
		{
			currRend->sprite.SetOrigin( 0, 0 );
		}
		// TODO: potentially set origin offset
		currRend->sprite.SetScale( currTransform->scale );
		currRend->sprite.SetPosition( currTransform->position );
		window->draw( currRend->sprite );
	}
}