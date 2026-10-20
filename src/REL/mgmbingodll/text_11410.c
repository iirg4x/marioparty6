/* Runs Treetop Bingo from its game selection through the rounds and results. */
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/gamemes.h"
#include "game/mgdata.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/object.h"
#include "game/frand.h"
#include "game/audio.h"
#include "game/wipe.h"
#define MGM_BINGO_SOUND_CARD_REVEAL 1212
#define MGM_BINGO_SOUND_CARD_CYCLE 1213
#define MGM_BINGO_SOUND_GAME_CONFIRM 1214
#define MGM_BINGO_SOUND_MINIGAME_TRANSITION 1261
#define BINGO_UNUSED_MINIGAME 255
#define BINGO_MINIGAME_TABLE_END 65535
#define BINGO_MINIGAME_NUMBER_BASE 601
#define BINGO_EXCLUDED_MINIGAME_INDEX 21
void fn_1_2E28(void);
void fn_1_1A8C(void);
void fn_1_5154(void);
void fn_1_74BC(void);
void fn_1_1720(void);
void fn_1_10548(void);
void fn_1_3B6C(void);
void fn_1_4300(s16 evtno);
void fn_1_C578(void);
void fn_1_11A60(void);
void fn_1_90F4(void);
void fn_1_799C(s32 enabled);
void _SetFlag(u32 flag);

void fn_1_1720(void);
void fn_1_10548(void);
void fn_1_3B6C(void);
void fn_1_C578(void);
void fn_1_11A60(void);
void fn_1_90F4(void);

#include "REL/mgmbingodll/module_types.h"
extern HU3D_MODELID fn_1_0(s32 modeNo, s32 timeNo, Point3d *position);
extern void fn_1_11B5C(void);

#include <string.h>

extern void fn_1_2A4(s16 minigameCount, u8 *minigameOrder);
extern void fn_1_43C(void);
extern void fn_1_5E0(s16 modeNo, s16 timeNo);
extern void fn_1_6F4(void);

#include "dolphin/mtx.h"

extern void SLSaveModeExec(s16 sdModeF);
extern void fn_1_7F0(void);
extern void fn_1_824(void);
extern void fn_1_1970(void);
extern void fn_1_1088(void);

extern s16 fn_1_38E4(void);
extern s16 fn_1_26EC(void);
extern void fn_1_108C(s16 pathIndex, s16 durationFrames);
extern void fn_1_10970(u16 enable);
extern s32 lbl_1_data_AC8 = -1;

s32 lbl_1_data_ACC = -1;

s32 lbl_1_data_AD0 = -1;

s32 lbl_1_data_AD4 = -1;

int lbl_1_data_AD8 = -1;

void fn_1_11410(void);
extern void fn_1_11D2C(void);
extern void fn_1_1B08(void);
extern void fn_1_1ED8(void);
extern void fn_1_4FB4(s16 animationEnabled);
extern void fn_1_C848(void);
extern void fn_1_CFA8(void);
extern void fn_1_D538(void);
extern s16 fn_1_F018(void);
extern void fn_1_F704(void);
extern void fn_1_FCF4(void);
extern void fn_1_FD14(void);

