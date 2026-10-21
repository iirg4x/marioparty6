/* Light Up My Night sequence setup, player state, cameras, and effects. */
#define HuAudFXPlayVolPan M617UnusedHuAudFXPlayVolPanPrototype
#define CharModelVoicePanSet M617UnusedCharModelVoicePanSetPrototype
#define CharModelVoiceVolSet M617UnusedCharModelVoiceVolSetPrototype
#include <string.h>
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "msm_stream.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m617dll/shared.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "humath.h"
#include "game/printfunc.h"
#undef HuAudFXPlayVolPan
#undef CharModelVoicePanSet
#undef CharModelVoiceVolSet

void fn_1_20A4(void);
void fn_1_51B4(void);

HU3D_MOTIONID CharMotionCreate(s16 charNo, unsigned int dataNum);
int HuAudJinglePlay(s16 streamId);
int HuAudBGMPlay(s16 streamId);
void HuAudSStreamFadeOut(int streamNo, s32 speed);
void HuAudSStreamStop(int streamNo);
void CharModelVoicePanSet();
void CharModelVoiceVolSet();

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);
void fn_1_118(s16 mode, s16 frameNo);
void fn_1_280(s16 mode, s16 frameNo);
void fn_1_2AC(s16 mode, s16 frameNo);
void fn_1_2D8(s16 mode, s16 frameNo);
void fn_1_304(s16 mode, s16 frameNo);
void fn_1_330(s16 mode, s16 frameNo);
void fn_1_35C(s16 mode, s16 frameNo);
void fn_1_388(s16 mode, s16 frameNo);
void fn_1_3B4(s16 mode, s16 frameNo);

typedef struct M617Vector3Pair {
    /* Starting positions and facing rotations for the two participants. */
    Vec vectors[2];
} M617Vector3Pair;

typedef struct M617PlayerLookup {
    s16 playerByGroup[2]; /* First configured player for group 0 or 1; -1 if absent. */
} M617PlayerLookup;

void fn_1_1780(s16 playerIndex);
void fn_1_93D0(void);
void fn_1_51B4(void);

