/* Boo'd Off the Stage module setup, player control, cameras and effects. */
#include "REL/m660dll/scene.h"
#include "REL/m660dll/camera_lighting.h"
#include "humath.h"
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "game/printfunc.h"
#include "dolphin/types.h"
#include "game/hsfex.h"
#include "REL/m660dll/player_slots.h"
#include "game/main.h"
#include "game/mg/actman.h"
#include "dolphin/gx.h"
#include "game/object.h"
#include "game/audio.h"
#include "msm_se.h"
#include "game/charman.h"
#include "game/gamemes.h"
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
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m660dll/particles.h"
#include "dolphin/os.h"
#include "datanum/charmot.h"
#include <string.h>

#define M660_EFFECT_MEM_GROUP 268435456

typedef struct M660DataNumbers {
    u32 dataIds[4];
} M660DataNumbers;

typedef struct M660ParticleValues {
    s16 effectValues[4];
} M660ParticleValues;

typedef struct M660ParticleHooks {
    HU3D_PARTICLE_HOOK updateHooks[4];
} M660ParticleHooks;

typedef struct M660EffectConfig_42E0 {
    s16 particleCount;
    s16 alpha;
    f32 opacityChange;
    f32 horizontalRadius;
    f32 spawnRadiusLimit;
    f32 spawnHeight;
    f32 baseParticleScale;
    s16 alternateAlpha;
    s16 unused1A;
    f32 forwardDistance;
    f32 sideDistance;
    f32 alternateRadiusLimit;
    f32 alternateParticleScale;
} M660EffectConfig_42E0;

typedef struct M660EffectConfigs_42E0 {
    M660EffectConfig_42E0 effects[3];
} M660EffectConfigs_42E0;

typedef struct M660VectorValues_4F64 {
    f32 yPositions[3];
} M660VectorValues_4F64;

typedef struct M660LayerValues_4F64 {
    s32 renderLayers[3];
} M660LayerValues_4F64;

typedef struct ParticleVectorValues_55DC {
    f32 variantValues[3];
} ParticleVectorValues_55DC;

typedef struct VoiceSoundIds_601C {
    s32 soundIds[14];
} VoiceSoundIds_601C;
extern int printf();
extern double sin(double);
extern double cos(double);
void fn_1_1C1C(s16 cameraIndex);
void fn_1_1C50(void);
void fn_1_1E24(void);
void fn_1_2090(void);
void fn_1_21A0(void);
void fn_1_24C4(void);
void fn_1_2B34(void);
s16 fn_1_2E80(f32 *value, f32 step, f32 start, f32 end);
MGTIMER *fn_1_2FA0(int type,int duration,int end,float x,float y);
void fn_1_3014(MGTIMER *timer);
void fn_1_3040(MGTIMER *timer);
void fn_1_3068(MGTIMER *timer);
void fn_1_30A0(s16 controller, s16 minimum, s16 maximum);
void fn_1_3338(s16 controllerIndex, s16 subjectIndex);
s16 fn_1_3AA4(s16 modelIndex, s16 lightId);
s32 fn_1_3BBC(HU3D_MODELID modelId, HU3D_LIGHTID lightId);
f32 fn_1_3C54(f32 range);
void fn_1_3CCC(void);
void fn_1_4004(Vec *origin);
void fn_1_42E0(M660Model *origin, s16 effectIndex);
void fn_1_4B38(M660Model *origin);
void fn_1_4F64(Vec *origin, s16 effectIndex);
void fn_1_55DC(s16 objectIndex, s16 variant);
s16 fn_1_57F8(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity,
              f32 initialScale, s16 state, M660ParticleParameters *parameters);
void fn_1_59F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_5E70(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_601C(M660Player *player, s16 direction);
void fn_1_64D0(void);
void fn_1_6614(void);
void fn_1_6678(void);
void fn_1_66C8(s16 streamSlot, s16 fadeFrames);
void fn_1_6774(s16 streamSelector);
void fn_1_67D4(s16 slot, s16 mode, Vec *position);

/* Module startup, sequence callbacks, player input, models and cameras. */

typedef struct M660PlayerLookup { s16 playerByGroup[2]; } M660PlayerLookup;
typedef struct M660PlayerPair { Vec playerVectors[2]; } M660PlayerPair;
typedef struct M660CameraPresetTable { f32 cameraSettings[4][13]; } M660CameraPresetTable;
typedef struct M660CameraIds { s16 cameraMasks[4]; } M660CameraIds;

extern s16 GwMgNightF;
extern HuVecF Center, CRot;
extern f32 CZoom;
void fn_1_176C(s16 cameraIndex);
void fn_1_1C1C(s16 cameraIndex);
void fn_1_2090(void);
void fn_1_A3E4(void);
void fn_1_64D0(void);
s32 fn_1_A9B0(void);
s32 fn_1_A9F0(void);
s32 fn_1_6F58(void);
s32 fn_1_D29C(void);
s32 fn_1_D2E8(void);
s32 fn_1_D328(void);
s32 fn_1_D57C(void);
s32 fn_1_D5A4(void);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

void fn_1_118(s16 mode, s16 frameNo);
void fn_1_26C(s16 mode, s16 frameNo);
void fn_1_298(s16 mode, s16 frameNo);
void fn_1_2C4(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_31C(s16 mode, s16 frameNo);
void fn_1_348(s16 mode, s16 frameNo);
void fn_1_374(s16 mode, s16 frameNo);
void fn_1_3A0(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_118, fn_1_26C, fn_1_298, fn_1_2C4, fn_1_2F0,
    fn_1_31C, fn_1_348, fn_1_374, fn_1_3A0
};

/* Runs module constructors and starts the minigame when the REL is loaded. */
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

/* Runs module destructors when the REL is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates the actor object and starts the minigame sequence during startup. */
void fn_1_A0(void) {
    lbl_1_bss_2678.objects = MgActorObjectSetup();
    omAddObjEx(lbl_1_bss_2678.objects, 32730, 0U, 0U, -1, omOutView);
    MgSeqCreate(&lbl_1_data_0);
    MgSeqModeDelaySet(2, 1000);
    MgSeqModeDelaySet(7, 1000);
}

/* Clears round state and initializes cameras, models and audio on sequence entry. */
void fn_1_118(s16 mode, s16 frameNo)
{
    s16 nightFlag;

    memset(&lbl_1_bss_2530, 0, 328);
    memset(&lbl_1_bss_24F4, 0, 60);
    memset(&lbl_1_bss_24A4, 0, 80);
    memset(&lbl_1_bss_2484, 0, 32);
    memset(lbl_1_bss_60C, 0, 7800);
    memset(lbl_1_bss_544, 0, 200);
    memset(lbl_1_bss_2DC, 0, 616);
    memset(&lbl_1_bss_1F4, 0, 232);
    memset(&lbl_1_bss_1E8, 0, 12);
    memset(lbl_1_bss_80, 0, 360);
    memset(&lbl_1_bss_38, 0, 72);
    memset(lbl_1_bss_0, 0, 56);
    nightFlag = GwMgNightF;
    lbl_1_bss_1F4.night = nightFlag;
    fn_1_176C(0);
    fn_1_2090();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_A3E4();
    fn_1_64D0();
    MgSeqModeNext();
}

void fn_1_26C(s16 mode, s16 frameNo)
{
    if (fn_1_A9B0() != 0) MgSeqModeNext();
}

void fn_1_298(s16 mode, s16 frameNo)
{
    if (fn_1_A9F0() != 0) MgSeqModeNext();
}

void fn_1_2C4(s16 mode, s16 frameNo)
{
    if (fn_1_6F58() != 0) MgSeqModeNext();
}

void fn_1_2F0(s16 mode, s16 frameNo)
{
    if (fn_1_D29C() != 0) MgSeqModeNext();
}

void fn_1_31C(s16 mode, s16 frameNo)
{
    if (fn_1_D2E8() != 0) MgSeqModeNext();
}

void fn_1_348(s16 mode, s16 frameNo)
{
    if (fn_1_D328() != 0) MgSeqModeNext();
}

void fn_1_374(s16 mode, s16 frameNo)
{
    if (fn_1_D57C() != 0) MgSeqModeNext();
}

void fn_1_3A0(s16 mode, s16 frameNo)
{
    if (fn_1_D5A4() != 0) MgSeqModeNext();
}

void fn_1_3CC(void)
{
}

/* Selects the first player in each team and initializes the two stage players. */
void fn_1_3D0(void)
{
    M660PlayerLookup playerLookup = {{-1, -1}};
    M660PlayerPair startingPositions = {{{-150.0f, 10.0f, 0.0f}, {150.0f, 10.0f, 0.0f}}};
    M660PlayerPair startingRotations = {{{0.0f, 180.0f, 0.0f}, {0.0f, 180.0f, 0.0f}}};
    s16 player = 0;
    s16 group = 0;
    for (; player < 4; player++) {
        group = GwPlayerConf[player].grpNo;

        if ((group == 0 || group == 1) &&
            playerLookup.playerByGroup[group] == -1) {
            playerLookup.playerByGroup[group] = player;
        }
    }

    for (player = 0; player < 2; player++) {
        lbl_1_bss_2DC[player].selectedPlayer =
            (s8)playerLookup.playerByGroup[player];
        lbl_1_bss_2DC[player].playerIndex = (s8)player;
        lbl_1_bss_2DC[player].character =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].charNo;
        lbl_1_bss_2DC[player].pad =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].padNo;
        lbl_1_bss_2DC[player].group =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].grpNo;
        lbl_1_bss_2DC[player].type =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].type;
        lbl_1_bss_2DC[player].difficulty =
            (s8)GwPlayerConf[playerLookup.playerByGroup[player]].comDif;
        lbl_1_bss_2DC[player].position =
            startingPositions.playerVectors[player];
        lbl_1_bss_2DC[player].rotation = startingRotations.playerVectors[player];
        lbl_1_bss_2DC[player].mode = -1;
    }
}

