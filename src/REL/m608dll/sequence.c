#include "REL/m608dll.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

/* button mask */
MGSEQ_PARAM lbl_1_data_0 = {
    0,
    1,
    fn_1_140,
    fn_1_190,
    fn_1_57C,
    fn_1_580,
    fn_1_1A2C,
    fn_1_1DD8,
    fn_1_2374,
    fn_1_2538,
    fn_1_253C,
};

f32 lbl_1_data_28[14] = {
    -35.0f,
    -35.0f,
    -35.0f,
    -25.0f,
    -35.0f,
    -35.0f,
    -35.0f,
    -5.0f,
    -35.0f,
    -10.0f,
    -5.0f,
    -10.0f,
    -10.0f,
    -10.0f,
};

f32 lbl_1_data_60[14] = {
    -10.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
};

f32 lbl_1_data_98[14] = {
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.4f,
    1.0f,
    1.0f,
    0.4f,
    1.0f,
    1.0f,
    1.0f,
};

char lbl_1_data_D0[9] = "ske_head";

char lbl_1_data_D9[16] = "1P neck ... %f\012";

char lbl_1_data_E9[16] = "2P neck ... %f\012";

char lbl_1_data_F9[16] = "3P neck ... %f\012";

char lbl_1_data_109[16] = "4P neck ... %f\012";

char lbl_1_data_119[14] = "snowboard_txt";

char lbl_1_data_127[5] = "p2st";

char lbl_1_data_12C[22] = "608kurukuru-pmov_null";

char lbl_1_data_142[5] = "p1st";

char lbl_1_data_147[26] = "*** wait vibration start\012";

char lbl_1_data_161[24] = "*** wait vibration end\012";

char lbl_1_data_179[16] = "*** jump start\012";

char lbl_1_data_189[7] = "pmov\000\000";

static M608PathSamples lbl_1_bss_10;

static s32 lbl_1_bss_C;

static s16 lbl_1_bss_8;

static f32 lbl_1_bss_4;

static f32 lbl_1_bss_0;

s32 fn_1_A0(s32 arg0, s32 arg1)
{
    s32 var_r31;

    var_r31 = arg0;
    if ((var_r31 == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0)) {
        var_r31 = HuAudBGMPlay((s16) arg1);
    }
    return var_r31;
}

void fn_1_104(s32 arg0)
{
    if (arg0 != -1) {
        HuAudSStreamFadeOut(arg0, 100);
    }
}

void fn_1_140(s16 mode, s16 frameNo)
{
    lbl_1_bss_0 = 0.0f;
    lbl_1_bss_4 = 0.0f;
    MgSeqModeNext();
}

void fn_1_190(s16 arg1, s16 frameNo)
{
    Point3d sp18;
    int charNo;
    char *itemHook;
    s32 var_r31;

    if (frameNo == 0) {
        fn_1_5E6C(0);
        var_r31 = 0;
        while (var_r31 < 4) {
            charNo = lbl_1_bss_3E80.characterNos[var_r31];
            CharMotionSet((s16) charNo, lbl_1_bss_3E80.characterMotions[var_r31][0]);
            Hu3DModelAttrSet(lbl_1_bss_3E80.characterModels[var_r31], 1073741825U);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsA[var_r31], lbl_1_data_5D0[var_r31], lbl_1_bss_3E80.characterModels[var_r31]);
            itemHook = CharModelItemHookGet(lbl_1_bss_3E80.characterNos[var_r31], 4, 2);
            Hu3DModelHookSet(lbl_1_bss_3E80.characterModels[var_r31], itemHook, lbl_1_bss_3E80.secondaryModels[var_r31]);
            var_r31 += 1;
        }
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[0], lbl_1_data_D0, fn_1_38F0);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[1], lbl_1_data_D0, fn_1_39B4);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[2], lbl_1_data_D0, fn_1_3A78);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[3], lbl_1_data_D0, fn_1_3B3C);
        OSReport(lbl_1_data_D9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]]);
        OSReport(lbl_1_data_E9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]]);
        OSReport(lbl_1_data_F9, lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]]);
        OSReport(lbl_1_data_109, lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]]);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[0], lbl_1_data_119, fn_1_3F18);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[1], lbl_1_data_119, fn_1_400C);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[2], lbl_1_data_119, fn_1_40F0);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[3], lbl_1_data_119, fn_1_41D4);
        fn_1_5FDC(3, 0);
        fn_1_5FDC(4, 2);
        if ((s32) lbl_1_bss_3E80.flag_44C == 0) {
            fn_1_5FDC(5, 4);
        }
        fn_1_5FDC(10, 8);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[1], lbl_1_data_127, &sp18);
        fn_1_5DA4(sp18);
        lbl_1_bss_3E80.audioHandle_454 = HuAudSStreamPlay(79);
        return;
    }
    if (fn_1_5F64() != 0) {
        fn_1_614C();
        lbl_1_bss_3E80.activePlayer = 0;
        lbl_1_bss_3E80.state = 0;
        lbl_1_bss_3E80.frame = 0;
        MgSeqModeNext();
    }
}

void fn_1_57C(s16 mode, s16 frameNo)
{

}

