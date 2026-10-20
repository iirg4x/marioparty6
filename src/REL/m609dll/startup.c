/* Sets up Daft Rafts players, cameras, particles, and sequence callbacks. */
#include "REL/m609dll/state.h"
#include <string.h>
#include "datadir_enum.h"
#include "game/gamework.h"
#include "game/pad.h"
#include <math.h>
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/process.h"
#include "game/memory.h"
#include "msm_stream.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/printfunc.h"
#include "game/mg/timer.h"

#define M609_MOTION_SHIFT_FLAGS_MASK 0x40000001

HU3D_MOTIONID CharMotionCreate(s16 charNo, unsigned int dataNum);

typedef struct M609PlayerVectorTable { Vec vectors[4]; } M609PlayerVectorTable;
typedef struct M609MotionData { u32 motionDataNumbers[11]; } M609MotionData;
typedef struct M609CameraMasks { s16 cameraBits[4]; } M609CameraMasks;
/* Rows store camera target, rotation, orbit distance, aspect ratio, viewport,
 * and field of view. */
typedef struct M609CameraPresetTable { f32 cameraValues[4][13]; } M609CameraPresetTable;

void fn_1_1E80(s16 cameraIndex);
void fn_1_1EB4(void);
void fn_1_234C(void);
void fn_1_D1CC(void);
void fn_1_43F4(void);
s32 fn_1_E424(void);
s32 fn_1_E474(void);
s32 fn_1_4EB0(void);
s32 fn_1_1115C(void);
s32 fn_1_111A8(void);
s32 fn_1_11658(void);
s32 fn_1_1183C(void);
s32 fn_1_1188C(void);
void fn_1_A0(void);
void fn_1_118(s16 mode, s16 frameNo);
void fn_1_25C(s16 mode, s16 frameNo);
void fn_1_288(s16 mode, s16 frameNo);
void fn_1_2B4(s16 mode, s16 frameNo);
void fn_1_2E0(s16 mode, s16 frameNo);
void fn_1_30C(s16 mode, s16 frameNo);
void fn_1_338(s16 mode, s16 frameNo);
void fn_1_364(s16 mode, s16 frameNo);
void fn_1_390(s16 mode, s16 frameNo);
void fn_1_3BC(void);
void fn_1_3C0(void);
void fn_1_670(void);
void fn_1_C30(void);
void fn_1_10FC(void);
void fn_1_120C(void);
void fn_1_19D0(s16 cameraIndex);

typedef struct ParticleColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} ParticleColor;

typedef struct ParticleEntry {
    s16 framesRemaining; /* Particle update calls before this entry expires. */
    u8 paddingBeforeVelocity[6]; /* Preserves the particle record layout. */
    struct M609Vector3 {
        f32 x;
        f32 y;
        f32 z;
    } velocity; /* Per-update world-space movement. */
    u8 paddingBeforeScale[24]; /* Preserves the particle record layout. */
    f32 scale; /* Rendered particle scale; zero marks an inactive entry. */
    u8 paddingBeforeZRotation[4]; /* Preserves the particle record layout. */
    f32 zRotation; /* Billboard Z rotation in radians, advanced by motionParameters[4]. */
    struct M609Vector3 position; /* Current world-space particle position. */
    ParticleColor color; /* Current RGBA color. */
} ParticleEntry;

typedef struct ParticleContext {
    u8 paddingBeforeActiveEntryIndex[2]; /* Preserves the particle context layout. */
    s16 activeEntryIndex; /* Index most recently assigned by the spawn helper. */
    u8 paddingBeforeParameters[24]; /* Preserves the particle context layout. */
    void *parameters; /* Per-entry color and motion settings. */
    u8 beforeEntryCount[16];
    s16 entryCount; /* Number of particle entries available to the callback. */
    u8 paddingBeforeUnusedCounter[2]; /* Preserves the particle context layout. */
    void *unusedCounterStorage; /* Unused view of the engine particle draw counter storage. */
    u8 paddingBeforeEntries[16]; /* Preserves the particle context layout. */
    ParticleEntry *entries; /* Particle records updated by the callback. */
} ParticleContext;

typedef struct ParticleConfiguration {
    s32 reservedValue;
    ParticleColor color; /* Starting RGBA color. */
    u8 targetColor[4]; /* Target RGB channels; the fourth byte is initialized but not read here. */
    /* Velocity scales, gravity, Z-rotation step, scale delta/decay, alpha step, and RGB rate. */
    f32 motionParameters[9];
} ParticleConfiguration;

/* World position used to choose horizontal stereo pan and depth-dependent volume. */
typedef struct M609VoicePanPosition { f32 x; f32 y; f32 z; } M609VoicePanPosition;

extern int fontcolor;

void fn_1_35D8(void);
void fn_1_3860(struct M609Vector3 *spawnPosition);
s16 fn_1_3A48(s16 effectIndex, struct M609Vector3 position,
              struct M609Vector3 velocity, f32 scale,
              s16 framesRemaining,
              ParticleConfiguration *configuration);
void fn_1_3BCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_234C(void);
void fn_1_46F8(s16 voiceSlot, s16 mode, M609VoicePanPosition *position);
void fn_1_3FF0(M609Player *player, s16 panMode);
void fn_1_2780(void);
void fn_1_2DF4(void);
void fn_1_32E4(MGTIMER *timer);
void fn_1_3310(MGTIMER *timer);
void fn_1_3338(MGTIMER *timer);
void fn_1_3370(s16 padNo, s16 playerNo);
void fn_1_43F4(void);
void fn_1_4538(void);
void fn_1_459C(void);
void fn_1_4698(s16 selection);
void fn_1_1E80(s16 cameraIndex);
void fn_1_1EB4(void);
void fn_1_20E0(void);
void fn_1_245C(void);
s32 fn_1_30D8(f32 *currentValue, f32 step, f32 startValue, f32 endValue);
f32 fn_1_31F8(f32 modulusOrResult);
void fn_1_45EC(s16 streamSlot, s16 fadeFrames);
MGTIMER *fn_1_3270(int type, int duration, int end, float x, float y);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

MGSEQ_PARAM lbl_1_data_0 = {0, 0, fn_1_118, fn_1_25C, fn_1_288, fn_1_2B4,
    fn_1_2E0, fn_1_30C, fn_1_338, fn_1_364, fn_1_390};

/* Runs constructors before setting up the minigame object manager and sequence. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs registered destructors when the minigame module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Initializes the actor manager, registers the camera-view object, and starts the sequence. */
void fn_1_A0(void) {
    lbl_1_bss_6D7C = MgActorObjectSetup();
    omAddObjEx(lbl_1_bss_6D7C, 32730, 0U, 0U, -1, omOutView);
    MgSeqCreate(&lbl_1_data_0);
    MgSeqModeDelaySet(2, 6000);
    MgSeqModeDelaySet(7, 6000);
}

