#ifndef M652_FLOAT4_H
#define M652_FLOAT4_H

#include "dolphin/mtx.h"

class Float4 {
public:
    float x, y, z, w;
    Float4 *fn_1_DA44(const Float4 *source);
    Float4 *fn_1_DA68(const Vec *source);
};

#endif
