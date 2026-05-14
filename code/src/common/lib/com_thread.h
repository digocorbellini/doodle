////////////////////////////////////////////////////////////
//
// Common thread helpers
//
////////////////////////////////////////////////////////////


#pragma once
#include "common/lib/com_assert.h"
#include <condition_variable>
#include <mutex>

void Com_SetMainThreadID();
bool Com_IsMainThread();

#define COM_ASSERT_IS_MAIN_THREAD() COM_ASSERT( Com_IsMainThread(), "Called using thread that is not main thread\n" )

////////////////////////////////////////////////////////////
//
// Generic thread-safe data wrapper using a mutex and
// condition variable.
//
// Three operations:
//
//   Access    — lock, read or write, unlock.
//               Use when no thread ever sleeps on this data.
//
//   Modify    — lock, write, unlock, then notify_all.
//               Use when another thread may be sleeping via WaitUntil.
//               Notify fires AFTER the lock is released so woken threads
//               can immediately acquire the lock without double-blocking.
//
//   WaitUntil — sleep until predicate(data) is true, then run a function.
//               The predicate is re-checked on every wakeup to guard
//               against spurious wakeups (condition variables can wake
//               without being explicitly notified — this is a known
//               behaviour of the standard and must always be handled).
//               Only useful on data that is written via Modify. If data
//               is only ever written with Access this will sleep forever.
//
////////////////////////////////////////////////////////////
namespace monitor_impl
{
    template <typename ParamType, typename PredicateType>
    concept IsPredicateFunctionPtr = std::is_same_v<PredicateType, bool( * )( ParamType )>;
}
template<typename T>
class Monitor
{
private:
    T m_data;
    std::mutex m_mutex;
    std::condition_variable m_cv;

public:    
    // Access data exclusively. No waiting, no notify.
    template<typename Func>
    auto Access( Func&& func )
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        return func( m_data );
    }

    // Modify data and wake all threads sleeping in WaitUntil.
    // Notify happens after the lock is released — not before —
    // so woken threads can acquire the lock without immediately
    // blocking again.
    template<typename Func>
    auto Modify( Func&& func )
    {
        {
            std::unique_lock<std::mutex> lock( m_mutex );
            func( m_data );
        }                   // lock released here
        m_cv.notify_all();  // notify after release
    }

    // Sleep until predicate(data) returns true, then run func under the lock.
    // Re-checks predicate on every wakeup to handle spurious wakeups.
    template<monitor_impl::IsPredicateFunctionPtr<T> Predicate, typename Func>
    auto WaitUntil( Predicate&& predicate, Func&& func )
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        m_cv.wait( lock, [&]()
                   {
                       return predicate( m_data );
                   } );
        return func( m_data );
    }
};

