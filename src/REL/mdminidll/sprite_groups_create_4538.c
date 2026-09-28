/* Recovered consumed view of the existing sprite-layout table. Field names
 * describe observed uses, not original names. Bytes 0x0A..0x0B are unknown.
 * The target consumes 27 rows at a 0x20-byte stride; no storage is added here. */
#define _MATH_H
#include "game/sprite.h"

typedef struct MDMinidllSpriteLayoutEntry {
    s16 groupIndex;
    s16 memberIndex;
    s16 animationIndex;
    s16 priority;
    s16 spriteParam;
    u8 unknown_0A[2];
    f32 positionX;
    f32 positionY;
    f32 scaleX;
    f32 scaleY;
    f32 rotation;
} MDMinidllSpriteLayoutEntry;

/* Existing target-owned tables; their complete declarations remain unknown. */
extern s32 lbl_1_data_98[];
extern s16 lbl_1_data_100[];
extern u8 lbl_1_data_114[];
extern s16 lbl_1_bss_71C[27];
extern s16 lbl_1_bss_752[9];
extern ANIMDATA *lbl_1_bss_764[26];

void fn_1_4538(void)
{
    s16 var_r30;
    s16 var_r29;
    MDMinidllSpriteLayoutEntry *var_r31;
    HUSPR_GROUP *var_r28;
    HUSPR_GROUPID var_r27;

    var_r30 = 0;
    while (var_r30 < 26) {
        lbl_1_bss_764[var_r30] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_98[var_r30], 268435456, HEAP_MODEL));
        var_r30 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 9) {
        lbl_1_bss_752[var_r30] = HuSprGrpCreate(lbl_1_data_100[var_r30]);
        var_r30 += 1;
    }
    var_r30 = 0;
    var_r31 = (MDMinidllSpriteLayoutEntry *) &lbl_1_data_114;
    while (var_r30 < 27) {
        lbl_1_bss_71C[var_r30] = HuSprCreate(lbl_1_bss_764[var_r31->animationIndex], (s16) (var_r31->priority + 6000), var_r31->spriteParam);
        HuSprGrpMemberSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, lbl_1_bss_71C[var_r30]);
        HuSprPosSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->positionX, var_r31->positionY);
        HuSprScaleSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->scaleX, var_r31->scaleY);
        HuSprZRotSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->rotation);
        var_r30 += 1;
        var_r31 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 9) {
        var_r27 = lbl_1_bss_752[var_r30];
        var_r28 = &HuSprGrpData[var_r27];
        var_r29 = 0;
        while (var_r29 < var_r28->sprNum) {
            HuSprAttrSet(var_r27, var_r29, 4);
            var_r29 += 1;
        }
        var_r30 += 1;
    }
}
