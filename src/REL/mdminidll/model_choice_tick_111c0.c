#define _MATH_H
#include "game/object.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_752[];
extern OMOBJ *lbl_1_bss_C;
extern f32 lbl_1_bss_38[];
extern Point3d lbl_1_data_4C8[];
extern s16 lbl_1_data_86E[];

extern const f32 lbl_1_rodata_1AC;
extern const f32 lbl_1_rodata_258;

void fn_1_111C0(OMOBJ *arg0)
{
    Point3d sp8;
    s16 var_r30;
    OMOBJ *var_r27;
    OMOBJ *var_r26;
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

    if (arg0->work[0]++ >= 10U) {
        if ((s32) (HuPadDStkRep[0] & 1) != 0) {
            s16 var_r19;
            s16 var_r25;
            HUSPR_GROUP *var_r23;
            HUSPR_GROUPID var_r22;
            lbl_1_bss_804[1] -= 1;
            if (lbl_1_bss_804[1] < 0) {
                lbl_1_bss_804[1] += 6;
            }
            arg0->work[0] = 0;
            arg0->work[1] = 0;
            var_r19 = lbl_1_bss_804[1];
            var_r27 = lbl_1_bss_C;
            HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128);
            HuSprGrpTPLvlSet(lbl_1_bss_752[1], lbl_1_rodata_74);
            var_r22 = lbl_1_bss_752[1];
            var_r23 = &HuSprGrpData[var_r22];
            var_r25 = 0;
            while (var_r25 < var_r23->sprNum) {
                HuSprAttrSet(var_r22, var_r25, 4);
                var_r25 += 1;
            }
            HuSprAttrReset(lbl_1_bss_752[1], var_r19, 4);
            var_r27->work[2] = 1;
            var_r27->work[3] = 0;
            lbl_1_bss_38[0] = lbl_1_rodata_70;
            if (lbl_1_data_86E[0] == 1) {
                HuAudFXPlay(0);
            }
        } else if ((s32) (HuPadDStkRep[0] & 2) != 0) {
            s16 var_r18;
            s16 var_r24;
            HUSPR_GROUP *var_r21;
            HUSPR_GROUPID var_r20;
            lbl_1_bss_804[1] += 1;
            if (lbl_1_bss_804[1] >= 6) {
                lbl_1_bss_804[1] -= 6;
            }
            arg0->work[0] = 0;
            arg0->work[1] = 1;
            var_r18 = lbl_1_bss_804[1];
            var_r26 = lbl_1_bss_C;
            HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128);
            HuSprGrpTPLvlSet(lbl_1_bss_752[1], lbl_1_rodata_74);
            var_r20 = lbl_1_bss_752[1];
            var_r21 = &HuSprGrpData[var_r20];
            var_r24 = 0;
            while (var_r24 < var_r21->sprNum) {
                HuSprAttrSet(var_r20, var_r24, 4);
                var_r24 += 1;
            }
            HuSprAttrReset(lbl_1_bss_752[1], var_r18, 4);
            var_r26->work[2] = 1;
            var_r26->work[3] = 0;
            lbl_1_bss_38[1] = lbl_1_rodata_70;
            if (lbl_1_data_86E[1] == 1) {
                HuAudFXPlay(0);
            }
        }
    }
    var_r30 = 0;
    while (var_r30 < 6) {
        if (var_r30 == lbl_1_bss_804[1]) {
            MDMinidllModelAnimationRecord *var_r31;
            var_r31 = &lbl_1_bss_230[var_r30 + 11];
            Hu3DModelPosGet(var_r31->model, &sp8);
            var_f31 = lbl_1_data_4C8[var_r30 + 7].x;
            var_f25 = sp8.x;
            if (var_f25 == var_f31) {
                var_f26 = var_f31;
            } else {
                var_f26 = (var_f31 + (var_f25 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
            }
            sp8.x = var_f26;
            var_f30 = lbl_1_data_4C8[var_r30 + 7].y;
            var_f23 = sp8.y;
            if (var_f23 == var_f30) {
                var_f24 = var_f30;
            } else {
                var_f24 = (var_f30 + (var_f23 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
            }
            sp8.y = var_f24;
            var_f29 = lbl_1_data_4C8[var_r30 + 7].z;
            var_f21 = sp8.z;
            if (var_f21 == var_f29) {
                var_f22 = var_f29;
            } else {
                var_f22 = (var_f29 + (var_f21 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
            }
            sp8.z = var_f22;
            Hu3DModelPosSetV(var_r31->model, &sp8);
            if (var_r31->unknown_30++ > 30) {
                var_r31->unknown_30 = 35;
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
        } else {
            MDMinidllModelAnimationRecord *var_r29;
            var_r29 = &lbl_1_bss_230[var_r30 + 11];
            Hu3DModelPosSetV(var_r29->model, &lbl_1_data_4C8[var_r30 + 1]);
            Hu3DModelRotSet(var_r29->model, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
            Hu3DModelScaleSet(var_r29->model, lbl_1_rodata_178, lbl_1_rodata_178, lbl_1_rodata_6C);
            var_r29->unknown_30 = 0;
            Hu3DModelLayerSet(var_r29->model, 1);
        }
        var_r30 += 1;
    }
}
