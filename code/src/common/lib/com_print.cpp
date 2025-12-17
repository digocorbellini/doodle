#include "com_print.h"
#include <cstdarg>
#include "common/global_defines.h"
#include <iostream>

using namespace std;

// TODO: add support for also printing everything to a log file

// ===============================
// Private Internal Helpers
// ===============================

static void InternalPrintfImplementation( FILE* const stream, const char* prefixStr, const char* fmtStr, va_list args )
{
#if USING( DEV_PRINT )
	if ( prefixStr )
	{
		fputs( prefixStr, stream );
	}
	vfprintf( stream, fmtStr, args );
#endif // #if USING( DEV_PRINT )
}


// ===============================
// Public Functions
// ===============================

void Com_Printf( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stdout, nullptr, fmtStr, args );
	va_end( args );
}


void Com_PrintfError( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stderr, "ERROR: ", fmtStr, args);
	va_end( args );
}


void Com_PrintfWarning( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stdout, "WARNING: ", fmtStr, args );
	va_end( args );
}

