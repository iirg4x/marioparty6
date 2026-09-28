#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/model_animation.h"

/* Existing target readonly f32 scalar; original declaration/name unknown. */
extern const f32 lbl_1_rodata_6C;

void fn_1_9E8C(s16 arg0, Point3d *arg1, f32 farg0)
{
    MDMinidllModelAnimationRecord *entry;

    entry = &lbl_1_bss_230[arg0];
    Hu3DModelPosSet(entry->model, arg1->x, arg1->y, arg1->z);
    Hu3DModelScaleSet(entry->model, farg0, farg0, lbl_1_rodata_6C);
    Hu3DModelAttrReset(entry->model, 1U);
}