/* MgSeq calls this start callback to clear minigame state and initialize the first scene. */
void fn_1_118(s16 mode, s16 frameNo)
{
    s16 nightFlag;

    memset(lbl_1_bss_379C, 0, sizeof(lbl_1_bss_379C));
    memset(&lbl_1_bss_3760, 0, sizeof(lbl_1_bss_3760));
    memset(&lbl_1_bss_3710, 0, sizeof(lbl_1_bss_3710));
    memset(&lbl_1_bss_3708, 0, sizeof(lbl_1_bss_3708));
    memset(lbl_1_bss_E88, 0, sizeof(lbl_1_bss_E88));
    memset(lbl_1_bss_DC0, 0, sizeof(lbl_1_bss_DC0));
    memset(lbl_1_bss_AB0, 0, sizeof(lbl_1_bss_AB0));
    memset(&lbl_1_bss_9C8, 0, sizeof(lbl_1_bss_9C8));
    memset(&lbl_1_bss_9BC, 0, sizeof(lbl_1_bss_9BC));
    memset(lbl_1_bss_458, 0, sizeof(lbl_1_bss_458));
    memset(lbl_1_bss_398, 0, sizeof(lbl_1_bss_398));
    memset(lbl_1_bss_370, 0, sizeof(lbl_1_bss_370));
    memset(lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    nightFlag = GwMgNightF;
    lbl_1_bss_9C8.nightFlag = nightFlag;
    fn_1_1780(0);
    fn_1_20A4();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_93D0();
    fn_1_51B4();
    MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_A3BC reports completion. */
void fn_1_280(s16 mode, s16 frameNo)
{
    if (fn_1_A3BC() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_A3FC reports completion. */
void fn_1_2AC(s16 mode, s16 frameNo)
{
    if (fn_1_A3FC() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_5C3C reports completion. */
void fn_1_2D8(s16 mode, s16 frameNo)
{
    if (fn_1_5C3C() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_D3CC reports completion. */
void fn_1_304(s16 mode, s16 frameNo)
{
    if (fn_1_D3CC() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_D424 reports completion. */
void fn_1_330(s16 mode, s16 frameNo)
{
    if (fn_1_D424() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_D474 reports completion. */
void fn_1_35C(s16 mode, s16 frameNo)
{
    if (fn_1_D474() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_D6B0 reports completion. */
void fn_1_388(s16 mode, s16 frameNo)
{
    if (fn_1_D6B0() != 0) MgSeqModeNext();
}

/* MgSeq calls this phase callback each frame; advance after fn_1_D6D4 reports completion. */
void fn_1_3B4(s16 mode, s16 frameNo)
{
    if (fn_1_D6D4() != 0) MgSeqModeNext();
}

void fn_1_3E0(void)
{
}

/* Participant setup uses this to select the first player in each group and build both records. */
void fn_1_3E4(void)
{
    M617PlayerLookup playerLookup = { { -1, -1 } };
    M617Vector3Pair positions = { { {-730.0f, 10.0f, -730.0f}, {650.0f, 10.0f, 650.0f} } };
    M617Vector3Pair startRotations = { { {0.0f, -135.0f, 0.0f}, {0.0f, 225.0f, 0.0f} } };
    s16 player = 0;
    s16 group = 0;

    for (; player < 4; player++) {
        group = GwPlayerConf[player].grpNo;

        if ((group == 0 || group == 1) && playerLookup.playerByGroup[group] == -1) {
            playerLookup.playerByGroup[group] = player;
        }
    }

    for (player = 0; player < 2; player++) {
        lbl_1_bss_AB0[player].playerIndex =
            (s8)playerLookup.playerByGroup[player];
        lbl_1_bss_AB0[player].participantIndex = (s8)player;
        lbl_1_bss_AB0[player].character =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].charNo;
        lbl_1_bss_AB0[player].controllerPort =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].padNo;
        lbl_1_bss_AB0[player].group =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].grpNo;
        lbl_1_bss_AB0[player].computerControlled =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].type;
        lbl_1_bss_AB0[player].difficulty =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].comDif;
        lbl_1_bss_AB0[player].position = positions.vectors[player];
        lbl_1_bss_AB0[player].rotation = startRotations.vectors[player];
        lbl_1_bss_AB0[player].resultState = -1;
    }
}

/* Frame updates copy all four pad states, but clear participant input for computer control or
 * nonzero motionState. Human stick X suppresses only negative values within ten of zero. */
void fn_1_730(void)
{
    s32 index;
    s32 stickResult;
    s32 deadZoneValue;

    index = 0;
    for (; index < 2; index++) {

        if (lbl_1_bss_AB0[index].computerControlled != 0 ||
            lbl_1_bss_AB0[index].motionState != 0) {
            memset(&lbl_1_bss_AB0[index], 0, 20);
        } else {
            lbl_1_bss_AB0[index].input.buttons =
                HuPadBtn[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.buttonsDown =
                HuPadBtnDown[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.buttonsRepeat =
                HuPadBtnRep[lbl_1_bss_AB0[index].controllerPort];

            if (HuPadStkX[lbl_1_bss_AB0[index].controllerPort] > 0) {
                stickResult = HuPadStkX[lbl_1_bss_AB0[index].controllerPort];
            } else {

                /* Negative stick values inside the dead zone are treated as zero. */
                if ((s8)HuPadStkX[lbl_1_bss_AB0[index].controllerPort] * -1 < 10) {
                    deadZoneValue = 0;
                } else {
                    deadZoneValue = HuPadStkX[lbl_1_bss_AB0[index].controllerPort];
                }
                stickResult = deadZoneValue;
            }
            lbl_1_bss_AB0[index].input.stickX = stickResult;
            lbl_1_bss_AB0[index].input.stickY =
                (s8)HuPadStkY[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.subStickX =
                (s8)HuPadSubStkX[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.subStickY =
                (s8)HuPadSubStkY[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.triggerL =
                HuPadTrigL[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.triggerR =
                HuPadTrigR[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.dpad =
                HuPadDStk[lbl_1_bss_AB0[index].controllerPort];
            lbl_1_bss_AB0[index].input.dpadRepeat =
                HuPadDStkRep[lbl_1_bss_AB0[index].controllerPort];
        }
    }

    index = 0;
    for (; index < 4; index++) {
        lbl_1_bss_9C8.padSnapshots[index].buttons = HuPadBtn[index];
        lbl_1_bss_9C8.padSnapshots[index].buttonsDown = HuPadBtnDown[index];
        lbl_1_bss_9C8.padSnapshots[index].buttonsRepeat = HuPadBtnRep[index];
        lbl_1_bss_9C8.padSnapshots[index].stickX = (s8)HuPadStkX[index];
        lbl_1_bss_9C8.padSnapshots[index].stickY = (s8)HuPadStkY[index];
        lbl_1_bss_9C8.padSnapshots[index].subStickX = (s8)HuPadSubStkX[index];
        lbl_1_bss_9C8.padSnapshots[index].subStickY = (s8)HuPadSubStkY[index];
        lbl_1_bss_9C8.padSnapshots[index].triggerL = HuPadTrigL[index];
        lbl_1_bss_9C8.padSnapshots[index].triggerR = HuPadTrigR[index];
        lbl_1_bss_9C8.padSnapshots[index].dpad = HuPadDStk[index];
        lbl_1_bss_9C8.padSnapshots[index].dpadRepeat = HuPadDStkRep[index];
    }
}

/* Refreshes scene-model transforms after stored positions and rotations change. */
void fn_1_CF0(void)
{
    s32 unusedIndex;
    s32 modelIndex;

    modelIndex = 2;
    while (modelIndex < 96) {
        Hu3DModelPosSet(lbl_1_bss_E88[modelIndex].modelId,
            lbl_1_bss_E88[modelIndex].position.x,
            lbl_1_bss_E88[modelIndex].position.y,
            lbl_1_bss_E88[modelIndex].position.z);
        Hu3DModelRotSet(lbl_1_bss_E88[modelIndex].modelId,
            lbl_1_bss_E88[modelIndex].rotation.x,
            lbl_1_bss_E88[modelIndex].rotation.y,
            lbl_1_bss_E88[modelIndex].rotation.z);
        modelIndex += 1;
    }
    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_AB0[modelIndex].transformState == 0 ||
            lbl_1_bss_AB0[modelIndex].transformState == 2) {
            Hu3DModelPosSet(lbl_1_bss_E88[modelIndex].modelId,
                lbl_1_bss_AB0[modelIndex].position.x,
                lbl_1_bss_AB0[modelIndex].position.y,
                lbl_1_bss_AB0[modelIndex].position.z);
            Hu3DModelRotSet(lbl_1_bss_E88[modelIndex].modelId,
                lbl_1_bss_AB0[modelIndex].rotation.x,
                lbl_1_bss_AB0[modelIndex].rotation.y,
                lbl_1_bss_AB0[modelIndex].rotation.z);
        }
        modelIndex += 1;
    }
}

typedef struct M617MotionFiles {
    u32 motionC000m1_300; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_306; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_307; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_432; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_433; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_434; /* Motion resource used by the corresponding motion slot. */
    u32 motionC000m1_612; /* Motion resource used by the corresponding motion slot. */
} M617MotionFiles;

const M617MotionFiles lbl_1_rodata_44 = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 7), DATANUM(DATA_mariomot, 95),
    DATANUM(DATA_mariomot, 96), DATANUM(DATA_mariomot, 97),
    DATANUM(DATA_mario, 148)
};

/* Called by fn_1_93D0 during scene setup to load seven motions for each participant. */
void fn_1_ECC(void)
{
    M617MotionFiles loadedFiles;
    s16 playerIndex;
    s16 motionIndex;

    loadedFiles = lbl_1_rodata_44;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        for (motionIndex = 0; motionIndex < 7; motionIndex++) {
            lbl_1_bss_DC0[playerIndex].motionIds[motionIndex] = CharMotionCreate(
                lbl_1_bss_AB0[playerIndex].character,
                ((u32 *)&loadedFiles)[motionIndex]);
        }
    }
}

#define motion (&lbl_1_bss_DC0[playerIndex])

/* Called by fn_1_24D8 each scene update to apply participant animation changes and motion
 * shifts. */
void fn_1_FBC(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 2) {
        if (motion->mode != motion->previousMode) {
            Hu3DMotionSet(lbl_1_bss_E88[playerIndex].modelId,
                motion->motionIds[motion->mode]);
            motion->previousMode = motion->mode;
            motion->maximumTime = Hu3DMotionMaxTimeGet(
                lbl_1_bss_E88[playerIndex].modelId);
        }
        if (motion->speed != motion->previousSpeed) {
            Hu3DMotionSpeedSet(lbl_1_bss_E88[playerIndex].modelId,
                motion->speed);
            motion->previousSpeed = motion->speed;
        }
        if (motion->attributes != motion->previousAttributes) {
            Hu3DModelAttrSet(lbl_1_bss_E88[playerIndex].modelId,
                motion->attributes);
            motion->previousAttributes = motion->attributes;
        }
        if (motion->attributesToReset != 0) {
            Hu3DModelAttrReset(lbl_1_bss_E88[playerIndex].modelId,
                motion->attributesToReset);
            /* Keep the stored attribute mask in sync with the flags just cleared. */
            motion->attributesToReset = ~motion->attributesToReset;
            motion->previousAttributes = motion->attributes =
                motion->attributes & motion->attributesToReset;
            motion->attributesToReset = 0;
        }
        if (motion->time != motion->previousTime) {
            Hu3DMotionTimeSet(lbl_1_bss_E88[playerIndex].modelId,
                motion->time);
        }
        switch (motion->shiftState) {
        case 0:
            break;
        case 1:
            Hu3DMotionShiftSet(lbl_1_bss_E88[playerIndex].modelId,
                motion->motionIds[motion->shiftMotion], motion->shiftStart,
                motion->shiftEnd, motion->shiftFlags);
            motion->shiftMaximumTime = Hu3DMotionShiftMaxTimeGet(
                lbl_1_bss_E88[playerIndex].modelId);
            motion->shiftState = 2;
            break;
        case 2:
            motion->shiftTime = Hu3DMotionShiftTimeGet(
                lbl_1_bss_E88[playerIndex].modelId);
            if ((motion->shiftFlags & HU3D_MOTATTR_LOOP) == 0 &&
                motion->shiftTime >= motion->shiftEnd - 1.0f) {
                motion->shiftState = 3;
            }
            break;
        case 3:
            motion->shiftState = 0;
            break;
        }
        motion->time = Hu3DMotionTimeGet(lbl_1_bss_E88[playerIndex].modelId);
        motion->previousTime = motion->time;
        motion->motionEnded = Hu3DMotionEndCheck(
            lbl_1_bss_E88[playerIndex].modelId);
        if (motion->motionEnded != 0 && motion->resetRequested != 0) {
            motion->motionEnded = 0;
            motion->resetRequested = 0;
            motion->mode = motion->resetMode;
            motion->time = motion->transition;
            motion->attributes = motion->resetAttributes;
            motion->attributesToReset = motion->resetAttributesToClear;
        }
        playerIndex++;
    }
}

#undef motion

/* Called by fn_1_118 to create the selected participant camera and initialize its look target. */
void fn_1_1780(s16 playerIndex)
{
    typedef struct M617CameraPresetTable {
        /* Position xyz, rotation xyz, zoom, aspect, viewSettings rectangle, and field of view. */
        f32 settings[4][13];
    } M617CameraPresetTable;
    typedef struct M617CameraIds {
        s16 masks[4]; /* Camera selection bits. */
    } M617CameraIds;
    typedef struct M617CameraRecord {
        s32 cameraIndex; /* Camera preset index. */
        u8 beforeRotation[12]; /* Unaccessed bytes before the rotation. */
        Vec rotation; /* Euler angles in degrees. */
        Vec position; /* Look-target position. */
        u8 beforeViewport[12]; /* Unaccessed bytes before the view settings. */
        f32 viewSettings[6]; /* Zoom, aspect ratio, viewport x/y/width/height. */
        f32 fieldOfView; /* Vertical field of view in degrees. */
    } M617CameraRecord;

    extern Vec Center;
    extern Vec CRot;
    extern f32 CZoom;
    extern void fn_1_1C30(s16);
    M617CameraPresetTable presets = { {
        { -737.0f, 456.0f, 55.0f, -24.0f, 0.0f, 0.0f, 197.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 20.98f },
        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f }
    } };
    M617CameraIds cameraIds = { { 1, 2, 4, 8 } };

    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].cameraIndex = playerIndex;
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].position.x = presets.settings[playerIndex][0];
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].position.y = presets.settings[playerIndex][1];
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].position.z = presets.settings[playerIndex][2];
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].rotation.x = presets.settings[playerIndex][3];
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].rotation.y = presets.settings[playerIndex][4];
    ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].rotation.z = presets.settings[playerIndex][5];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[0] =
        presets.settings[playerIndex][6];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[1] =
        presets.settings[playerIndex][7];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[2] =
        presets.settings[playerIndex][8];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[3] =
        presets.settings[playerIndex][9];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[4] =
        presets.settings[playerIndex][10];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].viewSettings[5] =
        presets.settings[playerIndex][11];
    ((M617CameraRecord *) lbl_1_bss_379C)[playerIndex].fieldOfView =
        presets.settings[playerIndex][12];
    Hu3DCameraCreate(cameraIds.masks[playerIndex]);
    Hu3DCameraViewportSet(cameraIds.masks[playerIndex],
                          ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].viewSettings[2],
                          ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].viewSettings[3],
                          ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].viewSettings[4],
                          ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].viewSettings[5],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds.masks[playerIndex],
                             ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].fieldOfView,
                             20.0f, 40000.0f,
                             ((M617CameraRecord *)lbl_1_bss_379C)[playerIndex].viewSettings[1]);
    Center.x = presets.settings[playerIndex][0];
    Center.y = presets.settings[playerIndex][1];
    Center.z = presets.settings[playerIndex][2];
    CRot.x = presets.settings[playerIndex][3];
    CRot.y = presets.settings[playerIndex][4];
    CRot.z = presets.settings[playerIndex][5];
    CZoom = presets.settings[playerIndex][6];
    fn_1_1C30(playerIndex);
}

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_118, fn_1_280, fn_1_2AC, fn_1_2D8, fn_1_304,
    fn_1_330, fn_1_35C, fn_1_388, fn_1_3B4
};

