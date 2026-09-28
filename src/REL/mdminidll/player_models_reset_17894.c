#define _MATH_H
#include "game/main.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "REL/mdminidll/row_state.h"

extern Point3d lbl_1_data_4C8[];
extern ANIMDATA *lbl_1_bss_6C8[];
extern s16 lbl_1_bss_752[];

void fn_1_17894(void)
{
    s16 var_r31;
    s16 var_r29;
    s16 var_r17;
    s16 spC;
    s16 spA;
    s16 sp8;
    MDMinidllModelAnimationRecord *var_r22;
    MDMinidllModelAnimationRecord *var_r28;
    MDMinidllModelAnimationRecord *var_r27;
    MDMinidllModelAnimationRecord *var_r26;
    MDMinidllModelAnimationRecord *var_r25;
    MDMinidllModelAnimationRecord *var_r30;
    s16 var_r24;
    s16 var_r23;
    HUSPR_GROUP *var_r21;
    s16 var_r20;
    HUSPR_GROUP *var_r19;
    s16 var_r18;

    lbl_1_bss_804[0] = 0;
    lbl_1_bss_804[1] = 0;
    for (var_r31 = 0; var_r31 < 4; var_r31 += 1) {
        lbl_1_bss_7CC[var_r31].unknown_00 = 0;
        lbl_1_bss_7CC[var_r31].unknown_02 = 0;
        lbl_1_bss_7CC[var_r31].comF = GwPlayer[var_r31].comF;
        lbl_1_bss_7CC[var_r31].comDif = GwPlayer[var_r31].comDif;
        lbl_1_bss_7CC[var_r31].charNo = GwPlayer[var_r31].charNo;
        lbl_1_bss_7CC[var_r31].padNo = GwPlayer[var_r31].padNo;
        if (lbl_1_bss_7CC[var_r31].comF == 0) {
            spC = lbl_1_bss_7CC[var_r31].padNo;
            lbl_1_bss_80[var_r31].x = lbl_1_rodata_1A0;
            lbl_1_bss_80[var_r31].y = lbl_1_rodata_1A0;
            var_r20 = lbl_1_bss_752[var_r31 + 4];
            var_r21 = &HuSprGrpData[var_r20];
            var_r24 = 0;
            while (var_r24 < var_r21->sprNum) {
                HuSprAttrReset(var_r20, var_r24, 4);
                var_r24 += 1;
            }
            HuSprBankSet(lbl_1_bss_752[var_r31 + 4], 0, spC);
            lbl_1_bss_804[0] += 1;
        } else {
            spA = lbl_1_bss_7CC[var_r31].comDif + 5;
            lbl_1_bss_80[var_r31].x = lbl_1_rodata_1A0;
            lbl_1_bss_80[var_r31].y = lbl_1_rodata_1A0;
            var_r18 = lbl_1_bss_752[var_r31 + 4];
            var_r19 = &HuSprGrpData[var_r18];
            var_r23 = 0;
            while (var_r23 < var_r19->sprNum) {
                HuSprAttrReset(var_r18, var_r23, 4);
                var_r23 += 1;
            }
            HuSprBankSet(lbl_1_bss_752[var_r31 + 4], 0, spA);
        }
    }

    for (var_r31 = 0; var_r31 < 11; var_r31 += 1) {
        for (var_r29 = 0; var_r29 < 4; var_r29 += 1) {
            var_r17 = var_r29;
            var_r22 = &lbl_1_bss_230[var_r31];
            Hu3DAnimAnimSet(var_r22->animation[var_r17],
                            lbl_1_bss_6C8[var_r31]);
        }
        var_r28 = &lbl_1_bss_230[var_r31];
        Hu3DAnimAnimSet(var_r28->animation[1], lbl_1_bss_6C8[var_r31]);
        Hu3DAnimBankSet(var_r28->animation[1], 2U);
        var_r27 = &lbl_1_bss_230[var_r31];
        Hu3DAnimAnimSet(var_r27->animation[3], lbl_1_bss_6C8[var_r31]);
        Hu3DAnimBankSet(var_r27->animation[3], 2U);
        var_r26 = &lbl_1_bss_230[var_r31];
        Hu3DAnimAnimSet(var_r26->animation[0], lbl_1_bss_6C8[var_r31]);
        Hu3DAnimBankSet(var_r26->animation[0], 1U);
        var_r25 = &lbl_1_bss_230[var_r31];
        Hu3DAnimAnimSet(var_r25->animation[2], lbl_1_bss_6C8[var_r31]);
        Hu3DAnimBankSet(var_r25->animation[2], 1U);
    }

    var_r31 = 0;
    while (var_r31 < 4) {
        sp8 = (s16)lbl_1_bss_7CC[var_r31].charNo;
        var_r30 = &lbl_1_bss_230[sp8];
        Hu3DModelPosSet(var_r30->model,
                        lbl_1_data_4C8[var_r31 + 25].x,
                        lbl_1_data_4C8[var_r31 + 25].y,
                        lbl_1_data_4C8[var_r31 + 25].z);
        Hu3DModelScaleSet(var_r30->model, lbl_1_rodata_178,
                          lbl_1_rodata_178, lbl_1_rodata_6C);
        Hu3DModelAttrReset(var_r30->model, 1U);
        var_r31 += 1;
    }
}
