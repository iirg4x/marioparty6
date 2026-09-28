#define _MATH_H
#include "dolphin.h"
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

/* This consumer view authenticates only the model handle at 0 and the
 * halfword counter at 0x30. The intervening bytes remain intentionally
 * unknown; total record stride is target-backed at 0x38. */
typedef struct MDMinidllModelView {
    s16 model;
    u8 unknown_02_2F[46];
    s16 counter;
    u8 unknown_32_37[6];
} MDMinidllModelView;

extern MDMinidllModelView lbl_1_bss_230[];
extern Point3d lbl_1_data_4C8[];

/* Target relocation sites authenticate these four-byte f32 owners. */
extern const f32 lbl_1_rodata_258;
extern const f32 lbl_1_rodata_1AC;
extern const f32 lbl_1_rodata_94;

void fn_1_10DCC(s16 arg0)
{
    Point3d sp8;
    MDMinidllModelView *var_r31;
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

    var_r31 = &lbl_1_bss_230[arg0 + 11];
    Hu3DModelPosGet(var_r31->model, &sp8);
    var_f31 = lbl_1_data_4C8[arg0 + 7].x;
    var_f25 = sp8.x;
    if (var_f25 == var_f31) {
        var_f26 = var_f31;
    } else {
        var_f26 = (var_f31 + (var_f25 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
    }
    sp8.x = var_f26;
    var_f30 = lbl_1_data_4C8[arg0 + 7].y;
    var_f23 = sp8.y;
    if (var_f23 == var_f30) {
        var_f24 = var_f30;
    } else {
        var_f24 = (var_f30 + (var_f23 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
    }
    sp8.y = var_f24;
    var_f29 = lbl_1_data_4C8[arg0 + 7].z;
    var_f21 = sp8.z;
    if (var_f21 == var_f29) {
        var_f22 = var_f29;
    } else {
        var_f22 = (var_f29 + (var_f21 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
    }
    sp8.z = var_f22;
    Hu3DModelPosSetV(var_r31->model, &sp8);
    if (var_r31->counter++ > 30) {
        var_r31->counter = 35;
        Hu3DModelRotGet(var_r31->model, &sp8);
        sp8.y += lbl_1_rodata_1AC;
        if (sp8.y >= lbl_1_rodata_94) {
            sp8.y -= lbl_1_rodata_94;
        }
        Hu3DModelRotSetV(var_r31->model, &sp8);
    }
    Hu3DModelScaleGet(var_r31->model, &sp8);
    var_f27 = sp8.x;
    if (var_f27 == lbl_1_rodata_258) {
        var_f28 = lbl_1_rodata_258;
    } else {
        var_f28 = (lbl_1_rodata_258 + (var_f27 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
    }
    sp8.x = var_f28;
    sp8.y = var_f28;
    Hu3DModelScaleSetV(var_r31->model, &sp8);
    Hu3DModelLayerSet(var_r31->model, 2);
}
