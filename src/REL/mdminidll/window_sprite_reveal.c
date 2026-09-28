#define _MATH_H
#include "game/main.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/process.h"

extern HUWINID lbl_1_bss_858[];
extern HUSPR_GROUPID lbl_1_bss_752[];

void fn_1_15D30(void)
{
    s16 var_r31;
    s16 var_r30;
    HUSPR_GROUP *var_r29;
    s16 var_r28;

    var_r31 = 0;
    while (var_r31 < 4) {
        HuWinDispOff(lbl_1_bss_858[var_r31]);
        var_r31 += 1;
    }
    HuSprPriSet(lbl_1_bss_752[8], 0, 5500);
    var_r28 = lbl_1_bss_752[8];
    var_r29 = &HuSprGrpData[var_r28];
    var_r30 = 0;
    while (var_r30 < var_r29->sprNum) {
        HuSprAttrReset(var_r28, var_r30, 4);
        var_r30 += 1;
    }
    HuPrcSleep(5);
}
