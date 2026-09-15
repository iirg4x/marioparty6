#include "REL/m650/m650.h"

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_304(s16 mode, s16 frame);
void fn_1_3AC(s16 mode, s16 frame);
void fn_1_3B0(s16 mode, s16 frame);
void fn_1_3B4(s16 mode, s16 frame);
void fn_1_3B8(void);
void fn_1_484(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *obj);
void fn_1_1228(void);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_2010(s16 arg0);
void fn_1_21CC(OMOBJ *obj);
void fn_1_22A8(s16 player, s16 motion);
void fn_1_2320(s16 arg0, s16 arg1);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_2464(s16 player);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_26D0(void);
void fn_1_273C(s16 arg0);
void fn_1_289C(s16 player);
s16 fn_1_2CAC(s16 arg0);
void fn_1_2F1C(s16 arg0);
void fn_1_33F8(s16 player);
void fn_1_3F28(s16 player);
void fn_1_4430(s16 player);
void fn_1_473C(s16 arg0);
void fn_1_48C0(s16 arg0);
void fn_1_49C8(s16 arg0);
void fn_1_4BD4(s16 arg0);
s16 fn_1_4D58(s16 arg0);
void fn_1_5258(void);
void fn_1_5308(void);
void fn_1_5354(void);
void fn_1_5394(void);
void fn_1_54D4(void);
void fn_1_5544(s16 arg0);
void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1);
s16 fn_1_5728(void);
void fn_1_5A24(s16 arg0);
void fn_1_5E40(void);
void fn_1_62E0(void);
void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);
s16 fn_1_69B0(s16 player);
s16 fn_1_6D88(s16 player, HuVecF *pos);
s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius);
void fn_1_6F54(void);
s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate);

extern GXColor lbl_1_data_40[2];
extern Point3d lbl_1_data_28;
extern Point3d lbl_1_data_34;
extern s16 lbl_1_bss_4AC[4];
extern s16 lbl_1_bss_4B4;
extern s16 lbl_1_bss_4B6;
extern s32 lbl_1_bss_2A8;
extern s32 lbl_1_data_48[4];
extern s32 lbl_1_data_80[2];
extern s32 lbl_1_data_88[2];
extern s32 lbl_1_data_90[2];
extern s32 lbl_1_data_98[2];
extern s32 lbl_1_data_A0[2];

Point3d lbl_1_data_28 = { -2000.0f, 10000.0f, -400.0f };
Point3d lbl_1_data_34 = { -0.4f, -0.8f, -0.8f };
GXColor lbl_1_data_40[2] = { {128,128,128,128}, {144,144,144,144} };
s32 lbl_1_data_48[4] = { 7143442, 7143443, 7143444, 7143445 };
M650Motion lbl_1_data_58[5] = {
    { 9306277, 1073741825 },
    { 9633814, 1073741825 },
    { 9306278, 0 },
    { 9633792, 1073741825 },
    { 9633798, 0 },
};
s32 lbl_1_data_80[2] = { 7143424, 7143430 };
s32 lbl_1_data_88[2] = { 7143425, 7143431 };
s32 lbl_1_data_90[2] = { 7143429, 7143434 };
s32 lbl_1_data_98[2] = { 7143436, 7143437 };
s32 lbl_1_data_A0[2] = { 7143438, 7143439 };
/* Retail .data+A8..150 is unreferenced initialized storage. Its original
 * declaration and purpose are unknown; these are not inferred file IDs. */
u8 lbl_1_data_A8[168] = {
    63, 12, 204, 205, 63, 128, 0, 0, 63, 25, 153, 154, 63, 12, 204, 205,
    63, 128, 0, 0, 63, 0, 0, 0, 63, 64, 0, 0, 63, 128, 0, 0,
    63, 64, 0, 0, 63, 25, 153, 154, 63, 128, 0, 0, 63, 25, 153, 154,
    63, 64, 0, 0, 63, 128, 0, 0, 63, 51, 51, 51, 63, 64, 0, 0,
    63, 128, 0, 0, 63, 64, 0, 0, 63, 76, 204, 205, 63, 128, 0, 0,
    63, 51, 51, 51, 63, 0, 0, 0, 63, 128, 0, 0, 63, 0, 0, 0,
    63, 25, 153, 154, 63, 128, 0, 0, 63, 25, 153, 154, 63, 76, 204, 205,
    63, 128, 0, 0, 63, 51, 51, 51, 63, 0, 0, 0, 63, 128, 0, 0,
    63, 0, 0, 0, 63, 76, 204, 205, 63, 128, 0, 0, 63, 51, 51, 51,
    63, 76, 204, 205, 63, 128, 0, 0, 63, 51, 51, 51, 63, 76, 204, 205,
    63, 128, 0, 0, 63, 51, 51, 51,
};
u16 lbl_1_data_150[4] = { 1, 2, 4, 8 };

M650Player lbl_1_bss_28[4];

void fn_1_14FC(void)
{
    u16 cameras[4] = { 1, 2, 4, 8 };
    s32 var_r31;
    OMOBJ *viewObj;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;

    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DCameraCreate(cameras[var_r31]);
        if (var_r31 != 0) {
            var_f31 = 0.0f;
        } else {
            var_f31 = 480.0f;
        }
        if (var_r31 != 0) {
            var_f30 = 0.0f;
        } else {
            var_f30 = 640.0f;
        }
        Hu3DCameraViewportSet(cameras[var_r31], 0.0f, 0.0f, var_f30, var_f31, 0.0f, 1.0f);
        Hu3DCameraPerspectiveSet(cameras[var_r31], 40.0f, 60.0f, 25000.0f, 1.2f);
        if (var_r31 != 0) {
            var_f29 = 0.0f;
        } else {
            var_f29 = 480.0f;
        }
        if (var_r31 != 0) {
            var_f28 = 0.0f;
        } else {
            var_f28 = 640.0f;
        }
        Hu3DCameraScissorSet(cameras[var_r31], 0U, 0U, (u32) var_f28, (u32) var_f29);
        var_r31 += 1;
    }
    viewObj = omAddObjEx(lbl_1_bss_1C, 32730, 0U, 0U, -1, omOutViewMulti);
    viewObj->work[0] = 4;
    var_r31 = 0;
    while (var_r31 < 4) {
        CenterM[var_r31].x = 0.0f;
        CenterM[var_r31].y = 200.0f;
        CenterM[var_r31].z = 0.0f;
        CRotM[var_r31].x = -10.0f;
        CRotM[var_r31].y = 0.0f;
        CRotM[var_r31].z = 0.0f;
        CZoomM[var_r31] = 1000.0f;
        var_r31 += 1;
    }
}

