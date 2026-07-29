#include <dolphin/types.h>

extern const float lbl_1_rodata_10;

float fn_1_0(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_10) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}
