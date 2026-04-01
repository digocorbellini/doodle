////////////////////////////////////////////////////////////
//
// Wrapper around SFML Sprite that uses ResourceHandle<Texture>
// instead of storing raw texture pointers. The texture pointer
// is resolved and set just before drawing to ensure it remains
// valid and serializable.
//
////////////////////////////////////////////////////////////

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include "core/resource_management/resource_manager.h"

class SpriteRenderer : public sf::Drawable
{
private:
	sf::Sprite sprite;
	ResourceHandle<sf::Texture> textureHandle;

	/// <summary>
	/// Virtual method required by sf::Drawable.
	/// Resolves texture and draws the sprite.
	/// </summary>
	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const override
	{
		sf::Texture* texturePtr = ResourceMannager_GetResource( textureHandle );
		if ( texturePtr )
		{
			const_cast<sf::Sprite&>( sprite ).setTexture( *texturePtr );
			target.draw( sprite, states );
		}
	}

public:
	SpriteRenderer() = default;
	
	/// <summary>
	/// Sets the texture for this sprite via a resource handle.
	/// Does NOT immediately set the SFML sprite's texture pointer.
	/// </summary>
	/// <param name="handle">The resource handle for the texture</param>
	void SetTexture( ResourceHandle<sf::Texture> handle )
	{
		textureHandle = handle;
	}

	/// <summary>
	/// Get the texture resource handle.
	/// </summary>
	/// <returns>The resource handle for this sprite's texture</returns>
	ResourceHandle<sf::Texture> GetTextureHandle() const
	{
		return textureHandle;
	}

	/// <summary>
	/// Set the position of the sprite.
	/// </summary>
	void SetPosition( float x, float y )
	{
		sprite.setPosition( x, y );
	}

	void SetPosition( const sf::Vector2f& position )
	{
		sprite.setPosition( position );
	}

	/// <summary>
	/// Get the position of the sprite.
	/// </summary>
	sf::Vector2f GetPosition() const
	{
		return sprite.getPosition();
	}

	/// <summary>
	/// Set the scale of the sprite.
	/// </summary>
	void SetScale( float scaleX, float scaleY )
	{
		sprite.setScale( scaleX, scaleY );
	}

	void SetScale( const sf::Vector2f& scale )
	{
		sprite.setScale( scale );
	}

	/// <summary>
	/// Get the scale of the sprite.
	/// </summary>
	sf::Vector2f GetScale() const
	{
		return sprite.getScale();
	}

	/// <summary>
	/// Set the rotation of the sprite in degrees.
	/// </summary>
	void SetRotation( float angle )
	{
		sprite.setRotation( angle );
	}

	/// <summary>
	/// Get the rotation of the sprite in degrees.
	/// </summary>
	float GetRotation() const
	{
		return sprite.getRotation();
	}

	/// <summary>
	/// Set the origin (pivot point) of the sprite.
	/// </summary>
	void SetOrigin( float x, float y )
	{
		sprite.setOrigin( x, y );
	}

	/// <summary>
	/// Set the origin (pivot point) of the sprite.
	/// </summary>
	void SetOrigin( const sf::Vector2f& origin )
	{
		sprite.setOrigin( origin );
	}

	/// <summary>
	/// Get the origin (pivot point) of the sprite.
	/// </summary>
	sf::Vector2f GetOrigin() const
	{
		return sprite.getOrigin();
	}

	/// <summary>
	/// Get the underlying SFML sprite (const).
	/// Useful for reading sprite properties that aren't exposed as methods.
	/// </summary>
	const sf::Sprite& GetSprite() const
	{
		return sprite;
	}
};
