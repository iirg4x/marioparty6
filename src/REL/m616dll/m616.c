#include "REL/m616dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/pad.h"
#include "game/frand.h"
#include "datadir_enum.h"
#include <string.h>

HuVecF lbl_1_bss_0;
M616Work lbl_1_bss_10;

static MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_140, fn_1_160, fn_1_4BC, fn_1_4C0, fn_1_F14,
    fn_1_F78, fn_1_1284, fn_1_135C, fn_1_1360
};

static s32 lbl_1_data_1F0[17] = {
    DATANUM(DATA_m616, 0), DATANUM(DATA_m616, 1), DATANUM(DATA_m616, 2),
    DATANUM(DATA_m616, 3), DATANUM(DATA_m616, 6), DATANUM(DATA_m616, 4),
    DATANUM(DATA_m616, 7), DATANUM(DATA_m616, 8), DATANUM(DATA_m616, 7),
    DATANUM(DATA_m616, 5), DATANUM(DATA_m616, 11), DATANUM(DATA_m616, 9),
    DATANUM(DATA_m616, 11), DATANUM(DATA_m616, 10), DATANUM(DATA_m616, 11),
    DATANUM(DATA_m616, 9), DATANUM(DATA_m616, 11)
};

static s32 lbl_1_data_234[6] = {
    DATANUM(DATA_m616, 22), DATANUM(DATA_m616, 23), DATANUM(DATA_m616, 24),
    DATANUM(DATA_m616, 25), DATANUM(DATA_m616, 26), DATANUM(DATA_m616, 27)
};

static s32 lbl_1_data_24C[13] = {
    DATANUM(DATA_m616, 28), DATANUM(DATA_m616, 29), DATANUM(DATA_m616, 30),
    DATANUM(DATA_m616, 31), DATANUM(DATA_m616, 33), DATANUM(DATA_m616, 32),
    DATANUM(DATA_m616, 34), DATANUM(DATA_m616, 35), DATANUM(DATA_m616, 36),
    DATANUM(DATA_m616, 37), DATANUM(DATA_m616, 38), DATANUM(DATA_m616, 39),
    DATANUM(DATA_m616, 40)
};

static unsigned int lbl_1_data_280[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 3), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 111), DATANUM(DATA_mariomot, 112),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 36), DATANUM(DATA_mariomot, 37),
    DATANUM(DATA_mario, 154), DATANUM(DATA_mario, 155), 0
};

typedef struct M616CpuParam {
    u32 unk_00;
    u32 repeatPercent;
} M616CpuParam;

static M616CpuParam lbl_1_data_2B0[4] = {
    { 80, 50 }, { 85, 30 }, { 90, 10 }, { 100, 0 }
};

static s32 lbl_1_data_2D0[3] = {
    DATANUM(DATA_m616, 16), DATANUM(DATA_m616, 17), DATANUM(DATA_m616, 18)
};

s32 fn_1_A0(s32 streamNo, s32 bgmId)
{
    s32 result = streamNo;

    if (result == -1) {
        if (GameMesFXPlayCheck(MgSeqGameMesIdGet()) != 0) {
            result = HuAudBGMPlay((s16)bgmId);
        }
    }
    return result;
}

void fn_1_104(s32 streamNo)
{
    if (streamNo != -1) {
        HuAudSStreamFadeOut(streamNo, 100);
    }
}

