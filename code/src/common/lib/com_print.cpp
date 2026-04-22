#include "com_print.h"

#if IS_ENABLED( DEV_PRINT )
#include <cstdarg>
#include <iostream>

using namespace std;

// TODO: add support for also printing everything to a log file potentially with ms time stamps

// ===============================
// Private Internal Helpers
// ===============================

static void InternalPrintfImplementation( FILE* const stream, const char* prefixStr, const char* fmtStr, va_list args )
{
	if ( prefixStr )
	{
		fputs( prefixStr, stream );
	}
	vfprintf( stream, fmtStr, args );
}


// ===============================
// Public Functions
// ===============================

void Com_PrintfImpl( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stdout, nullptr, fmtStr, args );
	va_end( args );
}


void Com_PrintfErrorImpl( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stderr, "ERROR: ", fmtStr, args);
	va_end( args );
}


void Com_PrintfWarningImpl( const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	InternalPrintfImplementation( stdout, "WARNING: ", fmtStr, args );
	va_end( args );
}
#endif // #if IS_ENABLED( DEV_PRINT )
