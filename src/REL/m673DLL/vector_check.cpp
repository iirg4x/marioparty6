// Checks whether all three components of a vector are zero.

#include "REL/m673DLL/vector_check.h"

// Returns whether all three vector components are zero.
u8 fn_1_B58(const Vec *vector)
{
    u8 allZero;
    u8 xyZero;

    allZero = 0;
    xyZero = 0;
    if (0.0f == vector->x && 0.0f == vector->y) {
        xyZero = 1;
    }
    if (xyZero != 0 && 0.0f == vector->z) {
        allZero = 1;
    }
    return allZero;
}
