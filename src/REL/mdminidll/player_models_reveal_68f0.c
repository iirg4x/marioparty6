#define _MATH_H
#include "game/object.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/model_animation.h"

extern OMOBJ *lbl_1_bss_14;
/* Target-backed consumed f32 views; original declarations are unknown. */
extern const f32 lbl_1_rodata_188;
extern const f32 lbl_1_rodata_74;
void fn_1_66F4(OMOBJ *obj);
void fn_1_25EF0(s16 player, Point3d *position, s16 type, s16 state);

void fn_1_68F0(void)
{
    Point3d sp8;
    MDMinidllPlayerConfig *row;
    s16 i;
    OMOBJ *obj;
    s16 count;

    count = 4;
    obj = lbl_1_bss_14;
    if (lbl_1_bss_804[1] == 5) {
        count = 1;
    } else {
        count = 4;
    }
    i = 0;
    row = &lbl_1_bss_7CC[i];
    while (i < count) {
        Hu3DModelPosGet(lbl_1_bss_230[row->charNo].model, &sp8);
        if (row->comF != 0) {
            fn_1_25EF0(i, &sp8, 4, 0);
        } else {
            fn_1_25EF0(i, &sp8, row->padNo, 0);
        }
        i += 1;
        row += 1;
    }
    i = 0;
    row = &lbl_1_bss_7CC[i];
    while (i < count) {
        Hu3DModelPosGet(lbl_1_bss_230[row->charNo].model, &sp8);
        sp8.y -= lbl_1_rodata_188;
        Hu3DModelPosSetV(obj->mdlId[row->charNo], &sp8);
        Hu3DModelScaleSet(obj->mdlId[row->charNo], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
        Hu3DModelAttrReset(obj->mdlId[row->charNo], 1U);
        i += 1;
        row += 1;
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_66F4;
    HuPrcSleep(15);
    HuAudFXPlay(1185);
    i = 0;
    row = &lbl_1_bss_7CC[i];
    while (i < count) {
        Hu3DModelPosGet(lbl_1_bss_230[row->charNo].model, &sp8);
        if (row->comF != 0) {
            fn_1_25EF0(i, &sp8, 4, 1);
        } else {
            fn_1_25EF0(i, &sp8, row->padNo, 1);
        }
        i += 1;
        row += 1;
    }
}
