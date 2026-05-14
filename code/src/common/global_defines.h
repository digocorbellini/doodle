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

// ==================
// Platform Defines
// ==================

#define PLATFORM_WINDOWS DISABLED
#define PLATFORM_POSIX DISABLED

// only flip one platform to enabled
#if defined(_WIN32) 
#undef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS ENABLED
// Ensure we are building 64-bit
#ifndef _WIN64
#error "32-bit builds are not supported"
#endif
// example of another platform definition but currently don't support anything other than
// windows
#elif defined(__linux__) || defined(__APPLE__)
#undef PLATFORM_POSIX
#define PLATFORM_POSIX ENABLED
#else
#error "Unknown platform"
#endif

// ==================
// Engine Defines
// ==================

#define DEV_BUILD ENABLED

#define DEV_PRINT REQUIRES( IS_ENABLED( DEV_BUILD ) )
#define DEV_ASSERTS REQUIRES( IS_ENABLED( DEV_BUILD ) )
#define HASHED_STRING_CACHING REQUIRES( IS_ENABLED( DEV_BUILD ) )

#if IS_ENABLED( DEV_BUILD )
#define GAME_DIR_PATH "../../../game/"
#else //#if IS_ENABLED( DEV_BUILD )
#define GAME_DIR_PATH "./game/"
#endif // #elif //#if IS_ENABLED( DEV_BUILD )
