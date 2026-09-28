#define _MATH_H
#include "dolphin/gx.h"

extern void *lbl_1_bss_888;

void fn_1_1F508(void)
{
    if (lbl_1_bss_888) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_888, GX_FALSE);
    }
}
