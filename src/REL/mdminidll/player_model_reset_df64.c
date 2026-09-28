#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/readonly_scalars.h"

extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_24;
extern HUSPR_GROUPID lbl_1_bss_752[];

void fn_1_DF64(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *var_r30;
    s16 var_r31;

    var_r31 = 0;
    var_r30 = lbl_1_bss_7CC;
    while (var_r31 < 4) {
        if (var_r30->comF == 0) {
            if (var_r30->unknown_00 == 0) {
                OMOBJ *var_r29;

                var_r29 = lbl_1_bss_1C;
                Hu3DModelAttrSet(var_r29->mdlId[var_r31], 1U);
                HuSprScaleSet(lbl_1_bss_752[3], (s16)var_r31,
                              lbl_1_rodata_1C4, lbl_1_rodata_1C4);
                HuSprAttrSet(lbl_1_bss_752[3], (s16)var_r31, 4);
            }
        }
        var_r31 += 1;
        var_r30 += 1;
    }

    var_r31 = 0;
    while (var_r31 < 11) {
        MDMinidllModelAnimationRecord *var_r28;

        var_r28 = &lbl_1_bss_230[var_r31];
        Hu3DModelRotSet(var_r28->model, lbl_1_rodata_74, lbl_1_rodata_74,
                        lbl_1_rodata_74);
        var_r31 += 1;
    }

    arg0->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}