/* Samples controller input for the two stage players and all four controller slots. */
void fn_1_71C(void)
{
    s32 index;
    s32 stickResult;
    s32 deadZoneValue;

    index = 0;
    for (; index < 2; index++) {
        if ((&lbl_1_bss_2DC[index])->type != 0 ||
            (&lbl_1_bss_2DC[index])->inputDisabled != 0) {
            memset((&lbl_1_bss_2DC[index]), 0, 20);
        } else {
            (&lbl_1_bss_2DC[index])->input.buttons =
                HuPadBtn[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.pressedButtons =
                HuPadBtnDown[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.repeatedButtons =
                HuPadBtnRep[(&lbl_1_bss_2DC[index])->pad];
            /* Only the nonpositive X branch applies the dead zone; positive X and Y pass
             * through. */
            if (HuPadStkX[(&lbl_1_bss_2DC[index])->pad] > 0) {
                stickResult = HuPadStkX[(&lbl_1_bss_2DC[index])->pad];
            } else {
                if ((s8)HuPadStkX[(&lbl_1_bss_2DC[index])->pad] * -1 < 10) {
                    deadZoneValue = 0;
                } else {
                    deadZoneValue = HuPadStkX[(&lbl_1_bss_2DC[index])->pad];
                }
                stickResult = deadZoneValue;
            }
            (&lbl_1_bss_2DC[index])->input.stickX = stickResult;
            (&lbl_1_bss_2DC[index])->input.stickY =
                (s8)HuPadStkY[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.substickX =
                (s8)HuPadSubStkX[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.substickY =
                (s8)HuPadSubStkY[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.triggerL = HuPadTrigL[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.triggerR = HuPadTrigR[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.directionalPad = HuPadDStk[(&lbl_1_bss_2DC[index])->pad];
            (&lbl_1_bss_2DC[index])->input.repeatedDirectionalPad =
                HuPadDStkRep[(&lbl_1_bss_2DC[index])->pad];
        }
    }

    index = 0;
    for (; index < 4; index++) {
        lbl_1_bss_1F4.samples[index].buttons = HuPadBtn[index];
        lbl_1_bss_1F4.samples[index].pressedButtons = HuPadBtnDown[index];
        lbl_1_bss_1F4.samples[index].repeatedButtons = HuPadBtnRep[index];
        lbl_1_bss_1F4.samples[index].stickX = (s8)HuPadStkX[index];
        lbl_1_bss_1F4.samples[index].stickY = (s8)HuPadStkY[index];
        lbl_1_bss_1F4.samples[index].substickX = (s8)HuPadSubStkX[index];
        lbl_1_bss_1F4.samples[index].substickY = (s8)HuPadSubStkY[index];
        lbl_1_bss_1F4.samples[index].triggerL = HuPadTrigL[index];
        lbl_1_bss_1F4.samples[index].triggerR = HuPadTrigR[index];
        lbl_1_bss_1F4.samples[index].directionalPad = HuPadDStk[index];
        lbl_1_bss_1F4.samples[index].repeatedDirectionalPad = HuPadDStkRep[index];
    }
}

/* Applies stored position and rotation state to stage models during object updates. */
void fn_1_CDC(void)
{
    s32 modelIndex = 2;
    while (modelIndex < 75) {
        Hu3DModelPosSet(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].position.x,
            lbl_1_bss_60C[modelIndex].position.y,
            lbl_1_bss_60C[modelIndex].position.z);
        Hu3DModelRotSet(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].rotation.x,
            lbl_1_bss_60C[modelIndex].rotation.y,
            lbl_1_bss_60C[modelIndex].rotation.z);
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_2DC[modelIndex].state == 0 ||
            lbl_1_bss_2DC[modelIndex].state == 2) {
            Hu3DModelPosSet(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].position.x,
                lbl_1_bss_2DC[modelIndex].position.y,
                lbl_1_bss_2DC[modelIndex].position.z);
            Hu3DModelRotSet(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].rotation.x,
                lbl_1_bss_2DC[modelIndex].rotation.y,
                lbl_1_bss_2DC[modelIndex].rotation.z);
        }
        modelIndex++;
    }
}

/* Loads the seven character motions used by each stage player during setup. */
void fn_1_EB8(void)
{
    u32 motionData[7] = {
        CHARMOT_HSF_c000m1_300,
        CHARMOT_HSF_c000m1_302,
        CHARMOT_HSF_c000m1_306,
        CHARMOT_HSF_c000m1_307,
        CHARMOT_HSF_c000m1_323,
        CHARMOT_HSF_c000m1_324,
        DATANUM(DATA_mariomot, 34),
    };
    s16 playerIndex;
    s16 motionIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        motionIndex = 0;
        while (motionIndex < 7) {
            lbl_1_bss_544[playerIndex].motionIds[motionIndex] =
                CharMotionCreate(lbl_1_bss_2DC[playerIndex].character,
                                 motionData[motionIndex]);
            motionIndex++;
        }
        playerIndex++;
    }
}

/* Applies motion settings and handles motion shifts for both player models each update. */
void fn_1_FA8(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 2) {
        if (lbl_1_bss_544[playerIndex].mode != lbl_1_bss_544[playerIndex].previousMode) {
            Hu3DMotionSet(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].motionIds[lbl_1_bss_544[playerIndex].mode]);
            lbl_1_bss_544[playerIndex].previousMode = lbl_1_bss_544[playerIndex].mode;
            lbl_1_bss_544[playerIndex].maximumTime = Hu3DMotionMaxTimeGet(
                lbl_1_bss_60C[playerIndex].modelId);
        }
        if (lbl_1_bss_544[playerIndex].speed != lbl_1_bss_544[playerIndex].previousSpeed) {
            Hu3DMotionSpeedSet(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].speed);
            lbl_1_bss_544[playerIndex].previousSpeed = lbl_1_bss_544[playerIndex].speed;
        }
        if (lbl_1_bss_544[playerIndex].attributes !=
            lbl_1_bss_544[playerIndex].previousAttributes) {
            Hu3DModelAttrSet(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].attributes);
            lbl_1_bss_544[playerIndex].previousAttributes =
                lbl_1_bss_544[playerIndex].attributes;
        }
        if (lbl_1_bss_544[playerIndex].attributesToReset != 0) {
            Hu3DModelAttrReset(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].attributesToReset);
            lbl_1_bss_544[playerIndex].attributesToReset =
                ~lbl_1_bss_544[playerIndex].attributesToReset;
            lbl_1_bss_544[playerIndex].previousAttributes =
                lbl_1_bss_544[playerIndex].attributes =
                    lbl_1_bss_544[playerIndex].attributes &
                    lbl_1_bss_544[playerIndex].attributesToReset;
            lbl_1_bss_544[playerIndex].attributesToReset = 0;
        }
        if (lbl_1_bss_544[playerIndex].time != lbl_1_bss_544[playerIndex].previousTime) {
            Hu3DMotionTimeSet(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].time);
        }

        switch (lbl_1_bss_544[playerIndex].shiftState) {
        case 0:
            break;
        case 1:
            Hu3DMotionShiftSet(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].motionIds[lbl_1_bss_544[playerIndex].shiftMotion],
                lbl_1_bss_544[playerIndex].shiftStart, lbl_1_bss_544[playerIndex].shiftEnd,
                lbl_1_bss_544[playerIndex].shiftFlags);
            lbl_1_bss_544[playerIndex].shiftMaximumTime = Hu3DMotionShiftMaxTimeGet(
                lbl_1_bss_60C[playerIndex].modelId);
            lbl_1_bss_544[playerIndex].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_544[playerIndex].shiftTime = Hu3DMotionShiftTimeGet(
                lbl_1_bss_60C[playerIndex].modelId);
            if ((lbl_1_bss_544[playerIndex].shiftFlags & HU3D_MOTATTR_LOOP) == 0 &&
                lbl_1_bss_544[playerIndex].shiftTime >=
                    lbl_1_bss_544[playerIndex].shiftEnd - 1.0f) {
                lbl_1_bss_544[playerIndex].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_544[playerIndex].shiftState = 0;
            break;
        }

        lbl_1_bss_544[playerIndex].time = Hu3DMotionTimeGet(
            lbl_1_bss_60C[playerIndex].modelId);
        lbl_1_bss_544[playerIndex].previousTime = lbl_1_bss_544[playerIndex].time;
        lbl_1_bss_544[playerIndex].motionEnded = Hu3DMotionEndCheck(
            lbl_1_bss_60C[playerIndex].modelId);
        if (lbl_1_bss_544[playerIndex].motionEnded != 0 &&
            lbl_1_bss_544[playerIndex].resetRequested != 0) {
            lbl_1_bss_544[playerIndex].motionEnded = 0;
            lbl_1_bss_544[playerIndex].resetRequested = 0;
            lbl_1_bss_544[playerIndex].mode = lbl_1_bss_544[playerIndex].resetMode;
            lbl_1_bss_544[playerIndex].time = lbl_1_bss_544[playerIndex].transition;
            lbl_1_bss_544[playerIndex].attributes =
                lbl_1_bss_544[playerIndex].resetAttributes;
            lbl_1_bss_544[playerIndex].attributesToReset =
                lbl_1_bss_544[playerIndex].resetAttributesToClear;
        }
        playerIndex++;
    }
}

