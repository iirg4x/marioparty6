#include <dolphin/types.h>

extern const float lbl_1_rodata_40;
extern const float lbl_1_rodata_44;

float fn_1_620(float a, float b, float c, float t)
{
    float inv = lbl_1_rodata_40 - t;

    return (c * (t * t))
        + ((a * (inv * inv)) + ((b * (inv * t)) * lbl_1_rodata_44));
}
