#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

s32 lbl_1_data_4B0[6] = { 4391013, 4391009, 4391010, 4391012, 4391011, 0 };

M608SpriteGroupWork lbl_1_bss_3E68;

void fn_1_6828(void)
{
    s32 var_r31;
    void *temp_r3;

    lbl_1_bss_3E68.currentMember = -1;
    lbl_1_bss_3E68.group = HuSprGrpCreate(5);
    var_r31 = 0;
    while (var_r31 < 5) {
        temp_r3 = HuDataSelHeapReadNum(lbl_1_data_4B0[var_r31], 268435456, HEAP_MODEL);
        if (temp_r3 == NULL) {
            /* Retail has both updates here; their original intent is unknown. */
            var_r31 += 1;
            var_r31 -= 1;
        }
        lbl_1_bss_3E68.anim[var_r31] = HuSprAnimRead(temp_r3);
        HuSprGrpMemberSet(lbl_1_bss_3E68.group, (s16) var_r31, HuSprCreate(lbl_1_bss_3E68.anim[var_r31], 10, 0));
        HuSprAttrSet(lbl_1_bss_3E68.group, (s16) var_r31, 4);
        var_r31 += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_3E68.group, 480.0f, 304.0f);
}

void fn_1_6958(s32 member)
{
    if (lbl_1_bss_3E68.currentMember >= 0) {
        HuSprAttrSet(lbl_1_bss_3E68.group, lbl_1_bss_3E68.currentMember, 4);
    }
    lbl_1_bss_3E68.currentMember = (s16) member;
    if (lbl_1_bss_3E68.currentMember >= 0) {
        HuSprAttrReset(lbl_1_bss_3E68.group, lbl_1_bss_3E68.currentMember, 4);
    }
}
