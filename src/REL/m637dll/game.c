#include "REL/m637dll.h"

u32 lbl_1_data_28[9] = {
    9633792,
    9633793,
    9633794,
    9306218,
    9306219,
    9633828,
    9633829,
    9633830,
    9633832,
};
s32 lbl_1_data_4C[4] = { 12, 7, 2, 0 };
s32 lbl_1_data_5C[3] = { 15, 25, 1 };
char lbl_1_data_68[13] = { 105, 116, 101, 109, 104, 111, 111, 107, 95, 115, 97, 111, 0 };

s32 lbl_1_bss_374;
s32 lbl_1_bss_370;
s32 lbl_1_bss_36C;
s32 lbl_1_bss_368;
s32 lbl_1_bss_364;
s32 lbl_1_bss_360;
s32 lbl_1_bss_35C;
s32 lbl_1_bss_358;
s32 lbl_1_bss_354;
s32 lbl_1_bss_350;
MGTIMER *lbl_1_bss_34C;
OM_CAMERA_VIEW lbl_1_bss_330;
M637Record2A0 lbl_1_bss_2A0[4];
s32 lbl_1_bss_290[4];
M637Record1D0 lbl_1_bss_1D0[2];
M637Record118 lbl_1_bss_118[2];
s32 lbl_1_bss_114;
s32 lbl_1_bss_110;
s16 lbl_1_bss_10E;
s16 lbl_1_bss_10C;
s16 lbl_1_bss_10A;
s16 lbl_1_bss_108;
s32 lbl_1_bss_104;
f32 lbl_1_bss_100;
f32 lbl_1_bss_F8[2];
M637RecordB8 lbl_1_bss_B8[2];
M637Record08 lbl_1_bss_8[4];

void fn_1_548(void)
{
    fn_1_181C();
    lbl_1_bss_1D0->field18[0] = lbl_1_bss_1D0[1].field18[0] = lbl_1_bss_118->field48 = frandmod(8);
    lbl_1_bss_1D0->field18[1] = lbl_1_bss_1D0[1].field18[1] = lbl_1_bss_118[1].field48 = frandmod(8);
    fn_1_1CF0();
    fn_1_2D7C();
    fn_1_3460();
    lbl_1_bss_374 = 0;
    lbl_1_bss_370 = 0;
    lbl_1_bss_36C = -1;
    lbl_1_bss_364 = 0;
    lbl_1_bss_360 = 0;
    lbl_1_bss_35C = 0;
    lbl_1_bss_358 = 0;
    lbl_1_bss_354 = 0;
    lbl_1_bss_350 = -1;
}


s32 fn_1_65C(void)
{
    s32 var_r31;
    s32 var_r30;

    var_r30 = 0;
    switch ((s32) lbl_1_bss_374) {                  /* irregular */
    case 0:
        if ((s32) lbl_1_bss_36C == -1) {
            lbl_1_bss_36C = 1;
        }
        if ((s32) lbl_1_bss_36C != 0) {
            fn_1_5914(1);
        }
        if (fn_1_4C24((lbl_1_bss_36C * 5) + 10) != 0) {
            lbl_1_bss_34C = MgTimerCreate(0);
            MgTimerParamSet(lbl_1_bss_34C, 900, 0, 0);
            MgTimerPosSet(lbl_1_bss_34C, 288.0f, 410.0f);
            lbl_1_bss_36C = 0;
            lbl_1_bss_374 += 1;
            HuAudFXPlay(1881);
        }
        break;
    case 1:
        MgTimerModeOnSet(lbl_1_bss_34C, 1);
        var_r31 = 0;
        while (var_r31 < 4) {
            lbl_1_bss_8[var_r31].field04 = 0;
            lbl_1_bss_8[var_r31].field0C = lbl_1_data_4C[lbl_1_bss_8[var_r31].field00];
            omVibrate((s16) var_r31, 20, 7, 3);
            var_r31 += 1;
        }
        lbl_1_bss_374 += 1;
        break;
    case 2:
        fn_1_6BD4();
        if (MgTimerDoneCheck(lbl_1_bss_34C) != 0) {
            lbl_1_bss_368 = 0;
            lbl_1_bss_370 += 1;
            lbl_1_bss_374 += 3;
        } else if (fn_1_36E8(1) != 0) {
            lbl_1_bss_368 = 0;
            MgTimerModeOffSet(lbl_1_bss_34C);
            lbl_1_bss_370 += 1;
            lbl_1_bss_374 += 1;
        }
        break;
    case 3:
        if ((fn_1_36E8(0) != 0) && (fn_1_603C() != 0)) {
            lbl_1_bss_374 += 1;
        }
        break;
    case 4:
        if (((s32) lbl_1_bss_370 == 5) || (lbl_1_bss_2A0->count20 == 3) || (lbl_1_bss_2A0[2].count20 == 3)) {
            MgTimerKill(lbl_1_bss_34C);
            var_r30 = 1;
            HuAudSStreamFadeOut(lbl_1_bss_350, 100);
        } else if (fn_1_4C24(5) != 0) {
            MgTimerKill(lbl_1_bss_34C);
            lbl_1_bss_374 = 0;
        }
        break;
    case 5:
        if ((fn_1_36E8(0) != 0) && (fn_1_65A0() != 0)) {
            lbl_1_bss_374 -= 1;
        }
        break;
    }
    return var_r30;
}


void fn_1_A54(void)
{
    if ((s32) lbl_1_bss_364 == 0) {
        lbl_1_bss_354 = 1;
        if (++lbl_1_bss_360 < 60) {
            lbl_1_bss_1D0->field10[0] = 4.0f;
            lbl_1_bss_1D0->field10[1] = 4.0f;
        }
        if (fn_1_36E8(0) != 0) {
            lbl_1_bss_364 += 1;
            lbl_1_bss_360 = 0;
            return;
        }
    } else {
        lbl_1_bss_354 = 2;
        if (++lbl_1_bss_360 < 60) {
            lbl_1_bss_1D0->field10[0] = -4.0f;
            lbl_1_bss_1D0->field10[1] = -4.0f;
        }
        fn_1_36E8(0);
    }
}


void fn_1_B94(void)
{
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;

    fn_1_36E8(0);
    if (lbl_1_bss_2A0->count20 > lbl_1_bss_2A0[2].count20) {
        MgSeqWinnerSet(GwPlayerConf[lbl_1_bss_290[0]].charNo, GwPlayerConf[lbl_1_bss_290[1]].charNo, -1, -1);
        var_r31 = lbl_1_bss_290[0];
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[var_r31].mgCoinBonus = 10;
        }
        var_r30 = lbl_1_bss_290[1];
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[var_r30].mgCoinBonus = 10;
        }
        CharModelVoiceFlagSet(GwPlayerConf[lbl_1_bss_290[0]].charNo, 1);
        CharModelVoiceFlagSet(GwPlayerConf[lbl_1_bss_290[1]].charNo, 1);
        return;
    }
    if (lbl_1_bss_2A0->count20 < lbl_1_bss_2A0[2].count20) {
        MgSeqWinnerSet(GwPlayerConf[lbl_1_bss_290[2]].charNo, GwPlayerConf[lbl_1_bss_290[3]].charNo, -1, -1);
        var_r29 = lbl_1_bss_290[2];
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[var_r29].mgCoinBonus = 10;
        }
        var_r28 = lbl_1_bss_290[3];
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[var_r28].mgCoinBonus = 10;
        }
        CharModelVoiceFlagSet(GwPlayerConf[lbl_1_bss_290[2]].charNo, 1);
        CharModelVoiceFlagSet(GwPlayerConf[lbl_1_bss_290[3]].charNo, 1);
        return;
    }
    MgSeqWinnerSet(-1, -1, -1, -1);
}


