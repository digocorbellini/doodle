#include "common/lib/com_assert.h"
#include "hashed_string_internal.h"
#include <unordered_map>
#include "string.h"
#include <iostream>
#include "common/lib/com_array.h"

// TODO: eventually wrap this in dev only defines so that the caching and the use of 
// strings for hashes does not exist in ship

using namespace std;
using namespace hashed_string;

static constexpr size_t MAX_CACHED_STRING_COUNT = 512;

// TODO: instead of maintaining an unordered map where we can not
// maintain a static buffer and will need dynamic memory allocation, 
// perhaps it would be best to maintain a minheap and then use binary search
// to find cached values. So both insertion and search will be O(logn). Which is 
// better than a simple array with a linear search but worse than a map with O(1)

// TODO: for now just do a simple linear search for cached hashes
static char s_cachedStrings[MAX_CACHED_STRING_COUNT][HASHED_STRING_MAX_LENGTH];
static uint64_t s_cachedHashes[MAX_CACHED_STRING_COUNT];
static size_t s_cachedStringHashLength = 0;
static_assert( ARRAY_SIZE( s_cachedStrings ) == ARRAY_SIZE( s_cachedHashes ), "Cached strings and cached hash must be the same length" );


// =================================
// Public Functions
// =================================

void hashed_string::CacheStringHash( const char* str, const uint64_t hash )
{
	COM_ASSERT( s_cachedStringHashLength < MAX_CACHED_STRING_COUNT - 1, "%s - exceeded max cached string count: %zu > %zu\n", __FUNCTION__, s_cachedStringHashLength, MAX_CACHED_STRING_COUNT );
	
	const char* hashToString = hashed_string::GetCachedStringForHash( hash );
	if ( hashToString )
	{
		// check for duplicate hashes
		if ( strncmp( str, hashToString, HASHED_STRING_MAX_LENGTH ) != 0 )
		{
			COM_ASSERT( false, "%s - duplicate hash found for strings '%s' and '%s'. Can be resolved by renaming either string.\n", __FUNCTION__, str, hashToString );
		}
		else
		{
			return;
		}
	}

	++s_cachedStringHashLength;
	strncpy_s( s_cachedStrings[s_cachedStringHashLength], str, HASHED_STRING_MAX_LENGTH );
	s_cachedHashes[s_cachedStringHashLength] = hash;
}


const char* hashed_string::GetCachedStringForHash( const uint64_t hash )
{
	for ( int i = 0; i < MAX_CACHED_STRING_COUNT; ++i )
	{
		if ( s_cachedHashes[i] == hash )
		{
			return s_cachedStrings[i];
		}
	}

	return nullptr;
}
