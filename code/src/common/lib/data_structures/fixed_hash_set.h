////////////////////////////////////////////////////////////
//
// Linear probing set implementation using a static
// buffer for storage
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/hashing/hash.h"

template<class KeyType, size_t N, class Hash = std::hash<KeyType>, class KeyEqual = std::equal_to<KeyType>>
class FixedHashSet
{
private:
	enum class EntryState
	{
		Available,
		Claimed,
		Deleted
	};

	struct Entry
	{
		EntryState state;
		KeyType val;
	};

	Entry m_buffer[N] = {};
	size_t m_count = 0;
	Hash m_hash{};
	KeyEqual m_keyEqual{};

	Entry* getEntry( const KeyType& key )
	{
		const size_t hash = m_hash( key );
		const size_t hashIndex = hash % N;
		for ( size_t i = 0; i < N; ++i )
		{
			const size_t currIndex = ( hashIndex + i ) % N;
			Entry* currEntry = &m_buffer[currIndex];
			if ( currEntry->state == EntryState::Available )
			{
				return nullptr;
			}
			else if ( currEntry->state == EntryState::Deleted )
			{
				continue;
			}

			if ( m_keyEqual( currEntry->val, key ) )
			{
				return currEntry;
			}
		}

		return nullptr;
	}

public:
	class Iterator
	{
	private:
		const FixedHashSet* m_set;
		size_t m_index;

		size_t GetNextIndex( const size_t startingIndex, const bool includeStartingIndex = false )
		{
			if ( startingIndex >= N )
			{
				return N;
			}

			const size_t indexToStart = includeStartingIndex ? startingIndex : startingIndex + 1;
			for ( size_t i = indexToStart; i < N; ++i )
			{
				const Entry* currEntry = &m_set->m_buffer[i];
				if ( currEntry->state == EntryState::Claimed )
				{
					return i;
				}
			}

			return N;
		}

	public:
		Iterator( const FixedHashSet* set, const size_t startingIndex ) : m_set( set )
		{
			m_index = GetNextIndex( startingIndex, true );
		}

		const KeyType& operator*() const
		{
			COM_ASSERT( m_index < N, "Iterator can not be dereferenced while pointing to end of set");
			return m_set->m_buffer[m_index].val;
		}

		const KeyType* operator->() const
		{
			COM_ASSERT( m_index < N, "Iterator can not be dereferenced while pointing to end of set" );
			return &m_set->m_buffer[m_index].val;
		}

		Iterator& operator++()
		{
			m_index = GetNextIndex( m_index );
			return *this;
		}

		bool operator==( const Iterator& other ) const 
		{
			return m_index == other.m_index;
		}

		bool operator!=( const Iterator& other ) const 
		{
			return m_index != other.m_index;
		}
	};

	FixedHashSet() = default;

	bool Insert( const KeyType& key )
	{
		if ( m_count == N )
		{
			return false;
		}

		// see if key already exists
		Entry* existingEntry = getEntry( key );
		if ( existingEntry )
		{
			return false;
		}

		// insert a new entry
		const size_t hash = m_hash( key );
		const size_t hashIndex = hash % N;
		for ( size_t i = 0; i < N; ++i )
		{
			const size_t currIndex = ( hashIndex + i ) % N;
			Entry* currEntry = &m_buffer[currIndex];
			if ( currEntry->state != EntryState::Claimed )
			{
				currEntry->state = EntryState::Claimed;
				currEntry->val = key;
				++m_count;

				return true;
			}
		}

		return false;
	}

	bool Remove( const KeyType& key )
	{
		Entry* entry = getEntry( key );
		if ( !entry )
		{
			return false;
		}

		entry->state = EntryState::Deleted;
		--m_count;
		return true;
	}

	bool Contains( const KeyType& key )
	{
		const Entry* entry = getEntry( key );
		return entry != nullptr;
	}

	void Clear()
	{
		for ( size_t i = 0; i < N; ++i )
		{
			m_buffer[i].state = EntryState::Available;
		}
		m_count = 0;
	}

	size_t Count() const
	{
		return m_count;
	}

	size_t Capacity() const
	{
		return N;
	}

	bool IsFull() const
	{
		return m_count == N;
	}

	Iterator begin() const
	{
		return Iterator( this, 0 );
	}

	Iterator end() const
	{
		return Iterator( this, N );
	}
};

// TODO: make a fixed string variant of this set