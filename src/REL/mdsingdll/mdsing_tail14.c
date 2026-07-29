#include "dolphin/gx.h"
#include "dolphin/types.h"

extern void *lbl_1_bss_13C8;
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;

float fn_1_2D0FC(float arg0, float arg1, float arg2, float arg3)
{
    if (arg2 <= lbl_1_rodata_398) {
        return arg0;
    }
    if (arg2 >= arg3) {
        return arg1;
    }
    return arg0 + ((arg2 / arg3) * (arg1 - arg0));
}

float fn_1_2D140(float arg0, float arg1, float arg2)
{
    if (arg0 == arg1) {
        return arg1;
    }
    return (arg1 + (arg0 * (arg2 - lbl_1_rodata_3BC))) / arg2;
}

void fn_1_2D170(s16 layerNo)
{
    if (lbl_1_bss_13C8) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_13C8, GX_FALSE);
    }
}
