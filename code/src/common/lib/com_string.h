////////////////////////////////////////////////////////////
//
// String helpers
//
////////////////////////////////////////////////////////////

#pragma once
#include <cstddef>
#include <cstring>
#include <string>

namespace ObfuscatedStringImpl
{
    ////////////////////////////////////////////////////////////
    // Using simple XOR cipher with changing compile time seed
    // in order to obfuscate strings in memory
    ////////////////////////////////////////////////////////////

    // seed derived from compile time so the key changes each build
    static constexpr size_t OBFUSCATED_STRING_SEED =
        ( __TIME__[0] - '0' ) * 100000 +
        ( __TIME__[1] - '0' ) * 10000 +
        ( __TIME__[3] - '0' ) * 1000 +
        ( __TIME__[4] - '0' ) * 100 +
        ( __TIME__[6] - '0' ) * 10 +
        ( __TIME__[7] - '0' );

    static constexpr size_t KNUTH_CONSTANT = 2654435761ULL;
    static constexpr size_t CHAR_BITMASK = 0xFF;


    // per-character key derived from seed + position so each char has a different key
    constexpr char ObfuscatedStringGetKey( size_t i )
    {
        return static_cast<char>( ( OBFUSCATED_STRING_SEED ^ ( i * KNUTH_CONSTANT ) ) & CHAR_BITMASK );
    }

    template<size_t N>
    struct ObfuscatedStringLiteral
    {
        char encrypted[N] = { 0 };

        // occurs at compile time
        constexpr ObfuscatedStringLiteral( const char( &str )[N] )
        {
            for ( size_t i = 0; i < N; i++ )
                encrypted[i] = str[i] ^ ObfuscatedStringGetKey( i );
        }
    };

    template<size_t N>
    struct DeobfuscatedString
    {
        char buf[N] = { 0 };

        // occurs at runtime
        DeobfuscatedString( const ObfuscatedStringLiteral<N>& s )
        {
            for ( size_t i = 0; i < N; i++ )
                buf[i] = s.encrypted[i] ^ ObfuscatedStringGetKey( i );
        }

        template<size_t A, size_t B>
        DeobfuscatedString( const DeobfuscatedString<A>& a, const DeobfuscatedString<B>& b )
        {
            static_assert( A + B - 1 == N, "Buffer size mismatch" );
            memcpy( buf, a.buf, A - 1 );           // copy a without null terminator
            memcpy( buf + A - 1, b.buf, B );       // copy b including null terminator
        }

        operator const char* ( ) const
        {
            return buf;
        }

        std::string ToStdString() const
        {
            return std::string( buf );
        }
    };

    template<size_t A, size_t B>
    DeobfuscatedString<A + B - 1> ObfuscatedStringConcatHelper( const DeobfuscatedString<A>& a, const ObfuscatedStringLiteral<B>& b )
    {
        // -1 in order to ignore 1 of the 2 null terminators 
        return DeobfuscatedString<A + B - 1>( a, DeobfuscatedString<B>( b ) );
    }
}

// meant to obfuscate sting in memory so that it is harder to perform simple string searches
#define OBFUSCATED_STRING( str ) ObfuscatedStringImpl::DeobfuscatedString<sizeof( str )>( ObfuscatedStringImpl::ObfuscatedStringLiteral<sizeof( str )>( str ) )
#define OBFUSCATED_STRING_CONCAT( obfuscatedStr, str ) ObfuscatedStringImpl::ObfuscatedStringConcatHelper( obfuscatedStr, ObfuscatedStringImpl::ObfuscatedStringLiteral<sizeof( str )>( str ) )