/* Applies a camera preset and creates its viewport and perspective during setup. */
void fn_1_176C(s16 cameraIndex)
{
    M660CameraPresetTable presets = { { { 0.0f, 101.0f, 61.0f, -78.0f, 0.0f, 0.0f, 1039.1f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 50.209999f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f } } };
    M660CameraIds cameraIds = {{1, 2, 4, 8}};

    lbl_1_bss_2530.records[cameraIndex].cameraIndex = cameraIndex;
    lbl_1_bss_2530.records[cameraIndex].position.x = presets.cameraSettings[cameraIndex][0];
    lbl_1_bss_2530.records[cameraIndex].position.y = presets.cameraSettings[cameraIndex][1];
    lbl_1_bss_2530.records[cameraIndex].position.z = presets.cameraSettings[cameraIndex][2];
    lbl_1_bss_2530.records[cameraIndex].rotation.x = presets.cameraSettings[cameraIndex][3];
    lbl_1_bss_2530.records[cameraIndex].rotation.y = presets.cameraSettings[cameraIndex][4];
    lbl_1_bss_2530.records[cameraIndex].rotation.z = presets.cameraSettings[cameraIndex][5];
    lbl_1_bss_2530.records[cameraIndex].zoom = presets.cameraSettings[cameraIndex][6];
    lbl_1_bss_2530.records[cameraIndex].viewport[0] = presets.cameraSettings[cameraIndex][7];
    lbl_1_bss_2530.records[cameraIndex].viewport[1] = presets.cameraSettings[cameraIndex][8];
    lbl_1_bss_2530.records[cameraIndex].viewport[2] = presets.cameraSettings[cameraIndex][9];
    lbl_1_bss_2530.records[cameraIndex].viewport[3] = presets.cameraSettings[cameraIndex][10];
    lbl_1_bss_2530.records[cameraIndex].viewport[4] = presets.cameraSettings[cameraIndex][11];
    lbl_1_bss_2530.records[cameraIndex].viewport[5] = presets.cameraSettings[cameraIndex][12];

    Hu3DCameraCreate(cameraIds.cameraMasks[cameraIndex]);
    Hu3DCameraViewportSet(cameraIds.cameraMasks[cameraIndex],
                          lbl_1_bss_2530.records[cameraIndex].viewport[1],
                          lbl_1_bss_2530.records[cameraIndex].viewport[2],
                          lbl_1_bss_2530.records[cameraIndex].viewport[3],
                          lbl_1_bss_2530.records[cameraIndex].viewport[4],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds.cameraMasks[cameraIndex],
                             lbl_1_bss_2530.records[cameraIndex].viewport[5],
                             20.0f, 40000.0f,
                             lbl_1_bss_2530.records[cameraIndex].viewport[0]);
    Center.x = presets.cameraSettings[cameraIndex][0];
    Center.y = presets.cameraSettings[cameraIndex][1];
    Center.z = presets.cameraSettings[cameraIndex][2];
    CRot.x = presets.cameraSettings[cameraIndex][3];
    CRot.y = presets.cameraSettings[cameraIndex][4];
    CRot.z = presets.cameraSettings[cameraIndex][5];
    CZoom = presets.cameraSettings[cameraIndex][6];
    fn_1_1C1C(cameraIndex);
}

u16 lbl_1_bss_2680[4];
M660ManagerState lbl_1_bss_2678;
M660CameraState lbl_1_bss_2530;
M660LightState lbl_1_bss_24F4;
M660ShadowCamera lbl_1_bss_24A4;
M660ParticleModuleData lbl_1_bss_2484;
M660Model lbl_1_bss_60C[75];
M660Motion lbl_1_bss_544[2];
M660Player lbl_1_bss_2DC[2];
M660StageInput lbl_1_bss_1F4;
M660PlayerSlots lbl_1_bss_1E8;
M660Slot lbl_1_bss_80[9];
M660SceneLight lbl_1_bss_38;
M660StageConnection lbl_1_bss_0[14];

void fn_1_1C1C(s16 cameraIndex) {
    M660HalfwordQuartet values = {1, 2, 4, 8};

}

/* Creates the directional stage light during scene initialization. */
void fn_1_1C50(void) {
    Hu3DLighInit();

    lbl_1_bss_24F4.position.x = 0.0f;
    lbl_1_bss_24F4.position.y = 0.0f;
    lbl_1_bss_24F4.position.z = 0.0f;
    lbl_1_bss_24F4.aim.x = 0.0f;
    lbl_1_bss_24F4.aim.y = 0.0f;
    lbl_1_bss_24F4.aim.z = 0.0f;
    lbl_1_bss_24F4.orientation.x = -70.0f;
    lbl_1_bss_24F4.orientation.y = 0.0f;
    lbl_1_bss_24F4.orientation.z = 0.0f;
    lbl_1_bss_24F4.range = 1000.0f;

    lbl_1_bss_24F4.color.r = 255;
    lbl_1_bss_24F4.color.g = 255;
    lbl_1_bss_24F4.color.b = 255;
    lbl_1_bss_24F4.color.a = 255;

    PSVECSubtract(&lbl_1_bss_24F4.aim, &lbl_1_bss_24F4.position,
                  &lbl_1_bss_24F4.direction);
    lbl_1_bss_24F4.id = Hu3DGLightCreateV(
        &lbl_1_bss_24F4.position, &lbl_1_bss_24F4.direction,
        &lbl_1_bss_24F4.color);
    fn_1_1E24();
    Hu3DGLightStaticSet(lbl_1_bss_24F4.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_24F4.id);
}

/* Repositions the stage light from its target, range and orientation settings. */
void fn_1_1E24(void) {
    lbl_1_bss_24F4.position.x =
        lbl_1_bss_24F4.aim.x +
        lbl_1_bss_24F4.range *
            (cos(3.141592653589793 * lbl_1_bss_24F4.orientation.x /
                 180.0) *
             sin(3.141592653589793 * lbl_1_bss_24F4.orientation.y /
                 180.0));
    lbl_1_bss_24F4.position.y =
        lbl_1_bss_24F4.aim.y +
        lbl_1_bss_24F4.range *
            -sin(3.141592653589793 * lbl_1_bss_24F4.orientation.x /
                 180.0);
    lbl_1_bss_24F4.position.z =
        lbl_1_bss_24F4.aim.z +
        lbl_1_bss_24F4.range *
            (cos(3.141592653589793 * lbl_1_bss_24F4.orientation.x /
                 180.0) *
             cos(3.141592653589793 * lbl_1_bss_24F4.orientation.y /
                 180.0));
    PSVECSubtract(&lbl_1_bss_24F4.aim, &lbl_1_bss_24F4.position,
                  &lbl_1_bss_24F4.direction);
    PSVECNormalize(&lbl_1_bss_24F4.direction, &lbl_1_bss_24F4.direction);
    Hu3DGLightPosSetV(lbl_1_bss_24F4.id, &lbl_1_bss_24F4.position,
                      &lbl_1_bss_24F4.direction);
    Hu3DGLightColorSet(lbl_1_bss_24F4.id, lbl_1_bss_24F4.color.r,
                       lbl_1_bss_24F4.color.g, lbl_1_bss_24F4.color.b,
                       lbl_1_bss_24F4.color.a);
}

/* Initializes the scene shadow camera and its starting orbit during setup. */
void fn_1_2090(void)
{
    Hu3DShadowCreate(5.0f, 16000.0f, 21000.0f);
    Hu3DShadowSizeSet(480);
    lbl_1_bss_24A4.azimuth = -90.0f;
    lbl_1_bss_24A4.elevation = 0.0f;
    lbl_1_bss_24A4.unknown14 = 0.0f;
    lbl_1_bss_24A4.target.x = 0.0f;
    lbl_1_bss_24A4.target.y = -100.0f;
    lbl_1_bss_24A4.target.z = 0.0f;
    lbl_1_bss_24A4.shadowTransparency = 0.2f;
    lbl_1_bss_24A4.orbitDistance = 21000.0f;
    fn_1_21A0();
}

