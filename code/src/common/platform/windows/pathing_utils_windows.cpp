#include "common/global_defines.h"

#if IS_ENABLED( PLATFORM_WINDOWS )
#include "common/platform/pathing_utils.h"
#include <stdlib.h>


bool PathingUtils_GetAbsolutePath( const char* relativePath, char* outBuff, const size_t outBuffSize )
{
	return _fullpath( outBuff, relativePath, outBuffSize) != nullptr;
}
#endif // #if IS_ENABLED( PLATFORM_WINDOWS )