// Initializes the session, selects unlocked minigames, and sets up bingo play.
void fn_1_11410(void)
{
    s16 selectedCount;
    s16 remainingModeCount;
    OMOVLHIS *history;
    u8 selectedModes[100];
    Point3d shadowTop;
    Point3d shadowTarget;
    Point3d shadowBottom;
    s16 index;
    s16 unlockedModeCount;
    s16 outputIndex;
    s16 coinBonus;

    history = omOvlHisGet(0);
    if (history->evtno == 0) {
        *(s16 *)&MgModeSubMode = 1;
        memset(MgModeScore, 0, 8);
        MgGameNo = 2;
        memset(MgModeWork, BINGO_UNUSED_MINIGAME, 256);
        MgModeWork[0] = 0;
        MgModePlayNum = 0;
        for (index = 0; index < 4; index++) {
            MgBattleOrder[index] = index;
        }
        memset(MgModeWorkInt, 0, 64);
        memset(MgModeWorkFloat, 0, 64);
        index = 0;
        selectedCount = 0;
        while (MgDataTbl[index].ovl != BINGO_MINIGAME_TABLE_END) {
            remainingModeCount = index + BINGO_MINIGAME_NUMBER_BASE;
            if (GWMgUnlockGet(remainingModeCount) != 0 && MgDataTbl[index].type == 0) {
                remainingModeCount = 1;
                if (MgDataTbl[index].flag & MG_FLAG_MIC) {
                    remainingModeCount = 0;
                }
                if (index == BINGO_EXCLUDED_MINIGAME_INDEX) {
                    remainingModeCount = 0;
                }
                if (remainingModeCount != 0) {
                    MgModeWork[selectedCount] = index;
                    selectedCount++;
                }
            }
            index++;
        }
        index = 0;
        unlockedModeCount = 0;
        while (index < 100) {
            selectedModes[index] = MgModeWork[index];
            if (MgModeWork[index] != BINGO_UNUSED_MINIGAME) {
                unlockedModeCount++;
            }
            index++;
        }
        outputIndex = 0;
        remainingModeCount = unlockedModeCount;
        while (remainingModeCount > 0) {
            index = frandmod(remainingModeCount);
            selectedCount = 0;
            while (selectedCount < unlockedModeCount) {
                if (selectedModes[selectedCount] != BINGO_UNUSED_MINIGAME) {
                    if (index == 0) {
                        MgModeWork[outputIndex] = selectedModes[selectedCount];
                        selectedModes[selectedCount] = BINGO_UNUSED_MINIGAME;
                        outputIndex++;
                        break;
                    } else {
                        index--;
                    }
                }
                selectedCount++;
            }
            remainingModeCount--;
        }
    } else {
        *(s16 *)&MgModeSubMode = history->evtno;
    }
        _SetFlag(FLAG_INST_MG_MODE);
    Hu3DBGColorSet(0, 0, 0);
    index = Hu3DGLightCreate(
        (0.0f), (1000.0f), (1000.0f),
        (0.0f), (-1.0f), (-0.5f),
        255, 255, 255);
    Hu3DGLightInfinitytSet(index);
    Hu3DShadowCreate((35.0f), (1000.0f), (5000.0f));
    shadowTop.x = (0.0f);
    shadowTop.y = (3500.0f);
    shadowTop.z = (-250.0f);
    shadowBottom.x = (0.0f);
    shadowBottom.y = (1.0f);
    shadowBottom.z = (-0.25f);
    shadowTarget.x = (0.0f);
    shadowTarget.y = (2100.0f);
    shadowTarget.z = (0.0f);
    Hu3DShadowPosSet(&shadowTop, &shadowBottom, &shadowTarget);
    Hu3DShadowTPLvlSet((0.6f));
    if (history->evtno != 0) {
        for (index = 0; index < 4; index++) {
            coinBonus = GwPlayer[index].mgCoinBonus;
            MgModeScore[index] = coinBonus;
            if (MgModeScore[index] < 10) {
                MgModeScore[index] = 0;
            }
            if (MgModeScore[index] >= 10) {
                MgModeScore[index] = 10;
            }
        }
        MgModePlayNum++;
        if (MgModeWork[MgModePlayNum] == BINGO_UNUSED_MINIGAME) {
            MgModePlayNum = 0;
        }
        MgModeWorkInt[15]++;
        // Preserve the five-round cycle when the completed-round counter wraps.
        if (MgModeWorkInt[15] >= 1005) {
            MgModeWorkInt[15] -= 1000;
        }
    }
    if (frandmod(2) != 0) {
        lbl_1_bss_AFC = 1;
    } else {
        lbl_1_bss_AFC = 0;
    }
    fn_1_1720();
    fn_1_10548();
    fn_1_3B6C();
    fn_1_4300(history->evtno);
    fn_1_C168();
    fn_1_EE74();
    if (history->evtno != 0) {
        fn_1_C578();
    }
    fn_1_11A60();
    fn_1_90F4();
    lbl_1_data_AC8 = HuAudSStreamPlay(MSM_STREAM_MGMUS_6);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 50);
    fn_1_799C(1);
    HuPrcVSleep();
    fn_1_799C(0);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