void fn_1_E4C(void)
{
    if (lbl_1_bss_2A0->count20 > lbl_1_bss_2A0[2].count20) {
        CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[0]].charNo, lbl_1_bss_2A0->motion[7], 0.0f, 8.0f, 0U);
        CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[1]].charNo, lbl_1_bss_2A0[1].motion[7], 0.0f, 8.0f, 0U);
        return;
    }
    if (lbl_1_bss_2A0->count20 < lbl_1_bss_2A0[2].count20) {
        CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[2]].charNo, lbl_1_bss_2A0[2].motion[7], 0.0f, 8.0f, 0U);
        CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[3]].charNo, lbl_1_bss_2A0[3].motion[7], 0.0f, 8.0f, 0U);
        return;
    }
    CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[0]].charNo, lbl_1_bss_2A0->motion[8], 0.0f, 8.0f, 0U);
    CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[1]].charNo, lbl_1_bss_2A0[1].motion[8], 0.0f, 8.0f, 0U);
    CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[2]].charNo, lbl_1_bss_2A0[2].motion[8], 0.0f, 8.0f, 0U);
    CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[3]].charNo, lbl_1_bss_2A0[3].motion[8], 0.0f, 8.0f, 0U);
}


s32 fn_1_1110(s32 arg0)
{
    s32 var_r31;
    s32 var_r30;
    HuVecF *var_r29;
    HuVecF *var_r28;
    HuVecF *var_r27;
    HuVecF *var_r26;
    HuVecF *var_r25;
    HuVecF *var_r24;
    HuVecF *var_r23;
    HuVecF *var_r22;

    var_r30 = 0;
    var_r31 = 0;
    if (arg0 == 0) {
        {
            HuVecF field5C = { -400.0f, -20.0f, 0.0f };
            var_r29 = &field5C;
            lbl_1_bss_330.center = *var_r29;
        }
        {
            HuVecF field50 = { 0.0f, 40.0f, 0.0f };
            var_r28 = &field50;
            lbl_1_bss_330.rot = *var_r28;
        }
        lbl_1_bss_330.zoom = 1000.0f;
        omCameraViewSet(&lbl_1_bss_330);
    } else if (arg0 == 1) {
        {
            HuVecF field44 = { 0.0f, 0.0f, 0.0f };
            var_r27 = &field44;
            lbl_1_bss_330.center = *var_r27;
        }
        {
            HuVecF field38 = { -3.5511f, 0.0f, 0.0f };
            var_r26 = &field38;
            lbl_1_bss_330.rot = *var_r26;
        }
        lbl_1_bss_330.zoom = 1550.0f;
        var_r31 = 1;
    } else if (arg0 == 2) {
        if (lbl_1_bss_2A0->count20 > lbl_1_bss_2A0[2].count20) {
            {
                HuVecF field2C = { -400.0f, -20.0f, 0.0f };
                var_r25 = &field2C;
                lbl_1_bss_330.center = *var_r25;
            }
            {
                HuVecF field20 = { -15.0f, 40.0f, 0.0f };
                var_r24 = &field20;
                lbl_1_bss_330.rot = *var_r24;
            }
            lbl_1_bss_330.zoom = 800.0f;
            var_r31 = 1;
        } else if (lbl_1_bss_2A0->count20 < lbl_1_bss_2A0[2].count20) {
            {
                HuVecF field14 = { 400.0f, -20.0f, 0.0f };
                var_r23 = &field14;
                lbl_1_bss_330.center = *var_r23;
            }
            {
                HuVecF field08 = { -15.0f, -40.0f, 0.0f };
                var_r22 = &field08;
                lbl_1_bss_330.rot = *var_r22;
            }
            lbl_1_bss_330.zoom = 800.0f;
            var_r31 = 1;
        }
    }
    if (var_r31 != 0) {
        omCameraViewMoveSimple(&lbl_1_bss_330, 100);
        var_r30 = 100;
    }
    return var_r30;
}


void fn_1_1464(s32 arg0)
{
    HuVecF sp8;
    s32 var_r31;
    s32 var_r30;

    if (arg0 != 0) {
        if (arg0 == 1) {
            if (++lbl_1_bss_35C == 56) {
                HuAudFXPlay(1876);
            }
            lbl_1_bss_100 = (f32) (650.0 * sin((3.141592653589793 * (f64) lbl_1_bss_F8[0]) / 180.0));
            Hu3DModelPosSet(lbl_1_bss_10C, 0.0f, 950.0f - lbl_1_bss_100, -300.0f);
            lbl_1_bss_F8[0] += 0.9f;
        } else if (arg0 == 2) {
            if (lbl_1_bss_F8[1] <= 90.0f) {
                lbl_1_bss_100 = (f32) (50.0 * sin((3.141592653589793 * (f64) lbl_1_bss_F8[1]) / 180.0));
                Hu3DModelPosSet(lbl_1_bss_10C, 0.0f, 300.0f + lbl_1_bss_100, -300.0f);
                lbl_1_bss_F8[1] += 1.125f;
                if (lbl_1_bss_F8[1] > 90.0f) {
                    lbl_1_bss_100 = 600.0f;
                }
            }
            if ((s32) lbl_1_bss_350 == -1) {
                lbl_1_bss_350 = HuAudBGMPlay(83);
            }
        } else {
            lbl_1_bss_100 -= 6.0f;
            Hu3DModelPosSet(lbl_1_bss_10C, 0.0f, 950.0f - lbl_1_bss_100, -300.0f);
        }
    }
    Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &sp8);
    sp8.y -= 80.0f;
    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        var_r30 = 0;
        while (var_r30 < 8) {
            Hu3DModelPosSetV(lbl_1_bss_118[var_r31].pair[var_r30][0], &sp8);
            Hu3DModelPosSetV(lbl_1_bss_118[var_r31].pair[var_r30][1], &sp8);
            var_r30 += 1;
        }
    }
    Hu3DModelPosSetV(lbl_1_bss_10E, &sp8);
    Hu3DModelPosSetV(lbl_1_bss_10A, &sp8);
}


s32 fn_1_1814(void)
{
    return 100;
}


void fn_1_181C(void)
{
    f32 sp8;
    s32 var_r31;
    s32 var_r30;
    s16 var_r29;
    s32 var_r28;
    s32 var_r27;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    var_r28 = 0;
    var_r27 = 2;
    var_r31 = 0;
    while (var_r31 < 4) {
        if (GwPlayerConf[var_r31].grpNo == 0) {
            lbl_1_bss_290[var_r28++] = var_r31;
        } else {
            lbl_1_bss_290[var_r27++] = var_r31;
        }
        var_r31 += 1;
    }
    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        var_r29 = GwPlayerConf[lbl_1_bss_290[var_r31]].charNo;
        if (var_r31 / 2 != 0) {
            var_f28 = 400.0f;
        } else {
            var_f28 = -400.0f;
        }
        var_f29 = var_f28;
        if (var_r31 % 2 != 0) {
            var_f27 = 125.0f;
        } else {
            var_f27 = -125.0f;
        }
        var_f30 = var_f27;
        sp8 = var_f29 + var_f30;
        if (var_r31 < 2) {
            var_f26 = 40.0f;
        } else {
            var_f26 = -40.0f;
        }
        var_f31 = var_f26;
        lbl_1_bss_2A0[var_r31].model = CharModelCreate(var_r29, 2);
        Hu3DModelPosSet(lbl_1_bss_2A0[var_r31].model, (f32) ((f64) var_f29 + ((f64) var_f30 * cos((3.141592653589793 * -var_f31) / 180.0))), -20.0f, (f32) (var_f30 * sin((3.141592653589793 * -var_f31) / 180.0)));
        Hu3DModelRotSet(lbl_1_bss_2A0[var_r31].model, 0.0f, var_f31, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2A0[var_r31].model, 1U);
        Hu3DModelAttrSet(lbl_1_bss_2A0[var_r31].model, 1073741825U);
        Hu3DModelShadowSet(lbl_1_bss_2A0[var_r31].model);
        var_r30 = 0;
        while (var_r30 < 9) {
            lbl_1_bss_2A0[var_r31].motion[var_r30] = CharMotionCreate(var_r29, lbl_1_data_28[var_r30]);
            var_r30 += 1;
        }
        lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[0];
        CharMotionSet(var_r29, lbl_1_bss_2A0[var_r31].selectedMotion);
        CharMotionDataClose(var_r29);
        CharModelVoiceFlagSet(var_r29, 0);
        lbl_1_bss_2A0[var_r31].field18 = 0;
        lbl_1_bss_2A0[var_r31].count20 = 0;
        lbl_1_bss_8[var_r31].field00 = (s32) GwPlayerConf[lbl_1_bss_290[var_r31]].comDif;
        lbl_1_bss_8[var_r31].field24 = lbl_1_data_5C[0];
        lbl_1_bss_8[var_r31].field14 = var_r31 % 2;
        lbl_1_bss_8[var_r31].field18 = var_r31 / 2;
        lbl_1_bss_8[var_r31].field1C = var_r31 % 2;
    }
    HuPrcChildCreate(fn_1_67C4, 100U, 8192U, 0, HuPrcCurrentGet());
}