/* Recalculates the shadow camera vectors and applies them to camera zero. */
void fn_1_21A0(void)
{
    lbl_1_bss_24A4.cameraPosition.x = (f32)((f64)lbl_1_bss_24A4.target.x +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         (cos((3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0) *
          sin((3.141592653589793 * (f64)lbl_1_bss_24A4.elevation) / 180.0))));
    lbl_1_bss_24A4.cameraPosition.y = (f32)((f64)lbl_1_bss_24A4.target.y +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         -sin((3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0)));
    lbl_1_bss_24A4.cameraPosition.z = (f32)((f64)lbl_1_bss_24A4.target.z +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         (cos((3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0) *
          cos((3.141592653589793 * (f64)lbl_1_bss_24A4.elevation) / 180.0))));

    lbl_1_bss_24A4.cameraUp.x = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0) *
        sin((3.141592653589793 * (f64)lbl_1_bss_24A4.elevation) / 180.0));
    lbl_1_bss_24A4.cameraUp.y = (f32)cos(
        (3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0);
    lbl_1_bss_24A4.cameraUp.z = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_24A4.azimuth) / 180.0) *
        cos((3.141592653589793 * (f64)lbl_1_bss_24A4.elevation) / 180.0));

    Hu3DShadowTPLvlSet(lbl_1_bss_24A4.shadowTransparency);
    Hu3DShadowPosSet(&lbl_1_bss_24A4.cameraPosition, &lbl_1_bss_24A4.cameraUp,
                     &lbl_1_bss_24A4.target);
}

/* Synchronizes player and prop transforms and motions, then refreshes the shadow camera each
 * frame. */
void fn_1_24C4(void)
{
    Vec playerPosition;
    s16 playerIndex;
    s32 modelIndex;

    fontcolor = 15;
    playerIndex = 0;
    while (playerIndex < 2) {
        playerPosition.x = lbl_1_bss_2DC[playerIndex].position.x;
        playerPosition.y = lbl_1_bss_2DC[playerIndex].position.y;
        playerPosition.z = lbl_1_bss_2DC[playerIndex].position.z;
        MgActorPosSetRaw(lbl_1_bss_1E8.players[playerIndex]->actor,
                         &playerPosition);
        lbl_1_bss_1E8.players[playerIndex]->actor->rotY =
            lbl_1_bss_2DC[playerIndex].rotation.y;
        lbl_1_bss_60C[playerIndex].position =
            lbl_1_bss_2DC[playerIndex].position;
        lbl_1_bss_60C[playerIndex].rotation =
            lbl_1_bss_2DC[playerIndex].rotation;
        playerIndex++;
    }

    modelIndex = 2;
    while (modelIndex < 75) {
        Hu3DModelPosSet(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].position.x,
            lbl_1_bss_60C[modelIndex].position.y,
            lbl_1_bss_60C[modelIndex].position.z);
        Hu3DModelRotSet(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].rotation.x,
            lbl_1_bss_60C[modelIndex].rotation.y,
            lbl_1_bss_60C[modelIndex].rotation.z);
        modelIndex++;
    }

    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_2DC[modelIndex].state == 0 ||
            lbl_1_bss_2DC[modelIndex].state == 2) {
            Hu3DModelPosSet(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].position.x,
                lbl_1_bss_2DC[modelIndex].position.y,
                lbl_1_bss_2DC[modelIndex].position.z);
            Hu3DModelRotSet(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].rotation.x,
                lbl_1_bss_2DC[modelIndex].rotation.y,
                lbl_1_bss_2DC[modelIndex].rotation.z);
        }
        modelIndex++;
    }

    fn_1_FA8();
    fn_1_1C1C(0);
    fn_1_21A0();
}

/* Runs actor movement, retaining player positions and collision partners per frame. */
void fn_1_2B34(void) {
    Point3d position;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_2DC[playerIndex].state != 0) {
            if ((s8) lbl_1_bss_2DC[playerIndex].inputDisabled != 0) {
                MgPlayerPadSet(lbl_1_bss_1E8.players[playerIndex], 0, 0, 0, 0);
            } else if ((s8) lbl_1_bss_2DC[playerIndex].type != 0) {
                MgPlayerPadSet(lbl_1_bss_1E8.players[playerIndex],
                               lbl_1_bss_2DC[playerIndex].stickXWork,
                               lbl_1_bss_2DC[playerIndex].stickYWork,
                               lbl_1_bss_2DC[playerIndex].pressedButtonsWork,
                               lbl_1_bss_2DC[playerIndex].buttonsWork);
            }
            MgActorPosGet(lbl_1_bss_1E8.players[playerIndex]->actor, &position);
            lbl_1_bss_2DC[playerIndex].previousPosition.x = position.x;
            lbl_1_bss_2DC[playerIndex].previousPosition.y = position.y;
            lbl_1_bss_2DC[playerIndex].previousPosition.z = position.z;
            lbl_1_bss_2DC[playerIndex].previousMesh = lbl_1_bss_2DC[playerIndex].storedMesh;
            lbl_1_bss_2DC[playerIndex].storedMesh = -1;
            if ((lbl_1_bss_2DC[playerIndex].previousPartner !=
                 lbl_1_bss_2DC[playerIndex].partnerIndex) ||
                (lbl_1_bss_2DC[playerIndex].partnerIndex == -1)) {
                lbl_1_bss_2DC[playerIndex].collisionCount = 0;
            }
            lbl_1_bss_2DC[playerIndex].previousPartner = lbl_1_bss_2DC[playerIndex].partnerIndex;
            lbl_1_bss_2DC[playerIndex].partnerIndex = -1;
        }
        playerIndex += 1;
    }
    MgActorExec();
    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_2DC[playerIndex].state != 0) {
            MgActorPosGet(lbl_1_bss_1E8.players[playerIndex]->actor, &position);
            lbl_1_bss_2DC[playerIndex].position.x = position.x;
            lbl_1_bss_2DC[playerIndex].position.y = position.y;
            lbl_1_bss_2DC[playerIndex].position.z = position.z;
            lbl_1_bss_2DC[playerIndex].rotation.y =
                (f32) lbl_1_bss_1E8.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

/* Advances a value toward its endpoint and returns remaining frames, or zero at the end. */
s16 fn_1_2E80(f32 *value, f32 step, f32 start, f32 end)
{
    f32 startDistance;
    f32 currentDistance;
    s16 frames = 0;
    f32 increment = (end - start) / step;

    *value += increment;
    if (increment) {
        frames = (s16)((end - *value) / increment);
    }
    frames++;
    if (start - end > 0.0f) {
        startDistance = start - end;
    } else {
        startDistance = start - -1.0f * end;
    }
    increment = startDistance;
    if (start - *value > 0.0f) {
        currentDistance = start - *value;
    } else {
        currentDistance = start - -1.0f * *value;
    }
    start = currentDistance;
    if (increment <= start) {
        *value = end;
        frames = 0;
    }
    return frames;
}

/* Creates and positions a timer, converting its duration and end time from seconds to frames. */
MGTIMER *fn_1_2FA0(int type,int duration,int end,float x,float y) {
    MGTIMER *timer=MgTimerCreate(type);
    MgTimerParamSet(timer,duration*60,end*60,0);
    MgTimerPosSet(timer,x,y);
    return timer;
}

void fn_1_3014(MGTIMER *timer) {
    MgTimerModeOnSet(timer, 1);
}

/* Calls the timer completion check and discards its result. */
void fn_1_3040(MGTIMER *timer) {
    MgTimerDoneCheck(timer);
}

/* Hides the timer display, then kills the timer and releases its resources. */
void fn_1_3068(MGTIMER *timer) {
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

/* Changes the selected object with controller input and prints selected transforms. */
void fn_1_30A0(s16 controller, s16 minimum, s16 maximum)
{
    s16 player;

    if (HuPadTrigL[controller] > 50) {
        if (HuPadBtnDown[controller] & PAD_BUTTON_X) {
            lbl_1_bss_1F4.selectedObject++;
        }
        if (HuPadBtnDown[controller] & PAD_BUTTON_Y) {
            lbl_1_bss_1F4.selectedObject--;
        }
    }

    if (lbl_1_bss_1F4.selectedObject < minimum) {
        lbl_1_bss_1F4.selectedObject = maximum;
    }
    if (lbl_1_bss_1F4.selectedObject > maximum) {
        lbl_1_bss_1F4.selectedObject = minimum;
    }
    fn_1_3338(controller, lbl_1_bss_1F4.selectedObject);

    if ((HuPadBtnDown[controller] & PAD_BUTTON_X) &&
        (HuPadBtnDown[controller] & PAD_BUTTON_Y)) {
        for (player = minimum; player <= maximum; player++) {
            if (player >= 0 && player <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n", player,
                       lbl_1_bss_2DC[player].position.x,
                       lbl_1_bss_2DC[player].position.y,
                       lbl_1_bss_2DC[player].position.z,
                       lbl_1_bss_2DC[player].rotation.y);
            } else {
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n", player,
                       lbl_1_bss_60C[player].position.x,
                       lbl_1_bss_60C[player].position.y,
                       lbl_1_bss_60C[player].position.z,
                       lbl_1_bss_60C[player].rotation.y);
            }
        }
    }
}