void fn_1_580(s16 arg1, s16 frameNo)
{
    Point3d sp104;
    Point3d spF8;
    Point3d spEC;
    Point3d spE0;
    Point3d spD4;
    Point3d spC8;
    Point3d spBC;
    Point3d spB0;
    Point3d spA4;
    Point3d sp98;
    int temp_r28;
    int temp_r27;
    s32 temp_r31;
    u32 var_r24;
    s32 var_r26;
    s32 var_r25;
    s32 var_r30;
    s32 var_r29;
    s32 var_r23;

    temp_r31 = lbl_1_bss_3E80.activePlayer;
    temp_r28 = lbl_1_bss_3E80.characterModels[temp_r31];
    temp_r27 = lbl_1_bss_3E80.characterNos[temp_r31];
    if (frameNo == 0) {
        fn_1_6578();
    }
    Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &sp104);
    fn_1_38EC(sp104);
    switch (lbl_1_bss_3E80.state) {  /* switch 1 */
    case 0:                                         /* switch 1 */
        fn_1_5E6C(1);
        CharMotionSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][1]);
        Hu3DModelAttrReset(lbl_1_bss_3E80.characterModels[temp_r31], 1073741825U);
        Hu3DMotionSpeedSet(lbl_1_bss_3E80.characterModels[temp_r31], 0.5f);
        Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[temp_r31], 1.0f);
        Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[temp_r31], lbl_1_data_D0, fn_1_3C84);
        Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[temp_r31], lbl_1_data_119, fn_1_3E00);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[temp_r31], lbl_1_data_142, &spF8);
        fn_1_5DA4(spF8);
        lbl_1_bss_3E80.state = 1U;
        lbl_1_bss_3E80.frame = 0;
        break;
    case 1:                                         /* switch 1 */
        if (fn_1_5F64() != 0) {
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[0], lbl_1_data_D0, fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[0], lbl_1_data_119, fn_1_3D7C);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[0]);
            Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[0]);
            fn_1_5E6C(3);
            lbl_1_bss_3E80.state = 4U;
            lbl_1_bss_3E80.frame = 0;
        } else {
            omSetRot(lbl_1_bss_3E80.playerObjects[0], 0.0f, lbl_1_bss_0, 0.0f);
            lbl_1_bss_0 += 1.4f;
            lbl_1_bss_4 += lbl_1_data_98[temp_r27];
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 2:                                         /* switch 1 */
        fn_1_5E6C(3);
        lbl_1_bss_3E80.state = 4U;
        lbl_1_bss_3E80.frame = 0;
        break;
    case 3:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142, &spEC);
            fn_1_5DA4(spEC);
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[temp_r31], lbl_1_data_D0, fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[temp_r31], lbl_1_data_119, fn_1_3E00);
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, 45.0f, 0.0f);
            CharMotionSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][2]);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[temp_r31]);
            fn_1_5F90(0);
            fn_1_5F90(1);
            fn_1_5F90(7);
            fn_1_5F90(8);
            fn_1_5F90(9);
            fn_1_5F90(5);
            fn_1_5FDC(3, 1);
            fn_1_5FDC(4, 3);
            if ((s32) lbl_1_bss_3E80.flag_44C == temp_r31) {
                fn_1_5FDC(5, 5);
            }
            fn_1_5FDC(10, 8);
            fn_1_605C(9);
            CharMotionShiftSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][2], 0.0f, 8.0f, 1073741825U);
            fn_1_5E6C(3);
            lbl_1_bss_3E80.state = 4U;
            lbl_1_bss_3E80.frame = 0;
        } else {
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 4:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, 55.0f, 0.0f);
            Hu3DObjHookSet(lbl_1_bss_3E80.characterModels[temp_r31], lbl_1_data_D0, fn_1_3C00);
            Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[temp_r31], lbl_1_data_119, fn_1_3D7C);
            CharMotionShiftSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][2], 0.0f, 8.0f, 1073741825U);
            omVibrate((s16) temp_r31, 30, 7, 3);
            var_r30 = 0;
            while (var_r30 < 27) {
                Hu3DModelAttrSet(lbl_1_bss_3E80.models[var_r30], 1U);
                var_r30 += 1;
            }
            lbl_1_bss_3E80.timingSubstate = 0;
            lbl_1_bss_3E80.angle = 0.0f;
            lbl_1_bss_3E80.from = 0.0f;
            lbl_1_bss_3E80.to = 0.0f;
            lbl_1_bss_3E80.frame_1BC = 0;
            lbl_1_bss_3E80.count = 0;
            lbl_1_bss_3E80.scores[lbl_1_bss_3E80.activePlayer] = 0;
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142, &spE0);
            fn_1_5DA4(spE0);
            Hu3DLayerHookSet(3, fn_1_437C);
            OSReport(lbl_1_data_147);
            goto block_25;
        }
        if ((s32) lbl_1_bss_3E80.frame >= 60) {
            Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsA[0]);
            Hu3DModelHookSet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, (s16) temp_r28);
            Hu3DMotionSet(lbl_1_bss_3E80.mainModel, lbl_1_bss_3E80.mainMotion);
            Hu3DModelAttrSet(lbl_1_bss_3E80.mainModel, 1073741826U);
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet((s16) temp_r28, 0.0f, 0.0f, 0.0f);
            Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
            Hu3DMotionCalc((s16) temp_r28);
            Hu3DMotionCalc(lbl_1_bss_3E80.playerModelsA[0]);
            Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[temp_r31]);
            Hu3DMotionTimingHookSet(lbl_1_bss_3E80.models_212_213[0], NULL);
            Hu3DMotionTimingHookSet(lbl_1_bss_3E80.models_212_213[1], NULL);
            if (temp_r31 < 3) {
                Hu3DMotionTimingHookSet(lbl_1_bss_3E80.models_212_213[0], fn_1_2FD8);
                Hu3DMotionTimeSet(lbl_1_bss_3E80.models_212_213[0], 0.0f);
            } else {
                Hu3DMotionTimingHookSet(lbl_1_bss_3E80.models_212_213[1], fn_1_2FD8);
                Hu3DMotionTimeSet(lbl_1_bss_3E80.models_212_213[1], 0.0f);
            }
            lbl_1_bss_3E80.timingSubstate = 0;
            lbl_1_bss_3E80.state = 5U;
            lbl_1_bss_3E80.frame = 0;
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142, &spD4);
            fn_1_5DA4(spD4);
            OSReport(lbl_1_data_161);
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsA[0], lbl_1_data_142, &spC8);
            fn_1_5DA4(spC8);