M617RouteGroup lbl_1_bss_38F0[24];
u16 lbl_1_bss_38E8[4];
HUPROCESS *lbl_1_bss_38E4;
u8 lbl_1_bss_379C[328];
M617LightInitState lbl_1_bss_3760;
M617ShadowWork lbl_1_bss_3710;
ParticleModuleData lbl_1_bss_3708;
M617ModelSetupRecord lbl_1_bss_E88[96];
M617ParticipantMotionState lbl_1_bss_DC0[2];
M617PlayerStateData lbl_1_bss_AB0[2];
M617State lbl_1_bss_9C8;
M617PlayerBindings lbl_1_bss_9BC;
M617LightSource lbl_1_bss_458[15];
M617BoxEdgeLinks lbl_1_bss_398[24];
M617SpriteGroup lbl_1_bss_370[2];
M617ScoreBoxEntry lbl_1_bss_0[22];

#define _DOLPHIN_MATH

extern f64 sin(f64 angle);

extern f64 cos(f64 angle);

extern void fn_1_1E38(void);

typedef struct M617CameraMaskSet {
    u16 firstCameraMask; /* Selects camera 0. */
    u16 secondCameraMask; /* Selects camera 1. */
    u16 thirdCameraMask; /* Selects camera 2. */
    u16 fourthCameraMask; /* Selects camera 3. */
} M617CameraMaskSet;

/* Camera setup and scene updates call this; its local camera-mask table has no effect. */
void fn_1_1C30(s16 cameraIndex)
{
    M617CameraMaskSet cameraMasks = { 1, 2, 4, 8 };
}

/* Lighting setup calls this to initialize the minigame light's position, direction, and color. */
void fn_1_1C64(void)
{
    /* Reset both light pools before creating this scene's white directional light. */
    Hu3DLighInit();

    lbl_1_bss_3760.position.x = 0.0f;
    lbl_1_bss_3760.position.y = 0.0f;
    lbl_1_bss_3760.position.z = 0.0f;
    lbl_1_bss_3760.aim.x = 0.0f;
    lbl_1_bss_3760.aim.y = 0.0f;
    lbl_1_bss_3760.aim.z = 0.0f;
    lbl_1_bss_3760.orientation.x = -70.0f;
    lbl_1_bss_3760.orientation.y = 0.0f;
    lbl_1_bss_3760.orientation.z = 0.0f;
    lbl_1_bss_3760.range = 1000.0f;

    lbl_1_bss_3760.color.r = 255;
    lbl_1_bss_3760.color.g = 255;
    lbl_1_bss_3760.color.b = 255;
    lbl_1_bss_3760.color.a = 255;

    PSVECSubtract(&lbl_1_bss_3760.aim, &lbl_1_bss_3760.position,
                  &lbl_1_bss_3760.direction);
    /* The zero direction is changed to +Z by light creation, then replaced below. */
    lbl_1_bss_3760.id = Hu3DGLightCreateV(
        &lbl_1_bss_3760.position, &lbl_1_bss_3760.direction,
        &lbl_1_bss_3760.color);
    fn_1_1E38();
    Hu3DGLightStaticSet(lbl_1_bss_3760.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_3760.id);
}

/* Called by fn_1_1C64 to recompute the light from its aim, range, and orientation. */
void fn_1_1E38(void)
{
    lbl_1_bss_3760.position.x =
        lbl_1_bss_3760.aim.x +
        lbl_1_bss_3760.range *
            (cos(3.141592653589793 * lbl_1_bss_3760.orientation.x /
                 180.0) *
             sin(3.141592653589793 * lbl_1_bss_3760.orientation.y /
                 180.0));
    lbl_1_bss_3760.position.y =
        lbl_1_bss_3760.aim.y +
        lbl_1_bss_3760.range *
            -sin(3.141592653589793 * lbl_1_bss_3760.orientation.x /
                 180.0);
    lbl_1_bss_3760.position.z =
        lbl_1_bss_3760.aim.z +
        lbl_1_bss_3760.range *
            (cos(3.141592653589793 * lbl_1_bss_3760.orientation.x /
                 180.0) *
             cos(3.141592653589793 * lbl_1_bss_3760.orientation.y /
                 180.0));
    PSVECSubtract(&lbl_1_bss_3760.aim, &lbl_1_bss_3760.position,
                  &lbl_1_bss_3760.direction);
    PSVECNormalize(&lbl_1_bss_3760.direction, &lbl_1_bss_3760.direction);
    Hu3DGLightPosSetV(lbl_1_bss_3760.id, &lbl_1_bss_3760.position,
                      &lbl_1_bss_3760.direction);
    Hu3DGLightColorSet(lbl_1_bss_3760.id, lbl_1_bss_3760.color.r,
                       lbl_1_bss_3760.color.g, lbl_1_bss_3760.color.b,
                       lbl_1_bss_3760.color.a);
}

void fn_1_21B4(void);

/* Called by fn_1_118 during scene startup to configure the shadow camera's initial pose. */
void fn_1_20A4(void)
{
    Hu3DShadowCreate(5.0f, 16000.0f, 21000.0f);
    Hu3DShadowSizeSet(480);
    lbl_1_bss_3710.angleY = -90.0f;
    lbl_1_bss_3710.angleX = 0.0f;
    lbl_1_bss_3710.angleZ = 0.0f;
    lbl_1_bss_3710.originX = 0.0f;
    lbl_1_bss_3710.originY = -100.0f;
    lbl_1_bss_3710.originZ = 0.0f;
    lbl_1_bss_3710.shadowOpacity = 0.5f;
    lbl_1_bss_3710.radius = 21000.0f;
    fn_1_21B4();
}

/* Called by fn_1_20A4 to compute the shadow camera position and direction from its angles. */
void fn_1_21B4(void)
{
    lbl_1_bss_3710.positionX = (f32)((f64)lbl_1_bss_3710.originX +
        ((f64)lbl_1_bss_3710.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
          sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0))));
    lbl_1_bss_3710.positionY = (f32)((f64)lbl_1_bss_3710.originY +
        ((f64)lbl_1_bss_3710.radius *
         -sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0)));
    lbl_1_bss_3710.positionZ = (f32)((f64)lbl_1_bss_3710.originZ +
        ((f64)lbl_1_bss_3710.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
          cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0))));
    lbl_1_bss_3710.directionX = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0));
    lbl_1_bss_3710.directionY = (f32)cos(
        (3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0);
    lbl_1_bss_3710.directionZ = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
        cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0));
    Hu3DShadowTPLvlSet(lbl_1_bss_3710.shadowOpacity);
    Hu3DShadowPosSet((HuVecF *)&lbl_1_bss_3710,
        (HuVecF *)&lbl_1_bss_3710.directionX,
        (HuVecF *)&lbl_1_bss_3710.originX);
}

typedef M617ModelSetupRecord M617PlayerModel;

void fn_1_FBC(void);

