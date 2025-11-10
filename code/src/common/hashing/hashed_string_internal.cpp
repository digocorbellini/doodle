#include "common/global_defines.h"

#if USING( HASHED_STRING_CACHING )
#include "common/lib/com_array.h"
#include "common/lib/com_assert.h"
#include "hashed_string_internal.h"
#include <inttypes.h>
#include <iostream>
#include "string.h"
#include <unordered_map>

using namespace std;
using namespace hashed_string;

static constexpr size_t MAX_CACHED_STRING_COUNT = ( 1 << 15 );

struct CachedHashString
{
	uint64_t hash = 0ull;
	char str[HASHED_STRING_MAX_LENGTH];
};

// TODO: this is pretty intense memory wise, so maybe it is better to have a static buffer
// and then blocks are allocated which perfectly fit the given string lengths instead of 
// making all strings 64 bytes long. Although this feature should be dev only... so maybe that's ok?
// linear probing hash map
static CachedHashString s_cachedHashStringMap[MAX_CACHED_STRING_COUNT];


// =================================
// Private Helpers
// =================================

bool AddStringHashToCache( const char* str, const uint64_t hash )
{
	// find an open index to cache the given hash and string pair
	const size_t hashIndex = hash % MAX_CACHED_STRING_COUNT;
	CachedHashString* newHashStringIndex = nullptr;
	for ( size_t i = 0; i < MAX_CACHED_STRING_COUNT; ++i )
	{
		const size_t currHashIndex = ( hashIndex + i ) % MAX_CACHED_STRING_COUNT;
		CachedHashString* currHashStringIndex = &s_cachedHashStringMap[currHashIndex];
		const uint64_t currHash = currHashStringIndex->hash;
		if ( currHash == 0 )
		{
			newHashStringIndex = currHashStringIndex;
			break;
		}
		else if ( currHash == hash )
		{
			const char* currStr = currHashStringIndex->str;
			// true if hash already exists for this string, and false if hash 
			// already exists for another string which means a hash collision has occured
			return strncmp( str, currStr, HASHED_STRING_MAX_LENGTH ) == 0;
		}
	}

	// see if no open index was found
	if ( !newHashStringIndex )
	{
		return false;
	}

	// cache str and hash pair at empty index
	newHashStringIndex->hash = hash;
	strncpy_s( newHashStringIndex->str, str, HASHED_STRING_MAX_LENGTH );
	return true;
}

// TODO: see if I need to support removing cached string. Probably not since I want 
// to be able to catch all duplicate hashes possible in dev.


// =================================
// Public Functions
// =================================

void hashed_string::CacheStringHash( const char* str, const uint64_t hash )
{
	const char* hashToString = GetCachedStringForHash( hash );
	if ( hashToString )
	{
		// check for duplicate hashes
		if ( strncmp( str, hashToString, HASHED_STRING_MAX_LENGTH ) != 0 )
		{
			COM_ASSERT( false, "%s - duplicate hash [%" PRIu64 "] found for strings '%s' and '%s'. Can be resolved by renaming either string.\n", __FUNCTION__, hash, str, hashToString );
			return;
		}
		else
		{
			// this hash/string kvp has already been cached
			return;
		}
	}

	COM_VERIFY_TRUE( AddStringHashToCache( str, hash ), "%s - unable to add hash/string kvp ('%s', %" PRIu64 ") to cache. Might need to bump max hashed strings count. Current max: %zu\n", __FUNCTION__, str, hash, MAX_CACHED_STRING_COUNT );
}


const char* hashed_string::GetCachedStringForHash( const uint64_t hash )
{
	const size_t hashIndex = hash % MAX_CACHED_STRING_COUNT;
	for ( size_t i = 0; i < MAX_CACHED_STRING_COUNT; ++i )
	{
		const size_t currHashIndex = ( hashIndex + i ) % MAX_CACHED_STRING_COUNT;
		CachedHashString* currHashStringIndex = &s_cachedHashStringMap[currHashIndex];
		const uint64_t currHash = currHashStringIndex->hash;
		if ( currHash == hash )
		{
			return currHashStringIndex->str;
		}
	}

	return nullptr;
}
#endif // #if USING( HASHED_STRING_CACHING )