block_25:
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 5:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            lbl_1_bss_C = 0;
            lbl_1_bss_8 = Hu3DHookFuncCreate(fn_1_38E8);
            Hu3DModelLayerSet(lbl_1_bss_8, 1);
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, 0.0f, 0.0f);
            CharMotionShiftSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][2], 0.0f, 8.0f, 1073741825U);
            if (temp_r31 == 0) {
                fn_1_605C(8);
            }
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(lbl_1_bss_3E80.mainModel, 1073741826U);
            HuAudFXPlay(1628);
            OSReport(lbl_1_data_179);
        }
        if ((s32) lbl_1_bss_3E80.frame == 19) {
            lbl_1_bss_3E80.audioHandle_450 = HuAudFXPlay(1623);
            HuAudFXPitchSet(lbl_1_bss_3E80.audioHandle_450, 0);
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &spBC);
        fn_1_5DA4(spBC);
        if ((s32) lbl_1_bss_3E80.frame >= 19) {
            var_r24 = ((lbl_1_bss_3E80.frame - 19) * 8191) / 46;
            if ((s32) var_r24 >= 8191) {
                var_r24 = 8191;
            }
            HuAudFXPitchSet(lbl_1_bss_3E80.audioHandle_450, (s16) var_r24);
        }
        fn_1_6958(0);
        lbl_1_bss_3E80.frame++;
        break;
    case 6:                                         /* switch 1 */
        var_r29 = 0;
        var_r25 = 0;
        if (GwPlayerConf[temp_r31].type == 1) {
            var_r29 = fn_1_6B90(temp_r31);
        } else {
            var_r29 = HuPadBtnRep[lbl_1_bss_3E80.padNos[temp_r31]];
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &spB0);
        fn_1_5DA4(spB0);
        var_r29 &= 3840;
        switch (lbl_1_bss_3E80.scores[temp_r31] % 4) {                        /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            if (var_r29 == 256) {
                var_r25 = 1;
            }
            break;
        case 1:                                     /* switch 2 */
            if (var_r29 == 512) {
                var_r25 = 1;
            }
            break;
        case 2:                                     /* switch 2 */
            if (var_r29 == 2048) {
                var_r25 = 1;
            }
            break;
        case 3:                                     /* switch 2 */
            if (var_r29 == 1024) {
                var_r25 = 1;
            }
            break;
        }
        fn_1_6958((lbl_1_bss_3E80.scores[temp_r31] % 4) + 1);
        if (var_r25 != 0) {
            if ((s32) lbl_1_bss_3E80.count < 27) {
                lbl_1_bss_3E80.scores[temp_r31]++;
                if ((lbl_1_bss_3E80.scores[temp_r31] % 4) == 0) {
                    Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_189, &lbl_1_bss_3E80.positions_22C[lbl_1_bss_3E80.count]);
                    (lbl_1_bss_3E80.objects[lbl_1_bss_3E80.count])->work[1] = 30;
                    lbl_1_bss_3E80.count++;
                    var_r23 = (lbl_1_bss_3E80.count - 1) * 341;
                    if (var_r23 >= 8192) {
                        var_r23 = 8191;
                    }
                    {
                        int effectHandle = HuAudFXPlay(1626);
                        HuAudFXPitchSet(effectHandle, (s16) var_r23);
                    }
                }
                fn_1_6534((s16) temp_r31, (s16) (lbl_1_bss_3E80.scores[temp_r31] * 90));
                Hu3DMotionTimeSet(lbl_1_bss_3E80.models_184_199[0], 0.0f);
            }
            lbl_1_bss_3E80.from = (f32) lbl_1_bss_3E80.to;
            lbl_1_bss_3E80.to = (f32) (lbl_1_bss_3E80.from - 90.0f);
            lbl_1_bss_3E80.angle = (f32) lbl_1_bss_3E80.from;
            lbl_1_bss_3E80.frame_1BC = 0;
            HuAudFXPlay(1625);
        }
        fn_1_42B8();
        lbl_1_bss_3E80.frame++;
        break;
    case 7:                                         /* switch 1 */
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &spA4);
        fn_1_5DA4(spA4);
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DModelHookReset(lbl_1_bss_3E80.secondaryModels[temp_r31]);
            fn_1_6958(-1);
            omVibrate((s16) temp_r31, 30, 7, 3);
            HuAudFXPlay(1627);
            lbl_1_bss_3E80.audioHandle_450 = HuAudFXPlay(1623);
            HuAudFXPitchSet(lbl_1_bss_3E80.audioHandle_450, 4096);
        }
        if (temp_r31 == 3) {
            if ((lbl_1_bss_3E80.cameraMaxTime - lbl_1_bss_3E80.cameraTime) <= 240.0f) {
                MgSeqModeNext();
            }
            goto block_71;
        }
        if ((lbl_1_bss_3E80.cameraMaxTime - lbl_1_bss_3E80.cameraTime) <= 60.0f) {
            lbl_1_bss_3E80.state = 8U;
            lbl_1_bss_3E80.frame = 0;
            WipeCreate(2, 0, 60);
            HuAudFXFadeOut(lbl_1_bss_3E80.audioHandle_450, 1000);
        } else {
block_71:
            fn_1_42B8();
            lbl_1_bss_3E80.frame++;
        }
        break;
    case 8:                                         /* switch 1 */
        if ((s32) lbl_1_bss_3E80.frame == 0) {
            Hu3DLayerHookReset(3);
            Hu3DModelKill(lbl_1_bss_8);
        }
        if ((s32) lbl_1_bss_3E80.frame >= 60) {
            Hu3DModelHookReset((s16) temp_r28);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[temp_r31], lbl_1_data_624[temp_r31], (s16) temp_r28);
            CharMotionSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r31][5]);
            omSetRot(lbl_1_bss_3E80.playerObjects[temp_r31], 0.0f, -90.0f, 0.0f);
            Hu3DModelHookReset((s16) temp_r28);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[temp_r31], lbl_1_data_684[temp_r31], lbl_1_bss_3E80.secondaryModels[temp_r31]);
            var_r30 = 0;
            while (var_r30 < 4) {
                Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsA[var_r30]);
                var_r30 += 1;
            }
            var_r26 = 0;
            var_r30 = temp_r31 + 1;
            while (var_r30 < 4) {
                if (var_r26 == 0) {
                    omSetRot(lbl_1_bss_3E80.playerObjects[var_r30], 0.0f, 90.0f, 0.0f);
                }
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsA[var_r26], lbl_1_data_5D0[var_r26], lbl_1_bss_3E80.characterModels[var_r30]);
                var_r26 += 1;
                var_r30 += 1;
            }
            Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &sp98);
            fn_1_5DA4(sp98);
            if (temp_r31 < 3) {
                fn_1_5E6C(2);
                WipeCreate(1, 5, 60);
                lbl_1_bss_3E80.activePlayer++;
                Hu3DObjHookSet(lbl_1_bss_3E80.secondaryModels[lbl_1_bss_3E80.activePlayer], lbl_1_data_119, fn_1_3D7C);
                lbl_1_bss_3E80.state = 3U;
                lbl_1_bss_3E80.frame = 0;
            } else {
                MgSeqModeNext();
                goto block_86;
            }
        } else {
block_86:
            fn_1_42B8();
            lbl_1_bss_3E80.frame++;
        }
        break;
    }
    Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
    Hu3DMotionCalc((s16) temp_r28);
    Hu3DMotionCalc(lbl_1_bss_3E80.secondaryModels[temp_r31]);
}

