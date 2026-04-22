#include "hashed_string.h"

using namespace std;
using namespace hashed_string;

HashedString::HashedString( const char* str, const size_t strSize ) noexcept
{
	COM_ASSERT( strSize <= HASHED_STRING_MAX_LENGTH, "%s - Invalid string length (%zu > %zu). Can not be hashed.\n",
					   __FUNCTION__, strSize, HASHED_STRING_MAX_LENGTH );

	hash = hashed_string::FNV1A_64_Hash( str, strSize );
#if IS_ENABLED( HASHED_STRING_CACHING )
	hashed_string::CacheStringHash( str, hash );
#endif // #if IS_ENABLED( HASHED_STRING_CACHING )
}


const char* HashedString::GetStringForHash()
{
#if IS_ENABLED( HASHED_STRING_CACHING )
	return GetCachedStringForHash( hash );
#else // #if IS_ENABLED( HASHED_STRING_CACHING )
	return nullptr;
#endif // #else // #if IS_ENABLED( HASHED_STRING_CACHING )
}