// The previous-minigame lookup is unused; the display always starts with minigame zero's day card.
void fn_1_11A60(void) {
    Point3d initialPosition;
    u8 previousMinigame;

    initialPosition.x = (0.0f);
    initialPosition.y = (0.0f);
    initialPosition.z = (0.0f);
    previousMinigame = 0;
    if (MgModePlayNum == 0) {
        previousMinigame = MgModeWork[MgModePlayNum];
    } else {
        previousMinigame = MgModeWork[MgModePlayNum - 1];
    }
    lbl_1_bss_AFA = fn_1_0(0, 0, &initialPosition);
    Hu3DModelScaleSet(lbl_1_bss_AFA, (0.0f), (0.0f), (0.0f));
    fn_1_11B5C();
}

// Shuffles up to ten minigames and loads at most five cards during display setup.
void fn_1_11B5C(void) {
    u8 minigameOrder[11];
    s16 minigameIndex;
    s16 otherIndex;
    s16 minigameCount;
    u8 swapMinigame;

    lbl_1_bss_AF8 = 1;
    memset(minigameOrder, BINGO_UNUSED_MINIGAME, sizeof(minigameOrder));
    minigameCount = 0;
    for (minigameIndex = 0;
         (minigameIndex < 10) && (MgModeWork[minigameIndex] != BINGO_UNUSED_MINIGAME);
         minigameIndex++) {
        minigameOrder[minigameIndex] = MgModeWork[minigameIndex];
        minigameCount++;
    }

    for (minigameIndex = 0;
         (minigameIndex < minigameCount) && (minigameOrder[minigameIndex] != BINGO_UNUSED_MINIGAME);
         minigameIndex++) {
        for (otherIndex = minigameIndex + 1;
             (otherIndex < minigameCount) && (minigameOrder[otherIndex] != BINGO_UNUSED_MINIGAME);
             otherIndex++) {
            if (frandmod(100) < 50) {
                swapMinigame = minigameOrder[minigameIndex];
                minigameOrder[minigameIndex] = minigameOrder[otherIndex];
                minigameOrder[otherIndex] = swapMinigame;
            }
        }
    }

    HuPrcVSleep();
    if (minigameCount >= 5) {
        fn_1_2A4(5, minigameOrder);
    } else {
        fn_1_2A4(minigameCount, minigameOrder);
    }
    HuPrcVSleep();
}

// Releases cached card animations when a round or the minigame ends.
void fn_1_11CE8(void) {
    if (lbl_1_bss_AF8 != 0) {
        fn_1_4F8();
    }
    lbl_1_bss_AF8 = 0;
}

// Reveals the bingo card, cycles through choices, and switches to the selected game.
void fn_1_11D2C(void) {
    Vec position;
    f32 scale;
    s16 frame;

    HuPrcVSleep();
    position.x = (0.0f);
    position.y = (18.0f);
    position.z = (-250.0f);
    PSVECAdd(&Hu3DCamera[0].pos, &position, &position);
    Hu3DModelPosSetV(lbl_1_bss_AFA, &position);

    PSVECSubtract(&Hu3DCamera[0].pos, &Hu3DCamera[0].target, &position);
    scale = (f32)(atan2(-position.y, position.z) /
                  (3.141592653589793) * (180.0));
    position.y = (f32)(atan2(position.x, position.z) /
                       (3.141592653589793) * (180.0));
    position.x = scale;
    position.z = (0.0f);
    Hu3DModelRotSetV(lbl_1_bss_AFA, &position);

    scale = (0.0f);
    Hu3DModelScaleSet(lbl_1_bss_AFA, scale, scale, scale);
    HuPrcVSleep();
    fn_1_43C();
    HuPrcVSleep();
    HuAudFXPlay(MGM_BINGO_SOUND_CARD_REVEAL);

    while (scale < (0.12f)) {
        Hu3DModelScaleSet(lbl_1_bss_AFA, scale, scale, scale);
        scale += (0.012f);
        if (scale >= (0.12f)) {
            scale = (0.12f);
        }
        HuPrcVSleep();
    }
    Hu3DModelScaleSet(lbl_1_bss_AFA, (0.12f),
                      (0.12f), (0.12f));
    HuPrcVSleep();
    lbl_1_data_AD0 = HuAudFXPlay(MGM_BINGO_SOUND_CARD_CYCLE);

    frame = 1;
    while (frame != 0) {
        fn_1_43C();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        frame++;
        if (frame > 25) {
            frame = 0;
        }
    }
    if (lbl_1_bss_AF8 != 0) {
        fn_1_4F8();
    }
    lbl_1_bss_AF8 = 0;
    if (lbl_1_data_AD0 >= 0) {
        HuAudFXStop(lbl_1_data_AD0);
    }
    lbl_1_data_AD0 = -1;
    HuAudFXPlay(MGM_BINGO_SOUND_GAME_CONFIRM);
    fn_1_5E0(MgModeWork[MgModePlayNum], lbl_1_bss_AFC);
    fn_1_6F4();
    HuPrcVSleep();
}