void fn_1_1A2C(s16 arg1, s16 frameNo)
{
    Point3d sp20;
    s16 winners[4];
    s16 var_r30;
    s32 var_r31;
    s32 var_r28;
    s32 streamId;
    s32 var_r29;

    if (frameNo == 0) {
        if (((s32) lbl_1_bss_3E80.scores[0] == 0) && ((s32) lbl_1_bss_3E80.scores[1] == 0) && ((s32) lbl_1_bss_3E80.scores[2] == 0) && ((s32) lbl_1_bss_3E80.scores[3] == 0)) {
            lbl_1_bss_3E80.winners[0] = lbl_1_bss_3E80.winners[1] = lbl_1_bss_3E80.winners[2] = lbl_1_bss_3E80.winners[3] = 0;
            lbl_1_bss_3E80.flag = 0;
            winners[0] = winners[1] = winners[2] = winners[3] = -1;
        } else {
            var_r30 = 0;
            var_r29 = 0;
            var_r28 = 0;
            var_r31 = 0;
            while (var_r31 < 4) {
                if (var_r30 < (s32) lbl_1_bss_3E80.scores[var_r31]) {
                    var_r30 = (s16) lbl_1_bss_3E80.scores[var_r31];
                }
                var_r31 += 1;
            }
            var_r31 = 0;
            while (var_r31 < 4) {
                if (lbl_1_bss_3E80.scores[var_r31] == var_r30) {
                    winners[var_r29++] = var_r31;
                    lbl_1_bss_3E80.winners[var_r31] = 1;
                    if (GwPlayerConf[var_r31].type == 0) {
                        var_r28 = 1;
                    }
                } else {
                    lbl_1_bss_3E80.winners[var_r31] = 0;
                }
                var_r31 += 1;
            }
            while (var_r29 < 4) {
                winners[var_r29] = -1;
                var_r29 += 1;
            }
            if ((var_r28 != 0) && (var_r30 > (s32) (((s32) lbl_1_bss_3E80.record) / 90))) {
                OSReport(lbl_1_data_1B4, (s16) var_r30, ((s32) lbl_1_bss_3E80.record));
                lbl_1_bss_3E80.flag = 1;
                lbl_1_bss_3E80.record = (s32) var_r30;
                MgSeqRecordSet(var_r30 * 90);
            }
        }
        MgSeqWinnerSet(
            winners[0] >= 0 ? lbl_1_bss_3E80.characterNos[winners[0]] : -1,
            winners[1] >= 0 ? lbl_1_bss_3E80.characterNos[winners[1]] : -1,
            winners[2] >= 0 ? lbl_1_bss_3E80.characterNos[winners[2]] : -1,
            winners[3] >= 0 ? lbl_1_bss_3E80.characterNos[winners[3]] : -1);
        streamId = lbl_1_bss_3E80.audioHandle_454;
        if (streamId != -1) {
            HuAudSStreamFadeOut(streamId, 100);
        }
    }
    Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &sp20);
    fn_1_5DA4(sp20);
}