/* Moves or rotates the selected player or prop from controller input. */
void fn_1_3338(s16 controllerIndex, s16 subjectIndex)
{
    f32 movementStep = 5.0f;
    s16 textX = 25;
    s16 textY = 430;

    if (subjectIndex >= 0 && subjectIndex <= 1) {
        if ((HuPadBtnRep[controllerIndex] & PAD_BUTTON_X) != 0) {
            if (HuPadStkX[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.x += movementStep;
            }
            if (HuPadStkX[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.x -= movementStep;
            }
            if (HuPadStkY[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.z -= movementStep;
            }
            if (HuPadStkY[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.z += movementStep;
            }
        } else if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_X) != 0) {
            if (HuPadStkX[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.x += movementStep;
            }
            if (HuPadStkX[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.x -= movementStep;
            }
            if (HuPadStkY[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.z -= movementStep;
            }
            if (HuPadStkY[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.z += movementStep;
            }
        }
        if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_A) != 0) {
            lbl_1_bss_2DC[subjectIndex].rotation.y += 90.0f;
        }
        if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_B) != 0) {
            lbl_1_bss_2DC[subjectIndex].rotation.y -= 90.0f;
        }
        print8(textX, textY, 1.5f,
               "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               subjectIndex,
               lbl_1_bss_2DC[subjectIndex].position.x,
               lbl_1_bss_2DC[subjectIndex].position.y,
               lbl_1_bss_2DC[subjectIndex].position.z,
               lbl_1_bss_2DC[subjectIndex].rotation.y);
        return;
    }

    if ((HuPadBtnRep[controllerIndex] & PAD_BUTTON_X) != 0) {
        if (HuPadStkX[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.x += movementStep;
        }
        if (HuPadStkX[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.x -= movementStep;
        }
        if (HuPadStkY[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.z -= movementStep;
        }
        if (HuPadStkY[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.z += movementStep;
        }
    } else if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_X) != 0) {
        if (HuPadStkX[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.x += movementStep;
        }
        if (HuPadStkX[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.x -= movementStep;
        }
        if (HuPadStkY[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.z -= movementStep;
        }
        if (HuPadStkY[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.z += movementStep;
        }
    }
    if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_A) != 0) {
        lbl_1_bss_60C[subjectIndex].rotation.y += 90.0f;
    }
    if ((HuPadBtnDown[controllerIndex] & PAD_BUTTON_B) != 0) {
        lbl_1_bss_60C[subjectIndex].rotation.y -= 90.0f;
    }
    print8(textX, textY, 1.5f,
           "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
           subjectIndex,
           lbl_1_bss_60C[subjectIndex].position.x,
           lbl_1_bss_60C[subjectIndex].position.y,
           lbl_1_bss_60C[subjectIndex].position.z,
           lbl_1_bss_60C[subjectIndex].rotation.y);
}

/* Adds a local light to a model unless it is already attached or the slots are full. */
s16 fn_1_3AA4(s16 modelIndex, s16 lightId)
{
    HU3D_MODEL *model;
    s16 attachResult;
    s16 lightSlot;

    attachResult = 0;
    model = &Hu3DData[modelIndex];
    lightSlot = 0;
    while (lightSlot < HU3D_MODEL_LLIGHT_MAX) {
        if (model->LLightId[lightSlot] == lightId) {
            attachResult = -1;
            break;
        }
        lightSlot++;
    }

    if (attachResult != -1) {
        lightSlot = 0;
        while (lightSlot < HU3D_MODEL_LLIGHT_MAX) {
            if (model->LLightId[lightSlot] == -1) {
                model->LLightId[lightSlot] = lightId;
                model->attr |= HU3D_ATTR_LLIGHT;
                attachResult = 1;
                break;
            }
            lightSlot++;
        }
    }

    if (attachResult == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", modelIndex, lightId);
        attachResult = -1;
    }

    return attachResult;
}

/* Removes a light ID from the model's local-light list, returning 1 when it is present. */
s32 fn_1_3BBC(HU3D_MODELID modelId, HU3D_LIGHTID lightId) {
    HU3D_MODEL *model;
    s32 removeResult;
    s16 lightSlot;

    removeResult = -1;
    model = &Hu3DData[modelId];
    lightSlot = 0;
    while (lightSlot < HU3D_MODEL_LLIGHT_MAX) {
        if (model->LLightId[lightSlot] == lightId) {
            model->LLightId[lightSlot] = -1;
            removeResult = 1;
            break;
        }
        lightSlot++;
    }
    return removeResult;
}

/* Returns a random integer below the positive range as a float for effect choices. */
f32 fn_1_3C54(f32 range)
{
    s32 randomValue;

    randomValue = (s32)range;
    randomValue = frandmod(randomValue);
    range = (f32)randomValue;
    return range;
}

/* Loads particle resources and initializes the four particle effect models at setup. */
void fn_1_3CCC(void)
{
    M660DataNumbers dataNumbers = { { DATANUM(DATA_m660, 36), DATANUM(DATA_m660, 37),
                                      DATANUM(DATA_m660, 37), DATANUM(DATA_m660, 37) } };
    M660ParticleValues maximumParticles = {{100, 500, 100, 1}};
    M660ParticleValues layers = {{0, 6, 1, 5}};
    M660ParticleValues cameras = {{1, 1, 1, 1}};
    M660ParticleValues blendModes = {{1, 0, 0, 0}};
    M660ParticleHooks hooks = {{fn_1_59F8, fn_1_59F8, fn_1_59F8, fn_1_5E70}};
    s16 effectIndex = 0;

    while (effectIndex < 4) {
        ANIMDATA *animation;
        M660ParticleContext *context;
        M660ParticleEntry *entry;
        s16 particleIndex;

        lbl_1_bss_2484.maximumCounts[effectIndex] =
            maximumParticles.effectValues[effectIndex];
        animation = HuSprAnimRead(HuDataSelHeapReadNum(
            dataNumbers.dataIds[effectIndex], M660_EFFECT_MEM_GROUP, HEAP_MODEL));
        lbl_1_bss_2484.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, maximumParticles.effectValues[effectIndex] * 60,
            M660_EFFECT_MEM_GROUP);
        lbl_1_bss_2484.modelIds[effectIndex] = Hu3DParticleCreate(
            animation, maximumParticles.effectValues[effectIndex]);
        Hu3DModelLayerSet(lbl_1_bss_2484.modelIds[effectIndex],
                          layers.effectValues[effectIndex]);
        Hu3DParticleHookSet(lbl_1_bss_2484.modelIds[effectIndex],
                            hooks.updateHooks[effectIndex]);
        Hu3DModelCameraSet(lbl_1_bss_2484.modelIds[effectIndex],
                           cameras.effectValues[effectIndex]);
        Hu3DParticleBlendModeSet(lbl_1_bss_2484.modelIds[effectIndex],
                                  (u8)blendModes.effectValues[effectIndex]);

        context = Hu3DData[lbl_1_bss_2484.modelIds[effectIndex]].hookData;
        context->lastEntry = 0;
        context->parameters = lbl_1_bss_2484.allocation[effectIndex];
        context->count = 0;
        particleIndex = 0;
        entry = context->entries;
        while (particleIndex < context->entryCount) {
            entry->height = 0.0f;
            entry->state = 0;
            particleIndex++;
            entry++;
        }
        effectIndex++;
    }
}

/* Emits a small randomized particle effect at the supplied position. */
void fn_1_4004(Vec *origin)
{
    Vec velocity;
    Vec position;
    M660ParticleParameters parameters;
    s32 randomLimit;
    s32 randomHeightLimit;
    f32 randomValue;
    f32 heightRandom;
    f32 initialScale;
    f32 centeredRandomValue;
    f32 horizontalVelocity;
    f32 baseHeight;
    f32 verticalVelocity;

    parameters.value00 = 0;
    parameters.color.red = 255;
    parameters.color.green = 255;
    parameters.color.blue = 255;
    parameters.color.alpha = 255;
    parameters.finalColor[0] = 0;
    parameters.finalColor[1] = 0;
    parameters.finalColor[2] = 0;
    parameters.finalColor[3] = 0;
    parameters.motionValueDelta = 0.0f;
    parameters.opacityChange = 0.01f;
    parameters.colorChangeRate = 0.01f;
    parameters.velocityYAcceleration = 0.0f;

    position.x = origin->x;
    position.y = origin->y;
    position.z = origin->z;
    initialScale = 30.0f;
    parameters.height = -0.1f;
    parameters.heightScale = 1.2f;
    parameters.velocityScale[0] = 1.05f;
    parameters.velocityScale[1] = 0.95f;
    parameters.velocityScale[2] = 1.0f;
    randomValue = 3.0f;
    randomLimit = (s32)randomValue;
    randomLimit = frandmod(randomLimit);
    randomValue = (f32)randomLimit;
    centeredRandomValue = randomValue;
    horizontalVelocity = centeredRandomValue;
    velocity.x = 0.4f *
                 (horizontalVelocity - 1.0f);
    heightRandom = 4.0f;
    randomHeightLimit = (s32)heightRandom;
    randomHeightLimit = frandmod(randomHeightLimit);
    heightRandom = (f32)randomHeightLimit;
    baseHeight = heightRandom;
    verticalVelocity = baseHeight;
    velocity.y = 3.0f + verticalVelocity;
    velocity.z = 0.0f;

    fn_1_57F8(0, -1, position, velocity, initialScale, 20, &parameters);
}

/* Emits randomized particles for the selected effect around the model position. The origin draw
* is overwritten with zero, so origin 0 is always used while the draw still advances the RNG. */
void fn_1_42E0(M660Model *origin, s16 effectIndex)
{
    M660EffectConfigs_42E0 configs = {{
    {3, 160, -8.0f, 0.0f, 80.0f, 450.0f, 190.0f, 128, 0, 30.0f, 140.0f, 20.0f, 100.0f},
    {3, 160, -5.0f, 20.0f, 120.0f, 450.0f, 400.0f, 128, 0, 70.0f, 280.0f, 30.0f, 200.0f},
    {5, 160, -4.0f, 40.0f, 220.0f, 600.0f, 600.0f, 128, 0, 70.0f, 500.0f, 40.0f, 200.0f}
    }};
    M660ParticleParameters parameters;
    Vec spawnOrigins[3];
    Vec velocity;
    Vec position;
    f32 angle;
    f32 radius;
    f32 particleScale;
    f32 alpha;
    s16 particleIndex;
    s16 selectedOrigin;
    s16 lifetime;

    spawnOrigins[0].x = (f32)((f64)origin->position.x -
         ((f64)configs.effects[effectIndex].horizontalRadius *
         sin((3.141592653589793 * (origin->rotation.y - 90.0f)) / 180.0)));
    spawnOrigins[0].z = (f32)((f64)origin->position.z -
         ((f64)configs.effects[effectIndex].horizontalRadius *
         cos((3.141592653589793 * (origin->rotation.y - 90.0f)) / 180.0)));
    position.x = (f32)((f64)origin->position.x -
         ((f64)configs.effects[effectIndex].forwardDistance *
         sin((3.141592653589793 * (origin->rotation.y - 90.0f)) / 180.0)));
    position.z = (f32)((f64)origin->position.z -
         ((f64)configs.effects[effectIndex].forwardDistance *
         cos((3.141592653589793 * (origin->rotation.y - 90.0f)) / 180.0)));
    spawnOrigins[1].x = (f32)((f64)position.x +
         ((f64)configs.effects[effectIndex].sideDistance *
         sin((3.141592653589793 * origin->rotation.y) / 180.0)));
    spawnOrigins[1].z = (f32)((f64)position.z +
         ((f64)configs.effects[effectIndex].sideDistance *
         cos((3.141592653589793 * origin->rotation.y) / 180.0)));
    spawnOrigins[2].x = (f32)((f64)position.x -
         ((f64)configs.effects[effectIndex].sideDistance *
         sin((3.141592653589793 * origin->rotation.y) / 180.0)));
    spawnOrigins[2].z = (f32)((f64)position.z -
         ((f64)configs.effects[effectIndex].sideDistance *
         cos((3.141592653589793 * origin->rotation.y) / 180.0)));
    particleIndex = 0;

     while (particleIndex < configs.effects[effectIndex].particleCount) {
        selectedOrigin = frandmod(100);
        selectedOrigin = 0;
        if (selectedOrigin == 0) {
             alpha = configs.effects[effectIndex].alpha;
             radius = configs.effects[effectIndex].spawnRadiusLimit;
             particleScale = configs.effects[effectIndex].baseParticleScale;
        } else {
             alpha = configs.effects[effectIndex].alternateAlpha;
             radius = configs.effects[effectIndex].alternateRadiusLimit;
             particleScale = configs.effects[effectIndex].alternateParticleScale;
        }

        lifetime = 10000;
        parameters.value00 = 0;
        parameters.color.red = 255;
        parameters.color.green = 159;
        parameters.color.blue = 159;
        parameters.color.alpha = (u8)alpha;
        parameters.finalColor[0] = 255;
        parameters.finalColor[1] = 159;
        parameters.finalColor[2] = 159;
        parameters.finalColor[3] = 0;
        parameters.motionValueDelta = 0.0f;
         parameters.opacityChange = configs.effects[effectIndex].opacityChange;
        parameters.colorChangeRate = 0.01f;
        parameters.velocityYAcceleration = 0.0f;

        angle = fn_1_3C54(360.0f);
        radius = fn_1_3C54(radius);
        position.x = (f32)((f64)spawnOrigins[selectedOrigin].x +
            ((f64)radius * sin((3.141592653589793 * angle) / 180.0)));
         position.y = configs.effects[effectIndex].spawnHeight;
        position.z = (f32)((f64)spawnOrigins[selectedOrigin].z +
            ((f64)radius * cos((3.141592653589793 * angle) / 180.0)));
        particleScale += fn_1_3C54(50.0f);
        if (effectIndex == 0) {
            parameters.height = -8.0f;
        } else if (effectIndex == 1) {
            parameters.height = -14.0f;
        } else if (effectIndex == 2) {
            parameters.height = -14.0f;
        }
        parameters.heightScale = 1.0f;
        parameters.velocityScale[0] = 1.0f;
        parameters.velocityScale[1] = 1.0f;
        parameters.velocityScale[2] = 1.0f;
        velocity.x = 0.0f;
        velocity.y = 1.0f;
        velocity.z = 0.0f;
        fn_1_57F8(1, -1, position, velocity, particleScale, lifetime, &parameters);
        particleIndex++;
    }
}

/* Emits three randomized particles around a model attachment point. */
void fn_1_4B38(M660Model *origin)
{
    Vec velocity;
    Vec position;
    Vec sourcePosition;
    M660ParticleParameters parameters;
    s16 effectIndex;

    sourcePosition = origin->attachment;
    sourcePosition.x += 10.0f;
    effectIndex = 0;

    while (effectIndex < 3) {
        s16 lifetime = 10000;
        f32 angle;
        f32 radius;
        f32 scaleRandom;
        f32 trigAngle;
        f32 trigRadius;
        f32 initialScale;
        f32 randomAngle;
        f32 centeredAngle;
        f32 randomRadius;
        f32 centeredRadius;
        f32 randomScale;
        f32 centeredScale;
        s32 angleLimit;
        s32 radiusLimit;
        s32 scaleLimit;

        parameters.value00 = 0;
        parameters.color.red = 255;
        parameters.color.green = 159;
        parameters.color.blue = 159;
        parameters.color.alpha = 128;
        parameters.finalColor[0] = 255;
        parameters.finalColor[1] = 159;
        parameters.finalColor[2] = 159;
        parameters.finalColor[3] = 0;
        parameters.motionValueDelta = 0.0f;
        parameters.opacityChange = -3.0f;
        parameters.colorChangeRate = 0.01f;
        parameters.velocityYAcceleration = 0.0f;

        angle = 360.0f;
        angleLimit = (s32)angle;
        angleLimit = frandmod(angleLimit);
        angle = (f32)angleLimit;
        randomAngle = angle;
        centeredAngle = randomAngle;
        trigAngle = centeredAngle;
        radius = 80.0f;
        radiusLimit = (s32)radius;
        radiusLimit = frandmod(radiusLimit);
        radius = (f32)radiusLimit;
        randomRadius = radius;
        centeredRadius = randomRadius;
        trigRadius = centeredRadius;
        position.x = (f32)((f64)sourcePosition.x +
                            ((f64)trigRadius *
                            sin((3.141592653589793 * (f64)trigAngle) /
                                180.0)));
        position.y = (f32)((f64)sourcePosition.y +
                            ((f64)trigRadius *
                            cos((3.141592653589793 * (f64)trigAngle) /
                                180.0)));
        position.z = 40.0f + sourcePosition.z;
        scaleRandom = 50.0f;
        scaleLimit = (s32)scaleRandom;
        scaleLimit = frandmod(scaleLimit);
        scaleRandom = (f32)scaleLimit;
        randomScale = scaleRandom;
        centeredScale = randomScale;
        initialScale = 150.0f + centeredScale;
        parameters.height = -2.0f;
        parameters.heightScale = 1.0f;
        parameters.velocityScale[0] = 1.0f;
        parameters.velocityScale[1] = 1.0f;
        parameters.velocityScale[2] = 1.0f;
        velocity.x = 3.0f;
        velocity.y = 0.0f;
        velocity.z = 0.0f;

        fn_1_57F8(1, -1, position, velocity, initialScale, lifetime,
                  (M660ParticleParameters *)&parameters);
        effectIndex++;
    }
}

/* Sets origin Y to 200 and model 2 to layer 1, then emits 30 particles with randomized velocity,
* alpha and fade rate; the first angle and radius draws are discarded but still advance the RNG. */
void fn_1_4F64(Vec *origin, s16 effectIndex)
{
    M660ParticleParameters parameters;
    Vec velocity;
    Vec position;
    M660VectorValues_4F64 verticalPositions = {{200.0f, 200.0f, 200.0f}};
    M660LayerValues_4F64 layers = {{1, 1, 1}};
    s16 particleIndex;

    origin->y = verticalPositions.yPositions[effectIndex];
    Hu3DModelLayerSet(lbl_1_bss_2484.modelIds[2],
                      (s16)layers.renderLayers[effectIndex]);

    particleIndex = 0;
    while (particleIndex < 30) {
        s16 lifetime = 10000;
        f32 angle;
        f32 radius;
        f32 initialScale;

        parameters.value00 = 0;
        parameters.color.red = 255;
        parameters.color.green = 159;
        parameters.color.blue = 159;
        parameters.color.alpha = 128;
        parameters.finalColor[0] = 255;
        parameters.finalColor[1] = 159;
        parameters.finalColor[2] = 159;
        parameters.finalColor[3] = 0;
        parameters.motionValueDelta = 0.0f;
        parameters.opacityChange = -6.0f;
        parameters.colorChangeRate = 0.01f;
        parameters.velocityYAcceleration = 0.0f;
        parameters.color.alpha = (u8)(frandmod(48) + 96);

        parameters.opacityChange = -(4.0f + fn_1_3C54(3.0f));
        angle = fn_1_3C54(360.0f);
        radius = fn_1_3C54(50.0f);
        position = *origin;
        initialScale = 10.0f + fn_1_3C54(10.0f);
        parameters.height = 4.0f;
        parameters.heightScale = 1.0f;
        parameters.velocityScale[0] = 0.94f;
        parameters.velocityScale[1] = 0.94f;
        parameters.velocityScale[2] = 0.94f;
        angle = fn_1_3C54(360.0f);
        velocity.x = (f32)(sin((3.141592653589793 * (f64)angle) /
                                180.0) *
                           (f64)(fn_1_3C54(20.0f) - 10.0f));
        velocity.y = fn_1_3C54(20.0f) -
                     10.0f;
        velocity.z = (f32)(cos((3.141592653589793 * (f64)angle) /
                                180.0) *
                           (f64)(fn_1_3C54(20.0f) -
                                 10.0f));

        fn_1_57F8(2, -1, position, velocity, initialScale, lifetime, &parameters);
        particleIndex++;
    }
}

/* Creates a particle that follows a stage object at one of three fixed heights. */
void fn_1_55DC(s16 objectIndex, s16 variant)
{
    M660ParticleParameters parameters;
    Vec velocity;
    Vec position;
    ParticleVectorValues_55DC positionValues = {{350.0f, 550.0f, 750.0f}};
    ParticleVectorValues_55DC scaleValues = {{300.0f, 400.0f, 500.0f}};
    s16 particleState = 1;

    parameters.value00 = 0;
    parameters.color.red = 255;
    parameters.color.green = 159;
    parameters.color.blue = 159;
    parameters.color.alpha = 208;
    parameters.finalColor[0] = 255;
    parameters.finalColor[1] = 159;
    parameters.finalColor[2] = 159;
    parameters.finalColor[3] = 48;
    parameters.motionValueDelta = 0.0f;
    parameters.opacityChange = 0.0f;
    parameters.colorChangeRate = 0.0f;
    parameters.velocityYAcceleration = 0.0f;

    position = lbl_1_bss_60C[objectIndex].position;
    position.y = positionValues.variantValues[variant];

    parameters.height = 0.0f;
    parameters.heightScale = 0.0f;
    parameters.velocityScale[0] = 1.0f;
    parameters.velocityScale[1] = 1.0f;
    parameters.velocityScale[2] = 1.0f;
    velocity.x = velocity.z = 0.0f;
    velocity.y = positionValues.variantValues[variant];
    fn_1_57F8(3, objectIndex, position, velocity, scaleValues.variantValues[variant],
              particleState, (M660ParticleParameters *)&parameters);
}

/* Stores a particle in the first inactive entry and returns its index, or -1 if full. */
 s16 fn_1_57F8(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity,
              f32 initialScale, s16 state, M660ParticleParameters *parameters) {
    HU3D_MODEL *model;
    s16 result;
    M660ParticleContext *context;
    M660ParticleParameters *currentParameters;
    s16 entryIndex;
    M660ParticleEntry *entry;

    result = -1;
    model = &Hu3DData[lbl_1_bss_2484.modelIds[modelNumber]];
    context = model->hookData;
    currentParameters = context->parameters;
    entryIndex = 0;
    entry = context->entries;

    while (entryIndex < context->entryCount) {
        if (entry->state <= 0) {
            currentParameters[entryIndex] = *parameters;
            currentParameters[entryIndex].values04[2] =
                currentParameters[entryIndex].values04[1] =
                    currentParameters[entryIndex].values04[0] = 0.0f;

            entry->position = position;
            entry->velocity = velocity;
            entry->origin = position;
            entry->baseScale = entry->colorIndex = entry->speedDecay = 0.0f;
            entry->color = currentParameters[entryIndex].color;
            entry->height = initialScale;
            entry->state = state;
            entry->ownerIndex = ownerIndex;
            context->lastEntry = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

/* Updates particle motion, color and height values each particle animation step. */
void fn_1_59F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M660ParticleParameters *parameters =
        ((M660ParticleContext *)particle)->parameters;
    M660ParticleEntry *entry = ((M660ParticleContext *)particle)->entries;
    s16 entryNo = 0;

    while (entryNo < ((M660ParticleContext *)particle)->entryCount) {
        if (entry->state-- > 0) {
            s16 channelValue;

            entry->velocity.x *= parameters[entryNo].velocityScale[0];
            entry->velocity.y *= parameters[entryNo].velocityScale[1];
            entry->velocity.z *= parameters[entryNo].velocityScale[2];
            PSVECAdd((Point3d *)&entry->velocity, (Point3d *)&entry->position,
                     (Point3d *)&entry->position);
            entry->velocity.y += parameters[entryNo].velocityYAcceleration;
            entry->motionValue += parameters[entryNo].motionValueDelta;

            channelValue = (s16)((f32)entry->color.red +
                                 (parameters[entryNo].colorChangeRate *
                                  (f32)(parameters[entryNo].finalColor[0] -
                                        parameters[entryNo].color.red)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.red = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.green +
                                 (parameters[entryNo].colorChangeRate *
                                  (f32)(parameters[entryNo].finalColor[1] -
                                        parameters[entryNo].color.green)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.green = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.blue +
                                 (parameters[entryNo].colorChangeRate *
                                  (f32)(parameters[entryNo].finalColor[2] -
                                        parameters[entryNo].color.blue)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.blue = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.alpha +
                                 parameters[entryNo].opacityChange);
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.alpha = (u8)channelValue;
            if (channelValue <= 0) {
                entry->state = 0;
                entry->height = 0.0f;
            }

            parameters[entryNo].height *= parameters[entryNo].heightScale;
            entry->height += parameters[entryNo].height;
            if (entry->height <= 0.0f) {
                entry->height = 0.0f;
                entry->state = 0;
            }
            DCStoreRangeNoSync(((M660ParticleContext *)particle)->entries,
                               ((M660ParticleContext *)particle)->entryCount * 72);
        } else {
            entry->height = 0.0f;
            entry->state = -1;
        }
        entryNo++;
        entry++;
    }
}

/* Follows each particle's stage object until the result phase, then follows the losing player or
 * hides the particle on a tie. */
void fn_1_5E70(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M660ParticleParameters *parameters = particle->work;
    HU3D_PARTICLE_DATA *entry = particle->data;
    s16 index = 0;

    while (index < particle->maxCnt) {
        switch (entry->time) {
        case 0:
            break;
        case 1:
        {
            entry->pos = lbl_1_bss_60C[entry->parManId].position;
            entry->pos.y = entry->vel.y;
            if (lbl_1_bss_1F4.phase >= 2) {
                entry->parManId = 72;
                entry->scale = 250.0f;
                entry->vel.z = 80.0f;
                entry->time++;
            }
            break;
        }
        case 2:
            if (lbl_1_bss_1F4.tie == 0) {
                entry->pos = lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position;
                entry->pos.y += 30.0f;
                entry->pos.z += 100.0f;
            } else {
                entry->scale = 0.0f;
                entry->time = 0;
            }
            break;
        }
        DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
        index++;
        entry++;
    }
}

/* Sets voice pan and volume, then plays the setup sound once. Modes 1 and 3-6 use fixed
* volume; both callers use mode 6, which pans by X and forces maximum volume. */
void fn_1_601C(M660Player *player, s16 direction)
{
    s16 pan;
    s16 volume;
    f32 panScale;
    f32 volumeScale;
    f32 positionDelta;

    if (direction == 0 || direction == 1 || direction == 6) {
        if (lbl_1_bss_1F4.rightEdge >= player->position.x &&
            lbl_1_bss_1F4.leftEdge <= player->position.x) {
            panScale =
                (f32)(lbl_1_bss_1F4.maximumX - lbl_1_bss_1F4.minimumX) /
                (lbl_1_bss_1F4.rightEdge - lbl_1_bss_1F4.leftEdge);
            positionDelta = player->position.x - lbl_1_bss_1F4.leftEdge;
            positionDelta *= panScale;
            pan = (s16)positionDelta;
            pan += lbl_1_bss_1F4.minimumX;
        } else if (lbl_1_bss_1F4.rightEdge < player->position.x) {
            pan = lbl_1_bss_1F4.maximumX;
        } else if (lbl_1_bss_1F4.leftEdge > player->position.x) {
            pan = lbl_1_bss_1F4.minimumX;
        }
    }

    if (direction == 0 || direction == 2) {
        if (lbl_1_bss_1F4.bottomEdge >= player->position.z &&
            lbl_1_bss_1F4.topEdge <= player->position.z) {
            volumeScale =
                (f32)(lbl_1_bss_1F4.maximumY - lbl_1_bss_1F4.minimumY) /
                (lbl_1_bss_1F4.bottomEdge - lbl_1_bss_1F4.topEdge);
            positionDelta = player->position.z - lbl_1_bss_1F4.topEdge;
            positionDelta *= volumeScale;
            volume = (s16)positionDelta;
            volume += lbl_1_bss_1F4.minimumY;
        } else if (lbl_1_bss_1F4.bottomEdge < player->position.z) {
            volume = lbl_1_bss_1F4.maximumY;
        } else if (lbl_1_bss_1F4.topEdge > player->position.z) {
            volume = lbl_1_bss_1F4.minimumY;
        }
    }

    if (direction == 2 || direction == 3 || direction == 4 || direction == 5) {
        pan = lbl_1_bss_1F4.minimumX +
              (lbl_1_bss_1F4.maximumX - lbl_1_bss_1F4.minimumX) / 2;
    }
    if (direction == 1 || direction == 3) {
        volume = lbl_1_bss_1F4.minimumY +
                 (lbl_1_bss_1F4.maximumY - lbl_1_bss_1F4.minimumY) / 2;
    }
    if (direction == 4) {
        volume = lbl_1_bss_1F4.minimumY;
    }
    if (direction == 5 || direction == 6) {
        volume = lbl_1_bss_1F4.maximumY;
    }

    CharModelVoicePanSet(player->character, pan);
    CharModelVoiceVolSet(player->character, volume);
    if (player->setupState == 1) {
        VoiceSoundIds_601C soundIds = {{MSM_SE_CHARVOICE_MARIO + 3,
            MSM_SE_CHARVOICE_LUIGI + 3, MSM_SE_CHARVOICE_PEACH + 3,
            MSM_SE_CHARVOICE_YOSHI + 3, MSM_SE_CHARVOICE_WARIO + 3,
            MSM_SE_CHARVOICE_DAISY + 3, MSM_SE_CHARVOICE_WALUIGI + 3,
            MSM_SE_CHARVOICE_KINOPIO + 3, MSM_SE_CHARVOICE_TERESA + 3,
            MSM_SE_CHARVOICE_MINIKOOPA + 3, MSM_SE_CHARVOICE_KINOPIKO + 3,
            MSM_SE_CHARVOICE_MINIKOOPAR + 3, MSM_SE_CHARVOICE_MINIKOOPAG + 3,
            MSM_SE_CHARVOICE_MINIKOOPAB + 3}};

        HuAudFXPlayVolPan(soundIds.soundIds[player->character], volume, pan);
        player->setupState = -1;
    }
}

/* Initializes streamed audio handles, spatial sound bounds and four effect-sound slots. */
void fn_1_64D0(void) {
    s16 soundSlot;

    lbl_1_bss_1F4.firstStreamId = -1;
    lbl_1_bss_1F4.secondStreamId = -1;
    lbl_1_bss_1F4.backgroundTrackId = 85;
    lbl_1_bss_1F4.jingleId = -1;
    lbl_1_bss_1F4.minimumX = 32;
    lbl_1_bss_1F4.maximumX = 96;
    lbl_1_bss_1F4.leftEdge = (f32) -800.0f;
    lbl_1_bss_1F4.rightEdge = (f32) 800.0f;
    lbl_1_bss_1F4.minimumY = 32;
    lbl_1_bss_1F4.maximumY = 110;
    lbl_1_bss_1F4.topEdge = (f32) -800.0f;
    lbl_1_bss_1F4.bottomEdge = (f32) 800.0f;
    soundSlot = 0;
    while (soundSlot < 4) {
        lbl_1_bss_1F4.soundStatus[soundSlot] = -1;
        lbl_1_bss_1F4.soundEffectId[soundSlot] = -1;
        soundSlot += 1;
    }
}

/* Starts the queued jingle once the second stream channel becomes available. */
void fn_1_6614(void) {
    if (((s32) lbl_1_bss_1F4.jingleId != -1) && ((s32) lbl_1_bss_1F4.secondStreamId == -1)) {
        lbl_1_bss_1F4.secondStreamId = HuAudJinglePlay((s16) lbl_1_bss_1F4.jingleId);
    }
}

/* Starts the selected background track when no first-channel stream is active. */
void fn_1_6678(void) {
    if ((s32) lbl_1_bss_1F4.firstStreamId == -1) {
        lbl_1_bss_1F4.firstStreamId = HuAudBGMPlay((s16) lbl_1_bss_1F4.backgroundTrackId);
    }
}

/* Fades the selected stream and clears its handle after requesting the fade. */
void fn_1_66C8(s16 streamSlot, s16 fadeFrames) {
    s16 streamId = streamSlot != 0
                       ? lbl_1_bss_1F4.firstStreamId
                       : lbl_1_bss_1F4.secondStreamId;
    if (streamId != -1) {
        HuAudSStreamFadeOut(streamId, fadeFrames);
        if (streamSlot != 0) {
            lbl_1_bss_1F4.firstStreamId = -1;
            return;
        }
        lbl_1_bss_1F4.secondStreamId = -1;
    }
}

/* Stops the selected jingle or background stream immediately. */
void fn_1_6774(s16 streamSelector)
{
    s16 streamId;

    if (streamSelector != 0) {
        streamId = (s16)lbl_1_bss_1F4.firstStreamId;
    } else {
        streamId = (s16)lbl_1_bss_1F4.secondStreamId;
    }

    HuAudSStreamStop(streamId);
}

/* Starts a stage sound with the selected pan and volume mode; every caller uses mode 5 and a NULL
* position, centering pan and forcing maximum volume without reading the position. */
void fn_1_67D4(s16 slot, s16 mode, Vec *position)
{
    /* This legacy call accepts full-width volume and pan arguments. */
    int HuAudFXPlayVolPan();
    s32 pan;
    s32 volume;
    f32 positionDelta;
    f32 panScale;
    f32 volumeScale;

    if (mode <= 5) {
    if (mode == 0 || mode == 1) {
        if (lbl_1_bss_1F4.rightEdge >= position->x &&
            lbl_1_bss_1F4.leftEdge <= position->x) {
            panScale = (f32)(lbl_1_bss_1F4.maximumX -
                             lbl_1_bss_1F4.minimumX) /
                       (lbl_1_bss_1F4.rightEdge -
                        lbl_1_bss_1F4.leftEdge);
            positionDelta = position->x -
                            lbl_1_bss_1F4.leftEdge;
            positionDelta *= panScale;
            pan = (s32)positionDelta;
            pan += lbl_1_bss_1F4.minimumX;
        } else if (lbl_1_bss_1F4.rightEdge < position->x) {
            pan = lbl_1_bss_1F4.maximumX;
        } else if (lbl_1_bss_1F4.leftEdge > position->x) {
            pan = lbl_1_bss_1F4.minimumX;
        }
    }

    if (mode == 0 || mode == 2) {
        if (lbl_1_bss_1F4.bottomEdge >= position->z &&
            lbl_1_bss_1F4.topEdge <= position->z) {
            volumeScale = (f32)(lbl_1_bss_1F4.maximumY -
                                lbl_1_bss_1F4.minimumY) /
                          (lbl_1_bss_1F4.bottomEdge -
                           lbl_1_bss_1F4.topEdge);
            positionDelta = position->z -
                            lbl_1_bss_1F4.topEdge;
            positionDelta *= volumeScale;
            volume = (s32)positionDelta;
            volume += lbl_1_bss_1F4.minimumY;
        } else if (lbl_1_bss_1F4.bottomEdge < position->z) {
            volume = lbl_1_bss_1F4.maximumY;
        } else if (lbl_1_bss_1F4.topEdge > position->z) {
            volume = lbl_1_bss_1F4.minimumY;
        }
    }

    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
        pan = (s16)(lbl_1_bss_1F4.minimumX +
                    (lbl_1_bss_1F4.maximumX -
                     lbl_1_bss_1F4.minimumX) / 2);
    }

    if (mode == 1 || mode == 3) {
        volume = (s16)(lbl_1_bss_1F4.minimumY +
                       (lbl_1_bss_1F4.maximumY -
                        lbl_1_bss_1F4.minimumY) / 2);
    }

    if (mode == 4) {
        volume = lbl_1_bss_1F4.minimumY;
    }

    if (mode == 5) {
        volume = lbl_1_bss_1F4.maximumY;
    }
    lbl_1_bss_1F4.soundStatus[slot] = 1;
    lbl_1_bss_1F4.soundEffectId[slot] =
        HuAudFXPlayVolPan(lbl_1_data_F6[slot], volume, pan);
    }
}

s16 lbl_1_data_F6[5] = {2111, 2112, 2113, 2114, 0};
