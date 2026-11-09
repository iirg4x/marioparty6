#ifndef REL_M652DLL_MOTION_ALGORITHMS_H
#define REL_M652DLL_MOTION_ALGORITHMS_H
template<class T> static inline T Square2664(const T &value)
{
    return value * value;
}

template<class T> static inline T Interpolate2664(T first, T last, double amount)
{
    return (1.0 - amount) * first + amount * last;
}

#endif
