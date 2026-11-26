#ifndef REL_MINIGAME_VALUE_BOUNDS_H
#define REL_MINIGAME_VALUE_BOUNDS_H

// Returns the greater of two values.
template<class T>
static inline T maximumValue(T first, T second)
{
    return first > second ? first : second;
}

// Returns the smaller of two values.
template<class T>
static inline T minimumValue(T first, T second)
{
    return first < second ? first : second;
}

#endif
