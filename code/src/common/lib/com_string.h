////////////////////////////////////////////////////////////
//
// String helpers
//
////////////////////////////////////////////////////////////

#pragma once
#include <cstddef>
#include <cstring>
#include <string>

// ========================
// Obfuscated String
// ========================

namespace ObfuscatedStringImpl
{
    ////////////////////////////////////////////////////////////
    // Using simple XOR cipher with changing compile time seed
    // in order to obfuscate strings in memory
    ////////////////////////////////////////////////////////////

    //change seed if strings are decrypted 
    static constexpr size_t OBFUSCATED_STRING_SEED = 1234;
    static constexpr size_t KNUTH_CONSTANT = 2654435761ULL;
    static constexpr size_t CHAR_BITMASK = 0xFF;


    // per-character key derived from seed + position so each char has a different key
    constexpr char ObfuscatedStringGetKey( size_t i )
    {
        return static_cast<char>( ( OBFUSCATED_STRING_SEED ^ ( i * KNUTH_CONSTANT ) ) & CHAR_BITMASK );
    }

    template<size_t N>
    struct ObfuscatedString
    {
        char encrypted[N] = { 0 };

        // occurs at compile time
        constexpr ObfuscatedString( const char( &str )[N] )
        {
            for ( size_t i = 0; i < N - 1; i++ )
            {
                encrypted[i] = str[i] ^ ObfuscatedStringGetKey( i );
            }
            encrypted[N - 1] = '\0';
        }
    };

    template<size_t N>
    struct DeobfuscatedString
    {
        char buf[N] = { 0 };

        // occurs at runtime
        DeobfuscatedString( const ObfuscatedString<N>& s )
        {
            for ( size_t i = 0; i < N - 1; i++ )
            {
                buf[i] = s.encrypted[i] ^ ObfuscatedStringGetKey( i );
            }
            buf[N - 1] = '\0';
        }

        operator const char* () const
        {
            return buf;
        }
    };
}

// meant to obfuscate string in binary executable
#define OBFUSCATED_STRING( str ) \
    []() -> const char* { \
        static constexpr ObfuscatedStringImpl::ObfuscatedString<sizeof(str)> encrypted( str ); \
        static const ObfuscatedStringImpl::DeobfuscatedString<sizeof(str)> deobfuscated( encrypted ); \
        return deobfuscated; \
    }()

// ========================
// String Helpers
// ========================

inline bool Com_StrEq( const char* a, const char* b, const size_t max )
{
    return strncmp( a, b, max ) == 0;
}

inline bool Com_StrEmpty( const char* str )
{
    return !str || str[0] == '\0';
}