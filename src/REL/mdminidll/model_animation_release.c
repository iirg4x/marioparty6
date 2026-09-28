#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/gamework.h"
#include "REL/mdminidll/model_animation.h"

extern OMOBJ *lbl_1_bss_24;

void fn_1_B610(void)
{
    s16 var_r31;
    s16 var_r30;

    if (lbl_1_bss_24) {
        *lbl_1_bss_24->mtnId = -1;
        for (var_r31 = 20; var_r31 >= 0; var_r31--) {
            for (var_r30 = 0; var_r30 < 4; var_r30++) {
                Hu3DAnimKill(lbl_1_bss_230[var_r31].animation[var_r30]);
            }
            Hu3DModelKill(lbl_1_bss_230[var_r31].model);
        }
    }
    lbl_1_bss_24 = NULL;
}
