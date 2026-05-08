////////////////////////////////////////////////////////////
//
// Binary heap implementation using static buffer for 
// storage
//
////////////////////////////////////////////////////////////

#pragma once
#include <algorithm>
#include "common/types.h"
#include "common/lib/com_assert.h"

template<typename T, size_t N, typename Compare = std::less<T>>
class FixedHeap
{
private:
	T buffer[N] = {};
	size_t count = 0;
	Compare comp{};
	
public:
	FixedHeap() = default;

	bool Push( const T& value )
	{
		if ( count == N )
		{
			return false;
		}
		
		buffer[count] = value;
		++count;
		if ( count == 1 )
		{
			return true;
		}

		std::push_heap( buffer, buffer + count, comp );
		return true;
	}
	bool Pop()
	{
		if ( count == 0 )
		{
			return false;
		}

		if ( count == 1 )
		{
			count = 0;
			return true;
		}

		std::pop_heap( buffer, buffer + count, comp );
		--count;
		return true;
	}

	const T& Top() const
	{
		COM_ASSERT( count > 0, "[FixedHeap]: attempted to peak at top of heap when heap is empty\n" );
		return buffer[0];
	}

	const size_t Size() const
	{
		return count;
	}

	const size_t Capacity() const
	{
		return N;
	}

	const bool IsFull() const
	{
		return count == N;
	}

	void Clear()
	{
		count = 0;
	}

	const T& operator[]( size_t index ) const
	{
		COM_INDEX_ASSERT( index, N, "[FixedHeap]: attempted to index using invalid index. %zu >= %zu\n", index, N );
		return buffer[index];
	}
};