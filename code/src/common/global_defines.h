////////////////////////////////////////////////////////////
//
// This file is meant to contain all globally used defined
// values
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/lib/com_string.h"

// This allows the use of #if USING( X ) to fail at compile time if X has not been defined instead
// of failing silently.
#define IN_USE &&
#define NOT_IN_USE &&!
#define USING(x) (true x true)
#define USE_IF(x) && x &&

#define DEV_BUILD IN_USE

#define DEV_PRINT USE_IF( USING( DEV_BUILD ) )
#define DEV_ASSERTS USE_IF( USING( DEV_BUILD ) )
#define HASHED_STRING_CACHING USE_IF( USING( DEV_BUILD ) )

#if USING( DEV_BUILD )
#define GAME_DIR_PATH OBFUSCATED_STRING( "../../../game/" )
#elif //#if USING( DEV_BUILD )
#defiine GAME_DIR_PATH OBFUSCATED_STRING( "./game/" )
#endif // #elif //#if USING( DEV_BUILD )
