#include "common/global_defines.h"

#if IS_ENABLED( PLATFORM_WINDOWS )
#include "common/platform/platform_thread.h"
#include <Windows.h>

// =====================
// Public Functions
// =====================

void Platform_SleepMs( uint32_t miliseconds )
{
	Sleep( static_cast<DWORD>( miliseconds ) );
}
#endif // #if IS_ENABLED( PLATFORM_WINDOWS )