void fn_1_1DD8(s16 arg1, s16 frameNo)
{
    Point3d sp2C;
    Point3d sp20;
    OMOBJ *temp_r3;
    int temp_r28;
    int temp_r27;
    s32 temp_r30;
    s32 var_r31;
    s32 score;

    temp_r30 = lbl_1_bss_3E80.activePlayer;
    temp_r28 = lbl_1_bss_3E80.characterModels[temp_r30];
    temp_r27 = lbl_1_bss_3E80.characterNos[temp_r30];
    if ((frameNo == 0) && (_CheckFlag(196610U) != 0)) {
        var_r31 = 0;
        while (var_r31 < 4) {
            score = lbl_1_bss_3E80.scores[var_r31] * 90;
            GwPlayer[var_r31].mgScore = score;
            var_r31 += 1;
        }
        MgSeqModeSet(9U);
        return;
    }
    if (frameNo == 0) {
        WipeCreate(2, 0, 60);
        HuAudFXFadeOut(lbl_1_bss_3E80.audioHandle_450, 1000);
        lbl_1_bss_3E80.frame = 0;
        var_r31 = 0;
        while (var_r31 < 60) {
            Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_12C, &sp2C);
            fn_1_5DA4(sp2C);
            HuPrcVSleep();
            var_r31 += 1;
        }
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_212_213[0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_212_213[1], 1U);
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[6], 1U);
        Hu3DModelAttrSet(lbl_1_bss_3E80.mainModel, 1U);
        Hu3DModelHookReset(lbl_1_bss_3E80.mainModel);
        Hu3DModelHookReset((s16) temp_r28);
        Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[temp_r30], lbl_1_data_624[temp_r30], (s16) temp_r28);
        CharMotionSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[temp_r30][5]);
        omSetRot(lbl_1_bss_3E80.playerObjects[temp_r30], 0.0f, -90.0f, 0.0f);
        Hu3DModelHookReset((s16) temp_r28);
        Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[temp_r30], lbl_1_data_684[temp_r30], lbl_1_bss_3E80.secondaryModels[temp_r30]);
        fn_1_5E6C(5);
        var_r31 = 0;
        while (var_r31 < 4) {
            temp_r27 = lbl_1_bss_3E80.characterNos[var_r31] = GwPlayerConf[var_r31].charNo;
            Hu3DModelHookReset(lbl_1_bss_3E80.playerModelsB[var_r31]);
            Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[var_r31], lbl_1_data_684[var_r31], lbl_1_bss_3E80.secondaryModels[temp_r30]);
            if ((s32) lbl_1_bss_3E80.winners[var_r31] == 1) {
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsC[var_r31], lbl_1_data_64C[var_r31], lbl_1_bss_3E80.characterModels[var_r31]);
                if (_CheckFlag(65551U) == 0) {
                    GwPlayer[var_r31].mgCoinBonus = 10;
                }
            } else {
                Hu3DModelHookSet(lbl_1_bss_3E80.playerModelsB[var_r31], lbl_1_data_624[var_r31], lbl_1_bss_3E80.characterModels[var_r31]);
            }
            omSetRot(lbl_1_bss_3E80.playerObjects[var_r31], 0.0f, -90.0f, 0.0f);
            CharMotionSet((s16) temp_r27, lbl_1_bss_3E80.characterMotions[var_r31][5]);
            Hu3DModelAttrSet(lbl_1_bss_3E80.characterModels[var_r31], 1073741825U);
            var_r31 += 1;
        }
        if ((_CheckFlag(65551U) == 0) && ((s32) lbl_1_bss_3E80.flag != 0)) {
            temp_r3 = omAddObjEx(lbl_1_bss_3E80.objectManager, 16, 0U, 0U, -1, fn_1_66C0);
            ((float *) temp_r3->work)[0] = 1.0f;
            temp_r3->work[1] = 1;
            temp_r3->work[2] = 0;
        }
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[1], lbl_1_data_1D1, &sp20);
        fn_1_5DA4(sp20);
        WipeCreate(1, 5, 60);
        lbl_1_bss_3E80.frame = 0;
        var_r31 = 0;
        while (var_r31 < 60) {
            HuPrcVSleep();
            var_r31 += 1;
        }
        if (((s32) lbl_1_bss_3E80.flag == 0) || (_CheckFlag(65551U) != 0)) {
            MgSeqModeNext();
        }
    }
}

