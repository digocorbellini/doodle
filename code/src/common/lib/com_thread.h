////////////////////////////////////////////////////////////
//
// Common thread helpers
//
////////////////////////////////////////////////////////////


#pragma once
#include "common/lib/com_assert.h"

void Com_SetMainThreadID();
bool Com_IsMainThread();

#define COM_ASSERT_IS_MAIN_THREAD() COM_ASSERT( Com_IsMainThread(), "Called using thread that is not main thread\n" )

