////////////////////////////////////////////////////////////
//
// This file is meant to contain all custom typedefs for 
// commonly used variable types. 
//
////////////////////////////////////////////////////////////

#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>

typedef std::uint8_t Byte;
typedef std::uint64_t Entity_t;

template <typename T>
constexpr std::underlying_type_t<T> GetUndelyingEnumVal( T enumVal )
{
	static_assert( std::is_enum_v<T>, "The given value is not of type Enum" );
	return static_cast<std::underlying_type_t<T>>( enumVal );
}