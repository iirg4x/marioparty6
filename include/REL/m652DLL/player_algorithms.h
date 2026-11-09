#ifndef REL_M652DLL_PLAYER_ALGORITHMS_H
#define REL_M652DLL_PLAYER_ALGORITHMS_H

long RandomInteger(long limit);

static inline int Frames07EF8(float seconds) { return (int)(60.0f * seconds); }

static inline int RandomIndex56D0(int count) { return RandomInteger(count); }

template<class T> static inline void Swap56D0(T &first, T &second)
{
    T value = first;
    first = second;
    second = value;
}

template<class Iterator>
static inline void SwapIter56D0(Iterator first, Iterator second)
{
    Swap56D0(*first, *second);
}

template<class Iterator>
static inline void ShuffleRange56D0(Iterator first, Iterator last)
{
    int remaining = last - first;
    if (remaining > 1) {
        --last;
        while (first < last) {
            SwapIter56D0(first, first + RandomIndex56D0(remaining));
            ++first;
            --remaining;
        }
    }
}

#endif
