#define _MATH_H
#include "REL/m602Dll.h"

u8 lbl_1_data_4B0[3] = { 64, 64, 64 };

/* RGB consumed by MgScoreBoxColorSet. */
struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4B4[4] = { { 66.0f, 58.0f }, { 184.0f, 58.0f }, { 391.0f, 58.0f }, { 509.0f, 58.0f } };

struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4E4[2] = { { 6.0f, 0.0f }, { 30.0f, 0.0f } };

char lbl_1_data_4FC[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 49, 0 };

char lbl_1_data_50C[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 50, 0 };

char lbl_1_data_51C[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 51, 0 };

char lbl_1_data_52C[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 52, 0 };

s16 lbl_1_bss_49A[4];

s16 lbl_1_bss_492[4];

s16 lbl_1_bss_48A[4];

s16 lbl_1_bss_482[4];

s16 lbl_1_bss_480;

MGTIMER *lbl_1_bss_47C;

u8 lbl_1_bss_478;

void fn_1_9ED8(void)
{
    lbl_1_bss_478 = 0;
}

s32 fn_1_9EEC(void)
{
    if (lbl_1_bss_478 != 0) {
        return MgTimerValueGet(lbl_1_bss_47C);
    }
    return -1;
}

void fn_1_9F34(void)
{
    if (lbl_1_bss_478 != 0) {
        MgTimerKill(lbl_1_bss_47C);
        lbl_1_bss_478 = 0;
    }
    lbl_1_bss_478 = 1;
    lbl_1_bss_47C = MgTimerCreate(0);
    MgTimerParamSet(lbl_1_bss_47C, 300, 0, 0);
    MgTimerModeOnSet(lbl_1_bss_47C, 1);
}

s32 fn_1_9FD8(void)
{
    if (lbl_1_bss_478 == 0) {
        return 1;
    }
    if (MgTimerDoneCheck(lbl_1_bss_47C) != 0) {
        return 1;
    }
    return 0;
}

void fn_1_A034(void)
{
    if (lbl_1_bss_478 != 0) {
        MgTimerRecordDispOff(lbl_1_bss_47C);
    }
}

void fn_1_A074(void)
{
    struct {
        /* Conservative frame-consumed layout, not an original record type.
         * The prefix is unread; its original type and purpose are unknown. */
        u8 unknown00[4];
        ANIMDATA *entry[2];
    } anim;
    s16 var_r31;
    s16 member;
    s16 sprite;

    anim.entry[0] = HuSprAnimRead(HuDataSelHeapReadNum(10158110, 268435456, HEAP_MODEL));
    anim.entry[1] = HuSprAnimRead(HuDataSelHeapReadNum(10158111, 268435456, HEAP_MODEL));
    var_r31 = 0;
    while (var_r31 < 4) {
        Point3d sp8;
        lbl_1_bss_492[var_r31] = 0;
        lbl_1_bss_49A[var_r31] = MgScoreBoxCreateChar(100, 36, GwPlayerConf[var_r31].charNo);
        MgScoreBoxColorSet(lbl_1_bss_49A[var_r31], lbl_1_data_4B0[0], lbl_1_data_4B0[1], lbl_1_data_4B0[2]);
        MgScoreBoxPosSet(lbl_1_bss_49A[var_r31], lbl_1_data_4B4[var_r31].unk0, lbl_1_data_4B4[var_r31].unk4);
        MgScoreBoxDispSet(lbl_1_bss_49A[var_r31], 0);
        lbl_1_bss_48A[var_r31] = HuSprGrpCreate(5);
        member = 1;
        sprite = HuSprCreate(anim.entry[0], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[var_r31], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[var_r31], member, lbl_1_data_4E4[0].unk0, lbl_1_data_4E4[0].unk4);
        member = 2;
        sprite = HuSprCreate(anim.entry[0], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[var_r31], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[var_r31], member, lbl_1_data_4E4[1].unk0, lbl_1_data_4E4[1].unk4);
        member = 3;
        sprite = HuSprCreate(anim.entry[1], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[var_r31], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[var_r31], member, lbl_1_data_4E4[0].unk0, lbl_1_data_4E4[0].unk4);
        HuSprAttrSet(lbl_1_bss_48A[var_r31], member, 4);
        member = 4;
        sprite = HuSprCreate(anim.entry[1], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[var_r31], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[var_r31], member, lbl_1_data_4E4[1].unk0, lbl_1_data_4E4[1].unk4);
        HuSprAttrSet(lbl_1_bss_48A[var_r31], member, 4);
        HuSprGrpPosSet(lbl_1_bss_48A[var_r31], lbl_1_data_4B4[var_r31].unk0, lbl_1_data_4B4[var_r31].unk4);
        HuSprGrpScaleSet(lbl_1_bss_48A[var_r31], 0.0f, 0.0f);
        lbl_1_bss_482[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(3997707, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_482[var_r31], 8U);
        Hu3DModelLayerSet(lbl_1_bss_482[var_r31], 1);
        Hu3DModelAttrSet(lbl_1_bss_482[var_r31], 1073741825U);
        if (var_r31 == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_4FC, &sp8);
        }
        if (var_r31 == 1) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_50C, &sp8);
        }
        if (var_r31 == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_51C, &sp8);
        }
        if (var_r31 == 3) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_52C, &sp8);
        }
        sp8.z += 0.25f;
        Hu3DModelPosSetV(lbl_1_bss_482[var_r31], &sp8);
        Hu3DMotionSpeedSet(lbl_1_bss_482[var_r31], 0.0001f);
        var_r31 += 1;
    }
}