/* Called by phase updates to sync participant models, animation, and shadow placement. */
void fn_1_24D8(void)
{
    Vec playerPosition;
    s16 playerIndex;
    s32 modelIndex;
    struct M617FourHalfwords {
        u16 first;
        u16 second;
        u16 third;
        u16 fourth;
    };

    fontcolor = 15;
    playerIndex = 0;
    while (playerIndex < 2) {
        playerPosition.x = lbl_1_bss_AB0[playerIndex].position.x;
        playerPosition.y = lbl_1_bss_AB0[playerIndex].position.y;
        playerPosition.z = lbl_1_bss_AB0[playerIndex].position.z;
        MgActorPosSetRaw(
            lbl_1_bss_9BC.players[playerIndex]->actor,
            &playerPosition);
        lbl_1_bss_9BC.players[playerIndex]->actor->rotY =
            lbl_1_bss_AB0[playerIndex].rotation.y;
        lbl_1_bss_E88[playerIndex].position = lbl_1_bss_AB0[playerIndex].position;
        lbl_1_bss_E88[playerIndex].rotation = lbl_1_bss_AB0[playerIndex].rotation;
        playerIndex++;
    }

    modelIndex = 2;
    while (modelIndex < 96) {
        Hu3DModelPosSet(lbl_1_bss_E88[modelIndex].modelId,
            lbl_1_bss_E88[modelIndex].position.x,
            lbl_1_bss_E88[modelIndex].position.y,
            lbl_1_bss_E88[modelIndex].position.z);
        Hu3DModelRotSet(lbl_1_bss_E88[modelIndex].modelId,
            lbl_1_bss_E88[modelIndex].rotation.x,
            lbl_1_bss_E88[modelIndex].rotation.y,
            lbl_1_bss_E88[modelIndex].rotation.z);
        modelIndex++;
    }

    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_AB0[modelIndex].transformState == 0 ||
            lbl_1_bss_AB0[modelIndex].transformState == 2) {
            Hu3DModelPosSet(lbl_1_bss_E88[modelIndex].modelId,
                lbl_1_bss_AB0[modelIndex].position.x,
                lbl_1_bss_AB0[modelIndex].position.y,
                lbl_1_bss_AB0[modelIndex].position.z);
            Hu3DModelRotSet(lbl_1_bss_E88[modelIndex].modelId,
                lbl_1_bss_AB0[modelIndex].rotation.x,
                lbl_1_bss_AB0[modelIndex].rotation.y,
                lbl_1_bss_AB0[modelIndex].rotation.z);
        }
        modelIndex++;
    }

    fn_1_FBC();
    fn_1_1C30(0);
    lbl_1_bss_3710.positionX = (f32)((f64)lbl_1_bss_3710.originX +
        ((f64)lbl_1_bss_3710.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
          sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0))));
    lbl_1_bss_3710.positionY = (f32)((f64)lbl_1_bss_3710.originY +
        ((f64)lbl_1_bss_3710.radius *
         -sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0)));
    lbl_1_bss_3710.positionZ = (f32)((f64)lbl_1_bss_3710.originZ +
        ((f64)lbl_1_bss_3710.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
          cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0))));
    lbl_1_bss_3710.directionX = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0));
    lbl_1_bss_3710.directionY = (f32)cos(
        (3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0);
    lbl_1_bss_3710.directionZ = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_3710.angleY) / 180.0) *
        cos((3.141592653589793 * (f64)lbl_1_bss_3710.angleX) / 180.0));
    Hu3DShadowTPLvlSet(lbl_1_bss_3710.shadowOpacity);
    Hu3DShadowPosSet((HuVecF *)&lbl_1_bss_3710,
        (HuVecF *)&lbl_1_bss_3710.directionX,
        (HuVecF *)&lbl_1_bss_3710.originX);
}

extern int printf();

#define PLAYER_STATE(i) (lbl_1_bss_AB0[(i)])

#define MODEL_STATE(i) (lbl_1_bss_E88[(i)])

void fn_1_33C0(s16 controller, s16 selection);

/* MgSeq phase callbacks call this each frame to update actors and participant positions. */
void fn_1_2B48(void) {
    Point3d actorPosition;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_AB0[playerIndex].transformState != 0) {
            if ((s8) lbl_1_bss_AB0[playerIndex].motionState != 0) {
                MgPlayerPadSet(lbl_1_bss_9BC.players[playerIndex], 0, 0, 0, 0);
            } else if ((s8) lbl_1_bss_AB0[playerIndex].computerControlled != 0) {
                MgPlayerPadSet(lbl_1_bss_9BC.players[playerIndex],
                               lbl_1_bss_AB0[playerIndex].computerStickX,
                               lbl_1_bss_AB0[playerIndex].computerStickY,
                               lbl_1_bss_AB0[playerIndex].computerButtonsDown,
                               lbl_1_bss_AB0[playerIndex].computerButtonsHeld);
            }
            MgActorPosGet(lbl_1_bss_9BC.players[playerIndex]->actor, &actorPosition);
            lbl_1_bss_AB0[playerIndex].target.x = actorPosition.x;
            lbl_1_bss_AB0[playerIndex].target.y = actorPosition.y;
            lbl_1_bss_AB0[playerIndex].target.z = actorPosition.z;
            lbl_1_bss_AB0[playerIndex].previousCollisionMeshIndex =
                lbl_1_bss_AB0[playerIndex].collisionMeshIndex;
            lbl_1_bss_AB0[playerIndex].collisionMeshIndex = -1;
            if ((lbl_1_bss_AB0[playerIndex].previousLinkedPlayer !=
                 lbl_1_bss_AB0[playerIndex].linkedPlayer) ||
                (lbl_1_bss_AB0[playerIndex].linkedPlayer == -1)) {
                lbl_1_bss_AB0[playerIndex].orbitCounter = 0;
            }
            lbl_1_bss_AB0[playerIndex].previousLinkedPlayer =
                lbl_1_bss_AB0[playerIndex].linkedPlayer;
            lbl_1_bss_AB0[playerIndex].linkedPlayer = -1;
        }
        playerIndex += 1;
    }
    MgActorExec();
    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_AB0[playerIndex].transformState != 0) {
            MgActorPosGet(lbl_1_bss_9BC.players[playerIndex]->actor, &actorPosition);
            lbl_1_bss_AB0[playerIndex].position.x = actorPosition.x;
            lbl_1_bss_AB0[playerIndex].position.y = actorPosition.y;
            lbl_1_bss_AB0[playerIndex].position.z = actorPosition.z;
            lbl_1_bss_AB0[playerIndex].rotation.y = lbl_1_bss_9BC.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

/* Camera transitions call this each frame to move a value from its start to its end. */
s32 fn_1_2E94(f32 *value, f32 duration, f32 start, f32 end)
{
    f32 increment;
    f32 startDistance;
    f32 valueDistance;
    s32 frameCount;

    frameCount = 0;
    increment = (end - start) / duration;
    *value += increment;
    if (increment) {
        frameCount = (s32) ((end - *value) / increment);
    }
    frameCount++;
    if ((start - end) > 0.0f) {
        startDistance = start - end;
    } else {
        startDistance = start - (-1.0f * end);
    }
    increment = startDistance;
    if ((start - *value) > 0.0f) {
        valueDistance = start - *value;
    } else {
        valueDistance = start - (-1.0f * *value);
    }
    start = valueDistance;
    /* The nonpositive branches above add the operands when testing the endpoint. */
    if (increment <= start) {
        *value = end;
        frameCount = 0;
    }
    return frameCount;
}

/* The intro creates its on-screen timer here; duration and end are converted from seconds. */
MGTIMER *fn_1_2FB4(int type, int duration, int end, float x, float y)
{
    MGTIMER *timer = MgTimerCreate(type);
    MgTimerParamSet(timer, duration * 60, end * 60, 0);
    MgTimerPosSet(timer, x, y);
    return timer;
}

void fn_1_3028(MGTIMER *timer)
{
    MgTimerModeOnSet(timer, 1);
}

/* Queries timer completion but discards the result; the game-phase caller still treats this
 * wrapper as returning a status. */
void fn_1_3054(MGTIMER *timer)
{
    MgTimerDoneCheck(timer);
}

/* Timer transitions and sequence cleanup call this to hide and destroy the timer. */
void fn_1_307C(MGTIMER *timer)
{
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

/* Model inspection controls use this to change the selected model and show its transform. */
void fn_1_30B4(s16 controller, s16 minimum, s16 maximum)
{
    s16 player;

    if (HuPadTrigL[controller] > 50) {
        if (HuPadBtnDown[controller] & PAD_BUTTON_X) {
            lbl_1_bss_9C8.selection++;
        }
        if (HuPadBtnDown[controller] & PAD_BUTTON_Y) {
            lbl_1_bss_9C8.selection--;
        }
    }

    if (lbl_1_bss_9C8.selection < minimum) {
        lbl_1_bss_9C8.selection = maximum;
    }
    if (lbl_1_bss_9C8.selection > maximum) {
        lbl_1_bss_9C8.selection = minimum;
    }

    for (player = minimum; player <= maximum; player++) {
        Hu3DModelTPLvlSet(lbl_1_bss_E88[player].modelId, 1.0f);
    }
    Hu3DModelTPLvlSet(lbl_1_bss_E88[lbl_1_bss_9C8.selection].modelId,
                      0.5f);
    fn_1_33C0(controller, lbl_1_bss_9C8.selection);

    if ((HuPadBtnDown[controller] & PAD_BUTTON_X) && (HuPadBtnDown[controller] & PAD_BUTTON_Y)) {
        for (player = minimum; player <= maximum; player++) {
            if (player >= 0 && player <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n", player,
                       lbl_1_bss_AB0[player].position.x,
                       lbl_1_bss_AB0[player].position.y,
                       lbl_1_bss_AB0[player].position.z,
                       lbl_1_bss_AB0[player].rotation.y);
            } else {
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n", player,
                       lbl_1_bss_E88[player].position.x,
                       lbl_1_bss_E88[player].position.y,
                       lbl_1_bss_E88[player].position.z,
                        lbl_1_bss_E88[player].rotation.y);
            }
        }
    }
}

