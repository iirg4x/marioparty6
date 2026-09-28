#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"
#include "REL/mdminidll/player_config.h"

typedef struct MDMinidllSelectionView {
    u8 unknown_00[16];
    s16 word_10;
} MDMinidllSelectionView;

extern MDMinidllSelectionView lbl_1_data_89A[];
extern Point3d lbl_1_data_4C8[];
extern Point3d lbl_1_bss_50[4];
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_24;
extern HUSPR_GROUPID lbl_1_bss_752[];
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_90;
extern const f32 lbl_1_rodata_1C0;
extern const f32 lbl_1_rodata_1C4;

void fn_1_D554(OMOBJ *object);

void fn_1_DD00(OMOBJ *object)
{
    s32 index = 0;
    MDMinidllPlayerConfig *entry = lbl_1_bss_7CC;

    while ((s16)index < 4) {
        if (entry->comF == 0 && entry->unknown_00 == 0) {
            Point3d *position;
            OMOBJ *models;
            s16 bank;

            lbl_1_data_89A[entry->charNo].word_10 = 1;
            bank = entry->padNo;
            position = &lbl_1_data_4C8[entry->charNo + 14];
            models = lbl_1_bss_1C;

            Hu3DModelPosSet(models->mdlId[(s16)index], position->x, position->y, position->z);
            Hu3DModelRotSet(models->mdlId[(s16)index], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_90);
            Hu3DModelScaleSet(models->mdlId[(s16)index], lbl_1_rodata_6C, lbl_1_rodata_6C, lbl_1_rodata_6C);

            lbl_1_bss_50[(s16)index].x = position->x - lbl_1_rodata_1C0;
            lbl_1_bss_50[(s16)index].y = lbl_1_rodata_1C0 + position->y;
            lbl_1_bss_50[(s16)index].z = lbl_1_rodata_1C0 + position->z;

            Hu3DModelAttrReset(models->mdlId[(s16)index], 1U);
            HuSprScaleSet(lbl_1_bss_752[3], (s16)index, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
            HuSprAttrReset(lbl_1_bss_752[3], (s16)index, 4);
            HuSprBankSet(lbl_1_bss_752[3], (s16)index, bank);
        }
        index += 1;
        entry += 1;
    }

    object->work[0] = 10;
    object->objFunc = fn_1_D554;
    *lbl_1_bss_24->mtnId = 1;
}
