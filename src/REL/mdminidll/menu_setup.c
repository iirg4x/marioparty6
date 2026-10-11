/* Resources, player options, camera and windows for the minigame-mode menu. */
#include "REL/mdminidll/window_callback.h"
/* Host builds use the complete audio declarations. */
#ifndef __MWERKS__
#include "game/audio.h"
#endif
#include "dolphin/os.h"
#include "datadir_enum.h"
#include "game/flag.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/wipe.h"

typedef struct MDMinidllAudioTable {
    s32 soundId[16]; /* Two sets of seven selection sounds, each followed by -1. */
} MDMinidllAudioTable;

extern s32 lbl_1_data_690;
extern char lbl_1_data_694[];

/* The window message callback plays its indexed sound once per new message; the empty exception
 * list leaves the panned-sound branches unused. */
void fn_1_0(HUWINID unusedWindow, u32 messageId, s16 soundIndex)
{
    s32 panExceptions[1] = {-1};
    MDMinidllAudioTable sounds = {
        { MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28, MSM_SE_GUIDE_29,
          MSM_SE_GUIDE_30, MSM_SE_GUIDE_31, -1, MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19,
          MSM_SE_GUIDE_20, MSM_SE_GUIDE_21, MSM_SE_GUIDE_22, MSM_SE_GUIDE_23, -1 }
    };
    s16 exceptionIndex;

    soundIndex -= 1;
    OSReport(lbl_1_data_694, (s16) soundIndex);
    if ((u32) lbl_1_data_690 != messageId) {
        lbl_1_data_690 = messageId;
        exceptionIndex = 0;
nextSoundException:
        if (panExceptions[exceptionIndex] == -1) {
            HuAudFXPlay(sounds.soundId[soundIndex]);
            return;
        }
        if (messageId == (u32) panExceptions[exceptionIndex]) {
            if (soundIndex >= 8) {
                HuAudFXPlayPan(sounds.soundId[soundIndex], 80);
                return;
            }
            HuAudFXPlayPan(sounds.soundId[soundIndex], 48);
            return;
        }
        exceptionIndex += 1;
        goto nextSoundException;
    }
}

#include "game/gamework.h"

/* Minigame lists used to decide which menu modes are available. */
extern s16 lbl_1_data_0[];
extern s16 lbl_1_data_2E[];
extern s16 lbl_1_data_44[];
extern s16 lbl_1_data_5C[];
extern s16 lbl_1_data_7C[];

/* Mode availability checks the unlock records required by each menu mode. */
s16 fn_1_1A4(s16 mode)
{
    s16 index;
    s16 result;

    result = 0;
    switch (mode) {
    case 0:
        result = 1;
        break;
    default:
        break;
    case 1:
        index = 0;
battlePrimaryList:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto battlePrimaryList;
        }
        index = 0;
battleSecondaryList:
        if (lbl_1_data_2E[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_2E[index]) != 1) {
            index += 1;
            goto battleSecondaryList;
        }
        index = 0;
battleTertiaryList:
        if (lbl_1_data_44[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_44[index]) != 1) {
            index += 1;
            goto battleTertiaryList;
        }
        result = 1;
        break;
    case 2:
        index = 0;
bingoGameList:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto bingoGameList;
        }
        result = 1;
        break;
    case 3:
        index = 0;
tournamentGameList:
        if (lbl_1_data_5C[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_5C[index]) != 1) {
            index += 1;
            goto tournamentGameList;
        }
        result = 1;
        break;
    case 4:
        index = 0;
decathlonGameList:
        if (lbl_1_data_7C[index] == -1) {
            return 1;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_7C[index]) != 0) {
            index += 1;
            goto decathlonGameList;
        }
        result = 0;
        break;
    case 5:
        index = 0;
singlePlayerPrimaryList:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto singlePlayerPrimaryList;
        }
        index = 0;
singlePlayerSecondaryList:
        if (lbl_1_data_2E[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_2E[index]) != 1) {
            index += 1;
            goto singlePlayerSecondaryList;
        }
        index = 0;
singlePlayerTertiaryList:
        if (lbl_1_data_5C[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_5C[index]) != 1) {
            index += 1;
            goto singlePlayerTertiaryList;
        }
        result = 1;
        break;
    }
    return result;
}

#include "game/gamework.h"
extern s16 lbl_1_bss_28;
extern s16 lbl_1_bss_2A;
extern s16 lbl_1_data_6B6[6];
s16 fn_1_1A4(s16 mode);

/* Menu setup snapshots bank unlocks and computes each mode's availability. */
void fn_1_558(void)
{
    lbl_1_bss_28 = 0;
    lbl_1_bss_2A = 0;
    if (GWBankFlagGet(2) != 0) lbl_1_bss_28 = 1;
    if (GWBankFlagGet(3) != 0) lbl_1_bss_2A = 1;
    lbl_1_data_6B6[0] = 1;
    lbl_1_data_6B6[1] = 1;
    lbl_1_data_6B6[2] = 1;
    lbl_1_data_6B6[3] = 1;
    lbl_1_data_6B6[4] = 1;
    lbl_1_data_6B6[5] = 0;
    if (GWBankFlagGet(7) != 0) lbl_1_data_6B6[5] = 1;
    lbl_1_data_6B6[1] = fn_1_1A4(1);
    lbl_1_data_6B6[2] = fn_1_1A4(2);
    lbl_1_data_6B6[3] = fn_1_1A4(3);
    lbl_1_data_6B6[4] = fn_1_1A4(4);
    if (lbl_1_data_6B6[5] == 1) lbl_1_data_6B6[5] = fn_1_1A4(5);
}

#include "game/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/process.h"

/* Character motion archives are reloaded when any selected character lacks one. */
void fn_1_6CC(void)
{
    s16 playerIndex;
    s32 characterMotionReadStatus;

    playerIndex = 0;
    while (playerIndex < 4) {
        if ((void *) CharMotionAMemPGet((s16) GwPlayer[playerIndex].charNo) == NULL) {
            break;
        }
        playerIndex += 1;
    }
    if (playerIndex != 4) {
        CharDataClose(CHARNO_NONE);
        playerIndex = 0;
        while (playerIndex < 4) {
            characterMotionReadStatus = HuDataDirReadAsync(
                CharDataDirTbl[GwPlayer[playerIndex].charNo][4]);
            /* A failed asynchronous read is not retried before motion initialization. */
            if (characterMotionReadStatus != -1) {
                while (HuDataGetAsyncStat(characterMotionReadStatus) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[playerIndex].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[playerIndex].charNo][4]);
            playerIndex += 1;
        }
    }
}

#include "game/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/process.h"
#include "game/armem.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "REL/mdminidll/player_config.h"

#define MDMINIDLL_MOTION_BLEND_FRAMES 15.0f

typedef struct MDMinidllMotionDirectoryIds {
    s32 directoryIds[6];
} MDMinidllMotionDirectoryIds;

extern s16 lbl_1_bss_2C;

/* The child created by fn_1_A4C prepares character motion archives and loads the selected mode
* archive only for modes zero through four; mode five skips that archive load. */
void fn_1_81C(void)
{
    MDMinidllMotionDirectoryIds motionDirectoryIds = {
        {DATA_mgmfree, DATA_mgmbattle, DATA_mgmbingo, DATA_mgmtournament,
         DATA_mgmdecathlon, DATA_mgmrensho}};
    s16 playerIndex;
    s32 selectedModeMotionReadStatus;
    s32 characterMotionReadStatus;

    playerIndex = 0;
    while (playerIndex < 4) {
        if ((void *) CharMotionAMemPGet((s16) GwPlayer[playerIndex].charNo) == NULL) {
            break;
        }
        playerIndex += 1;
    }
    if (playerIndex != 4) {
        CharDataClose(CHARNO_NONE);
        playerIndex = 0;
        while (playerIndex < 4) {
            characterMotionReadStatus = HuDataDirReadAsync(
                CharDataDirTbl[GwPlayer[playerIndex].charNo][4]);
            /* A failed asynchronous read is not retried before motion initialization. */
            if (characterMotionReadStatus != -1) {
                while (HuDataGetAsyncStat(characterMotionReadStatus) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[playerIndex].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[playerIndex].charNo][4]);
            playerIndex += 1;
        }
    }
    if (lbl_1_bss_804[1] <= 4) {
        selectedModeMotionReadStatus = HuDataDirReadAsync(
            motionDirectoryIds.directoryIds[lbl_1_bss_804[1]]);
        if (selectedModeMotionReadStatus != -1) {
            while (HuDataGetAsyncStat(selectedModeMotionReadStatus) == 0) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(motionDirectoryIds.directoryIds[lbl_1_bss_804[1]]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(motionDirectoryIds.directoryIds[lbl_1_bss_804[1]]);
    }
    lbl_1_bss_2C = 1;
    HuPrcEnd();
processEndWait:
    HuPrcVSleep();
    goto processEndWait;
}

#include "dolphin/os.h"
#include "game/armem.h"
#include "game/data.h"
#include "game/object.h"
#include "game/process.h"
#include "game/charman.h"

extern s16 lbl_1_bss_2C;
extern OMOBJMAN *lbl_1_bss_4;
extern char lbl_1_data_6C2[];
extern char lbl_1_data_6F3[];
extern char lbl_1_data_704[];
extern char lbl_1_data_716[];
extern char lbl_1_data_728[];
extern char lbl_1_data_736[];
extern char lbl_1_data_738[];
extern char lbl_1_data_767[];

void fn_1_81C(void);

/* Called by fn_1_1E1AC during its transition to release mode data and start the motion loader. */
void fn_1_A4C(void)
{
    lbl_1_bss_2C = 0;
    OSReport(lbl_1_data_6C2);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
    HuDataDirClose(DATA_mdmini);
    HuPrcChildCreate(fn_1_81C, 256, 16384, 0, lbl_1_bss_4);
}

/* The menu reset helper releases cached minigame-mode archives and prints the remaining ARAM
 * allocations. */
void fn_1_B18(void)
{
    HuARDirFree(DATA_mgmfree);
    HuARDirFree(DATA_mgmbattle);
    HuARDirFree(DATA_mgmbingo);
    HuARDirFree(DATA_mgmtournament);
    HuARDirFree(DATA_mgmdecathlon);
    OSReport(lbl_1_data_738);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
}

/* The menu return helper closes every character archive, releases mode archives and prints the
 * remaining ARAM allocations. */
void fn_1_BD0(void)
{
    CharDataClose(CHARNO_NONE);
    HuARDirFree(DATA_mgmfree);
    HuARDirFree(DATA_mgmbattle);
    HuARDirFree(DATA_mgmbingo);
    HuARDirFree(DATA_mgmtournament);
    HuARDirFree(DATA_mgmdecathlon);
    OSReport(lbl_1_data_767);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
}

/* Minigame-mode setup puts every player on team zero. */
void fn_1_C90(void)
{
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        GwPlayer[playerIndex].comF = lbl_1_bss_7CC[playerIndex].comF;
        GwPlayer[playerIndex].comDif = lbl_1_bss_7CC[playerIndex].comDif;
        GwPlayer[playerIndex].charNo = lbl_1_bss_7CC[playerIndex].charNo;
        GwPlayer[playerIndex].padNo = (s8) lbl_1_bss_7CC[playerIndex].padNo;
        GwPlayer[playerIndex].team = 0;
        playerIndex += 1;
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        GwPlayerConf[playerIndex].grpNo = 0;
        GwPlayerConf[playerIndex].type = GwPlayer[playerIndex].comF;
        GwPlayerConf[playerIndex].comDif = GwPlayer[playerIndex].comDif;
        GwPlayerConf[playerIndex].charNo = GwPlayer[playerIndex].charNo;
        GwPlayerConf[playerIndex].padNo = GwPlayer[playerIndex].padNo;
        GwPlayerConf[playerIndex].grpNo = GwPlayer[playerIndex].team;
        playerIndex += 1;
    }
}

#include "game/object.h"
#include "game/process.h"
#include "game/armem.h"
#include "dolphin/os.h"
#include "REL/mdminidll/player_config.h"

extern s16 lbl_1_bss_2C;
extern char lbl_1_data_798[];
extern char lbl_1_data_6F3[];
extern char lbl_1_data_704[];
extern char lbl_1_data_716[];
extern char lbl_1_data_728[];

/* During the mode handoff, waits for the motion loader to finish, logs ARAM state and queues the
* chosen minigame-mode overlay. */
void fn_1_F10(void)
{
    OMOVLHIS *historyEntry;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_2C == 0);

    historyEntry = omOvlHisGet(0);
    omOvlHisChg(0, historyEntry->ovl, 1, 0);
    OSReport(lbl_1_data_798);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);

    switch (lbl_1_bss_804[1]) {
    case 0:
        omOvlCallEx(DLL_mgmfreedll, 1, 0, 0);
        break;
    case 1:
        omOvlCallEx(DLL_mgmbattledll, 1, 0, 0);
        break;
    case 2:
        omOvlCallEx(DLL_mgmbingodll, 1, 0, 0);
        break;
    case 3:
        omOvlCallEx(DLL_mgmtournamentdll, 1, 0, 0);
        break;
    case 4:
        omOvlCallEx(DLL_mgmdecathlondll, 1, 0, 0);
        break;
    case 5:
        omOvlCallEx(DLL_mgmrenshodll, 1, 0, 0);
        break;
    }
}

#include "game/hu3d.h"

/* Curve and easing helpers used by menu model and camera transitions. */

/* Character departure paths use this to evaluate one quadratic Bezier coordinate. */
f32 fn_1_10AC(f32 startValue, f32 controlValue, f32 endValue, f32 progress)
{
    f32 remaining;

    remaining = 1.0f - progress;
    return (endValue * (progress * progress)) +
        ((startValue * (remaining * remaining)) +
         (2.0f * (controlValue * (remaining * progress))));
}

/* Character departure updates evaluate the same curve independently for x, y and z. */
void fn_1_1108(Point3d *position, const Point3d *start,
               const Point3d *control, const Point3d *end, f32 progress)
{
    position->x = fn_1_10AC(start->x, control->x, end->x, progress);
    position->y = fn_1_10AC(start->y, control->y, end->y, progress);
    position->z = fn_1_10AC(start->z, control->z, end->z, progress);
}

/* Camera updates use this easing step to approach a target each frame. */
f32 fn_1_1310(f32 currentValue, f32 targetValue, f32 easingFactor)
{
    if (currentValue == targetValue) {
        return targetValue;
    }
    return (targetValue + (currentValue * (easingFactor - 1.0f))) / easingFactor;
}

/* Camera interpolation applies the easing step to each vector component. */
void fn_1_1340(Vec *current, const Vec *target, float weight)
{
    current->x = fn_1_1310(current->x, target->x, weight);
    current->y = fn_1_1310(current->y, target->y, weight);
    current->z = fn_1_1310(current->z, target->z, weight);
}

/* Menu transitions use a quarter-sine ease-out curve, clamped at its start and end. */
f32 fn_1_1484(f32 startValue, f32 endValue, f32 elapsed, f32 duration)
{
    if (elapsed <= 0.0f) {
        return startValue;
    }
    if (elapsed >= duration) {
        return endValue;
    }
    return (f32) ((f64) startValue +
        ((f64) (endValue - startValue) *
         sin((M_PI *
              (f64) ((90.0f / duration) * elapsed)) /
             180.0)));
}

/* Model motion uses this bounded linear interpolation for each position axis. */
float fn_1_156C(float startValue, float endValue, float elapsed, float duration)
{
    if (elapsed <= 0.0f) {
        return startValue;
    }
    if (elapsed >= duration) {
        return endValue;
    }
    return startValue + ((elapsed / duration) * (endValue - startValue));
}

/* Host movement updates position and turns the model toward its linear path. */
void fn_1_15B0(s16 modelId, Point3d *start, Point3d *end, f32 time, f32 duration)
{
    Point3d modelPosition;
    Point3d modelRotation;
    Point3d position;
    Point3d rotation;

    Hu3DModelPosGet(modelId, &modelPosition);
    Hu3DModelRotGet(modelId, &modelRotation);
    position.x = fn_1_156C(start->x, end->x, time, duration);
    position.y = fn_1_156C(start->y, end->y, time, duration);
    position.z = fn_1_156C(start->z, end->z, time, duration);
    modelPosition.x -= position.x;
    modelPosition.z -= position.z;
    rotation.y = (f32) -(180.0 * (atan2((f64) modelPosition.x, -modelPosition.z) / M_PI));
    if (modelRotation.y - rotation.y > 180.0f) {
        modelRotation.y -= 360.0f;
    } else if (modelRotation.y - rotation.y < -180.0f) {
        modelRotation.y += 360.0f;
    }
    rotation.x = modelRotation.x;
    rotation.y = fn_1_1310(modelRotation.y, rotation.y, 10.0f);
    rotation.z = modelRotation.z;
    Hu3DModelPosSetV(modelId, &position);
    Hu3DModelRotSetV(modelId, &rotation);
}

/* Host movement eases the model toward its final heading after the position path ends. */
void fn_1_1918(s16 modelId, f32 targetYaw)
{
    Point3d modelRotation;
    Point3d rotation;

    Hu3DModelRotGet(modelId, &modelRotation);
    rotation.y = targetYaw;
    if (modelRotation.y - rotation.y > 180.0f) {
        modelRotation.y -= 360.0f;
    } else if (modelRotation.y - rotation.y < -180.0f) {
        modelRotation.y += 360.0f;
    }
    rotation.x = modelRotation.x;
    rotation.y = fn_1_1310(modelRotation.y, rotation.y, 10.0f);
    rotation.z = modelRotation.z;
    Hu3DModelRotSetV(modelId, &rotation);
}

/* Host movement switches from following a linear path to easing toward the final heading. */
void fn_1_1A58(s16 modelId, Point3d *start, Point3d *end, f32 time, f32 duration, f32 targetYaw)
{
    if (time <= duration) {
        fn_1_15B0(modelId, start, end, time, duration);
    } else {
        fn_1_1918(modelId, targetYaw);
    }
}

/* Preview departure effects rise from the starting height to the peak at half-duration, then
 * return to the starting height. */
f32 fn_1_1ED8(f32 startValue, f32 endValue, f32 elapsed, f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return startValue;
    }
    return (f32) ((f64) startValue +
        ((f64) (endValue - startValue) *
         sin((M_PI *
              (f64) ((180.0f / duration) * elapsed)) /
             180.0)));
}

#include "REL/mdminidll/camera.h"
#include "string.h"
#include "game/pad.h"

static inline float fn_1_1868_inlined(float startValue, float endValue, float elapsedFrames,
                                      float durationFrames);
static inline void fn_1_1404_inlined(HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c,
                                     float t);
static inline float fn_1_160C_inlined(float currentValue, float targetValue, float easingFactor);
static inline float fn_1_13A8_inlined(float startValue, float controlValue, float endValue,
                                      float progress);

/* Camera updates clamp elapsed transition time before interpolating a value. */
static inline float fn_1_1868_inlined(float startValue, float endValue, float elapsedFrames,
                                      float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((elapsedFrames / durationFrames) * (endValue - startValue));
}

/* Host departure paths evaluate the curve for all three position axes. */
static inline void fn_1_1404_inlined(
    HuVecF *position, const HuVecF *start, const HuVecF *control,
    const HuVecF *end, float progress)
{
    position->x = fn_1_13A8_inlined(start->x, control->x, end->x, progress);
    position->y = fn_1_13A8_inlined(start->y, control->y, end->y, progress);
    position->z = fn_1_13A8_inlined(start->z, control->z, end->z, progress);
}

/* Host departure turns use this weighted step to approach the travel heading. */
static inline float fn_1_160C_inlined(float currentValue, float targetValue, float easingFactor)
{
    if (currentValue == targetValue) {
        return targetValue;
    }
    return (targetValue + (currentValue * (easingFactor - 1.0f))) / easingFactor;
}

/* Host departure paths use this quadratic curve for one position coordinate. */
static inline float fn_1_13A8_inlined(float startValue, float controlValue, float endValue,
                                      float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

/* The host departure callback moves its model along a Bezier path and faces it along its travel. */
void fn_1_1FAC(
    HU3D_MODELID modelId, const HuVecF *start, const HuVecF *control,
    const HuVecF *end,
    float time, float duration)
{
    HuVecF pathPosition;
    HuVecF modelPosition;
    HuVecF modelRotation;
    float pathProgress;

    pathProgress = fn_1_1868_inlined(0.0f, 1.0f, time, duration);
    fn_1_1404_inlined(&pathPosition, start, control, end, pathProgress);
    Hu3DModelPosGet(modelId, &modelPosition);
    Hu3DModelRotGet(modelId, &modelRotation);
    modelPosition.x -= pathPosition.x;
    modelPosition.z -= pathPosition.z;
    modelPosition.y = -(180.0 * (atan2(modelPosition.x, -modelPosition.z) / M_PI));
    if (modelRotation.y - modelPosition.y > 180.0f) {
        modelRotation.y -= 360.0f;
    } else if (modelRotation.y - modelPosition.y < -180.0f) {
        modelRotation.y += 360.0f;
    }
    modelRotation.y = fn_1_160C_inlined(modelRotation.y, modelPosition.y, 10.0f);
    Hu3DModelPosSet(modelId, pathPosition.x, pathPosition.y, pathPosition.z);
    Hu3DModelRotSet(modelId, 0.0f, modelRotation.y, 0.0f);
}

/* Entering player setup resets the four menu entries to their default inputs. */
void fn_1_2400(void)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;

    playerIndex = 0;
    playerConfig = lbl_1_bss_7CC;
    while (playerIndex < 4) {
        playerConfig->selectionPhase = 0;
        playerConfig->unusedSetupValue = 0;
        playerConfig->comDif = 0;
        playerConfig->charNo = playerIndex;
        playerConfig->padNo = playerIndex;
        playerIndex += 1;
        playerConfig += 1;
    }
}

/* Player setup forces player zero human, then assigns the remaining requested humans to connected
 * controller slots one through three. */
void fn_1_2460(void)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 humanPlayersRemaining;
    s16 playerIndex;

    humanPlayersRemaining = lbl_1_bss_804[0];
    lbl_1_bss_7CC->comF = 0;
    humanPlayersRemaining -= 1;
    playerIndex = 1;
    playerConfig = &lbl_1_bss_7CC[1];
    while (playerIndex < 4) {
        if (humanPlayersRemaining > 0 && HuPadStatGet(playerIndex) == 0) {
            playerConfig->comF = 0;
            humanPlayersRemaining -= 1;
        } else {
            playerConfig->comF = 1;
        }
        playerIndex += 1;
        playerConfig += 1;
    }
}

/* A camera transition snaps the live camera values to their targets. */
void fn_1_2514(MDMinidllCameraState *camera)
{
    memcpy(&camera->center, &camera->centerTarget, sizeof(HuVecF));
    memcpy(&camera->rotation, &camera->rotationTarget, sizeof(HuVecF));
    camera->zoom = camera->zoomTarget;
}

/* A camera transition snapshots current values as the new targets. */
void fn_1_2564(MDMinidllCameraState *camera)
{
    memcpy(&camera->centerTarget, &camera->center, sizeof(HuVecF));
    memcpy(&camera->rotationTarget, &camera->rotation, sizeof(HuVecF));
    camera->zoomTarget = camera->zoom;
}

/* Each camera update eases its center, rotation and zoom toward the current target using the
 * supplied divisor. */
void fn_1_25B4(MDMinidllCameraState *cameraState, float easingFactor)
{
    fn_1_1340(&cameraState->center, &cameraState->centerTarget, easingFactor);
    fn_1_1340(&cameraState->rotation, &cameraState->rotationTarget, easingFactor);
    cameraState->zoom = fn_1_1310(cameraState->zoom, cameraState->zoomTarget, easingFactor);
}

/* Camera transitions replace the menu camera callback through this helper. */
void fn_1_2868(MDMinidllCameraCallback callback)
{
    lbl_1_bss_808.callback = callback;
}

#include "game/printfunc.h"
extern s16 lbl_1_bss_0;
extern HU3D_LIGHTID lbl_1_bss_860[2];
extern char lbl_1_data_7C8[], lbl_1_data_7EA[], lbl_1_data_804[], lbl_1_data_81E[];

/* The camera-test callback moves the menu view from controller-zero sticks and triggers, then
 * prints its center, rotation and zoom. */
