#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

extern float lbl_1_rodata_318;

float fn_1_DF18(float start, float end, float time)
{
    if (start == end || time <= lbl_1_rodata_318) {
        return end;
    }
    return (start * (time - lbl_1_rodata_318) + end) / time;
}

void fn_1_DF60(HuVecF *dest, HuVecF *target, float time)
{
    dest->x = fn_1_DF18(dest->x, target->x, time);
    dest->y = fn_1_DF18(dest->y, target->y, time);
    dest->z = fn_1_DF18(dest->z, target->z, time);
}
