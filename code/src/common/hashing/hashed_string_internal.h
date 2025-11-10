#pragma once
#include "common/types.h"
#include "common/global_defines.h"

// TODO: need some way to detect collisions and then to assert when it occurs. Must catch the case of both
// dynamically hashed and statically hashed strings somehow...

namespace hashed_string
{
	constexpr std::size_t HASHED_STRING_MAX_LENGTH = 64ull;

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

#if USING( HASHED_STRING_CACHING )
	/// <summary>
	/// Cache the given string to hash mapping. Will assert if we have reached the max
	/// number of hashed strings or if the given string has a hash that already exists
	/// for another string.
	/// </summary>
	/// <param name="str">The string used to create the given hash</param>
	/// <param name="hash">The hash value for the given string</param>
	void CacheStringHash( const char* str, const std::uint64_t hash );


	/// <summary>
	/// Get the corresponding mapped string for the given hash.
	/// </summary>
	/// <param name="hash">The hash for which to get the cached string for</param>
	/// <returns>The string which is mapped tot he given hash. If no string exists
	/// for the hash, then return nullptr.</returns>
	const char* GetCachedStringForHash( const std::uint64_t hash );
#endif // #if USING( HASHED_STRING_CACHING )
}