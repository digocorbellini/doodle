////////////////////////////////////////////////////////////
//
// This file is meant to contain all globally used defined
// values
//
////////////////////////////////////////////////////////////

#pragma once
// This allows the use of #if USING( X ) to fail at compile time if X has not been defined instead
// of failing silently.
#define IN_USE &&
#define NOT_IN_USE &&!
#define USING(x) (true x true)
#define USE_IF(x) && x &&

#define DEV_BUILD IN_USE

#define HASHED_STRING_CACHING USE_IF( USING( DEV_BUILD ) )