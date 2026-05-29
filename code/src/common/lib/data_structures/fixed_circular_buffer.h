////////////////////////////////////////////////////////////
//
// Circular buffer of fixed size. A circular buffer 
// uses the first in first out concept to add and remove
// data in contiguous segements. Allows for removal
// from a list without requiring shifting.
//
////////////////////////////////////////////////////////////


#pragma once
#include <cstring>
#include <optional>
#include <type_traits>

template<typename T, size_t N>
class FixedCircularBuffer
{
private:
	T m_buff[N];
	size_t m_frontIndex = 0;
	size_t m_backIndex = 0;
	size_t m_size = 0;

	size_t MaxContiguousPopLen() const 
	{
		if ( IsEmpty() )
		{
			return 0;
		}

		// handle cases:
		// One segment :  [           |front==<this is the max pop len>==back|         ]
		// Two segments : [======back|        |front=====<this is the max pop len>=====]
		return ( m_frontIndex < m_backIndex ) ? m_backIndex - m_frontIndex : N - m_frontIndex;
	}

	size_t MaxContiguousPushLen() const
	{
		if ( IsFull() )
		{
			return 0;
		}
		else if ( IsEmpty() )
		{
			return N - m_backIndex;
		}

		// handle cases:
		// One segment :  [           |front=========back| <this is the max push len> ]
		// Two segments : [=========back| <this is the max push len> |front===========]
		return ( m_frontIndex < m_backIndex ) ? N - m_backIndex : m_frontIndex - m_backIndex;
	}
	
public:
	FixedCircularBuffer() = default;

	bool PushBack( T val )
	{
		if ( IsFull() )
		{
			return false;
		}

		m_buff[m_backIndex] = val;
		++m_size;
		m_backIndex = ( m_backIndex + 1 ) % N;
		return true;
	}

	std::optional<T> PopFront()
	{
		if ( IsEmpty() )
		{
			return std::nullopt;
		}

		T val = m_buff[m_frontIndex];
		m_frontIndex = ( m_frontIndex + 1 ) % N;
		--m_size;
		return val;
	}

	size_t BulkPopFront( T* outBuff, const size_t buffSize )
	{
		if ( !outBuff || IsEmpty() )
		{
			return 0;
		}


		// avoid popping more than what's available
		const size_t maxBuffSize = std::min( buffSize, m_size );
		size_t numVals = 0;

		// see if we can use faster memcpy
		if constexpr ( std::is_trivially_copyable_v<T> )
		{
			// Have to handle the following cases
			// One segment : [     |front======back|     ]
			// Two segments : [====back|      |front=====]

			const size_t firstSegmentSize = std::min( maxBuffSize, MaxContiguousPopLen() );
			const size_t secondSegmentSize = maxBuffSize - firstSegmentSize;

			std::memcpy( outBuff, m_buff + m_frontIndex, firstSegmentSize * sizeof( T ) );

			// see if second copy is needed due to wrap around
			if ( secondSegmentSize > 0 )
			{
				std::memcpy( outBuff + firstSegmentSize, m_buff, secondSegmentSize * sizeof( T ) );
			}

			m_size -= maxBuffSize;
			m_frontIndex = ( m_frontIndex + maxBuffSize ) % N;
			numVals = maxBuffSize;
		}
		else
		{
			for ( size_t i = 0; i < maxBuffSize; ++i )
			{
				std::optional<T> val = PopFront();
				if ( !val.has_value() )
				{
					break;
				}

				outBuff[i] = val.value();
				++numVals;
			}
		}

		return numVals;
	}

	bool BulkPushBack( const T* buff, const size_t buffSize )
	{
		if ( !buff || IsFull() || RemainingCapacity() < buffSize )
		{
			return false;
		}

		if ( buffSize == 0 )
		{
			return true;
		}

		// see if we can use faster memcpy
		if constexpr ( std::is_trivially_copyable_v<T> )
		{
			// Have to handle the following cases
			// One segment : [     |front======back|     ]
			// Two segments : [====back|      |front=====]

			const size_t firstSegmentSize = std::min( buffSize, MaxContiguousPushLen() );
			const size_t secondSegmentSize = buffSize - firstSegmentSize;

			std::memcpy( m_buff + m_backIndex, buff, firstSegmentSize * sizeof( T ) );

			// see if second copy is needed due to wrap around
			if ( secondSegmentSize > 0 )
			{
				std::memcpy( m_buff, buff + firstSegmentSize, secondSegmentSize * sizeof( T ) );
			}

			m_size += buffSize;
			m_backIndex = ( m_backIndex + buffSize ) % N;
		}
		else
		{
			for ( size_t i = 0; i < buffSize; ++i )
			{
				if ( !PushBack( buff[i] ) )
				{
					return false;
				}
			}
		}

		return true;
	}

	size_t RemoveFront( const size_t count )
	{
		if ( IsEmpty() )
		{
			return 0;
		}

		const size_t removeCount = std::min( count, m_size );
		m_frontIndex = ( m_frontIndex + removeCount ) % N;
		m_size -= removeCount;
		return removeCount;
	}

	size_t Size() const
	{
		return m_size;
	}

	size_t Capacity() const
	{
		return N;
	}

	size_t RemainingCapacity() const
	{
		return N - m_size;
	}

	bool IsFull() const
	{
		return m_size == N;
	}

	bool IsEmpty() const
	{
		return m_size == 0;
	}

	void Clear()
	{
		m_frontIndex = 0;
		m_backIndex = 0;
		m_size = 0;
	}

	std::optional<T> operator[]( const size_t index ) const
	{
		if ( index >= m_size )
		{
			return std::nullopt;
		}

		const size_t indexToUse = ( m_frontIndex + index ) % N;
		return m_buff[indexToUse];
	}
};


// TODO: create a char buffer variant which holds some string helper methods (Ex: substring)