void fn_1_140(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

void fn_1_160(s16 mode, s16 frameNo)
{
    s32 player;
    HU3D_MODELID model;
    Mtx matrix;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;

    if (frameNo == 0) {
        for (player = 0; player < 4; player++) {
            fn_1_215C(player, 5);
            if (player != 0) {
                Hu3DModelHookSet(lbl_1_bss_10.playerModels[player], "P1st",
                    lbl_1_bss_10.characterModels[player]);
                fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
            } else {
                fn_1_22BC(player, 4, HU3D_MOTATTR_LOOP);
                Hu3DMotionTimeSet(lbl_1_bss_10.unk_CE, 0.0f);
                Hu3DMotionTimingHookSet(lbl_1_bss_10.unk_CE, fn_1_139C);
            }
        }
        fn_1_1FF4(0);
        lbl_1_bss_10.timingState = 0;
        MgSeqModeChangeOff();
        lbl_1_bss_10.streamNo = HuAudSStreamPlay(83);
    }
    model = lbl_1_bss_10.characterModels[0];
    Hu3DModelObjMtxGet(lbl_1_bss_10.unk_CE, "P1stmov", matrix);
    Hu3DMtxTransGet(matrix, &pos);
    Hu3DMtxRotGet(matrix, &rot);
    Hu3DMtxScaleGet(matrix, &scale);
    Hu3DModelPosSetV(model, &pos);
    Hu3DModelRotSetV(model, &rot);
    Hu3DModelScaleSetV(model, &scale);
    OSReport("pos %f, %f, %f\n", pos.x, pos.y, pos.z);
    Hu3DModelObjPosGet(lbl_1_bss_10.unk_CE, "P1stmov", &lbl_1_bss_0);
    if (fn_1_20D8()) {
        HU3D_MODELID character;
        HuVecF origin;
        HuVecF rotation;
        HuVecF unitScale;

        Hu3DMotionTimingHookReset(lbl_1_bss_10.unk_CE);
        Hu3DModelHookReset(lbl_1_bss_10.unk_CE);
        Hu3DModelHookSet(lbl_1_bss_10.playerModels[0], "P1st",
            lbl_1_bss_10.characterModels[0]);
        character = lbl_1_bss_10.characterModels[0];
        origin.x = 0.0f;
        origin.y = 0.0f;
        origin.z = 0.0f;
        rotation.x = 0.0f;
        rotation.y = 0.0f;
        rotation.z = 0.0f;
        unitScale.x = 1.0f;
        unitScale.y = 1.0f;
        unitScale.z = 1.0f;
        Hu3DModelPosSetV(character, &origin);
        Hu3DModelRotSetV(character, &rotation);
        Hu3DModelScaleSetV(character, &unitScale);
        fn_1_1FF4(1);
        for (player = 0; player < 4; player++) {
            fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
            lbl_1_bss_10.previousButtons[player] = 0;
        }
        fn_1_23B0();
        lbl_1_bss_10.sequenceState = 1;
        lbl_1_bss_10.sequenceFrame = 0;
        MgSeqModeNext();
    }
}

void fn_1_4BC(s16 mode, s16 frameNo)
{
}

void fn_1_4C0(s16 mode, s16 frameNo)
{
    s32 player;
    s32 choice;
    HU3D_MOTIONID motion;
    HU3D_MODELID model;

    switch (lbl_1_bss_10.sequenceState) {
    case 0:
        if (lbl_1_bss_10.sequenceFrame == 0) {
            for (player = 0; player < 4; player++) {
                fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
                lbl_1_bss_10.previousButtons[player] = 0;
            }
            fn_1_23B0();
        }
        if (lbl_1_bss_10.sequenceFrame >= 120) {
            lbl_1_bss_10.sequenceState = 1;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 1:
        for (player = 0; player < 4; player++) {
            fn_1_22BC(player, 9, HU3D_MOTATTR_LOOP);
            lbl_1_bss_10.choices[player] = 0;
            fn_1_215C(player, 5);
        }
        MgTimerParamSet(lbl_1_bss_10.timer, 300, 0, 0);
        MgTimerModeOnSet(lbl_1_bss_10.timer, MGTIMER_OFFTYPE_FADEOUT);
        lbl_1_bss_10.sequenceState = 2;
        lbl_1_bss_10.sequenceFrame = 0;
        break;
    case 2:
        if (MgTimerDoneCheck(lbl_1_bss_10.timer)) {
            lbl_1_bss_10.sequenceState = 3;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        for (player = 0; player < 4; player++) {
            choice = lbl_1_bss_10.choices[player];
            if (GwPlayerConf[player].type == 1) {
                if (lbl_1_bss_10.sequenceFrame >= lbl_1_bss_10.cpuInputFrames[player]) {
                    choice = fn_1_2580(player);
                } else {
                    choice = 0;
                }
            } else {
                u16 buttons = HuPadBtn[lbl_1_bss_10.padNos[player]]
                    & (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R);
                switch (buttons & ~lbl_1_bss_10.previousButtons[player]) {
                case PAD_BUTTON_A:
                    choice = 1;
                    break;
                case PAD_BUTTON_B:
                    choice = 2;
                    break;
                case PAD_BUTTON_TRIGGER_L:
                    choice = 3;
                    break;
                case PAD_BUTTON_TRIGGER_R:
                    choice = 4;
                    break;
                default:
                    OSReport("同時！！！\n");
                    if (buttons & ~lbl_1_bss_10.previousButtons[player] & PAD_BUTTON_A) {
                        choice = 1;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[player] & PAD_BUTTON_B) {
                        choice = 2;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[player] & PAD_BUTTON_TRIGGER_L) {
                        choice = 3;
                    } else if (buttons & ~lbl_1_bss_10.previousButtons[player] & PAD_BUTTON_TRIGGER_R) {
                        choice = 4;
                    }
                    break;
                }
                lbl_1_bss_10.previousButtons[player] = buttons;
            }
            if (choice != lbl_1_bss_10.choices[player]) {
                fn_1_22BC(player, 10, 0);
                omVibrate(player, 10, 7, 3);
                fn_1_215C(player, 12);
                HuAudFXPlay(1741);
            }
            lbl_1_bss_10.choices[player] = choice;
            motion = lbl_1_bss_10.characterMotions[player][10];
            model = lbl_1_bss_10.characterModels[player];
            if (Hu3DMotionShiftIDGet(model) < 0 && motion == Hu3DMotionIDGet(model)
                && Hu3DMotionEndCheck(model)) {
                fn_1_22BC(player, 9, 0);
                OSReport("push end\n");
            }
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 3:
        if (lbl_1_bss_10.sequenceFrame == 0) {
            for (player = 0; player < 4; player++) {
                s32 motions[5] = { 7, 0, 1, 2, 3 };

                fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
                if (lbl_1_bss_10.choices[player] != 0) {
                    fn_1_215C(player, motions[lbl_1_bss_10.choices[player]]);
                }
            }
        } else if (lbl_1_bss_10.sequenceFrame >= 60) {
            lbl_1_bss_10.sequenceState = 4;
            lbl_1_bss_10.sequenceFrame = 0;
            break;
        }
        lbl_1_bss_10.sequenceFrame++;
        break;
    case 4:
        {
            s32 j;
            s32 winners = 0;
            s32 count;
            s32 winner[4];

            OSReport("entry ... %d, %d, %d, %d\n", lbl_1_bss_10.choices[0], lbl_1_bss_10.choices[1],
                lbl_1_bss_10.choices[2], lbl_1_bss_10.choices[3]);
            for (player = 0; player < 4; player++) {
                if (lbl_1_bss_10.choices[player] == 0) {
                    winner[player] = 0;
                } else {
                    count = 0;
                    for (j = 0; j < 4; j++) {
                        if (lbl_1_bss_10.choices[j] == lbl_1_bss_10.choices[player]) {
                            count++;
                        }
                    }
                    if (count == 1) {
                        winner[player] = 1;
                    } else {
                        winner[player] = 0;
                    }
                }
                if (winner[player] == 1) {
                    winners++;
                }
            }
            if (winners != 0) {
                for (player = 0; player < 4; player++) {
                    if (winner[player] == 1) {
                        lbl_1_bss_10.scores[player]++;
                        fn_1_22BC(player, 7, 0);
                        CharFXPlay(lbl_1_bss_10.characterNos[player], 579);
                        fn_1_2104(player, lbl_1_bss_10.scores[player]);
                        switch (lbl_1_bss_10.choices[player]) {
                        case 1:
                            fn_1_215C(player, 8);
                            break;
                        case 2:
                            fn_1_215C(player, 9);
                            break;
                        case 3:
                            fn_1_215C(player, 10);
                            break;
                        case 4:
                            fn_1_215C(player, 11);
                            break;
                        }
                    } else {
                        fn_1_22BC(player, 8, 0);
                    }
                }
                fn_1_2254(0);
                HuAudFXPlay(1742);
                HuAudFXPlay(1743);
                lbl_1_bss_10.sequenceState = 5;
                lbl_1_bss_10.sequenceFrame = 0;
            } else {
                for (player = 0; player < 4; player++) {
                    fn_1_22BC(player, 8, 0);
                }
                if (lbl_1_bss_10.roundNo == 9) {
                    lbl_1_bss_10.sequenceState = 8;
                    lbl_1_bss_10.sequenceFrame = 0;
                } else {
                    lbl_1_bss_10.roundNo++;
                    lbl_1_bss_10.sequenceState = 0;
                    lbl_1_bss_10.sequenceFrame = 0;
                }
            }
        }
        break;
    case 5:
        for (player = 0; player < 4; player++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_10.characterModels[player])) {
                fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
            }
        }
        {
            for (player = 0; player < 4; player++) {
                if (!Hu3DMotionEndCheck(lbl_1_bss_10.playerModels[player])) {
                    break;
                }
            }
            if (player == 4) {
                HuAudFXPlay(1745);
                HuAudFXPlay(1744);
                for (player = 0; player < 4; player++) {
                    if (lbl_1_bss_10.scores[player] == 3) {
                        break;
                    }
                }
                if (player != 4 || lbl_1_bss_10.roundNo == 9) {
                    lbl_1_bss_10.sequenceState = 8;
                    lbl_1_bss_10.sequenceFrame = 0;
                } else {
                    lbl_1_bss_10.roundNo++;
                    lbl_1_bss_10.sequenceState = 0;
                    lbl_1_bss_10.sequenceFrame = 0;
                }
            } else {
                lbl_1_bss_10.sequenceFrame++;
            }
        }
        break;
    case 8:
        MgSeqModeNext();
        break;
    }
}

void fn_1_F14(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        fn_1_104(lbl_1_bss_10.streamNo);
        OSReport("enter sequence ... finish\n");
    }
}

void fn_1_F78(s16 mode, s16 frameNo)
{
    s32 i;
    s32 mask;
    s32 winnerCount = 0;
    s16 motion = -1;
    s16 winners[4] = { -1, -1, -1, -1 };
    BOOL draw = TRUE;

    if (frameNo == 0) {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10.scores[i] == 3) {
                winners[winnerCount++] = lbl_1_bss_10.characterNos[i];
                if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                    GwPlayer[i].mgCoinBonus = 10;
                }
                draw = FALSE;
            }
        }
        MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        if (!draw) {
            for (i = 0; i < 4; i++) {
                if (lbl_1_bss_10.scores[i] == 1) {
                    fn_1_2104(i, 4);
                    if (motion != 1) {
                        motion = 1;
                    }
                } else if (lbl_1_bss_10.scores[i] == 2) {
                    fn_1_2104(i, 5);
                    if (motion == -1) {
                        motion = 2;
                    }
                }
                fn_1_215C(i, 5);
            }
            if (motion != -1) {
                fn_1_2254(motion);
                HuAudFXPlay(1742);
                HuAudFXPlay(1743);
            }
            mask = 0;
            {
                s32 cameras[16] = { 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
                for (i = 0; i < 4; i++) {
                    if (lbl_1_bss_10.scores[i] == 3) {
                        mask |= 1 << i;
                    }
                }
                fn_1_1FF4(cameras[mask]);
            }
        }
        OSReport("enter sequence ... end\n");
    } else {
        for (i = 0; i < 4; i++) {
            if (!Hu3DMotionEndCheck(lbl_1_bss_10.playerModels[i])) {
                break;
            }
        }
        if (i == 4) {
            HuAudFXPlay(1745);
            HuAudFXPlay(1744);
            MgSeqModeNext();
        }
    }
}

