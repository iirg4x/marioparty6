#ifndef REL_M652DLL_MOTION_ALGORITHMS_H
#define REL_M652DLL_MOTION_ALGORITHMS_H
#include "REL/framework/square.h"
/* Interpolates between two values with a double-precision amount. */
template<class T> static inline T interpolateValue(T first, T last, double amount)
{
    return (1.0 - amount) * first + amount * last;
}

#endif
