#include "common/global_defines.h"

#if IS_ENABLED( PLATFORM_WINDOWS )
#include "common/platform/com_thread.h"
#include <Windows.h>

// =====================
// Public Functions
// =====================

void ComThread_SleepMs( uint32_t miliseconds )
{
	Sleep( static_cast<DWORD>( miliseconds ) );
}
#endif // #if IS_ENABLED( PLATFORM_WINDOWS )