void fn_1_1830(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    s16 temp_r3;
    s16 temp_r3_2;

    temp_r3 = Hu3DGLightCreateV(&lbl_1_data_28, &lbl_1_data_34, &lbl_1_data_40[lbl_1_bss_0.night]);
    Hu3DGLightStaticSet(temp_r3, 0);
    Hu3DGLightInfinitytSet(temp_r3);
    if (lbl_1_bss_0.night == 0) {
        Hu3DShineSet(1);
        Hu3DAmbColorSet(0.5f, 0.5f, 0.5f);
    }
    Hu3DShadowMultiCreate(30.0f, 500.0f, 80000.0f, 15);
    shadowPos.x = -750.0f;
    shadowPos.y = 4500.0f;
    shadowPos.z = -1000.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = 0.0f;
    shadowTarget.z = 0.0f;
    Hu3DShadowMultiPosSet(&shadowPos, &shadowUp, &shadowTarget, 15);
    if (lbl_1_bss_0.night == 0) {
        Hu3DShadowMultiColSet(130U, 30U, 0U, 15);
        Hu3DShadowMultiTPLvlSet(0.8f, 15);
    } else {
        Hu3DShadowMultiColSet(0U, 0U, 0U, 15);
        Hu3DShadowMultiTPLvlSet(0.5f, 15);
    }
    temp_r3_2 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_80[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(temp_r3_2, 65535U);
    Hu3DModelShadowMapSet(temp_r3_2);
    if (lbl_1_bss_0.night == 0) {
        Hu3DModelShadowMapTPLvlSet(temp_r3_2, 1.0f);
    } else {
        Hu3DModelShadowMapTPLvlSet(temp_r3_2, 0.5f);
    }
    temp_r3_2 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_88[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(temp_r3_2, 65535U);
}

void fn_1_1AE4(void)
{
    OMOBJ *spC;
    s16 sp8;
    s16 temp_r3;
    M650Player *temp_r30;
    s32 var_r29;
    s32 var_r28;

    for (var_r29 = 0; var_r29 < 4; var_r29++) {
        temp_r30 = &lbl_1_bss_28[var_r29];
        temp_r30->charNo = sp8 = GwPlayerConf[var_r29].charNo;
        temp_r30->unk12 = GwPlayerConf[var_r29].padNo;
        if (GwPlayerConf[var_r29].type == 0) {
            temp_r30->unk14 = -1;
        } else {
            temp_r30->unk14 = GwPlayerConf[var_r29].comDif;
        }
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(7143428, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        Hu3DModelAttrSet(temp_r3, 1U);
        temp_r30->model1E = temp_r3;
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(7143441, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        var_r28 = 0;
        while (var_r28 < 4) {
            temp_r30->jointMotionIDs[var_r28] = Hu3DJointMotion(temp_r3, HuDataSelHeapReadNum(lbl_1_data_48[var_r28], 268435456, HEAP_MODEL));
            var_r28 += 1;
        }
        Hu3DMotionSet(temp_r3, temp_r30->jointMotionIDs[0]);
        Hu3DModelAttrSet(temp_r3, 1073741825U);
        Hu3DModelAmbSet(temp_r3, 1.0f, 1.0f, 1.0f);
        Hu3DModelShadowSet(temp_r3);
        temp_r30->model20 = temp_r3;
        temp_r3 = CharModelCreate(temp_r30->charNo, 8);
        Hu3DModelShadowSet(temp_r3);
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        temp_r30->model = temp_r3;
        var_r28 = 0;
        while (var_r28 < 5) {
            temp_r30->motionIDs[var_r28] = CharMotionCreate(temp_r30->charNo, (u32) lbl_1_data_58[var_r28].file);
            var_r28 += 1;
        }
        CharMotionShiftSet(temp_r30->charNo, temp_r30->motionIDs[0], 0.0f, 0.0f, lbl_1_data_58->unk4);
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(7143440, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        Hu3DModelAttrSet(temp_r3, 1U);
        temp_r30->model52 = temp_r3;
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_90[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        Hu3DModelScaleSet(temp_r3, 0.3f, 1.0f, 0.3f);
        Hu3DModelPosSet(temp_r3, 0.0f, 0.2f, 0.0f);
        Hu3DModelAttrSet(temp_r3, 1U);
        Hu3DModelTPLvlSet(temp_r3, 0.5f);
        temp_r30->model54 = temp_r3;
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_98[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
        Hu3DMotionSpeedSet(temp_r3, 0.0f);
        Hu3DModelAttrSet(temp_r3, 1U);
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        temp_r30->model7A = temp_r3;
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_A0[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
        Hu3DMotionSpeedSet(temp_r3, 0.0f);
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        Hu3DModelAttrSet(temp_r3, 1U);
        temp_r30->model7C = temp_r3;
        temp_r3 = Hu3DModelCreate(HuDataSelHeapReadNum(7143435, 268435456, HEAP_MODEL));
        Hu3DMotionSpeedSet(temp_r3, 0.0f);
        Hu3DModelCameraSet(temp_r3, lbl_1_data_150[var_r29]);
        Hu3DModelAttrSet(temp_r3, 1U);
        temp_r30->model78 = temp_r3;
        temp_r30->unk76 = -1;
    }
    CharEffectLayerSet(3);
    spC = omAddObjEx(lbl_1_bss_1C, 80, 0U, 0U, -1, fn_1_21CC);
}

void fn_1_2010(s16 arg0)
{
    Mtx sp18;
    Point3d spC;
    s16 sp8;
    M650Player *temp_r31;

    temp_r31 = &lbl_1_bss_28[arg0];
    sp8 = MgSeqModeGet();
        switch (temp_r31->unk1C) {
        case 1:
            fn_1_273C(arg0);
            break;
        case 2:
            fn_1_2F1C(arg0);
            fn_1_33F8(arg0);
            break;
        case 3:
            fn_1_33F8(arg0);
            break;
        case 4:
            fn_1_3F28(arg0);
            break;
        case 5:
            fn_1_4430(arg0);
            break;
        case 6:
            fn_1_473C(arg0);
            break;
        case 7:
            fn_1_48C0(arg0);
            break;
        case 8:
            fn_1_49C8(arg0);
            break;
        case 10:
            fn_1_4BD4(arg0);
            break;
        }
    Hu3DModelPosGet(temp_r31->model20, &spC);
    Hu3DModelPosSetV(temp_r31->model1E, &spC);
    spC.y += 0.2;
    if (lbl_1_bss_0.unk12 == 0) {
        Hu3DModelObjMtxGet(temp_r31->model20, "itemhook_C", sp18);
        Hu3DModelMtxSet(temp_r31->model, &sp18);
    } else {
        mtxRot(sp18, 0.0f, 180.0f, 0.0f);
        mtxTransCat(sp18, 0.0f, 0.0f, 900.0f);
        Hu3DModelMtxSet(temp_r31->model, &sp18);
    }
    fn_1_5A24(arg0);
}

void fn_1_21CC(OMOBJ *obj)
{
    s32 var_r31;
    u32 temp_r30;

    temp_r30 = MgSeqModeGet();
    var_r31 = 0;
    while (var_r31 < 4) {
        fn_1_2010((s16) var_r31);
        var_r31 += 1;
    }
    if (fn_1_5728() != 0) {
        MgSeqModeNext();
    }
    if (_CheckFlag(196610U) != 0) {
        for (var_r31 = 0; var_r31 < 4; var_r31++) {
            if (lbl_1_bss_28[var_r31].timer && MgTimerDoneCheck(lbl_1_bss_28[var_r31].timer) == 1) {
                break;
            }
        }
        if ((var_r31 < 4) && (temp_r30 == 5)) {
            MgSeqModeNext();
        }
    }
}

void fn_1_22A8(s16 player, s16 motion)
{
    M650Player *work;

    work = &lbl_1_bss_28[player];
    Hu3DMotionShiftSet(work->model20, work->jointMotionIDs[motion], 0.0f, 8.0f, 1073741825U);
}

void fn_1_2320(s16 arg0, s16 arg1)
{
    CharMotionShiftSet(lbl_1_bss_28[arg0].charNo, lbl_1_bss_28[arg0].motionIDs[arg1], 0.0f, 8.0f, lbl_1_data_58[arg1].unk4);
}

void fn_1_23C4(s16 player, s16 motion, float blend)
{
    CharMotionShiftSet(lbl_1_bss_28[player].charNo,
        lbl_1_bss_28[player].motionIDs[motion], 0.0f, blend, lbl_1_data_58[motion].unk4);
}

void fn_1_2464(s16 player)
{
    M650Player *work;
    work = &lbl_1_bss_28[player];
    work->unk16 = 120;
    work->unk1C = 6;
    fn_1_22A8(player, 2);
    fn_1_23C4(player, 1, 8.0f);
}

void fn_1_2558(void)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_28[var_r31].unk1C = 1;
        fn_1_289C((s16) var_r31);
        var_r31 += 1;
    }
}

void fn_1_25B0(void)
{
    int var_r31;

    lbl_1_bss_0.unk10 = 1;
    var_r31 = 0;
    while (var_r31 < 4) {
        switch (lbl_1_bss_28[var_r31].unk1C) {
        case 1:
            lbl_1_bss_28[var_r31].unk1C = 9;
            Hu3DModelAttrSet(lbl_1_bss_28[var_r31].model1E, 1U);
            break;
        case 4:
            if (lbl_1_bss_0.winner != var_r31) {
                lbl_1_bss_28[var_r31].unk1C = 9;
                fn_1_22A8(var_r31, 0);
            }
            break;
        }
        var_r31 += 1;
    }
}

void fn_1_26D0(void)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_28[var_r31].unk1C = 9;
        Hu3DModelAttrSet(lbl_1_bss_28[var_r31].model1E, 1U);
        var_r31 += 1;
    }
}

void fn_1_273C(s16 arg0)
{
    M650Player *temp_r31;

    temp_r31 = &lbl_1_bss_28[arg0];
    Hu3DModelAttrReset(temp_r31->model1E, 1U);
    Hu3DModelRotSet(temp_r31->model1E, 0.0f, temp_r31->unk44, 0.0f);
    if (temp_r31->flag50 != 0) {
        temp_r31->unk44 += 1.7142857f;
        if (temp_r31->unk44 >= 30.0f) {
            temp_r31->unk44 = 30.0f;
            temp_r31->flag50 = 0;
        }
    } else {
        temp_r31->unk44 -= 1.7142857f;
        if (temp_r31->unk44 <= -30.0f) {
            temp_r31->unk44 = -30.0f;
            temp_r31->flag50 = 1;
        }
    }
    if (fn_1_2CAC(arg0) != 0) {
        temp_r31->unk1C = 2;
        temp_r31->unk48 = 0.0f;
        temp_r31->unk18 = 0;
        temp_r31->unk4C = 0.0f;
    }
}

void fn_1_289C(s16 player)
{
    HuVecF pos;
    HuVecF delta;
    s16 delay[4] = { 210, 140, 50, 0 };
    s16 delayRange[4] = { 105, 70, 35, 70 };
    s16 chance[4] = { 90, 45, 15, 5 };
    M650Player *work;
    int i;

    work = &lbl_1_bss_28[player];
    if (work->unk14 != -1) {
        work->unk98 = delay[work->unk14] + frandmod(delayRange[work->unk14]);
        if (frandmod(100) < chance[work->unk14]) {
            work->unk9C = 1;
        } else {
            work->unk9C = 0;
        }
        Hu3DModelPosGet(work->model20, &pos);
        work->candidateCount = 0;
        work->unk9A = 0;
        for (i = 0; i < 32; i++) {
            if (lbl_1_bss_2AC[i][player].state != 0) continue;
            if (lbl_1_data_248[i].pos.z < pos.z - 50.0f) continue;
            if (lbl_1_data_248[i].pos.z > 900.0f + pos.z) continue;
            if (lbl_1_data_248[i].pos.x > 500.0f + pos.x) continue;
            if (lbl_1_data_248[i].pos.x < pos.x - 500.0f) continue;
            delta.x = lbl_1_data_248[i].pos.x - pos.x;
            delta.y = 0.0f;
            delta.z = lbl_1_data_248[i].pos.z - pos.z;
            if (sqrtf(PSVECSquareMag(&delta)) > 1000.0f) continue;
            work->candidateIDs[work->candidateCount] = i;
            if (++work->candidateCount == 12) {
                OSReport("WARNING! obst cnt\n");
                print8(100, player * 16 + 100, 1.8f, "OBST CNT OVER %d", player);
                return;
            }
        }
    }
}

s16 fn_1_2CAC(s16 arg0)
{
    Point3d spC;
    f32 sp8;
    M650Player *temp_r31;
    s32 var_r30;
    s16 var_r29;
    s16 var_r28;
    f32 temp_f31;

    temp_r31 = &lbl_1_bss_28[arg0];
    Hu3DModelPosGet(temp_r31->model20, &spC);
    if (temp_r31->unk14 == -1) {
        return (s16) (HuPadBtnDown[temp_r31->unk12] & 256);
    }
    if (temp_r31->unk98 != 0) {
        temp_r31->unk98 -= 1;
        return 0;
    }
    temp_f31 = (f32) (800.0 * sin((3.141592653589793 * (f64) temp_r31->unk44) / 180.0));
    sp8 = (f32) (800.0 * cos((3.141592653589793 * (f64) temp_r31->unk44) / 180.0));
    if (((spC.x + temp_f31) >= 1150.0f) || ((spC.x + temp_f31) <= -1150.0f)) {
        var_r29 = 1;
    } else {
        var_r29 = 0;
    }
    var_r28 = 0;
    var_r30 = 0;
    while (var_r30 < temp_r31->candidateCount) {
        if (fn_1_7000(arg0, &spC, temp_r31->unk44, temp_r31->candidateIDs[var_r30]) != 0) {
            var_r28 = 1;
        } else {
            var_r30 += 1;
            continue;
        }
        break;
    }
    if (temp_r31->unk9A++ >= 70) {
        if (temp_r31->unk9C != 0) {
            return 1;
        }
        if (var_r29 != 0) {
            return 0;
        }
        return 1;
    }
    if (temp_r31->unk9C != 0) {
        if ((var_r28 != 0) || (var_r29 != 0) || (temp_r31->candidateCount == 0)) {
            return 1;
        }
        return 0;
    }
    if ((var_r28 == 0) && (var_r29 == 0)) {
        return 1;
    }
    return 0;
}

void fn_1_2F1C(s16 arg0)
{
    M650Player *temp_r31 = &lbl_1_bss_28[arg0];
    Point3d sp38;
    Point3d sp2C;
    HuVecF offset;
    s32 sounds[4] = { M650_EFFECT_2028, M650_EFFECT_2029, M650_EFFECT_2030, M650_EFFECT_2031 };
    f32 spC;
    f32 sp8;
    f32 temp_f31;
    f32 motionMax;

    Hu3DModelRotGet(temp_r31->model20, &sp38);
    if (sp38.y > temp_r31->unk44) {
        sp38.y -= 2.0f;
        if (sp38.y < temp_r31->unk44) {
            sp38.y = temp_r31->unk44;
        }
    } else if (sp38.y < temp_r31->unk44) {
        sp38.y += 2.0f;
        if (sp38.y > temp_r31->unk44) {
            sp38.y = temp_r31->unk44;
        }
    }
    Hu3DModelRotSetV(temp_r31->model20, &sp38);
    if (temp_r31->unk18 == 30) {
        fn_1_23C4(arg0, 2, 8.0f);
        Hu3DModelPosSet(temp_r31->model52, 0.0f, 0.0f, 0.0f);
        Hu3DModelHookSet(temp_r31->model, "f-itemhook-r", temp_r31->model52);
        Hu3DModelRotSet(temp_r31->model52, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(temp_r31->model52, 1U);
    }
    if (temp_r31->unk18 > 30) {
        temp_f31 = Hu3DMotionTimeGet(temp_r31->model);
        motionMax = Hu3DMotionMaxTimeGet(temp_r31->model);
        if ((temp_f31 >= motionMax) && (temp_r31->unk18 > 44)) {
            fn_1_23C4(arg0, 0, 8.0f);
            temp_r31->unk1C = 3;
            temp_r31->unk18 = 0;
            return;
        }
        if (temp_r31->unk18 == 44) {
            HuAudFXPlay(sounds[arg0]);
            Hu3DModelObjPosGet(temp_r31->model, "f-itemhook-r", &sp2C);
            offset.x = offset.y = offset.z = 0.0f;
            Hu3DModelHookReset(temp_r31->model);
            Hu3DModelPosSet(temp_r31->model52, sp2C.x + offset.x, sp2C.y + offset.y, sp2C.z + offset.z);
            Hu3DModelAttrReset(temp_r31->model54, 1U);
            Hu3DModelPosSet(temp_r31->model54, sp2C.x + offset.x, 0.2f, sp2C.z + offset.z);
            Hu3DModelRotSet(temp_r31->model52, 0.0f, 45.0f, 0.0f);
            fn_1_5600(arg0, sp2C.y + offset.y, &sp8, &spC);
            temp_r31->pos.x = (sp8 * cos((3.141592653589793 * spC) / 180.0)) * sin((3.141592653589793 * temp_r31->unk44) / 180.0);
            temp_r31->pos.y = (f32) ((f64) sp8 * sin((3.141592653589793 * (f64) spC) / 180.0));
            temp_r31->pos.z = (sp8 * cos((3.141592653589793 * spC) / 180.0)) * cos((3.141592653589793 * temp_r31->unk44) / 180.0);
            Hu3DModelAttrSet(temp_r31->model1E, 1U);
        }
    }
    temp_r31->unk18 += 1;
}

void fn_1_33F8(s16 player)
{
    M650Player *work = &lbl_1_bss_28[player];
    HuVecF pos;
    HuVecF horizontal;
    HuVecF reflection;
    HuVecF normal;
    s32 hitSounds[4] = { M650_EFFECT_2032, M650_EFFECT_2033, M650_EFFECT_2034, M650_EFFECT_2035 };
    s32 groundSounds[4] = { M650_EFFECT_2036, M650_EFFECT_2037, M650_EFFECT_2038, M650_EFFECT_2039 };
    float speed;
    float distanceSquared;
    float radiusSquared;
    float penetration;
    float velocityLength;
    s16 hit;

    if (work->pos.x != 0.0f || work->pos.y != 0.0f || work->pos.z != 0.0f) {
        work->unk1A++;
        speed = HuMagVecF(&work->pos);
        Hu3DModelPosGet(work->model52, &pos);
        pos.x += work->pos.x;
        pos.y += work->pos.y;
        pos.z += work->pos.z;
        horizontal.x = work->pos.x;
        horizontal.y = 0.0f;
        horizontal.z = work->pos.z;
        work->unk4C += sqrtf(PSVECSquareMag(&horizontal));
        if (pos.y <= 0.5f) {
            HuAudFXPlay(groundSounds[player]);
            pos.y = 0.5f;
            work->pos.x *= 0.4;
            work->pos.y *= -0.4;
            work->pos.z *= 0.4;
            if (work->pos.y < 1.0f) {
                work->unk1A = 0;
                if (lbl_1_bss_0.unk10 == 0) {
                    work->unk1C = 4;
                    fn_1_22A8(player, 1);
                    /* Retail discards this promoted player value; original purpose unknown. */
                    (void)(int)player;
                    HuAudFXPlayPan(1001, 32);
                } else {
                    work->unk1C = 9;
                }
                work->pos.x = work->pos.y = work->pos.z = 0.0f;
            }
        }
        if (pos.x <= -1210.0f) {
            HuAudFXPlay(hitSounds[player]);
            pos.x = -1210.0f;
            normal.x = 1.0f;
            normal.y = normal.z = 0.0f;
            C_VECReflect(&work->pos, &normal, &reflection);
            work->pos.x = 0.5 * reflection.x * speed;
            work->pos.y = 0.5 * reflection.y * speed;
            work->pos.z = 0.5 * reflection.z * speed;
        }
        if (pos.x >= 1210.0f) {
            HuAudFXPlay(hitSounds[player]);
            pos.x = 1210.0f;
            normal.x = -1.0f;
            normal.y = normal.z = 0.0f;
            C_VECReflect(&work->pos, &normal, &reflection);
            work->pos.x = 0.5 * reflection.x * speed;
            work->pos.y = 0.5 * reflection.y * speed;
            work->pos.z = 0.5 * reflection.z * speed;
        }
        hit = fn_1_6D88(player, &pos);
        if (hit >= 0) {
            HuAudFXPlay(hitSounds[player]);
            horizontal.x = pos.x;
            horizontal.y = 0.0f;
            horizontal.z = pos.z;
            distanceSquared = PSVECSquareDistance(&lbl_1_data_248[hit].pos, &horizontal);
            radiusSquared = 8100.0f;
            if (distanceSquared < radiusSquared) {
                penetration = sqrtf(radiusSquared - distanceSquared);
                normal.x = normal.y = normal.z = 0.0f;
                velocityLength = sqrtf(PSVECSquareDistance(&work->pos, &normal));
                if (velocityLength > 0.0f) {
                    pos.x -= (work->pos.x * penetration) / velocityLength;
                    pos.y -= (work->pos.y * penetration) / velocityLength;
                    pos.z -= (work->pos.z * penetration) / velocityLength;
                }
            }
            normal.x = pos.x - lbl_1_data_248[hit].pos.x;
            normal.y = 0.0f;
            normal.z = pos.z - lbl_1_data_248[hit].pos.z;
            PSVECNormalize(&normal, &normal);
            if (work->pos.x || work->pos.y || work->pos.z) {
                C_VECReflect(&work->pos, &normal, &reflection);
                work->pos.x = 0.4 * reflection.x * speed;
                work->pos.y = 0.4 * reflection.y * speed;
                work->pos.z = 0.4 * reflection.z * speed;
            }
            pos.x = 90.0f * normal.x + lbl_1_data_248[hit].pos.x;
            pos.z = 90.0f * normal.z + lbl_1_data_248[hit].pos.z;
        }
        Hu3DModelPosSetV(work->model52, &pos);
        Hu3DModelPosSet(work->model54, pos.x, 0.2f, pos.z);
        work->pos.y -= 1.6333333f;
    }
}

void fn_1_3F28(s16 player)
{
    M650Player *work = &lbl_1_bss_28[player];
    HuVecF pos;
    HuVecF itemPos;
    HuVecF rot;
    HuVecF delta;
    s32 sounds[4] = { M650_EFFECT_2040, M650_EFFECT_2041, M650_EFFECT_2042, M650_EFFECT_2043 };
    float angle;
    float distance;

    if (work->unk48 < 12.0f) work->unk48 += 1.0f;
    Hu3DModelPosGet(work->model52, &itemPos);
    Hu3DModelPosGet(work->model20, &pos);
    itemPos.y = 0.0f;
    delta.x = itemPos.x - pos.x;
    delta.z = itemPos.z - pos.z;
    distance = HuMagPoint2D(delta.x, delta.z);
    if (distance > work->unk48) {
        work->direction.x = work->unk48 * (delta.x / distance);
        work->direction.z = work->unk48 * (delta.z / distance);
    } else {
        work->direction.x = delta.x;
        work->direction.z = delta.z;
    }
    Hu3DModelRotGet(work->model20, &rot);
    if (work->direction.x == 0.0f) {
        angle = 0.0f;
    } else {
        angle = 180.0 * (atan2(work->direction.x, work->direction.z) / M_PI);
    }
    if (rot.y < angle) {
        rot.y += 2.0f;
        if (rot.y > angle) rot.y = angle;
    } else if (rot.y > angle) {
        rot.y -= 2.0f;
        if (rot.y < angle) rot.y = angle;
    }
    Hu3DModelRotSetV(work->model20, &rot);
    if (work->unk74 != 0 && pos.z >= 5500.0f) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        work->unk1C = 10;
        fn_1_22A8(player, 0);
    } else if (distance > work->unk48) {
        pos.x += work->direction.x;
        pos.z += work->direction.z;
        Hu3DModelPosSetV(work->model20, &pos);
    } else {
        Hu3DModelPosSetV(work->model20, &itemPos);
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        if (lbl_1_bss_0.unk10 == 0) {
            if (work->unk74 == 0) {
                work->unk1C = 7;
            } else {
                work->unk1C = 10;
                fn_1_22A8(player, 0);
            }
        } else {
            fn_1_22A8(player, 0);
            work->unk1C = 9;
        }
    }
    if (fn_1_69B0(player)) {
        work->unk1C = 5;
        omVibrate(player, 20, 7, 3);
    }
}

void fn_1_4430(s16 player)
{
    HuVecF pos;
    M650Player *work;
    s16 duration;

    work = &lbl_1_bss_28[player];
    duration = work->unk48;
    if (work->unk18 % 4 == 0) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
    } else if (work->unk18 % 4 == 2) {
        Hu3DModelAttrReset(work->model52, 1);
        Hu3DModelAttrReset(work->model54, 1);
    }
    if (work->unk18++ > duration) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        if (lbl_1_bss_0.unk10 == 0) {
            work->unk1C = 6;
            fn_1_2464(player);
        } else {
            work->unk1C = 9;
            fn_1_22A8(player, 0);
        }
        Hu3DMotionSpeedSet(work->model20, 1.0f);
        work->unk18 = 0;
        return;
    }
    Hu3DModelPosGet(work->model20, &pos);
    pos.x += (work->reflected.x * work->unk48) / 2.0f;
    pos.z += (work->reflected.z * work->unk48) / 2.0f;
    if (pos.x < -1150.0f) pos.x = -1150.0f;
    if (pos.x > 1150.0f) pos.x = 1150.0f;
    Hu3DModelPosSetV(work->model20, &pos);
    Hu3DMotionSpeedSet(work->model20, 0.3f);
}

void fn_1_473C(s16 arg0)
{
    Point3d sp8;
    M650Player *temp_r31;

    temp_r31 = &lbl_1_bss_28[arg0];
    if (--temp_r31->unk16 != 0) {
        Hu3DModelRotGet(temp_r31->model20, &sp8);
        sp8.y += 3.0f;
        if (sp8.y >= 180.0f) {
            sp8.y -= 360.0f;
        }
        Hu3DModelRotSetV(temp_r31->model20, &sp8);
        return;
    }
    fn_1_22A8(arg0, 0);
    fn_1_23C4(arg0, 0, 8.0f);
    temp_r31->unk1C = 8;
    temp_r31->unk44 = 0.0f;
    temp_r31->flag50 = 0;
}

void fn_1_48C0(s16 arg0)
{
    M650Player *temp_r31;

    temp_r31 = &lbl_1_bss_28[arg0];
    if (temp_r31->unk18++ == 0) {
        fn_1_22A8(arg0, 3);
    }
    if (temp_r31->unk18 >= 30) {
        fn_1_22A8(arg0, 0);
        temp_r31->unk1C = 8;
        temp_r31->unk18 = 0;
    }
}

void fn_1_49C8(s16 arg0)
{
    Point3d sp8;
    M650Player *temp_r31;
    s16 temp_r28;

    temp_r31 = &lbl_1_bss_28[arg0];
    temp_r28 = MgSeqModeGet();
    Hu3DModelRotGet(temp_r31->model20, &sp8);
    if (sp8.y < 0.0f) {
        sp8.y += 2.0f;
        if (sp8.y > 0.0f) {
            sp8.y = 0.0f;
        }
    } else if (sp8.y > 0.0f) {
        sp8.y -= 2.0f;
        if (sp8.y < 0.0f) {
            sp8.y = 0.0f;
        }
    }
    Hu3DModelRotSetV(temp_r31->model20, &sp8);
    if (sp8.y == 0.0f) {
        temp_r31->unk48 = 0.0f;
        if (temp_r28 == 5) {
            if (lbl_1_bss_28[arg0].unk74 == 0) {
                temp_r31->unk1C = 1;
                temp_r31->unk44 = 0.0f;
                fn_1_289C(arg0);
            } else {
                temp_r31->unk1C = 10;
            }
        } else {
            temp_r31->unk1C = 9;
            fn_1_22A8(arg0, 0);
        }
        temp_r31->pos.x = temp_r31->pos.y = temp_r31->pos.z = 0.0f;
    }
}

void fn_1_4BD4(s16 arg0)
{
    Point3d spC;
    s16 sp8;
    M650Player *temp_r31;

    temp_r31 = &lbl_1_bss_28[arg0];
    sp8 = MgSeqModeGet();
    Hu3DModelRotGet(temp_r31->model20, &spC);
    if (spC.y < 180.0f) {
        spC.y += 4.0f;
        if (spC.y > 180.0f) {
            spC.y = 180.0f;
        }
    } else if (spC.y > -180.0f) {
        spC.y -= 4.0f;
        if (spC.y < -180.0f) {
            spC.y = 180.0f;
        }
    }
    Hu3DModelRotSetV(temp_r31->model20, &spC);
    if (spC.y == 180.0f) {
        temp_r31->unk1C = 9;
        fn_1_22A8(arg0, 3);
    }
}

s16 fn_1_4D58(s16 arg0)
{
    u16 cameras[4] = { 1, 2, 4, 8 };
    /* Each axis carries the viewport origin and extent. */
    f32 viewX[2];
    f32 viewY[2];
    s32 var_r31;
    f32 temp_f31;
    f32 temp_f30;
    f32 temp_f29;
    f32 temp_f28;

    temp_f31 = 640.0f - (5.3333335f * (f32) arg0);
    temp_f30 = 480.0f - (4.0f * (f32) arg0);
    temp_f29 = (16.0f * (f32) arg0) / 60.0f;
    temp_f28 = (40.0f * (f32) arg0) / 60.0f;
    var_r31 = 0;
    while (var_r31 < 4) {
        switch (var_r31) {                          /* irregular */
        case 0:
            viewX[0] = 0.0f;
            viewY[0] = 0.0f;
            viewX[1] = temp_f31 + temp_f29;
            viewY[1] = temp_f30 + temp_f28;
            Hu3DCameraScissorSet(cameras[var_r31], 0U, 0U, (u32) (temp_f31 - 2.0f), (u32) (temp_f30 - 2.0f));
            break;
        case 1:
            viewX[0] = temp_f31 - temp_f29;
            viewY[0] = 0.0f;
            viewX[1] = (640.0f - temp_f31) + temp_f29;
            viewY[1] = temp_f30 + temp_f28;
            Hu3DCameraScissorSet(cameras[var_r31], (u32) (2.0f + temp_f31), 0U, (u32) ((640.0f - temp_f31) - 2.0f), (u32) (temp_f30 - 2.0f));
            break;
        case 2:
            viewX[0] = 0.0f;
            viewY[0] = temp_f30 - temp_f28;
            viewX[1] = temp_f31 + temp_f29;
            viewY[1] = (480.0f - temp_f30) + temp_f28;
            Hu3DCameraScissorSet(cameras[var_r31], 0U, (u32) (2.0f + temp_f30), (u32) (temp_f31 - 2.0f), (u32) ((480.0f - temp_f30) - 2.0f));
            break;
        case 3:
            viewX[0] = temp_f31 - temp_f29;
            viewY[0] = temp_f30 - temp_f28;
            viewX[1] = (640.0f - temp_f31) + temp_f29;
            viewY[1] = (480.0f - temp_f30) + temp_f28;
            Hu3DCameraScissorSet(cameras[var_r31], (u32) (2.0f + temp_f31), (u32) (2.0f + temp_f30), (u32) ((640.0f - temp_f31) - 2.0f), (u32) ((480.0f - temp_f30) - 2.0f));
            break;
        }
        Hu3DCameraViewportSet(cameras[var_r31], viewX[0], viewY[0], viewX[1], viewY[1], 0.0f, 1.0f);
        var_r31 += 1;
    }
    if (arg0 >= 60) {
        return 1;
    }
    return 0;
}

void fn_1_5258(void)
{
    if (_CheckFlag(196610U) == 0) {
        lbl_1_bss_0.record = GWRecordGet(GW_RECORD_M650);
        if ((s32) lbl_1_bss_0.record == 0) {
            lbl_1_bss_0.record = 3600;
        }
        lbl_1_bss_0.timer = MgTimerCreate(1);
        MgTimerParamSet(lbl_1_bss_0.timer, 0, 18000, lbl_1_bss_0.record);
        MgTimerRecordDispOn(lbl_1_bss_0.timer);
    }
}

void fn_1_5308(void)
{
    if (_CheckFlag(196610U) == 0) {
        MgTimerModeOnSet(lbl_1_bss_0.timer, 0);
        HuAudFXPlay(13);
    }
}

void fn_1_5354(void)
{
    if (_CheckFlag(196610U) == 0) {
        MgTimerModeOffSet(lbl_1_bss_0.timer);
    }
}

void fn_1_5394(void)
{
    HuVecF positions[4] = {
        { 76.0f, 224.0f, 0.0f },
        { 516.0f, 224.0f, 0.0f },
        { 76.0f, 424.0f, 0.0f },
        { 516.0f, 424.0f, 0.0f },
    };
    s32 i;

    if (_CheckFlag(196610U) != 0) {
        i = 0;
        while (i < 4) {
            lbl_1_bss_28[i].timer = MgTimerCreate(2);
            MgTimerParamSet(lbl_1_bss_28[i].timer, 0, 5400, 0);
            MgTimerPosSet(lbl_1_bss_28[i].timer, positions[i].x, positions[i].y);
            MgTimerRecordDispOn(lbl_1_bss_28[i].timer);
            i += 1;
        }
    }
}

void fn_1_54D4(void)
{
    s32 var_r31;

    if (_CheckFlag(196610U) != 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            MgTimerModeOnSet(lbl_1_bss_28[var_r31].timer, 2);
            var_r31 += 1;
        }
        HuAudFXPlay(13);
    }
}

void fn_1_5544(s16 arg0)
{
    if (_CheckFlag(196610U) != 0) {
        MgTimerModeOffSet(lbl_1_bss_28[arg0].timer);
        if ((s32) lbl_1_bss_28[arg0].timer->stopF == 0) {
            lbl_1_bss_28[arg0].timer->stopF = 1;
            lbl_1_bss_28[arg0].timer->mode = 1;
        }
    }
}

void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1)
{
    f32 height;

    height = 5.444444768958626 - value / 100.0f;
    *out1 = (f32) (180.0 * (atan2(height, 8.300000190734863) / 3.141592653589793));
    if (*out1 < 90.0f) {
        *out0 = (f32) (8.300000190734863 / (0.3333333432674408 * cos((3.141592653589793 * (f64) *out1) / 180.0)));
        return;
    }
    *out0 = 0.0f;
}

