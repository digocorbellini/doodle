#pragma once
#include <assert.h>
#include "common/global_defines.h"
#include "common/types.h"
#include "HashedString_Internal.h"

constexpr std::size_t HASHED_STRING_MAX_LENGTH = 64ull;

class HashedString
{
	std::uint64_t hash;
#if USING( DEV_BUILD )
	char str[HASHED_STRING_MAX_LENGTH];
#endif // #if USING( DEV_BUILD )

public:
	// Compile-time hashing
	template <std::size_t N>
	constexpr HashedString( const char( &str )[N] ) noexcept
	{
		static_assert( N <= HASHED_STRING_MAX_LENGTH, "Invalid string length. Can not be hashed." );
		hash = hashed_string::fnv1a_64_hash( str, N );
#if USING( DEV_BUILD )
		hashed_string::constexpr_strncpy( this->str, str, HASHED_STRING_MAX_LENGTH );
#endif // #if USING( DEV_BUILD )
	}

	constexpr HashedString( const std::uint64_t hashVal ) noexcept
		: hash( hashVal )
#if USING( DEV_BUILD )

		, str{ 0 }
#endif // #if USING( DEV_BUILD )
	{
#if USING( DEV_BUILD )

		// TODO: figure out how to translate hash val to store in str
#endif // #if USING( DEV_BUILD )
	}

	constexpr HashedString( const HashedString& other ) noexcept
		: hash( other.hash )
#if USING( DEV_BUILD )
		, str{ 0 }
#endif // #if USING( DEV_BUILD )
	{
#if USING( DEV_BUILD )
		hashed_string::constexpr_strncpy( this->str, str, HASHED_STRING_MAX_LENGTH );
#endif // #if USING( DEV_BUILD )
	}

	constexpr std::uint64_t GetHash() noexcept
	{
		return hash;
	}

#if USING( DEV_BUILD )
	constexpr const char* GetDebugString() noexcept
	{
		return str;
	}
#endif // #if USING( DEV_BUILD )
};

constexpr HashedString INVALID_HASHED_STRING = { 0 };