/* fn_1_30B4 calls this to move or rotate the selected participant or scene model. */
void fn_1_33C0(s16 padIndex, s16 selectedModelIndex)
{
    if (selectedModelIndex >= 0 && selectedModelIndex <= 1) {
        if ((HuPadBtnRep[padIndex] & PAD_BUTTON_X) != 0) {
            if (HuPadStkX[padIndex] > 10) {
                PLAYER_STATE(selectedModelIndex).position.x += 5.0f;
            }
            if (HuPadStkX[padIndex] < -10) {
                PLAYER_STATE(selectedModelIndex).position.x -= 5.0f;
            }
            if (HuPadStkY[padIndex] > 10) {
                PLAYER_STATE(selectedModelIndex).position.z -= 5.0f;
            }
            if (HuPadStkY[padIndex] < -10) {
                PLAYER_STATE(selectedModelIndex).position.z += 5.0f;
            }
        } else if ((HuPadBtnDown[padIndex] & PAD_BUTTON_X) != 0) {
            if (HuPadStkX[padIndex] > 10) {
                PLAYER_STATE(selectedModelIndex).position.x += 5.0f;
            }
            if (HuPadStkX[padIndex] < -10) {
                PLAYER_STATE(selectedModelIndex).position.x -= 5.0f;
            }
            if (HuPadStkY[padIndex] > 10) {
                PLAYER_STATE(selectedModelIndex).position.z -= 5.0f;
            }
            if (HuPadStkY[padIndex] < -10) {
                PLAYER_STATE(selectedModelIndex).position.z += 5.0f;
            }
        }
        if ((HuPadBtnDown[padIndex] & PAD_BUTTON_A) != 0) {
            PLAYER_STATE(selectedModelIndex).rotation.y += 90.0f;
        }
        if ((HuPadBtnDown[padIndex] & PAD_BUTTON_B) != 0) {
            PLAYER_STATE(selectedModelIndex).rotation.y -= 90.0f;
        }
        print8(25, 156, 1.5f,
            "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
            selectedModelIndex,
            PLAYER_STATE(selectedModelIndex).position.x,
            PLAYER_STATE(selectedModelIndex).position.y,
            PLAYER_STATE(selectedModelIndex).position.z,
            PLAYER_STATE(selectedModelIndex).rotation.y);
        return;
    }

    if ((HuPadBtnRep[padIndex] & PAD_BUTTON_X) != 0) {
        if (HuPadStkX[padIndex] > 10) {
            MODEL_STATE(selectedModelIndex).position.x += 5.0f;
        }
        if (HuPadStkX[padIndex] < -10) {
            MODEL_STATE(selectedModelIndex).position.x -= 5.0f;
        }
        if (HuPadStkY[padIndex] > 10) {
            MODEL_STATE(selectedModelIndex).position.z -= 5.0f;
        }
        if (HuPadStkY[padIndex] < -10) {
            MODEL_STATE(selectedModelIndex).position.z += 5.0f;
        }
    } else if ((HuPadBtnDown[padIndex] & PAD_BUTTON_X) != 0) {
        if (HuPadStkX[padIndex] > 10) {
            MODEL_STATE(selectedModelIndex).position.x += 5.0f;
        }
        if (HuPadStkX[padIndex] < -10) {
            MODEL_STATE(selectedModelIndex).position.x -= 5.0f;
        }
        if (HuPadStkY[padIndex] > 10) {
            MODEL_STATE(selectedModelIndex).position.z -= 5.0f;
        }
        if (HuPadStkY[padIndex] < -10) {
            MODEL_STATE(selectedModelIndex).position.z += 5.0f;
        }
    }
    if ((HuPadBtnDown[padIndex] & PAD_BUTTON_A) != 0) {
        MODEL_STATE(selectedModelIndex).rotation.y += 90.0f;
    }
    if ((HuPadBtnDown[padIndex] & PAD_BUTTON_B) != 0) {
        MODEL_STATE(selectedModelIndex).rotation.y -= 90.0f;
    }
    print8(25, 156, 1.5f,
        "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
        selectedModelIndex,
        MODEL_STATE(selectedModelIndex).position.x, MODEL_STATE(selectedModelIndex).position.y,
        MODEL_STATE(selectedModelIndex).position.z,
         MODEL_STATE(selectedModelIndex).rotation.y);
}

#undef PLAYER_STATE

#undef MODEL_STATE

s32 frandmod(s32 maximum);

extern int printf(const char *format, s32 modelId, s32 lightId);

/* Light setup calls this to attach a shared local-light-pool entry to a model's light list. */
s16 fn_1_3BB8(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *model;
    s16 result;
    s16 index;

    result = 0;
    model = &Hu3DData[modelId];
    index = 0;
    while (index < 8) {
        if (model->LLightId[index] == lightId) {
            result = -1;
            break;
        }
        index++;
    }
    if (result != -1) {
        index = 0;
        while (index < 8) {
            if (model->LLightId[index] == -1) {
                model->LLightId[index] = lightId;
                model->attr |= HU3D_ATTR_LLIGHT;
                result = 1;
                break;
            }
            index++;
        }
    }
    if (result == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", (s16) modelId, (s16) lightId);
        result = -1;
    }
    return result;
}

/* Light cleanup calls this to remove a shared local-light-pool entry from a model's light list. */
s32 fn_1_3CD0(HU3D_MODELID modelId, HU3D_LIGHTID lightId)
{
    HU3D_MODEL *model;
    s32 result;
    s16 index;

    result = -1;
    model = &Hu3DData[modelId];
    index = 0;
    while (index < HU3D_MODEL_LLIGHT_MAX) {
        if (model->LLightId[index] == lightId) {
            model->LLightId[index] = -1;
            result = 1;
            break;
        }
        index++;
    }
    return result;
}

/* Light effects use this to return a random integer-valued float below the input limit. */
f32 fn_1_3D68(f32 value)
{
    s32 result = value;

    result = frandmod(result);
    value = result;
    return value;
}

/* Drifting particle parameters and animation state. */

typedef struct M617ParticleVector {
    f32 x; /* X component in world coordinates. */
    f32 y; /* Y component in world coordinates. */
    f32 z; /* Z component in world coordinates. */
} M617ParticleVector;

typedef struct M617ParticleColor {
    u8 red; /* Red channel, 0..255. */
    u8 green; /* Green channel, 0..255. */
    u8 blue; /* Blue channel, 0..255. */
    u8 alpha; /* Alpha channel, 0..255. */
} M617ParticleColor;

typedef struct M617ParticleParameters {
    u32 value00;
    M617ParticleVector playerMovementOffset; /* Accumulated player movement after emission. */
    M617ParticleColor startColor; /* Initial RGBA channels, each 0..255. */
    M617ParticleColor targetColor; /* RGBA endpoint for the per-frame color changes. */
    f32 velocityScale[3]; /* Per-frame X/Y/Z velocity multipliers. */
    f32 verticalAcceleration; /* Added to vertical velocity each frame. */
    f32 rotationIncrement; /* Added to billboard Z rotation each frame. */
    f32 scaleIncrement; /* Added to particle scale each frame. */
    f32 scaleIncrementMultiplier; /* Per-frame multiplier for scaleIncrement. */
    f32 alphaIncrement; /* Added to alpha each frame. */
    f32 colorProgress; /* Fraction of the start-to-target RGB difference added each frame. */
} M617ParticleParameters;

typedef struct M617ParticleEntry {
    s16 framesRemaining; /* Lifetime in update frames; zero marks an inactive entry. */
    s16 playerIndex; /* Participant to follow, or -1 for independent motion. */
    u32 renderAttributesAndCamera; /* Engine particle attribute halfword and camera-mask
                                    * halfword. */
    M617ParticleVector velocity; /* Position change per frame. */
    M617ParticleVector emissionPosition; /* World position supplied at emission. */
    M617ParticleVector velocityOffset; /* Accumulated velocity for particles following a player. */
    f32 scale; /* Billboard scale; nonpositive values make an entry available for reuse. */
    f32 scaleY; /* Engine vertical particle scale. */
    f32 rotationZ; /* Billboard rotation in degrees. */
    M617ParticleVector position; /* Current world position. */
    M617ParticleColor color; /* Current RGBA channels. */
} M617ParticleEntry;

typedef struct M617ParticleContext {
    u8 dataCountBytes[2]; /* Engine particle-data count storage, not read here. */
    s16 lastEntryIndex; /* Index most recently initialized by the emitter. */
    u8 positionAndSpawnCenterBytes[24]; /* Engine position and spawn-center vectors. */
    M617ParticleParameters *parameters; /* One parameter record per particle entry. */
    u8 animationAndAttributeBytes[16]; /* Engine animation state, attributes, and unaccessed
                                        * bytes. */
    s16 entryCount; /* Maximum number of entries in the effect. */
    u8 beforeFrameCounter[2]; /* Unaccessed bytes before the engine frame counter. */
    void *frameCounterStorage; /* Engine frame-counter storage; initialized to zero here. */
    u8 historyAndAnimationBytes[16]; /* Engine counters, display-list size, and animation
                                      * pointer. */
    M617ParticleEntry *entries; /* Engine particle entries updated by the render hook. */
} M617ParticleContext;

typedef struct M617CopyColor {
    u8 red; /* Red channel, 0..255. */
    u8 green; /* Green channel, 0..255. */
    u8 blue; /* Blue channel, 0..255. */
    u8 alpha; /* Alpha channel, 0..255. */
} M617CopyColor;

