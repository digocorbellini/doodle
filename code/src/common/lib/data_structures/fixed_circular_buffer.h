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
protected:
	struct RawSegments
	{
		const T* segment1 = nullptr;
		size_t segment1Size = 0;

		const T* segment2 = nullptr;
		size_t segment2Size = 0;
	};

	T m_buff[N];
	size_t m_frontIndex = 0;
	size_t m_backIndex = 0;
	size_t m_size = 0;

	RawSegments GetRawAllocatedSegments() const
	{
		RawSegments segments;
		if ( IsEmpty() )
		{
			return segments;
		}

		segments.segment1 = m_buff + m_frontIndex;
		segments.segment1Size = MaxContiguousPopLen();

		// see if second segment exists
		if ( segments.segment1Size < m_size )
		{
			segments.segment2 = m_buff;
			segments.segment2Size = m_size - segments.segment1Size;
		}

		return segments;
	}

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

	size_t RealIndexToCircularIndex( const size_t realIndex ) const
	{
		return ( realIndex >= m_frontIndex ) ? realIndex - m_frontIndex : ( N - m_frontIndex + realIndex );
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

	size_t CopyRange( T* outBuff, const size_t startOffset, const size_t count ) const
	{
		if ( !outBuff || startOffset >= m_size || count == 0 )
		{
			return 0;
		}

		// truncate copy if attempting to copy more than size
		const size_t copyCount = std::min( count, m_size - startOffset );

		// see ifwe can use faster memcpy
		if constexpr ( std::is_trivially_copyable_v<T> )
		{
			const size_t startIndex = ( m_frontIndex + startOffset ) % N;

			const size_t maxPopSize = ( startIndex < m_backIndex ) ? m_backIndex - startIndex : N - startIndex;
			const size_t firstSegmentSize = std::min( copyCount, maxPopSize );
			const size_t secondSegmentSize = copyCount - firstSegmentSize;

			std::memcpy( outBuff, m_buff + startIndex, firstSegmentSize * sizeof( T ) );

			// see if second copy is needed due to wrap around
			if ( secondSegmentSize > 0 )
			{
				std::memcpy( outBuff + firstSegmentSize, m_buff, secondSegmentSize * sizeof( T ) );
			}		
		}
		else
		{
			for ( size_t i = 0; i < copyCount; ++i )
			{
				const size_t currIndex = ( m_frontIndex + i + startOffset ) % N;
				outBuff[i] = m_buff[currIndex];
			}
		}

		return copyCount;
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


// Char buffer variant which holds some string helper methods
template<size_t N>
class FixedCircularCharBuffer : public FixedCircularBuffer<char, N>
{
private:
	void CreateLpsArray( size_t lpsArr[N], const char* pattern, const size_t patternSize ) const
	{
		if ( N == 0 || patternSize > N )
		{
			return;
		}

		lpsArr[0] = 0;
		size_t currLpsLen = 0;
		size_t i = 1;
		while ( i < patternSize )
		{
			const char currChar = pattern[i];
			const char prevLpsEndChar = pattern[currLpsLen];
			if ( currChar == prevLpsEndChar )
			{
				++currLpsLen;
				lpsArr[i] = currLpsLen;
				++i;
			}
			else
			{
				if ( currLpsLen == 0 )
				{
					// no earlier matching characters, so keep len as 0
					lpsArr[i] = 0;
					++i;
				}
				else
				{					
					// Since the lpsArray already confirmed that characters from [0,currLpsLen - 1] (current longest prefix), match with characters in range 
					// [i - currLpsLen, i - 1] (suffix without character at i), we can re-use the previous LPS lengths to see if a prefix
					// exists with this new character at i. Set currLpsLen to the previous length and don't increment [i] so that loop can 
					// re-evaluate character at [i] given a previous prefix.
					currLpsLen = lpsArr[currLpsLen - 1];
				}
			}
		}
	}

public:
	std::optional<size_t> FindSubstringIndex( const char* str, const size_t strSize ) const
	{
		if ( !str || strSize == 0 )
		{
			return std::nullopt;
		}

		size_t lpsArr[N];
		CreateLpsArray( lpsArr, str, strSize );

		using Base = FixedCircularBuffer<char, N>;
		typename Base::RawSegments segments = this->GetRawAllocatedSegments();
		if ( !segments.segment1 || segments.segment1Size == 0 )
		{
			return std::nullopt;
		}

		// TODO: consolidate the duplicate logic in the segment 1 and segment 2 sections
		size_t currStrIndex = 0;
		size_t i = 0;
		while ( i < segments.segment1Size )
		{
			if ( segments.segment1[i] == str[currStrIndex] )
			{
				++currStrIndex;
				++i;
			}
			else
			{
				if ( currStrIndex == 0 )
				{
					++i;
				}
				else
				{
					// use lps to get prev longest suffix since that you know that characters prior to character at [i] were matching
					currStrIndex = lpsArr[currStrIndex - 1];
					continue;
				}
			}

			if ( currStrIndex >= strSize )
			{
				const size_t substrSegmentStartIndex = i - strSize;
				// subtract buffer start pointer from substring start pointer to get the index within the buffer for the substring start
				const size_t realBuffIndex = static_cast<size_t>( ( segments.segment1 + substrSegmentStartIndex ) - this->m_buff );
				return this->RealIndexToCircularIndex( realBuffIndex );
			}
		}		

		if ( !segments.segment2 || segments.segment2Size == 0 )
		{
			return std::nullopt;
		}

		i = 0;
		while ( i < segments.segment2Size )
		{
			if ( segments.segment2[i] == str[currStrIndex] )
			{
				++currStrIndex;
				++i;
			}
			else 
			{
				if ( currStrIndex == 0 )
				{
					++i;
				}
				else
				{
					// use lps to get prev longest suffix since that you know that characters prior to character at [i] were matching
					currStrIndex = lpsArr[currStrIndex - 1];
					continue;
				}
				
			}

			if ( currStrIndex >= strSize )
			{
				size_t realBuffIndex = 0;
				if ( strSize > i )
				{
					// start index is in segment 1
					const size_t lenInSegOne = strSize - i;
					const size_t substrSegmentStartIndex = segments.segment1Size - lenInSegOne;
					realBuffIndex = static_cast<size_t>( ( segments.segment1 + substrSegmentStartIndex ) - this->m_buff );
				}
				else
				{
					// start index is in segment 2
					const size_t substrSegmentStartIndex = i - strSize;
					// subtract buffer start pointer from substring start pointer to get the index within the buffer for the substring start
					realBuffIndex = static_cast<size_t>( ( segments.segment2 + substrSegmentStartIndex ) - this->m_buff );
				}

				return this->RealIndexToCircularIndex( realBuffIndex );
			}
		}

		return std::nullopt;
	}
};
