#ifndef REL_M623DLL_CIRCLE_ENTRY_H
#define REL_M623DLL_CIRCLE_ENTRY_H
#include "dolphin/mtx.h"

// Stores a soft circle's position, scale and opacity.
struct CircleEntry {
    Vec position;
    float scale;
    float alpha;
    // Copies the circle's position.
    void fn_1_D534(const Vec *source);
    void setAlpha(float opacity);
    void setScale(float radius);
};

CircleEntry *fn_1_D940(int viewIndex);
void fn_1_DA2C(CircleEntry *record);

#endif
