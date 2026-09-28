#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Existing module storage: six sprite scales, four target positions,
 * and the nine group IDs consumed by the other recovered UI functions. */
extern f32 lbl_1_bss_38[6];
extern Point3d lbl_1_bss_50[4];
extern HUSPR_GROUPID lbl_1_bss_752[9];
/* Target-backed readonly consumed views; these define no storage. */
extern const f32 lbl_1_rodata_1AC;
extern const f32 lbl_1_rodata_1C8;

void fn_1_8C08(OMOBJ *arg0)
{
    Point3d sp18;
    Point3d spC;
    s16 var_r31;
    s16 var_r29;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;
    f32 var_f25;
    f32 var_f24;
    f32 var_f23;
    f32 var_f22;
    f32 var_f21;
    f32 var_f20;
    f32 var_f19;
    f32 var_f18;

    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        Hu3DModelPosGet(arg0->mdlId[var_r31], &sp18);
        var_f31 = lbl_1_bss_50[var_r31].x;
        var_f23 = sp18.x;
        if (var_f23 == var_f31) {
            var_f24 = var_f31;
        } else {
            var_f24 = (var_f31 + (var_f23 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        sp18.x = var_f24;
        var_f30 = lbl_1_bss_50[var_r31].y;
        var_f21 = sp18.y;
        if (var_f21 == var_f30) {
            var_f22 = var_f30;
        } else {
            var_f22 = (var_f30 + (var_f21 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        sp18.y = var_f22;
        var_f29 = lbl_1_bss_50[var_r31].z;
        var_f19 = sp18.z;
        if (var_f19 == var_f29) {
            var_f20 = var_f29;
        } else {
            var_f20 = (var_f29 + (var_f19 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        sp18.z = var_f20;
        Hu3DModelPosSetV(arg0->mdlId[var_r31], &sp18);
        Hu3D3Dto2D(&sp18, 1, &spC);
        HuSprPosSet(lbl_1_bss_752[3], (s16) var_r31, spC.x, spC.y);
        HuSprScaleSet(lbl_1_bss_752[3], (s16) var_r31, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
        Hu3DModelRotGet(arg0->mdlId[var_r31], &sp18);
        var_f27 = sp18.z;
        if (var_f27 == lbl_1_rodata_74) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = (lbl_1_rodata_74 + (var_f27 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        sp18.z = var_f28;
        Hu3DModelRotSetV(arg0->mdlId[var_r31], &sp18);
        Hu3DModelScaleGet(arg0->mdlId[var_r31], &sp18);
        var_f25 = sp18.x;
        if (var_f25 == lbl_1_rodata_1C8) {
            var_f26 = lbl_1_rodata_1C8;
        } else {
            var_f26 = (lbl_1_rodata_1C8 + (var_f25 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        sp18.x = sp18.y = sp18.z = var_f26;
        Hu3DModelScaleSetV(arg0->mdlId[var_r31], &sp18);
    }
    for (var_r29 = 0; var_r29 < 6; var_r29++) {
        f32 sp8;
        sp8 = lbl_1_bss_38[var_r29];
        if (sp8 == lbl_1_rodata_6C) {
            var_f18 = lbl_1_rodata_6C;
        } else {
            var_f18 = (lbl_1_rodata_6C + (sp8 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
        }
        lbl_1_bss_38[var_r29] = var_f18;
        HuSprScaleSet(lbl_1_bss_752[2], var_r29, lbl_1_bss_38[var_r29], lbl_1_bss_38[var_r29]);
    }
}
