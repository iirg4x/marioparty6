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


typedef struct M616CpuParam {
    u32 unk_00;
    u32 repeatPercent;
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];

HuVecF lbl_1_bss_0;
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_140, fn_1_160, fn_1_4BC, fn_1_4C0, fn_1_F14,
    fn_1_F78, fn_1_1284, fn_1_135C, fn_1_1360
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
                u16 buttons;

                buttons = HuPadBtn[lbl_1_bss_10.padNos[player]];
                buttons &= PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R;
                switch (buttons & ~lbl_1_bss_10.previousButtons[player]) {
                case 0:
                    break;
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
                fn_1_22BC(player, 0, HU3D_MOTATTR_LOOP);
                {
                    s32 motions[5] = { 7, 0, 1, 2, 3 };

                    if (lbl_1_bss_10.choices[player] != 0) {
                        fn_1_215C(player, motions[lbl_1_bss_10.choices[player]]);
                    }
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
