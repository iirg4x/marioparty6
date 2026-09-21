#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

s32 lbl_1_data_410[15] = {
    DATANUM(DATA_mgconst, 0), DATANUM(DATA_mgconst, 1), DATANUM(DATA_mgconst, 2), DATANUM(DATA_mgconst, 3), DATANUM(DATA_mgconst, 4),
    DATANUM(DATA_mgconst, 5), DATANUM(DATA_mgconst, 6), DATANUM(DATA_mgconst, 7), DATANUM(DATA_mgconst, 8), DATANUM(DATA_mgconst, 9),
    DATANUM(DATA_mgconst, 10), DATANUM(DATA_mgconst, 11), DATANUM(DATA_mgconst, 12), DATANUM(DATA_mgconst, 13), DATANUM(DATA_mgconst, 69)
};

struct _struct_lbl_1_data_44C_0x14 lbl_1_data_44C[5] = {
    { 90, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 220, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 350, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 480, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 280, 58, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 66, 255, 122, 0 }
};

struct _struct_lbl_1_bss_3DF0_0x18 lbl_1_bss_3DF0[5];

void fn_1_6150(void)
{
    s32 spriteDataIndex;
    MGSCORE *temp_r3;
    s16 temp_r3_2;
    s16 var_r29;
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 5) {
        if (var_r31 < 4) {
            var_r29 = MgScoreBoxCreateChar(lbl_1_data_44C[var_r31].unk4, lbl_1_data_44C[var_r31].unk6, lbl_1_bss_3E80.characterNos[var_r31]);
            lbl_1_bss_3DF0[var_r31].unk0 = var_r29;
        } else {
            var_r29 = MgScoreBoxCreate(lbl_1_data_44C[var_r31].unk4, lbl_1_data_44C[var_r31].unk6);
            lbl_1_bss_3DF0[var_r31].unk0 = var_r29;
        }
        MgScoreBoxPosSet(var_r29, (f32) lbl_1_data_44C[var_r31].unk0, (f32) lbl_1_data_44C[var_r31].unk2);
        MgScoreBoxColorSet(var_r29, lbl_1_data_44C[var_r31].unk8, lbl_1_data_44C[var_r31].unk9, lbl_1_data_44C[var_r31].unkA);
        temp_r3 = MgScoreCreate(10158128, 10158128, 0);
        lbl_1_bss_3DF0[var_r31].unk4 = temp_r3;
        MgScoreUnitBankSet(temp_r3, 17);
        MgScoreMaxDigitSet(temp_r3, 4);
        MgScorePosSet(temp_r3, (f32) (lbl_1_data_44C[var_r31].unk0 - 12), (f32) lbl_1_data_44C[var_r31].unk2);
        MgScoreColorSet(temp_r3, lbl_1_data_44C[var_r31].unk10, lbl_1_data_44C[var_r31].unk11, lbl_1_data_44C[var_r31].unk12);
        if (var_r31 >= 4) {
            spriteDataIndex = 14;
            lbl_1_bss_3DF0[var_r31].unk8 = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_410[spriteDataIndex], 268435456, HEAP_MODEL));
            temp_r3_2 = HuSprGrpCreate(2);
            lbl_1_bss_3DF0[var_r31].unkC = temp_r3_2;
            lbl_1_bss_3DF0[var_r31].unkE = HuSprCreate(lbl_1_bss_3DF0[var_r31].unk8, 10, 0);
            HuSprGrpMemberSet(temp_r3_2, 0, lbl_1_bss_3DF0[var_r31].unkE);
            HuSprGrpPosSet(temp_r3_2, (f32) (lbl_1_data_44C[var_r31].unk0 - 40), (f32) lbl_1_data_44C[var_r31].unk2);
        }
        var_r31 += 1;
    }
    fn_1_6604();
}

void fn_1_6534(s16 scoreIndex, s16 value)
{
    MgScoreValueSet(lbl_1_bss_3DF0[scoreIndex].unk4, (s32) value);
}

void fn_1_6578(void)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[var_r31].unk0, 1);
        MgScoreDispOn(lbl_1_bss_3DF0[var_r31].unk4);
        HuSprAttrReset(lbl_1_bss_3DF0[var_r31].unkC, 0, 4);
        var_r31 += 1;
    }
}

void fn_1_6604(void)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[var_r31].unk0, 0);
        MgScoreDispOff(lbl_1_bss_3DF0[var_r31].unk4);
        var_r31 += 1;
    }
    if (_CheckFlag(196610U) != 0) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[4].unk0, 0);
        MgScoreDispOff(lbl_1_bss_3DF0[4].unk4);
        HuSprAttrSet(lbl_1_bss_3DF0[4].unkC, 0, 4);
    }
}

void fn_1_66C0(OMOBJ *arg0)
{
    f32 var_f31;
    u32 temp_r30;

    var_f31 = ((float *) arg0->work)[0];
    temp_r30 = arg0->work[2];
    if ((s32) temp_r30 >= 150) {
        if ((u32) arg0->work[1] != 0) {
            var_f31 += 0.02f;
            if (var_f31 >= 1.2f) {
                var_f31 = 1.2f;
                arg0->work[1] = 0;
            }
        } else {
            var_f31 -= 0.04f;
            if (var_f31 <= 1.0f) {
                var_f31 = 1.0f;
                arg0->work[1] = 1;
            }
        }
        MgScoreDigitScaleSet(lbl_1_bss_3DF0[4].unk4, var_f31, var_f31);
        HuSprScaleSet(lbl_1_bss_3DF0[4].unkC, 0, var_f31, var_f31);
        if ((s32) temp_r30 == 150) {
            GWRecordSet(GW_RECORD_M608, ((s32) lbl_1_bss_3E80.record) * 90);
            {
                s16 score = ((s32) lbl_1_bss_3E80.record) * 90;
                MgScoreValueSet(lbl_1_bss_3DF0[4].unk4, score);
            }
        }
    }
    ((float *) arg0->work)[0] = var_f31;
    arg0->work[2] = temp_r30 + 1;
}
