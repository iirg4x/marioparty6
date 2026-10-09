/* Owns the on-screen direction prompts used while riders roll through the course. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

s32 lbl_1_data_4B0[6] = { DATANUM(DATA_m608, 101), DATANUM(DATA_m608, 97), DATANUM(DATA_m608, 98),
                          DATANUM(DATA_m608, 100), DATANUM(DATA_m608, 99), 0 };

M608SpriteGroupWork lbl_1_bss_3E68;

/* Called during scene setup to load the five direction prompts and hide them initially. */
void fn_1_6828(void)
{
    s32 spriteIndex;
    void *spriteData;

    lbl_1_bss_3E68.currentMember = -1;
    lbl_1_bss_3E68.group = HuSprGrpCreate(5);
    spriteIndex = 0;
    while (spriteIndex < 5) {
        spriteData = HuDataSelHeapReadNum(lbl_1_data_4B0[spriteIndex], HU_MEMNUM_OVL, HEAP_MODEL);
        if (spriteData == NULL) {
            /* Keep this member index; the null resource still reaches HuSprAnimRead below. */
            spriteIndex += 1;
            spriteIndex -= 1;
        }
        lbl_1_bss_3E68.anim[spriteIndex] = HuSprAnimRead(spriteData);
        HuSprGrpMemberSet(lbl_1_bss_3E68.group, (s16) spriteIndex,
                          HuSprCreate(lbl_1_bss_3E68.anim[spriteIndex], 10, 0));
        HuSprAttrSet(lbl_1_bss_3E68.group, (s16) spriteIndex, HUSPR_ATTR_DISPOFF);
        spriteIndex += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_3E68.group, 480.0f, 304.0f);
}

/* Called by the rolling sequence to replace the visible direction prompt, or hide all with -1. */
void fn_1_6958(s32 spriteMemberIndex)
{
    if (lbl_1_bss_3E68.currentMember >= 0) {
        HuSprAttrSet(lbl_1_bss_3E68.group, lbl_1_bss_3E68.currentMember, HUSPR_ATTR_DISPOFF);
    }
    lbl_1_bss_3E68.currentMember = (s16) spriteMemberIndex;
    if (lbl_1_bss_3E68.currentMember >= 0) {
        HuSprAttrReset(lbl_1_bss_3E68.group, lbl_1_bss_3E68.currentMember, HUSPR_ATTR_DISPOFF);
    }
}