// Saves mode data and returns to the previous overlay after the player exits bingo.
void fn_1_12038(void) {
    fn_1_7F0();
    HuAudSStreamFadeOut(lbl_1_data_AC8, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_11CE8();
    fn_1_824();
    fn_1_1970();
    fn_1_1088();
    _ClearFlag(FLAG_INST_MG_MODE);
    SLSaveModeExec(0);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

// Saves mode data and returns to the previous overlay when the player declines another bingo
// session.
void fn_1_120E4(void) {
    fn_1_7F0();
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_11CE8();
    fn_1_824();
    fn_1_1970();
    fn_1_1088();
    _ClearFlag(FLAG_INST_MG_MODE);
    SLSaveModeExec(0);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

// Advances Treetop Bingo through its menus, minigame rounds, results, and exit.
void fn_1_1217C(void) {
    OMOVLHIS *history;
    s16 menuChoice;
    s16 shouldExit = 0;

    history = omOvlHisGet(0);
    if (MgModeSubMode == 0) {
        fn_1_11410();
        goto processSubMode;
    } else if (MgModeSubMode == 1) {
        fn_1_108C(1, 1);
        HuAudFXPlay(MGM_BINGO_SOUND_MINIGAME_TRANSITION);
        fn_1_4FB4(1);
        HuPrcSleep(40);
        fn_1_4FB4(0);
        HuPrcVSleep();
        fn_1_CFA8();
        fn_1_D538();
        fn_1_1B08();
        MgModeSubMode = 2;
        goto processSubMode;
    } else if (MgModeSubMode == 2) {
        MgModeSubMode = 3;
        goto processSubMode;
    } else if (MgModeSubMode == 3) {
        MgModeSubMode = 4;
        goto processSubMode;
    } else if (MgModeSubMode == 4) {
        menuChoice = fn_1_F018();
        MgModeSubMode = 5;
        if (menuChoice != 0) {
            MgModeSubMode = -1;
        }
        goto processSubMode;
    } else if (MgModeSubMode == 5) {
        fn_1_1ED8();
        MgModeSubMode = 6;
        goto processSubMode;
    } else if (MgModeSubMode == 6) {
        MgModeSubMode = 7;
        goto processSubMode;
    } else if (MgModeSubMode == 7) {
        fn_1_11D2C();
        MgModeSubMode = 8;
        goto processSubMode;
    } else if (MgModeSubMode == 8) {
        menuChoice = fn_1_26EC();
        if (menuChoice < 0) {
            fn_1_12038();
        }
        if (menuChoice > 0) {
            MgModeSubMode = 10;
        } else {
            MgModeSubMode = 9;
        }
        fn_1_C848();
        goto processSubMode;
    } else if (MgModeSubMode == 9) {
        s32 minigameNo;

        MgModeSubMode = 11;
        omOvlHisChg(0, history->ovl, (s32)MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamFadeOut(lbl_1_data_AC8, 1000);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 0;
        minigameNo = MgModeWork[MgModePlayNum] + BINGO_MINIGAME_NUMBER_BASE;
        GWMgUnlockSet(minigameNo);
        GwSystem.mgNo = minigameNo - BINGO_MINIGAME_NUMBER_BASE;
        if (lbl_1_bss_AFC != 0) {
            GwMgNightF = 1;
        } else {
            GwMgNightF = 0;
        }
        fn_1_11CE8();
        fn_1_824();
        omOvlCallEx(DLL_instdll, 1, 0, 0);
        for (;;) {
            HuPrcVSleep();
        }
    } else if (MgModeSubMode == 10) {
        s32 minigameNo;

        MgModeSubMode = 11;
        omOvlHisChg(0, history->ovl, (s32)MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamFadeOut(lbl_1_data_AC8, 1000);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 1;
        minigameNo = MgModeWork[MgModePlayNum] + BINGO_MINIGAME_NUMBER_BASE;
        GWMgUnlockSet(minigameNo);
        GwSystem.mgNo = minigameNo - BINGO_MINIGAME_NUMBER_BASE;
        if (lbl_1_bss_AFC != 0) {
            GwMgNightF = 1;
        } else {
            GwMgNightF = 0;
        }
        fn_1_11CE8();
        fn_1_824();
        omOvlCallEx(DLL_instdll, 1, 0, 0);
        for (;;) {
            HuPrcVSleep();
        }
    } else {
        shouldExit = 1;
    }

processSubMode:
    if (MgModeSubMode == 11) {
        MgModeSubMode = 12;
        return;
    } else if (MgModeSubMode == 12) {
        fn_1_F704();
        if ((MgModeWorkInt[0] >= MgGameNo) ||
            (MgModeWorkInt[1] >= MgGameNo) ||
            (MgModeWorkInt[2] >= MgGameNo) ||
            (MgModeWorkInt[3] >= MgGameNo)) {
            MgModeSubMode = 14;
        } else {
            MgModeSubMode = 13;
        }
    } else if (MgModeSubMode == 13) {
        fn_1_FCF4();
        MgModeSubMode = 6;
        return;
    } else if (MgModeSubMode == 14) {
        MgModeSubMode = 15;
        return;
    } else if (MgModeSubMode == 15) {
        fn_1_FD14();
        MgModeSubMode = 16;
        return;
    } else if (MgModeSubMode == 16) {
        MgModeSubMode = -1;
        menuChoice = fn_1_38E4();
        HuAudSStreamFadeOut(lbl_1_data_ACC, 1000);
        fn_1_10970(0);
        if (menuChoice != 0) {
            fn_1_7F0();
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
            while (WipeCheck() != 0) HuPrcVSleep();
            fn_1_11CE8();
            fn_1_824();
            _ClearFlag(FLAG_INST_MG_MODE);
            SLSaveModeExec(0);
            omOvlHisChg(0, history->ovl, 0, 0);
            omOvlReturnEx(0, 1);
            for (;;) HuPrcVSleep();
        }
        fn_1_120E4();
    } else if (shouldExit >= 1) {
        fn_1_12038();
    }
    return;
}

// Normalizes the input, or generates a random downward direction when its squared magnitude is at
// most 0.001.
s32 fn_1_12A88(Point3d *inputVector, Point3d *normalizedVector) {
    f32 verticalComponent;

    if (PSVECSquareMag(inputVector) <= 0.001) {
        normalizedVector->x = 0.01f * ((f32)frandmod(20) - 10.0f);
        normalizedVector->z = 0.01f * ((f32)frandmod(20) - 10.0f);
        // This draw always returns zero, so fallback directions point downward.
        if (frandmod(1) != 0) {
            verticalComponent = 0.01f;
        } else {
            verticalComponent = -0.01f;
        }
        normalizedVector->y = verticalComponent;
        PSVECNormalize(normalizedVector, normalizedVector);
        return 0;
    }
    PSVECNormalize(inputVector, normalizedVector);
    return 1;
}

// Rotates a position by the supplied X, Y, and Z angles before it is displayed.
void fn_1_12BE0(f32 angleX, f32 angleY, f32 angleZ,
                const Vec *source, Vec *destination)
{
    Mtx x_rotation;
    Mtx y_rotation;

    PSMTXRotRad(x_rotation, 'x', 0.017453292f * angleX);
    PSMTXRotRad(y_rotation, 'y', 0.017453292f * angleY);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXRotRad(y_rotation, 'z', 0.017453292f * angleZ);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXMultVecSR(x_rotation, source, destination);
}

OMOBJMAN *lbl_1_bss_B00;
s16 lbl_1_bss_AFC;
s16 lbl_1_bss_AFA;
s16 lbl_1_bss_AF8;
