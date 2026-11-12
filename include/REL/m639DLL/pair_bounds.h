#ifndef M639DLL_PAIR_BOUNDS_H
#define M639DLL_PAIR_BOUNDS_H

template <class T> inline T Min(T first, T second) { return (first < second) ? first : second; }
template<class T> static inline T lowerBoundPairValue(T lower, T value)
{
    if (lower > value) {
        return lower;
    }
    return value;
}

template<class T> static inline T clampPairValue(T value, T lower, T upper)
{
    if (value < lower) {
        return lower;
    }
    if (value > upper) {
        return upper;
    }
    return value;
}

template<class T> static inline void storeLowerBoundPairValue(float &target,
                                                            T lower, T value)
{
    T stored;
    T bounded = lowerBoundPairValue(lower, value);
    stored = bounded;
    target = (float)stored;
}

#endif