void fn_1_1CF0(void)
{
    s32 var_r31;
    s32 var_r30;
    s16 var_r29;
    s16 var_r28;
    f32 var_f31;
    f32 var_f30;

    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        if (var_r31 == 0) {
            var_r29 = 4;
            var_f30 = -400.0f;
            var_f31 = 40.0f;
        } else {
            var_r29 = 5;
            var_f30 = 400.0f;
            var_f31 = -40.0f;
        }
        var_r28 = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(var_r28, 1U);
        Hu3DModelPosSet(var_r28, var_f30, -200.0f, 0.0f);
        Hu3DModelRotSet(var_r28, 0.0f, var_f31, 0.0f);
        Hu3DModelLayerSet(var_r28, 3);
        var_r30 = 0;
        while (var_r30 < 2) {
            var_r29 = var_r30 == 0 ? 21 : 23;
            lbl_1_bss_1D0[var_r31].pair[var_r30][0] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0], 1U);
            Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0], var_f30, -200.0f, 0.0f);
            Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0], 0.0f, var_f31, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0], 3);
            Hu3DModelShadowMapSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0]);
            var_r29 = var_r30 == 0 ? 20 : 22;
            lbl_1_bss_1D0[var_r31].pair[var_r30][1] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1], 1U);
            Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1], var_f30, -200.0f, 0.0f);
            Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1], 0.0f, var_f31, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1], 3);
            Hu3DModelShadowMapSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1]);
            if (((s32) lbl_1_bss_1D0[var_r31].field18[var_r30] == 0) || ((s32) lbl_1_bss_1D0[var_r31].field18[var_r30] >= 5)) {
                lbl_1_bss_1D0[var_r31].field28[var_r30] = 0;
                Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].pair[var_r30][1], 1U);
            } else {
                lbl_1_bss_1D0[var_r31].field28[var_r30] = 1;
                Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].pair[var_r30][0], 1U);
            }
            lbl_1_bss_1D0[var_r31].field08[var_r30] = -60.0f * (f32) lbl_1_bss_1D0[var_r31].field18[var_r30];
            if (lbl_1_bss_1D0[var_r31].field08[var_r30] <= -240.0f) {
                lbl_1_bss_1D0[var_r31].field08[var_r30] += 240.0f;
            }
            if (lbl_1_bss_1D0[var_r31].field08[var_r30] <= -30.0f) {
                lbl_1_bss_1D0[var_r31].field08[var_r30] += 240.0f;
            }
            if (lbl_1_bss_1D0[var_r31].field08[var_r30] >= 210.0f) {
                lbl_1_bss_1D0[var_r31].field08[var_r30] -= 240.0f;
            }
            Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], -lbl_1_bss_1D0[var_r31].field08[var_r30], var_f31, 0.0f);
            lbl_1_bss_1D0[var_r31].field30[var_r30] = -60.0f * (f32) lbl_1_bss_1D0[var_r31].field18[var_r30];
            lbl_1_bss_1D0[var_r31].field10[var_r30] = 0.0f;
            lbl_1_bss_1D0[var_r31].field20[var_r30] = 1;
            var_r30 += 1;
        }
        lbl_1_bss_1D0[var_r31].field38 = Hu3DModelCreate(HuDataSelHeapReadNum(6291462, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].field38, 1U);
        Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].field38, var_f30, -200.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].field38, 0.0f, var_f31, 0.0f);
        Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].field38, 3);
        Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field38, 1073741825U);
        Hu3DMotionSpeedSet(lbl_1_bss_1D0[var_r31].field38, 0.0f);
        var_r29 = var_r31 == 0 ? 7 : 8;
        lbl_1_bss_1D0[var_r31].field3A = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].field3A, 1U);
        Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].field3A, var_f30, -200.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].field3A, 0.0f, var_f31, 0.0f);
        Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].field3A, 3);
        lbl_1_bss_1D0[var_r31].field3C = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field3A, HuDataSelHeapReadNum(6291465, 268435456, HEAP_MODEL));
        lbl_1_bss_1D0[var_r31].field3E = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field3A, HuDataSelHeapReadNum(6291466, 268435456, HEAP_MODEL));
        lbl_1_bss_1D0[var_r31].field40 = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field3A, HuDataSelHeapReadNum(6291467, 268435456, HEAP_MODEL));
        Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field3A, lbl_1_bss_1D0[var_r31].field3C);
        Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field3A, 1073741825U);
        lbl_1_bss_1D0[var_r31].field44 = 0;
        var_r30 = 0;
        while (var_r30 < 2) {
            if (var_r31 == 0) {
                var_r29 = var_r30 == 0 ? 12 : 13;
            } else {
                var_r29 = var_r30 == 0 ? 14 : 15;
            }
            lbl_1_bss_1D0[var_r31].field48[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].field48[var_r30], 1U);
            Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].field48[var_r30], var_f30, -200.0f, 0.0f);
            Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].field48[var_r30], 0.0f, var_f31, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].field48[var_r30], 3);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field48[var_r30], 1073741825U);
            var_r29 = var_r30 == 0 ? 16 : 17;
            lbl_1_bss_1D0[var_r31].field4C[var_r30][0] = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field48[var_r30], HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            var_r29 = var_r30 == 0 ? 18 : 19;
            lbl_1_bss_1D0[var_r31].field4C[var_r30][1] = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field48[var_r30], HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field48[var_r30], lbl_1_bss_1D0[var_r31].field4C[var_r30][0]);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field48[var_r30], 1073741825U);
            var_r30 += 1;
        }
        var_r30 = 0;
        while (var_r30 < 2) {
            var_r29 = var_r30 == 0 ? 58 : 60;
            lbl_1_bss_1D0[var_r31].field54[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 1U);
            Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].field54[var_r30], var_f30, -200.0f, 0.0f);
            Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 0.0f, var_f31, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 3);
            var_r29 = var_r30 == 0 ? 59 : 61;
            lbl_1_bss_1D0[var_r31].field58[var_r30] = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field54[var_r30], HuDataSelHeapReadNum(var_r29 + 6291456, 268435456, HEAP_MODEL));
            Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field54[var_r30], lbl_1_bss_1D0[var_r31].field58[var_r30]);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 1073741825U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 1U);
            var_r30 += 1;
        }
        lbl_1_bss_1D0[var_r31].field5C = Hu3DModelCreate(HuDataSelHeapReadNum(6291518, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_1D0[var_r31].field5C, 1U);
        Hu3DModelPosSet(lbl_1_bss_1D0[var_r31].field5C, var_f30, -200.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].field5C, 0.0f, var_f31, 0.0f);
        Hu3DModelLayerSet(lbl_1_bss_1D0[var_r31].field5C, 3);
        lbl_1_bss_1D0[var_r31].field5E = Hu3DJointMotion(lbl_1_bss_1D0[var_r31].field5C, HuDataSelHeapReadNum(6291519, 268435456, HEAP_MODEL));
        Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field5C, lbl_1_bss_1D0[var_r31].field5E);
        Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field5C, 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field5C, 1U);
    }
}