void fn_1_2878(OMOBJ *unusedObject, MDMinidllCameraState *cameraState)
{
    float stickX = HuPadStkX[0];
    float stickY = -HuPadStkY[0];
    HuVec2f subStick;
    float worldMoveX;
    float worldMoveZ;

    subStick.y = -HuPadSubStkY[0];
    subStick.x = HuPadSubStkX[0];
    cameraState->rotationTarget.x += 0.02f * subStick.y;
    if (cameraState->rotationTarget.x > 85.0f) {
        cameraState->rotationTarget.x = 85.0f;
    }
    if (cameraState->rotationTarget.x < -85.0f) {
        cameraState->rotationTarget.x = -85.0f;
    }
    cameraState->rotationTarget.y += 0.02f * subStick.x;

    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        cameraState->centerTarget.y -= 0.25f * stickY;
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        cameraState->zoomTarget += 0.25f * stickY;
        if (cameraState->zoomTarget < 5.0f) {
            cameraState->zoomTarget = 5.0f;
        }
    } else {
        worldMoveX = stickX * sin((M_PI * (90.0f + cameraState->rotation.y)) / 180.0)
            + stickY * sin((M_PI * (cameraState->rotation.y)) / 180.0);
        worldMoveZ = stickX * cos((M_PI * (90.0f + cameraState->rotation.y)) / 180.0)
            + stickY * cos((M_PI * (cameraState->rotation.y)) / 180.0);
        cameraState->centerTarget.x += 0.25f * worldMoveX;
        cameraState->centerTarget.z += 0.25f * worldMoveZ;
    }
    fn_1_25B4(cameraState, 15.0f);

    print8(16, 110, 1.0f, lbl_1_data_7C8);
    print8(16, 120, 1.0f, lbl_1_data_7EA,
        cameraState->center.x, cameraState->center.y, cameraState->center.z);
    print8(16, 130, 1.0f, lbl_1_data_804,
        cameraState->rotation.x, cameraState->rotation.y, cameraState->rotation.z);
    print8(16, 140, 1.0f, lbl_1_data_81E, cameraState->zoom);
}

/* The camera dispatch helper runs the active menu camera callback when one is installed. */
void fn_1_2F88(OMOBJ *cameraObject, MDMinidllCameraState *cameraState)
{
    if (cameraState->callback) {
        cameraState->callback(cameraObject, cameraState);
    }
}

/* The camera-view object runs its callback each update, copies the menu view to the shared camera
 * values and updates camera zero. */
void fn_1_2FD4(OMOBJ *cameraObject)
{
    MDMinidllCameraState *cameraState;

    cameraState = &lbl_1_bss_808;
    if (cameraState->callback) {
        cameraState->callback(cameraObject, cameraState);
    }
    Center.x = cameraState->center.x;
    Center.y = cameraState->center.y;
    Center.z = cameraState->center.z;
    CRot.x = cameraState->rotation.x;
    CRot.y = cameraState->rotation.y;
    CRot.z = cameraState->rotation.z;
    CZoom = cameraState->zoom;
    omOutView(cameraObject);
}

/* Menu initialization creates camera zero, sets the initial view for fresh entry or return and
 * installs its update object. */
void fn_1_30A4(MDMinidllCameraCallback callback)
{
    MDMinidllCameraState *cameraState;

    cameraState = &lbl_1_bss_808;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    memset(cameraState, 0, 80U);
    cameraState->callback = callback;
    cameraState->center.x = 0.0f;
    cameraState->center.y = 65.0f;
    cameraState->center.z = -800.0f;
    cameraState->rotation.x = -7.25f;
    cameraState->rotation.y = 0.0f;
    cameraState->rotation.z = 0.0f;
    if (lbl_1_bss_0 == 0) {
        cameraState->zoom = 2650.0f;
    } else {
        cameraState->zoom = 2150.0f;
    }
    cameraState->outViewObj = omAddObjEx(lbl_1_bss_4, 32730, 16U, 16U, -1, fn_1_2FD4);
}

/* Menu camera teardown disables camera zero and releases its view-update object. */
void fn_1_3240(void)
{
    MDMinidllCameraState *cameraState;
    cameraState = &lbl_1_bss_808;
    Hu3DCameraKill(1);
    if (cameraState->outViewObj) {
        omDelObjEx(lbl_1_bss_4, cameraState->outViewObj);
    }
    cameraState->outViewObj = NULL;
}

/* Menu scene setup creates two fixed white directional lights. */
void fn_1_329C(void)
{
    lbl_1_bss_860[0] = Hu3DGLightCreate(0.0f, 1.0f, 1.0f,
                                        0.0f, -1.0f, -1.0f,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[0]);
    Hu3DGLightStaticSet(lbl_1_bss_860[0], TRUE);
    lbl_1_bss_860[1] = Hu3DGLightCreate(-1.0f, 1.0f, -1.0f,
                                        1.0f, -1.0f, -1.0f,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[1]);
    Hu3DGLightStaticSet(lbl_1_bss_860[1], TRUE);
}

/* Menu scene teardown releases both directional lights. */
void fn_1_33C8(void)
{
    Hu3DGLightKill(lbl_1_bss_860[0]);
    Hu3DGLightKill(lbl_1_bss_860[1]);
}

#include "game/sprite.h"
extern HUWINID lbl_1_bss_858[4];
extern s16 lbl_1_data_82C[2];
extern s32 lbl_1_data_830[3];
typedef struct MDMinidllSpriteLayoutEntry {
    /* Sprite group and member slots populated by the menu setup loop. */
    s16 groupIndex;
    s16 memberIndex;
    /* Animation-table slot and render priority offset from 6000. */
    s16 animationIndex;
    s16 priority;
    /* Sprite creation parameter passed through to HuSprCreate. */
    s16 spriteParam;
    /* Padding that aligns the following floating-point layout values. */
    u8 alignmentPadding[2];
    /* Local position, scale and z rotation passed to the sprite transforms. */
    f32 positionX;
    f32 positionY;
    f32 scaleX;
    f32 scaleY;
    f32 rotation;
} MDMinidllSpriteLayoutEntry;

/* Sprite resource IDs, group sizes and initial member transforms. */
extern s32 lbl_1_data_98[];
extern s16 lbl_1_data_100[];
extern MDMinidllSpriteLayoutEntry lbl_1_data_114[27];
extern s16 lbl_1_bss_71C[27];
extern s16 lbl_1_bss_752[9];
extern ANIMDATA *lbl_1_bss_764[26];

/* Message helpers show the instruction window immediately or animate another menu window open. */
void fn_1_3404(s16 windowIndex)
{
    if (windowIndex == 0) {
        HuWinDispOn(lbl_1_bss_858[windowIndex]);
        return;
    }
    HuWinExOpen(lbl_1_bss_858[windowIndex]);
}

/* Message helpers hide the instruction window immediately or animate another menu window closed. */
void fn_1_3474(s16 windowIndex)
{
    if (windowIndex == 0) {
        HuWinDispOff(lbl_1_bss_858[windowIndex]);
        return;
    }
    HuWinExClose(lbl_1_bss_858[windowIndex]);
}

/* Menu message helpers wait until the selected window finishes its current text or input state. */
void fn_1_34E4(s16 windowIndex)
{
    HuWinMesWait(lbl_1_bss_858[windowIndex]);
}

/* Menu prompts wait for a choice, optionally disabling cancellation; choice mode two maps
 * cancellation to the second choice. */
s16 fn_1_3520(s16 windowIndex, s16 choiceMode)
{
    s16 selectedChoice;
    selectedChoice = 0;
    if (choiceMode == 1) {
        HuWinAttrSet(lbl_1_bss_858[windowIndex], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_858[windowIndex], HUWIN_ATTR_NOCANCEL);
    }
    selectedChoice = HuWinChoiceGet(lbl_1_bss_858[windowIndex], -1);
    if ((choiceMode == 2) && (selectedChoice == -1)) {
        selectedChoice = 1;
    }
    return selectedChoice;
}

/* When a menu prompt changes, centers its text, sets the reveal speed and clears the
 * sound-selection cache if the message differs. */
void fn_1_35F4(s16 windowIndex, u32 messageId, s16 textSpeed)
{
    HuWinAttrSet(lbl_1_bss_858[windowIndex], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_858[windowIndex], messageId);
    HuWinMesSpeedSet(lbl_1_bss_858[windowIndex], textSpeed);
    if ((s32)lbl_1_data_690 != (s32)messageId) {
        lbl_1_data_690 = -1U;
    }
}

/* Menu prompt helpers clear the current page and set the message used for one text-insertion
 * slot. */
void fn_1_36B0(s16 windowIndex, s32 messageId, s16 insertionSlot)
{
    HuWinHomeClear(lbl_1_bss_858[windowIndex]);
    HuWinInsertMesSet(lbl_1_bss_858[windowIndex], messageId, insertionSlot);
}

/* Menu initialization creates four message windows and assigns controller-zero input and the menu
 * sound callback to each. */
void fn_1_3720(void)
{
    s16 windowIndex;

    HuWinInit(1);
    lbl_1_bss_858[0] = HuWinExCreateFrame(16.0f, 337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_858[0]);
    HuWinBGTPLvlSet(lbl_1_bss_858[0], 0.0f);
    lbl_1_bss_858[1] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_858[1]);
    HuWinBGTPLvlSet(lbl_1_bss_858[1], 0.9f);
    lbl_1_bss_858[2] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_858[2]);
    HuWinBGTPLvlSet(lbl_1_bss_858[2], 0.9f);
    lbl_1_bss_858[3] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_858[3]);
    HuWinBGTPLvlSet(lbl_1_bss_858[3], 0.9f);
    for (windowIndex = 0; windowIndex < 4; windowIndex++) {
        winData[lbl_1_bss_858[windowIndex]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_858[windowIndex], fn_1_0);
    }
}

/* Menu window teardown releases all four menu windows and then clears the remaining window
 * slots. */
void fn_1_3948(void)
{
    s16 windowIndex;
    windowIndex = 0;
    while (windowIndex < 4) {
        HuWinExKill(lbl_1_bss_858[windowIndex]);
        windowIndex += 1;
    }
    HuWinAllKill();
}

/* Before changing prompt text, closes a different active window and opens the requested one with
 * cleared message caches. */
void fn_1_39A4(s16 windowIndex)
{
    if (lbl_1_data_82C[0] != -1 && lbl_1_data_82C[0] != windowIndex) {
        fn_1_3474(lbl_1_data_82C[0]);
    }
    if (lbl_1_data_82C[0] == -1 || lbl_1_data_82C[0] != windowIndex) {
        lbl_1_data_82C[0] = windowIndex;
        lbl_1_data_830[0] = -1;
        lbl_1_data_830[1] = -1;
        fn_1_3404(lbl_1_data_82C[0]);
    }
}

/* Menu transitions close the active prompt window and clear its cached main and inserted
 * messages. */
void fn_1_3B10(void)
{
    if (lbl_1_data_82C[0] != -1) {
        fn_1_3474(lbl_1_data_82C[0]);
    }
    lbl_1_data_82C[0] = -1;
    lbl_1_data_830[0] = -1;
    lbl_1_data_830[1] = -1;
}

/* Menu prompt sequences wait for the active message to finish when a prompt window is open. */
void fn_1_3BCC(void)
{
    if (lbl_1_data_82C[0] != -1) {
        fn_1_34E4(lbl_1_data_82C[0]);
    }
}

/* Menu confirmation prompts return the active window choice, or zero when no prompt window is
 * open. */
s16 fn_1_3C2C(s16 choiceMode)
{
    if (lbl_1_data_82C[0] != -1) {
        return fn_1_3520(lbl_1_data_82C[0], choiceMode);
    }
    return 0;
}

/* Updates a menu window's message when the caller changes its text selection. */
void fn_1_3D24(s16 windowNo, s32 messageId, s16 textSpeed)
{
    fn_1_39A4(windowNo);
    if (lbl_1_data_830[0] != messageId) {
        lbl_1_data_830[0] = messageId;
        lbl_1_data_830[1] = -1;
        fn_1_35F4(lbl_1_data_82C[0], (u32)lbl_1_data_830[0], textSpeed);
    }
}

/* Selection prompts update inserted text only when its message changes, clearing the main-message
 * cache for the next update. */
void fn_1_3F5C(s16 windowIndex, s32 messageId, s16 insertionSlot)
{
    fn_1_39A4(windowIndex);
    if (lbl_1_data_830[1] != messageId) {
        lbl_1_data_830[0] = -1;
        lbl_1_data_830[1] = messageId;
        fn_1_36B0(lbl_1_data_82C[0], lbl_1_data_830[1], insertionSlot);
    }
}

/* Ensures the secondary message window exists, then changes its message on demand. */
void fn_1_4150(s32 messageId)
{
    if (lbl_1_data_82C[1] == -1) {
        lbl_1_data_82C[1] = 0;
        lbl_1_data_830[2] = -1;
        fn_1_3404(lbl_1_data_82C[1]);
    }
    if (lbl_1_data_830[2] != messageId) {
        lbl_1_data_830[2] = messageId;
        fn_1_35F4(lbl_1_data_82C[1], lbl_1_data_830[2], 0);
    }
}

/* When a selection step ends, closes the instruction window and clears its cached message. */
void fn_1_42D4(void)
{
    if (lbl_1_data_82C[1] != -1) {
        fn_1_3474(lbl_1_data_82C[1]);
    }
    lbl_1_data_82C[1] = -1;
    lbl_1_data_830[2] = -1;
}

/* Sets the scene shadow during fn_1_16400 object-manager setup. */
void fn_1_4380(void)
{
    Point3d shadowPosition = {0.0f, 3000.0f, 600.0f};
    Point3d shadowUp = {0.0f, 1.0f, 0.0f};
    Point3d shadowTarget = {0.0f, 0.0f, 0.0f};

    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
}

void fn_1_4434(void)
{
}

