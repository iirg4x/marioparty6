#ifndef REL_M653DLL_ABSOLUTE_H
#define REL_M653DLL_ABSOLUTE_H
#include "dolphin/types.h"

// The size of a value, whatever its sign.
static inline f32 absolute(f32 value)
{
    if (value < 0.0f) {
        return -value;
    }
    return value;
}

#endif
