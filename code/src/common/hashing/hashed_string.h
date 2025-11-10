#pragma once
#include "common/lib/com_assert.h"
#include "common/global_defines.h"
#include "common/types.h"
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
		hash = hashed_string::FNV1A_64_Hash( str, N );
#if USING( HASHED_STRING_CACHING )
		hashed_string::CacheStringHash( str, hash );
#endif // #if USING( HASHED_STRING_CACHING )
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
};


constexpr HashedString INVALID_HASHED_STRING = { 0 };