/* Applies an attribute to every member of a menu sprite group when its state changes. */
void fn_1_4438(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Clears an attribute from every member of a menu sprite group when its state changes. */
void fn_1_44B8(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

/* Builds the selection view's sprite resources and members when fn_1_4538 sets up the view. */
static inline void fn_1_4538_inlined(void)
{
    s16 layoutIndex;
    MDMinidllSpriteLayoutEntry *layoutEntry;

    layoutIndex = 0;
    while (layoutIndex < 26) {
        lbl_1_bss_764[layoutIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_98[layoutIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        layoutIndex += 1;
    }
    layoutIndex = 0;
    while (layoutIndex < 9) {
        lbl_1_bss_752[layoutIndex] = HuSprGrpCreate(lbl_1_data_100[layoutIndex]);
        layoutIndex += 1;
    }
    layoutIndex = 0;
    layoutEntry = (MDMinidllSpriteLayoutEntry *) &lbl_1_data_114;
    while (layoutIndex < 27) {
        lbl_1_bss_71C[layoutIndex] =
            HuSprCreate(lbl_1_bss_764[layoutEntry->animationIndex],
                        (s16) (layoutEntry->priority + 6000), layoutEntry->spriteParam);
        HuSprGrpMemberSet(lbl_1_bss_752[layoutEntry->groupIndex], layoutEntry->memberIndex,
                          lbl_1_bss_71C[layoutIndex]);
        HuSprPosSet(lbl_1_bss_752[layoutEntry->groupIndex], layoutEntry->memberIndex,
                    layoutEntry->positionX, layoutEntry->positionY);
        HuSprScaleSet(lbl_1_bss_752[layoutEntry->groupIndex], layoutEntry->memberIndex,
                      layoutEntry->scaleX, layoutEntry->scaleY);
        HuSprZRotSet(lbl_1_bss_752[layoutEntry->groupIndex], layoutEntry->memberIndex,
                     layoutEntry->rotation);
        layoutIndex += 1;
        layoutEntry += 1;
    }
    layoutIndex = 0;
    while (layoutIndex < 9) {
        fn_1_4438(lbl_1_bss_752[layoutIndex], HUSPR_ATTR_DISPOFF);
        layoutIndex += 1;
    }
}

/* Menu scene setup builds the sprite groups and their layout, initially hiding every group. */
void fn_1_4538(void)
{
    fn_1_4538_inlined();
}

void fn_1_478C(void)
{
}

/* Menu objects, animation resources and window state. */

#include "dolphin/types.h"
#include "REL/mdminidll/camera.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/animdata.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "humath.h"

/* Motion phases and curved paths for the host and selected characters. */
typedef struct MDMinTrajectory {
    s16 state; /* 0 for the initial rise, 1 for path travel, 2 for completion. */
    HuVecF current; /* Captured starting position for the path. */
    HuVecF from; /* Curved-path control point. */
    HuVecF to; /* Destination position. */
    f32 elapsed; /* Update frames spent in the current phase. */
    f32 limit; /* Duration of the current phase, in update frames. */
} MDMinTrajectory;

u32 lbl_1_bss_864[5];
HU3D_LIGHTID lbl_1_bss_860[2];
HUWINID lbl_1_bss_858[4];
MDMinidllCameraState lbl_1_bss_808;
s16 lbl_1_bss_804[2];
MDMinidllPlayerConfig lbl_1_bss_7CC[4];
ANIMDATA *lbl_1_bss_764[26];
HUSPR_GROUPID lbl_1_bss_752[9];
HUSPRID lbl_1_bss_71C[27];
ANIMDATA *lbl_1_bss_6C8[21];
MDMinidllModelAnimationRecord lbl_1_bss_230[21];
MDMinTrajectory lbl_1_bss_1D0[2];
MDMinTrajectory lbl_1_bss_170[2];
MDMinTrajectory lbl_1_bss_B0[4];
Point3d lbl_1_bss_80[4];
Point3d lbl_1_bss_50[4];
f32 lbl_1_bss_38[6];
s16 lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
s16 lbl_1_bss_2C;
s16 lbl_1_bss_2A;
s16 lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_1C[2];
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJMAN *lbl_1_bss_4;
s16 lbl_1_bss_0;

typedef struct MDMinidllSelectionView {
    u8 neighborBytes[16]; /* Eight signed neighbor indices stored as byte pairs; -1 ends a link. */
    s16 claimed; /* 1 for a claimed character card; -1 when it is available. */
} MDMinidllSelectionView;

s16 lbl_1_data_0[23] = {
    601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 618,
    619, 620, 621, 648, 650, -1
};
s16 lbl_1_data_2E[11] = {625, 626, 627, 628, 629, 630, 631, 632, 633, 634, -1};
s16 lbl_1_data_44[12] = {635, 636, 637, 638, 639, 640, 641, 643, 644, 645, 668, -1};
s16 lbl_1_data_5C[16] = {
    617, 624, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, -1
};
s16 lbl_1_data_7C[12] = {601, 604, 606, 608, 615, 621, 648, 650, 646, 649, -1};
s32 lbl_1_data_94 = -1;
s32 lbl_1_data_98[26] = {
    DATANUM(DATA_mdmini, 22), DATANUM(DATA_mdmini, 4), DATANUM(DATA_mdmini, 5),
    DATANUM(DATA_mdmini, 6), DATANUM(DATA_mdmini, 7), DATANUM(DATA_mdmini, 8),
    DATANUM(DATA_mdmini, 9), DATANUM(DATA_mdmini, 16), DATANUM(DATA_mdmini, 17),
    DATANUM(DATA_mdmini, 10), DATANUM(DATA_mdmini, 11), DATANUM(DATA_mdmini, 12),
    DATANUM(DATA_mdmini, 13), DATANUM(DATA_mdmini, 14), DATANUM(DATA_mdmini, 15),
    DATANUM(DATA_mdmini, 24), DATANUM(DATA_mdmini, 24), DATANUM(DATA_mdmini, 24),
    DATANUM(DATA_mdmini, 24), DATANUM(DATA_mdmini, 24), DATANUM(DATA_mdmini, 24),
    DATANUM(DATA_mdmini, 21), DATANUM(DATA_mdmini, 19), DATANUM(DATA_mdmini, 18),
    DATANUM(DATA_mdmini, 23), DATANUM(DATA_mdmini, 93)
};
s16 lbl_1_data_100[9] = {2, 6, 6, 4, 2, 2, 2, 2, 1};
MDMinidllSpriteLayoutEntry lbl_1_data_114[27] = {
    {0, 0, 0, 10, 0, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 0, 15, 0, 0, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 1, 15, 0, 1, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 2, 15, 0, 5, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 3, 15, 0, 2, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 4, 15, 0, 3, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 5, 15, 0, 4, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 0, 21, 0, 0, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 1, 21, 0, 2, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 2, 21, 0, 5, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 3, 21, 0, 7, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 4, 21, 0, 4, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 5, 21, 0, 6, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {3, 0, 22, 0, 0, {0, 0}, 0.0f, 0.0f, 0.80000001f, 0.80000001f, 0.0f},
    {3, 1, 22, 0, 0, {0, 0}, 0.0f, 0.0f, 0.80000001f, 0.80000001f, 0.0f},
    {3, 2, 22, 0, 0, {0, 0}, 0.0f, 0.0f, 0.80000001f, 0.80000001f, 0.0f},
    {3, 3, 22, 0, 0, {0, 0}, 0.0f, 0.0f, 0.80000001f, 0.80000001f, 0.0f},
    {4, 0, 22, 9, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {4, 1, 23, 10, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {5, 0, 22, 9, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {5, 1, 23, 10, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {6, 0, 22, 9, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {6, 1, 23, 10, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {7, 0, 22, 9, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {7, 1, 23, 10, 0, {0, 0}, 22.0f, -24.0f, 1.0f, 1.0f, 0.0f},
    {0, 1, 24, 0, 0, {0, 0}, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {8, 0, 25, 0, 0, {0, 0}, 288.0f, 240.0f, 10.0f, 10.0f, 0.0f}
};
s32 lbl_1_data_474[21] = {
    DATANUM(DATA_mdmini, 28), DATANUM(DATA_mdmini, 29), DATANUM(DATA_mdmini, 30),
    DATANUM(DATA_mdmini, 31), DATANUM(DATA_mdmini, 32), DATANUM(DATA_mdmini, 33),
    DATANUM(DATA_mdmini, 34), DATANUM(DATA_mdmini, 35), DATANUM(DATA_mdmini, 36),
    DATANUM(DATA_mdmini, 37), DATANUM(DATA_mdmini, 38), DATANUM(DATA_mdmini, 39),
    DATANUM(DATA_mdmini, 40), DATANUM(DATA_mdmini, 41), DATANUM(DATA_mdmini, 45),
    DATANUM(DATA_mdmini, 42), DATANUM(DATA_mdmini, 43), DATANUM(DATA_mdmini, 44),
    DATANUM(DATA_mdmini, 26), DATANUM(DATA_mdmini, 27), DATANUM(DATA_mdmini, 46)
};
Point3d lbl_1_data_4C8[38] = {
    {0.0f, 275.0f, 0.0f}, {-200.0f, 325.0f, 0.0f}, {-120.0f, 225.0f, 0.0f},
    {-40.0f, 325.0f, 0.0f}, {40.0f, 225.0f, 0.0f}, {120.0f, 325.0f, 0.0f},
    {200.0f, 225.0f, 0.0f}, {-200.0f, 275.0f, 0.0f}, {-120.0f, 275.0f, 0.0f},
    {-40.0f, 275.0f, 0.0f}, {40.0f, 275.0f, 0.0f}, {120.0f, 275.0f, 0.0f},
    {200.0f, 275.0f, 0.0f}, {0.0f, 275.0f, -800.0f}, {-195.0f, 380.0f, 0.0f},
    {-65.0f, 380.0f, 0.0f}, {65.0f, 380.0f, 0.0f}, {195.0f, 380.0f, 0.0f},
    {-130.0f, 240.0f, 0.0f}, {0.0f, 240.0f, 0.0f}, {130.0f, 240.0f, 0.0f},
    {-195.0f, 100.0f, 0.0f}, {-65.0f, 100.0f, 0.0f}, {65.0f, 100.0f, 0.0f},
    {195.0f, 100.0f, 0.0f}, {-180.0f, 60.0f, 0.0f}, {-60.0f, 60.0f, 0.0f},
    {60.0f, 60.0f, 0.0f}, {180.0f, 60.0f, 0.0f}, {-180.0f, 125.0f, 0.0f},
    {-60.0f, 125.0f, 0.0f}, {60.0f, 125.0f, 0.0f}, {180.0f, 125.0f, 0.0f},
    {-180.0f, 275.0f, 0.0f}, {-60.0f, 275.0f, 0.0f}, {60.0f, 275.0f, 0.0f},
    {180.0f, 275.0f, 0.0f}, {0.0f, 125.0f, 0.0f}
};
s32 lbl_1_data_690 = -1;
char lbl_1_data_694[34] = "# ========== win callback :: %d\n";
s16 lbl_1_data_6B6[6] = {0};
char lbl_1_data_6C2[] = "\n>>>>>>>>>> mdminidll :: READ INIT!! <<<<<<<<<<\n";
char lbl_1_data_6F3[] = "0x%x :: _effect\n";
char lbl_1_data_704[] = "0x%x :: _gamemes\n";
char lbl_1_data_716[] = "0x%x :: _mgconst\n";
char lbl_1_data_728[] = "0x%x :: _win\n";
char lbl_1_data_736[] = "\n";
char lbl_1_data_738[] = ">>>>>>>>>> mdminidll :: objsetup!! <<<<<<<<<<\n";
char lbl_1_data_767[] = "\n>>>>>>>>>> mdminidll :: dllreturn!! <<<<<<<<<<\n";
char lbl_1_data_798[] = "\n>>>>>>>>>> mdminidll :: READ END!! <<<<<<<<<<\n";
char lbl_1_data_7C8[] = "-----===== CAMERA TEST =====-----";
char lbl_1_data_7EA[] = "CENTER : %.2f, %.2f, %.2f";
char lbl_1_data_804[] = "ROT    : %.2f, %.2f, %.2f";
char lbl_1_data_81E[] = "ZOOM   : %.2f";
/* Window indices used by the two menu-window helpers; -1 means no active window. */
s16 lbl_1_data_82C[2] = {-1, -1};
s32 lbl_1_data_830[3] = {-1, -1, -1};
char lbl_1_data_83C[] = "gN00m1-itemhook_R";
char lbl_1_data_84E[] = "gN01m1-itemhook_R";
char lbl_1_data_860[] = "dpic00";
char lbl_1_data_867[] = "dpic01";
s16 lbl_1_data_86E[6] = {0};
char lbl_1_data_87A[] = "ys77120";
char lbl_1_data_882[] = "ys77121";
char lbl_1_data_88A[] = "ys77060";
char lbl_1_data_892[] = "ys77061";
MDMinidllSelectionView lbl_1_data_89A[11] = {
    {{255, 255, 255, 255, 0, 1, 0, 5, 0, 4, 255, 255, 255, 255, 255, 255}, -1},
    {{255, 255, 255, 255, 0, 2, 0, 6, 0, 5, 0, 4, 0, 0, 255, 255}, -1},
    {{255, 255, 255, 255, 0, 3, 255, 255, 0, 6, 0, 5, 0, 1, 255, 255}, -1},
    {{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 6, 0, 2, 255, 255}, -1},
    {{0, 0, 0, 1, 0, 5, 0, 9, 0, 8, 0, 7, 255, 255, 255, 255}, -1},
    {{0, 1, 0, 2, 0, 6, 0, 10, 0, 9, 0, 8, 0, 4, 0, 0}, -1},
    {{0, 2, 0, 3, 0, 10, 0, 10, 0, 10, 0, 9, 0, 5, 0, 1}, -1},
    {{255, 255, 0, 4, 0, 8, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}, -1},
    {{0, 4, 0, 5, 0, 9, 255, 255, 255, 255, 255, 255, 0, 7, 255, 255}, -1},
    {{0, 5, 0, 6, 0, 10, 255, 255, 255, 255, 255, 255, 0, 8, 0, 4}, -1},
    {{0, 6, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 9, 0, 5}, -1}
};
char lbl_1_data_960[] = "\n-----===== MARIO PARTY 6 :: MINIGAME MODE =====-----\n\n";

void fn_1_23EC4(s16 modelNo, HuVecF *pos, s16 display);

/* After confirmation, updates only the second preview departure trajectory; the first source is
 * disabled, so the two-preview completion test never clears this callback. */
void fn_1_4790(OMOBJ *animationObject)
{
    Point3d position;
    MDMinTrajectory *first;
    MDMinTrajectory *second;
    s16 completed;
    OMOBJ *firstObj;
    OMOBJ *secondObj;
    f32 duration;

    completed = 0;
    duration = 60.0f;
    /* The first slot has no source object here. */
    firstObj = NULL;
    secondObj = lbl_1_bss_10;
    first = lbl_1_bss_1D0;
    second = &lbl_1_bss_1D0[1];
    if (firstObj != NULL) {
        position.x = fn_1_1484(first->current.x, 500.0f, first->elapsed, duration);
        position.y = fn_1_1ED8(first->current.y, 400.0f, first->elapsed, duration);
        position.z = first->current.z;
        if ((first->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_23EC4(0, NULL, 0);
        } else {
            fn_1_23EC4(0, &position, 1);
        }
    }
    if (secondObj != NULL) {
        position.x = fn_1_1484(second->current.x, -500.0f, second->elapsed, duration);
        position.y = fn_1_1ED8(second->current.y, 400.0f, second->elapsed, duration);
        position.z = second->current.z;
        if ((second->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_23EC4(1, NULL, 0);
        } else {
            fn_1_23EC4(1, &position, 1);
        }
    }
    if (completed >= 2) {
        animationObject->objFunc = NULL;
    }
}

/* Captures preview positions, then switches to the exit path after the confirmation delay. */
void fn_1_4BE0(OMOBJ *transitionObject)
{
    /* The first slot has no source object here. */
    OMOBJ *firstObject = NULL;
    OMOBJ *object = lbl_1_bss_10;
    MDMinTrajectory *first = lbl_1_bss_1D0;
    MDMinTrajectory *second = &lbl_1_bss_1D0[1];

    if (firstObject != NULL) {
        Hu3DModelObjPosGet(firstObject->mdlId[0], lbl_1_data_83C,
            &first->current);
        fn_1_23EC4(0, &first->current, 1);
        first->elapsed = 0.0f;
    }
    if (object != NULL) {
        Hu3DModelObjPosGet(object->mdlId[0], lbl_1_data_83C,
            &second->current);
        fn_1_23EC4(1, &second->current, 1);
        second->elapsed = 0.0f;
    }
    if (++transitionObject->work[0] > 30U) {
        transitionObject->work[0] = 0;
        transitionObject->objFunc = fn_1_4790;
    }
}

/* The start-confirmation helper changes the host motion and creates the delayed preview-departure
 * object with its sound. */
void fn_1_4CE4(void)
{
    Hu3DMotionShiftSet(*lbl_1_bss_10->mdlId, lbl_1_bss_10->mtnId[4],
                       0.0f, 5.0f, 0U);
    lbl_1_bss_30 = omAddObjEx(lbl_1_bss_4, 4096, 16, 16, -1, fn_1_4BE0);
    lbl_1_bss_30->work[0] = 0;
    HuAudFXPlay(MSM_SE_MENU_17);
}

/* During a menu prompt gesture, speeds the host motion briefly, then restores its looping idle
* motion. */
void fn_1_4D98(OMOBJ *entryObject)
{
    Hu3DMotionSpeedSet(*entryObject->mdlId, 2.0f);
    if (entryObject->work[3]++ > 30U) {
        entryObject->objFunc = NULL;
        Hu3DMotionSpeedSet(*entryObject->mdlId, 1.0f);
        Hu3DMotionShiftSet(*entryObject->mdlId, *entryObject->mtnId, 0.0f,
                           MDMINIDLL_MOTION_BLEND_FRAMES, HU3D_MOTATTR_LOOP);
    }
}

void fn_1_26454(s16 slot, HuVecF *position, s16 enabled, s16 pattern);

/* Before displaying a menu prompt, starts the host gesture and installs the callback that returns
 * it to idle. */
void fn_1_4E40(void)
{
    OMOBJ *hostObject;

    hostObject = lbl_1_bss_10;
    Hu3DMotionShiftSet(*hostObject->mdlId, hostObject->mtnId[3], 0.0f, 10.0f, 0U);
    hostObject->work[3] = 0;
    hostObject->objFunc = fn_1_4D98;
}

/* During the final start sequence, raises the host, follows its curved departure path and positions
 * the accompanying effect each update. */
void fn_1_4EB4(OMOBJ *hostObject)
{
    MDMinTrajectory *trajectory = &lbl_1_bss_170[1];
    HuVecF hostPosition;

    if (trajectory->state == 0) {
        Hu3DModelPosGet(hostObject->mdlId[0], &hostPosition);
        hostPosition.y += 5.0f;
        Hu3DModelPosSetV(hostObject->mdlId[0], &hostPosition);
        if ((trajectory->elapsed += 1.0f) > trajectory->limit) {
            Hu3DModelPosGet(hostObject->mdlId[0], &trajectory->current);
            trajectory->state = 1;
            trajectory->elapsed = 0.0f;
            trajectory->limit = 90.0f;
        }
    }
    if (trajectory->state == 1) {
        fn_1_1FAC(hostObject->mdlId[0], &trajectory->current, &trajectory->from,
            &trajectory->to, trajectory->elapsed, trajectory->limit);
        if ((trajectory->elapsed += 1.0f) > trajectory->limit) {
            HuAudFXStop(lbl_1_bss_864[1]);
            trajectory->state = 2;
        }
    }
    Hu3DModelPosGet(hostObject->mdlId[0], &hostPosition);
    fn_1_26454(5, &hostPosition, 1, 6);
    if (trajectory->state == 2) {
        hostObject->objFunc = NULL;
    }
}

/* The final start sequence prepares the host departure path, starts its motion and sound and
 * installs the path callback. */
void fn_1_5434(void)
{
    MDMinTrajectory *trajectory = &lbl_1_bss_170[1];
    OMOBJ *hostObject = lbl_1_bss_10;

    trajectory->state = 0;
    trajectory->elapsed = 0.0f;
    trajectory->limit = 20.0f;

    /* The fetched position is replaced with fixed coordinates for each of these three path
     * points. */
    Hu3DModelPosGet(hostObject->mdlId[0], &trajectory->current);
    trajectory->current.x = 325.0f;
    trajectory->current.y = 0.0f;
    trajectory->current.z = 100.0f;

    Hu3DModelPosGet(hostObject->mdlId[0], &trajectory->from);
    trajectory->from.x = 0.0f;
    trajectory->from.y = 400.0f;
    trajectory->from.z = 200.0f;

    Hu3DModelPosGet(hostObject->mdlId[0], &trajectory->to);
    trajectory->to.x = 0.0f;
    trajectory->to.y = 125.0f;
    trajectory->to.z = -1000.0f;

    Hu3DMotionShiftSet(hostObject->mdlId[0], hostObject->mtnId[2],
        0.0f, 0.0f, 0U);
    lbl_1_bss_864[1] = HuAudFXPlay(MSM_SE_BRD00_144);
    hostObject->objFunc = fn_1_4EB4;
}

/* After the player-count introduction requests host movement, moves and turns the host to its side
 * position before restoring idle. */
void fn_1_55B0(OMOBJ *hostObject)
{
    HuVecF path[2] = {
        { 0.0f, 0.0f, 0.0f },
        { 325.0f, 0.0f, 100.0f },
    };
    float elapsedFrames = hostObject->work[0];

    fn_1_1A58(
        hostObject->mdlId[0], &path[0], &path[1], elapsedFrames, 30.0f, -15.0f);
    Hu3DMotionSpeedSet(hostObject->mdlId[0], 1.5f);
    if (++hostObject->work[0] > 60) {
        Hu3DMotionSpeedSet(hostObject->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(hostObject->mdlId[0], hostObject->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        hostObject->objFunc = NULL;
    }
}

/* Requests the host move to its side position when its current x coordinate is at most 100. */
s32 fn_1_5B48(void)
{
    Point3d modelPosition;
    OMOBJ *menuCharacter;

    menuCharacter = lbl_1_bss_10;
    Hu3DModelPosGet(*menuCharacter->mdlId, &modelPosition);
    if (modelPosition.x > 100.0f) {
        return 0;
    }
    menuCharacter->work[0] = 0;
    menuCharacter->objFunc = fn_1_55B0;
    Hu3DMotionShiftSet(*menuCharacter->mdlId, menuCharacter->mtnId[1], 0.0f,
                        0.0f, HU3D_MOTATTR_LOOP);
    return 1;
}

/* The host initialization callback creates its model, attached item and five motions, placing it
 * for fresh entry or a return. */
void fn_1_5BF4(OMOBJ *hostObject)
{
    omSetStatBit(hostObject, OM_STAT_MODELPAUSE);
    hostObject->mdlId[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 47), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mtnId[0] =
        Hu3DJointMotion(hostObject->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 49), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mtnId[1] =
        Hu3DJointMotion(hostObject->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 50), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mtnId[2] =
        Hu3DJointMotion(hostObject->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 51), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mtnId[3] =
        Hu3DJointMotion(hostObject->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 52), HU_MEMNUM_OVL, HEAP_MODEL));
    hostObject->mtnId[4] =
        Hu3DJointMotion(hostObject->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 53), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(hostObject->mdlId[0], lbl_1_data_84E, hostObject->mdlId[1]);
    if (lbl_1_bss_0 == 0) {
        Hu3DModelPosSet(hostObject->mdlId[0], 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(hostObject->mdlId[0], 0.0f, 0.0f, 0.0f);
    } else {
        Hu3DModelPosSet(hostObject->mdlId[0], 325.0f, 0.0f, 100.0f);
        Hu3DModelRotSet(hostObject->mdlId[0], 0.0f, -15.0f, 0.0f);
    }
    Hu3DModelScaleSet(hostObject->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(hostObject->mdlId[0], 1);
    Hu3DMotionShiftSet(hostObject->mdlId[0], hostObject->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(hostObject->mdlId[0]);
    hostObject->objFunc = NULL;
}

/* Host teardown releases its motions, models and object-manager slot when present; it then writes
 * the callback through the supplied pointer unconditionally. */
void fn_1_5EC0(OMOBJ *hostObject)
{
    if (hostObject) {
        Hu3DModelHookReset(hostObject->mdlId[1]);
        Hu3DMotionKill(hostObject->mtnId[0]);
        Hu3DMotionKill(hostObject->mtnId[1]);
        Hu3DMotionKill(hostObject->mtnId[2]);
        Hu3DMotionKill(hostObject->mtnId[3]);
        Hu3DMotionKill(hostObject->mtnId[4]);
        Hu3DModelKill(hostObject->mdlId[0]);
        Hu3DModelKill(hostObject->mdlId[1]);
        omDelObjEx(lbl_1_bss_4, hostObject);
    }
    hostObject->objFunc = NULL;
}

void fn_1_25EF0(s16 slot, HuVecF *position, s16 pad, s16 phase);

/* During the final start sequence, moves selected characters along their departure paths, turns
 * them toward travel and hides each model when its path ends. */
void fn_1_5F6C(OMOBJ *characterObject)
{
    Point3d characterPosition;
    Point3d modelRotation;
    Point3d modelToPath;
    Point3d position;
    MDMinidllPlayerConfig *playerConfig;
    MDMinTrajectory *trajectory;
    s16 playerIndex;
    s16 playerCount;
    s16 playerModelId;
    f32 pathProgress;
    f32 progress;
    f32 elapsedFrames;
    f32 durationFrames;

    playerCount = 4;
    if (lbl_1_bss_804[1] == 5) {
        playerCount = 1;
    } else {
        playerCount = 4;
    }
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        trajectory = &lbl_1_bss_B0[playerIndex];
        if (trajectory->state == 1) {
            durationFrames = trajectory->limit;
            elapsedFrames = trajectory->elapsed;
            playerModelId = characterObject->mdlId[playerConfig->charNo];
            progress = fn_1_156C(0.0f, 1.0f, elapsedFrames, durationFrames);
            pathProgress = progress;
            fn_1_1108(&position, &trajectory->current, &trajectory->from, &trajectory->to,
                      pathProgress);
            Hu3DModelPosGet(playerModelId, &modelToPath);
            Hu3DModelRotGet(playerModelId, &modelRotation);
            modelToPath.x -= position.x;
            modelToPath.z -= position.z;
            modelToPath.y = (f32) -(180.0 * (atan2((f64) modelToPath.x, -modelToPath.z) / M_PI));
            if ((modelRotation.y - modelToPath.y) > 180.0f) {
                modelRotation.y -= 360.0f;
            } else if ((modelRotation.y - modelToPath.y) < -180.0f) {
                modelRotation.y += 360.0f;
            }
            modelRotation.y = fn_1_1310(modelRotation.y, modelToPath.y, 10.0f);
            Hu3DModelPosSet(playerModelId, position.x, position.y, position.z);
            Hu3DModelRotSet(playerModelId, 0.0f, modelRotation.y, 0.0f);
            if ((trajectory->elapsed += 1.0f) > trajectory->limit) {
                trajectory->state = 2;
                Hu3DModelAttrSet(characterObject->mdlId[playerConfig->charNo], HU3D_ATTR_DISPOFF);
            }
            Hu3DModelPosGet(characterObject->mdlId[playerConfig->charNo], &characterPosition);
            if (playerConfig->comF == 0) {
                fn_1_26454(playerIndex, &characterPosition, 1, playerConfig->padNo);
            } else {
                fn_1_26454(playerIndex, &characterPosition, 1, 4);
            }
        }
        playerIndex += 1;
        playerConfig += 1;
    }
    playerIndex = 0;
    while (playerIndex < playerCount) {
        trajectory = &lbl_1_bss_B0[playerIndex];
        if (trajectory->state == 2) {
            playerIndex += 1;
            continue;
        }
        break;
    }
    if (playerIndex == playerCount) {
        characterObject->objFunc = NULL;
    }
}

/* The final start sequence prepares departure paths for the selected character models and installs
 * their per-frame path update. */
void fn_1_6520(void)
{
    MDMinidllPlayerConfig *playerConfig;
    MDMinTrajectory *trajectory;
    OMOBJ *characterObject;
    s16 playerCount;
    s16 playerIndex;

    playerCount = 4;
    characterObject = lbl_1_bss_14;
    if (lbl_1_bss_804[1] == 5) {
        playerCount = 1;
    } else {
        playerCount = 4;
    }
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        trajectory = &lbl_1_bss_B0[playerIndex];
        trajectory->state = 1;
        trajectory->elapsed = 0.0f;
        trajectory->limit = 90.0f;
        Hu3DModelPosGet(characterObject->mdlId[playerConfig->charNo],
                        (Point3d *) &trajectory->current.x);
        Hu3DModelPosGet(characterObject->mdlId[playerConfig->charNo],
                        (Point3d *) &trajectory->from.x);
        trajectory->from.x += 0.5f * trajectory->from.x;
        trajectory->from.y = 550.0f;
        trajectory->from.z = 100.0f;
        Hu3DModelPosGet(characterObject->mdlId[playerConfig->charNo],
                        (Point3d *) &trajectory->to.x);
        trajectory->to.y = 225.0f;
        trajectory->to.z = -800.0f;
        Hu3DMotionShiftSet(characterObject->mdlId[playerConfig->charNo],
                           characterObject->mtnId[(playerConfig->charNo * 2) + 1], 0.0f, 10.0f,
                           HU3D_MOTATTR_LOOP);
        playerIndex += 1;
        playerConfig += 1;
    }
    HuAudFXPlay(MSM_SE_MENU_19);
    characterObject->objFunc = fn_1_5F6C;
}

/* Scales the selected character models in over 15 frames after fn_1_68F0 starts the entrance. */
void fn_1_66F4(OMOBJ *characterObject)
{
    s16 playerIndex;
    MDMinidllPlayerConfig *playerConfig;
    s16 playerCount;
    f32 scale;

    playerCount = 4;
    if (lbl_1_bss_804[1] == 5) {
        playerCount = 1;
    } else {
        playerCount = 4;
    }
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        scale = fn_1_156C(0.0f, 1.0f, (f32)characterObject->work[0],
                          (f32)characterObject->work[1]);
        Hu3DModelScaleSet(characterObject->mdlId[playerConfig->charNo], scale, scale, scale);
        playerIndex += 1;
        playerConfig += 1;
    }
    if ((characterObject->work[0] += 1.0f) >
        (u32)characterObject->work[1]) {
        characterObject->objFunc = NULL;
    }
}

/* After start confirmation, positions selected character models, enables their scale-in callback
 * and triggers two colored particle phases separated by a process sleep. */
void fn_1_68F0(void)
{
    Point3d characterPosition;
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;
    OMOBJ *characterModelObject;
    s16 playerCount;

    playerCount = 4;
    characterModelObject = lbl_1_bss_14;
    if (lbl_1_bss_804[1] == 5) {
        playerCount = 1;
    } else {
        playerCount = 4;
    }
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        Hu3DModelPosGet(lbl_1_bss_230[playerConfig->charNo].model, &characterPosition);
        if (playerConfig->comF != 0) {
            fn_1_25EF0(playerIndex, &characterPosition, 4, 0);
        } else {
            fn_1_25EF0(playerIndex, &characterPosition, playerConfig->padNo, 0);
        }
        playerIndex += 1;
        playerConfig += 1;
    }
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        Hu3DModelPosGet(lbl_1_bss_230[playerConfig->charNo].model, &characterPosition);
        characterPosition.y -= 70.0f;
        Hu3DModelPosSetV(characterModelObject->mdlId[playerConfig->charNo], &characterPosition);
        Hu3DModelScaleSet(characterModelObject->mdlId[playerConfig->charNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(characterModelObject->mdlId[playerConfig->charNo], HU3D_ATTR_DISPOFF);
        playerIndex += 1;
        playerConfig += 1;
    }
    characterModelObject->work[0] = 0;
    characterModelObject->work[1] = 15;
    characterModelObject->objFunc = fn_1_66F4;
    /* The controller sleeps the object manager between particle phases; character scale updates
     * wait until this sleep ends. */
    HuPrcSleep(15);
    HuAudFXPlay(MSM_SE_MENU_18);
    playerIndex = 0;
    playerConfig = &lbl_1_bss_7CC[playerIndex];
    while (playerIndex < playerCount) {
        Hu3DModelPosGet(lbl_1_bss_230[playerConfig->charNo].model, &characterPosition);
        if (playerConfig->comF != 0) {
            fn_1_25EF0(playerIndex, &characterPosition, 4, 1);
        } else {
            fn_1_25EF0(playerIndex, &characterPosition, playerConfig->padNo, 1);
        }
        playerIndex += 1;
        playerConfig += 1;
    }
}

/* Creates the character models and paired looping motions used by the mode menu. */
void fn_1_6B74(OMOBJ *characterObject)
{
    s16 characterIndex;

    omSetStatBit(characterObject, OM_STAT_MODELPAUSE);
    characterIndex = 0;
    while (characterIndex < 11) {
        characterObject->mdlId[characterIndex] = Hu3DModelCreate(HuDataSelHeapReadNum(
            characterIndex + DATANUM(DATA_mdmini, 54), HU_MEMNUM_OVL, HEAP_MODEL));
        characterObject->mtnId[characterIndex * 2] =
            Hu3DJointMotion(characterObject->mdlId[characterIndex],
                            HuDataSelHeapReadNum(characterIndex + DATANUM(DATA_mdmini, 65),
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        characterObject->mtnId[(characterIndex * 2) + 1] =
            Hu3DJointMotion(characterObject->mdlId[characterIndex],
                            HuDataSelHeapReadNum(characterIndex + DATANUM(DATA_mdmini, 76),
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(characterObject->mdlId[characterIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(characterObject->mdlId[characterIndex], 1);
        Hu3DMotionShiftSet(characterObject->mdlId[characterIndex],
                           characterObject->mtnId[characterIndex * 2], 0.0f, 0.0f,
                           HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(characterObject->mdlId[characterIndex]);
        characterIndex += 1;
    }
    characterObject->objFunc = NULL;
}

/* Releases the character-preview object's models and paired motions when it is removed. */
void fn_1_6D14(OMOBJ *characterObject)
{
    s16 characterIndex;

    if (characterObject) {
        characterIndex = 0;
        while (characterIndex < 11) {
            Hu3DMotionKill(characterObject->mtnId[characterIndex * 2]);
            Hu3DMotionKill(characterObject->mtnId[(characterIndex * 2) + 1]);
            Hu3DModelKill(characterObject->mdlId[characterIndex]);
            characterIndex += 1;
        }
        omDelObjEx(lbl_1_bss_4, characterObject);
    }
    characterObject = NULL;
}

/* Fades the initial menu sprites out over ten frames after fn_1_7014 starts the transition. */
void fn_1_6DC4(OMOBJ *menuObject)
{
    float opacity = fn_1_156C(1.0f, 0.0f, menuObject->work[0], 10.0f);

    HuSprGrpTPLvlSet(lbl_1_bss_752[0], opacity);
    if (++menuObject->work[0] > 10) {
        fn_1_4438(lbl_1_bss_752[0], HUSPR_ATTR_DISPOFF);
        menuObject->objFunc = NULL;
    }
}

/* Restores the initial menu sprite group during menu transitions when the entry state is zero. */
void fn_1_6F58(void)
{
    s16 memberIndex;
    HUSPR_GROUP *spriteGroup;
    HUSPR_GROUPID spriteGroupId;

    if (lbl_1_bss_0 == 0) {
        HuSprGrpPosSet(lbl_1_bss_752[0], 288.0f, 120.0f);
        spriteGroupId = lbl_1_bss_752[0];
        spriteGroup = &HuSprGrpData[spriteGroupId];
        memberIndex = 0;
        while (memberIndex < spriteGroup->sprNum) {
            HuSprAttrReset(spriteGroupId, memberIndex, HUSPR_ATTR_DISPOFF);
            memberIndex += 1;
        }
    }
}

/* Starts the initial menu sprite group fade when the menu advances to another selection step. */
void fn_1_7014(void)
{
    OMOBJ *menuObject;
    menuObject = lbl_1_bss_8;
    menuObject->work[0] = 0;
    menuObject->objFunc = fn_1_6DC4;
}

/* The initialization callback creates three menu models; it shows the opening sprite group only
 * on a fresh entry. */
void fn_1_7048(OMOBJ *menuObject)
{
    s16 modelIndex;
    s16 memberIndex;
    HUSPR_GROUP *spriteGroup;
    s16 spriteGroupId;

    omSetStatBit(menuObject, OM_STAT_MODELPAUSE);
    modelIndex = 0;
    while (modelIndex < 3) {
        menuObject->mdlId[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelIndex + DATANUM(DATA_mdmini, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        menuObject->mtnId[modelIndex] = Hu3DMotionIDGet(menuObject->mdlId[modelIndex]);
        Hu3DModelLayerSet(menuObject->mdlId[modelIndex], 1);
        Hu3DMotionShiftSet(menuObject->mdlId[modelIndex], menuObject->mtnId[modelIndex], 0.0f, 0.0f,
                           HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(menuObject->mdlId[modelIndex]);
        modelIndex += 1;
    }
    if (lbl_1_bss_0 == 0) {
        HuSprGrpPosSet(lbl_1_bss_752[0], 288.0f, 120.0f);
        spriteGroupId = lbl_1_bss_752[0];
        spriteGroup = &HuSprGrpData[spriteGroupId];
        memberIndex = 0;
        while (memberIndex < spriteGroup->sprNum) {
            HuSprAttrReset(spriteGroupId, memberIndex, HUSPR_ATTR_DISPOFF);
            memberIndex += 1;
        }
    }
    menuObject->objFunc = NULL;
}

/* Releases the three initial menu models and their motions during menu teardown. */
void fn_1_71E8(OMOBJ *menuObject)
{
    s16 modelIndex;

    if (menuObject) {
        menuObject->objFunc = NULL;
        modelIndex = 0;
        while (modelIndex < 3) {
            Hu3DMotionKill(menuObject->mtnId[modelIndex]);
            Hu3DModelKill(menuObject->mdlId[modelIndex]);
            modelIndex += 1;
        }
        omDelObjEx(lbl_1_bss_4, menuObject);
    }
    menuObject = NULL;
}

/* Shows the selected mode label and starts its ten-frame fade-in during mode selection. */
void fn_1_7280(s16 modeIndex)
{
    OMOBJ *displayObject = lbl_1_bss_C;

    HuSprGrpPosSet(lbl_1_bss_752[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], 0.0f);
    fn_1_4438(lbl_1_bss_752[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(
        lbl_1_bss_752[1], modeIndex, HUSPR_ATTR_DISPOFF);
    displayObject->work[2] = 1;
    displayObject->work[3] = 0;
}

/* Starts the mode label fade-out when a menu transition dismisses the current mode. */
void fn_1_7384(void)
{
    OMOBJ *displayObject;
    displayObject = lbl_1_bss_C;
    displayObject->work[2] = 2;
    displayObject->work[3] = 0;
}

/* Hides the mode label immediately when the menu resets its display. */
void fn_1_73B4(void)
{
    HuSprGrpPosSet(lbl_1_bss_752[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], 0.0f);
}

/* Updates the mode label fade each frame from the animated-display callback. */
void fn_1_7414(OMOBJ *displayObject)
{
    f32 opacity;

    switch ((s32)displayObject->work[2]) {
    case 1:
        opacity = fn_1_156C(0.0f, 1.0f, (f32)displayObject->work[3], 10.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_752[1], opacity);
        if (++displayObject->work[3] > 10U) {
            displayObject->work[2] = 0;
        }
        break;
    case 2:
        opacity = fn_1_156C(1.0f, 0.0f, (f32)displayObject->work[3], 10.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_752[1], opacity);
        if (++displayObject->work[3] > 10U) {
            displayObject->work[2] = 0;
        }
        break;
    }
}

/* Changes the second display model to the selected mode and starts fading the first model out. */
void fn_1_765C(s16 modeIndex)
{
    OMOBJ *displayObject;
    displayObject = lbl_1_bss_C;
    Hu3DAnimAnimSet(displayObject->mtnId[2], lbl_1_bss_764[modeIndex * 2 + 3]);
    Hu3DAnimAnimSet(displayObject->mtnId[3], lbl_1_bss_764[modeIndex * 2 + 4]);
    displayObject->work[0] = 1;
    displayObject->work[1] = 0;
}

/* Starts fading the first display model in and the mode label out during a menu transition. */
void fn_1_7700(void)
{
    OMOBJ *modelFadeObject;
    OMOBJ *labelFadeObject;
    modelFadeObject = lbl_1_bss_C;
    modelFadeObject->work[0] = 2;
    modelFadeObject->work[1] = 0;
    labelFadeObject = lbl_1_bss_C;
    labelFadeObject->work[2] = 2;
    labelFadeObject->work[3] = 0;
}

/* Rotates both display models and updates their model and label fades each frame. */
void fn_1_7754(OMOBJ *displayObject)
{
    s16 modelIndex;
    f32 opacity;

    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        Point3d rotation;
        Hu3DModelRotGet(displayObject->mdlId[modelIndex], &rotation);
        rotation.z -= 0.1f;
        if (rotation.z < 0.0f) {
            rotation.z += 360.0f;
        }
        Hu3DModelRotSetV(displayObject->mdlId[modelIndex], &rotation);
    }
    switch ((s32)displayObject->work[0]) {
    case 1:
        opacity = fn_1_156C(1.0f, 0.0f, (f32)displayObject->work[1], 30.0f);
        Hu3DModelTPLvlSet(*displayObject->mdlId, opacity);
        if (++displayObject->work[1] > 30U) {
            displayObject->work[0] = 0;
        }
        break;
    case 2:
        opacity = fn_1_156C(0.0f, 1.0f, (f32)displayObject->work[1], 30.0f);
        Hu3DModelTPLvlSet(*displayObject->mdlId, opacity);
        if (++displayObject->work[1] > 30U) {
            displayObject->work[0] = 0;
        }
        break;
    }
    fn_1_7414(displayObject);
}

/* Builds the mode's two-model animated display during object initialization. */
void fn_1_7C4C(OMOBJ *object)
{
    s16 modelIndex;

    omSetStatBit(object, OM_STAT_MODELPAUSE);
    modelIndex = 0;
    while (modelIndex < 2) {
        if (modelIndex == 0) {
            object->mdlId[modelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 3), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            object->mdlId[modelIndex] = Hu3DModelLink(*object->mdlId);
        }
        Hu3DModelPosSet(object->mdlId[modelIndex], 0.0f, -100.0f,
                        -850.0f - (f32) modelIndex);
        Hu3DModelRotSet(object->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(object->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(object->mdlId[modelIndex], 0);
        object->mtnId[modelIndex * 2] =
            Hu3DAnimCreate(lbl_1_bss_764[1], object->mdlId[modelIndex], lbl_1_data_860);
        object->mtnId[(modelIndex * 2) + 1] =
            Hu3DAnimCreate(lbl_1_bss_764[2], object->mdlId[modelIndex], lbl_1_data_867);
        modelIndex += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_752[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], 0.0f);
    object->objFunc = fn_1_7754;
}

/* Releases the two animated display models and their texture animations during menu teardown. */
void fn_1_7E90(OMOBJ *displayObject)
{
    s16 modelIndex;

    if (displayObject) {
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            Hu3DAnimKill(displayObject->mtnId[2 * modelIndex]);
            Hu3DAnimKill(displayObject->mtnId[(2 * modelIndex) + 1]);
            displayObject->mtnId[2 * modelIndex] = -1;
            displayObject->mtnId[(2 * modelIndex) + 1] = -1;
        }
        for (modelIndex = 1; modelIndex >= 0; modelIndex--) {
            Hu3DModelKill(displayObject->mdlId[modelIndex]);
        }
        omDelObjEx(lbl_1_bss_4, displayObject);
    }
    displayObject = NULL;
}

/* Resets a player's menu sprite group and selects its requested bank. */
void fn_1_7F8C(s16 playerIndex, s16 bankIndex) {
    s16 memberIndex;
    HUSPR_GROUP *spriteGroup;
    HUSPR_GROUPID spriteGroupId;

    lbl_1_bss_80[playerIndex].x = 0.85f;
    lbl_1_bss_80[playerIndex].y = 0.85f;
    spriteGroupId = lbl_1_bss_752[playerIndex + 4];
    spriteGroup = &HuSprGrpData[spriteGroupId];
    memberIndex = 0;
    while (memberIndex < spriteGroup->sprNum) {
        HuSprAttrReset(spriteGroupId, memberIndex, HUSPR_ATTR_DISPOFF);
        memberIndex += 1;
    }
    HuSprBankSet(lbl_1_bss_752[playerIndex + 4], 0, bankIndex);
}

/* Shows a player confirmation sprite and sets its target size when a character is accepted. */
void fn_1_809C(s16 playerIndex, s16 bankIndex)
{
    lbl_1_bss_80[playerIndex].x = 0.8500000238418579f;
    fn_1_44B8(lbl_1_bss_752[playerIndex + 4], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_752[playerIndex + 4], 0, bankIndex);
}

/* Starts shrinking a player confirmation sprite when its character choice is cancelled. */
void fn_1_8188(s16 playerIndex)
{
    lbl_1_bss_80[playerIndex].x = 0.0f;
}

/* Changes the player confirmation sprite bank when the CPU difficulty selection changes. */
void fn_1_81B0(s16 playerIndex, s16 bankIndex)
{
    HuSprBankSet(lbl_1_bss_752[playerIndex + 4], 0, bankIndex);
}

/* Each frame, follows selected character models with player indicators and eases their displayed
 * sizes. */
void fn_1_81FC(OMOBJ *indicatorObject)
{
    Point3d characterPosition;
    Point3d screenPosition;
    MDMinidllPlayerConfig *playerConfigCursor;
    s16 playerIndex;

    playerIndex = 0;
    playerConfigCursor = lbl_1_bss_7CC;
    while (playerIndex < 4) {
        Hu3DModelPosGet(lbl_1_bss_230[lbl_1_bss_7CC[playerIndex].charNo].model, &characterPosition);
        Hu3D3Dto2D(&characterPosition, 1, &screenPosition);
        HuSprGrpPosSet(lbl_1_bss_752[playerIndex + 4], screenPosition.x - 18.0f,
                       (f32) (s16) (screenPosition.y - 23.0f));
        HuSprAttrReset(lbl_1_bss_752[playerIndex + 4], 0, HUSPR_ATTR_LINEAR);
        lbl_1_bss_80[playerIndex].y =
            fn_1_1310(lbl_1_bss_80[playerIndex].y, lbl_1_bss_80[playerIndex].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_752[playerIndex + 4], lbl_1_bss_80[playerIndex].y,
                         lbl_1_bss_80[playerIndex].y);
        playerIndex += 1;
        playerConfigCursor += 1;
    }
}

/* Initializes player-indicator scale state and installs its per-frame character-following
 * callback. */
void fn_1_8430(OMOBJ *indicatorObject)
{
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_80[playerIndex].x = 0.0f;
        lbl_1_bss_80[playerIndex].y = 0.0f;
        lbl_1_bss_80[playerIndex].z = 0.0f;
        playerIndex += 1;
    }
    indicatorObject->objFunc = fn_1_81FC;
}

void fn_1_84D4(void)
{
}

static const Point3d lbl_1_rodata_1B0 = {0.0f, 0.0f, 0.0f};

/* Positions a menu arrow relative to camera 0 for the arrow helpers; a null position uses the
 * screen origin. */
void fn_1_84D8(s16 memberIndex, Point3d *worldPosition, f32 xOffset, f32 yOffset)
{
    Point3d screenPosition;

    screenPosition = lbl_1_rodata_1B0;
    if (worldPosition) {
        Hu3D3Dto2D(worldPosition, 1, &screenPosition);
    }
    HuSprPosSet(lbl_1_bss_752[2], memberIndex, screenPosition.x + xOffset,
                screenPosition.y + yOffset);
}

/* Shows a navigation arrow at the requested position and starts its scale pulse during a selection
 * step. */
void fn_1_8570(
    s16 memberIndex, HuVecF *worldPosition, float xOffset, float yOffset)
{
    fn_1_84D8(memberIndex, worldPosition, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_752[2], memberIndex, 2.0f, 2.0f);
    HuSprAttrReset(lbl_1_bss_752[2], memberIndex, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_38[memberIndex] = 2.0f;
    lbl_1_data_86E[memberIndex] = 1;
}

/* Hides one navigation arrow and clears its activity flag when its selection step no longer needs
 * it. */
void fn_1_8694(s16 memberIndex)
{
    HuSprAttrSet(lbl_1_bss_752[2], memberIndex, HUSPR_ATTR_DISPOFF);
    lbl_1_data_86E[memberIndex] = 0;
}

/* Pulses a navigation arrow after a move; the move sound plays only while that arrow is active. */
void fn_1_86F0(s16 memberIndex)
{
    lbl_1_bss_38[memberIndex] = 2.0f;
    if (lbl_1_data_86E[memberIndex] == 1) {
        HuAudFXPlay(MSM_SE_CMN_01);
    }
}

/* Eases the six navigation arrows back to normal size during each selection-marker update. */
void fn_1_8764(void)
{
    s16 memberIndex;

    memberIndex = 0;
    while (memberIndex < 6) {
        lbl_1_bss_38[memberIndex] = fn_1_1310(lbl_1_bss_38[memberIndex], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_752[2], memberIndex, lbl_1_bss_38[memberIndex],
                      lbl_1_bss_38[memberIndex]);
        memberIndex += 1;
    }
}

/* Hides all six navigation arrows and clears their activity flags when the selection step ends. */
void fn_1_8888(void)
{
    s16 memberIndex;
    memberIndex = 0;
    while (memberIndex < 6) {
        HuSprAttrSet(lbl_1_bss_752[2], memberIndex, HUSPR_ATTR_DISPOFF);
        lbl_1_data_86E[memberIndex] = 0;
        /* The same activity flag is cleared again. */
        lbl_1_data_86E[memberIndex] = 0;
        memberIndex += 1;
    }
}

/* Sets the selection marker destination with fixed offsets from the character after a cursor
 * move. */
void fn_1_8914(s16 playerIndex, const Point3d *characterPosition)
{
    lbl_1_bss_50[playerIndex].x = characterPosition->x - 50.0f;
    lbl_1_bss_50[playerIndex].y = 50.0f + characterPosition->y;
    lbl_1_bss_50[playerIndex].z = 50.0f + characterPosition->z;
}

/* Restores a player selection marker and its pad or difficulty sprite when the character choice
 * becomes editable. */
void fn_1_899C(s32 playerIndex, Point3d *characterPosition, s16 bankIndex)
{
    OMOBJ *markerObject = lbl_1_bss_1C[0];

    Hu3DModelPosSet(markerObject->mdlId[(s16) playerIndex], characterPosition->x,
                    characterPosition->y, characterPosition->z);
    Hu3DModelRotSet(markerObject->mdlId[(s16)playerIndex], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(markerObject->mdlId[(s16)playerIndex], 1.0f, 1.0f, 1.0f);

    lbl_1_bss_50[(s16)playerIndex].x = characterPosition->x - 50.0f;
    lbl_1_bss_50[(s16)playerIndex].y = 50.0f + characterPosition->y;
    lbl_1_bss_50[(s16)playerIndex].z = 50.0f + characterPosition->z;

    Hu3DModelAttrReset(markerObject->mdlId[(s16)playerIndex], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_752[3], (s16)playerIndex, 0.8f, 0.8f);
    HuSprAttrReset(lbl_1_bss_752[3], (s16)playerIndex, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_752[3], (s16)playerIndex, bankIndex);
}

/* Hides a player selection marker and its sprite after the character choice is accepted. */
void fn_1_8B70(s32 playerIndex)
{
    OMOBJ *markerObject;

    markerObject = lbl_1_bss_1C[0];
    Hu3DModelAttrSet(markerObject->mdlId[(s16)playerIndex], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_752[3], (s16)playerIndex, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_752[3], (s16)playerIndex, HUSPR_ATTR_DISPOFF);
}

typedef struct {
    /* Three neighbor directions to try for each of 12 input classifications. */
    s16 directionPriorities[12][3];
} MDMinNeighborTable;
typedef struct _struct_lbl_1_data_89A_0x12 {
    s16 neighborCharacter[8]; /* Linked character index by direction; -1 means no link. */
    s16 selectionState; /* -1 for an available character; other values reserve it. */
} LBL_1_DATA_E12_ENTRY;

/* Each frame, eases selection markers toward their target poses, aligns their sprites and updates
 * arrow pulses. */
void fn_1_8C08(OMOBJ *markerObject)
{
    HuVecF modelTransform;
    HuVecF screenPosition;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelPosGet(markerObject->mdlId[playerIndex], &modelTransform);
        fn_1_1340(&modelTransform, &lbl_1_bss_50[playerIndex], 3.0f);
        Hu3DModelPosSetV(markerObject->mdlId[playerIndex], &modelTransform);
        Hu3D3Dto2D(&modelTransform, 1, &screenPosition);
        HuSprPosSet(lbl_1_bss_752[3], (s16)playerIndex, screenPosition.x, screenPosition.y);
        HuSprScaleSet(lbl_1_bss_752[3], (s16)playerIndex, 0.8f, 0.8f);

        Hu3DModelRotGet(markerObject->mdlId[playerIndex], &modelTransform);
        modelTransform.z = fn_1_1310(modelTransform.z, 0.0f, 3.0f);
        Hu3DModelRotSetV(markerObject->mdlId[playerIndex], &modelTransform);

        Hu3DModelScaleGet(markerObject->mdlId[playerIndex], &modelTransform);
        modelTransform.x = modelTransform.y = modelTransform.z =
            fn_1_1310(modelTransform.x, 0.4f, 3.0f);
        Hu3DModelScaleSetV(markerObject->mdlId[playerIndex], &modelTransform);
    }
    fn_1_8764();
}

/* Creates and hides the four player selection markers, then installs their per-frame update during
 * menu setup. */
void fn_1_90C8(OMOBJ *markerObject)
{
    s16 playerIndex;
    s16 arrowIndex;
    OMOBJ *registeredMarkerObject;
    s16 memberIndex;

    omSetStatBit(markerObject, OM_STAT_MODELPAUSE);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (playerIndex == 0) {
            markerObject->mdlId[playerIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 20), HU_MEMNUM_OVL, HEAP_MODEL));
            markerObject->mtnId[playerIndex] = Hu3DMotionIDGet(*markerObject->mdlId);
        } else {
            markerObject->mdlId[playerIndex] = Hu3DModelLink(*markerObject->mdlId);
        }
        Hu3DModelLayerSet(markerObject->mdlId[playerIndex], 1);
        Hu3DMotionShiftSet(markerObject->mdlId[playerIndex], *markerObject->mtnId, 0.0f, 0.0f,
                           HU3D_MOTATTR_LOOP);
        registeredMarkerObject = lbl_1_bss_1C[0];
        Hu3DModelAttrSet(registeredMarkerObject->mdlId[playerIndex], HU3D_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_752[3], (s16) playerIndex, 0.8f, 0.8f);
        HuSprAttrSet(lbl_1_bss_752[3], (s16) playerIndex, HUSPR_ATTR_DISPOFF);
        playerIndex += 1;
    }
    arrowIndex = 0;
    while (arrowIndex < 6) {
        memberIndex = arrowIndex;
        HuSprAttrSet(lbl_1_bss_752[2], memberIndex, HUSPR_ATTR_DISPOFF);
        lbl_1_data_86E[memberIndex] = 0;
        /* The second write repeats the clear of this arrow's activity flag. */
        lbl_1_data_86E[arrowIndex] = 0;
        arrowIndex += 1;
    }
    markerObject->objFunc = fn_1_8C08;
}

/* Releases the shared marker motion and four selection-marker models during menu teardown. */
void fn_1_92C0(OMOBJ *markerObject)
{
    s16 modelIndex;
    if (markerObject) {
        Hu3DMotionKill(*markerObject->mtnId);
        modelIndex = 3;
        while (modelIndex >= 0) {
            Hu3DModelKill(markerObject->mdlId[modelIndex]);
            modelIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_4, markerObject);
    }
    /* Clearing this local leaves the caller's stored pointer unchanged. */
    markerObject = NULL;
}

/* Finds an unoccupied character for fn_1_9730 by following neighbor links and trying fallback
 * directions. */
s16 fn_1_9348(s16 currentCharacter, s16 direction, s16 fallbackDirection, s16 lastFallbackDirection,
              LBL_1_DATA_E12_ENTRY *selectionEntries, s16 characterCount)
{
    s16 nextCharacter;
    s16 characterIndex;

    if (direction == -1) {
        return -1;
    }
    nextCharacter = ((s16 (*)[9])selectionEntries)[currentCharacter][direction];
    if (nextCharacter == -1) {
        nextCharacter = fn_1_9348(currentCharacter, fallbackDirection, lastFallbackDirection, -1,
                                  selectionEntries, characterCount);
    }
    characterIndex = 0;
    while (characterIndex < characterCount) {
        if ((currentCharacter != characterIndex) &&
            (((s16(*)[9]) selectionEntries)[characterIndex][8] != -1) &&
            (nextCharacter == characterIndex)) {
            nextCharacter = fn_1_9348(nextCharacter, direction, fallbackDirection,
                                      lastFallbackDirection, selectionEntries, characterCount);
            if (nextCharacter == -1) {
                nextCharacter =
                    fn_1_9348(currentCharacter, fallbackDirection, lastFallbackDirection, -1,
                              selectionEntries, characterCount);
            }
        }
        characterIndex += 1;
    }
    return nextCharacter;
}

static const MDMinNeighborTable lbl_1_rodata_1CC = {
    {{2,1,3},{6,7,5},{0,7,1},{4,5,3},{1,2,0},{3,2,4},{7,6,0},{5,6,4},
     {1,0,2},{7,0,6},{3,4,2},{5,4,6}}
};

/* Chooses the neighbor-direction priorities for the input class before character navigation
 * searches for a free slot. */
s16 fn_1_9730(s16 currentCharacter, LBL_1_DATA_E12_ENTRY *selectionEntries,
              s16 directionClass, s16 characterCount)
{
    MDMinNeighborTable neighbors = lbl_1_rodata_1CC;
    return fn_1_9348(currentCharacter, neighbors.directionPriorities[directionClass][0],
                     neighbors.directionPriorities[directionClass][1],
                     neighbors.directionPriorities[directionClass][2], selectionEntries,
                     characterCount);
}

/* Moves the character cursor from repeated directional input, skipping occupied characters and
 * updating both selection records. */
s16 fn_1_97D8(s16 padNo, s16 *currentCharacter,
    LBL_1_DATA_E12_ENTRY *selectionEntries, s16 characterCount)
{
    s16 nextCharacter = 0;
    s16 directionClass = 0;
    HuVecF inputDirection = { 0.0f, 0.0f, 0.0f };

    if (HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
        inputDirection.y = 1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
        inputDirection.y = -1.0f;
    }
    if (HuPadDStkRep[padNo] & PAD_BUTTON_LEFT) {
        inputDirection.x = -1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_RIGHT) {
        inputDirection.x = 1.0f;
    }
    if (HuAbs(inputDirection.x) < 0.25f
        && HuAbs(inputDirection.y) < 0.25f) {
        return FALSE;
    }

    inputDirection.z = sqrtf(
        (inputDirection.x * inputDirection.x) + (inputDirection.y * inputDirection.y));
    inputDirection.x /= inputDirection.z;
    inputDirection.y /= inputDirection.z;
    if (HuAbs(inputDirection.y) < 0.25f) {
        if (inputDirection.x > 0.0f) {
            directionClass = 0;
        } else {
            directionClass = 1;
        }
    } else if (HuAbs(inputDirection.x) < 0.25f) {
        if (inputDirection.y > 0.0f) {
            directionClass = 2;
        } else {
            directionClass = 3;
        }
    } else if (HuAbs(inputDirection.x) > HuAbs(inputDirection.y)) {
        if (inputDirection.x > 0.0f) {
            if (inputDirection.y > 0.0f) {
                directionClass = 4;
            } else {
                directionClass = 5;
            }
        } else if (inputDirection.y > 0.0f) {
            directionClass = 6;
        } else {
            directionClass = 7;
        }
    } else if (inputDirection.y > 0.0f) {
        if (inputDirection.x > 0.0f) {
            directionClass = 8;
        } else {
            directionClass = 9;
        }
    } else if (inputDirection.x > 0.0f) {
        directionClass = 10;
    } else {
        directionClass = 11;
    }

    if ((nextCharacter = fn_1_9730(*currentCharacter, selectionEntries, directionClass,
                                   characterCount)) != -1) {
        selectionEntries[*currentCharacter].selectionState = -1;
        *currentCharacter = nextCharacter;
        selectionEntries[*currentCharacter].selectionState = 1;
        HuAudFXPlay(MSM_SE_CMN_01);
        return TRUE;
    }
    return FALSE;
}

/* Character-selection callbacks replace one texture animation here; bank -1 leaves the new
 * animation at bank zero. */
static inline void fn_1_A3A8_inlined(s16 modelIndex, s16 animationSlot, s16 animationDataIndex,
                                     s16 bankIndex)
{
    MDMinidllModelAnimationRecord *modelRecord = &lbl_1_bss_230[modelIndex];

    Hu3DAnimAnimSet(modelRecord->animation[animationSlot], lbl_1_bss_6C8[animationDataIndex]);
    if (bankIndex != -1) {
        Hu3DAnimBankSet(modelRecord->animation[animationSlot], bankIndex);
    }
}

/* Replaces a selection model texture animation during menu updates; bank -1 leaves the new
 * animation at bank zero. */
void fn_1_9D58(s16 modelIndex, s16 animationSlot, s16 animationDataIndex, s16 bankIndex)
{
    MDMinidllModelAnimationRecord *modelRecord = &lbl_1_bss_230[modelIndex];

    Hu3DAnimAnimSet(modelRecord->animation[animationSlot], lbl_1_bss_6C8[animationDataIndex]);
    if (bankIndex != -1) {
        Hu3DAnimBankSet(modelRecord->animation[animationSlot], bankIndex);
    }
}

/* Changes all four texture animations on a selection card during character or mode changes. */
void fn_1_9E00(s16 modelIndex, s16 animationDataIndex)
{
    s16 animationSlot;

    for (animationSlot = 0; animationSlot < 4; animationSlot++) {
        fn_1_9D58(modelIndex, animationSlot, animationDataIndex, -1);
    }
}

/* Menu placement helper that reveals a selection model at a position with x/y scaling and unit
 * depth. */
void fn_1_9E8C(s16 modelIndex, Point3d *position, f32 scale)
{
    MDMinidllModelAnimationRecord *modelRecord;

    modelRecord = &lbl_1_bss_230[modelIndex];
    Hu3DModelPosSet(modelRecord->model, position->x, position->y, position->z);
    Hu3DModelScaleSet(modelRecord->model, scale, scale, 1.0f);
    Hu3DModelAttrReset(modelRecord->model, HU3D_ATTR_DISPOFF);
}

/* Spins and scales a selection model into view during a timed menu entrance; models started
 * together share one sound. */
void fn_1_9F24(
    s16 modelIndex, s16 elapsedFrames, s16 durationFrames, HuVecF *position, float scale)
{
    MDMinidllModelAnimationRecord *modelRecord = &lbl_1_bss_230[modelIndex];
    float transformValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosSet(modelRecord->model, position->x, position->y, position->z);
        Hu3DModelRotSet(modelRecord->model, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(modelRecord->model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(modelRecord->model, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_34 = 0;
    }
    transformValue = fn_1_1868_inlined(-540.0f, 0.0f, elapsedFrames, durationFrames);
    Hu3DModelRotSet(modelRecord->model, 0.0f, transformValue, 0.0f);
    transformValue = fn_1_1868_inlined(0.0f, scale, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(modelRecord->model, transformValue, transformValue, 1.0f);
}

/* Spins and shrinks a selection model for a timed menu exit, using the pose saved on frame zero. */
void fn_1_A330(s16 modelIndex, s16 elapsedFrames, s16 durationFrames)
{
    MDMinidllModelAnimationRecord *modelState;
    f32 transformValue;

    modelState = &lbl_1_bss_230[modelIndex];
    if (elapsedFrames == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosGet(modelState->model, &modelState->savedPosition);
        Hu3DModelRotGet(modelState->model, &modelState->savedRotation);
        Hu3DModelScaleGet(modelState->model, &modelState->savedScale);
    } else {
        lbl_1_bss_34 = 0;
    }
    transformValue =
        fn_1_1868_inlined(modelState->savedRotation.y, modelState->savedRotation.y - 540.0f,
                          (f32) elapsedFrames, (f32) durationFrames);
    Hu3DModelRotSet(modelState->model, 0.0f, transformValue, 0.0f);
    transformValue = fn_1_1868_inlined(modelState->savedScale.x, 0.0f, (f32) elapsedFrames,
                                       (f32) durationFrames);
    Hu3DModelScaleSet(modelState->model, transformValue, transformValue, 1.0f);
    if (elapsedFrames <= durationFrames) {
        Hu3DModelAttrReset(modelState->model, HU3D_ATTR_DISPOFF);
    }
}

/* Moves and resizes a selection model from its frame-zero pose during menu transitions; depth scale
 * is forced to one. */
void fn_1_A700(s16 modelIndex, s16 elapsedFrames, s16 durationFrames, Point3d *targetPosition,
               f32 targetScale)
{
    Point3d position;
    MDMinidllModelAnimationRecord *model;
    f32 scale;

    model = &lbl_1_bss_230[modelIndex];
    if (elapsedFrames == 0) {
        Hu3DModelPosGet(model->model, &model->savedPosition);
        Hu3DModelRotGet(model->model, &model->savedRotation);
        Hu3DModelScaleGet(model->model, &model->savedScale);
    }
    position.x = fn_1_1868_inlined(model->savedPosition.x, targetPosition->x, (f32) elapsedFrames,
                                   (f32) durationFrames);
    position.y = fn_1_1868_inlined(model->savedPosition.y, targetPosition->y, (f32) elapsedFrames,
                                   (f32) durationFrames);
    position.z = fn_1_1868_inlined(model->savedPosition.z, targetPosition->z, (f32) elapsedFrames,
                                   (f32) durationFrames);
    Hu3DModelPosSet(model->model, position.x, position.y, position.z);
    scale = fn_1_1868_inlined(model->savedScale.x, targetScale, (f32) elapsedFrames,
                              (f32) durationFrames);
    Hu3DModelScaleSet(model->model, scale, scale, 1.0f);
}

/* Selection callbacks replace one texture animation here; bank -1 leaves the new animation at bank
 * zero. */
static inline void fn_1_C158_inlined(s16 modelIndex, s16 animationSlot, s16 animationDataIndex,
                                     s16 bankIndex)
{
    MDMinidllModelAnimationRecord *modelRecord = &lbl_1_bss_230[modelIndex];

    Hu3DAnimAnimSet(modelRecord->animation[animationSlot], lbl_1_bss_6C8[animationDataIndex]);
    if (bankIndex != -1) {
        Hu3DAnimBankSet(modelRecord->animation[animationSlot], bankIndex);
    }
}

/* Selection callbacks change all four texture animations on a card with the same animation data. */
static inline void fn_1_C200_inlined(s16 modelIndex, s16 animationDataIndex)
{
    s16 animationSlot;

    for (animationSlot = 0; animationSlot < 4; animationSlot++) {
        fn_1_C158_inlined(modelIndex, animationSlot, animationDataIndex, -1);
    }
}

/* Flips a player-count card, changes its animation halfway through and resets yaw at the end of the
 * count-change animation. */
void fn_1_ACF0(s16 modelIndex, s32 animationOffset, s32 baseAnimationIndex, s16 elapsedFrames,
               s16 durationFrames, s16 turnDirection)
{
    Point3d rotation;
    MDMinidllModelAnimationRecord *modelRecord;
    f32 absoluteYaw;

    modelRecord = &lbl_1_bss_230[(s16)modelIndex];
    if (((s16)elapsedFrames == 0) || ((s16)elapsedFrames >= (s16)durationFrames)) {
        Hu3DModelRotSet(modelRecord->model, 0.0f, 0.0f, 0.0f);
    }
    if ((s16)elapsedFrames >= (s16)durationFrames) {
        fn_1_9E00((s16)modelIndex, (s16)(baseAnimationIndex + animationOffset));
        return;
    }
    Hu3DModelRotGet(modelRecord->model, &rotation);
    rotation.y = fn_1_156C(0.0f, 180.0f * (f32) (s16) turnDirection, (f32) (s16) elapsedFrames,
                           (f32) (s16) durationFrames);
    if (rotation.y < 0.0f) {
        absoluteYaw = -rotation.y;
    } else {
        absoluteYaw = rotation.y;
    }
    if (absoluteYaw >= 90.0f) {
        fn_1_9E00((s16)modelIndex, (s16)(baseAnimationIndex + animationOffset));
    }
    Hu3DModelRotSetV(modelRecord->model, &rotation);
}

/* Turns a mode card toward zero yaw during a menu transition; the supplied yaw only chooses
 * wrapping of the starting angle. */
void fn_1_B078(s16 modelIndex, float referenceYaw, s16 elapsedFrames, s16 durationFrames)
{
    MDMinidllModelAnimationRecord *modelRecord = &lbl_1_bss_230[modelIndex];
    float yaw;

    if (elapsedFrames == 0) {
        Hu3DModelRotGet(modelRecord->model, &modelRecord->savedRotation);
        if (modelRecord->savedRotation.y - referenceYaw > 180.0f) {
            modelRecord->savedRotation.y -= 360.0f;
        } else if (modelRecord->savedRotation.y - referenceYaw < -180.0f) {
            modelRecord->savedRotation.y += 360.0f;
        }
    }
    yaw = fn_1_1868_inlined(modelRecord->savedRotation.y, 0.0f, elapsedFrames,
                            durationFrames);
    Hu3DModelRotSet(modelRecord->model, 0.0f, yaw, 0.0f);
}

/* Stores the menu-controller completion state used by the selection process. */
void fn_1_B2E0(s16 completionState)
{
    *lbl_1_bss_24->mtnId = completionState;
}

/* Returns the menu-controller completion state to the selection process. */
s16 fn_1_B2F8(void)
{
    return *lbl_1_bss_24->mtnId;
}

/* Reports whether the current menu-controller step has reached ten update frames. */
s16 fn_1_B310(void)
{
    OMOBJ *controllerObject;
    controllerObject = lbl_1_bss_24;
    if (controllerObject->work[0] >= 10U) {
        return 1;
    }
    return 0;
}

/* Starts a menu-controller callback with cleared counters and waits until its first motion slot
 * signals completion. */
void fn_1_B348(void (*callback)(OMOBJ *))
{
    OMOBJ *controllerObject;
    controllerObject = lbl_1_bss_24;
    *lbl_1_bss_24->mtnId = 0;
    controllerObject->work[0] = 0;
    controllerObject->work[1] = 0;
    controllerObject->work[2] = 0;
    controllerObject->work[3] = 0;
    controllerObject->objFunc = callback;
    while (*lbl_1_bss_24->mtnId == 0) {
        HuPrcVSleep();
    }
}

typedef struct MDMinidllAnimationNameTable { char *name[4]; } MDMinidllAnimationNameTable;
static const MDMinidllAnimationNameTable lbl_1_rodata_238 = { { lbl_1_data_87A, lbl_1_data_882,
                                                                lbl_1_data_88A, lbl_1_data_892 } };

/* Menu initialization builds 21 selection cards with four texture animations each and allocates
 * their step controller. */
void fn_1_B3E0(void)
{
    MDMinidllAnimationNameTable textureNames;
    s16 modelIndex;
    s16 animationSlot;

    textureNames = lbl_1_rodata_238;
    modelIndex = 0;
    while (modelIndex < 21) {
        lbl_1_bss_6C8[modelIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_474[modelIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        modelIndex += 1;
    }

    for (modelIndex = 0; modelIndex < 21; modelIndex++) {
        memset(&lbl_1_bss_230[modelIndex], 0, sizeof(lbl_1_bss_230[modelIndex]));
        if (modelIndex == 0) {
            lbl_1_bss_230[modelIndex].model = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_mdmini, 25), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_230[modelIndex].model = Hu3DModelLink(lbl_1_bss_230[0].model);
        }

        animationSlot = 0;
        while (animationSlot < 4) {
            lbl_1_bss_230[modelIndex].animation[animationSlot] =
                Hu3DAnimCreate(lbl_1_bss_6C8[0], lbl_1_bss_230[modelIndex].model,
                               textureNames.name[animationSlot]);
            animationSlot += 1;
        }
        Hu3DModelLayerSet(lbl_1_bss_230[modelIndex].model, 1);
        Hu3DModelAttrSet(lbl_1_bss_230[modelIndex].model, HU3D_ATTR_DISPOFF);
    }

    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, NULL);
}

/* Releases all selection cards during menu teardown and clears the shared controller pointer. */
void fn_1_B610(void)
{
    s16 modelIndex;
    s16 animationSlot;

    if (lbl_1_bss_24) {
        *lbl_1_bss_24->mtnId = -1;
        for (modelIndex = 20; modelIndex >= 0; modelIndex--) {
            for (animationSlot = 0; animationSlot < 4; animationSlot++) {
                Hu3DAnimKill(lbl_1_bss_230[modelIndex].animation[animationSlot]);
            }
            Hu3DModelKill(lbl_1_bss_230[modelIndex].model);
        }
    }
    lbl_1_bss_24 = NULL;
}

/* Handles the human-player count each frame and flips the count cards; input wraps between one
* and the connected-controller count, with the upper limit forced to at least one. */
void fn_1_B6EC(OMOBJ *controllerObject)
{
    s16 minimumPlayers = 1;
    s16 connectedPlayers = 0;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (HuPadStatGet(playerIndex) == 0) {
            connectedPlayers++;
        }
    }
    if (connectedPlayers < minimumPlayers) {
        connectedPlayers = minimumPlayers;
    }
    if (connectedPlayers >= lbl_1_bss_804[0]) {
        if (controllerObject->work[0]++ > 10) {
            if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                lbl_1_bss_804[0]--;
                if (lbl_1_bss_804[0] < minimumPlayers) {
                    lbl_1_bss_804[0] = connectedPlayers;
                }
                controllerObject->work[0] = 0;
                controllerObject->work[1] = 0;
                fn_1_86F0(4);
            } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                lbl_1_bss_804[0]++;
                if (lbl_1_bss_804[0] >= connectedPlayers + 1) {
                    lbl_1_bss_804[0] = minimumPlayers;
                }
                controllerObject->work[0] = 0;
                controllerObject->work[1] = 1;
                fn_1_86F0(5);
            }
            if (connectedPlayers == minimumPlayers) {
                controllerObject->work[0] = 12;
                if (controllerObject->work[3] == 1) {
                    controllerObject->work[3] = 0;
                    fn_1_8694(4);
                    fn_1_8694(5);
                }
            } else if (controllerObject->work[3] == 0) {
                controllerObject->work[3] = 1;
                fn_1_8570(4, lbl_1_data_4C8, -180.0f, 0.0f);
                fn_1_8570(5, lbl_1_data_4C8, 180.0f, 0.0f);
            }
        }
    } else {
        lbl_1_bss_804[0] = connectedPlayers;
        controllerObject->work[0] = 0;
        controllerObject->work[1] = 0;
    }
    if (controllerObject->work[0] <= 10) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (controllerObject->work[1]) {
                if (playerIndex < lbl_1_bss_804[0]) {
                    fn_1_ACF0(playerIndex + 17, 1, 18,
                        controllerObject->work[0], 10, 1);
                } else {
                    fn_1_ACF0(playerIndex + 17, 0, 18,
                        controllerObject->work[0], 10, 1);
                }
            } else if (playerIndex < lbl_1_bss_804[0]) {
                fn_1_ACF0(playerIndex + 17, 1, 18,
                    controllerObject->work[0], 10, -1);
            } else {
                fn_1_ACF0(playerIndex + 17, 0, 18,
                    controllerObject->work[0], 10, -1);
            }
        }
    }
}

void fn_1_C6A8(OMOBJ *obj);
void fn_1_C6D8(OMOBJ *obj);
void fn_1_C770(OMOBJ *controllerObject);
void fn_1_D09C(OMOBJ *obj);
void fn_1_D450(s16 playerIndex);
void fn_1_D4EC(s16 playerIndex);

/* Enables player-count input after the card entrance and signals the waiting selection process. */
void fn_1_C6A8(OMOBJ *controllerObject)
{
    controllerObject->work[0] = 10;
    controllerObject->objFunc = fn_1_B6EC;
    *lbl_1_bss_24->mtnId = 1;
}

/* Dismisses player-count arrows and signals completion when the count-selection step ends. */
void fn_1_C6D8(OMOBJ *controllerObject)
{
    HuSprAttrSet(lbl_1_bss_752[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_86E[4] = 0;
    HuSprAttrSet(lbl_1_bss_752[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_86E[5] = 0;
    controllerObject->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* Shows the four player-count cards over ten frames before the selection process starts accepting
 * input. */
void fn_1_C770(OMOBJ *controllerObject)
{
    s16 playerIndex;
    s16 minimumPlayers;

    minimumPlayers = 1;
    playerIndex = 0;
    while (playerIndex < 4) {
        s16 entranceFrame;
        MDMinidllModelAnimationRecord *entranceCard;
        f32 entranceValue;
        entranceFrame = (s16) controllerObject->work[0];
        entranceCard = &lbl_1_bss_230[(s16) (playerIndex + 17)];
        if (entranceFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(entranceCard->model, lbl_1_data_4C8[playerIndex + 33].x,
                            lbl_1_data_4C8[playerIndex + 33].y, lbl_1_data_4C8[playerIndex + 33].z);
            Hu3DModelRotSet(entranceCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entranceCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entranceCard->model, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        entranceValue = fn_1_156C(-540.0f, 0.0f, (f32)entranceFrame, 10.0f);
        Hu3DModelRotSet(entranceCard->model, 0.0f, entranceValue, 0.0f);
        entranceValue = fn_1_156C(0.0f, 1.25f, (f32)entranceFrame, 10.0f);
        Hu3DModelScaleSet(entranceCard->model, entranceValue, entranceValue, 1.0f);
        playerIndex += 1;
    }
    if ((u32) controllerObject->work[0] == 0) {
        playerIndex = 0;
        lbl_1_bss_804[0] = 0;
        while (playerIndex < 4) {
            if (HuPadStatGet(playerIndex) == 0) {
                lbl_1_bss_804[0] += 1;
            }
            playerIndex += 1;
        }
        if (lbl_1_bss_804[0] < minimumPlayers) {
            lbl_1_bss_804[0] = minimumPlayers;
        }
        playerIndex = 0;
        while (playerIndex < 4) {
            if (playerIndex < lbl_1_bss_804[0]) {
                fn_1_9E00((s16)(playerIndex + 17), 19);
            } else {
                fn_1_9E00((s16)(playerIndex + 17), 18);
            }
            playerIndex += 1;
        }
    }
    /* Apply the four entrance poses again after choosing the count-card animations. */
    playerIndex = 0;
    while (playerIndex < 4) {
        s16 repeatFrame;
        MDMinidllModelAnimationRecord *repeatCard;
        f32 repeatValue;
        repeatFrame = (s16) controllerObject->work[0];
        repeatCard = &lbl_1_bss_230[(s16) (playerIndex + 17)];
        if (repeatFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(repeatCard->model, lbl_1_data_4C8[playerIndex + 33].x,
                            lbl_1_data_4C8[playerIndex + 33].y, lbl_1_data_4C8[playerIndex + 33].z);
            Hu3DModelRotSet(repeatCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(repeatCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(repeatCard->model, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        repeatValue = fn_1_156C(-540.0f, 0.0f, (f32)repeatFrame, 10.0f);
        Hu3DModelRotSet(repeatCard->model, 0.0f, repeatValue, 0.0f);
        repeatValue = fn_1_156C(0.0f, 1.25f, (f32)repeatFrame, 10.0f);
        Hu3DModelScaleSet(repeatCard->model, repeatValue, repeatValue, 1.0f);
        playerIndex += 1;
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Spins and shrinks the four player-count cards over ten frames, then signals the selection process
 * to continue. */
void fn_1_D09C(OMOBJ *controllerObject)
{
    MDMinidllModelAnimationRecord *modelRecord;
    float transformValue;
    s16 playerIndex;
    s16 elapsedFrames;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        elapsedFrames = controllerObject->work[0];
        modelRecord = &lbl_1_bss_230[(s16)(playerIndex + 17)];
        if (elapsedFrames == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(modelRecord->model, &modelRecord->savedPosition);
            Hu3DModelRotGet(modelRecord->model, &modelRecord->savedRotation);
            Hu3DModelScaleGet(modelRecord->model, &modelRecord->savedScale);
        } else {
            lbl_1_bss_34 = 0;
        }
        transformValue = fn_1_1868_inlined(modelRecord->savedRotation.y,
                                           modelRecord->savedRotation.y - 540.0f,
                                           elapsedFrames, 10.0f);
        Hu3DModelRotSet(modelRecord->model, 0.0f, transformValue, 0.0f);
        transformValue = fn_1_1868_inlined(modelRecord->savedScale.x, 0.0f,
                                           elapsedFrames, 10.0f);
        Hu3DModelScaleSet(modelRecord->model, transformValue, transformValue, 1.0f);
        if (elapsedFrames <= 10) {
            Hu3DModelAttrReset(modelRecord->model, HU3D_ATTR_DISPOFF);
        }
    }
    if (controllerObject->work[0]++ > 10) {
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Rotates a character card by six degrees during each update while a player is choosing that
 * character. */
void fn_1_D450(s16 characterIndex)
{
    Point3d rotation;
    MDMinidllModelAnimationRecord *modelRecord;

    modelRecord = &lbl_1_bss_230[characterIndex];
    Hu3DModelRotGet(modelRecord->model, &rotation);
    rotation.y += 6.0f;
    if (rotation.y > 360.0f) {
        rotation.y -= 360.0f;
    }
    Hu3DModelRotSetV(modelRecord->model, &rotation);
}

/* Restores a character card to zero rotation when it is no longer being chosen. */
void fn_1_D4EC(s16 characterIndex)
{
    MDMinidllModelAnimationRecord *modelRecord;

    modelRecord = &lbl_1_bss_230[characterIndex];
    Hu3DModelRotSet(modelRecord->model, 0.0f,
                    0.0f, 0.0f);
}

void fn_1_25BF4(s16 particleNo, Point3d *position, s16 colorNo);

/* Handles human character choices each frame, then rotates any character card that is still being
 * chosen. */
void fn_1_D554(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 selectionSlot;
    s16 playerIndex;

    selectionSlot = 0;
    playerConfig = lbl_1_bss_7CC;
    while (selectionSlot < 4) {
        if (playerConfig->comF == 0) {
            if (playerConfig->selectionPhase == 0) {
                if (controllerObject->work[selectionSlot]++ > 10U) {
                    if (fn_1_97D8(playerConfig->padNo, &playerConfig->charNo,
                                  (LBL_1_DATA_E12_ENTRY *) lbl_1_data_89A, 11) != 0) {
                        fn_1_8914(selectionSlot, &lbl_1_data_4C8[playerConfig->charNo + 14]);
                        controllerObject->work[selectionSlot] = 0;
                    } else if ((s32) (HuPadBtnDown[playerConfig->padNo] & PAD_BUTTON_A) != 0) {
                        if ((lbl_1_bss_28 == 0) && (playerConfig->charNo == 10)) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                        } else {
                            CharFXPlay(playerConfig->charNo, CHARVOICEID(14));
                            playerConfig->selectionPhase = 1;
                            fn_1_8B70(selectionSlot);
                            fn_1_809C(selectionSlot, playerConfig->padNo);
                            fn_1_9D58((s16)playerConfig->charNo, 0, (s16)playerConfig->charNo, 1);
                            fn_1_9D58((s16)playerConfig->charNo, 2, (s16)playerConfig->charNo, 1);
                            fn_1_25BF4(selectionSlot, &lbl_1_data_4C8[playerConfig->charNo + 14],
                                       playerConfig->padNo);
                        }
                    }
                }
            } else if ((s32) (HuPadBtnDown[playerConfig->padNo] & PAD_BUTTON_B) != 0) {
                playerConfig->selectionPhase = 0;
                fn_1_899C(selectionSlot, &lbl_1_data_4C8[playerConfig->charNo + 14],
                          playerConfig->padNo);
                fn_1_8188(selectionSlot);
                fn_1_9D58((s16)playerConfig->charNo, 0, (s16)playerConfig->charNo, 0);
                fn_1_9D58((s16)playerConfig->charNo, 2, (s16)playerConfig->charNo, 0);
            }
        }
        selectionSlot += 1;
        playerConfig += 1;
    }
    selectionSlot = 0;
    while (selectionSlot < 11) {
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            if ((playerConfig->comF != 0) || (playerConfig->charNo != selectionSlot) ||
                (playerConfig->selectionPhase != 0)) {
                playerIndex += 1;
                playerConfig += 1;
                continue;
            }
            break;
        }
        if (playerIndex == 4) {
            fn_1_D4EC(selectionSlot);
        } else {
            fn_1_D450(selectionSlot);
        }
        selectionSlot += 1;
    }
}

/* Restores markers for editable human character choices and starts their input callback for the
 * selection process. */
void fn_1_DD00(OMOBJ *controllerObject)
{
    s32 playerIndex = 0;
    MDMinidllPlayerConfig *playerConfig = lbl_1_bss_7CC;

    while ((s16)playerIndex < 4) {
        if (playerConfig->comF == 0 && playerConfig->selectionPhase == 0) {
            Point3d *characterPosition;
            OMOBJ *markerObject;
            s16 bankIndex;

            lbl_1_data_89A[playerConfig->charNo].claimed = 1;
            bankIndex = playerConfig->padNo;
            characterPosition = &lbl_1_data_4C8[playerConfig->charNo + 14];
            markerObject = lbl_1_bss_1C[0];

            Hu3DModelPosSet(markerObject->mdlId[(s16) playerIndex], characterPosition->x,
                            characterPosition->y, characterPosition->z);
            Hu3DModelRotSet(markerObject->mdlId[(s16)playerIndex], 0.0f, 0.0f, 180.0f);
            Hu3DModelScaleSet(markerObject->mdlId[(s16)playerIndex], 1.0f, 1.0f, 1.0f);

            lbl_1_bss_50[(s16)playerIndex].x = characterPosition->x - 50.0f;
            lbl_1_bss_50[(s16)playerIndex].y = 50.0f + characterPosition->y;
            lbl_1_bss_50[(s16)playerIndex].z = 50.0f + characterPosition->z;

            Hu3DModelAttrReset(markerObject->mdlId[(s16)playerIndex], HU3D_ATTR_DISPOFF);
            HuSprScaleSet(lbl_1_bss_752[3], (s16) playerIndex, 0.800000011920929f,
                          0.800000011920929f);
            HuSprAttrReset(lbl_1_bss_752[3], (s16)playerIndex, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_752[3], (s16)playerIndex, bankIndex);
        }
        playerIndex += 1;
        playerConfig += 1;
    }

    /* Only the first player starts with ten input-delay ticks already elapsed. */
    controllerObject->work[0] = 10;
    controllerObject->objFunc = fn_1_D554;
    *lbl_1_bss_24->mtnId = 1;
}

/* Dismisses editable human selection markers, resets character-card rotations and signals the end
 * of the step. */
void fn_1_DF64(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 selectionSlot;

    selectionSlot = 0;
    playerConfig = lbl_1_bss_7CC;
    while (selectionSlot < 4) {
        if (playerConfig->comF == 0) {
            if (playerConfig->selectionPhase == 0) {
                OMOBJ *markerObject;

                markerObject = lbl_1_bss_1C[0];
                Hu3DModelAttrSet(markerObject->mdlId[selectionSlot], HU3D_ATTR_DISPOFF);
                HuSprScaleSet(lbl_1_bss_752[3], (s16)selectionSlot,
                              0.800000011920929f, 0.800000011920929f);
                HuSprAttrSet(lbl_1_bss_752[3], (s16)selectionSlot, HUSPR_ATTR_DISPOFF);
            }
        }
        selectionSlot += 1;
        playerConfig += 1;
    }

    selectionSlot = 0;
    while (selectionSlot < 11) {
        MDMinidllModelAnimationRecord *modelRecord;

        modelRecord = &lbl_1_bss_230[selectionSlot];
        Hu3DModelRotSet(modelRecord->model, 0.0f, 0.0f,
                        0.0f);
        selectionSlot += 1;
    }

    controllerObject->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* Reopens the first player choice and restores its character-card texture banks during selection
 * backtracking. */
void fn_1_E0B0(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig = &lbl_1_bss_7CC[0];

    playerConfig->selectionPhase = 0;
    lbl_1_bss_80[0].x = 0.0f;
    fn_1_C158_inlined((s16)playerConfig->charNo, 0, (s16)playerConfig->charNo, 0);
    fn_1_C158_inlined((s16)playerConfig->charNo, 2, (s16)playerConfig->charNo, 0);
    controllerObject->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_F0A8(OMOBJ *obj);

/* Handles one CPU player character and difficulty choice from controller 0 each frame. */
void fn_1_E1C0(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    HuVecF characterPosition;
    s16 selectionSlot;
    s16 otherPlayerIndex;

    for (selectionSlot = 0, playerConfig = lbl_1_bss_7CC; selectionSlot < 4;
         selectionSlot++, playerConfig++) {
        if (playerConfig->comF == 1 && playerConfig->selectionPhase != 2) {
            if (playerConfig->selectionPhase == 0) {
                if (controllerObject->work[0]++ > 10) {
                    if (fn_1_97D8(
                            0, &playerConfig->charNo, (LBL_1_DATA_E12_ENTRY *)lbl_1_data_89A, 11)) {
                        fn_1_8914(
                            selectionSlot, &lbl_1_data_4C8[playerConfig->charNo + 14]);
                        controllerObject->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_28 == 0 && playerConfig->charNo == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            break;
                        }
                        CharFXPlay(playerConfig->charNo, CHARVOICEID(14));
                        playerConfig->selectionPhase = 1;
                        playerConfig->comDif = 1;
                        fn_1_8B70(selectionSlot);
                        fn_1_809C(selectionSlot, playerConfig->comDif + 5);
                        fn_1_C158_inlined(
                            (s16)playerConfig->charNo, 0, (s16)playerConfig->charNo, 1);
                        fn_1_C158_inlined(
                            (s16)playerConfig->charNo, 2, (s16)playerConfig->charNo, 1);
                        Hu3DModelPosGet(
                            lbl_1_bss_230[playerConfig->charNo].model, &characterPosition);
                        fn_1_8570(4, &characterPosition, -40.0f, -45.0f);
                        fn_1_8570(5, &characterPosition, 40.0f, -45.0f);
                        fn_1_25BF4(selectionSlot,
                            &lbl_1_data_4C8[playerConfig->charNo + 14], 4);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        lbl_1_data_89A[playerConfig->charNo].claimed = -1;
                        fn_1_8B70(selectionSlot);
                        for (otherPlayerIndex = 3; otherPlayerIndex >= 0; otherPlayerIndex--) {
                            if (lbl_1_bss_7CC[otherPlayerIndex].comF == 1
                                && lbl_1_bss_7CC[otherPlayerIndex].selectionPhase == 2) {
                                lbl_1_bss_7CC[otherPlayerIndex].selectionPhase = 0;
                                lbl_1_data_89A[
                                    lbl_1_bss_7CC[otherPlayerIndex].charNo]
                                    .claimed = -1;
                                fn_1_8188(otherPlayerIndex);
                                fn_1_C158_inlined((s16)lbl_1_bss_7CC[otherPlayerIndex].charNo, 0,
                                    (s16)lbl_1_bss_7CC[otherPlayerIndex].charNo, 0);
                                fn_1_C158_inlined((s16)lbl_1_bss_7CC[otherPlayerIndex].charNo, 2,
                                    (s16)lbl_1_bss_7CC[otherPlayerIndex].charNo, 0);
                                break;
                            }
                        }
                        if (otherPlayerIndex != -1) {
                            controllerObject->objFunc = fn_1_F0A8;
                        } else {
                            controllerObject->objFunc = NULL;
                        }
                    }
                }
            } else if (playerConfig->selectionPhase == 1) {
                if (controllerObject->work[0]++ > 10) {
                    if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                        playerConfig->comDif--;
                        if (lbl_1_bss_2A == 0) {
                            if (playerConfig->comDif < 0) {
                                playerConfig->comDif += 3;
                            }
                        } else {
                            if (playerConfig->comDif < 0) {
                                playerConfig->comDif += 4;
                            }
                        }
                        controllerObject->work[0] = 0;
                        fn_1_86F0(4);
                    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                        playerConfig->comDif++;
                        if (lbl_1_bss_2A == 0) {
                            if (playerConfig->comDif > 2) {
                                playerConfig->comDif -= 3;
                            }
                        } else if (playerConfig->comDif > 3) {
                            playerConfig->comDif -= 4;
                        }
                        controllerObject->work[0] = 0;
                        fn_1_86F0(5);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        HuAudFXPlay(MSM_SE_CMN_02);
                        playerConfig->selectionPhase = 2;
                        fn_1_8694(4);
                        fn_1_8694(5);
                        for (otherPlayerIndex = 0; otherPlayerIndex < 4; otherPlayerIndex++) {
                            if (lbl_1_bss_7CC[otherPlayerIndex].comF == 1
                                && lbl_1_bss_7CC[otherPlayerIndex].selectionPhase == 0) {
                                break;
                            }
                        }
                        if (otherPlayerIndex != 4) {
                            controllerObject->objFunc = fn_1_F0A8;
                        } else {
                            controllerObject->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        playerConfig->selectionPhase = 0;
                        fn_1_8694(4);
                        fn_1_8694(5);
                        fn_1_C158_inlined(
                            (s16)playerConfig->charNo, 0, (s16)playerConfig->charNo, 0);
                        fn_1_C158_inlined(
                            (s16)playerConfig->charNo, 2, (s16)playerConfig->charNo, 0);
                        fn_1_899C(
                            selectionSlot, &lbl_1_data_4C8[playerConfig->charNo + 14], 4);
                        fn_1_8188(selectionSlot);
                    }
                }
                fn_1_81B0(selectionSlot, playerConfig->comDif + 5);
            }
            break;
        }
    }
    for (selectionSlot = 0; selectionSlot < 11; selectionSlot++) {
        if (playerConfig->charNo == selectionSlot && playerConfig->selectionPhase == 0) {
            fn_1_D450(selectionSlot);
        } else {
            fn_1_D4EC(selectionSlot);
        }
    }
}

/* Prepares the next unfinished CPU choice and enables its character or difficulty input
 * callback. */
void fn_1_F0A8(OMOBJ *controllerObject)
{
    Point3d characterPosition;
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;
    Point3d *markerPosition;
    OMOBJ *markerObject;
    s16 characterIndex;

    playerIndex = 0;
    playerConfig = lbl_1_bss_7CC;
    while (playerIndex < 4) {
        if (playerConfig->comF == 1) {
            if (playerConfig->selectionPhase == 0) {
                characterIndex = 0;
                while (characterIndex < 11) {
                    if (lbl_1_data_89A[characterIndex].claimed == -1) {
                        playerConfig->charNo = characterIndex;
                        lbl_1_data_89A[characterIndex].claimed = 1;
                    } else {
                        characterIndex += 1;
                        continue;
                    }
                    break;
                }
                markerPosition = &lbl_1_data_4C8[playerConfig->charNo + 14];
                markerObject = lbl_1_bss_1C[0];
                Hu3DModelPosSet(markerObject->mdlId[playerIndex], markerPosition->x,
                                markerPosition->y, markerPosition->z);
                Hu3DModelRotSet(markerObject->mdlId[playerIndex], 0.0f, 0.0f, 180.0f);
                Hu3DModelScaleSet(markerObject->mdlId[playerIndex], 1.0f, 1.0f, 1.0f);
                lbl_1_bss_50[playerIndex].x = markerPosition->x - 50.0f;
                lbl_1_bss_50[playerIndex].y = 50.0f + markerPosition->y;
                lbl_1_bss_50[playerIndex].z = 50.0f + markerPosition->z;
                Hu3DModelAttrReset(markerObject->mdlId[playerIndex], HU3D_ATTR_DISPOFF);
                HuSprScaleSet(lbl_1_bss_752[3], (s16) playerIndex, 0.800000011920929f,
                              0.800000011920929f);
                HuSprAttrReset(lbl_1_bss_752[3], (s16) playerIndex, HUSPR_ATTR_DISPOFF);
                HuSprBankSet(lbl_1_bss_752[3], (s16) playerIndex, 4);
                break;
            } else if (playerConfig->selectionPhase == 1) {
                Hu3DModelPosGet(lbl_1_bss_230[playerConfig->charNo].model, &characterPosition);
                fn_1_8570(4, &characterPosition, -40.0f, -45.0f);
                fn_1_8570(5, &characterPosition, 40.0f, -45.0f);
                break;
            }
        }
        playerIndex += 1;
        playerConfig += 1;
    }
    controllerObject->work[0] = 10;
    controllerObject->objFunc = fn_1_E1C0;
    *lbl_1_bss_24->mtnId = 1;
}

/* Dismisses the unfinished CPU choice, resets character-card rotations and signals the end of the
 * selection step. */
void fn_1_F52C(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 selectionSlot;

    selectionSlot = 0;
    playerConfig = lbl_1_bss_7CC;
    while (selectionSlot < 4) {
        if (playerConfig->comF == 1) {
            if (playerConfig->selectionPhase == 0) {
                OMOBJ *markerObject;

                lbl_1_data_89A[playerConfig->charNo].claimed = -1;
                markerObject = lbl_1_bss_1C[0];
                Hu3DModelAttrSet(markerObject->mdlId[selectionSlot], HU3D_ATTR_DISPOFF);
                HuSprScaleSet(lbl_1_bss_752[3], (s16)selectionSlot,
                              0.800000011920929f, 0.800000011920929f);
                HuSprAttrSet(lbl_1_bss_752[3], (s16)selectionSlot, HUSPR_ATTR_DISPOFF);
            } else if (playerConfig->selectionPhase == 1) {
                HuSprAttrSet(lbl_1_bss_752[2], 4, HUSPR_ATTR_DISPOFF);
                lbl_1_data_86E[4] = 0;
                HuSprAttrSet(lbl_1_bss_752[2], 5, HUSPR_ATTR_DISPOFF);
                lbl_1_data_86E[5] = 0;
            } else {
                goto next_player;
            }
            break;
        } else {
        next_player:
            selectionSlot += 1;
            playerConfig += 1;
            continue;
        }
    }

    selectionSlot = 0;
    while (selectionSlot < 11) {
        MDMinidllModelAnimationRecord *modelRecord;

        modelRecord = &lbl_1_bss_230[selectionSlot];
        Hu3DModelRotSet(modelRecord->model, 0.0f,
                        0.0f, 0.0f);
        selectionSlot += 1;
    }

    controllerObject->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* The character-selection step resets player choices and spins all character cards into view. */
void fn_1_F6F8(OMOBJ *controllerObject)
{
    s16 characterIndex;
    s16 entranceFrame;
    s16 playerIndex;
    f32 entranceValue;

    if ((u32) controllerObject->work[0] == 0) {
        MDMinidllPlayerConfig *playerConfig;
        characterIndex = 0;
        while (characterIndex < 11) {
            fn_1_9E00(characterIndex, characterIndex);
            fn_1_9D58(characterIndex, 1, characterIndex, 2);
            fn_1_9D58(characterIndex, 3, characterIndex, 2);
            lbl_1_data_89A[characterIndex].claimed = -1;
            characterIndex += 1;
        }
        if (lbl_1_bss_28 == 0) {
            fn_1_9E00(10, 11);
        }
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            playerConfig->selectionPhase = 0;
            playerConfig->unusedSetupValue = 0;
            playerConfig->comDif = 0;
            playerConfig->charNo = playerIndex;
            playerConfig->padNo = playerIndex;
            playerIndex += 1;
            playerConfig += 1;
        }
    }
    characterIndex = 0;
    while (characterIndex < 11) {
        MDMinidllModelAnimationRecord *characterCard;
        entranceFrame = (s16) controllerObject->work[0];
        characterCard = &lbl_1_bss_230[characterIndex];
        if (entranceFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(characterCard->model, lbl_1_data_4C8[characterIndex + 14].x,
                            lbl_1_data_4C8[characterIndex + 14].y,
                            lbl_1_data_4C8[characterIndex + 14].z);
            Hu3DModelRotSet(characterCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(characterCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(characterCard->model, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        entranceValue = fn_1_156C(-540.0f, 0.0f, (f32) entranceFrame, 10.0f);
        Hu3DModelRotSet(characterCard->model, 0.0f, entranceValue, 0.0f);
        entranceValue = fn_1_156C(0.0f, 1.25f, (f32) entranceFrame, 10.0f);
        Hu3DModelScaleSet(characterCard->model, entranceValue, entranceValue, 1.0f);
        characterIndex += 1;
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* After character selection is confirmed, spins unused cards away and moves the four chosen cards
 * to their player positions. */
void fn_1_FCC0(OMOBJ *controllerObject)
{
    Point3d position;
    s16 selectionIndex;
    s16 moveFrame;
    s16 dismissFrame;
    s16 playerIndex;
    s16 characterIndex;
    f32 dismissValue;
    f32 cardScale;

    selectionIndex = 0;
    while (selectionIndex < 11) {
        playerIndex = 0;
        while (playerIndex < 4) {
            if (selectionIndex != lbl_1_bss_7CC[playerIndex].charNo) {
                playerIndex += 1;
                continue;
            }
            break;
        }
        if (playerIndex == 4) {
            MDMinidllModelAnimationRecord *unselectedCard;
            dismissFrame = (s16) controllerObject->work[0];
            unselectedCard = &lbl_1_bss_230[selectionIndex];
            if (dismissFrame == 0) {
                if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                    lbl_1_bss_34 = 1;
                }
                Hu3DModelPosGet(unselectedCard->model, &unselectedCard->savedPosition);
                Hu3DModelRotGet(unselectedCard->model, &unselectedCard->savedRotation);
                Hu3DModelScaleGet(unselectedCard->model, &unselectedCard->savedScale);
            } else {
                lbl_1_bss_34 = 0;
            }
            dismissValue =
                fn_1_156C(unselectedCard->savedRotation.y, unselectedCard->savedRotation.y - 540.0f,
                          (f32) dismissFrame, 10.0f);
            Hu3DModelRotSet(unselectedCard->model, 0.0f, dismissValue, 0.0f);
            dismissValue = fn_1_156C(unselectedCard->savedScale.x, 0.0f, (f32) dismissFrame, 10.0f);
            Hu3DModelScaleSet(unselectedCard->model, dismissValue, dismissValue, 1.0f);
            if (dismissFrame <= 10) {
                Hu3DModelAttrReset(unselectedCard->model, HU3D_ATTR_DISPOFF);
            }
        }
        selectionIndex += 1;
    }
    if ((u32) controllerObject->work[0] >= 10U) {
        selectionIndex = 0;
        while (selectionIndex < 4) {
            MDMinidllModelAnimationRecord *selectedCard;
            moveFrame = controllerObject->work[0] - 10;
            characterIndex = (s16) lbl_1_bss_7CC[selectionIndex].charNo;
            selectedCard = &lbl_1_bss_230[characterIndex];
            if (moveFrame == 0) {
                Hu3DModelPosGet(selectedCard->model, &selectedCard->savedPosition);
                Hu3DModelRotGet(selectedCard->model, &selectedCard->savedRotation);
                Hu3DModelScaleGet(selectedCard->model, &selectedCard->savedScale);
            }
            position.x = fn_1_156C(selectedCard->savedPosition.x,
                                   lbl_1_data_4C8[selectionIndex + 25].x, (f32) moveFrame, 10.0f);
            position.y = fn_1_156C(selectedCard->savedPosition.y,
                                   lbl_1_data_4C8[selectionIndex + 25].y, (f32) moveFrame, 10.0f);
            position.z = fn_1_156C(selectedCard->savedPosition.z,
                                   lbl_1_data_4C8[selectionIndex + 25].z, (f32) moveFrame, 10.0f);
            Hu3DModelPosSet(selectedCard->model, position.x, position.y, position.z);
            cardScale = fn_1_156C(selectedCard->savedScale.x, 1.25f, (f32) moveFrame, 10.0f);
            Hu3DModelScaleSet(selectedCard->model, cardScale, cardScale, 1.0f);
            selectionIndex += 1;
        }
    }
    if (controllerObject->work[0]++ > 20U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* When mode selection returns to character selection, spins and shrinks the four chosen cards
 * before the full character list reappears. */
void fn_1_105DC(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    MDMinidllModelAnimationRecord *characterCard;
    float dismissValue;
    s32 characterIndex;
    s16 playerIndex;
    s16 dismissFrame;

    if (controllerObject->work[0] == 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_80[playerIndex].x = 0.0f;
        }
    }
    for (playerIndex = 0, playerConfig = lbl_1_bss_7CC; playerIndex < 4;
         playerIndex++, playerConfig++) {
        dismissFrame = controllerObject->work[0];
        characterIndex = (s16)playerConfig->charNo;
        characterCard = &lbl_1_bss_230[characterIndex];
        if (dismissFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(characterCard->model, &characterCard->savedPosition);
            Hu3DModelRotGet(characterCard->model, &characterCard->savedRotation);
            Hu3DModelScaleGet(characterCard->model, &characterCard->savedScale);
        } else {
            lbl_1_bss_34 = 0;
        }
        dismissValue = fn_1_1868_inlined(characterCard->savedRotation.y,
                                         characterCard->savedRotation.y - 540.0f,
                                         dismissFrame, 10.0f);
        Hu3DModelRotSet(characterCard->model, 0.0f, dismissValue, 0.0f);
        dismissValue = fn_1_1868_inlined(characterCard->savedScale.x, 0.0f,
                                         dismissFrame, 10.0f);
        Hu3DModelScaleSet(characterCard->model, dismissValue, dismissValue, 1.0f);
        if (dismissFrame <= 10) {
            Hu3DModelAttrReset(characterCard->model, HU3D_ATTR_DISPOFF);
        }
    }
    if (controllerObject->work[0]++ > 10) {
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* When character selection returns to player-count selection, spins and shrinks every character
 * card and signals completion. */
void fn_1_109D4(OMOBJ *controllerObject)
{
    MDMinidllModelAnimationRecord *characterCard;
    s16 characterIndex;
    s16 dismissFrame;
    f32 dismissValue;

    if (controllerObject->work[0] == 0) {
        for (characterIndex = 0; characterIndex < 4; characterIndex++) {
            lbl_1_bss_80[characterIndex].x = 0.0f;
        }
    }
    for (characterIndex = 0; characterIndex < 11; characterIndex++) {
        dismissFrame = controllerObject->work[0];
        characterCard = &lbl_1_bss_230[characterIndex];
        if (dismissFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(characterCard->model, &characterCard->savedPosition);
            Hu3DModelRotGet(characterCard->model, &characterCard->savedRotation);
            Hu3DModelScaleGet(characterCard->model, &characterCard->savedScale);
        } else {
            lbl_1_bss_34 = 0;
        }
        dismissValue = fn_1_1868_inlined(characterCard->savedRotation.y,
            characterCard->savedRotation.y - 540.0f, dismissFrame, 10.0f);
        Hu3DModelRotSet(characterCard->model, 0.0f, dismissValue, 0.0f);
        dismissValue = fn_1_1868_inlined(characterCard->savedScale.x,
            0.0f, dismissFrame, 10.0f);
        Hu3DModelScaleSet(characterCard->model, dismissValue, dismissValue, 1.0f);
        if (dismissFrame <= 10) {
            Hu3DModelAttrReset(characterCard->model, HU3D_ATTR_DISPOFF);
        }
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Each mode-selection update enlarges the highlighted card and eases it toward the middle row at
* y = 275. Rotation starts on the 32nd highlighted update. */
void fn_1_10DCC(s16 modeIndex)
{
    Point3d transform;
    MDMinidllModelAnimationRecord *modeCard;

    modeCard = &lbl_1_bss_230[modeIndex + 11];
    Hu3DModelPosGet(modeCard->model, &transform);
    fn_1_1340(&transform, &lbl_1_data_4C8[modeIndex + 7], 5.0f);
    Hu3DModelPosSetV(modeCard->model, &transform);
    if (modeCard->highlightFrame++ > 30) {
        modeCard->highlightFrame = 35;
        Hu3DModelRotGet(modeCard->model, &transform);
        transform.y += 3.0f;
        if (transform.y >= 360.0f) {
            transform.y -= 360.0f;
        }
        Hu3DModelRotSetV(modeCard->model, &transform);
    }
    Hu3DModelScaleGet(modeCard->model, &transform);
    transform.y = transform.x = fn_1_1310(transform.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(modeCard->model, &transform);
    Hu3DModelLayerSet(modeCard->model, 2);
}

/* Each mode-selection update restores an unhighlighted card to its resting position, scale and
 * rendering layer. */
void fn_1_110EC(s16 modeIndex)
{
    MDMinidllModelAnimationRecord *modeCard = &lbl_1_bss_230[modeIndex + 11];
    Hu3DModelPosSetV(modeCard->model, &lbl_1_data_4C8[modeIndex + 1]);
    Hu3DModelRotSet(modeCard->model, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modeCard->model, 1.25f, 1.25f, 1.0f);
    modeCard->highlightFrame = 0;
    Hu3DModelLayerSet(modeCard->model, 1);
}

/* While the mode-selection process waits for input, cycles the highlighted mode with controller
 * zero and updates all six card poses. */
void fn_1_111C0(OMOBJ *controllerObject)
{
    s16 modeIndex;
    if (controllerObject->work[0]++ >= 10U) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_804[1]--;
            if (lbl_1_bss_804[1] < 0) {
                lbl_1_bss_804[1] += 6;
            }
            controllerObject->work[0] = 0;
            controllerObject->work[1] = 0;
            fn_1_7280(lbl_1_bss_804[1]);
            lbl_1_bss_38[0] = 2.0f;
            if (lbl_1_data_86E[0] == 1) {
            HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_804[1]++;
            if (lbl_1_bss_804[1] >= 6) {
                lbl_1_bss_804[1] -= 6;
            }
            controllerObject->work[0] = 0;
            controllerObject->work[1] = 1;
            fn_1_7280(lbl_1_bss_804[1]);
            lbl_1_bss_38[1] = 2.0f;
            if (lbl_1_data_86E[1] == 1) {
            HuAudFXPlay(MSM_SE_CMN_01);
            }
        }
    }
    for (modeIndex = 0; modeIndex < 6; modeIndex++) {
        if (modeIndex == lbl_1_bss_804[1]) {
            fn_1_10DCC(modeIndex);
        } else {
            fn_1_110EC(modeIndex);
        }
    }
}

/* The mode-selection process shows both arrows, resets card rotation delays and enables mode input
 * after the entrance. */
void fn_1_118A0(OMOBJ *controllerObject)
{
    s16 modeIndex;
    for (modeIndex = 0; modeIndex < 6; modeIndex++) {
        lbl_1_bss_230[modeIndex + 11].highlightFrame = 0;
    }
    fn_1_8570(0, lbl_1_data_4C8, -240.0f, 0.0f);
    fn_1_8570(1, lbl_1_data_4C8, 240.0f, 0.0f);
    controllerObject->work[0] = 10;
    controllerObject->objFunc = fn_1_111C0;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* When mode input ends, hides its arrows and turns all six cards to face forward before the next
 * transition. */
void fn_1_11B0C(OMOBJ *controllerObject)
{
    s16 modeIndex;
    if (controllerObject->work[0] == 0) {
        fn_1_8694(0);
        fn_1_8694(1);
    }
    for (modeIndex = 0; modeIndex < 6; modeIndex++) {
        lbl_1_bss_230[modeIndex + 11].highlightFrame = 0;
        fn_1_B078(modeIndex + 11, 0.0f, controllerObject->work[0], 10);
    }
    if (controllerObject->work[0]++ > 10) {
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* The mode-selection step loads available or locked card animations and spins all six cards into
 * view, initially highlighting the first. */
void fn_1_11E14(OMOBJ *controllerObject)
{
    s16 modeIndex;

    lbl_1_bss_804[1] = 0;
    if ((u32) controllerObject->work[0] == 0) {
        modeIndex = 0;
        while (modeIndex < 6) {
            fn_1_9E00((s16)(modeIndex + 11), (s16)(modeIndex + 12));
            if (lbl_1_data_6B6[modeIndex] == 0) {
                fn_1_9D58((s16)(modeIndex + 11), 2, 20, -1);
                fn_1_9D58((s16)(modeIndex + 11), 3, 20, -1);
            }
            modeIndex += 1;
        }
    }
    modeIndex = 0;
    while (modeIndex < 6) {
        s16 entranceFrame;
        MDMinidllModelAnimationRecord *modeCard;
        f32 entranceValue;

        entranceFrame = (s16) controllerObject->work[0];
        modeCard = &lbl_1_bss_230[(s16) (modeIndex + 11)];
        if (entranceFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(modeCard->model, lbl_1_data_4C8[modeIndex + 1].x,
                            lbl_1_data_4C8[modeIndex + 1].y, lbl_1_data_4C8[modeIndex + 1].z);
            Hu3DModelRotSet(modeCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(modeCard->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(modeCard->model, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        entranceValue = fn_1_156C(-540.0f, 0.0f, (f32) entranceFrame, 10.0f);
        Hu3DModelRotSet(modeCard->model, 0.0f, entranceValue, 0.0f);
        entranceValue = fn_1_156C(0.0f, 1.25f, (f32) entranceFrame, 10.0f);
        Hu3DModelScaleSet(modeCard->model, entranceValue, entranceValue, 1.0f);
        modeIndex += 1;
    }
    if (controllerObject->work[0]++ > 10U) {
        Hu3DZClearLayerSet(2);
        fn_1_7280(lbl_1_bss_804[1]);
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* When returning from the start prompt to mode selection, fades the first display model in and the
 * mode label out before the cards return. */
void fn_1_1240C(OMOBJ *controllerObject)
{
    OMOBJ *modelFadeObject;
    OMOBJ *labelFadeObject;
    if (controllerObject->work[0] == 0) {
        modelFadeObject = lbl_1_bss_C;
        modelFadeObject->work[0] = 2;
        modelFadeObject->work[1] = 0;
        labelFadeObject = lbl_1_bss_C;
        labelFadeObject->work[2] = 2;
        labelFadeObject->work[3] = 0;
    }
    if (controllerObject->work[0]++ > 30U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* After a mode is confirmed, removes the other cards, moves the selected card to the center and
 * switches the animated display to that mode. */
void fn_1_124A0(OMOBJ *controllerObject)
{
    Point3d rotation;
    Point3d position;
    s16 moveFrame;
    s16 dismissFrame;
    OMOBJ *displayObject;
    s16 modeIndex;
    s16 selectedMode;
    s16 selectedCardIndex;
    f32 dismissValue;
    f32 cardScale;

    modeIndex = 0;
    while (modeIndex < 6) {
        if (modeIndex != lbl_1_bss_804[1]) {
            MDMinidllModelAnimationRecord *unselectedCard;
            dismissFrame = (s16)controllerObject->work[0];
            unselectedCard = &lbl_1_bss_230[(s16)(modeIndex + 11)];
            if (dismissFrame == 0) {
                if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                    lbl_1_bss_34 = 1;
                }
                Hu3DModelPosGet(unselectedCard->model,
                                &unselectedCard->savedPosition);
                Hu3DModelRotGet(unselectedCard->model,
                                &unselectedCard->savedRotation);
                Hu3DModelScaleGet(unselectedCard->model,
                                  &unselectedCard->savedScale);
            } else {
                lbl_1_bss_34 = 0;
            }
            dismissValue = fn_1_156C(unselectedCard->savedRotation.y,
                unselectedCard->savedRotation.y - 540.0f, (f32)dismissFrame, 10.0f);
            Hu3DModelRotSet(unselectedCard->model, 0.0f, dismissValue,
                            0.0f);
            dismissValue = fn_1_156C(unselectedCard->savedScale.x,
                0.0f, (f32)dismissFrame, 10.0f);
            Hu3DModelScaleSet(unselectedCard->model, dismissValue, dismissValue,
                              1.0f);
            if (dismissFrame <= 10) {
                Hu3DModelAttrReset(unselectedCard->model, HU3D_ATTR_DISPOFF);
            }
        }
        modeIndex += 1;
    }

    if ((u32)controllerObject->work[0] >= 10U) {
        MDMinidllModelAnimationRecord *selectedCard;
        moveFrame = controllerObject->work[0] - 10;
        selectedCardIndex = lbl_1_bss_804[1] + 11;
        selectedCard = &lbl_1_bss_230[selectedCardIndex];
        if (moveFrame == 0) {
            Hu3DModelPosGet(selectedCard->model,
                            &selectedCard->savedPosition);
            Hu3DModelRotGet(selectedCard->model,
                            &selectedCard->savedRotation);
            Hu3DModelScaleGet(selectedCard->model,
                              &selectedCard->savedScale);
        }
        position.x = fn_1_156C(selectedCard->savedPosition.x,
            lbl_1_data_4C8[13].x, (f32)moveFrame, 10.0f);
        position.y = fn_1_156C(selectedCard->savedPosition.y,
            lbl_1_data_4C8[13].y, (f32)moveFrame, 10.0f);
        position.z = fn_1_156C(selectedCard->savedPosition.z,
            lbl_1_data_4C8[13].z, (f32)moveFrame, 10.0f);
        Hu3DModelPosSet(selectedCard->model, position.x, position.y, position.z);
        cardScale = fn_1_156C(selectedCard->savedScale.x,
            1.5f, (f32)moveFrame, 10.0f);
        Hu3DModelScaleSet(selectedCard->model, cardScale, cardScale, 1.0f);
        Hu3DModelRotGet(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model,
                        &rotation);
        rotation.y += 54.0f;
        Hu3DModelRotSetV(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model,
                         &rotation);
    }

    if ((u32)controllerObject->work[0] == 20U) {
        Hu3DLayerHookReset(2);
        fn_1_259FC(&lbl_1_data_4C8[13]);
        HuAudFXPlay(MSM_SE_MENU_15);
        selectedMode = lbl_1_bss_804[1];
        displayObject = lbl_1_bss_C;
        Hu3DAnimAnimSet(displayObject->mtnId[2], lbl_1_bss_764[(selectedMode * 2) + 3]);
        Hu3DAnimAnimSet(displayObject->mtnId[3], lbl_1_bss_764[(selectedMode * 2) + 4]);
        displayObject->work[0] = 1;
        displayObject->work[1] = 0;
        Hu3DModelAttrSet(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model, HU3D_ATTR_DISPOFF);
    }

    if (controllerObject->work[0]++ > 90U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* When mode selection returns to character selection, fades the mode label out and spins all mode
 * cards away. */
void fn_1_12E88(OMOBJ *controllerObject)
{
    MDMinidllModelAnimationRecord *modeCard;
    s16 modeIndex;
    OMOBJ *labelFadeObject;
    float dismissValue;
    s16 dismissFrame;

    if (controllerObject->work[0] == 0) {
        labelFadeObject = lbl_1_bss_C;
        labelFadeObject->work[2] = 2;
        labelFadeObject->work[3] = 0;
    }
    for (modeIndex = 0; modeIndex < 6; modeIndex++) {
        dismissFrame = controllerObject->work[0];
        modeCard = &lbl_1_bss_230[(s16)(modeIndex + 11)];
        if (dismissFrame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(modeCard->model, &modeCard->savedPosition);
            Hu3DModelRotGet(modeCard->model, &modeCard->savedRotation);
            Hu3DModelScaleGet(modeCard->model, &modeCard->savedScale);
        } else {
            lbl_1_bss_34 = 0;
        }
        dismissValue = fn_1_1868_inlined(
            modeCard->savedRotation.y, modeCard->savedRotation.y - 540.0f, dismissFrame, 10.0f);
        Hu3DModelRotSet(modeCard->model, 0.0f, dismissValue, 0.0f);
        dismissValue = fn_1_1868_inlined(modeCard->savedScale.x, 0.0f, dismissFrame, 10.0f);
        Hu3DModelScaleSet(modeCard->model, dismissValue, dismissValue, 1.0f);
        if (dismissFrame <= 10) {
            Hu3DModelAttrReset(modeCard->model, HU3D_ATTR_DISPOFF);
        }
    }
    if (controllerObject->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Before the start prompt, moves player cards into the confirmation layout; the single-player mode
 * keeps only the first card visible. */
void fn_1_1325C(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;

    HuSprGrpPosSet(lbl_1_bss_752[1], 288.0f, 60.0f + (f32)(controllerObject->work[0] * 5));
    if (lbl_1_bss_804[1] == 5) {
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            if (playerIndex == 0) {
                fn_1_A700((s16)playerConfig->charNo, (s16)controllerObject->work[0], 10,
                            &lbl_1_data_4C8[37], 1.25f);
            } else {
                lbl_1_bss_80[playerIndex].x = 0.0f;
                fn_1_A330((s16)playerConfig->charNo, (s16)controllerObject->work[0], 10);
            }
            playerIndex++;
            playerConfig++;
        }
    } else {
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            fn_1_A700((s16)playerConfig->charNo, (s16)controllerObject->work[0], 10,
                        &lbl_1_data_4C8[playerIndex + 29], 1.25f);
            playerIndex++;
            playerConfig++;
        }
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* After the start prompt is accepted, removes the player cards and brings the chosen character
 * models into the scene. */
void fn_1_1406C(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;
    OMOBJ *labelFadeObject;

    if (controllerObject->work[0] == 0) {
        labelFadeObject = lbl_1_bss_C;
        labelFadeObject->work[2] = 2;
        labelFadeObject->work[3] = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_80[playerIndex].x = 0.0f;
        }
    }
    for (playerIndex = 0, playerConfig = lbl_1_bss_7CC; playerIndex < 4;
         playerIndex++, playerConfig++) {
        fn_1_A330((s16)playerConfig->charNo, (s16)controllerObject->work[0], 10);
    }
    if (controllerObject->work[0]++ > 10U) {
        fn_1_68F0();
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* When the start prompt restarts setup, fades the display back in, hides the mode label and removes
 * the chosen player cards. */
void fn_1_14484(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;
    OMOBJ *modelFadeObject;
    OMOBJ *labelFadeObject;

    if (controllerObject->work[0] == 0) {
        modelFadeObject = lbl_1_bss_C;
        modelFadeObject->work[0] = 2;
        modelFadeObject->work[1] = 0;
        labelFadeObject = lbl_1_bss_C;
        labelFadeObject->work[2] = 2;
        labelFadeObject->work[3] = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_80[playerIndex].x = 0.0f;
        }
    }
    for (playerIndex = 0, playerConfig = lbl_1_bss_7CC; playerIndex < 4;
         playerIndex++, playerConfig++) {
        fn_1_A330((s16)playerConfig->charNo, (s16)controllerObject->work[0], 10);
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* When the start prompt returns to mode selection, moves player cards back to their previous row.
 * In single-player mode, players one through three receive CPU difficulty badges regardless of
 * their human/CPU settings. */
void fn_1_148B4(OMOBJ *controllerObject)
{
    MDMinidllPlayerConfig *playerConfig;
    s16 playerIndex;

    if (lbl_1_bss_804[1] == 5) {
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            if (playerIndex == 0) {
                fn_1_A700((s16) playerConfig->charNo, (s16) controllerObject->work[0], 10,
                          &lbl_1_data_4C8[playerIndex + 25], 1.25f);
            } else {
                fn_1_809C(playerIndex, playerConfig->comDif + 5);
                fn_1_9F24((s16) playerConfig->charNo, (s16) controllerObject->work[0], 10,
                          &lbl_1_data_4C8[playerIndex + 25], 1.25f);
            }
            playerIndex += 1;
            playerConfig += 1;
        }
    } else {
        playerIndex = 0;
        playerConfig = lbl_1_bss_7CC;
        while (playerIndex < 4) {
            fn_1_A700((s16) playerConfig->charNo, (s16) controllerObject->work[0], 10,
                      &lbl_1_data_4C8[playerIndex + 25], 1.25f);
            playerIndex += 1;
            playerConfig += 1;
        }
    }
    if (controllerObject->work[0]++ > 10U) {
        controllerObject->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* On menu exit, releases the initial menu, animated display and host models together with their
 * object-manager slots. */
static inline void fn_1_157B4_inlined(void)
{
    OMOBJ *hostObject;
    OMOBJ *displayObject;
    s16 displayModelIndex;
    OMOBJ *menuObject;
    s16 menuModelIndex;

    menuObject = lbl_1_bss_8;
    if (menuObject) {
        menuObject->objFunc = NULL;
        menuModelIndex = 0;
        while (menuModelIndex < 3) {
            Hu3DMotionKill(menuObject->mtnId[menuModelIndex]);
            Hu3DModelKill(menuObject->mdlId[menuModelIndex]);
            menuModelIndex += 1;
        }
        omDelObjEx(lbl_1_bss_4, menuObject);
    }
    /* Clearing these locals leaves the shared object pointers unchanged. */
    menuObject = NULL;
    displayObject = lbl_1_bss_C;
    if (displayObject) {
        displayModelIndex = 0;
        while (displayModelIndex < 2) {
            Hu3DAnimKill(displayObject->mtnId[displayModelIndex * 2]);
            Hu3DAnimKill(displayObject->mtnId[displayModelIndex * 2 + 1]);
            displayObject->mtnId[displayModelIndex * 2] = -1;
            displayObject->mtnId[displayModelIndex * 2 + 1] = -1;
            displayModelIndex += 1;
        }
        displayModelIndex = 1;
        while (displayModelIndex >= 0) {
            Hu3DModelKill(displayObject->mdlId[displayModelIndex]);
            displayModelIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_4, displayObject);
    }
    displayObject = NULL;
    hostObject = lbl_1_bss_10;
    if (hostObject) {
        Hu3DModelHookReset(hostObject->mdlId[1]);
        Hu3DMotionKill(hostObject->mtnId[0]);
        Hu3DMotionKill(hostObject->mtnId[1]);
        Hu3DMotionKill(hostObject->mtnId[2]);
        Hu3DMotionKill(hostObject->mtnId[3]);
        Hu3DMotionKill(hostObject->mtnId[4]);
        Hu3DModelKill(hostObject->mdlId[0]);
        Hu3DModelKill(hostObject->mdlId[1]);
        omDelObjEx(lbl_1_bss_4, hostObject);
    }
    /* The callback is cleared unconditionally after the object-manager slot is released. */
    hostObject->objFunc = NULL;
}

/* The exit process releases the initial menu, animated display and host objects before saving and
 * returning. */
void fn_1_157B4(void)
{
    fn_1_157B4_inlined();
}

/* Menu teardown releases selection cards, windows, lights and the camera-view object. */
void fn_1_159BC(void)
{
    MDMinidllCameraState *cameraState;
    s16 cardIndex;
    s16 animationSlot;
    s16 windowIndex;

    fn_1_257D0();
    if (lbl_1_bss_24) {
        *lbl_1_bss_24->mtnId = -1;
        cardIndex = 20;
        while (cardIndex >= 0) {
            animationSlot = 0;
            while (animationSlot < 4) {
                Hu3DAnimKill(lbl_1_bss_230[cardIndex].animation[animationSlot]);
                animationSlot++;
            }
            Hu3DModelKill(lbl_1_bss_230[cardIndex].model);
            cardIndex--;
        }
    }
    lbl_1_bss_24 = NULL;
    windowIndex = 0;
    while (windowIndex < 4) {
        HuWinExKill(lbl_1_bss_858[windowIndex]);
        windowIndex++;
    }
    HuWinAllKill();
    Hu3DGLightKill(lbl_1_bss_860[0]);
    Hu3DGLightKill(lbl_1_bss_860[1]);
    cameraState = &lbl_1_bss_808;
    Hu3DCameraKill(1);
    if (cameraState->outViewObj) {
        omDelObjEx(lbl_1_bss_4, cameraState->outViewObj);
    }
    cameraState->outViewObj = NULL;
}

/* The system-exit callback waits for the outgoing wipe, releases menu resources and returns to the
 * previous overlay. */
void fn_1_15B3C(OMOBJ *unusedObject)
{
    MDMinidllCameraState *cameraState;
    s16 cardIndex;
    s16 animationSlot;
    s16 windowIndex;

    if (WipeCheck() == 0) {
        fn_1_257D0();
        if (lbl_1_bss_24) {
            *lbl_1_bss_24->mtnId = -1;
            cardIndex = 20;
            while (cardIndex >= 0) {
                animationSlot = 0;
                while (animationSlot < 4) {
                    Hu3DAnimKill(lbl_1_bss_230[cardIndex].animation[animationSlot]);
                    animationSlot++;
                }
                Hu3DModelKill(lbl_1_bss_230[cardIndex].model);
                cardIndex--;
            }
        }
        lbl_1_bss_24 = NULL;
        windowIndex = 0;
        while (windowIndex < 4) {
            HuWinExKill(lbl_1_bss_858[windowIndex]);
            windowIndex++;
        }
        HuWinAllKill();
        Hu3DGLightKill(lbl_1_bss_860[0]);
        Hu3DGLightKill(lbl_1_bss_860[1]);
        cameraState = &lbl_1_bss_808;
        Hu3DCameraKill(1);
        if (cameraState->outViewObj) {
            omDelObjEx(lbl_1_bss_4, cameraState->outViewObj);
        }
        cameraState->outViewObj = NULL;
        omOvlReturnEx(1, 1);
    }
}

/* The object manager checks the system exit request each update and starts the wipe before
 * switching to menu teardown. */
void fn_1_15CD8(OMOBJ *exitObject)
{
    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        exitObject->objFunc = fn_1_15B3C;
    }
}

/* During menu dismissal, hides all four message windows, shows the exit sprite group and waits five
 * scheduler ticks. */
void fn_1_15D30(void)
{
    s16 windowIndex;
    s16 memberIndex;
    HUSPR_GROUP *spriteGroup;
    s16 spriteGroupId;

    windowIndex = 0;
    while (windowIndex < 4) {
        HuWinDispOff(lbl_1_bss_858[windowIndex]);
        windowIndex += 1;
    }
    HuSprPriSet(lbl_1_bss_752[8], 0, 5500);
    spriteGroupId = lbl_1_bss_752[8];
    spriteGroup = &HuSprGrpData[spriteGroupId];
    memberIndex = 0;
    while (memberIndex < spriteGroup->sprNum) {
        HuSprAttrReset(spriteGroupId, memberIndex, HUSPR_ATTR_DISPOFF);
        memberIndex += 1;
    }
    HuPrcSleep(5);
}

#include "game/saveload.h"
s16 fn_1_1E86C(s32 unused);
/* The child menu process runs selection, then wipes out and either saves before returning or waits
 * for loading and enters the chosen mode. */
void fn_1_15E0C(void)
{
    OMOVLHIS *history;
    s16 menuResult;
    s16 windowIndex;
    menuResult = 0;
    menuResult = fn_1_1E86C(0);
    if (lbl_1_data_94 != - 1)
    {
        HuAudSStreamFadeOut(lbl_1_data_94, 1000);
        lbl_1_data_94 = - 1;
    }
    switch (menuResult)
    {
    case 0 :
    case 1 :
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0)
        {
            HuPrcVSleep();
        }
        fn_1_157B4();
        HuPrcSleep(5);
        windowIndex = 0;
        while (windowIndex < 4)
        {
            HuWinDispOff(lbl_1_bss_858[windowIndex]);
            windowIndex += 1;
        }
        HuSprPriSet(lbl_1_bss_752[8], 0, 5500);
        fn_1_44B8(lbl_1_bss_752[8], HUSPR_ATTR_DISPOFF);
        HuPrcSleep(5);
        _ClearFlag(FLAG_INST_MG_MODE);
        _ClearFlag(FLAG_INST_DECA);
        _ClearFlag(FLAG_INST_NO_HISCHG);
        _ClearFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
        GWBankStarAdd((u16)(GWMgPlayNumGet() / 10));
        SLSaveModeExec(0);
        break;
    case 2 :
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0)
        {
            HuPrcVSleep();
        }
        break;
    }
    if (menuResult == 2)
    {

        do
        {
            HuPrcVSleep();
        }
        while (lbl_1_bss_2C == 0);
        history = omOvlHisGet(0);
        omOvlHisChg(0, history->ovl, 1, 0);
        OSReport(lbl_1_data_798);
        OSReport(lbl_1_data_6F3, 33);
        OSReport(lbl_1_data_704, 36);
        OSReport(lbl_1_data_716, 155);
        OSReport(lbl_1_data_728, 242);
        HuAMemDump();
        OSReport(lbl_1_data_736);
        switch (lbl_1_bss_804[1])
        {
        case 0 :
            omOvlCallEx(DLL_mgmfreedll, 1, 0, 0);
            break;
        case 1 :
            omOvlCallEx(DLL_mgmbattledll, 1, 0, 0);
            break;
        case 2 :
            omOvlCallEx(DLL_mgmbingodll, 1, 0, 0);
            break;
        case 3 :
            omOvlCallEx(DLL_mgmtournamentdll, 1, 0, 0);
            break;
        case 4 :
            omOvlCallEx(DLL_mgmdecathlondll, 1, 0, 0);
            break;
        case 5 :
            omOvlCallEx(DLL_mgmrenshodll, 1, 0, 0);
            break;
        }

    }
    else
    {
        CharDataClose(CHARNO_NONE);
        HuARDirFree(DATA_mgmfree);
        HuARDirFree(DATA_mgmbattle);
        HuARDirFree(DATA_mgmbingo);
        HuARDirFree(DATA_mgmtournament);
        HuARDirFree(DATA_mgmdecathlon);
        OSReport(lbl_1_data_767);
        OSReport(lbl_1_data_6F3, 33);
        OSReport(lbl_1_data_704, 36);
        OSReport(lbl_1_data_716, 155);
        OSReport(lbl_1_data_728, 242);
        HuAMemDump();
        OSReport(lbl_1_data_736);
        omOvlReturnEx(1, 1);
    }
    HuPrcEnd();
    while (1)
    {
        HuPrcVSleep();
    }
}

void fn_1_17294(OMOBJ *, MDMinidllCameraState *);
void fn_1_24D80(HUPROCESS *);
/* Overlay initialization creates the menu object manager, camera, sprites, windows, selection cards
 * and the child selection process. */
void fn_1_16400(void)
{
    lbl_1_bss_4 = omInitObjMan(27, 8192);
    omGameSysInit(lbl_1_bss_4);
    fn_1_30A4(fn_1_17294);
    fn_1_329C();
    fn_1_3720();
    fn_1_4380();
    fn_1_4538();
    fn_1_B3E0();
    fn_1_24D80(lbl_1_bss_4);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_7048);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_7C4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_5BF4);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 32U, -1, fn_1_6B74);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_8430);
    lbl_1_bss_1C[0] = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_90C8);
    HuPrcChildCreate(fn_1_15E0C, 12288U, 12288U, 0, lbl_1_bss_4);
}

#include "game/mgdata.h"
/* Overlay entry releases prior mode archives, refreshes unlock availability and resets minigame
 * flags before creating the menu. */
void fn_1_16F28(void)
{
    OSReport(lbl_1_data_960);
    HuARDirFree(DATA_mgmfree);
    HuARDirFree(DATA_mgmbattle);
    HuARDirFree(DATA_mgmbingo);
    HuARDirFree(DATA_mgmtournament);
    HuARDirFree(DATA_mgmdecathlon);
    OSReport(lbl_1_data_738);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
    MgPauseExitF = 0;
    lbl_1_bss_28 = 0;
    lbl_1_bss_2A = 0;
    if (GWBankFlagGet(2) != 0) lbl_1_bss_28 = 1;
    if (GWBankFlagGet(3) != 0) lbl_1_bss_2A = 1;
    lbl_1_data_6B6[0] = 1;
    lbl_1_data_6B6[1] = 1;
    lbl_1_data_6B6[2] = 1;
    lbl_1_data_6B6[3] = 1;
    lbl_1_data_6B6[4] = 1;
    lbl_1_data_6B6[5] = 0;
    if (GWBankFlagGet(7) != 0) lbl_1_data_6B6[5] = 1;
    lbl_1_data_6B6[1] = fn_1_1A4(1);
    lbl_1_data_6B6[2] = fn_1_1A4(2);
    lbl_1_data_6B6[3] = fn_1_1A4(3);
    lbl_1_data_6B6[4] = fn_1_1A4(4);
    if (lbl_1_data_6B6[5] == 1) lbl_1_data_6B6[5] = fn_1_1A4(5);
    lbl_1_bss_0 = (s16)omovlevtno;
    if (lbl_1_bss_0 == 0) GWMgPlayNumSet(0U);
    MgInstExitF = 0;
    MgPauseExitF = 0;
    MgExitReq = 0;
    _SetFlag(FLAG_INST_MG_MODE);
    _ClearFlag(FLAG_INST_DECA);
    _ClearFlag(FLAG_INST_NO_HISCHG);
    _ClearFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
    fn_1_16400();
}

typedef void (*MDMinidllVoidFunc)(void);
extern const MDMinidllVoidFunc _ctors[];
extern const MDMinidllVoidFunc _dtors[];
void fn_1_16F28(void);

/* Overlay entry runs each constructor, then initializes the minigame-mode menu. */
int _prolog(void)
{
    const MDMinidllVoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_16F28();
    return 0;
}

/* Overlay unload runs each registered destructor. */
void _epilog(void)
{
    const MDMinidllVoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

/* Menu camera updates set the target view and ease the current center, rotation and zoom toward
 * it. */
void fn_1_17294(OMOBJ *unusedObject, MDMinidllCameraState *cameraState)
{
    cameraState->centerTarget.x = 0.0f;
    cameraState->centerTarget.y = 65.0f;
    cameraState->centerTarget.z = -800.0f;
    cameraState->rotationTarget.x = -7.25f;
    cameraState->rotationTarget.y = 0.0f;
    cameraState->rotationTarget.z = 0.0f;
    cameraState->zoomTarget = 2150.0f;
    fn_1_25B4(cameraState, 15.0f);
}

/* During the final departure sequence, moves the camera center back and upward while leveling its
 * pitch over sixty frames. */
void fn_1_175F8(OMOBJ *unusedObject, MDMinidllCameraState *cameraState)
{
    cameraState->center.y =
        fn_1_1868_inlined(cameraState->centerTarget.y, 275.0f, cameraState->transitionFrame, 60.0f);
    cameraState->center.z = fn_1_1868_inlined(cameraState->centerTarget.z, -2900.0f,
                                              cameraState->transitionFrame, 60.0f);
    cameraState->rotation.x =
        fn_1_1868_inlined(cameraState->rotationTarget.x, 0.0f, cameraState->transitionFrame, 60.0f);
    cameraState->transitionFrame += 1.0f;
}

/* A camera transition setup callback restores the saved target view and starts the departure-camera
 * update. */
void fn_1_17820(OMOBJ *unusedObject, MDMinidllCameraState *cameraState)
{
    cameraState->transitionFrame = 0.0f;
    memcpy(&cameraState->center, &cameraState->centerTarget, 12U);
    memcpy(&cameraState->rotation, &cameraState->rotationTarget, 12U);
    cameraState->zoom = cameraState->zoomTarget;
    lbl_1_bss_808.callback = fn_1_175F8;
}

/* When returning from a played mode, restores player choices, controller or CPU badges and selected
 * character cards from the saved player settings. */
void fn_1_17894(void)
{
    s16 selectionIndex;
    s16 animationIndex;
    s16 animationSlot;
    s16 padNumber;
    s16 difficultyBank;
    s16 characterIndex;
    MDMinidllModelAnimationRecord *animationCard;
    MDMinidllModelAnimationRecord *backCard;
    MDMinidllModelAnimationRecord *alternateBackCard;
    MDMinidllModelAnimationRecord *frontCard;
    MDMinidllModelAnimationRecord *alternateFrontCard;
    MDMinidllModelAnimationRecord *selectedCard;
    s16 humanMemberIndex;
    s16 cpuMemberIndex;
    HUSPR_GROUP *humanBadgeGroup;
    s16 humanBadgeGroupId;
    HUSPR_GROUP *cpuBadgeGroup;
    s16 cpuBadgeGroupId;

    lbl_1_bss_804[0] = 0;
    lbl_1_bss_804[1] = 0;
    for (selectionIndex = 0; selectionIndex < 4; selectionIndex += 1) {
        lbl_1_bss_7CC[selectionIndex].selectionPhase = 0;
        lbl_1_bss_7CC[selectionIndex].unusedSetupValue = 0;
        lbl_1_bss_7CC[selectionIndex].comF = GwPlayer[selectionIndex].comF;
        lbl_1_bss_7CC[selectionIndex].comDif = GwPlayer[selectionIndex].comDif;
        lbl_1_bss_7CC[selectionIndex].charNo = GwPlayer[selectionIndex].charNo;
        lbl_1_bss_7CC[selectionIndex].padNo = GwPlayer[selectionIndex].padNo;
        if (lbl_1_bss_7CC[selectionIndex].comF == 0) {
            padNumber = lbl_1_bss_7CC[selectionIndex].padNo;
            lbl_1_bss_80[selectionIndex].x = 0.85000002384185791f;
            lbl_1_bss_80[selectionIndex].y = 0.85000002384185791f;
            humanBadgeGroupId = lbl_1_bss_752[selectionIndex + 4];
            humanBadgeGroup = &HuSprGrpData[humanBadgeGroupId];
            humanMemberIndex = 0;
            while (humanMemberIndex < humanBadgeGroup->sprNum) {
                HuSprAttrReset(humanBadgeGroupId, humanMemberIndex, HUSPR_ATTR_DISPOFF);
                humanMemberIndex += 1;
            }
            HuSprBankSet(lbl_1_bss_752[selectionIndex + 4], 0, padNumber);
            lbl_1_bss_804[0] += 1;
        } else {
            difficultyBank = lbl_1_bss_7CC[selectionIndex].comDif + 5;
            lbl_1_bss_80[selectionIndex].x = 0.85000002384185791f;
            lbl_1_bss_80[selectionIndex].y = 0.85000002384185791f;
            cpuBadgeGroupId = lbl_1_bss_752[selectionIndex + 4];
            cpuBadgeGroup = &HuSprGrpData[cpuBadgeGroupId];
            cpuMemberIndex = 0;
            while (cpuMemberIndex < cpuBadgeGroup->sprNum) {
                HuSprAttrReset(cpuBadgeGroupId, cpuMemberIndex, HUSPR_ATTR_DISPOFF);
                cpuMemberIndex += 1;
            }
            HuSprBankSet(lbl_1_bss_752[selectionIndex + 4], 0, difficultyBank);
        }
    }

    for (selectionIndex = 0; selectionIndex < 11; selectionIndex += 1) {
        for (animationIndex = 0; animationIndex < 4; animationIndex += 1) {
            animationSlot = animationIndex;
            animationCard = &lbl_1_bss_230[selectionIndex];
            Hu3DAnimAnimSet(animationCard->animation[animationSlot],
                            lbl_1_bss_6C8[selectionIndex]);
        }
        backCard = &lbl_1_bss_230[selectionIndex];
        Hu3DAnimAnimSet(backCard->animation[1], lbl_1_bss_6C8[selectionIndex]);
        Hu3DAnimBankSet(backCard->animation[1], 2U);
        alternateBackCard = &lbl_1_bss_230[selectionIndex];
        Hu3DAnimAnimSet(alternateBackCard->animation[3], lbl_1_bss_6C8[selectionIndex]);
        Hu3DAnimBankSet(alternateBackCard->animation[3], 2U);
        frontCard = &lbl_1_bss_230[selectionIndex];
        Hu3DAnimAnimSet(frontCard->animation[0], lbl_1_bss_6C8[selectionIndex]);
        Hu3DAnimBankSet(frontCard->animation[0], 1U);
        alternateFrontCard = &lbl_1_bss_230[selectionIndex];
        Hu3DAnimAnimSet(alternateFrontCard->animation[2], lbl_1_bss_6C8[selectionIndex]);
        Hu3DAnimBankSet(alternateFrontCard->animation[2], 1U);
    }

    selectionIndex = 0;
    while (selectionIndex < 4) {
        characterIndex = (s16)lbl_1_bss_7CC[selectionIndex].charNo;
        selectedCard = &lbl_1_bss_230[characterIndex];
        Hu3DModelPosSet(selectedCard->model,
                        lbl_1_data_4C8[selectionIndex + 25].x,
                        lbl_1_data_4C8[selectionIndex + 25].y,
                        lbl_1_data_4C8[selectionIndex + 25].z);
        Hu3DModelScaleSet(selectedCard->model, 1.2500000000000000f,
                          1.2500000000000000f, 1.0f);
        Hu3DModelAttrReset(selectedCard->model, HU3D_ATTR_DISPOFF);
        selectionIndex += 1;
    }
}

/* The menu-opening helper restores player settings on return, starts menu music and waits for the
 * incoming wipe. */
s32 fn_1_17E78(void)
{
    HuPrcSleep(5);
    if (lbl_1_bss_0 == 1) fn_1_17894();
    lbl_1_data_94 = HuAudSStreamPlay(MSM_STREAM_W02);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck() != 0) HuPrcVSleep();
    return 1;
}

/* For a fresh menu entry, shows the opening message, waits for it to finish and fades the opening
 * sprites out. */
s32 fn_1_17EF4(void)
{
    fn_1_4E40();
    fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 0), 1);
    fn_1_3BCC();
    fn_1_7014();
    return 1;
}

/* When a selection step is cancelled, shows the exit-confirmation prompt and returns its choice. */
s16 fn_1_181CC(void)
{
    fn_1_4E40();
    fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 2), 1);
    return fn_1_3C2C(2);
}

/* Before the player-count prompt, requests host movement and closes the message window if the host
 * is left of the dismissal threshold. */
s32 fn_1_184D4(void)
{
    Point3d position;
    Hu3DModelPosGet(*lbl_1_bss_10->mdlId, &position);
    if (position.x < 200.0f) {
        /* The move helper can decline; the window still closes and the delay still runs. */
        fn_1_5B48();
        fn_1_3B10();
        HuPrcSleep(60);
    }
}

/* The menu state process dismisses the host as needed, then shows and waits for the player-count
 * introduction. */
s32 fn_1_18650(void)
{
    fn_1_184D4();
    fn_1_4E40();
    fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 1), 1);
    fn_1_3BCC();
    return 1;
}

void fn_1_26394(void);
s16 fn_1_19940(s16);
s16 fn_1_1A6C0(s16);
/* The player-count step shows its cards and prompt and accepts confirmation or cancellation.
* It removes the cards after confirmation; confirmed cancellation leaves them displayed. */
s16 fn_1_18A5C(s16 unusedSelectionState)
{
    OMOBJ *hostObject;
    s16 selectionResult = 0;

    fn_1_B348(fn_1_C770);
    while (1) {
        HuPrcVSleep();
        fn_1_B348(fn_1_C6A8);
        fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
        hostObject = lbl_1_bss_10;
        Hu3DMotionShiftSet(*hostObject->mdlId, hostObject->mtnId[3], 0.0f, 10.0f, 0U);
        hostObject->work[3] = 0;
        hostObject->objFunc = fn_1_4D98;
        fn_1_3D24(2, lbl_1_bss_804[0] + MESSNUM(MESS_PARTY_SETTING, 9), 0);
        while (1) {
            HuPrcVSleep();
            fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
            fn_1_3D24(2, lbl_1_bss_804[0] + MESSNUM(MESS_PARTY_SETTING, 9), 0);
            if (fn_1_B310() == 0) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                fn_1_2460();
                selectionResult = 0;
                break;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectionResult = -1;
                break;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectionResult = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_B348(fn_1_C6D8);
        if (selectionResult == -1) {
            if (fn_1_181CC()) {
                continue;
            }
            selectionResult = -1;
        }
        break;
    }
    if (selectionResult != -1) {
        fn_1_B348(fn_1_D09C);
    }
    return selectionResult;
}

/* During human character selection, reports completion when every human has confirmed, or whether
 * player one can cancel. */
s16 fn_1_198A0(void)
{
    s16 playerIndex;
    s16 confirmedPlayers;
    playerIndex = 0;
    confirmedPlayers = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_7CC[playerIndex].selectionPhase == 1) {
            confirmedPlayers += 1;
        }
        playerIndex += 1;
    }
    if (lbl_1_bss_804[0] <= confirmedPlayers) {
        return 0;
    }
    if (lbl_1_bss_7CC[0].selectionPhase == 0) {
        return 1;
    }
    return -1;
}

/* The character-selection process waits for human choices and handles back or exit input before
 * dismissing the selection arrows. */
s16 fn_1_19940(s16 returningFromPreviousStep)
{
    s16 selectionResult = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_B348(fn_1_E0B0);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
        fn_1_4E40();
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 7), 0);
        fn_1_B348(fn_1_DD00);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 7), 0);
            selectionResult = fn_1_198A0();
            if (!fn_1_B310()) {
                continue;
            }
            if (selectionResult == 0) {
                selectionResult = 0;
                break;
            }
            if (selectionResult == 1 && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectionResult = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectionResult = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_B348(fn_1_DF64);
        if (selectionResult != -1) {
            break;
        }
        if (!fn_1_181CC()) {
            selectionResult = -1;
            break;
        }
    }
    return selectionResult;
}

/* The CPU-selection process queries the first unfinished CPU, the first CPU slot and the number of
 * CPU choices still pending. */
s16 fn_1_1A58C(s16 *pendingPlayerIndex, s16 *firstCpuIndex, s16 *lastPlayerIndex)
{
    s16 playerIndex;
    s16 pendingCpuCount;

    pendingCpuCount = 0;
    *pendingPlayerIndex = 0;
    *firstCpuIndex = 0;
    *lastPlayerIndex = 3;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_7CC[playerIndex].comF == 1) {
            *firstCpuIndex = playerIndex;
        } else {
            playerIndex += 1;
            continue;
        }
        break;
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        if ((lbl_1_bss_7CC[playerIndex].comF != 1) ||
            (lbl_1_bss_7CC[playerIndex].selectionPhase == 2)) {
            playerIndex += 1;
            continue;
        }
        break;
    }
    *pendingPlayerIndex = playerIndex;
    playerIndex = 0;
    while (playerIndex < 4) {
        if ((lbl_1_bss_7CC[playerIndex].comF == 1) &&
            (lbl_1_bss_7CC[playerIndex].selectionPhase != 2)) {
            pendingCpuCount += 1;
        }
        playerIndex += 1;
    }
    return pendingCpuCount;
}

/* The CPU-selection step shows character or difficulty prompts and waits until all CPU choices
 * finish, the player goes back or exit is confirmed. */
s16 fn_1_1A6C0(s16 unusedSelectionState)
{
    s16 selectionResult;
    s16 pendingPlayerIndex;
    s16 firstCpuIndex;
    s16 lastPlayerIndex;
    s16 pendingCpuCount;

    selectionResult = 0;
    pendingPlayerIndex = 0;
    firstCpuIndex = 0;
    lastPlayerIndex = 0;
    pendingCpuCount = 0;
    pendingCpuCount = fn_1_1A58C(&pendingPlayerIndex, &firstCpuIndex, &lastPlayerIndex);
again:
    HuPrcVSleep();
    fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
    fn_1_4E40();
    if (lbl_1_bss_7CC[pendingPlayerIndex].selectionPhase == 0) {
        fn_1_3F5C(2, MESSNUM(MESS_PARTY_SETTING, 19) - (pendingCpuCount - 1), 0);
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 8), 0);
    } else {
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 12), 0);
    }
    fn_1_B348(fn_1_F0A8);
poll:
    HuPrcVSleep();
    if (lbl_1_bss_7CC[pendingPlayerIndex].selectionPhase == 0) {
        fn_1_3F5C(2, MESSNUM(MESS_PARTY_SETTING, 19) - (pendingCpuCount - 1), 0);
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 8), 0);
    } else {
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 12), 0);
    }
    if (fn_1_B310() == 0) {
        goto poll;
    }
    pendingCpuCount = fn_1_1A58C(&pendingPlayerIndex, &firstCpuIndex, &lastPlayerIndex);
    if (pendingPlayerIndex > lastPlayerIndex) {
        selectionResult = 0;
    } else if ((firstCpuIndex == pendingPlayerIndex) &&
               (lbl_1_bss_7CC[pendingPlayerIndex].selectionPhase == 0) &&
               ((HuPadBtnDown[0] & PAD_BUTTON_B) != 0)) {
        HuAudFXPlay(MSM_SE_CMN_04);
        selectionResult = 1;
    } else if ((HuPadBtnDown[0] & PAD_BUTTON_X) != 0) {
        HuAudFXPlay(MSM_SE_CMN_04);
        selectionResult = -1;
    } else {
        goto poll;
    }
    fn_1_42D4();
    fn_1_B348(fn_1_F52C);
    if (selectionResult == -1) {
        if (!fn_1_181CC()) {
            selectionResult = -1;
        } else {
            goto again;
        }
    }
    return selectionResult;
}

/* The menu state process runs human and CPU character selection, then moves confirmed cards to the
 * mode layout or dismisses them when going back. */
s16 fn_1_1BCAC(s16 returningFromModeSelection)
{
    s16 selectionResult;
    s16 returningFromCpuSelection;
    selectionResult = 0;
    returningFromCpuSelection = 0;
    if (returningFromModeSelection != 0) {
        fn_1_B348(fn_1_105DC);
    }
    fn_1_B348(fn_1_F6F8);
    switch (lbl_1_bss_804[0]) {
    case 0:
        selectionResult = fn_1_1A6C0(0);
        break;
    case 4:
        selectionResult = fn_1_19940(0);
        break;
    default:
        while (1) {
            HuPrcVSleep();
            selectionResult = fn_1_19940(returningFromCpuSelection);
            if (selectionResult != 0) {
                break;
            }
            selectionResult = fn_1_1A6C0(0);
            if (selectionResult != 1) {
                break;
            }
            returningFromCpuSelection = 1;
        }
        break;
    }
    if (selectionResult == 0) {
        fn_1_B348(fn_1_FCC0);
    } else if (selectionResult == 1) {
        fn_1_B348(fn_1_109D4);
    }
    return selectionResult;
}

typedef struct { s32 message[6]; } ModeMessages;
const ModeMessages lbl_1_rodata_270 = {
    { MESSNUM(MESS_MGM_MAIN_MENU, 13), MESSNUM(MESS_MGM_MAIN_MENU, 14),
      MESSNUM(MESS_MGM_MAIN_MENU, 17), MESSNUM(MESS_MGM_MAIN_MENU, 15),
      MESSNUM(MESS_MGM_MAIN_MENU, 16), MESSNUM(MESS_MGM_MAIN_MENU, 18) }
};

/* The menu state process runs six-mode selection, rejects locked modes and handles confirmation,
 * back and exit transitions. */
s16 fn_1_1BF60(s16 returningFromStartPrompt)
{
    ModeMessages messages;
    s16 selectionResult;

    selectionResult = 0;
    messages = lbl_1_rodata_270;
    if (returningFromStartPrompt != 0) {
        fn_1_B348(fn_1_1240C);
    }
    fn_1_B348(fn_1_11E14);
restart:
    HuPrcVSleep();
    fn_1_B348(fn_1_118A0);
    fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
    fn_1_4E40();
    if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 1) {
        fn_1_3D24(2, messages.message[lbl_1_bss_804[1]], 0);
    } else if ((lbl_1_bss_804[1] == 5) && (GWBankFlagGet(7) == 0)) {
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 23), 0);
    } else {
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 22), 0);
    }
    while (1) {
        HuPrcVSleep();
        if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 1) {
            fn_1_3D24(2, messages.message[lbl_1_bss_804[1]], 0);
        } else if ((lbl_1_bss_804[1] == 5) && (GWBankFlagGet(7) == 0)) {
            fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 23), 0);
        } else {
            fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 22), 0);
        }
        if (fn_1_B310() == 0) {
            continue;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 0) {
                HuAudFXPlay(MSM_SE_CMN_05);
                continue;
            }
            HuAudFXPlay(MSM_SE_CMN_03);
            selectionResult = 0;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            selectionResult = 1;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
            HuAudFXPlay(MSM_SE_CMN_04);
            selectionResult = -1;
        } else {
            continue;
        }
        break;
    }
    fn_1_42D4();
    fn_1_B348(fn_1_11B0C);
    if (selectionResult == -1) {
        if (fn_1_181CC()) {
            goto restart;
        }
        selectionResult = -1;
    }
    if (selectionResult == 0) {
        fn_1_B348(fn_1_124A0);
    } else if (selectionResult == 1) {
        fn_1_B348(fn_1_12E88);
    }
    return selectionResult;
}

/* After a mode is chosen, asks whether to start; declining offers setup restart or a return to mode
 * selection. */
s16 fn_1_1D640(void)
{
    OMOBJ *hostObject;
    OMOBJ *retryHostObject;
    s16 confirmationResult = 0;

    fn_1_B348(fn_1_1325C);
    HuPrcVSleep();
    hostObject = lbl_1_bss_10;
    Hu3DMotionShiftSet(*hostObject->mdlId, hostObject->mtnId[3], 0.0f, 10.0f, 0U);
    hostObject->work[3] = 0;
    hostObject->objFunc = fn_1_4D98;
    fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 19), 1);
    confirmationResult = fn_1_3C2C(2);
    if (confirmationResult == 0) {
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 21), 1);
        HuPrcSleep(60);
        Hu3DMotionShiftSet(*lbl_1_bss_10->mdlId, lbl_1_bss_10->mtnId[4],
                           0.0f, 5.0f, 0U);
        lbl_1_bss_30 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_4BE0);
        lbl_1_bss_30->work[0] = 0;
        HuAudFXPlay(MSM_SE_MENU_17);
        HuPrcSleep(60);
        fn_1_3B10();
        fn_1_B348(fn_1_1406C);
        confirmationResult = 0;
    } else {
        retryHostObject = lbl_1_bss_10;
        Hu3DMotionShiftSet(*retryHostObject->mdlId, retryHostObject->mtnId[3], 0.0f, 10.0f, 0U);
        retryHostObject->work[3] = 0;
        retryHostObject->objFunc = fn_1_4D98;
        fn_1_3D24(2, MESSNUM(MESS_MGM_MAIN_MENU, 20), 1);
        confirmationResult = fn_1_3C2C(2);
        if (confirmationResult == 0) {
            fn_1_B348(fn_1_14484);
            confirmationResult = 1;
        } else {
            fn_1_B348(fn_1_148B4);
            confirmationResult = 2;
        }
    }
    return confirmationResult;
}

void fn_1_1E1AC(void);

/* When the start prompt is accepted, runs the character and host departure sequence, fades menu
 * music and moves the camera away. */
void fn_1_1E1AC(void)
{
    fn_1_C90();
    fn_1_A4C();
    HuPrcSleep(10);
    fn_1_6520();
    fn_1_5434();
    HuPrcSleep(10);
    fn_1_26394();
    if (lbl_1_data_94 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_94, 3000);
        lbl_1_data_94 = -1;
    }
    HuAudFXPlay(MSM_SE_MENU_20);
    HuPrcSleep(90);
    fn_1_2868(fn_1_175F8);
    HuPrcSleep(60);
}

/* Called by fn_1_15E0C to open the menu and advance its mode-selection states. */
s16 fn_1_1E86C(s32 unused)
{
    s16 retry;
    s16 result;
    s16 state;

    state = 0;
    result = 0;
    retry = 0;
    HuPrcSleep(5);
    if (lbl_1_bss_0 == 1) {
        fn_1_17894();
    }
    lbl_1_data_94 = HuAudSStreamPlay(MSM_STREAM_W02);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_0 == 0) {
        fn_1_17EF4();
        state = 0;
        retry = 0;
    } else {
        state = 3;
        retry = 0;
    }
    while (1) {
        HuPrcVSleep();
        switch (state) {
        case 0:
            fn_1_18650();
            state = 1;
            retry = 0;
            break;
        case 1:
            result = fn_1_18A5C(retry);
            if (result == 0) {
                state = 2;
                retry = 0;
            }
            break;
        case 2:
            result = fn_1_1BCAC(retry);
            if (result == 0) {
                state = 3;
                retry = 0;
            } else if (result == 1) {
                state = 1;
                retry = 1;
            }
            break;
        case 3:
            result = fn_1_1BF60(retry);
            if (result == 0) {
                state = 4;
                retry = 0;
            } else if (result == 1) {
                state = 2;
                retry = 1;
            }
            break;
        case 4:
            result = fn_1_1D640();
            if (result == 0) {
                state = 5;
                retry = 0;
            } else if (result == 1) {
                state = 0;
                retry = 0;
            } else if (result == 2) {
                state = 3;
                retry = 1;
            }
            break;
        case 5:
            fn_1_1E1AC();
            return 2;
        }
        if (result == -1) {
            return 0;
        }
    }
}