#define M617_PARTICLE_VELOCITY_X 0
#define M617_PARTICLE_VELOCITY_Y 1
#define M617_PARTICLE_VELOCITY_Z 2
#define M617_PARTICLE_VERTICAL_ACCELERATION 3
#define M617_PARTICLE_ROTATION_INCREMENT 4
#define M617_PARTICLE_SCALE_INCREMENT 5
#define M617_PARTICLE_SCALE_MULTIPLIER 6
#define M617_PARTICLE_ALPHA_INCREMENT 7
#define M617_PARTICLE_COLOR_PROGRESS 8

typedef struct M617CopyData {
    u32 value00;
    f32 playerOffsetX; /* Accumulated player movement on X; cleared at emission. */
    f32 playerOffsetY; /* Accumulated player movement on Y; cleared at emission. */
    f32 playerOffsetZ; /* Accumulated player movement on Z; cleared at emission. */
    M617CopyColor color; /* Initial RGBA channels, each 0..255. */
    u8 targetColor[4]; /* Target RGBA channels. */
    f32 animationParameters[9]; /* Velocity multipliers, acceleration, rotation, scale, and
                                 * color. */
} M617CopyData;

typedef struct M617Vector3 {
    f32 x; /* X component in world coordinates. */
    f32 y; /* Y component in world coordinates. */
    f32 z; /* Z component in world coordinates. */
} M617Vector3;

typedef struct M617CopyParticleEntry {
    s16 framesRemaining; /* Lifetime in update frames. */
    s16 playerIndex; /* Participant to follow, or -1 for independent motion. */
    u32 renderAttributesAndCamera; /* Engine particle attribute halfword and camera-mask
                                    * halfword. */
    M617Vector3 velocity; /* Position change per frame. */
    M617Vector3 emissionPosition; /* World position supplied at emission. */
    f32 offsetX; /* Accumulated X velocity; cleared at emission. */
    f32 offsetY; /* Accumulated Y velocity; cleared at emission. */
    f32 offsetZ; /* Accumulated Z velocity; cleared at emission. */
    f32 scale; /* Billboard scale; nonpositive values make the entry available for reuse. */
    u32 scaleYBits; /* Engine vertical-scale storage, untouched by emission. */
    u32 rotationZBits; /* Engine Z-rotation storage, untouched by emission. */
    M617Vector3 position; /* Current world position. */
    M617CopyColor color; /* Current RGBA channels. */
} M617CopyParticleEntry;

typedef struct M617CopyParticleContext {
    u8 dataCountBytes[2]; /* Engine particle-data count storage, not read here. */
    s16 lastEntryIndex; /* Index most recently initialized by the emitter. */
    u8 positionAndSpawnCenterBytes[24]; /* Engine position and spawn-center vectors. */
    M617CopyData *entryData; /* Per-entry animation parameters. */
    u8 animationAndAttributeBytes[16]; /* Engine animation state, attributes, and unaccessed
                                        * bytes. */
    s16 entryCount; /* Maximum number of entries in the effect. */
    u8 beforeEntries[22]; /* Engine counters, display-list size, animation pointer, and unaccessed
                           * bytes. */
    M617CopyParticleEntry *entries; /* Engine particle entries available for emission. */
} M617CopyParticleContext;

void fn_1_484C(HU3D_MODEL *model, M617ParticleContext *context, Mtx matrix);

void fn_1_3DE0(void);

s16 fn_1_463C(s16 modelIndex, s16 playerIndex, M617Vector3 emissionPosition, M617Vector3 velocity,
              f32 initialScale, s16 framesRemaining, const M617CopyData *copyData);

/* fn_1_93D0 calls this during scene setup to allocate the 500-entry drifting-particle effect. */
void fn_1_3DE0(void)
{
    u32 dataNumbers[1] = { DATANUM(DATA_m617, 51) };
    s16 maximumParticles[1] = { 500 };
    s16 layers[1] = { 5 };
    s16 cameras[1] = { 1 };
    s16 blendModes[1] = { 1 };
    s16 effectIndex;

    for (effectIndex = 0; effectIndex < 1; effectIndex++) {
        ANIMDATA *animation;
        M617ParticleContext *context;
        M617ParticleEntry *entry;
        s16 particleIndex;

        lbl_1_bss_3708.maximumCounts[effectIndex] = maximumParticles[effectIndex];
        animation = HuSprAnimRead(HuDataSelHeapReadNum(
            dataNumbers[effectIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3708.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, maximumParticles[effectIndex] * 60, HU_MEMNUM_OVL);
        lbl_1_bss_3708.modelIds[effectIndex] = Hu3DParticleCreate(
            animation, maximumParticles[effectIndex]);
        Hu3DModelLayerSet(lbl_1_bss_3708.modelIds[effectIndex], layers[effectIndex]);
        Hu3DParticleHookSet(lbl_1_bss_3708.modelIds[effectIndex],
                            (HU3D_PARTICLE_HOOK)fn_1_484C);
        Hu3DModelCameraSet(lbl_1_bss_3708.modelIds[effectIndex], cameras[effectIndex]);
        Hu3DParticleBlendModeSet(lbl_1_bss_3708.modelIds[effectIndex],
                                 (u8)blendModes[effectIndex]);

        context = Hu3DData[lbl_1_bss_3708.modelIds[effectIndex]].hookData;
        context->lastEntryIndex = 0;
        context->parameters = lbl_1_bss_3708.allocation[effectIndex];
        context->frameCounterStorage = 0;
        particleIndex = 0;
        entry = context->entries;
        for (; particleIndex < context->entryCount; particleIndex++, entry++) {
            entry->scale = 0.0f;
            entry->framesRemaining = 0;
        }
    }
}

/* The light update calls this to emit a drifting particle at a light's object position. */
s16 fn_1_4070(const M617Vector3 *position)
{
    M617Vector3 offset;
    M617Vector3 positionValue;
    M617CopyData copyData;
    s32 randomLimit;
    s32 randomHeightLimit;
    f32 randomValue;
    f32 heightRandom;
    f32 initialScale;
    f32 centeredRandomValue;
    f32 horizontalVelocity;
    f32 baseHeight;
    f32 verticalVelocity;
    copyData.value00 = 0;
    copyData.color.red = 255;
    copyData.color.green = 255;
    copyData.color.blue = 255;
    copyData.color.alpha = 255;
    copyData.targetColor[0] = 0;
    copyData.targetColor[1] = 0;
    copyData.targetColor[2] = 0;
    copyData.targetColor[3] = 0;
    copyData.animationParameters[M617_PARTICLE_ROTATION_INCREMENT] = 0.0f;
    copyData.animationParameters[M617_PARTICLE_ALPHA_INCREMENT] = 0.01f;
    copyData.animationParameters[M617_PARTICLE_COLOR_PROGRESS] = 0.01f;
    copyData.animationParameters[M617_PARTICLE_VERTICAL_ACCELERATION] = 0.0f;
    positionValue.x = position->x;
    positionValue.y = 10.0f + position->y;
    positionValue.z = position->z;
    initialScale = 28.0f;
    copyData.animationParameters[M617_PARTICLE_SCALE_INCREMENT] = (-0.1f);
    copyData.animationParameters[M617_PARTICLE_SCALE_MULTIPLIER] = 1.2f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_X] = 1.05f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_Y] = 0.95f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_Z] = 1.0f;
    randomValue = 3.0f;
    randomLimit = (s32) randomValue;
    randomLimit = frandmod(randomLimit);
    randomValue = (f32) randomLimit;
    centeredRandomValue = randomValue;
    horizontalVelocity = centeredRandomValue;
    offset.x = 0.4f *
               (horizontalVelocity - 1.0f);
    heightRandom = 2.0f;
    randomHeightLimit = (s32) heightRandom;
    randomHeightLimit = frandmod(randomHeightLimit);
    heightRandom = (f32) randomHeightLimit;
    baseHeight = heightRandom;
    verticalVelocity = baseHeight;
    offset.y = 4.0f + verticalVelocity;
    offset.z = 0.0f;
    return fn_1_463C(0, - 1, positionValue, offset, initialScale, 20, &copyData);
}