void fn_1_1284(s16 mode, s16 frameNo)
{
    s32 player;

    if (frameNo == 0) {
        for (player = 0; player < 4; player++) {
            if (lbl_1_bss_10.scores[player] == 3) {
                fn_1_22BC(player, 2, 0);
                Hu3DMotionTimeSet(lbl_1_bss_10.resultModels[player], 0.0f);
                Hu3DModelAttrReset(lbl_1_bss_10.resultModels[player], HU3D_ATTR_DISPOFF);
            } else {
                fn_1_22BC(player, 3, 0);
            }
        }
        OSReport("enter sequence ... result\n");
    }
}

void fn_1_135C(s16 mode, s16 frameNo)
{
}

void fn_1_1360(s16 mode, s16 frameNo)
{
}

void fn_1_1364(OMOBJ *object)
{
    lbl_1_bss_10.cameraTime = Hu3DMotionTimeGet(lbl_1_bss_10.activeCameraModel);
}

void fn_1_139C(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lagF)
{
    if (lagF == TRUE) {
        switch (lbl_1_bss_10.timingState) {
        case 0:
            fn_1_22BC(0, 1, 0);
            lbl_1_bss_10.timingState++;
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], FALSE);
            break;
        case 1:
            fn_1_22BC(0, 6, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 2:
            fn_1_22BC(0, 4, HU3D_MOTATTR_LOOP);
            CharModelLandDustCreate(lbl_1_bss_10.characterNos[0], &lbl_1_bss_0);
            lbl_1_bss_10.timingState++;
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], TRUE);
            break;
        case 3:
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], FALSE);
            fn_1_22BC(0, 5, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 4:
            fn_1_22BC(0, 6, 0);
            lbl_1_bss_10.timingState++;
            break;
        case 5:
            CharModelVoiceFlagSet(lbl_1_bss_10.characterNos[0], TRUE);
            fn_1_22BC(0, 0, 0);
            CharModelLandDustCreate(lbl_1_bss_10.characterNos[0], &lbl_1_bss_0);
            lbl_1_bss_10.timingState++;
            break;
        }
    }
}