void fn_1_2D7C(void)
{
    HuVecF sp8;
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s16 var_r28;
    s16 var_r27;

    lbl_1_bss_10C = Hu3DModelCreate(HuDataSelHeapReadNum(6291513, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_10C, 1U);
    Hu3DModelPosSet(lbl_1_bss_10C, 0.0f, 950.0f, -300.0f);
    Hu3DModelRotSet(lbl_1_bss_10C, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(lbl_1_bss_10C, 1073741825U);
    Hu3DModelLayerSet(lbl_1_bss_10C, 2);
    Hu3DModelScaleSet(lbl_1_bss_10C, 1.3f, 1.3f, 1.0f);
    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        lbl_1_bss_118[var_r31].field50 = lbl_1_bss_118[var_r31].field48;
        lbl_1_bss_118[var_r31].field54 = lbl_1_bss_118[var_r31].field48;
        lbl_1_bss_118[var_r31].field58 = lbl_1_bss_118[var_r31].field48;
        var_r30 = 0;
        while (var_r30 < 8) {
            lbl_1_bss_118[var_r31].value[var_r30] = 950.0f;
            Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &sp8);
            sp8.y -= 280.0f;
            if (var_r31 == 0) {
                var_r28 = 25;
            } else {
                var_r28 = 41;
            }
            var_r29 = var_r28;
            lbl_1_bss_118[var_r31].pair[var_r30][0] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456 + var_r30, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_118[var_r31].pair[var_r30][0], 1U);
            Hu3DModelPosSetV(lbl_1_bss_118[var_r31].pair[var_r30][0], &sp8);
            Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[var_r30][0], 0.0f, 0.0f, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_118[var_r31].pair[var_r30][0], 2);
            if (var_r30 != lbl_1_bss_118[var_r31].field48) {
                Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[var_r30][0], 1U);
            }
            if (var_r31 == 0) {
                var_r27 = 33;
            } else {
                var_r27 = 49;
            }
            var_r29 = var_r27;
            lbl_1_bss_118[var_r31].pair[var_r30][1] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r29 + 6291456 + var_r30, 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_118[var_r31].pair[var_r30][1], 1U);
            Hu3DModelPosSetV(lbl_1_bss_118[var_r31].pair[var_r30][1], &sp8);
            Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[var_r30][1], 0.0f, 0.0f, 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_118[var_r31].pair[var_r30][1], 2);
            if (var_r30 != lbl_1_bss_118[var_r31].field48) {
                Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[var_r30][1], 1U);
            }
            lbl_1_bss_118[var_r31].field4C = lbl_1_bss_118[var_r31].field48;
            var_r30 += 1;
        }
    }
    lbl_1_bss_114 = 0;
    lbl_1_bss_110 = 0;
    lbl_1_bss_F8[0] = lbl_1_bss_F8[1] = 0.0f;
    lbl_1_bss_10E = Hu3DModelCreate(HuDataSelHeapReadNum(6291480, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_10E, 1U);
    Hu3DModelObjPosGet(lbl_1_bss_10C, lbl_1_data_68, &sp8);
    sp8.y -= 80.0f;
    Hu3DModelPosSetV(lbl_1_bss_10E, &sp8);
    Hu3DModelRotSet(lbl_1_bss_10E, 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(lbl_1_bss_10E, 2);
    lbl_1_bss_10A = Hu3DModelCreate(HuDataSelHeapReadNum(6291520, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_10A, 1U);
    Hu3DModelPosSetV(lbl_1_bss_10A, &sp8);
    Hu3DModelRotSet(lbl_1_bss_10A, 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(lbl_1_bss_10A, 3);
    lbl_1_bss_108 = Hu3DJointMotion(lbl_1_bss_10A, HuDataSelHeapReadNum(6291521, 268435456, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_10A, lbl_1_bss_108);
    Hu3DModelAttrSet(lbl_1_bss_10A, 1073741825U);
    Hu3DModelAttrSet(lbl_1_bss_10A, 1U);
}


void fn_1_3460(void)
{
    s16 var_r31;

    var_r31 = Hu3DModelCreate(HuDataSelHeapReadNum(6291456, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(var_r31, 1U);
    Hu3DModelPosSet(var_r31, 0.0f, -200.0f, 0.0f);
    Hu3DModelRotSet(var_r31, 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(var_r31, 1);
    var_r31 = Hu3DModelCreate(HuDataSelHeapReadNum(6291457, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(var_r31, 1U);
    Hu3DModelPosSet(var_r31, 0.0f, -200.0f, 0.0f);
    Hu3DModelRotSet(var_r31, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(var_r31, 1073741825U);
    Hu3DModelLayerSet(var_r31, 1);
    var_r31 = Hu3DModelCreate(HuDataSelHeapReadNum(6291458, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(var_r31, 1U);
    Hu3DModelPosSet(var_r31, 0.0f, -200.0f, 0.0f);
    Hu3DModelRotSet(var_r31, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(var_r31, 1073741825U);
    Hu3DModelLayerSet(var_r31, 1);
    var_r31 = Hu3DModelCreate(HuDataSelHeapReadNum(6291459, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(var_r31, 1U);
    Hu3DModelPosSet(var_r31, 0.0f, -200.0f, 0.0f);
    Hu3DModelRotSet(var_r31, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(var_r31, 1073741825U);
    Hu3DModelLayerSet(var_r31, 1);
    fn_1_5914(0);
}


s32 fn_1_36E8(s32 arg0)
{
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;
    s32 var_r27;
    s32 var_r26;
    s32 var_r25;
    s32 var_r24;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;

    var_r28 = 0;
    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        var_r30 = 0;
        while (var_r30 < 2) {
            var_r29 = var_r30 + (var_r31 * 2);
            if (arg0 != 0) {
                if ((s32) (HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r29]].padNo] & 256) != 0) {
                    lbl_1_bss_1D0[var_r31].field10[var_r30] += 1.0f;
                    if (lbl_1_bss_1D0[var_r31].field10[var_r30] > 4.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] = 4.0f;
                    }
                    if (lbl_1_bss_1D0[var_r31].field10[var_r30] == 0.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] += 0.01f;
                    }
                    lbl_1_bss_2A0[var_r29].field18 = 0;
                    lbl_1_bss_354 = 1;
                } else if ((s32) (HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r29]].padNo] & 512) != 0) {
                    lbl_1_bss_1D0[var_r31].field10[var_r30] -= 1.0f;
                    if (lbl_1_bss_1D0[var_r31].field10[var_r30] < -4.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] = -4.0f;
                    }
                    if (lbl_1_bss_1D0[var_r31].field10[var_r30] == 0.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] -= 0.01f;
                    }
                    lbl_1_bss_2A0[var_r29].field18 = 0;
                    lbl_1_bss_354 = 2;
                } else {
                    lbl_1_bss_2A0[var_r29].field18 += 1;
                }
                if (lbl_1_bss_2A0[var_r29].field18 > 10) {
                    if (lbl_1_bss_1D0[var_r31].field10[var_r30] > 0.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] -= 0.05f;
                        if (lbl_1_bss_1D0[var_r31].field10[var_r30] < 0.0f) {
                            lbl_1_bss_1D0[var_r31].field10[var_r30] = 0.0f;
                        }
                    } else if (lbl_1_bss_1D0[var_r31].field10[var_r30] < 0.0f) {
                        lbl_1_bss_1D0[var_r31].field10[var_r30] += 0.05f;
                        if (lbl_1_bss_1D0[var_r31].field10[var_r30] > 0.0f) {
                            lbl_1_bss_1D0[var_r31].field10[var_r30] = 0.0f;
                        }
                    }
                }
            } else if (lbl_1_bss_1D0[var_r31].field10[var_r30] > 0.0f) {
                lbl_1_bss_1D0[var_r31].field10[var_r30] -= 0.1f;
                if (lbl_1_bss_1D0[var_r31].field10[var_r30] < 0.0f) {
                    lbl_1_bss_1D0[var_r31].field10[var_r30] = 0.0f;
                }
            } else if (lbl_1_bss_1D0[var_r31].field10[var_r30] < 0.0f) {
                lbl_1_bss_1D0[var_r31].field10[var_r30] += 0.1f;
                if (lbl_1_bss_1D0[var_r31].field10[var_r30] > 0.0f) {
                    lbl_1_bss_1D0[var_r31].field10[var_r30] = 0.0f;
                }
            }
            if (var_r31 == 0) {
                var_f30 = 1.0f;
            } else {
                var_f30 = -1.0f;
            }
            var_f29 = 40.0f * var_f30;
            if (lbl_1_bss_1D0[var_r31].field10[var_r30]) {
                lbl_1_bss_1D0[var_r31].field08[var_r30] += lbl_1_bss_1D0[var_r31].field10[var_r30];
                if (lbl_1_bss_1D0[var_r31].field08[var_r30] <= -30.0f) {
                    lbl_1_bss_1D0[var_r31].field08[var_r30] += 240.0f;
                    Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], 1U);
                    lbl_1_bss_1D0[var_r31].field28[var_r30] ^= 1;
                    Hu3DModelAttrReset(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], 1U);
                }
                if (lbl_1_bss_1D0[var_r31].field08[var_r30] >= 210.0f) {
                    lbl_1_bss_1D0[var_r31].field08[var_r30] -= 240.0f;
                    Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], 1U);
                    lbl_1_bss_1D0[var_r31].field28[var_r30] ^= 1;
                    Hu3DModelAttrReset(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], 1U);
                }
                Hu3DModelRotSet(lbl_1_bss_1D0[var_r31].pair[var_r30][lbl_1_bss_1D0[var_r31].field28[var_r30]], -lbl_1_bss_1D0[var_r31].field08[var_r30], var_f29, 0.0f);
                lbl_1_bss_1D0[var_r31].field30[var_r30] += lbl_1_bss_1D0[var_r31].field10[var_r30];
                if (lbl_1_bss_1D0[var_r31].field30[var_r30] >= 30.0f) {
                    lbl_1_bss_1D0[var_r31].field30[var_r30] -= 480.0f;
                }
                if (lbl_1_bss_1D0[var_r31].field30[var_r30] < -450.0f) {
                    lbl_1_bss_1D0[var_r31].field30[var_r30] += 480.0f;
                }
                lbl_1_bss_358 = lbl_1_bss_1D0[var_r31].field18[var_r30];
                var_f31 = lbl_1_bss_1D0[var_r31].field30[var_r30];
                if ((var_f31 >= -30.0f) && (var_f31 < 30.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 0;
                } else if ((var_f31 >= -90.0f) && (var_f31 < -30.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 1;
                } else if ((var_f31 >= -150.0f) && (var_f31 < -90.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 2;
                } else if ((var_f31 >= -210.0f) && (var_f31 < -150.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 3;
                } else if ((var_f31 >= -270.0f) && (var_f31 < -210.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 4;
                } else if ((var_f31 >= -330.0f) && (var_f31 < -270.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 5;
                } else if ((var_f31 >= -390.0f) && (var_f31 < -330.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 6;
                } else if ((var_f31 >= -450.0f) && (var_f31 < -390.0f)) {
                    lbl_1_bss_1D0[var_r31].field18[var_r30] = 7;
                }
                if ((s32) lbl_1_bss_358 != (s32) lbl_1_bss_1D0[var_r31].field18[var_r30]) {
                    if (var_r31 == 0) {
                        if ((s32) lbl_1_bss_354 == 1) {
                            var_r25 = 1872;
                        } else {
                            var_r25 = 1874;
                        }
                        HuAudFXPlay(var_r25);
                    } else {
                        if ((s32) lbl_1_bss_354 == 1) {
                            var_r24 = 1873;
                        } else {
                            var_r24 = 1875;
                        }
                        HuAudFXPlay(var_r24);
                    }
                }
                var_f31 = lbl_1_bss_1D0[var_r31].field30[var_r30] + (60.0f * (f32) lbl_1_bss_1D0[var_r31].field18[var_r30]);
                if ((var_f31 > 10.0f) || (var_f31 < -10.0f)) {
                    lbl_1_bss_1D0[var_r31].field20[var_r30] = 0;
                } else if ((s32) lbl_1_bss_1D0[var_r31].field20[var_r30] == 0) {
                    lbl_1_bss_1D0[var_r31].field20[var_r30] = 1;
                }
            }
            var_r30 += 1;
        }
        if ((lbl_1_bss_1D0[var_r31].field10[0] != 0.0f) || (lbl_1_bss_1D0[var_r31].field10[1] != 0.0f)) {
            Hu3DMotionSpeedSet(lbl_1_bss_1D0[var_r31].field38, 1.0f);
            if (lbl_1_bss_1D0[var_r31].field44 == 0) {
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field3A, lbl_1_bss_1D0[var_r31].field3E);
                lbl_1_bss_1D0[var_r31].field44 = 1;
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field48[0], lbl_1_bss_1D0[var_r31].field4C[0][1]);
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field48[1], lbl_1_bss_1D0[var_r31].field4C[1][1]);
            }
        } else {
            Hu3DMotionSpeedSet(lbl_1_bss_1D0[var_r31].field38, 0.0f);
            if (lbl_1_bss_1D0[var_r31].field44 == 1) {
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field3A, lbl_1_bss_1D0[var_r31].field3C);
                lbl_1_bss_1D0[var_r31].field44 = 0;
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field48[0], lbl_1_bss_1D0[var_r31].field4C[0][0]);
                Hu3DMotionSet(lbl_1_bss_1D0[var_r31].field48[1], lbl_1_bss_1D0[var_r31].field4C[1][0]);
            }
        }
    }
    if ((arg0 == 0) && (lbl_1_bss_1D0->field10[0] == 0.0f) && (lbl_1_bss_1D0->field10[1] == 0.0f) && (lbl_1_bss_1D0[1].field10[0] == 0.0f) && (lbl_1_bss_1D0[1].field10[1] == 0.0f)) {
        var_r28 = 1;
    }
    if (arg0 != 0) {
        var_r27 = 0;
        var_r26 = 0;
        for (var_r31 = 0; var_r31 < 2; var_r31++) {
            var_r30 = 0;
            while (var_r30 < 2) {
                if (((s32) lbl_1_bss_1D0[var_r31].field20[var_r30] != 0) && (lbl_1_bss_118[var_r30].field48 == (s32) lbl_1_bss_1D0[var_r31].field18[var_r30])) {
                    Hu3DModelAttrReset(lbl_1_bss_1D0[var_r31].field54[var_r30], 1U);
                    if ((s32) lbl_1_bss_1D0[var_r31].field20[var_r30] == 1) {
                        if (var_r31 == 0) {
                            HuAudFXPlay(1877);
                        } else {
                            HuAudFXPlay(1878);
                        }
                        lbl_1_bss_1D0[var_r31].field20[var_r30] = 2;
                    }
                } else {
                    Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[var_r30], 1U);
                }
                var_r30 += 1;
            }
        }
        if ((lbl_1_bss_1D0->field10[0] == 0.0f) && ((s32) lbl_1_bss_1D0->field20[0] != 0) && (lbl_1_bss_118->field48 == (s32) lbl_1_bss_1D0->field18[0]) && (lbl_1_bss_1D0->field10[1] == 0.0f) && ((s32) lbl_1_bss_1D0->field20[1] != 0) && (lbl_1_bss_118[1].field48 == (s32) lbl_1_bss_1D0->field18[1])) {
            var_r27 = 1;
        }
        if ((lbl_1_bss_1D0[1].field10[0] == 0.0f) && ((s32) lbl_1_bss_1D0[1].field20[0] != 0) && (lbl_1_bss_118->field48 == (s32) lbl_1_bss_1D0[1].field18[0]) && (lbl_1_bss_1D0[1].field10[1] == 0.0f) && ((s32) lbl_1_bss_1D0[1].field20[1] != 0) && (lbl_1_bss_118[1].field48 == (s32) lbl_1_bss_1D0[1].field18[1])) {
            var_r26 = 1;
        }
        if (var_r27 != 0) {
            lbl_1_bss_2A0->field1C = lbl_1_bss_2A0[1].field1C = 1;
            if (var_r26 == 0) {
                lbl_1_bss_2A0[2].field1C = lbl_1_bss_2A0[3].field1C = 0;
            }
            lbl_1_bss_2A0->count20 += 1;
            lbl_1_bss_2A0[1].count20 += 1;
            var_r28 = 1;
        }
        if (var_r26 != 0) {
            if (var_r27 == 0) {
                lbl_1_bss_2A0->field1C = lbl_1_bss_2A0[1].field1C = 0;
            }
            lbl_1_bss_2A0[2].field1C = lbl_1_bss_2A0[3].field1C = 1;
            lbl_1_bss_2A0[2].count20 += 1;
            lbl_1_bss_2A0[3].count20 += 1;
            var_r28 = 1;
        }
        if (var_r28 != 0) {
            fn_1_5C98(2);
        }
    }
    return var_r28;
}