/* The light update calls this to emit a drifting particle associated with a player. */
s16 fn_1_435C(s16 playerIndex, const M617Vector3 *position)
{
    M617CopyData copyData;
    M617Vector3 offset;
    M617Vector3 positionValue;
    f32 randomLimitX;
    f32 randomLimitY;
    f32 initialScale;
    f32 randomSampleX;
    f32 randomCenteredX;
    f32 randomSampleY;
    f32 randomCenteredY;
    s32 randomValueX;
    s32 randomValueY;

    copyData.value00 = 0;
    copyData.color.red = 255;
    copyData.color.green = 255;
    copyData.color.blue = 255;
    copyData.color.alpha = 255;
    copyData.targetColor[0] = 0;
    copyData.targetColor[1] = 0;
    copyData.targetColor[2] = 0;
    copyData.targetColor[3] = 0;
    copyData.animationParameters[M617_PARTICLE_ROTATION_INCREMENT] = 0.0f;
    copyData.animationParameters[M617_PARTICLE_ALPHA_INCREMENT] = 0.01f;
    copyData.animationParameters[M617_PARTICLE_COLOR_PROGRESS] = 0.01f;
    copyData.animationParameters[M617_PARTICLE_VERTICAL_ACCELERATION] = 0.0f;
    positionValue.x = position->x;
    positionValue.y = position->y;
    positionValue.z = position->z;
    initialScale = 30.0f;
    copyData.animationParameters[M617_PARTICLE_SCALE_INCREMENT] = (-0.1f);
    copyData.animationParameters[M617_PARTICLE_SCALE_MULTIPLIER] = 1.2f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_X] = 1.05f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_Y] = 0.95f;
    copyData.animationParameters[M617_PARTICLE_VELOCITY_Z] = 1.0f;
    randomLimitX = 3.0f;
    randomValueX = (s32)randomLimitX;
    randomValueX = frandmod(randomValueX);
    randomLimitX = (f32)randomValueX;
    randomSampleX = randomLimitX;
    randomCenteredX = randomSampleX;
    offset.x = 0.4f *
               (randomCenteredX - 1.0f);
    randomLimitY = 3.0f;
    randomValueY = (s32)randomLimitY;
    randomValueY = frandmod(randomValueY);
    randomLimitY = (f32)randomValueY;
    randomSampleY = randomLimitY;
    randomCenteredY = randomSampleY;
    offset.y = 5.0f + randomCenteredY;
    offset.z = 0.0f;

    return fn_1_463C(0, playerIndex, positionValue, offset, initialScale, 20,
                     &copyData);
}

