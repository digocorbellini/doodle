////////////////////////////////////////////////////////////
//
// This file is meant to contain all globally used defined
// values
//
////////////////////////////////////////////////////////////

#pragma once

// This allows the use of #if IS_ENABLED( X ) to fail at compile time if X has not been defined instead
// of failing silently.
#define ENABLED &&
#define DISABLED &&!
#define IS_ENABLED(x) (true x true)
#define REQUIRES(x) && x &&

#define DEV_BUILD ENABLED

#define DEV_PRINT REQUIRES( IS_ENABLED( DEV_BUILD ) )
#define DEV_ASSERTS REQUIRES( IS_ENABLED( DEV_BUILD ) )
#define HASHED_STRING_CACHING REQUIRES( IS_ENABLED( DEV_BUILD ) )

#if IS_ENABLED( DEV_BUILD )
#define GAME_DIR_PATH "../../../game/"
#else //#if IS_ENABLED( DEV_BUILD )
#define GAME_DIR_PATH "./game/"
#endif // #elif //#if IS_ENABLED( DEV_BUILD )
