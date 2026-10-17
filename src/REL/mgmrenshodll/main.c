/* Runs the minigame streak mode and selects each round's minigame. */
#include "REL/mgmrenshodll/module_types.h"
#include "game/mgdata.h"
#include "game/flag.h"
#include "game/saveload.h"
#include "datadir_enum.h"
#include "msm_stream.h"
#include "msm_se.h"

#define MSM_STREAM_RENSHO 56
#define GW_MGNO_BASE 601
#define MSM_SE_RENSHO_PREVIEW_ENTER 1212
#define MSM_SE_RENSHO_PREVIEW_HOLD 1213
#define MSM_SE_RENSHO_PREVIEW_EXIT 1214

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
OMOBJMAN *lbl_1_bss_158;
s16 lbl_1_bss_154;
HU3D_MODELID lbl_1_bss_152;
s16 lbl_1_bss_150;
s32 lbl_1_data_500 = -1;
s32 lbl_1_data_504 = -1;
s32 lbl_1_data_508 = -1;
s32 lbl_1_data_50C = -1;
s32 lbl_1_data_510 = -1;
s32 lbl_1_data_514 = -1;
s32 lbl_1_data_518 = -1;
s32 lbl_1_data_51C = -1;

extern s16 MgModeSubMode;
extern void fn_1_E9C(void);
extern void fn_1_EEBC(void);
extern HU3D_MODELID fn_1_0(s32 minigameIndex, s32 stillIndex, Vec *position);
void fn_1_E908(void);
extern void fn_1_2A4(s16 count, u8 *minigameOrder);
extern void fn_1_9B9C(void);
extern void fn_1_43C(void);
extern void fn_1_4F8(void);
extern void fn_1_5E0(s16 minigameIndex, s16 stillIndex);
extern void fn_1_6F4(void);
extern int MgModeWorkInt[16];
extern u8 MgModePlayNum;
void fn_1_106C(s16 selectedCamera);
void fn_1_94A8(void);
void fn_1_9D2C(void);
void fn_1_14B8(void);
void fn_1_2F9C(void);
void fn_1_78FC(void);
void fn_1_8600(void);
void fn_1_9978(void);
void fn_1_E80C(void);
void fn_1_9A70(void);
extern void fn_1_824(void);
extern void fn_1_1028(void);
extern void fn_1_15CC(void);
extern void fn_1_7F0(void);
extern void fn_1_DDB8(void);
extern void fn_1_16D0(void);
extern void fn_1_A0CC(void);
extern void fn_1_17DC(void);
extern void fn_1_A348(void);
extern void fn_1_1938(void);
extern void fn_1_EAD8(void);
extern s16 fn_1_1A78(void);
extern void fn_1_9E20(void);
extern void fn_1_2038(void);
extern void fn_1_D5A0(void);
extern void fn_1_221C(void);
extern void fn_1_D964(void);
extern s16 fn_1_2A00(void);
extern void fn_1_F54C(void);
void fn_1_EEBC(void);

/* Called by _prolog during overlay startup; creates the manager and round process. */
void fn_1_DC60(void)
{
    lbl_1_bss_158 = omInitObjMan(100, 8192);
    MgModeSubMode = 0;
    fn_1_E9C();
    HuPrcChildCreate(fn_1_EEBC, 100U, 12288U, 0, lbl_1_bss_158);
}

/* Runs overlay constructors, then starts Endurance Alley's round manager. */
s32 _prolog(void)
{
    const VoidFunc *constructor;

    constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_DC60();
    return 0;
}

