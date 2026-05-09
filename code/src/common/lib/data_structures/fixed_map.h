////////////////////////////////////////////////////////////
//
// Linear probing map implementation using a static
// buffer for storage
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/hashing/hash.h"
#include <cstring>
#include <functional>


template<class KeyType, class ValType, size_t N, class Hash = std::hash<KeyType>, class KeyEqual = std::equal_to<KeyType>>
class FixedMap
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
		KeyType key;
		ValType val;
	};

	Entry m_buffer[N] = {};
	size_t m_count = 0;
	Hash m_hash{};
	KeyEqual m_keyEqual{};

	Entry* getEntry(const KeyType & key)
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

			if ( m_keyEqual( currEntry->key, key ) )
			{
				return currEntry;
			}
		}

		return nullptr;
	}

public:
	FixedMap() = default;

	bool Insert( const KeyType& key, const ValType& value )
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
		const size_t hash = m_hash(key);
		const size_t hashIndex = hash % N;
		for ( size_t i = 0; i < N; ++i )
		{
			const size_t currIndex = ( hashIndex + i ) % N;
			Entry* currEntry = &m_buffer[currIndex];
			if ( currEntry->state != EntryState::Claimed )
			{
				currEntry->key = key;
				currEntry->val = value;
				currEntry->state = EntryState::Claimed;
				++m_count;

				return true;
			}
		}

		return false;
	}

	bool Remove( const KeyType& key )
	{
		Entry* entry = getEntry(key);
		if ( !entry )
		{
			return false;
		}

		entry->state = EntryState::Deleted;
		--m_count;
		return true;
	}

	const ValType* Get( const KeyType& key )
	{
		const Entry* entry = getEntry( key );
		if ( !entry )
		{
			return nullptr;
		}

		return &entry->val;
	}

	const ValType* operator[]( const KeyType& key )
	{
		return Get( key );
	}

	bool ContainsKey( const KeyType& key )
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

	bool IsFull() const
	{
		return m_count == N;
	}
};


// ========================
// Useful Map Aliases
// ========================

namespace FixedMapStringKeyImpl
{
	struct Hash
	{
		size_t operator()( const char* str ) const
		{
			return static_cast<size_t>( FNV1A_64_Hash( str, std::strlen( str ) ) );
		}
	};
	
	struct Equals
	{
		bool operator()( const char* a, const char* b ) const
		{
			return std::strcmp( a, b ) == 0;
		}
	};
}

template<class ValType, size_t N>
using FixedMapStringKey = FixedMap<const char*, ValType, N, FixedMapStringKeyImpl::Hash, FixedMapStringKeyImpl::Equals>;