s32 fn_1_4C24(s32 arg0)
{
    s32 var_r31;
    s32 var_r29;
    s32 var_r28;

    var_r29 = 0;
    switch ((s32) lbl_1_bss_114) {                  /* irregular */
    case 0:
        lbl_1_bss_110 += 1;
        lbl_1_bss_104 = 0;
        var_r31 = 0;
        while (var_r31 < 2) {
            if ((s32) lbl_1_bss_110 == arg0) {
                var_r28 = 1;
            } else {
                var_r28 = 0;
            }
            lbl_1_bss_118[var_r31].field48 = fn_1_5444(var_r31, var_r28);
            if (lbl_1_bss_118[var_r31].field48 == lbl_1_bss_118[var_r31].field58) {
                lbl_1_bss_104 += 1;
            }
            var_r31 += 1;
        }
        if (((s32) lbl_1_bss_104 == 2) && ((s32) lbl_1_bss_110 == arg0) && ((s32) lbl_1_bss_374 == 0)) {
            fn_1_55A4(arg0);
        } else {
            var_r31 = 0;
            while (var_r31 < 2) {
                lbl_1_bss_118[var_r31].field54 = lbl_1_bss_118[var_r31].field48;
                if (((s32) lbl_1_bss_110 == arg0) && ((s32) lbl_1_bss_374 == 0)) {
                    lbl_1_bss_118[var_r31].field50 = lbl_1_bss_118[var_r31].field48;
                }
                lbl_1_bss_118[var_r31].field40 = -20.0f;
                Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], lbl_1_bss_118[var_r31].field40, 0.0f, 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], 1U);
                Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][1], 1U);
                lbl_1_bss_118[var_r31].field44 = 0.0f;
                Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], lbl_1_bss_118[var_r31].field44, 0.0f, 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], 1U);
                Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][1], 1U);
                var_r31 += 1;
            }
        }
        lbl_1_bss_114 += 1;
        HuAudFXPlay(1882);
        break;
    case 1:
    case 2:
    case 3:
        var_r31 = 0;
        while (var_r31 < 2) {
            lbl_1_bss_118[var_r31].field44 += 20.0f;
            Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], lbl_1_bss_118[var_r31].field44, 0.0f, 0.0f);
            if (lbl_1_bss_118[var_r31].field44 >= 90.0f) {
                Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], 1U);
                Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][1], 1U);
                Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][1], -180.0f + lbl_1_bss_118[var_r31].field44, 0.0f, 0.0f);
            }
            if ((s32) lbl_1_bss_114 == 1) {
                lbl_1_bss_118[var_r31].field40 += 20.0f;
                Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], lbl_1_bss_118[var_r31].field40, 0.0f, 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], 1U);
            }
            if ((s32) lbl_1_bss_114 == 3) {
                Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][1], 1U);
            }
            var_r31 += 1;
        }
        if ((lbl_1_bss_118->field44 == 20.0f) || (lbl_1_bss_118->field44 == 160.0f) || (lbl_1_bss_118->field44 == 180.0f)) {
            lbl_1_bss_114 += 1;
        }
        break;
    case 4:
        lbl_1_bss_114 = 0;
        lbl_1_bss_118->field4C = lbl_1_bss_118->field48;
        lbl_1_bss_118[1].field4C = lbl_1_bss_118[1].field48;
        if ((s32) lbl_1_bss_110 == arg0) {
            lbl_1_bss_110 = 0;
            var_r29 = 1;
        }
        break;
    }
    return var_r29;
}