/* Sequence initialization callback: clears module state and initializes the current day or night
 * scene. */
void fn_1_118(s16 mode, s16 frameNo) {
    s16 nightFlag;
    memset(((u8 *)&lbl_1_bss_6C34), 0, 328);
    memset(((u8 *)&lbl_1_bss_6BF8), 0, 60);
    memset(lbl_1_bss_6BBC, 0, 60);
    memset(((u8 *)&lbl_1_bss_6B6C), 0, 80);
    memset(((u8 *)&lbl_1_bss_6B64), 0, 8);
    memset(((u8 *)lbl_1_bss_8BC), 0, 25256);
    memset(((u8 *)lbl_1_bss_70C), 0, 432);
    memset(lbl_1_bss_24C, 0, 1216);
    memset(&lbl_1_bss_30, 0, 540);
    memset(((u8 *)&lbl_1_bss_4), 0, 44);
    memset(lbl_1_bss_0, 0, 4);

    nightFlag = GwMgNightF;
    lbl_1_bss_30.night = nightFlag;
    fn_1_19D0(0);
    fn_1_1EB4();
    fn_1_234C();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_D1CC();
    fn_1_43F4();
    MgSeqModeNext();
}

void fn_1_25C(s16 mode, s16 frameNo)
{
    if (fn_1_E424() != 0) MgSeqModeNext();
}

void fn_1_288(s16 mode, s16 frameNo)
{
    if (fn_1_E474() != 0) MgSeqModeNext();
}

void fn_1_2B4(s16 mode, s16 frameNo)
{
    if (fn_1_4EB0() != 0) MgSeqModeNext();
}

void fn_1_2E0(s16 mode, s16 frameNo)
{
    if (fn_1_1115C() != 0) MgSeqModeNext();
}

void fn_1_30C(s16 mode, s16 frameNo)
{
    if (fn_1_111A8() != 0) MgSeqModeNext();
}

void fn_1_338(s16 mode, s16 frameNo)
{
    if (fn_1_11658() != 0) MgSeqModeNext();
}

void fn_1_364(s16 mode, s16 frameNo)
{
    if (fn_1_1183C() != 0) MgSeqModeNext();
}

void fn_1_390(s16 mode, s16 frameNo)
{
    if (fn_1_1188C() != 0) MgSeqModeNext();
}

void fn_1_3BC(void)
{
}

/* Called during player setup to copy game settings and starting positions for four players. */
void fn_1_3C0(void) {
    M609PlayerVectorTable initialPositions = { { { -210.0f, 1370.0f, -1940.0f },
                                                 { -70.0f, 1360.0f, -1940.0f },
                                                 { 70.0f, 1360.0f, -1940.0f },
                                                 { 210.0f, 1370.0f, -1940.0f } } };
    M609PlayerVectorTable initialOrientations = {
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }
    };
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        lbl_1_bss_24C[playerNo].playerNo = playerNo;
        lbl_1_bss_24C[playerNo].characterNo = GwPlayerConf[playerNo].charNo;
        lbl_1_bss_24C[playerNo].padNo = GwPlayerConf[playerNo].padNo;
        lbl_1_bss_24C[playerNo].groupNo = GwPlayerConf[playerNo].grpNo;
        lbl_1_bss_24C[playerNo].participantType = GwPlayerConf[playerNo].type;
        lbl_1_bss_24C[playerNo].difficulty = GwPlayerConf[playerNo].comDif;
        lbl_1_bss_24C[playerNo].position = initialPositions.vectors[playerNo];
        lbl_1_bss_24C[playerNo].rotation = initialOrientations.vectors[playerNo];
        lbl_1_bss_24C[playerNo].mode = 0;
    }
}

