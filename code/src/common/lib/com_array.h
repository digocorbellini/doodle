////////////////////////////////////////////////////////////
//
// Array helper functions and macros
//
////////////////////////////////////////////////////////////

#pragma once

// Gotten from: https://randomascii.wordpress.com/2011/09/13/analyze-for-visual-studio-the-ugly-part-5/
template <typename T, size_t N>
char ( *MyNumberOf( T( & )[N] ) )[N];
#define INTERNAL_ARRAY_SIZE(arr) (sizeof(*MyNumberOf(arr))+0)

#define ARRAY_SIZE(arr) INTERNAL_ARRAY_SIZE(arr)