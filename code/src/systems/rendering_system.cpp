#include "core/components.h"
#include "rendering_system.h"

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
	// TODO: cache these in system constructor? Or maybe make an ECS function for getting the desired component given an entity ID
	SpriteRenderer2D* spriteRendererList = ECS_GetComponentList<SpriteRenderer2D>( ComponentType::SpriteRenderer2D );
	EntityTransform2D* entityTransformList = ECS_GetComponentList<EntityTransform2D>( ComponentType::EntityTransform2D );

	for ( EntityIterator itr = entityIterator; itr != entityIterator.end(); ++itr )
	{
		EntityID currEntity = *itr;

		// skip entities that do not have the required components 
		if ( ECS_GetEntityComponentsMask( currEntity ) != RENDERING_SYSTEM_COMP_MASK )
		{
			continue;
		}

		// TODO: yea definitely better to have an ECS function for getting this instead so that I can avoid out of bounds indexes
		SpriteRenderer2D* currRend = &spriteRendererList[currEntity];
		EntityTransform2D* currTransform = &entityTransformList[currEntity];

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