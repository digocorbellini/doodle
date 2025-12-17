#include "com_print.h"
#include <cstdarg>
#include <iostream>

using namespace std;


// ===============================
// Private Internal Helpers
// ===============================

static void InternalPrintfImplementation( FILE* const stream, const char* prefixStr, const char* fmtStr, ... )
{
	va_list args;
	va_start( args, fmtStr );
	if ( prefixStr )
	{
		fputs( prefixStr, stream );
	}
	vfprintf( stream, fmtStr, args );
	va_end( args );
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