/* const */
static const s16 lbl_1_rodata_1AC[6] = { 60, 105, 150, 195, 240, 0 };

void fn_1_A648(s16 arg0)
{
    s16 var_r31;
    s32 var_r30;

    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_rodata_1AC[var_r31] <= arg0) && (arg0 < lbl_1_rodata_1AC[var_r31 + 1])) {
            var_r30 = 1;
            if (arg0 == lbl_1_rodata_1AC[var_r31]) {
                HuAudFXPlay(1569);
            }
        } else {
            var_r30 = 0;
        }
        switch (var_r30) {                          /* irregular */
        case 0:
            Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 0.5f);
            break;
        case 1:
            Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 2.5f);
            break;
        }
        var_r31 += 1;
    }
}

void fn_1_A78C(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        MgScoreBoxDispSet(lbl_1_bss_49A[var_r31], 1);
        HuSprGrpScaleSet(lbl_1_bss_48A[var_r31], 1.0f, 1.0f);
        var_r31 += 1;
    }
}

void fn_1_A81C(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        switch (lbl_1_bss_492[var_r31]) {
        case 0:
            Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 0.5f);
            break;
        case 1:
            Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 1.5f);
            break;
        case 2:
            Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 2.5f);
            break;
        }
        var_r31 += 1;
    }
}

void fn_1_A914(s16 arg0, s16 arg1)
{
    lbl_1_bss_492[arg0] = arg1;
    switch (arg1) {                                 /* irregular */
    case 0:
        HuSprAttrReset(lbl_1_bss_48A[arg0], 1, 4);
        HuSprAttrReset(lbl_1_bss_48A[arg0], 2, 4);
        HuSprAttrSet(lbl_1_bss_48A[arg0], 3, 4);
        HuSprAttrSet(lbl_1_bss_48A[arg0], 4, 4);
        break;
    case 1:
        HuSprAttrSet(lbl_1_bss_48A[arg0], 1, 4);
        HuSprAttrReset(lbl_1_bss_48A[arg0], 2, 4);
        HuSprAttrReset(lbl_1_bss_48A[arg0], 3, 4);
        HuSprAttrSet(lbl_1_bss_48A[arg0], 4, 4);
        break;
    case 2:
        HuSprAttrSet(lbl_1_bss_48A[arg0], 1, 4);
        HuSprAttrSet(lbl_1_bss_48A[arg0], 2, 4);
        HuSprAttrReset(lbl_1_bss_48A[arg0], 3, 4);
        HuSprAttrReset(lbl_1_bss_48A[arg0], 4, 4);
        break;
    }
}

s16 fn_1_AB40(s16 arg0)
{
    return lbl_1_bss_492[arg0];
}

s32 fn_1_AB5C(void)
{
    return 2;
}

void fn_1_AB64(void)
{

}

void fn_1_AB68(s32 arg0)
{
    s16 temp_r29;
    s16 var_r31;
    s32 var_r30;

    temp_r29 = fn_1_51D4();
    if (arg0 != 0) {
        lbl_1_bss_480 = 0;
    }
    var_r30 = lbl_1_bss_480 / 10;
    if (var_r30 != 0) {
        var_r30 %= 2;
    } else {
        var_r30 = 0;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if (var_r31 == temp_r29) {
            switch (var_r30) {                      /* irregular */
            case 0:
                Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 0.5f);
                break;
            case 1:
                Hu3DMotionTimeSet(lbl_1_bss_482[var_r31], 2.5f);
                break;
            }
        }
        var_r31 += 1;
    }
    lbl_1_bss_480 += 1;
}