/* Called each input update to copy pad state for active players and the minigame controller. */
void fn_1_670(void) {
    s32 playerOrPadIndex;
    s32 adjustedStickX;
    s32 filteredStickX;

    for (playerOrPadIndex = 0; playerOrPadIndex < 4; playerOrPadIndex++) {
        if (lbl_1_bss_24C[playerOrPadIndex].participantType != 0 ||
            lbl_1_bss_24C[playerOrPadIndex].disableInput != 0) {
            memset(&lbl_1_bss_24C[playerOrPadIndex], 0, 20);
        } else {
            lbl_1_bss_24C[playerOrPadIndex].padInput.buttons =
                HuPadBtn[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.buttonsDown =
                HuPadBtnDown[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.buttonsRepeat =
                HuPadBtnRep[lbl_1_bss_24C[playerOrPadIndex].padNo];
            if (HuPadStkX[lbl_1_bss_24C[playerOrPadIndex].padNo] > 0) {
                adjustedStickX =
                    HuPadStkX[lbl_1_bss_24C[playerOrPadIndex].padNo];
            } else {
                /* Negative stick values inside the ten-unit dead zone are ignored. */
                if ((HuPadStkX[lbl_1_bss_24C[playerOrPadIndex].padNo] * -1) < 10) {
                    filteredStickX = 0;
                } else {
                    filteredStickX =
                        HuPadStkX[lbl_1_bss_24C[playerOrPadIndex].padNo];
                }
                adjustedStickX = filteredStickX;
            }
            lbl_1_bss_24C[playerOrPadIndex].padInput.stickX = adjustedStickX;
            lbl_1_bss_24C[playerOrPadIndex].padInput.stickY =
                (s8)HuPadStkY[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.substickX =
                (s8)HuPadSubStkX[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.substickY =
                (s8)HuPadSubStkY[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.triggerL =
                HuPadTrigL[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.triggerR =
                HuPadTrigR[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.directions =
                HuPadDStk[lbl_1_bss_24C[playerOrPadIndex].padNo];
            lbl_1_bss_24C[playerOrPadIndex].padInput.directionsRepeat =
                HuPadDStkRep[lbl_1_bss_24C[playerOrPadIndex].padNo];
        }
    }

    for (playerOrPadIndex = 0; playerOrPadIndex < 4; playerOrPadIndex++) {
        lbl_1_bss_30.padInputs[playerOrPadIndex].buttons = HuPadBtn[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].buttonsDown = HuPadBtnDown[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].buttonsRepeat = HuPadBtnRep[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].stickX = (s8)HuPadStkX[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].stickY = (s8)HuPadStkY[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].substickX = (s8)HuPadSubStkX[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].substickY = (s8)HuPadSubStkY[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].triggerL = HuPadTrigL[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].triggerR = HuPadTrigR[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].directions = HuPadDStk[playerOrPadIndex];
        lbl_1_bss_30.padInputs[playerOrPadIndex].directionsRepeat = HuPadDStkRep[playerOrPadIndex];
    }
}

/* Called from the frame update to apply stored player and set-piece transforms to their models. */
void fn_1_C30(void) {
    s32 modelIndex;

    for (modelIndex = 4; modelIndex <= 16; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    for (modelIndex = 17; modelIndex <= 26; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    for (modelIndex = 28; modelIndex <= 31; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    for (modelIndex = 80; modelIndex <= 143; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    for (modelIndex = 144; modelIndex <= 153; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    for (modelIndex = 0; modelIndex <= 3; modelIndex++) {
        if (lbl_1_bss_24C[modelIndex].movementFlag == 0 ||
            lbl_1_bss_24C[modelIndex].movementFlag == 2) {
            Hu3DModelPosSet(
                lbl_1_bss_8BC[modelIndex].model,
                lbl_1_bss_24C[modelIndex].position.x,
                lbl_1_bss_24C[modelIndex].position.y,
                lbl_1_bss_24C[modelIndex].position.z);
            Hu3DModelRotSet(
                lbl_1_bss_8BC[modelIndex].model,
                lbl_1_bss_24C[modelIndex].rotation.x,
                lbl_1_bss_24C[modelIndex].rotation.y,
                lbl_1_bss_24C[modelIndex].rotation.z);
        }
    }
}

/* Called during character setup to load the shared motion set for each player. */
void fn_1_10FC(void) {
    M609MotionData motionData = { { DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 2),
                                    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 20),
                                    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mario, 28),
                                    DATANUM(DATA_mariomot, 24), DATANUM(DATA_mario, 33),
                                    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 50),
                                    DATANUM(DATA_mariomot, 79) } };
    s16 playerNo;
    s16 motionIndex;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        for (motionIndex = 0; motionIndex < 11; motionIndex++) {
            lbl_1_bss_70C[playerNo].motionIds[motionIndex] = CharMotionCreate(
                (s16)lbl_1_bss_24C[playerNo].characterNo,
                motionData.motionDataNumbers[motionIndex]);
        }
    }
}

/* Called by the frame update to apply queued character motion changes and track playback. */
void fn_1_120C(void)
{
    s16 playerNo = 0;

    while (playerNo < 4) {
        if (lbl_1_bss_70C[playerNo].mode !=
            lbl_1_bss_70C[playerNo].previousMode) {
            Hu3DMotionSet(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].motionIds[
                    lbl_1_bss_70C[playerNo].mode]);
            lbl_1_bss_70C[playerNo].previousMode =
                lbl_1_bss_70C[playerNo].mode;
            lbl_1_bss_70C[playerNo].maximumTime = Hu3DMotionMaxTimeGet(
                lbl_1_bss_8BC[playerNo].model);
        }
        if (lbl_1_bss_70C[playerNo].speed !=
            lbl_1_bss_70C[playerNo].previousSpeed) {
            Hu3DMotionSpeedSet(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].speed);
            lbl_1_bss_70C[playerNo].previousSpeed =
                lbl_1_bss_70C[playerNo].speed;
        }
        if (lbl_1_bss_70C[playerNo].attributes !=
            lbl_1_bss_70C[playerNo].previousAttributes) {
            Hu3DModelAttrSet(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].attributes);
            lbl_1_bss_70C[playerNo].previousAttributes =
                lbl_1_bss_70C[playerNo].attributes;
        }
        if (lbl_1_bss_70C[playerNo].attributesToReset != 0) {
            Hu3DModelAttrReset(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].attributesToReset);
            /* Complement the reset mask before retaining the remaining attributes. */
            lbl_1_bss_70C[playerNo].attributesToReset =
                ~lbl_1_bss_70C[playerNo].attributesToReset;
            lbl_1_bss_70C[playerNo].previousAttributes =
                lbl_1_bss_70C[playerNo].attributes =
                    lbl_1_bss_70C[playerNo].attributes &
                    lbl_1_bss_70C[playerNo].attributesToReset;
            lbl_1_bss_70C[playerNo].attributesToReset = 0;
        }
        if (lbl_1_bss_70C[playerNo].time !=
            lbl_1_bss_70C[playerNo].previousTime) {
            Hu3DMotionTimeSet(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].time);
        }

        switch (lbl_1_bss_70C[playerNo].shiftState) {
        case 0:
            break;
        case 1:
            Hu3DMotionShiftSet(lbl_1_bss_8BC[playerNo].model,
                lbl_1_bss_70C[playerNo].motionIds[
                    lbl_1_bss_70C[playerNo].shiftMotion],
                lbl_1_bss_70C[playerNo].shiftStart,
                lbl_1_bss_70C[playerNo].blendDuration,
                lbl_1_bss_70C[playerNo].shiftFlags);
            lbl_1_bss_70C[playerNo].shiftMaximumTime =
                Hu3DMotionShiftMaxTimeGet(lbl_1_bss_8BC[playerNo].model);
            lbl_1_bss_70C[playerNo].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_70C[playerNo].shiftTime = Hu3DMotionShiftTimeGet(
                lbl_1_bss_8BC[playerNo].model);
            /* The motion tag or loop bit prevents this state advance; otherwise compare playback
             * time with the blend duration minus one. */
            if ((lbl_1_bss_70C[playerNo].shiftFlags & M609_MOTION_SHIFT_FLAGS_MASK) == 0 &&
                lbl_1_bss_70C[playerNo].shiftTime >=
                    lbl_1_bss_70C[playerNo].blendDuration - 1.0f) {
                lbl_1_bss_70C[playerNo].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_70C[playerNo].shiftState = 0;
            break;
        }

        lbl_1_bss_70C[playerNo].time = Hu3DMotionTimeGet(
            lbl_1_bss_8BC[playerNo].model);
        lbl_1_bss_70C[playerNo].previousTime =
            lbl_1_bss_70C[playerNo].time;
        lbl_1_bss_70C[playerNo].motionEnded = Hu3DMotionEndCheck(
            lbl_1_bss_8BC[playerNo].model);
        if (lbl_1_bss_70C[playerNo].motionEnded != 0 &&
            lbl_1_bss_70C[playerNo].resetRequested != 0) {
            lbl_1_bss_70C[playerNo].motionEnded = 0;
            lbl_1_bss_70C[playerNo].resetRequested = 0;
            lbl_1_bss_70C[playerNo].mode =
                lbl_1_bss_70C[playerNo].resetMode;
            lbl_1_bss_70C[playerNo].time =
                lbl_1_bss_70C[playerNo].transition;
            lbl_1_bss_70C[playerNo].attributes =
                lbl_1_bss_70C[playerNo].resetAttributes;
            lbl_1_bss_70C[playerNo].attributesToReset =
                lbl_1_bss_70C[playerNo].resetAttributesToClear;
        }
        playerNo++;
    }
}

/* Creates a camera during scene setup and loads its target, rotation, orbit distance,
 * and lens settings. */
void fn_1_19D0(s16 cameraIndex) {
    M609CameraPresetTable presets = { { { 0.0f, 1454.0f, -1850.0f, -44.0f, 0.0f, 0.0f, 812.0f,
                                          1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 32.2f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f } } };
    M609CameraMasks cameraMasks = {{1, 2, 4, 8}};

    lbl_1_bss_6C34.cameras[cameraIndex].cameraIndex = cameraIndex;
    lbl_1_bss_6C34.cameras[cameraIndex].target.x = presets.cameraValues[cameraIndex][0];
    lbl_1_bss_6C34.cameras[cameraIndex].target.y = presets.cameraValues[cameraIndex][1];
    lbl_1_bss_6C34.cameras[cameraIndex].target.z = presets.cameraValues[cameraIndex][2];
    lbl_1_bss_6C34.cameras[cameraIndex].rotation.x = presets.cameraValues[cameraIndex][3];
    lbl_1_bss_6C34.cameras[cameraIndex].rotation.y = presets.cameraValues[cameraIndex][4];
    lbl_1_bss_6C34.cameras[cameraIndex].rotation.z = presets.cameraValues[cameraIndex][5];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[0] =
        presets.cameraValues[cameraIndex][6];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[1] =
        presets.cameraValues[cameraIndex][7];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[2] =
        presets.cameraValues[cameraIndex][8];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[3] =
        presets.cameraValues[cameraIndex][9];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[4] =
        presets.cameraValues[cameraIndex][10];
    lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[5] =
        presets.cameraValues[cameraIndex][11];
    lbl_1_bss_6C34.cameras[cameraIndex].fieldOfView = presets.cameraValues[cameraIndex][12];

    Hu3DCameraCreate(cameraMasks.cameraBits[cameraIndex]);
    Hu3DCameraViewportSet(cameraMasks.cameraBits[cameraIndex],
                          lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[2],
                          lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[3],
                          lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[4],
                          lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[5],
                          (0.0f),
                          1.0f);
    Hu3DCameraPerspectiveSet(cameraMasks.cameraBits[cameraIndex],
                             lbl_1_bss_6C34.cameras[cameraIndex].fieldOfView,
                             (20.0f), (20000.0f),
                             lbl_1_bss_6C34.cameras[cameraIndex].projectionAndViewport[1]);
    Center.x = presets.cameraValues[cameraIndex][0];
    Center.y = presets.cameraValues[cameraIndex][1];
    Center.z = presets.cameraValues[cameraIndex][2];
    CRot.x = presets.cameraValues[cameraIndex][3];
    CRot.y = presets.cameraValues[cameraIndex][4];
    CRot.z = presets.cameraValues[cameraIndex][5];
    CZoom = presets.cameraValues[cameraIndex][6];
    fn_1_1E80(cameraIndex);
}

/* Called during camera setup and frame updates; its local camera mask values are unused. */
void fn_1_1E80(s16 cameraIndex) {
    struct Values {
        u16 cameraOneBit;
        u16 cameraTwoBit;
        u16 cameraFourBit;
        u16 cameraEightBit;
    } cameraMasks = {1, 2, 4, 8};

}

/* Called during scene setup to create the minigame's directional light and set its color. */
void fn_1_1EB4(void) {
    Hu3DLighInit();
    lbl_1_bss_6BF8.position.x = (0.0f);
    lbl_1_bss_6BF8.position.y = (0.0f);
    lbl_1_bss_6BF8.position.z = (0.0f);
    lbl_1_bss_6BF8.aim.x = (0.0f);
    lbl_1_bss_6BF8.aim.y = (0.0f);
    lbl_1_bss_6BF8.aim.z = (0.0f);
    lbl_1_bss_6BF8.orientation.x = (-100.0f);
    lbl_1_bss_6BF8.orientation.y = (0.0f);
    lbl_1_bss_6BF8.orientation.z = (0.0f);
    lbl_1_bss_6BF8.range = (1000.0f);

    if (lbl_1_bss_30.night == 0) {
        lbl_1_bss_6BF8.color.r = 255;
        lbl_1_bss_6BF8.color.g = 255;
        lbl_1_bss_6BF8.color.b = 255;
        lbl_1_bss_6BF8.color.a = 255;
    } else {
        lbl_1_bss_6BF8.color.r = 64;
        lbl_1_bss_6BF8.color.g = 64;
        lbl_1_bss_6BF8.color.b = 64;
        lbl_1_bss_6BF8.color.a = 64;
    }

    PSVECSubtract(&lbl_1_bss_6BF8.aim, &lbl_1_bss_6BF8.position,
                  &lbl_1_bss_6BF8.direction);
    lbl_1_bss_6BF8.id = Hu3DGLightCreateV(
        &lbl_1_bss_6BF8.position, &lbl_1_bss_6BF8.direction,
        &lbl_1_bss_6BF8.color);
    fn_1_20E0();
    Hu3DGLightStaticSet(lbl_1_bss_6BF8.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_6BF8.id);
}

/* Called by setup and camera updates to place the directional light from its angles. */
void fn_1_20E0(void) {
    lbl_1_bss_6BF8.position.x =
        lbl_1_bss_6BF8.aim.x +
        lbl_1_bss_6BF8.range *
            (cos((3.141592653589793) * lbl_1_bss_6BF8.orientation.x /
                 (180.0)) *
             sin((3.141592653589793) * lbl_1_bss_6BF8.orientation.y /
                 (180.0)));
    lbl_1_bss_6BF8.position.y =
        lbl_1_bss_6BF8.aim.y +
        lbl_1_bss_6BF8.range *
            -sin((3.141592653589793) * lbl_1_bss_6BF8.orientation.x /
                 (180.0));
    lbl_1_bss_6BF8.position.z =
        lbl_1_bss_6BF8.aim.z +
        lbl_1_bss_6BF8.range *
            (cos((3.141592653589793) * lbl_1_bss_6BF8.orientation.x /
                 (180.0)) *
             cos((3.141592653589793) * lbl_1_bss_6BF8.orientation.y /
                 (180.0)));
    PSVECSubtract(&lbl_1_bss_6BF8.aim, &lbl_1_bss_6BF8.position,
                  &lbl_1_bss_6BF8.direction);
    PSVECNormalize(&lbl_1_bss_6BF8.direction, &lbl_1_bss_6BF8.direction);
    Hu3DGLightPosSetV(lbl_1_bss_6BF8.id, &lbl_1_bss_6BF8.position,
                      &lbl_1_bss_6BF8.direction);
    Hu3DGLightColorSet(lbl_1_bss_6BF8.id, lbl_1_bss_6BF8.color.r,
                       lbl_1_bss_6BF8.color.g, lbl_1_bss_6BF8.color.b,
                       lbl_1_bss_6BF8.color.a);
}

/* Called during scene setup to initialize the board shadow camera and transparency. */
void fn_1_234C(void) {
    Hu3DShadowCreate((5.0f), (19500.0f),
                     (22500.0f));
    Hu3DShadowSizeSet(480);
    lbl_1_bss_6B6C.verticalAngle = (-90.0f);
    lbl_1_bss_6B6C.horizontalAngle = (0.0f);
    /* This setup value is stored but is not read by this module. */
    lbl_1_bss_6B6C.state14 = (0.0f);
    lbl_1_bss_6B6C.target.x = (0.0f);
    lbl_1_bss_6B6C.target.y = (-100.0f);
    lbl_1_bss_6B6C.target.z = (-1940.0f);
    lbl_1_bss_6B6C.shadowTransparency = (0.5f);
    lbl_1_bss_6B6C.orbitDistance = (21500.0f);
    fn_1_245C();
}

/* Called by the frame update to position the shadow camera around its target. */
void fn_1_245C(void)
{
    lbl_1_bss_6B6C.cameraPosition.x = (f32)((f64)lbl_1_bss_6B6C.target.x +
        ((f64)lbl_1_bss_6B6C.orbitDistance *
         (cos(((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0)) *
          sin(((3.141592653589793) * (f64)lbl_1_bss_6B6C.horizontalAngle) / (180.0)))));
    lbl_1_bss_6B6C.cameraPosition.y = (f32)((f64)lbl_1_bss_6B6C.target.y +
        ((f64)lbl_1_bss_6B6C.orbitDistance *
         -sin(((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0))));
    lbl_1_bss_6B6C.cameraPosition.z = (f32)((f64)lbl_1_bss_6B6C.target.z +
        ((f64)lbl_1_bss_6B6C.orbitDistance *
         (cos(((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0)) *
          cos(((3.141592653589793) * (f64)lbl_1_bss_6B6C.horizontalAngle) / (180.0)))));

    lbl_1_bss_6B6C.cameraUp.x = (f32)(
        sin(((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0)) *
        sin(((3.141592653589793) * (f64)lbl_1_bss_6B6C.horizontalAngle) / (180.0)));
    lbl_1_bss_6B6C.cameraUp.y = (f32)cos(
        ((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0));
    lbl_1_bss_6B6C.cameraUp.z = (f32)(
        sin(((3.141592653589793) * (f64)lbl_1_bss_6B6C.verticalAngle) / (180.0)) *
        cos(((3.141592653589793) * (f64)lbl_1_bss_6B6C.horizontalAngle) / (180.0)));

    Hu3DShadowTPLvlSet(lbl_1_bss_6B6C.shadowTransparency);
    Hu3DShadowPosSet(&lbl_1_bss_6B6C.cameraPosition, &lbl_1_bss_6B6C.cameraUp,
                     &lbl_1_bss_6B6C.target);
}

/* Called during each frame to update actor and model positions from player state. */
void fn_1_2780(void) {
    Vec position;
    s16 playerIndex;

    fontcolor = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        position.x = lbl_1_bss_24C[playerIndex].position.x;
        position.y = lbl_1_bss_24C[playerIndex].position.y;
        position.z = lbl_1_bss_24C[playerIndex].position.z;
        MgActorPosSetRaw(
            lbl_1_bss_4.players[playerIndex]->actor,
            &position);
        lbl_1_bss_4.players[playerIndex]->actor->rotY =
            lbl_1_bss_24C[playerIndex].rotation.y;
    }
    fn_1_C30();
    fn_1_120C();
    fn_1_1E80(0);
    fn_1_20E0();
    fn_1_245C();
}

/* Runs the actor manager and copies active actors' positions and yaw into player state.
 * Disabled or computer input also replaces the shared pad stick values and button state;
 * MgPlayerPadSet preserves existing Start and Z button bits. */
void fn_1_2DF4(void) {
    Point3d actorPosition;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_24C[playerIndex].movementFlag != 0) {
            if (lbl_1_bss_24C[playerIndex].disableInput != 0) {
                MgPlayerPadSet(lbl_1_bss_4.players[playerIndex], 0, 0, 0, 0);
            } else if (lbl_1_bss_24C[playerIndex].participantType != 0) {
                MgPlayerPadSet(lbl_1_bss_4.players[playerIndex],
                    lbl_1_bss_24C[playerIndex].computerStickX,
                    lbl_1_bss_24C[playerIndex].computerStickY,
                    lbl_1_bss_24C[playerIndex].computerButtonsDown,
                    lbl_1_bss_24C[playerIndex].computerButtonsHeld);
            }
            MgActorPosGet(lbl_1_bss_4.players[playerIndex]->actor, &actorPosition);
            lbl_1_bss_24C[playerIndex].previousPosition.x = actorPosition.x;
            lbl_1_bss_24C[playerIndex].previousPosition.y = actorPosition.y;
            lbl_1_bss_24C[playerIndex].previousPosition.z = actorPosition.z;
            lbl_1_bss_24C[playerIndex].previousCollisionMesh =
                lbl_1_bss_24C[playerIndex].collisionMesh;
            lbl_1_bss_24C[playerIndex].collisionMesh = -1;
            lbl_1_bss_24C[playerIndex].linkState = lbl_1_bss_24C[playerIndex].linkedPlayer;
            lbl_1_bss_24C[playerIndex].linkedPlayer = -1;
        }
        playerIndex += 1;
    }
    MgActorExec();
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_24C[playerIndex].movementFlag != 0) {
            MgActorPosGet(lbl_1_bss_4.players[playerIndex]->actor, &actorPosition);
            lbl_1_bss_24C[playerIndex].position.x = actorPosition.x;
            lbl_1_bss_24C[playerIndex].position.y = actorPosition.y;
            lbl_1_bss_24C[playerIndex].position.z = actorPosition.z;
            lbl_1_bss_24C[playerIndex].rotation.y = lbl_1_bss_4.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

/* Adds the initial end value minus the initial start value, divided by step, and estimates
 * the remaining updates. The clamp uses the initial start value plus the end value or
 * current value in the non-positive branches, so it can snap to the endpoint before
 * a conventional distance check would. */
s32 fn_1_30D8(f32 *currentValue, f32 step, f32 startOrClampValue, f32 endValue)
{
    f32 startClampValue;
    f32 currentClampValue;
    s32 frames = 0;
    f32 incrementOrClampValue = (endValue - startOrClampValue) / step;

    *currentValue += incrementOrClampValue;
    if (incrementOrClampValue) {
        frames = (s32)((endValue - *currentValue) / incrementOrClampValue);
    }
    frames++;
    if (startOrClampValue - endValue > (0.0f)) {
        startClampValue = startOrClampValue - endValue;
    } else {
        startClampValue = startOrClampValue - (-1.0f) * endValue;
    }
    incrementOrClampValue = startClampValue;
    if (startOrClampValue - *currentValue > (0.0f)) {
        currentClampValue = startOrClampValue - *currentValue;
    } else {
        currentClampValue = startOrClampValue - (-1.0f) * *currentValue;
    }
    startOrClampValue = currentClampValue;
    if (incrementOrClampValue <= startOrClampValue) {
        *currentValue = endValue;
        frames = 0;
    }
    return frames;
}

/* Truncates the modulus to an integer for frandmod and returns its integer result as a float. */
f32 fn_1_31F8(f32 modulusOrResult) {
    s32 integerModulusOrResult = modulusOrResult;
    integerModulusOrResult = frandmod(integerModulusOrResult);
    modulusOrResult = integerModulusOrResult;
    return modulusOrResult;
}

/* Unused timer helper: converts duration and endpoint from seconds to frames
 * and sets its position. */
MGTIMER *fn_1_3270(int type, int duration, int end, float x, float y)
{
    MGTIMER *timer = MgTimerCreate(type);
    MgTimerParamSet(timer, duration * 60, end * 60, 0);
    MgTimerPosSet(timer, x, y);
    return timer;
}

void fn_1_32E4(MGTIMER *timer) {
    MgTimerModeOnSet(timer, 1);
}

/* Queries timer completion and discards the result; the timer is left unchanged. */
void fn_1_3310(MGTIMER *timer) {
    MgTimerDoneCheck(timer);
}

void fn_1_3338(MGTIMER *timer) {
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

/* Applies debug stick movement to a player position and prints the resulting coordinates. */
void fn_1_3370(s16 padNo, s16 playerNo) {
    if (HuPadStkX[padNo] > 10) {
        lbl_1_bss_24C[playerNo].position.x += (10.0f);
    }
    if (HuPadStkX[padNo] < -10) {
        lbl_1_bss_24C[playerNo].position.x -= (10.0f);
    }
    if (HuPadStkY[padNo] > 10) {
        lbl_1_bss_24C[playerNo].position.z -= (10.0f);
    }
    if (HuPadStkY[padNo] < -10) {
        lbl_1_bss_24C[playerNo].position.z += (10.0f);
    }
    /* These overlapping substick checks can apply both vertical adjustments near center. */
    if (HuPadSubStkY[padNo] < 10) {
        lbl_1_bss_24C[playerNo].position.y -= (10.0f);
    }
    if (HuPadSubStkY[padNo] > -10) {
        lbl_1_bss_24C[playerNo].position.y += (10.0f);
    }

    print8(25, 86, (1.5f), "OBJ X:%.1f Y:%.1f Z:%.1f",
           lbl_1_bss_24C[playerNo].position.x,
           lbl_1_bss_24C[playerNo].position.y,
           lbl_1_bss_24C[playerNo].position.z);
}

s16 lbl_1_data_42[11] = {1629, 1630, 1631, 1632, 1633, 1634,
    1635, 1636, 1637, 0, 0};

/* Unused particle setup helper: allocates a thousand-entry pool and attaches its update
 * callback. */
void fn_1_35D8(void) {
    u32 animationFiles[1] = {DATANUM(DATA_m609, 50)};
    s16 particleCounts[1] = {1000};
    s16 layers[1] = {6};
    s16 cameras[1] = {1};
    s16 blendModes[1] = {1};
    s16 effectIndex;

    for (effectIndex = 0; effectIndex < 1; effectIndex++) {
        ANIMDATA *animation;
        HU3D_PARTICLE *particle;
        HU3D_PARTICLE_DATA *data;
        s16 particleIndex;

        lbl_1_bss_6B64.particleCounts[effectIndex] = particleCounts[effectIndex];
        animation = HuSprAnimRead(HuDataSelHeapReadNum(
            animationFiles[effectIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_6B64.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, particleCounts[effectIndex] * 48, HU_MEMNUM_OVL);
        lbl_1_bss_6B64.modelIds[effectIndex] = Hu3DParticleCreate(
            animation, particleCounts[effectIndex]);
        Hu3DModelLayerSet(lbl_1_bss_6B64.modelIds[effectIndex],
                          layers[effectIndex]);
        Hu3DParticleHookSet(lbl_1_bss_6B64.modelIds[effectIndex], fn_1_3BCC);
        Hu3DModelCameraSet(lbl_1_bss_6B64.modelIds[effectIndex], (u16)cameras[effectIndex]);
        Hu3DParticleBlendModeSet(lbl_1_bss_6B64.modelIds[effectIndex],
                                 (u8)blendModes[effectIndex]);

        particle = Hu3DData[lbl_1_bss_6B64.modelIds[effectIndex]].hookData;
        particle->emitCnt = 0;
        particle->work = lbl_1_bss_6B64.allocation[effectIndex];
        particle->count = 0;
        particleIndex = 0;
        data = particle->data;
        for (; particleIndex < particle->maxCnt; particleIndex++, data++) {
            data->scale = 0.0f;
        }
    }
}

/* Unused spawn helper: attempts twelve overlapping particles at the supplied position,
 * all with the same upward velocity. */
void fn_1_3860(struct M609Vector3 *spawnPosition) {
    ParticleConfiguration configuration;
    struct M609Vector3 velocity;
    struct M609Vector3 landingPosition;
    s16 particleAttempt;
    f32 scale;

    configuration.reservedValue = 0;
    configuration.color.red = -1;
    configuration.color.green = -1;
    configuration.color.blue = -1;
    configuration.color.alpha = -1;
    configuration.targetColor[0] = 0;
    configuration.targetColor[1] = 0;
    configuration.targetColor[2] = 0;
    configuration.targetColor[3] = 0;
    configuration.motionParameters[0] = 1.0f;
    configuration.motionParameters[1] = 0.9f;
    configuration.motionParameters[2] = 1.0f;
    configuration.motionParameters[4] = 0.0f;
    configuration.motionParameters[5] = 0.0f;
    configuration.motionParameters[6] = 0.0f;
    /* The callback truncates alpha to an integer each update;
     * this increment leaves alpha at 255. */
    configuration.motionParameters[7] = 0.01f;
    configuration.motionParameters[8] = 0.01f;
    for (particleAttempt = 0; particleAttempt < 12; particleAttempt++) {
        configuration.motionParameters[3] = 0.0f;
        landingPosition.x = spawnPosition->x;
        landingPosition.y = spawnPosition->y;
        landingPosition.z = spawnPosition->z;
        velocity.x = 0.0f;
        velocity.y = 10.0f;
        velocity.z = 0.0f;
        scale = 80.0f;
        fn_1_3A48(0, landingPosition, velocity, scale, 120,
                  &configuration);
    }
}

/* Called by fn_1_3860 to initialize one inactive landing-dust entry. */
s16 fn_1_3A48(s16 effectIndex, struct M609Vector3 position,
              struct M609Vector3 velocity, f32 scale,
              s16 framesRemaining,
              ParticleConfiguration *configuration) {
    HU3D_MODEL *modelData;
    s16 spawnedParticleIndex;
    ParticleContext *context;
    ParticleConfiguration *configurations;
    s16 particleIndex;
    ParticleEntry *entry;

    spawnedParticleIndex = -1;
    modelData = &Hu3DData[lbl_1_bss_6B64.modelIds[effectIndex]];
    context = modelData->hookData;
    configurations = context->parameters;
    particleIndex = 0;
    entry = context->entries;

    for (; particleIndex < context->entryCount; particleIndex++, entry++) {
        if (entry->scale == 0.0f) {
            configurations[particleIndex] = *configuration;
            entry->position = position;
            entry->velocity = velocity;
            entry->color = configurations[particleIndex].color;
            entry->scale = scale;
            entry->framesRemaining = framesRemaining;
            context->activeEntryIndex = particleIndex;
            spawnedParticleIndex = particleIndex;
            break;
        }
    }

    return spawnedParticleIndex;
}

/* Particle callback: advances dust motion, fades its colors, and retires expired entries. */
void fn_1_3BCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix) {
    ParticleConfiguration *parameters = ((ParticleContext *)particle)->parameters;
    ParticleEntry *entry = ((ParticleContext *)particle)->entries;
    s16 entryNo = 0;

    while (entryNo < ((ParticleContext *)particle)->entryCount) {
        if (entry->scale > 0.0f) {
            s16 channelValue;

            entry->velocity.x *= parameters[entryNo].motionParameters[0];
            entry->velocity.y *= parameters[entryNo].motionParameters[1];
            entry->velocity.z *= parameters[entryNo].motionParameters[2];
            PSVECAdd((Point3d *)&entry->velocity, (Point3d *)&entry->position,
                     (Point3d *)&entry->position);
            entry->velocity.y += parameters[entryNo].motionParameters[3];
            entry->zRotation += parameters[entryNo].motionParameters[4];

            channelValue = (s16)((f32)entry->color.red +
                                  (parameters[entryNo].motionParameters[8] *
                                   (f32)(parameters[entryNo].targetColor[0] -
                                        parameters[entryNo].color.red)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.red = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.green +
                                  (parameters[entryNo].motionParameters[8] *
                                   (f32)(parameters[entryNo].targetColor[1] -
                                        parameters[entryNo].color.green)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.green = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.blue +
                                  (parameters[entryNo].motionParameters[8] *
                                   (f32)(parameters[entryNo].targetColor[2] -
                                        parameters[entryNo].color.blue)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.blue = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.alpha +
                                  parameters[entryNo].motionParameters[7]);
            if (channelValue < 0) {
                entry->scale = 0.0f;
            }
            entry->color.alpha = (u8)channelValue;

            parameters[entryNo].motionParameters[5] *=
                parameters[entryNo].motionParameters[6];
            entry->scale += parameters[entryNo].motionParameters[5];
            if (--entry->framesRemaining <= 0) {
                entry->scale = 0.0f;
            }
            /* Store the whole particle pool for each active entry,
             * including entries already stored above. */
            DCStoreRangeNoSync(((ParticleContext *)particle)->entries,
                                ((ParticleContext *)particle)->entryCount * 72);
        }
        entryNo++;
        entry++;
    }
}

/* Reaction updates use mode 0 to map player X/Z to voice pan/volume. Mode 1 fixes volume
 * at the midpoint; mode 2 fixes pan at the midpoint; modes 3 through 5 use fixed pan and
 * volume. Mode 6 maps X to pan and fixes volume at the high bound. */
void fn_1_3FF0(M609Player *player, s16 panMode) {
    f32 delta;
    f32 panPerWorldUnit;
    f32 volumeScale;
    s32 horizontal;
    s32 volume;

    if (panMode == 0 || panMode == 1 || panMode == 6) {
        if (lbl_1_bss_30.positionXHigh >= player->position.x &&
            lbl_1_bss_30.positionXLow <= player->position.x) {
            panPerWorldUnit = (f32)(lbl_1_bss_30.panXHigh -
                                       lbl_1_bss_30.panXLow) /
                                 (lbl_1_bss_30.positionXHigh -
                                  lbl_1_bss_30.positionXLow);
            delta = player->position.x - lbl_1_bss_30.positionXLow;
            delta *= panPerWorldUnit;
            horizontal = (s32)delta;
            horizontal += lbl_1_bss_30.panXLow;
        } else if (lbl_1_bss_30.positionXHigh < player->position.x) {
            horizontal = lbl_1_bss_30.panXHigh;
        } else if (lbl_1_bss_30.positionXLow > player->position.x) {
            horizontal = lbl_1_bss_30.panXLow;
        }
    }
    if (panMode == 0 || panMode == 2) {
        if (lbl_1_bss_30.positionZHigh >= player->position.z &&
            lbl_1_bss_30.positionZLow <= player->position.z) {
            volumeScale = (f32)(lbl_1_bss_30.volumeZHigh -
                                     lbl_1_bss_30.volumeZLow) /
                               (lbl_1_bss_30.positionZHigh -
                                lbl_1_bss_30.positionZLow);
            delta = player->position.z - lbl_1_bss_30.positionZLow;
            delta *= volumeScale;
            volume = (s32)delta;
            volume += lbl_1_bss_30.volumeZLow;
        } else if (lbl_1_bss_30.positionZHigh < player->position.z) {
            volume = lbl_1_bss_30.volumeZHigh;
        } else if (lbl_1_bss_30.positionZLow > player->position.z) {
            volume = lbl_1_bss_30.volumeZLow;
        }
    }
    if (panMode == 2 || panMode == 3 || panMode == 4 || panMode == 5) {
        horizontal = (s16)(lbl_1_bss_30.panXLow +
                           (lbl_1_bss_30.panXHigh -
                            lbl_1_bss_30.panXLow) / 2);
    }
    if (panMode == 1 || panMode == 3) {
        volume = (s16)(lbl_1_bss_30.volumeZLow +
                         (lbl_1_bss_30.volumeZHigh -
                          lbl_1_bss_30.volumeZLow) / 2);
    }
    if (panMode == 4) {
        volume = lbl_1_bss_30.volumeZLow;
    }
    if (panMode == 5 || panMode == 6) {
        volume = lbl_1_bss_30.volumeZHigh;
    }
    CharModelVoicePanSet(player->characterNo, horizontal);
    CharModelVoiceVolSet(player->characterNo, volume);
}

/* Initializes music handles, sound slots, horizontal pan ranges, and depth-volume bounds. */
void fn_1_43F4(void) {
    s16 voiceSlot;

    (lbl_1_bss_30.musicHandle) = -1;
    (lbl_1_bss_30.jingleHandle) = -1;
    (lbl_1_bss_30.musicId) = MSM_STREAM_MGTOUR_END;
    (lbl_1_bss_30.jingleId) = -1;
    (lbl_1_bss_30.panXLow) = 32;
    (lbl_1_bss_30.panXHigh) = 96;
    (lbl_1_bss_30.positionXLow) = (f32) (-600.0f);
    (lbl_1_bss_30.positionXHigh) = (f32) (600.0f);
    (lbl_1_bss_30.volumeZLow) = 96;
    (lbl_1_bss_30.volumeZHigh) = 110;
    (lbl_1_bss_30.positionZLow) = (f32) (-800.0f);
    (lbl_1_bss_30.positionZHigh) = (f32) (100.0f);
    voiceSlot = 0;
    while (voiceSlot < 9) {
        (lbl_1_bss_30.soundSlotStates[voiceSlot]) = -1;
        (lbl_1_bss_30.soundHandles[voiceSlot]) = -1;
        voiceSlot += 1;
    }
}

/* Unused helper: starts a queued result jingle when no jingle handle is stored. */
void fn_1_4538(void) {
    if (((s32) (lbl_1_bss_30.jingleId) != -1) &&
        ((s32) (lbl_1_bss_30.jingleHandle) == -1)) {
        (lbl_1_bss_30.jingleHandle) =
            HuAudJinglePlay((s16) (lbl_1_bss_30.jingleId));
    }
}

/* Starts the configured background music when no music handle is stored. */
void fn_1_459C(void) {
    if ((s32) (lbl_1_bss_30.musicHandle) == -1) {
        (lbl_1_bss_30.musicHandle) =
            HuAudBGMPlay((s16) (lbl_1_bss_30.musicId));
    }
}

/* Requests a music or jingle fade during sequence transitions and immediately clears
 * its stored handle, allowing another start while the fade may still be active. */
void fn_1_45EC(s16 streamSlot, s16 fadeFrames) {
    s16 streamId = streamSlot != 0
                       ? lbl_1_bss_30.musicHandle
                       : lbl_1_bss_30.jingleHandle;
    if (streamId != -1) {
        HuAudSStreamFadeOut(streamId, fadeFrames);
        if (streamSlot != 0) {
            lbl_1_bss_30.musicHandle = -1;
            return;
        }
        lbl_1_bss_30.jingleHandle = -1;
    }
}

/* Unused helper: stops the selected music or jingle stream
 * and leaves its stored handle unchanged. */
void fn_1_4698(s16 selection) {
    s16 streamId;

    if (selection != 0) {
        streamId = lbl_1_bss_30.musicHandle;
    } else {
        streamId = lbl_1_bss_30.jingleHandle;
    }
    HuAudSStreamStop(streamId);
}

/* Plays the selected course sound effect. Mode 0 maps X/Z to pan/volume; mode 1 maps X
 * with midpoint volume, and mode 2 maps Z with midpoint pan. Modes 3 through 5 use fixed
 * values; slot 7 is not replayed while its slot is enabled. */
void fn_1_46F8(s16 voiceSlot, s16 panMode, M609VoicePanPosition *position) {
    f32 xPanScale;
    f32 volumeScale;
    f32 scaledOffset;
    s32 volume;
    s32 panX;

    if (panMode <= 5 && (voiceSlot != 7 || lbl_1_bss_30.soundSlotStates[voiceSlot] == -1)) {
        if (panMode == 0 || panMode == 1) {
            if (lbl_1_bss_30.positionXHigh >= position->x &&
                lbl_1_bss_30.positionXLow <= position->x) {
                xPanScale = (f32)(lbl_1_bss_30.panXHigh -
                                  lbl_1_bss_30.panXLow) /
                            (lbl_1_bss_30.positionXHigh -
                             lbl_1_bss_30.positionXLow);
                scaledOffset = position->x - lbl_1_bss_30.positionXLow;
                scaledOffset *= xPanScale;
                panX = (s32)scaledOffset;
                panX += lbl_1_bss_30.panXLow;
            } else if (lbl_1_bss_30.positionXHigh < position->x) {
                panX = lbl_1_bss_30.panXHigh;
            } else if (lbl_1_bss_30.positionXLow > position->x) {
                panX = lbl_1_bss_30.panXLow;
            }
        }
        if (panMode == 0 || panMode == 2) {
            if (lbl_1_bss_30.positionZHigh >= position->z &&
                lbl_1_bss_30.positionZLow <= position->z) {
                volumeScale = (f32)(lbl_1_bss_30.volumeZHigh -
                                  lbl_1_bss_30.volumeZLow) /
                            (lbl_1_bss_30.positionZHigh -
                             lbl_1_bss_30.positionZLow);
                scaledOffset = position->z - lbl_1_bss_30.positionZLow;
                scaledOffset *= volumeScale;
                volume = (s32)scaledOffset;
                volume += lbl_1_bss_30.volumeZLow;
            } else if (lbl_1_bss_30.positionZHigh < position->z) {
                volume = lbl_1_bss_30.volumeZHigh;
            } else if (lbl_1_bss_30.positionZLow > position->z) {
                volume = lbl_1_bss_30.volumeZLow;
            }
        }
        if (panMode == 2 || panMode == 3 || panMode == 4 || panMode == 5) {
            panX = (s16)(lbl_1_bss_30.panXLow +
                         (lbl_1_bss_30.panXHigh -
                          lbl_1_bss_30.panXLow) / 2);
        }
        if (panMode == 1 || panMode == 3) {
            volume = (s16)(lbl_1_bss_30.volumeZLow +
                         (lbl_1_bss_30.volumeZHigh -
                          lbl_1_bss_30.volumeZLow) / 2);
        }
        if (panMode == 4) {
            volume = lbl_1_bss_30.volumeZLow;
        }
        if (panMode == 5) {
            volume = lbl_1_bss_30.volumeZHigh;
        }
        lbl_1_bss_30.soundSlotStates[voiceSlot] = 1;
        lbl_1_bss_30.soundHandles[voiceSlot] =
            HuAudFXPlayVolPan(lbl_1_data_42[voiceSlot], volume, panX);
    }
}

HUPROCESS *lbl_1_bss_6D7C;
M609CameraBank lbl_1_bss_6C34;
M609LightState lbl_1_bss_6BF8;
u8 lbl_1_bss_6BBC[60];
M609ShadowState lbl_1_bss_6B6C;
M609ParticleState lbl_1_bss_6B64;
M609Model lbl_1_bss_8BC[154];
M609Motion lbl_1_bss_70C[4];
M609Player lbl_1_bss_24C[4];
M609Control lbl_1_bss_30;
M609PlayerActors lbl_1_bss_4;
s16 lbl_1_bss_0[2];
