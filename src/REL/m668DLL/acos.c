#define _MATH_H
#include "math.h"
#include "humath.h"

/* Keep the SDK double-return contract without emitting weak math helpers. */
double acos(double x);

extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_18;
extern const float lbl_1_rodata_1C;

float fn_1_220(float x)
{
    if (lbl_1_rodata_10 < x) {
        if (x < lbl_1_rodata_14) {
            return (float)acos((double)x);
        }
        return lbl_1_rodata_18;
    }
    return lbl_1_rodata_1C;
}

void fn_1_2A4(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}
