/* Sets up the tournament menu and runs its selection and results sequence. */
#include <math.h>
#include "game/hu3d.h"
#include "game/gamemes.h"
#include "game/mgdata.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/frand.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/process.h"
#include "game/flag.h"
#include "game/saveload.h"
#include "string.h"
#include "msm_stream.h"
#include "REL/mgmtournamentdll/module_types.h"
#include "REL/mgmtournamentdll/mgmbingodll_module_types.h"

#define TOURNAMENT_GAME_COUNT 101
#define TOURNAMENT_GAME_NONE 255
#define TOURNAMENT_GAME_TABLE_END 65535
#define TOURNAMENT_UNLOCK_GAME_BASE 601

s32 lbl_1_data_848 = -1; /* Selection music stream handle, or -1 before playback. */
s32 lbl_1_data_84C = -1; /* Music stream started after the champion presentation. */
s32 lbl_1_data_850 = -1; /* Sound handle for the minigame selection prop. */
s32 lbl_1_data_854 = -1; /* Champion sound handle stopped at the final prompt. */
s32 lbl_1_data_858 = -1; /* Champion presentation jingle handle. */
extern s16 MgModeSubMode;
extern u8 MgModePlayNum;
extern u8 MgModeWork[256];
extern s16 lbl_1_bss_128;
void fn_1_D69C(void);