/* Runs overlay destructors when Endurance Alley is unloaded. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Builds the round list and scene, then starts the music and opening wipe. */
void fn_1_DDB8(void)
{
    OMOVLHIS *overlayHistory;
    u8 remainingModes[101];
    Vec shadowTop;
    Vec shadowCenter;
    Vec shadowUp;
    s16 i;
    s16 work;
    s16 j;
    s16 k;
    s16 count;
    s16 coin;
    overlayHistory = omOvlHisGet(0);
    if (overlayHistory->evtno == 0) {
        MgModeSubMode = 1;
        memset(MgModeScore, 0, 8U);
        MgGameNo = 100;
        memset(MgModeWork, 255, 256U);
        MgModeWork[0] = 0;
        MgModePlayNum = 0;
        i = 0;
        while (i < 4) {
            MgBattleOrder[i] = i;
            i++;
        }
        memset(MgModeWorkInt, 0, 64U);
        memset(MgModeWorkFloat, 0, 64U);

        i = 0;
        j = 0;
        while (MgDataTbl[i].ovl != 65535) {
            work = i + GW_MGNO_BASE;
            if ((GWMgUnlockGet(work) != 0) &&
                ((MgDataTbl[i].type == 0) || (MgDataTbl[i].type == 1))) {
                work = 1;
                if ((MgDataTbl[i].flag & MG_FLAG_MIC) != 0) {
                    work = 0;
                }
                if ((i == 21) || (i == 22)) {
                    work = 0;
                }
                if ((i == 15) || (i == 19) || (i == 54) || (i == 61)) {
                    work = 0;
                }
                if (work != 0) {
                    MgModeWork[j] = i;
                    j++;
                }
            }
            i++;
        }
        count = j / 2;

        k = 0;
        i = 0;
        while (MgDataTbl[i].ovl != 65535) {
            work = i + GW_MGNO_BASE;
            if ((GWMgUnlockGet(work) != 0) && (MgDataTbl[i].type == 6)) {
                work = 1;
                if ((MgDataTbl[i].flag & MG_FLAG_MIC) != 0) {
                    work = 0;
                }
                if ((i == 21) || (i == 22)) {
                    work = 0;
                }
                if ((i == 15) || (i == 19) || (i == 54) || (i == 61)) {
                    work = 0;
                }
                if (work != 0) {
                    k++;
                }
            }
            i++;
        }
        k = k - count;
        if (k > 0) {
            k = frandmod(k + 1);
        }
        i = 0;
        while ((MgDataTbl[i].ovl != 65535) && (count > 0)) {
            work = i + GW_MGNO_BASE;
            if ((GWMgUnlockGet(work) != 0) && (MgDataTbl[i].type == 6)) {
                work = 1;
                if ((MgDataTbl[i].flag & MG_FLAG_MIC) != 0) {
                    work = 0;
                }
                if ((i == 21) || (i == 22)) {
                    work = 0;
                }
                if ((i == 15) || (i == 19) || (i == 54) || (i == 61)) {
                    work = 0;
                }
                if (work != 0) {
                    if (k <= 0) {
                        MgModeWork[j] = i;
                        j++;
                        count--;
                    } else {
                        k--;
                    }
                }
            }
            i++;
        }

        i = 0;
        count = 0;
        while (i < 101) {
            remainingModes[i] = MgModeWork[i];
            if (MgModeWork[i] != 255) {
                count++;
            }
            i++;
        }
        k = 0;
        work = count;
        while (work > 0) {
            i = frandmod(work);
            j = 0;
            while (j < count) {
                if (remainingModes[j] != 255) {
                    if (i == 0) {
                        MgModeWork[k] = remainingModes[j];
                        remainingModes[j] = 255;
                        k++;
                        break;
                    } else {
                        i--;
                    }
                }
                j++;
            }
            work--;
        }
        i = 0;
        while (MgModeWork[i] != 255) {
            i++;
        }
        j = i;
        i = 0;
        while ((i <= MgGameNo) && (j <= MgGameNo)) {
            MgModeWork[j] = MgModeWork[i];
            i++;
            j++;
        }
        MgModeWork[j] = 255;
    } else {
        MgModeSubMode = overlayHistory->evtno;
    }

    _SetFlag(FLAG_INST_MG_MODE);
    Hu3DBGColorSet(0U, 0U, 0U);
    {
        i = Hu3DGLightCreate(
            1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            255U, 255U, 255U);
        Hu3DGLightInfinitytSet(i);
        i = Hu3DGLightCreate(
            -1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            255U, 255U, 255U);
        Hu3DGLightInfinitytSet(i);
    }
    Hu3DShadowCreate(45.0f, 1000.0f, 5000.0f);
    shadowTop.x = 0.0f;
    shadowTop.y = 3500.0f;
    shadowTop.z = -1250.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = -0.25f;
    shadowCenter.x = 0.0f;
    shadowCenter.y = 0.0f;
    shadowCenter.z = -1000.0f;
    Hu3DShadowPosSet(&shadowTop, &shadowUp, &shadowCenter);
    Hu3DShadowTPLvlSet(0.6f);
    if (overlayHistory->evtno != 0) {
        i = 0;
        while (i < 4) {
            coin = GwPlayer[i].mgCoinBonus;
            MgModeScore[i] = coin;
            i++;
        }
        MgModePlayNum++;
        if ((s32)MgModePlayNum >= MgGameNo) {
            MgModePlayNum = (u8)MgGameNo;
        }
    }
    fn_1_94A8();
    fn_1_9D2C();
    fn_1_14B8();
    fn_1_2F9C();
    fn_1_78FC();
    fn_1_8600();
    if (overlayHistory->evtno != 0) {
        fn_1_9978();
        fn_1_106C(2);
    }
    fn_1_E80C();
    HuDataDirClose(DATANUM(DATA_mgmrensho, 0));
    fn_1_9A70();
    if (MgModePlayNum == 0) {
        MgModeWorkInt[0] = 1;
    }
    if (MgModeWorkInt[0] != 0) {
        if (MgModePlayNum <= 31) {
            lbl_1_data_500 = HuAudSStreamPlay(MSM_STREAM_RENSHO);
        } else if (MgModePlayNum <= 41) {
            lbl_1_data_504 = HuAudSStreamPlay(MSM_STREAM_MGMUS_DONKEY);
        } else if (MgModePlayNum <= 61) {
            lbl_1_data_508 = HuAudSStreamPlay(MSM_STREAM_MGMUS_7);
        } else if (MgModePlayNum <= 81) {
            lbl_1_data_50C = HuAudSStreamPlay(MSM_STREAM_MGMUS_8);
        } else {
            lbl_1_data_510 = HuAudSStreamPlay(MSM_STREAM_MGMUS_9);
        }
    } else {
        lbl_1_data_514 = HuAudSStreamPlay(MSM_STREAM_RESULT);
    }
    if (MgModePlayNum == 0) {
        MgModeWorkInt[0] = 0;
    }
    WipeCreate(1, 0, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Called during mode setup to create the preview model and prepare the minigame order. */
void fn_1_E80C(void) {
    Vec initialPosition;
    u8 selectedMinigame;

    initialPosition.x = 0.0f;
    initialPosition.y = 0.0f;
    initialPosition.z = 0.0f;
    selectedMinigame = 0;
    if (MgModePlayNum == 0) {
        selectedMinigame = MgModeWork[MgModePlayNum];
    } else {
        selectedMinigame = MgModeWork[MgModePlayNum - 1];
    }
    /* The calculated minigame index is unused; setup always creates the preview with
     * minigame 0 and still 0. */
    lbl_1_bss_152 = fn_1_0(0, 0, &initialPosition);
    Hu3DModelScaleSet(lbl_1_bss_152, 0.0f, 0.0f, 0.0f);
    fn_1_E908();
}

/* Builds a randomized order of unlocked minigames for the instruction preview. */
void fn_1_E908(void) {
    u8 minigameOrder[11];
    s16 minigameOrderIndex;
    s16 otherIndex;
    s16 minigameCount;
    u8 minigameIndex;

    lbl_1_bss_150 = 1;
    memset(minigameOrder, 255, sizeof(minigameOrder));
    minigameCount = 0;
    for (minigameOrderIndex = 0;
         (minigameOrderIndex < 10) && (MgModeWork[minigameOrderIndex] != 255);
         minigameOrderIndex++) {
        minigameOrder[minigameOrderIndex] = MgModeWork[minigameOrderIndex];
        minigameCount++;
    }

    for (minigameOrderIndex = 0;
         (minigameOrderIndex < minigameCount) && (minigameOrder[minigameOrderIndex] != 255);
         minigameOrderIndex++) {
        for (otherIndex = minigameOrderIndex + 1;
             (otherIndex < minigameCount) && (minigameOrder[otherIndex] != 255);
             otherIndex++) {
            if (frandmod(100) < 50) {
                minigameIndex = minigameOrder[minigameOrderIndex];
                minigameOrder[minigameOrderIndex] = minigameOrder[otherIndex];
                minigameOrder[otherIndex] = minigameIndex;
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

/* Releases the instruction still list when a preview sequence ends. */
void fn_1_EA94(void) {
    if (lbl_1_bss_150 != 0) {
        fn_1_4F8();
    }
    lbl_1_bss_150 = 0;
}

/* Plays the selected round's preview sequence from the mode process. */
void fn_1_EAD8(void) {
    Vec position;
    f32 pitchOrScale;
    s16 frame;
    s16 stop;

    fn_1_9B9C();
    HuPrcVSleep();
    position.x = -76.0f;
    position.y = -63.0f;
    position.z = -200.0f;
    PSVECAdd(&Hu3DCamera[0].pos, &position, &position);
    Hu3DModelPosSetV(lbl_1_bss_152, &position);

    PSVECSubtract(&Hu3DCamera[0].pos, &Hu3DCamera[0].target, &position);
    pitchOrScale = (f32)(atan2(-position.y, position.z) / 3.141592653589793 * 180.0);
    position.y = (f32)(atan2(position.x, position.z) / 3.141592653589793 * 180.0);
    position.x = pitchOrScale;
    position.z = 0.0f;
    Hu3DModelRotSetV(lbl_1_bss_152, &position);

    pitchOrScale = 0.0f;
    Hu3DModelScaleSet(lbl_1_bss_152, pitchOrScale, pitchOrScale, pitchOrScale);
    HuPrcVSleep();
    fn_1_43C();
    HuPrcVSleep();
    HuAudFXPlay(MSM_SE_RENSHO_PREVIEW_ENTER);

    while (pitchOrScale < 0.18f) {
        Hu3DModelScaleSet(lbl_1_bss_152, pitchOrScale, pitchOrScale, pitchOrScale);
        pitchOrScale += 0.018000001f;
        if (pitchOrScale >= 0.18f) {
            pitchOrScale = 0.18f;
        }
        HuPrcVSleep();
    }
    Hu3DModelScaleSet(lbl_1_bss_152, 0.18f,
                      0.18f, 0.18f);
    HuPrcVSleep();
    lbl_1_data_51C = HuAudFXPlay(MSM_SE_RENSHO_PREVIEW_HOLD);

    frame = 1;
    /* stop is read without initialization; the 25-step limit applies only if this loop is
     * entered. */
    while (stop != 0) {
        fn_1_43C();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        frame++;
        if (frame > 25) {
            stop = 0;
        }
    }
    fn_1_EA94();
    if (lbl_1_data_51C >= 0) {
        HuAudFXStop(lbl_1_data_51C);
    }
    lbl_1_data_51C = -1;
    HuAudFXPlay(MSM_SE_RENSHO_PREVIEW_EXIT);
    if (frandmod(2) != 0) {
        lbl_1_bss_154 = 1;
    } else {
        lbl_1_bss_154 = 0;
    }
    fn_1_5E0(MgModeWork[MgModePlayNum], lbl_1_bss_154);
    fn_1_6F4();
    HuPrcVSleep();
}

/* Saves and returns from the mode after the final round or a canceled run. */
void fn_1_EE24(void)
{
    fn_1_7F0();
    WipeCreate(2, 0, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_EA94();
    fn_1_824();
    fn_1_15CC();
    fn_1_1028();
    _ClearFlag(FLAG_INST_MG_MODE);
    SLSaveModeExec(0);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

/* Advances the Endurance Alley state machine as the child process created during mode setup. */
void fn_1_EEBC(void)
{
    OMOVLHIS *history;
    s16 result;

    history = omOvlHisGet(0);
    switch (*(s16 *)&MgModeSubMode) {
    case 0:
        fn_1_DDB8();
        break;
    case 1:
        fn_1_106C(0);
        fn_1_16D0();
        *(s16 *)&MgModeSubMode = 2;
        break;
    case 2:
        fn_1_106C(1);
        fn_1_A0CC();
        HuPrcSleep(10);
        fn_1_106C(2);
        fn_1_17DC();
        fn_1_A348();
        HuPrcSleep(10);
        *(s16 *)&MgModeSubMode = 3;
        break;
    case 3:
        fn_1_1938();
        *(s16 *)&MgModeSubMode = 4;
        break;
    case 4:
        fn_1_EAD8();
        *(s16 *)&MgModeSubMode = 5;
        HuPrcSleep(25);
        break;
    case 5:
        result = fn_1_1A78();
        fn_1_9E20();
        if (result < 0) {
            HuAudSStreamAllFadeOut(1000);
            fn_1_EE24();
        }
        if (result > 0) {
            *(s16 *)&MgModeSubMode = 7;
        } else {
            *(s16 *)&MgModeSubMode = 6;
        }
        break;
    case 6: {
        u32 unlockNo;

        *(s16 *)&MgModeSubMode = 8;
        omOvlHisChg(0, history->ovl, *(s16 *)&MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamAllFadeOut(1000);
        WipeCreate(2, 0, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 0;
        unlockNo = GW_MGNO_BASE + MgModeWork[MgModePlayNum];
        GWMgUnlockSet(unlockNo);
        GwSystem.mgNo = unlockNo - GW_MGNO_BASE;
        if (lbl_1_bss_154 != 0) {
            GwMgNightF = 1;
        } else {
            GwMgNightF = 0;
        }
        fn_1_EA94();
        fn_1_824();
        omOvlCallEx(4, 1, 0, 0);
        while (1) {
            HuPrcVSleep();
        }
    }
    case 7: {
        u32 unlockNo;

        *(s16 *)&MgModeSubMode = 8;
        omOvlHisChg(0, history->ovl, *(s16 *)&MgModeSubMode, history->stat);
        fn_1_7F0();
        HuAudSStreamAllFadeOut(1000);
        WipeCreate(2, 0, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 1;
        unlockNo = GW_MGNO_BASE + MgModeWork[MgModePlayNum];
        GWMgUnlockSet(unlockNo);
        GwSystem.mgNo = unlockNo - GW_MGNO_BASE;
        if (lbl_1_bss_154 != 0) {
            GwMgNightF = 1;
        } else {
            GwMgNightF = 0;
        }
        fn_1_EA94();
        fn_1_824();
        omOvlCallEx(4, 1, 0, 0);
        while (1) {
            HuPrcVSleep();
        }
    }
    case 8:
        fn_1_D2BC();
        fn_1_1E9C();
        fn_1_D3A8();
        *(s16 *)&MgModeSubMode = 9;
        if (MgModeWorkInt[0] == 0) {
            *(s16 *)&MgModeSubMode = 11;
        }
        break;
    case 9:
        fn_1_D5A0();
        fn_1_2038();
        if (MgModePlayNum >= MgGameNo) {
            *(s16 *)&MgModeSubMode = 11;
        } else {
            *(s16 *)&MgModeSubMode = 10;
        }
        break;
    case 10:
        *(s16 *)&MgModeSubMode = 3;
        break;
    case 11:
        fn_1_221C();
        *(s16 *)&MgModeSubMode = 12;
        HuPrcVSleep();
        break;
    case 12:
        fn_1_D964();
        *(s16 *)&MgModeSubMode = 13;
        HuPrcVSleep();
        break;
    case 13:
        fn_1_9B9C();
        *(s16 *)&MgModeSubMode = -1;
        result = fn_1_2A00();
        HuAudSStreamAllFadeOut(1000);
        fn_1_F54C();
        if (result != 0) {
            fn_1_7F0();
            WipeCreate(2, 0, 50);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_EA94();
            fn_1_824();
            _ClearFlag(FLAG_INST_MG_MODE);
            SLSaveModeExec(0);
            omOvlHisChg(0, history->ovl, 0, 0);
            omOvlReturnEx(0, 1);
            while (1) {
                HuPrcVSleep();
            }
        }
        break;
    default:
        fn_1_EE24();
    }
}

/* Called at the end of the final round to save a new longest minigame order. */
void fn_1_F54C(void) {
    u8 completedGames;
    s16 gameIndex;

    completedGames = MgModePlayNum;
    if (MgModeWorkInt[0] == 0) {
        completedGames -= 1;
    }
    if (GwCommon.renshoMgRecordNum < completedGames) {
        GwCommon.renshoMgRecordNum = completedGames;
        gameIndex = 0;
        while (gameIndex < completedGames) {
            GwCommon.renshoMgRecord[gameIndex] = MgModeWork[gameIndex];
            gameIndex += 1;
        }
    }
}

/* Converts a direction to unit length, using a fallback when its squared magnitude is at most
 * 0.001. */
s32 fn_1_F5F8(Vec *direction, Vec *unitDirection) {
    f32 verticalOffset;

    if (PSVECSquareMag(direction) <= 0.001) {
        unitDirection->x = 0.01f * ((f32) frandmod(20) - 10.0f);
        unitDirection->z = 0.01f * ((f32) frandmod(20) - 10.0f);
        /* A modulus of one always yields zero, so the fallback points downward. */
        if (frandmod(1) != 0) {
            verticalOffset = 0.01f;
        } else {
            verticalOffset = -0.01f;
        }
        unitDirection->y = verticalOffset;
        PSVECNormalize(unitDirection, unitDirection);
        return 0;
    }
    PSVECNormalize(direction, unitDirection);
    return 1;
}

/* Rotates a direction through the supplied X, Y and Z angles for effect setup. */
void fn_1_F750(f32 angle_x, f32 angle_y, f32 angle_z,
               const Vec *source, Vec *destination)
{
    Mtx x_rotation;
    Mtx y_rotation;

    PSMTXRotRad(x_rotation, 'x', 0.017453292f * angle_x);
    PSMTXRotRad(y_rotation, 'y', 0.017453292f * angle_y);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXRotRad(y_rotation, 'z', 0.017453292f * angle_z);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXMultVecSR(x_rotation, source, destination);
}