/* The two emitters call this to initialize a free entry; -1 means the effect is full. */
s16 fn_1_463C(s16 modelIndex, s16 playerIndex, M617Vector3 emissionPosition,
              M617Vector3 velocity, f32 initialScale, s16 framesRemaining,
              const M617CopyData *copyData)
{
    HU3D_MODEL *model;
    s16 result;
    M617CopyParticleContext *context;
    M617CopyData *entryData;
    s16 entryIndex;
    M617CopyParticleEntry *entry;

    result = -1;
    model = &Hu3DData[lbl_1_bss_3708.modelIds[modelIndex]];
    context = model->hookData;
    entryData = context->entryData;
    entryIndex = 0;
    entry = context->entries;

    while (entryIndex < context->entryCount) {
        if (entry->scale <= 0.0f) {
            entryData[entryIndex] = *copyData;
            entryData[entryIndex].playerOffsetZ = entryData[entryIndex].playerOffsetY =
                entryData[entryIndex].playerOffsetX = 0.0f;

            entry->position = emissionPosition;
            entry->velocity = velocity;
            entry->emissionPosition = emissionPosition;
            entry->offsetZ = entry->offsetY = entry->offsetX = 0.0f;

            entry->color = entryData[entryIndex].color;
            entry->scale = initialScale;
            entry->framesRemaining = framesRemaining;
            entry->playerIndex = playerIndex;
            context->lastEntryIndex = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

/* The particle render hook advances active entries, follows player movement, and expires them. */
void fn_1_484C(HU3D_MODEL *model, M617ParticleContext *context, Mtx matrix)
{
    M617ParticleParameters *parameters;
    M617ParticleEntry *entry;
    s16 entryIndex;

    parameters = context->parameters;
    entry = context->entries;
    entryIndex = 0;
    while (entryIndex < context->entryCount) {
        if (entry->framesRemaining != 0) {
            entry->velocity.x *= parameters[entryIndex].velocityScale[0];
            entry->velocity.y *= parameters[entryIndex].velocityScale[1];
            entry->velocity.z *= parameters[entryIndex].velocityScale[2];

            if (entry->playerIndex < 0) {
                PSVECAdd((Vec *)&entry->velocity, (Vec *)&entry->position,
                         (Vec *)&entry->position);
            } else {
                M617ParticleVector playerMovement;

                if (entry->framesRemaining != 20) {
                    f32 frameFraction = (f32)entry->framesRemaining / 20.0f;

                    playerMovement.x = frameFraction *
                        lbl_1_bss_AB0[entry->playerIndex].movement.x;
                    playerMovement.y = frameFraction *
                        lbl_1_bss_AB0[entry->playerIndex].movement.y;
                    playerMovement.z = frameFraction *
                        lbl_1_bss_AB0[entry->playerIndex].movement.z;
                } else {
                    /* The first frame ignores player movement; later frames use remaining life. */
                    playerMovement.x = playerMovement.y =
                        playerMovement.z = 0.0f;
                }
                PSVECAdd((Vec *)&parameters[entryIndex].playerMovementOffset,
                         (Vec *)&playerMovement,
                         (Vec *)&parameters[entryIndex].playerMovementOffset);
                PSVECAdd((Vec *)&parameters[entryIndex].playerMovementOffset,
                         (Vec *)&entry->emissionPosition, (Vec *)&entry->position);
                PSVECAdd((Vec *)&entry->velocityOffset,
                         (Vec *)&entry->velocity,
                         (Vec *)&entry->velocityOffset);
                PSVECAdd((Vec *)&entry->velocityOffset,
                         (Vec *)&entry->position, (Vec *)&entry->position);
            }

            entry->velocity.y += parameters[entryIndex].verticalAcceleration;
            entry->rotationZ += parameters[entryIndex].rotationIncrement;

            {
                s16 channelValue;

                channelValue = (s16)((f32)entry->color.red +
                    parameters[entryIndex].colorProgress *
                    (f32)(parameters[entryIndex].targetColor.red -
                          parameters[entryIndex].startColor.red));
                if (channelValue < 0) {
                    channelValue = 0;
                } else if (channelValue > 255) {
                    channelValue = 255;
                }
                entry->color.red = (u8)channelValue;

                channelValue = (s16)((f32)entry->color.green +
                    parameters[entryIndex].colorProgress *
                    (f32)(parameters[entryIndex].targetColor.green -
                          parameters[entryIndex].startColor.green));
                if (channelValue < 0) {
                    channelValue = 0;
                } else if (channelValue > 255) {
                    channelValue = 255;
                }
                entry->color.green = (u8)channelValue;

                channelValue = (s16)((f32)entry->color.blue +
                    parameters[entryIndex].colorProgress *
                    (f32)(parameters[entryIndex].targetColor.blue -
                          parameters[entryIndex].startColor.blue));
                if (channelValue < 0) {
                    channelValue = 0;
                } else if (channelValue > 255) {
                    channelValue = 255;
                }
                entry->color.blue = (u8)channelValue;

                channelValue = (s16)((f32)entry->color.alpha +
                    parameters[entryIndex].alphaIncrement);
                /* Negative alpha frees the particle scale, then still narrows into the byte. */
                if (channelValue < 0) {
                    entry->scale = 0.0f;
                }
                entry->color.alpha = (u8)channelValue;
            }

            parameters[entryIndex].scaleIncrement *=
                parameters[entryIndex].scaleIncrementMultiplier;
            entry->scale += parameters[entryIndex].scaleIncrement;
            if (--entry->framesRemaining < 1) {
                entry->scale = 0.0f;
                entry->framesRemaining = 0;
            }
            /* Each active entry flushes the entire particle array. */
            DCStoreRangeNoSync(context->entries, context->entryCount * 72);
        }
        entryIndex++;
        entry++;
    }
}

/* Position-dependent character voice settings. */

/* Player updates set voice pan and volume from position; voiceMode selects fixed components. */
void fn_1_4DB0(M617PlayerStateData *work, s16 voiceMode)
{
    f32 delta;
    f32 horizontalFraction;
    f32 verticalFraction;
    s32 horizontal;
    s32 vertical;

    if (voiceMode == 0 || voiceMode == 1 || voiceMode == 6) {
        if (lbl_1_bss_9C8.panPositionMaximum >= work->position.x &&
            lbl_1_bss_9C8.panPositionMinimum <= work->position.x) {
            horizontalFraction = (f32)(
                lbl_1_bss_9C8.panMaximum -
                lbl_1_bss_9C8.panMinimum) /
                (lbl_1_bss_9C8.panPositionMaximum -
                 lbl_1_bss_9C8.panPositionMinimum);
            delta = work->position.x - lbl_1_bss_9C8.panPositionMinimum;
            delta *= horizontalFraction;
            horizontal = (s32)delta;
            horizontal += lbl_1_bss_9C8.panMinimum;
        } else if (lbl_1_bss_9C8.panPositionMaximum < work->position.x) {
            horizontal = lbl_1_bss_9C8.panMaximum;
        } else if (lbl_1_bss_9C8.panPositionMinimum > work->position.x) {
            horizontal = lbl_1_bss_9C8.panMinimum;
        }
    }

    if (voiceMode == 0 || voiceMode == 2) {
        if (lbl_1_bss_9C8.volumePositionMaximum >= work->position.z &&
            lbl_1_bss_9C8.volumePositionMinimum <= work->position.z) {
            verticalFraction = (f32)(
                lbl_1_bss_9C8.volumeMaximum -
                lbl_1_bss_9C8.volumeMinimum) /
                (lbl_1_bss_9C8.volumePositionMaximum -
                 lbl_1_bss_9C8.volumePositionMinimum);
            delta = work->position.z - lbl_1_bss_9C8.volumePositionMinimum;
            delta *= verticalFraction;
            vertical = (s32)delta;
            vertical += lbl_1_bss_9C8.volumeMinimum;
        } else {
            if (lbl_1_bss_9C8.volumePositionMaximum < work->position.z) {
                vertical = lbl_1_bss_9C8.volumeMaximum;
            } else if (lbl_1_bss_9C8.volumePositionMinimum > work->position.z) {
                vertical = lbl_1_bss_9C8.volumeMinimum;
            }
        }
    }
    if (voiceMode == 2 || voiceMode == 3 || voiceMode == 4 ||
        voiceMode == 5) {
        horizontal = (s16)(lbl_1_bss_9C8.panMinimum +
                     (lbl_1_bss_9C8.panMaximum -
                      lbl_1_bss_9C8.panMinimum) / 2);
    }
    if (voiceMode == 1 || voiceMode == 3) {
        vertical = (s16)(lbl_1_bss_9C8.volumeMinimum +
                   (lbl_1_bss_9C8.volumeMaximum -
                    lbl_1_bss_9C8.volumeMinimum) / 2);
    }
    if (voiceMode == 4) {
        vertical = lbl_1_bss_9C8.volumeMinimum;
    }
    if (voiceMode == 5 || voiceMode == 6) {
        vertical = lbl_1_bss_9C8.volumeMaximum;
    }

    CharModelVoicePanSet(work->character, horizontal);
    CharModelVoiceVolSet(work->character, vertical);
}

static const float lbl_1_rodata_1E0 = -800.0f;

static const float lbl_1_rodata_1E4 = 800.0f;

/* The sequence start callback initializes streamed audio and the three sound slots. */
void fn_1_51B4(void) {
    s16 soundSlotIndex;

    lbl_1_bss_9C8.firstStream = -1;
    lbl_1_bss_9C8.secondStream = -1;
    lbl_1_bss_9C8.bgmId = MSM_STREAM_MGMUS_27;
    lbl_1_bss_9C8.jingleId = -1;
    lbl_1_bss_9C8.panMinimum = 32;
    lbl_1_bss_9C8.panMaximum = 96;
    lbl_1_bss_9C8.panPositionMinimum = (f32) lbl_1_rodata_1E0;
    lbl_1_bss_9C8.panPositionMaximum = (f32) lbl_1_rodata_1E4;
    lbl_1_bss_9C8.volumeMinimum = 64;
    lbl_1_bss_9C8.volumeMaximum = 110;
    lbl_1_bss_9C8.volumePositionMinimum = (f32) lbl_1_rodata_1E0;
    lbl_1_bss_9C8.volumePositionMaximum = (f32) lbl_1_rodata_1E4;
    soundSlotIndex = 0;
    while (soundSlotIndex < 3) {
        lbl_1_bss_9C8.soundActive[soundSlotIndex] = -1;
        lbl_1_bss_9C8.soundId[soundSlotIndex] = -1;
        soundSlotIndex += 1;
    }
}

/* Audio sequence helper starts a requested jingle when no jingle stream is stored. */
void fn_1_52F8(void) {
    if (((s32) lbl_1_bss_9C8.jingleId != -1) && ((s32) lbl_1_bss_9C8.secondStream == -1)) {
        lbl_1_bss_9C8.secondStream = HuAudJinglePlay((s16) lbl_1_bss_9C8.jingleId);
    }
}

/* fn_1_CCB4 starts the minigame music here when its stream handle is still unset. */
void fn_1_535C(void) {
    if ((s32) lbl_1_bss_9C8.firstStream == -1) {
        lbl_1_bss_9C8.firstStream = HuAudBGMPlay((s16) lbl_1_bss_9C8.bgmId);
    }
}

/* Audio controls fade the selected music or jingle stream and clear its stored handle. */
void fn_1_53AC(s16 useFirstStream, s16 fadeFrames)
{
    s16 shortStreamId;
    s32 streamId;

    if (useFirstStream != 0) {
        streamId = lbl_1_bss_9C8.firstStream;
    } else {
        streamId = lbl_1_bss_9C8.secondStream;
    }
    shortStreamId = (s16) streamId;
    if (shortStreamId != -1) {
        HuAudSStreamFadeOut(shortStreamId, fadeFrames);
        if (useFirstStream != 0) {
            lbl_1_bss_9C8.firstStream = -1;
        } else {
            lbl_1_bss_9C8.secondStream = -1;
        }
    }
}

/* Audio controls stop the selected music or jingle stream immediately. */
void fn_1_5458(s16 useFirstStream)
{
    s16 streamId;

    if (useFirstStream != 0) {
        streamId = (s16) lbl_1_bss_9C8.firstStream;
    } else {
        streamId = (s16) lbl_1_bss_9C8.secondStream;
    }
    HuAudSStreamStop((s32) streamId);
}

/* Player and model updates map position to sound pan and volume, then start the slot's effect. */
void fn_1_54B8(s16 slot, s16 mode, Vec *position)
{
    s32 volume;
    s32 pan;
    f32 positionDelta;
    f32 panScale;
    f32 volumeScale;

    if (mode <= 5) {
        if (mode == 0 || mode == 1) {
            if (lbl_1_bss_9C8.panPositionMaximum >= position->x &&
                lbl_1_bss_9C8.panPositionMinimum <= position->x) {
                panScale = (f32)(lbl_1_bss_9C8.panMaximum -
                                 lbl_1_bss_9C8.panMinimum) /
                           (lbl_1_bss_9C8.panPositionMaximum -
                            lbl_1_bss_9C8.panPositionMinimum);
                positionDelta = position->x -
                                lbl_1_bss_9C8.panPositionMinimum;
                positionDelta *= panScale;
                pan = (s32)positionDelta;
                pan += lbl_1_bss_9C8.panMinimum;
            } else if (lbl_1_bss_9C8.panPositionMaximum < position->x) {
                pan = lbl_1_bss_9C8.panMaximum;
            } else if (lbl_1_bss_9C8.panPositionMinimum > position->x) {
                pan = lbl_1_bss_9C8.panMinimum;
            }
        }

        if (mode == 0 || mode == 2) {
            if (lbl_1_bss_9C8.volumePositionMaximum >= position->z &&
                lbl_1_bss_9C8.volumePositionMinimum <= position->z) {
                volumeScale = (f32)(lbl_1_bss_9C8.volumeMaximum -
                                    lbl_1_bss_9C8.volumeMinimum) /
                              (lbl_1_bss_9C8.volumePositionMaximum -
                               lbl_1_bss_9C8.volumePositionMinimum);
                positionDelta = position->z -
                                lbl_1_bss_9C8.volumePositionMinimum;
                positionDelta *= volumeScale;
                volume = (s32)positionDelta;
                volume += lbl_1_bss_9C8.volumeMinimum;
            } else if (lbl_1_bss_9C8.volumePositionMaximum < position->z) {
                volume = lbl_1_bss_9C8.volumeMaximum;
            } else if (lbl_1_bss_9C8.volumePositionMinimum > position->z) {
                volume = lbl_1_bss_9C8.volumeMinimum;
            }
        }

        if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
            pan = (s16)(lbl_1_bss_9C8.panMinimum +
                        (lbl_1_bss_9C8.panMaximum -
                         lbl_1_bss_9C8.panMinimum) / 2);
        }

        if (mode == 1 || mode == 3) {
            volume = (s16)(lbl_1_bss_9C8.volumeMinimum +
                           (lbl_1_bss_9C8.volumeMaximum -
                            lbl_1_bss_9C8.volumeMinimum) / 2);
        }

        if (mode == 4) {
            volume = lbl_1_bss_9C8.volumeMinimum;
        }

        if (mode == 5) {
            volume = lbl_1_bss_9C8.volumeMaximum;
        }

        lbl_1_bss_9C8.soundActive[slot] = 1;
        lbl_1_bss_9C8.soundId[slot] =
            HuAudFXPlayVolPan(lbl_1_data_F6[slot], volume, pan);
    }
}

void fn_1_58DC(s16 slotIndex, s16 pan);

void fn_1_596C(s16 slotIndex, s16 pitch);

void fn_1_59FC(s16 slotIndex, s16 volume);

/* Sound effects selected for the participant audio slots. */
#define M617_SOUND_SLOT_FIRST 1746
#define M617_SOUND_SLOT_SECOND 1747
#define M617_SOUND_SLOT_THIRD 1748
s16 lbl_1_data_F6[4] = { M617_SOUND_SLOT_FIRST, M617_SOUND_SLOT_SECOND, M617_SOUND_SLOT_THIRD, 0 };

u8 lbl_1_data_FE[2] = { 0, 0 };
