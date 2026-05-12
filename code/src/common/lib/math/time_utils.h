////////////////////////////////////////////////////////////
//
// Helpful time operations and conversions
//
////////////////////////////////////////////////////////////


#pragma once
#include "common/types.h"

template<typename T>
constexpr T NanoToSeconds( const NanoSeconds ns ) noexcept
{
	return static_cast<T>( ns * 1e-9 );
}