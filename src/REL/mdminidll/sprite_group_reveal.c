#define _MATH_H
#include "game/sprite.h"

extern s16 lbl_1_bss_0;
extern s16 lbl_1_bss_752[9];
/* Existing target readonly storage; original declaration/name unknown. */
extern const f32 lbl_1_rodata_18C;
extern const f32 lbl_1_rodata_190;

void fn_1_6F58(void)
{
    s16 member;
    HUSPR_GROUP *group;
    HUSPR_GROUPID groupId;

    if (lbl_1_bss_0 == 0) {
        HuSprGrpPosSet(lbl_1_bss_752[0], lbl_1_rodata_18C, lbl_1_rodata_190);
        groupId = lbl_1_bss_752[0];
        group = &HuSprGrpData[groupId];
        member = 0;
        while (member < group->sprNum) {
            HuSprAttrReset(groupId, member, 4);
            member += 1;
        }
    }
}
