#include "com_thread.h"
#include <thread>
#include <shared_mutex>

using namespace std;

static thread::id s_mainThreadID;

static thread::id  s_mainThreadId;
static shared_mutex s_mainThreadMutex;

void Com_SetMainThreadID()
{
	std::unique_lock<std::shared_mutex> lock( s_mainThreadMutex );
	s_mainThreadID = this_thread::get_id();
}


bool Com_IsMainThread()
{
	std::shared_lock<std::shared_mutex> lock( s_mainThreadMutex );
	return this_thread::get_id() == s_mainThreadID;
}
