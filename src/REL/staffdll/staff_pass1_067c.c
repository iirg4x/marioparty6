#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

extern const float lbl_1_rodata_40;
extern const float lbl_1_rodata_44;

static inline float fn_1_620(float a, float b, float c, float t)
{
    float inv = lbl_1_rodata_40 - t;

    return (c * (t * t))
        + ((a * (inv * inv)) + ((b * (inv * t)) * lbl_1_rodata_44));
}

void fn_1_67C(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_620(a->x, b->x, c->x, t);
    dst->y = fn_1_620(a->y, b->y, c->y, t);
    dst->z = fn_1_620(a->z, b->z, c->z, t);
}
