////////////////////////////////////////////////////////////
//
// Helper functions for performing vector math
//
////////////////////////////////////////////////////////////

#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>

namespace Vec2Math
{
	template<typename T>
	inline sf::Vector2<T> Scale( const sf::Vector2<T>& a, const sf::Vector2<T>& b ) noexcept
	{
		return  sf::Vector2<T>( a.x * b.x, a.y * b.y );
	}

	template<typename T>
	inline sf::Vector2<T> Scale( const sf::Vector2<T>& a, const T scalar ) noexcept
	{
		return  sf::Vector2<T>( a.x * scalar, a.y * scalar );
	}

	template<typename T>
	inline T Magnitude( const sf::Vector2<T>& a ) noexcept
	{
		return std::sqrt( ( a.x * a.x ) + ( a.y * a.y ) );
	}

	template<typename T>
	inline sf::Vector2<T> Normalize( const sf::Vector2<T>& a ) noexcept
	{
		const T magnitude = Magnitude( a );
		if ( magnitude == 0 )
		{
			return sf::Vector2<T>( 0, 0 );
		}
		return sf::Vector2<T>( a.x / magnitude, a.y / magnitude );
	}

	template<typename T>
	inline bool Equals( const sf::Vector2<T>& a, const sf::Vector2<T>& b ) noexcept
	{
		return a.x == b.x && a.y == b.y;
	}

	template<typename T>
	inline T Distance( const sf::Vector2<T>& a, const sf::Vector2<T>& b ) noexcept
	{
		const T xDist = a.x - b.x;
		const T yDist = a.y - b.y;
		return ( xDist * xDist ) + ( yDist * yDist );
	}

	template<typename T>
	inline sf::Vector2<T> Sum( const sf::Vector2<T>& a, const sf::Vector2<T>& b ) noexcept
	{
		return sf::Vector2<T>( a.x + b.x, a.y + b.y );
	}
}