s16 fn_1_5728(void)
{
    Point3d sp8;
    s16 temp_r29;
    s32 var_r31;
    s32 temp_r30;

    temp_r29 = MgSeqModeGet();
    if (temp_r29 != 5) {
        return 0;
    }
    if (_CheckFlag(196610U) != 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            Hu3DModelPosGet(lbl_1_bss_28[var_r31].model20, &sp8);
            if (sp8.z >= 5000.0f) {
                lbl_1_bss_28[var_r31].unk76 = MgTimerValueGet(lbl_1_bss_28[var_r31].timer);
                fn_1_5544(var_r31);
                lbl_1_bss_28[var_r31].unk74 = 1;
            }
            var_r31 += 1;
        }
        var_r31 = 0;
        while (var_r31 < 4) {
            if (lbl_1_bss_28[var_r31].unk74 != 0) {
                var_r31 += 1;
            } else {
                break;
            }
        }
        if (var_r31 >= 4) {
            return 1;
        }
    } else {
        var_r31 = 0;
        while (var_r31 < 4) {
            Hu3DModelPosGet(lbl_1_bss_28[var_r31].model20, &sp8);
            if (sp8.z >= 5000.0f) {
                lbl_1_bss_0.winner = var_r31;
                if (_CheckFlag(65551U) == 0) {
                    GwPlayer[var_r31].mgCoinBonus = 10;
                }
                temp_r30 = MgTimerValueGet(lbl_1_bss_0.timer);
                if ((lbl_1_bss_28[var_r31].unk14 == -1) &&
                    (temp_r30 < lbl_1_bss_0.record) &&
                    (_CheckFlag(65551U) == 0)) {
                    MgSeqRecordSet(temp_r30);
                    lbl_1_bss_0.recordChanged = 1;
                    GWRecordSet(GW_RECORD_M650, (u32) temp_r30);
                }
                fn_1_5354();
                return 1;
            }
            var_r31 += 1;
        }
    }
    return 0;
}