void fn_1_2374(s16 arg1, s16 frameNo)
{
    int charNo;
    s32 var_r28;
    s32 var_r29;
    s32 var_r31;

    /* Assigned in the winner branch; consumed only when its count is one. */
    var_r29 = 0;
    if (frameNo == 0) {
        OSReport(lbl_1_data_1D6);
        OSReport(lbl_1_data_1D6);
        OSReport(lbl_1_data_1D6);
        var_r31 = 0;
        while (var_r31 < 4) {
            charNo = lbl_1_bss_3E80.characterNos[var_r31] = GwPlayerConf[var_r31].charNo;
            CharMotionSet(charNo, lbl_1_bss_3E80.characterMotions[var_r31][7]);
            var_r31 += 1;
        }
        var_r31 = 0;
        while (var_r31 < 4) {
            charNo = lbl_1_bss_3E80.characterNos[var_r31] = GwPlayerConf[var_r31].charNo;
            if ((s32) lbl_1_bss_3E80.winners[var_r31] == 1) {
                CharMotionShiftSet((s16) charNo, lbl_1_bss_3E80.characterMotions[var_r31][6], 0.0f, 0.5f, 0U);
                var_r29 += 1;
                var_r28 = var_r31;
            } else {
                CharMotionShiftSet((s16) charNo, lbl_1_bss_3E80.characterMotions[var_r31][7], 0.0f, 0.5f, 0U);
            }
            var_r31 += 1;
        }
        if (var_r29 == 1) {
            fn_1_5E6C(var_r28 + 6);
        }
    }
}

void fn_1_2538(s16 mode, s16 frameNo)
{

}

void fn_1_253C(s16 mode, s16 frameNo)
{

}

void fn_1_2540(OMOBJ *arg0)
{
    Point3d sp78;
    Point3d sp6C;
    Point3d sp60;
    Point3d sp54;
    Point3d sp48;
    Point3d sp3C;
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;
    f32 temp_f25;
    f32 temp_f26;
    f32 temp_f31;

    lbl_1_bss_3E80.cameraTime = Hu3DMotionTimeGet(lbl_1_bss_3E80.activeCameraMotion);
    Hu3DCameraPosGet(1, &sp78, &sp60, &sp6C);
    Center = sp6C;
    sp54.x = sp78.x - sp6C.x;
    sp54.y = sp78.y - sp6C.y;
    sp54.z = sp78.z - sp6C.z;
    CRot.x = (f32) (180.0 * (atan2(-sp54.y, (f64) sqrtf((sp54.x * sp54.x) + (sp54.z * sp54.z))) / 3.141592653589793));
    CRot.y = (f32) (180.0 * (atan2((f64) sp54.x, (f64) sp54.z) / 3.141592653589793));
    CRot.z = 0.0f;
    CZoom = sqrtf((sp54.z * sp54.z) + ((sp54.x * sp54.x) + (sp54.y * sp54.y)));
    temp_f26 = CRot.x;
    temp_f25 = CRot.y;
    temp_f31 = CRot.z;
    sp48.x = (f32) ((f64) Center.x + ((f64) CZoom * (sin((3.141592653589793 * (f64) temp_f25) / 180.0) * (cos((3.141592653589793 * (f64) temp_f26) / 180.0)))));
    sp48.y = (f32) ((f64) Center.y + ((f64) CZoom * -sin((3.141592653589793 * (f64) temp_f26) / 180.0)));
    sp48.z = (f32) ((f64) Center.z + ((f64) CZoom * (cos((3.141592653589793 * (f64) temp_f25) / 180.0) * (cos((3.141592653589793 * (f64) temp_f26) / 180.0)))));
    sp3C.x = Center.x;
    sp3C.y = Center.y;
    sp3C.z = Center.z;
    sp24.x = (f32) (sin((3.141592653589793 * (f64) temp_f25) / 180.0) * (sin((3.141592653589793 * (f64) temp_f26) / 180.0)));
    sp24.y = (f32) cos((3.141592653589793 * (f64) temp_f26) / 180.0);
    sp24.z = (f32) (cos((3.141592653589793 * (f64) temp_f25) / 180.0) * (sin((3.141592653589793 * (f64) temp_f26) / 180.0)));
    PSVECSubtract(&sp48, &sp3C, &sp18);
    PSVECNormalize(&sp18, &sp18);
    sp30.x = (f32) (((f64) sp24.x * ((f64) (sp18.x * sp18.x) + ((f64) (1.0f - (sp18.x * sp18.x)) * cos((3.141592653589793 * (f64) temp_f31) / 180.0)))) + ((f64) sp24.y * (((f64) (sp18.x * sp18.y) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) - ((f64) sp18.z * sin((3.141592653589793 * (f64) temp_f31) / 180.0)))) + ((f64) sp24.z * (((f64) (sp18.x * sp18.z) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) + ((f64) sp18.y * sin((3.141592653589793 * (f64) temp_f31) / 180.0)))));
    sp30.y = (f32) (((f64) sp24.y * ((f64) (sp18.y * sp18.y) + ((f64) (1.0f - (sp18.y * sp18.y)) * cos((3.141592653589793 * (f64) temp_f31) / 180.0)))) + ((f64) sp24.x * (((f64) (sp18.x * sp18.y) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) + ((f64) sp18.z * sin((3.141592653589793 * (f64) temp_f31) / 180.0)))) + ((f64) sp24.z * (((f64) (sp18.y * sp18.z) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) - ((f64) sp18.x * sin((3.141592653589793 * (f64) temp_f31) / 180.0)))));
    sp30.z = (f32) (((f64) sp24.z * ((f64) (sp18.z * sp18.z) + ((f64) (1.0f - (sp18.z * sp18.z)) * cos((3.141592653589793 * (f64) temp_f31) / 180.0)))) + (((f64) sp24.x * (((f64) (sp18.x * sp18.z) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) - ((f64) sp18.y * sin((3.141592653589793 * (f64) temp_f31) / 180.0)))) + ((f64) sp24.y * (((f64) (sp18.y * sp18.z) * (1.0 - cos((3.141592653589793 * (f64) temp_f31) / 180.0))) + ((f64) sp18.x * sin((3.141592653589793 * (f64) temp_f31) / 180.0))))));
    PSVECNormalize(&sp30, &sp30);
    Hu3DCameraPosSet(1, sp48.x, sp48.y, sp48.z, sp30.x, sp30.y, sp30.z, sp3C.x, sp3C.y, sp3C.z);
}