s32 fn_1_5444(s32 var_r29, s32 arg1)
{
    s32 choices[8];
    s32 var_r31;
    s32 var_r30;
    s32 var_r28;

    var_r31 = var_r30 = 0;
    if (arg1 == 0) {
        while (var_r31 < 8) {
            if (var_r31 != lbl_1_bss_118[var_r29].field54) {
                (&choices[0])[var_r30] = var_r31;
                var_r30 += 1;
            }
            var_r31 += 1;
        }
    } else {
        while (var_r31 < 8) {
            if (arg1 == 1) {
                if ((var_r31 != lbl_1_bss_118[var_r29].field50) && (var_r31 != lbl_1_bss_118[var_r29].field54)) {
                    (&choices[0])[var_r30] = var_r31;
                    var_r30 += 1;
                }
            } else if ((var_r31 != lbl_1_bss_118[var_r29].field50) && (var_r31 != lbl_1_bss_118[var_r29].field54) && (var_r31 != lbl_1_bss_118[var_r29].field58)) {
                (&choices[0])[var_r30] = var_r31;
                var_r30 += 1;
            }
            var_r31 += 1;
        }
    }
    var_r28 = var_r30;
    return (&choices[0])[frandmod(var_r28)];
}


void fn_1_55A4(s32 arg0)
{
    s32 choices[8];
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;
    s32 var_r27;

    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        var_r30 = var_r29 = 0;
        while (var_r30 < 8) {
            if ((var_r30 != lbl_1_bss_118[var_r31].field50) && (var_r30 != lbl_1_bss_118[var_r31].field54) && (var_r30 != lbl_1_bss_118[var_r31].field58)) {
                (&choices[0])[var_r29] = var_r30;
                var_r29 += 1;
            }
            var_r30 += 1;
        }
        var_r27 = var_r29;
        var_r28 = (&choices[0])[frandmod(var_r27)];
        lbl_1_bss_118[var_r31].field48 = var_r28;
        lbl_1_bss_118[var_r31].field50 = lbl_1_bss_118[var_r31].field48;
        lbl_1_bss_118[var_r31].field54 = lbl_1_bss_118[var_r31].field48;
        lbl_1_bss_118[var_r31].field40 = -20.0f;
        Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], lbl_1_bss_118[var_r31].field40, 0.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field48][1], 1U);
        lbl_1_bss_118[var_r31].field44 = 0.0f;
        Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], lbl_1_bss_118[var_r31].field44, 0.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], 1U);
        Hu3DModelRotSet(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][0], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_118[var_r31].pair[lbl_1_bss_118[var_r31].field4C][1], 1U);
    }
}


void fn_1_5914(s32 arg0)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 2) {
        if (arg0 == 0) {
            lbl_1_bss_B8[var_r31].sizeX = 108;
            lbl_1_bss_B8[var_r31].sizeY = 36;
            if (var_r31 == 0) {
                lbl_1_bss_B8[var_r31].field10 = 16.0f + ((f32) lbl_1_bss_B8[var_r31].sizeX / 2.0f);
                lbl_1_bss_B8[var_r31].field08 = lbl_1_bss_B8[var_r31].field10 - 150.0f;
            } else {
                lbl_1_bss_B8[var_r31].field10 = 560.0f - ((f32) lbl_1_bss_B8[var_r31].sizeX / 2.0f);
                lbl_1_bss_B8[var_r31].field08 = 150.0f + lbl_1_bss_B8[var_r31].field10;
            }
            lbl_1_bss_B8[var_r31].field0C = 40.0f + ((f32) lbl_1_bss_B8[var_r31].sizeY / 2.0f);
            lbl_1_bss_B8[var_r31].scoreBox = MgScoreBoxCreate(lbl_1_bss_B8[var_r31].sizeX, lbl_1_bss_B8[var_r31].sizeY);
            MgScoreBoxPosSet(lbl_1_bss_B8[var_r31].scoreBox, lbl_1_bss_B8[var_r31].field08, lbl_1_bss_B8[var_r31].field0C);
            if (var_r31 == 0) {
                MgScoreBoxColorSet(lbl_1_bss_B8[var_r31].scoreBox, 250U, 0U, 30U);
            } else {
                MgScoreBoxColorSet(lbl_1_bss_B8[var_r31].scoreBox, 0U, 50U, 250U);
            }
        } else {
            lbl_1_bss_B8[var_r31].field08 = lbl_1_bss_B8[var_r31].field10;
            MgScoreBoxPosSet(lbl_1_bss_B8[var_r31].scoreBox, lbl_1_bss_B8[var_r31].field08, lbl_1_bss_B8[var_r31].field0C);
        }
        var_r31 += 1;
    }
    fn_1_5C98(arg0);
}