void fn_1_5A24(s16 arg0)
{
    OM_CAMERA_VIEW view;
    Point3d pos;
    M650Player *player;
    s16 seqMode;
    s16 time;

    player = &lbl_1_bss_28[arg0];
    seqMode = MgSeqModeGet();
    if (seqMode == 5) {
        Hu3DModelPosGet(player->model20, &pos);
        switch (player->unk1C) {                  /* irregular */
        case 1:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -27.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1300.0f;
            time = 30;
            break;
        case 2:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -8.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 500.0f;
            time = 30;
            break;
        case 3:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -8.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -15.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        case 10:
            view.center.x = pos.x;
            view.center.y = 180.0f;
            view.center.z = pos.z;
            view.rot.x = -11.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 2000.0f;
            time = 30;
            break;
        case 9:
            if (_CheckFlag(196610U) != 0) {
                view.center.x = pos.x;
                view.center.y = 180.0f;
                view.center.z = pos.z;
                view.rot.x = -11.0f;
                view.rot.y = -180.0f;
                view.rot.z = 0.0f;
                view.zoom = 2000.0f;
                time = 30;
            } else {
                view.center.x = pos.x;
                view.center.y = 280.0f;
                view.center.z = pos.z;
                view.rot.x = -15.0f;
                view.rot.y = -180.0f;
                view.rot.z = 0.0f;
                view.zoom = 1000.0f;
                time = 30;
            }
            break;
        default:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -15.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        }
        omCameraViewMoveSimpleMulti(lbl_1_data_150[arg0], &view, time);
    }
}