void fn_1_2FD8(s16 modelId, s16 motId, BOOL lagF)
{
    s32 spC;
    f32 var_f31;
    s16 temp_r30;
    s32 temp_r31;

    temp_r31 = lbl_1_bss_3E80.activePlayer;
    spC = (s32) lbl_1_bss_3E80.characterModels[temp_r31];
    temp_r30 = lbl_1_bss_3E80.characterNos[temp_r31];
    if (lagF == 1) {
        switch ((s32) lbl_1_bss_3E80.timingSubstate) { /* irregular */
        case 0:
            (lbl_1_bss_3E80.playerObjects[temp_r31])->objFunc = fn_1_33A4;
            CharMotionShiftSet((s16) temp_r30, lbl_1_bss_3E80.characterMotions[temp_r31][3], 0.0f, 8.0f, 1073741825U);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[temp_r31]);
            fn_1_605C(0);
            OSReport(lbl_1_data_217, lbl_1_bss_3E80.frame);
            lbl_1_bss_3E80.state = 6;
            lbl_1_bss_3E80.frame = 0;
            HuAudFXPlay(1624);
            HuAudFXStop(lbl_1_bss_3E80.audioHandle_450);
            lbl_1_bss_3E80.timingSubstate++;
            return;
        case 1:
            CharMotionShiftSet((s16) temp_r30, lbl_1_bss_3E80.characterMotions[temp_r31][0], 0.0f, 5.0f, 1073741825U);
            lbl_1_bss_3E80.timingSubstate++;
            return;
        case 2:
            CharMotionShiftSet((s16) temp_r30, lbl_1_bss_3E80.characterMotions[temp_r31][4], 0.0f, 8.0f, 0U);
            CharMotionSpeedSet((s16) temp_r30, 1.0f);
            Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[temp_r31], 1.0f);
            Hu3DModelAttrReset(lbl_1_bss_3E80.characterModels[temp_r31], 1073741825U);
            Hu3DMotionCalc(lbl_1_bss_3E80.characterModels[temp_r31]);
            (lbl_1_bss_3E80.playerObjects[temp_r31])->objFunc = fn_1_34D0;
            lbl_1_bss_3E80.state = 7;
            lbl_1_bss_3E80.frame = 0;
            var_f31 = lbl_1_bss_3E80.angle;
            while (var_f31 < -360.0f) {
                var_f31 += 360.0f;
            }
            if ((var_f31 == 0.0f) || (var_f31 == 360.0f)) {
                lbl_1_bss_3E80.from = 0.0f;
                lbl_1_bss_3E80.to = 0.0f;
            } else {
                lbl_1_bss_3E80.from = var_f31;
                lbl_1_bss_3E80.to = -360.0f;
            }
            lbl_1_bss_3E80.angle = var_f31;
            lbl_1_bss_3E80.frame_1BC = 0;
            fn_1_605C(1);
            fn_1_605C(7);
            lbl_1_bss_3E80.timingSubstate++;
            break;
        }
    }
}

void fn_1_33A4(OMOBJ *arg0)
{
    lbl_1_bss_3E80.angle = (f32) (lbl_1_bss_3E80.from + ((f32) lbl_1_bss_3E80.frame_1BC * ((lbl_1_bss_3E80.to - lbl_1_bss_3E80.from) / 5.0f)));
    if ((s32) lbl_1_bss_3E80.frame_1BC < 5) {
        lbl_1_bss_3E80.frame_1BC += 1.0f;
    }
    omSetRot(arg0, 0.0f, lbl_1_bss_3E80.angle, 0.0f);
}

void fn_1_34D0(OMOBJ *arg0)
{
    lbl_1_bss_3E80.angle = (f32) (lbl_1_bss_3E80.from + ((f32) lbl_1_bss_3E80.frame_1BC * ((lbl_1_bss_3E80.to - lbl_1_bss_3E80.from) / 10.0f)));
    if ((s32) lbl_1_bss_3E80.frame_1BC < 10) {
        lbl_1_bss_3E80.frame_1BC += 1.0f;
    }
    CharMotionSpeedSet(lbl_1_bss_3E80.characterNos[lbl_1_bss_3E80.activePlayer], 0.5f);
    Hu3DAnimSpeedSet(lbl_1_bss_3E80.characterModels[lbl_1_bss_3E80.activePlayer], 1.0f);
    omSetRot(arg0, 0.0f, lbl_1_bss_3E80.angle, 0.0f);
    if (Hu3DMotionEndCheck(lbl_1_bss_3E80.characterModels[lbl_1_bss_3E80.activePlayer]) == 1) {
        s16 charNo = lbl_1_bss_3E80.characterNos[lbl_1_bss_3E80.activePlayer];
        CharMotionShiftSet((s16) charNo, lbl_1_bss_3E80.characterMotions[lbl_1_bss_3E80.activePlayer][0], 0.0f, 5.0f, 1073741825U);
        OSReport(lbl_1_data_227);
        arg0->objFunc = NULL;
    }
}