void fn_1_5C98(s32 arg0)
{
    s32 var_r31;
    s32 var_r30;
    f32 var_f31;

    for (var_r31 = 0; var_r31 < 2; var_r31++) {
        var_r30 = 0;
        while (var_r30 < 3) {
            var_f31 = (32.0f * (f32) var_r30) + (lbl_1_bss_B8[var_r31].field08 - 32.0f);
            if (arg0 == 0) {
                lbl_1_bss_B8[var_r31].espA[var_r30] = espEntry(10158110U, 0, 0);
                espDrawNoSet(lbl_1_bss_B8[var_r31].espA[var_r30], 0);
                espPriSet(lbl_1_bss_B8[var_r31].espA[var_r30], 1);
                espPosSet(lbl_1_bss_B8[var_r31].espA[var_r30], var_f31, lbl_1_bss_B8[var_r31].field0C);
                espTPLvlSet(lbl_1_bss_B8[var_r31].espA[var_r30], 0.5f);
                lbl_1_bss_B8[var_r31].espB[var_r30] = espEntry(10158111U, 0, 0);
                espDrawNoSet(lbl_1_bss_B8[var_r31].espB[var_r30], 0);
                espPriSet(lbl_1_bss_B8[var_r31].espB[var_r30], 1);
                espPosSet(lbl_1_bss_B8[var_r31].espB[var_r30], var_f31, lbl_1_bss_B8[var_r31].field0C);
                espDispOff(lbl_1_bss_B8[var_r31].espB[var_r30]);
            } else if (arg0 == 1) {
                espPosSet(lbl_1_bss_B8[var_r31].espA[var_r30], var_f31, lbl_1_bss_B8[var_r31].field0C);
                espPosSet(lbl_1_bss_B8[var_r31].espB[var_r30], var_f31, lbl_1_bss_B8[var_r31].field0C);
            } else if (lbl_1_bss_2A0[var_r31 * 2].count20 > var_r30) {
                espDispOn(lbl_1_bss_B8[var_r31].espB[var_r30]);
                espDispOff(lbl_1_bss_B8[var_r31].espA[var_r30]);
            } else {
                espDispOn(lbl_1_bss_B8[var_r31].espA[var_r30]);
                espDispOff(lbl_1_bss_B8[var_r31].espB[var_r30]);
            }
            var_r30 += 1;
        }
    }
}


s32 fn_1_603C(void)
{
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;

    var_r30 = 0;
    if ((s32) lbl_1_bss_368 == 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (lbl_1_bss_2A0[var_r31].field1C != 0) {
                var_r29 = 5;
            } else {
                var_r29 = 6;
            }
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, lbl_1_bss_2A0[var_r31].motion[var_r29], 0.0f, 4.0f, 0U);
            if (lbl_1_bss_2A0[var_r31].field1C != 0) {
                CharFXPlay(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, 579);
            }
            var_r31 += 1;
        }
        lbl_1_bss_368 += 1;
        if (lbl_1_bss_34C->stopF == 0) {
            lbl_1_bss_34C->stopF = 1;
            lbl_1_bss_34C->mode = 1;
        }
        if (lbl_1_bss_2A0->field1C != 0) {
            Hu3DMotionSet(lbl_1_bss_1D0->field3A, lbl_1_bss_1D0->field40);
            Hu3DMotionSet(lbl_1_bss_1D0[1].field3A, lbl_1_bss_1D0[1].field3C);
        }
        if (lbl_1_bss_2A0[2].field1C != 0) {
            Hu3DMotionSet(lbl_1_bss_1D0->field3A, lbl_1_bss_1D0->field3C);
            Hu3DMotionSet(lbl_1_bss_1D0[1].field3A, lbl_1_bss_1D0[1].field40);
        }
        Hu3DModelAttrReset(lbl_1_bss_10A, 1U);
        if (lbl_1_bss_2A0->field1C != 0) {
            Hu3DModelAttrReset(lbl_1_bss_1D0->field54[0], 1U);
            Hu3DModelAttrReset(lbl_1_bss_1D0->field54[1], 1U);
            Hu3DModelAttrReset(lbl_1_bss_1D0->field5C, 1U);
            HuAudFXPlay(1879);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1D0->field54[0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0->field54[1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0->field5C, 1U);
        }
        if (lbl_1_bss_2A0[2].field1C != 0) {
            Hu3DModelAttrReset(lbl_1_bss_1D0[1].field54[0], 1U);
            Hu3DModelAttrReset(lbl_1_bss_1D0[1].field54[1], 1U);
            Hu3DModelAttrReset(lbl_1_bss_1D0[1].field5C, 1U);
            HuAudFXPlay(1880);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1D0[1].field54[0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[1].field54[1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[1].field5C, 1U);
        }
    } else if ((Hu3DMotionShiftIDGet(GwPlayerConf[lbl_1_bss_290[0]].charNo) == -1) && (CharMotionEndCheck(GwPlayerConf[lbl_1_bss_290[0]].charNo) != 0)) {
        var_r31 = 0;
        while (var_r31 < 4) {
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, lbl_1_bss_2A0[var_r31].motion[0], 0.0f, 8.0f, 1073741825U);
            var_r31 += 1;
        }
        lbl_1_bss_368 = 0;
        var_r30 = 1;
        Hu3DMotionSet(lbl_1_bss_1D0->field3A, lbl_1_bss_1D0->field3C);
        Hu3DMotionSet(lbl_1_bss_1D0[1].field3A, lbl_1_bss_1D0[1].field3C);
        lbl_1_bss_1D0->field44 = lbl_1_bss_1D0[1].field44 = 0;
        lbl_1_bss_2A0->field1C = lbl_1_bss_2A0->field1C = 0;
        lbl_1_bss_2A0[2].field1C = lbl_1_bss_2A0[3].field1C = 0;
        Hu3DModelAttrSet(lbl_1_bss_10A, 1U);
        var_r31 = 0;
        while (var_r31 < 2) {
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field5C, 1U);
            var_r31 += 1;
        }
    }
    return var_r30;
}


s32 fn_1_65A0(void)
{
    s32 var_r31;
    s32 var_r30;

    var_r30 = 0;
    if ((s32) lbl_1_bss_368 == 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, lbl_1_bss_2A0[var_r31].motion[6], 0.0f, 4.0f, 0U);
            var_r31 += 1;
        }
        var_r31 = 0;
        while (var_r31 < 2) {
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field54[1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_1D0[var_r31].field5C, 1U);
            var_r31 += 1;
        }
        lbl_1_bss_368 += 1;
    } else if ((Hu3DMotionShiftIDGet(GwPlayerConf[lbl_1_bss_290[0]].charNo) == -1) && (CharMotionEndCheck(GwPlayerConf[lbl_1_bss_290[0]].charNo) != 0)) {
        var_r31 = 0;
        while (var_r31 < 4) {
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, lbl_1_bss_2A0[var_r31].motion[0], 0.0f, 8.0f, 1073741825U);
            var_r31 += 1;
        }
        lbl_1_bss_368 = 0;
        var_r30 = 1;
    }
    return var_r30;
}