void fn_1_1590(void)
{
    s32 i;
    s32 j;
    s16 character;
    HU3D_LIGHTID light;

    memset(&lbl_1_bss_10, 0, sizeof(lbl_1_bss_10));
    lbl_1_bss_10.activeCameraModel = HU3D_MODELID_NONE;
    lbl_1_bss_10.objectManager = omInitObjMan(100, 8192);
    omGameSysInit(lbl_1_bss_10.objectManager);
    lbl_1_bss_10.shadowPos.x = 0.0f;
    lbl_1_bss_10.shadowPos.y = 3000.0f;
    lbl_1_bss_10.shadowPos.z = 0.0f;
    lbl_1_bss_10.shadowUp.x = 0.0f;
    lbl_1_bss_10.shadowUp.y = 0.0f;
    lbl_1_bss_10.shadowUp.z = 1.0f;
    lbl_1_bss_10.shadowTarget.x = 0.0f;
    lbl_1_bss_10.shadowTarget.y = -100.0f;
    lbl_1_bss_10.shadowTarget.z = 0.0f;
    Hu3DShadowCreate(30.0f, 1.0f, 13000.0f);
    Hu3DShadowPosSet(&lbl_1_bss_10.shadowPos, &lbl_1_bss_10.shadowUp, &lbl_1_bss_10.shadowTarget);
    Hu3DShadowColSet(50, 50, 50);
    Hu3DShadowSizeSet(192);
    Hu3DShadowTPLvlSet(0.7f);
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45.0f, 1.0f, 10000.0f, 1.2f);
    lbl_1_bss_10.cameraObject = omAddObjEx(lbl_1_bss_10.objectManager, 12288, 0, 0, -1, fn_1_1364);
    for (i = 0; i < 17; i++) {
        lbl_1_bss_10.cameraMotions[i] = Hu3DMotionCreateData(lbl_1_data_1F0[i]);
    }
    lbl_1_bss_10.unk_CC = Hu3DModelCreateData(DATANUM(DATA_m616, 20));
    Hu3DModelShadowSet(lbl_1_bss_10.unk_CC);
    lbl_1_bss_10.unk_CE = Hu3DModelCreateData(DATANUM(DATA_m616, 12));
    Hu3DModelShadowSet(lbl_1_bss_10.unk_CE);
    lbl_1_bss_10.resultModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 41));
    lbl_1_bss_10.resultModels[1] = Hu3DModelCreateData(DATANUM(DATA_m616, 42));
    lbl_1_bss_10.resultModels[2] = Hu3DModelCreateData(DATANUM(DATA_m616, 43));
    lbl_1_bss_10.resultModels[3] = Hu3DModelCreateData(DATANUM(DATA_m616, 44));
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(lbl_1_bss_10.resultModels[i], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_10.stageModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 13));
    Hu3DModelAttrSet(lbl_1_bss_10.stageModels[0], HU3D_MOTATTR_LOOP);
    Hu3DModelShadowMapObjSet(lbl_1_bss_10.stageModels[0], "616erande-stage01");
    lbl_1_bss_10.stageModels[1] = Hu3DModelCreateData(DATANUM(DATA_m616, 14));
    Hu3DModelAttrSet(lbl_1_bss_10.stageModels[1], HU3D_MOTATTR_LOOP);
    Hu3DModelShadowMapObjSet(lbl_1_bss_10.stageModels[1], "616erande-stage001");
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10.playerBaseModels[i] = Hu3DModelCreateData(DATANUM(DATA_m616, 21));
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 13; j++) {
            lbl_1_bss_10.playerBaseMotions[i][j] =
                Hu3DJointMotionData(lbl_1_bss_10.playerBaseModels[i], lbl_1_data_24C[j]);
        }
    }
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p1daiiti", lbl_1_bss_10.playerBaseModels[0]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p2daiiti", lbl_1_bss_10.playerBaseModels[1]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p3daiiti", lbl_1_bss_10.playerBaseModels[2]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p4daiiti", lbl_1_bss_10.playerBaseModels[3]);
    lbl_1_bss_10.centerModel = Hu3DModelCreateData(DATANUM(DATA_m616, 15));
    for (i = 0; i < 3; i++) {
        lbl_1_bss_10.centerMotions[i] = Hu3DJointMotionData(lbl_1_bss_10.centerModel, lbl_1_data_2D0[i]);
    }
    lbl_1_bss_10.playerModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 19));
    for (i = 1; i < 4; i++) {
        lbl_1_bss_10.playerModels[i] = Hu3DModelLink(lbl_1_bss_10.playerModels[0]);
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_10.playerMotions[i] = Hu3DJointMotionData(lbl_1_bss_10.playerModels[0], lbl_1_data_234[i]);
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelShadowMapObjSet(lbl_1_bss_10.playerModels[i], "p1neji_sha");
    }
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p1nejiiti", lbl_1_bss_10.playerModels[0]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p2nejiiti", lbl_1_bss_10.playerModels[1]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p3nejiiti", lbl_1_bss_10.playerModels[2]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p4nejiiti", lbl_1_bss_10.playerModels[3]);
    fn_1_2104(0, 0);
    fn_1_2104(1, 0);
    fn_1_2104(2, 0);
    fn_1_2104(3, 0);
    {
        HuVecF pos = { 100.0f, 800.0f, 1000.0f };
        HuVecF dir = { 0.3f, -0.8f, 0.3f };
        HuVecF unk_90 = { 20.0f, 45.0f, 1000.0f };
        GXColor color = { 255, 255, 255, 255 };
        HuVecF aim;
        HuVecF lightPos;

        light = Hu3DGLightCreateV(&pos, &dir, &color);
        Hu3DGLightInfinitytSet(light);
        Hu3DGLightStaticSet(light, TRUE);
        aim.x = 0.0f;
        aim.y = 0.0f;
        aim.z = 0.0f;
        lightPos.x = 1.0f;
        lightPos.y = 7825.0f;
        lightPos.z = 9582.0f;
        Hu3DGLightPosAimSetV(light, &lightPos, &aim);
    }
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10.padNos[i] = GwPlayerConf[i].padNo;
        character = GwPlayerConf[i].charNo;
        lbl_1_bss_10.characterNos[i] = character;
        lbl_1_bss_10.padNos[i] = GwPlayerConf[i].padNo;
        lbl_1_bss_10.characterModels[i] = CharModelMotListCreate(character, CHAR_MODEL2,
            lbl_1_data_280, lbl_1_bss_10.characterMotions[i]);
        Hu3DModelShadowSet(lbl_1_bss_10.characterModels[i]);
        CharMotionVoiceOnSet(character, 36, FALSE);
        CharMotionVoiceOnSet(character, 37, FALSE);
    }
    lbl_1_bss_10.timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
    CharEffectLayerSet(3);
    fn_1_1FF4(1);
    lbl_1_bss_10.streamNo = -1;
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_1FF4(s32 index)
{
    HU3D_MODELID model;
    HU3D_MOTIONID motion;

    motion = lbl_1_bss_10.cameraMotions[index];
    model = Hu3DModelCameraCreate(motion, HU3D_CAM0);
    Hu3DCameraMotionStart(model, HU3D_CAM0);
    lbl_1_bss_10.cameraTime = 0.0f;
    lbl_1_bss_10.cameraMaxTime = Hu3DMotionMaxTimeGet(model);
    if (lbl_1_bss_10.activeCameraModel != HU3D_MODELID_NONE) {
        Hu3DModelKill(lbl_1_bss_10.activeCameraModel);
    }
    lbl_1_bss_10.activeCameraModel = model;
    OSReport("start camera motion ... idx:%d time:%f\n", index, lbl_1_bss_10.cameraMaxTime);
}

