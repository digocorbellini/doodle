////////////////////////////////////////////////////////////
//
// Assert wrappers for facilitating asserting and for 
// debug printing in dev only.
//
////////////////////////////////////////////////////////////

#pragma once
#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include "common/global_defines.h"

#if USING( DEV_BUILD )
#define COM_ASSERT( expression, fmt, ... )		\
{												\
	if ( !(expression) )						\
	{											\
		fprintf( stderr, "ASSERT: " );			\
		fprintf( stderr, fmt, __VA_ARGS__ );	\
		assert( false );						\
	}											\
}
#else // #if USING( DEV_BUILD )
#define COM_ASSERT( expression, fmt, ... ) (void)0
#endif // #else // #if USING( DEV_BUILD )

#define COM_VERIFY_TRUE( expression, fmt, ... ) COM_ASSERT( expression, fmt, __VA_ARGS__ )
#define COM_VERIFY_FALSE( expression, fmt, ... ) COM_ASSERT( !(expression), fmt, __VA_ARGS__ )

#define COM_ALWAYS_ASSERT( fmt, ... ) COM_ASSERT( false, fmt, __VA_ARGS__ )