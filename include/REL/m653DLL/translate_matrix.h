#ifndef REL_M653DLL_TRANSLATE_MATRIX_H
#define REL_M653DLL_TRANSLATE_MATRIX_H
#include "dolphin/mtx.h"

// Copies source into destination with the translation (x, y, z) added.
static inline void translateMatrix(Mtx source, Mtx destination, f32 x, f32 y, f32 z)
{
    PSMTXTransApply(source, destination, x, y, z);
}

#endif