void fn_1_5E40(void)
{
    s32 i;

    if ((HuPadBtn[0] & 8192) != 0) {
        for (i = 0; i < 4; i++) {
            if ((HuPadBtn[0] & 1024) != 0) {
                CZoomM[i] += 5.0f;
            }
            if ((HuPadBtn[0] & 2048) != 0) {
                CZoomM[i] -= 5.0f;
            }
            if (HuPadSubStkY[0] > 32) {
                CRotM[i].x += 1.0f;
            }
            if (HuPadSubStkY[0] < -32) {
                CRotM[i].x -= 1.0f;
            }
            if (HuPadSubStkX[0] > 32) {
                CRotM[i].y += 1.0f;
            }
            if (HuPadSubStkX[0] < -32) {
                CRotM[i].y -= 1.0f;
            }
            if ((HuPadStkX[0] > 5) || (HuPadStkX[0] < -5)) {
                CenterM[i].x += HuPadStkX[0];
            }
            if ((HuPadStkY[0] > 5) || (HuPadStkY[0] < -5)) {
                CenterM[i].z -= HuPadStkY[0];
            }
            if ((HuPadBtn[0] & 8) != 0) {
                CenterM[i].y += 5.0f;
            }
            if ((HuPadBtn[0] & 4) != 0) {
                CenterM[i].y -= 5.0f;
            }
        }
        print8(100, 300, 1.8f, "CnrX:%f", CenterM->x);
        print8(100, 316, 1.8f, "CnrY:%f", CenterM->y);
        print8(100, 332, 1.8f, "CnrZ:%f", CenterM->z);
        print8(100, 348, 1.8f, "RotZ:%f", CRotM->x);
        print8(100, 364, 1.8f, "RotY:%f", CRotM->y);
        print8(100, 380, 1.8f, "RotZ:%f", CRotM->z);
        print8(100, 396, 1.8f, "Zoom:%f", CZoomM[0]);
    }
}
