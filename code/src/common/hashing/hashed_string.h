////////////////////////////////////////////////////////////
//
// HashedString is meant to be used to facilitate the
// hash look up of strings while also obfuscating 
// the original strings in memory in ship builds.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/lib/com_assert.h"
#include "common/global_defines.h"
#include "common/types.h"
#include "hash.h"
#include "hashed_string_internal.h"


class HashedString
{
	std::uint64_t hash;

public:
	// Compile-time hashing
	template <std::size_t N>
	constexpr HashedString( const char( &str )[N] ) noexcept
	{
		static_assert( N <= hashed_string::HASHED_STRING_MAX_LENGTH, "Invalid string length in static hashed string constructor. Can not be hashed." );
		hash = FNV1A_64_Hash( str, N );
#if IS_ENABLED( HASHED_STRING_CACHING )
		hashed_string::CacheStringHash( str, hash );
#endif // #if IS_ENABLED( HASHED_STRING_CACHING )
	}

	// Runtime hashing
	HashedString( const char* str, const std::size_t strSize ) noexcept;

	constexpr HashedString( const std::uint64_t hashVal ) noexcept : hash( hashVal ) {}
	constexpr HashedString( const HashedString& other ) noexcept : hash( other.hash ) {}

	constexpr std::uint64_t GetHash() noexcept
	{
		return hash;
	}

	const char* GetStringForHash();

	inline bool operator==( const HashedString& other ) const
	{
		return hash == other.hash;
	}

	inline bool operator!=( const HashedString& other ) const
	{
		return hash != other.hash;
	}

	inline bool operator==( const std::uint64_t& otherHash ) const
	{
		return hash == otherHash;
	}

	inline bool operator!=( const std::uint64_t& otherHash ) const
	{
		return hash != otherHash;
	}
};


constexpr HashedString INVALID_HASHED_STRING = { 0 };

