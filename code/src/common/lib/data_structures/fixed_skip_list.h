////////////////////////////////////////////////////////////
//
// Skip List implementation using static buffer for
// storage. Can be used for sorted O(logn) average
// insertion and O(n) sorted iteration.
//
////////////////////////////////////////////////////////////

#pragma once
#include "common/types.h"

template<typename T, size_t N, size_t MAX_LEVEL = 6, typename Compare = std::less<T>>
class FixedSkipList
{
private:
    static constexpr size_t RNG_SEED = 0x12345678;

    struct Node
    {
        T value;
        int next[MAX_LEVEL];
        int height;
    };

    Node   nodes[N];
    int    headIndex;
    int    firstFreeNodeIndex;
    size_t count;
    uint32_t rngSeed;
    Compare  comp{};

public:
    class Iterator
    {
    private:
        const FixedSkipList* list;
        int index;

    public:
        Iterator( const FixedSkipList* l, int idx ) : list( l ), index( idx )
        {}

        const T& operator*() const
        {
            return list->nodes[index].value;
        }
        const T* operator->() const
        {
            return &list->nodes[index].value;
        }

        Iterator& operator++()
        {
            index = list->nodes[index].next[0];
            return *this;
        }

        bool operator==( const Iterator& other ) const
        {
            return index == other.index;
        }
        bool operator!=( const Iterator& other ) const
        {
            return index != other.index;
        }
    };

private:
    int allocNode()
    {
        if ( firstFreeNodeIndex == -1 )
            return -1;

        int index = firstFreeNodeIndex;
        firstFreeNodeIndex = nodes[firstFreeNodeIndex].next[0];
        return index;
    }

    void freeNode( int index )
    {
        nodes[index].next[0] = firstFreeNodeIndex;
        firstFreeNodeIndex = index;
    }

    int randomHeight()
    {
        rngSeed = rngSeed * 1664525u + 1013904223u;
        uint32_t bits = rngSeed;

        int h = 1;
        while ( h < (int)MAX_LEVEL && ( bits & 1 ) )
        {
            h++;
            bits >>= 1;
        }
        return h;
    }

public:
    FixedSkipList() : headIndex( 0 ), firstFreeNodeIndex( 1 ), count( 0 ), rngSeed( RNG_SEED )
    {
        // Initialize head sentinel
        nodes[0].height = MAX_LEVEL;
        for ( size_t i = 0; i < MAX_LEVEL; ++i )
            nodes[0].next[i] = -1;

        // Initialize free list
        for ( size_t i = 1; i < N; ++i )
            nodes[i].next[0] = ( i + 1 < N ) ? int( i + 1 ) : -1;
    }

    bool Insert( const T& value )
    {
        if ( count == N - 1 )
            return false;

        int update[MAX_LEVEL];
        int x = headIndex;

        for ( int level = MAX_LEVEL - 1; level >= 0; --level )
        {
            while ( nodes[x].next[level] != -1 &&
                    comp( nodes[nodes[x].next[level]].value, value ) )
                x = nodes[x].next[level];
            update[level] = x;
        }

        int idx = allocNode();
        if ( idx == -1 )
            return false;

        int h = randomHeight();
        nodes[idx].value = value;
        nodes[idx].height = h;

        for ( int level = 0; level < h; ++level )
        {
            nodes[idx].next[level] = nodes[update[level]].next[level];
            nodes[update[level]].next[level] = idx;
        }

        count++;
        return true;
    }

    bool Remove( const T& value )
    {
        int update[MAX_LEVEL];
        int x = headIndex;

        for ( int level = MAX_LEVEL - 1; level >= 0; --level )
        {
            while ( nodes[x].next[level] != -1 &&
                    comp( nodes[nodes[x].next[level]].value, value ) )
                x = nodes[x].next[level];
            update[level] = x;
        }

        int target = nodes[x].next[0];
        if ( target == -1 || comp( value, nodes[target].value ) || comp( nodes[target].value, value ) )
            return false;

        for ( int level = 0; level < nodes[target].height; ++level )
        {
            if ( nodes[update[level]].next[level] == target )
                nodes[update[level]].next[level] = nodes[target].next[level];
        }

        freeNode( target );
        count--;
        return true;
    }

    bool Contains( const T& value ) const
    {
        int x = headIndex;
        for ( int level = MAX_LEVEL - 1; level >= 0; --level )
        {
            while ( nodes[x].next[level] != -1 &&
                    comp( nodes[nodes[x].next[level]].value, value ) )
                x = nodes[x].next[level];
        }
        int target = nodes[x].next[0];
        return target != -1 &&
            !comp( value, nodes[target].value ) &&
            !comp( nodes[target].value, value );
    }

    size_t Size() const
    {
        return count;
    }

    size_t Capacity() const
    {
        return N - 1;
    }

    bool IsFull() const
    {
        return count == N - 1;
    }

    void Clear()
    {
        for ( size_t i = 0; i < MAX_LEVEL; ++i )
            nodes[headIndex].next[i] = -1;

        firstFreeNodeIndex = 1;
        for ( size_t i = 1; i < N; ++i )
            nodes[i].next[0] = ( i + 1 < N ) ? int( i + 1 ) : -1;

        count = 0;
    }

    Iterator begin() const
    {
        return Iterator( this, nodes[headIndex].next[0] );
    }

    Iterator end()   const
    {
        return Iterator( this, -1 );
    }
};