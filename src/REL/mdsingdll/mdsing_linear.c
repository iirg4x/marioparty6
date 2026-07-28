#include "dolphin/types.h"

extern const float lbl_1_rodata_64;

float fn_1_16F0(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_64) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}