void fn_1_67C4(void)
{
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;

    fn_1_1464(0);
    if ((s32) lbl_1_bss_368 == 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            var_r30 = 0;
            if (Hu3DMotionShiftIDGet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo) == -1) {
                var_r29 = var_r31 / 2;
                var_r28 = var_r31 % 2;
                if (lbl_1_bss_1D0[var_r29].field10[var_r28] == 0.0f) {
                    if (lbl_1_bss_2A0[var_r31].selectedMotion != lbl_1_bss_2A0[var_r31].motion[0]) {
                        lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[0];
                        var_r30 = 1;
                    }
                } else if (lbl_1_bss_1D0[var_r29].field10[var_r28] > 3.0f) {
                    if (lbl_1_bss_2A0[var_r31].selectedMotion != lbl_1_bss_2A0[var_r31].motion[2]) {
                        lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[2];
                        var_r30 = 1;
                    }
                } else if (lbl_1_bss_1D0[var_r29].field10[var_r28] > 0.0f) {
                    if (lbl_1_bss_2A0[var_r31].selectedMotion != lbl_1_bss_2A0[var_r31].motion[1]) {
                        lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[1];
                        var_r30 = 1;
                    }
                } else if (lbl_1_bss_1D0[var_r29].field10[var_r28] < -3.0f) {
                    if (lbl_1_bss_2A0[var_r31].selectedMotion != lbl_1_bss_2A0[var_r31].motion[3]) {
                        lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[3];
                        var_r30 = 1;
                    }
                } else if ((lbl_1_bss_1D0[var_r29].field10[var_r28] < 0.0f) && (lbl_1_bss_2A0[var_r31].selectedMotion != lbl_1_bss_2A0[var_r31].motion[4])) {
                    lbl_1_bss_2A0[var_r31].selectedMotion = lbl_1_bss_2A0[var_r31].motion[4];
                    var_r30 = 1;
                }
                if (var_r30 != 0) {
                    CharMotionShiftSet(GwPlayerConf[lbl_1_bss_290[var_r31]].charNo, lbl_1_bss_2A0[var_r31].selectedMotion, 0.0f, 4.0f, 1073741825U);
                }
            }
            var_r31 += 1;
        }
    }
    HuPrcVSleep();
}


void fn_1_6BD4(void)
{
    s32 var_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;
    s32 var_r27;
    s32 var_r26;
    s32 var_r25;
    s32 var_r24;
    s32 var_r23;
    s32 var_r22;
    s32 var_r21;
    s32 var_r20;
    s32 var_r19;

    var_r31 = 0;
    while (var_r31 < 4) {
        if (GwPlayerConf[lbl_1_bss_290[var_r31]].type != 0) {
            HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r31]].padNo] = 0;
            if (lbl_1_bss_8[var_r31].field04 == 0) {
                if (lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C > (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) {
                    var_r30 = (lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C] + 8) - lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C;
                    var_r29 = lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C - lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C];
                    lbl_1_bss_8[var_r31].field28 = 0;
                } else if (lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C < (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) {
                    var_r30 = lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C] - lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C;
                    var_r29 = (lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C + 8) - lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C];
                    lbl_1_bss_8[var_r31].field28 = 0;
                } else if (lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C == (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) {
                    var_r30 = var_r29 = 0;
                    lbl_1_bss_8[var_r31].field28 = -1;
                    if ((s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field20[lbl_1_bss_8[var_r31].field1C] == 0) {
                        lbl_1_bss_8[var_r31].field08 = 2;
                        lbl_1_bss_8[var_r31].field24 = 0;
                        lbl_1_bss_8[var_r31].field28 = 1;
                    }
                } else {
                    var_r30 = var_r29 = 4;
                    lbl_1_bss_8[var_r31].field28 = 0;
                }
                if (var_r30 < var_r29) {
                    lbl_1_bss_8[var_r31].field20 = 0;
                    if (var_r30 > 2) {
                        var_r28 = 2;
                    } else {
                        var_r28 = var_r30;
                    }
                    lbl_1_bss_8[var_r31].field08 = var_r28;
                } else if (var_r30 > var_r29) {
                    lbl_1_bss_8[var_r31].field20 = 1;
                    if (var_r29 > 2) {
                        var_r27 = 2;
                    } else {
                        var_r27 = var_r29;
                    }
                    lbl_1_bss_8[var_r31].field08 = var_r27;
                } else if ((var_r30 != 0) && (var_r29 != 0)) {
                    lbl_1_bss_8[var_r31].field20 = frandmod(2);
                    if (lbl_1_bss_8[var_r31].field20 == 0) {
                        if (var_r30 > 2) {
                            var_r26 = 2;
                        } else {
                            var_r26 = var_r30;
                        }
                        lbl_1_bss_8[var_r31].field08 = var_r26;
                    } else {
                        if (var_r29 > 2) {
                            var_r25 = 2;
                        } else {
                            var_r25 = var_r29;
                        }
                        lbl_1_bss_8[var_r31].field08 = var_r25;
                    }
                } else if (lbl_1_bss_8[var_r31].field28 != 1) {
                    lbl_1_bss_8[var_r31].field08 = 0;
                }
                lbl_1_bss_8[var_r31].field04 = 1;
                if (lbl_1_bss_8[var_r31].field0C > 0) {
                    lbl_1_bss_8[var_r31].field20 ^= 1;
                    if (lbl_1_bss_8[var_r31].field08 == 1) {
                        var_r24 = 2;
                    } else {
                        if (lbl_1_bss_8[var_r31].field08 == 2) {
                            var_r23 = 1;
                        } else {
                            var_r23 = frandmod(2) + 1;
                        }
                        var_r24 = var_r23;
                    }
                    lbl_1_bss_8[var_r31].field08 = var_r24;
                    lbl_1_bss_8[var_r31].field0C -= 1;
                }
            }
            if (lbl_1_bss_8[var_r31].field28 == 1) {
                if (lbl_1_bss_8[var_r31].field24 == 0) {
                    if (lbl_1_bss_8[var_r31].field08 > 0) {
                        if (lbl_1_bss_8[var_r31].field20 == 0) {
                            var_r22 = 256;
                        } else {
                            var_r22 = 512;
                        }
                        HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r31]].padNo] = var_r22;
                        lbl_1_bss_8[var_r31].field08 -= 1;
                        lbl_1_bss_8[var_r31].field24 = lbl_1_data_5C[1];
                        if ((lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C == (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) && ((s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field20[lbl_1_bss_8[var_r31].field1C] != 0)) {
                            if (lbl_1_bss_8[var_r31].field20 != 0) {
                                var_r21 = 256;
                            } else {
                                var_r21 = 512;
                            }
                            HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r31]].padNo] = var_r21;
                        }
                    }
                    if (lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field10[lbl_1_bss_8[var_r31].field1C] == 0.0f) {
                        if (lbl_1_bss_8[var_r31].field08 == 0) {
                            lbl_1_bss_8[var_r31].field04 = 0;
                        } else if ((lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C == (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) && ((s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field20[lbl_1_bss_8[var_r31].field1C] != 0)) {
                            lbl_1_bss_8[var_r31].field04 = 0;
                        }
                    }
                } else {
                    lbl_1_bss_8[var_r31].field24 -= 1;
                }
            } else if (lbl_1_bss_8[var_r31].field24 == 0) {
                if (lbl_1_bss_8[var_r31].field08 > 0) {
                    if (lbl_1_bss_8[var_r31].field20 == 0) {
                        var_r20 = 256;
                    } else {
                        var_r20 = 512;
                    }
                    HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r31]].padNo] = var_r20;
                    lbl_1_bss_8[var_r31].field08 -= 1;
                    if ((lbl_1_bss_118[lbl_1_bss_8[var_r31].field14].field4C == (s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field18[lbl_1_bss_8[var_r31].field1C]) && ((s32) lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field20[lbl_1_bss_8[var_r31].field1C] != 0)) {
                        if (lbl_1_bss_8[var_r31].field20 != 0) {
                            var_r19 = 256;
                        } else {
                            var_r19 = 512;
                        }
                        HuPadBtnDown[GwPlayerConf[lbl_1_bss_290[var_r31]].padNo] = var_r19;
                    }
                }
                if (lbl_1_bss_1D0[lbl_1_bss_8[var_r31].field18].field10[lbl_1_bss_8[var_r31].field1C] == 0.0f) {
                    lbl_1_bss_8[var_r31].field04 = 0;
                    if (lbl_1_bss_8[var_r31].field08 == 0) {
                        lbl_1_bss_8[var_r31].field24 = lbl_1_data_5C[0];
                    }
                }
            } else {
                lbl_1_bss_8[var_r31].field24 -= 1;
            }
        }
        var_r31 += 1;
    }
}
