////////////////////////////////////////////////////////////
//
// Hashing function definitions
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/types.h"

constexpr std::uint64_t FNV_PRIME = 1099511628211ull;
constexpr std::uint64_t FNV_OFFSET_BASIS = 14695981039346656037ull;
// TODO: can eventually add salts for different hashes (Ex: if we want to have a
// "HashedAssetString" or something different to further prevent collisions depending 
// on the usage.

/// <summary>
/// Perform Fowler-Noll-Vo Hash Function 64 bit variant
/// </summary>
/// <param name="str">The string to be hashed</param>
/// <param name="strLen">The length of the string to be hashed</param>
/// <returns>A 64 bit hash for the given string</returns>
constexpr std::uint64_t FNV1A_64_Hash( const char* str, const std::size_t strLen ) noexcept
{
	std::uint64_t hash = FNV_OFFSET_BASIS;

	for ( std::size_t i = 0ull; i < strLen; ++i )
	{
		hash = ( hash ^ static_cast<unsigned char>( str[i] ) ) * FNV_PRIME;
	}

	return hash;
}