BOOL fn_1_20D8(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_10.activeCameraModel);
}

void fn_1_2104(s32 playerNo, s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.playerModels[playerNo], lbl_1_bss_10.playerMotions[motionNo]);
}

void fn_1_215C(s32 playerNo, s32 motionNo)
{
    if (motionNo == 12 || lbl_1_bss_10.playerBaseMotions[playerNo][motionNo]
        != Hu3DMotionIDGet(lbl_1_bss_10.playerBaseModels[playerNo])) {
        Hu3DMotionSet(lbl_1_bss_10.playerBaseModels[playerNo],
            lbl_1_bss_10.playerBaseMotions[playerNo][motionNo]);
        Hu3DModelAttrSet(lbl_1_bss_10.playerBaseModels[playerNo], HU3D_MOTATTR_LOOP);
        OSReport("base motion time ... %f\n",
            Hu3DMotionMaxTimeGet(lbl_1_bss_10.playerBaseModels[playerNo]));
    }
}

void fn_1_2254(s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.centerModel, lbl_1_bss_10.centerMotions[motionNo]);
    OSReport("gear motion time ... %f\n", Hu3DMotionMaxTimeGet(lbl_1_bss_10.centerModel));
}

void fn_1_22BC(s32 playerNo, s32 motionNo, u32 attr)
{
    HU3D_MOTIONID motion;
    float start = 0.0f;

    motion = Hu3DMotionIDGet(lbl_1_bss_10.characterModels[playerNo]);
    if (motionNo == 10 || motion != lbl_1_bss_10.characterMotions[playerNo][motionNo]) {
        CharMotionShiftSet(lbl_1_bss_10.characterNos[playerNo],
            lbl_1_bss_10.characterMotions[playerNo][motionNo], start, 8.0f, attr);
    }
}

void fn_1_23B0(void)
{
    s32 player;
    u16 choice;
    M616CpuParam *param;

    for (player = 0; player < 4; player++) {
        if (GwPlayerConf[player].type == 1) {
            param = &lbl_1_data_2B0[GwPlayerConf[player].comDif];
            if (lbl_1_bss_10.roundNo == 0) {
                do {
                    choice = frand() % 5;
                } while (choice == 0);
            } else if (frand() % 100 < param->repeatPercent) {
                choice = lbl_1_bss_10.cpuChoices[player];
            } else {
                do {
                    choice = frand() % 5;
                } while (choice == lbl_1_bss_10.cpuChoices[player] || choice == 0);
            }
            lbl_1_bss_10.cpuChoices[player] = choice;
            lbl_1_bss_10.cpuInputFrames[player] = frand() % 270;
            OSReport("com %d : btn...%d time ...%d\n", player,
                lbl_1_bss_10.cpuChoices[player], lbl_1_bss_10.cpuInputFrames[player]);
        }
    }
}

s32 fn_1_2580(s32 playerNo)
{
    return lbl_1_bss_10.cpuChoices[playerNo];
}