/* Overlay entry initializes the tournament roster and scene, restoring the saved mode on return. */
void fn_1_D69C(void)
{
    s16 remainingGameCount;
    OMOVLHIS *history;
    u8 remainingGames[TOURNAMENT_GAME_COUNT];
    Point3d shadowPosition;
    Point3d shadowTarget;
    Point3d shadowUp;
    s16 index;
    s16 unlockedGameCount;
    s16 candidateIndex;
    s16 gameOrderIndex;
    s16 coinBonus;

    history = omOvlHisGet(0);
    if (history->evtno == 0) {
        MgModeSubMode = 1;
        MgModeScore[0] = MgModeScore[1] = MgModeScore[2] = MgModeScore[3] = -1;
        MgGameNo = 0;
        memset(MgModeWork, TOURNAMENT_GAME_NONE, 256);
        MgModeWork[0] = 0;
        MgModePlayNum = 0;
        for (index = 0; index < 4; index++) {
            MgBattleOrder[index] = index;
        }
        memset(MgModeWorkInt, 0, 64);
        memset(MgModeWorkFloat, 0, 64);
        index = 0;
        candidateIndex = 0;
        /* Only unlocked Duel minigames enter the tournament's game list. */
        while (MgDataTbl[index].ovl != TOURNAMENT_GAME_TABLE_END) {
            remainingGameCount = index + TOURNAMENT_UNLOCK_GAME_BASE;
            if (fn_8003CC40(remainingGameCount) != 0 && MgDataTbl[index].type == MG_TYPE_KETTOU) {
                MgModeWork[candidateIndex] = index;
                candidateIndex++;
            }
            index++;
        }
        index = 0;
        unlockedGameCount = 0;
        while (index < TOURNAMENT_GAME_COUNT) {
            remainingGames[index] = MgModeWork[index];
            if (MgModeWork[index] != TOURNAMENT_GAME_NONE) {
                unlockedGameCount++;
            }
            index++;
        }
        /* Draw each game once, marking it unavailable after adding it to the shuffled list. */
        gameOrderIndex = 0;
        remainingGameCount = unlockedGameCount;
        while (remainingGameCount > 0) {
            index = frandmod(remainingGameCount);
            candidateIndex = 0;
            while (candidateIndex < unlockedGameCount) {
                if (remainingGames[candidateIndex] != TOURNAMENT_GAME_NONE) {
                    if (index == 0) {
                        MgModeWork[gameOrderIndex] = remainingGames[candidateIndex];
                        remainingGames[candidateIndex] = TOURNAMENT_GAME_NONE;
                        gameOrderIndex++;
                        break;
                    } else {
                        index--;
                    }
                }
                candidateIndex++;
            }
            remainingGameCount--;
        }
    } else {
        MgModeSubMode = history->evtno;
    }
    fn_8006A1C4(196609);
    index = Hu3DGLightCreate(
        0.0f, 1000.0f, 1000.0f,
        0.0f, -1.0f, -0.5f,
        255, 255, 255);
    Hu3DGLightInfinitytSet(index);
    Hu3DShadowCreate(30.0f, 20.0f, 12000.0f);
    shadowPosition.x = 0.0f;
    shadowPosition.y = 3450.0f;
    shadowPosition.z = 2850.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = -0.25f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = -1550.0f;
    shadowTarget.z = 1600.0f;
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(0.6f);
    fn_1_2E28();
    fn_1_1A8C();
    fn_1_5154();
    fn_1_74BC();
    if (history->evtno != 0) {
        /* Preserve the returned minigame's coin results before advancing its list entry. */
        for (index = 0; index < 4; index++) {
            coinBonus = lbl_80265750[index].mgCoinBonus;
            MgModeWorkInt[index + 8] = coinBonus;
        }
        MgModePlayNum++;
        if (MgModeWork[MgModePlayNum] == TOURNAMENT_GAME_NONE) {
            MgModePlayNum = 0;
        }
        fn_1_3E04();
        fn_1_7924();
        tournamentCameraRestore();
        fn_1_3F8C();
        fn_1_5590();
    }
    fn_1_B57C();
    lbl_1_data_848 = HuAudSStreamPlay(MSM_STREAM_MGMUS_5);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Overlay exit closes previews and windows, attempts to save changed common game data, and
* returns to its caller. */
void fn_1_DBB0(void) {
    fn_1_7F0();
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_B80C();
    fn_1_824();
    fn_1_4A1C();
    fn_1_1BA0();
    fn_1_1134();
    fn_8006A268(196609);
    SLSaveModeExec(0);
    fn_8003A5E4(1, 1);
    HuPrcEnd();
while (1) {
        HuPrcVSleep();
    }
}

/* Started by the overlay entry, this process handles the current tournament scene step,
 * including introductions, round choices, minigame handoff, results and replay. */
void fn_1_DC28(void)
{
    OMOVLHIS *history;
    s16 result;

    history = omOvlHisGet(0);
    switch (MgModeSubMode) {
    case 0:
        fn_1_D69C();
        break;
    case 1:
        tournamentCameraOpeningPlay();
        fn_1_1D74();
        fn_1_A6B4();
        MgModeSubMode = 2;
        break;
    case 2:
        tournamentCameraViewSet(1, 90);
        fn_1_5740(1, 90);
        HuPrcSleep(90);
        fn_1_5740(2, 90);
        HuPrcSleep(90);
        fn_1_AB48();
        fn_1_1E80();
        fn_1_3E04();
        fn_1_AD50();
        HuPrcVSleep();
        MgModeSubMode = 3;
        break;
    case 3:
        fn_1_B014();
        tournamentCameraRoundViewSet(60);
        fn_1_57E0(90);
        HuPrcSleep(90);
        fn_1_5A44(60);
        fn_1_1F90();
        /* fall through */
    case 4:
        fn_1_34F0();
        fn_1_B2E0();
        HuPrcSleep(76);
        MgModeSubMode = 5;
        break;
    case 5:
        fn_1_B850();
        MgModeSubMode = 6;
        HuPrcSleep(25);
        break;
    case 6:
        fn_1_64A8();
        result = fn_1_2128();
        if (result < 0) {
            HuAudSStreamFadeOut(lbl_1_data_848, 1000);
            fn_1_7F0();
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_B80C();
            fn_1_824();
            fn_1_4A1C();
            fn_1_1BA0();
            fn_1_1134();
            fn_8006A268(196609);
            SLSaveModeExec(0);
            fn_8003A5E4(1, 1);
            HuPrcEnd();
            while (1) {
                HuPrcVSleep();
            }
        }
        if (result == 2) {
            fn_1_6D78();
            MgModeSubMode = 9;
            fn_1_818C();
            omOvlHisChg(0, history->ovl, MgModeSubMode, history->stat);
            fn_1_7F0();
            HuAudSStreamFadeOut(lbl_1_data_848, 1000);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_B80C();
            fn_1_824();
            fn_8003A5E4(0, 1);
            while (1) {
                HuPrcVSleep();
            }
        }
        if (result > 0) {
            MgModeSubMode = 8;
        } else {
            MgModeSubMode = 7;
        }
        break;
    case 7: {
        s32 gameId;

        MgModeSubMode = 9;
        fn_1_818C();
        omOvlHisChg(0, history->ovl, MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamFadeOut(lbl_1_data_848, 1000);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        /* This round choice enters the minigame without its instruction screen. */
        lbl_80265B70.mgInstDispF = 0;
        gameId = MgModeWork[MgModePlayNum] + TOURNAMENT_UNLOCK_GAME_BASE;
        fn_8003CCAC(gameId);
        lbl_80265B70.mgNo = gameId - TOURNAMENT_UNLOCK_GAME_BASE;
        if (lbl_1_bss_128 != 0) {
            lbl_802C0278 = 1;
        } else {
            lbl_802C0278 = 0;
        }
        fn_1_B80C();
        fn_1_824();
        omOvlCallEx(4, 1, 0, 0);
        while (1) {
            HuPrcVSleep();
        }
    }
    case 8: {
        s32 gameId;

        MgModeSubMode = 9;
        fn_1_818C();
        omOvlHisChg(0, history->ovl, MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamFadeOut(lbl_1_data_848, 1000);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        /* This round choice enters through the minigame's instruction screen. */
        lbl_80265B70.mgInstDispF = 1;
        gameId = MgModeWork[MgModePlayNum] + TOURNAMENT_UNLOCK_GAME_BASE;
        fn_8003CCAC(gameId);
        lbl_80265B70.mgNo = gameId - TOURNAMENT_UNLOCK_GAME_BASE;
        if (lbl_1_bss_128 != 0) {
            lbl_802C0278 = 1;
        } else {
            lbl_802C0278 = 0;
        }
        fn_1_B80C();
        fn_1_824();
        omOvlCallEx(4, 1, 0, 0);
        while (1) {
            HuPrcVSleep();
        }
    }
    case 9:
        MgModeSubMode = 10;
        break;
    case 10:
        result = fn_1_BAD4();
        if (result > 0) {
            MgModeSubMode = 12;
        } else if (result < 0) {
            MgModeSubMode = 4;
        } else {
            MgModeSubMode = 11;
        }
        break;
    case 11:
        MgModeSubMode = 3;
        break;
    case 12:
        tournamentCameraViewSet(11, 60);
        fn_1_5740(13, 90);
        HuPrcSleep(90);
        fn_1_5740(14, 50);
        HuPrcSleep(50);
        fn_1_2948();
        MgModeSubMode = 13;
        HuPrcVSleep();
        break;
    case 13:
        fn_1_CF6C();
        MgModeSubMode = 14;
        HuPrcVSleep();
        break;
    case 14:
        HuPrcSleep(25);
        MgModeSubMode = -1;
        result = fn_1_2BA0();
        if (lbl_1_data_854 >= 0) {
            HuAudFXStop(lbl_1_data_854);
            lbl_1_data_854 = -1;
        }
        HuAudSStreamFadeOut(lbl_1_data_84C, 1000);
        if (result != 0) {
            fn_1_7F0();
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_B80C();
            fn_1_824();
            fn_1_4A1C();
            fn_8006A268(196609);
            SLSaveModeExec(0);
            omOvlHisChg(0, history->ovl, 0, 0);
            fn_8003A5E4(0, 1);
            while (1) {
                HuPrcVSleep();
            }
        }
        break;
    default:
        fn_1_7F0();
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        fn_1_B80C();
        fn_1_824();
        fn_1_4A1C();
        fn_1_1BA0();
        fn_1_1134();
        fn_8006A268(196609);
        SLSaveModeExec(0);
        fn_8003A5E4(1, 1);
        HuPrcEnd();
        while (1) {
            HuPrcVSleep();
        }
    }
}