void fn_1_3718(OMOBJ *arg0)
{
    float x, y, z;
    u32 temp_r30;

    temp_r30 = arg0->work[0];
    if ((s32) lbl_1_bss_3E80.count > (s32) temp_r30) {
        if ((u32) (Hu3DModelAttrGet(lbl_1_bss_3E80.models[temp_r30]) & 1) != 0) {
            Hu3DModelAttrReset(lbl_1_bss_3E80.models[temp_r30], 1U);
            Hu3DMotionTimeSet(lbl_1_bss_3E80.models[temp_r30], 0.0f);
        }
        if ((u32) arg0->work[1] != 0) {
            x = lbl_1_bss_10.points[lbl_1_bss_C - 1].x;
            y = lbl_1_bss_10.points[lbl_1_bss_C - 1].y;
            z = lbl_1_bss_10.points[lbl_1_bss_C - 1].z;
            omSetTra(arg0, x, y, z);
            arg0->work[1] -= 1;
        }
    }
}

void fn_1_388C(OMOBJ *arg0)
{
    u32 temp_r31;

    temp_r31 = arg0->work[0];
    *arg0->mdlId = lbl_1_bss_3E80.models[temp_r31];
    *arg0->mtnId = lbl_1_bss_3E80.motions[temp_r31];
    arg0->objFunc = fn_1_3718;
}

void fn_1_38E8(HU3D_MODEL *modelP, f32 (*mtx)[3][4])
{

}

void fn_1_38EC(Point3d position)
{

}

void fn_1_38F0(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(sp10, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]], 0.0f);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_39B4(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(sp10, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]], 0.0f);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3A78(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(sp10, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]], 0.0f);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3B3C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(sp10, 0.0f, lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]], 0.0f);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3C00(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3C84(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxRotCat(sp10, 0.0f, lbl_1_bss_4 + (lbl_1_data_60[lbl_1_bss_3E80.characterNos[0]] + lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]]), 0.0f);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3D7C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;

    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3E00(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;
    int temp_r30;

    temp_r30 = lbl_1_bss_3E80.characterNos[0];
    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    if (temp_r30 == 8) {
        mtxRotCat(sp10, 0.0f, -(lbl_1_bss_4 + (lbl_1_data_60[lbl_1_bss_3E80.characterNos[0]] + lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]])), 0.0f);
    }
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_3F18(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;
    int temp_r30;

    temp_r30 = lbl_1_bss_3E80.characterNos[0];
    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    if (temp_r30 == 8) {
        mtxRotCat(sp10, 0.0f, lbl_1_bss_0 + -lbl_1_data_28[lbl_1_bss_3E80.characterNos[0]], 0.0f);
    }
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_400C(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;
    int temp_r30;

    temp_r30 = lbl_1_bss_3E80.characterNos[1];
    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    if (temp_r30 == 8) {
        mtxRotCat(sp10, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[1]], 0.0f);
    }
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_40F0(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;
    int temp_r30;

    temp_r30 = lbl_1_bss_3E80.characterNos[2];
    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    if (temp_r30 == 8) {
        mtxRotCat(sp10, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[2]], 0.0f);
    }
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_41D4(HSF_OBJECT *obj, HSF_TRANSFORM *transform, f32 (*prev)[3][4], f32 (*curr)[3][4])
{
    Mtx sp10;
    int temp_r30;

    temp_r30 = lbl_1_bss_3E80.characterNos[3];
    PSMTXIdentity(sp10);
    PSMTXScale(sp10, transform->scale.x, transform->scale.y, transform->scale.z);
    mtxRotCat(sp10, transform->rot.x, transform->rot.y, transform->rot.z);
    if (temp_r30 == 8) {
        mtxRotCat(sp10, 0.0f, -lbl_1_data_28[lbl_1_bss_3E80.characterNos[3]], 0.0f);
    }
    mtxTransCat(sp10, transform->pos.x, transform->pos.y, transform->pos.z);
    PSMTXConcat(*prev, sp10, *curr);
}

void fn_1_42B8(void)
{
    Point3d sp8;

    if ((s32) (lbl_1_bss_C + 1) < 660) {
        Hu3DMotionCalc(lbl_1_bss_3E80.mainModel);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.mainModel, lbl_1_data_235, &sp8);
        lbl_1_bss_10.points[lbl_1_bss_C] = sp8;
        lbl_1_bss_C += 1;
        return;
    }
    OSReport(lbl_1_data_242);
}

void fn_1_437C(s16 layerNo)
{
    Mtx sp8;

    Hu3DModelObjMtxGet(lbl_1_bss_3E80.mainModel, lbl_1_data_24E, sp8);
    Hu3DModelMtxSet(lbl_1_bss_3E80.models_184_199[0], &sp8);
}

char lbl_1_data_1B4[29] = "*** new record ... %d -> %d\012";

char lbl_1_data_1D1[5] = "p2ls";

char lbl_1_data_1D6[65] = "###############################################################\012";

char lbl_1_data_217[16] = "now timing(%d)\012";

char lbl_1_data_227[14] = "*** loop ***\012";

char lbl_1_data_235[13] = "pmov_ptEFatt";

char lbl_1_data_242[12] = "*******max\012";

char lbl_1_data_24E[18] = "snowBDEFhook\000\000\000\000\000";
