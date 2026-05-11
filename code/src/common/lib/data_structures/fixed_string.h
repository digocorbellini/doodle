////////////////////////////////////////////////////////////
//
// Fixed buffer string and helpers for data structure 
// aliases
//
////////////////////////////////////////////////////////////

#pragma once
#include <cstring>
#include <functional>

template<size_t N>
class FixedString
{
private:
	char str[N] = { 0 };

public:
	FixedString() = default;

	FixedString( const char* other )
	{
		strncpy_s( str, other, N );
	}

	FixedString( const FixedString& other )
	{
		strncpy_s( str, other.str, N );
	}

	const char* c_str() const
	{
		return str;
	}

	size_t Length() const
	{
		return N;
	}

	bool operator==( const FixedString<N>& other ) const
	{
		return std::strncmp( str, other.str, N ) == 0;
	}

	FixedString& operator=( const FixedString& other )
	{
		strncpy_s( str, other.str, N );
		return *this;
	}
};


namespace FixedStringHashHelpers
{
	template<size_t N>
	struct Hash
	{
		size_t operator()( const FixedString<N>& str ) const
		{
			return static_cast<size_t>( FNV1A_64_Hash( str.c_str(), str.Length()) );
		}
	};

	template<size_t N>
	struct Equals
	{
		bool operator()( const FixedString<N>& a, const FixedString<N>& b ) const
		{
			return a == b;
		}
	};
}

