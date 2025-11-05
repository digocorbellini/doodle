#pragma once
#include <assert.h>
#include <format>
#include <iostream>
#include <stdarg.h>

#define COM_ASSERT( expression, fmt, ... )		\
{												\
	if ( !expression )							\
	{											\
		fprintf( stderr, "ASSERT: " );			\
		fprintf( stderr, fmt, __VA_ARGS__ );	\
		assert( false );						\
	}											\
}

#define COM_STATIC_ASSERT( expression, str ) static_assert( expression, str );

