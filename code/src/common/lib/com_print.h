////////////////////////////////////////////////////////////
//
// Printf wrapper functions and macros for facilitating
// inclusion of formatting and debug messaging, in
// addition to supporting dev only printing and 
// logging to a file.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/global_defines.h"

#if USING( DEV_PRINT )
void Com_PrintfImpl( const char* fmtStr, ... );
void Com_PrintfErrorImpl( const char* fmtStr, ... );
void Com_PrintfWarningImpl( const char* fmtStr, ... );

/// <summary>
/// Will print the given formatted string into stdout
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_Printf( fmtStr, ... )                                                               \
    Com_PrintfImpl( fmtStr, ##__VA_ARGS__ )

/// <summary>
/// Will print the given formatted string into stderr with an "ERROR:" prefix
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_PrintfError( fmtStr, ... )                                                          \
    Com_PrintfErrorImpl( fmtStr, ##__VA_ARGS__ )

/// <summary>
/// Will print the given formatted string into stdout with an "WARNING:" prefix
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_PrintfWarning( fmtStr, ... )                                                        \
    Com_PrintfWarningImpl( fmtStr, ##__VA_ARGS__ )

/// <summary>
/// Will print the given formatted string into stdout in the following format:
/// "[systemNameStr]: functionName - formattedStr\n"
/// </summary>
/// <param name="systemNameStr">A string representing the name of the system
/// is printing this formatted string</param>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_PrintfVerbose( systemNameStr, fmtStr, ... ) \
    Com_Printf( "[%s]: %s - " fmtStr "\n", systemNameStr, __FUNCTION__, ##__VA_ARGS__ );

/// <summary>
/// Will print the given formatted string into stderr in the following format:
/// "ERROR: [systemNameStr]: functionName - formattedStr\n"
/// </summary>
/// <param name="systemNameStr">A string representing the name of the system
/// is printing this formatted string</param>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_PrintfErrorVerbose( systemNameStr, fmtStr, ... ) \
    Com_PrintfError( "[%s]: %s - " fmtStr "\n", systemNameStr, __FUNCTION__, ##__VA_ARGS__ );

/// <summary>
/// Will print the given formatted string into stdout in the following format:
/// "WARNING: [systemNameStr]: functionName - formattedStr\n"
/// </summary>
/// <param name="systemNameStr">A string representing the name of the system
/// is printing this formatted string</param>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
#define Com_PrintfWarningVerbose( systemNameStr, fmtStr, ... ) \
    Com_PrintfWarning( "[%s]: %s - " fmtStr "\n", systemNameStr, __FUNCTION__, ##__VA_ARGS__ );

#else // #if USING( DEV_PRINT )
#define Com_Printf( fmtStr, ... )                             (void)0
#define Com_PrintfError( fmtStr, ... )                        (void)0
#define Com_PrintfWarning( fmtStr, ... )                      (void)0
#define Com_PrintfVerbose( systemNameStr, fmtStr, ... )       (void)0
#define Com_PrintfErrorVerbose( systemNameStr, fmtStr, ... )  (void)0
#define Com_PrintfWarningVerbose( systemNameStr, fmtStr, ... )(void)0
#endif