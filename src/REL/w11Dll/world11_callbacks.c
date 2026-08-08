#include "dolphin.h"
#include "humath.h"

#include "game/board/guide.h"
#include "game/hu3d.h"

/* These objects remain owned by the surrounding retail split while this
 * packet recovers only the callback ranges below. */
extern s16 lbl_1_bss_4;
extern s8 lbl_1_data_15[7];

extern void fn_1_5DC(s32 arg0, s32 arg1);
extern HU3D_MODELID mbObjModelIDGet(int modelId);
extern u32 mbMasuMAttrGet(s32 id);

void fn_1_510(void)
{
    if (lbl_1_bss_4 >= 0) {
        mbObjKill(lbl_1_bss_4);
    }
}

void fn_1_550(void)
{
}

void fn_1_554(void)
{
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_4), TRUE);
}

void fn_1_588(void)
{
}

s32 fn_1_58C(s32 arg0, s32 arg1)
{
    if ((mbMasuMAttrGet(arg1) & 1) != 0) {
        fn_1_5DC(arg0, arg1);
    }
    return 0;
}
