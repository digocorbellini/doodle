////////////////////////////////////////////////////////////
//
// Printf wrapper functions and macros for facilitating
// inclusion of formatting and debug messaging, in
// addition to supporting dev only printing and 
// logging to a file.
//
////////////////////////////////////////////////////////////

#pragma once

/// <summary>
/// Will print the given formatted string into stdout
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
void Com_Printf( const char* fmtStr, ... );

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
/// Will print the given formatted string into stderr with an "ERROR:" prefix
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
void Com_PrintfError( const char* fmt, ... );

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
/// Will print the given formatted string into stdout with an "WARNING:" prefix
/// </summary>
/// <param name="fmtStr">The string to be printed including format</param>
/// <param name="">Additional arguments to be used in print string</param>
void Com_PrintfWarning( const char* fmt, ... );

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