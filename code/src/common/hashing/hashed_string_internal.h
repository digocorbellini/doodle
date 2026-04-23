////////////////////////////////////////////////////////////
//
// Internal helper functions for hashed strings. None 
// of the functions in this file should be included or 
// used anywhere else.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/types.h"
#include "common/global_defines.h"


namespace hashed_string
{
	constexpr std::size_t HASHED_STRING_MAX_LENGTH = 64ull;

#if IS_ENABLED( HASHED_STRING_CACHING )
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
#endif // #if IS_ENABLED( HASHED_STRING_CACHING )
}