/* Boonanza! module core: module entry and sequence phase callbacks, player setup and
 * input, model and motion updates, camera, lighting and shadow, timers, particles and
 * sound. */
#include <math.h>
#include <string.h>

#include "dolphin/gx.h"

#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"

#include "game/object.h"

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

#include "datadir_enum.h"

#include "dolphin/types.h"

#include "include/game/mg/actman.h"

#include "include/game/gamemes.h"

#include "game/hu3d.h"

#include "humath.h"

#include "dolphin/mtx.h"

#include "REL/m661dll/system_state.h"
#include "REL/m661dll/player_actors.h"
#include "REL/m661dll/light.h"
#include "REL/m661dll/motion.h"
extern MotionControl lbl_1_bss_10[0x4E];
#include "REL/m661dll/score_display.h"
#include "REL/m661dll.h"
#include "game/printfunc.h"
#include "REL/m661dll/particle.h"
#include "REL/m661dll/sound.h"
#include "msm_se.h"

s32 fn_1_57A4(void);

typedef void (*VoidFunc)(void);

void fn_1_A0(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_118(void);
void fn_1_26C(s16 mode, s16 frameNo);
void fn_1_298(s16 mode, s16 frameNo);
void fn_1_2C4(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_31C(s16 mode, s16 frameNo);
void fn_1_348(s16 mode, s16 frameNo);
void fn_1_374(s16 mode, s16 frameNo);
void fn_1_3A0(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0,
    (MGSEQ_FUNC)fn_1_118,
    fn_1_26C,
    fn_1_298,
    fn_1_2C4,
    fn_1_2F0,
    fn_1_31C,
    fn_1_348,
    fn_1_374,
    fn_1_3A0,
};

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

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

typedef struct CameraRecord {
    s32 cameraIndex;
    u8 beforeRotation[0x0C];
    HuVecF rotation;
    HuVecF position;
    u8 beforeViewport[0x0C];
    f32 viewport[6];
    f32 fieldOfView;
} CameraRecord;

typedef struct CameraRecordStorage {
    CameraRecord records[4];
    /* The game's code never reads these bytes. */
    u8 unknown140[8]; /* No code names this field. */
} CameraRecordStorage;

typedef struct ProcessStorage {
    HUPROCESS *process;
    /* The target object is 8 bytes; only the first pointer is consumed here. */
    u8 unknown04[4]; /* No code names this field. */
} ProcessStorage;

extern ProcessStorage lbl_1_bss_31C0;
extern CameraRecordStorage lbl_1_bss_3078;

typedef struct ModelSetupRecord {
    HU3D_MODELID modelId;
    HuVecF position;
    HuVecF shadowPosition;
    HuVecF rotation;
} ModelSetupRecord;

extern ModelSetupRecord lbl_1_bss_1E3C[0x71];

typedef struct ModelAnimationWork {
    s16 motionIds[5];
    s16 motionId;
    s16 appliedMotionId;
    f32 motionSpeed;
    f32 appliedMotionSpeed;
    f32 motionTime;
    f32 appliedMotionTime;
    f32 motionDuration;
    u32 modelAttributes;
    u32 appliedModelAttributes;
    u32 attributesToReset;
    s32 motionEnded;
    s16 repeatMotion;
    s16 nextMotionId;
    f32 nextMotionTime;
    u32 nextModelAttributes;
    u32 transitionState;
    s16 shiftState;
    s16 shiftedMotionId;
    u32 shiftAttributes;
    f32 shiftTime;
    f32 shiftDuration;
    f32 shiftElapsed;
    f32 shiftMaxTime;
    /* The two animation records have a 0x60-byte stride; these bytes are unidentified. */
    u8 unknown5C[4]; /* No code names this field. */
} ModelAnimationWork;

extern ModelAnimationWork lbl_1_bss_1D7C[2];

typedef struct PlayerRoundState {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 inputStickX;
    s8 inputStickY;
    s8 subStickX;
    s8 subStickY;
    u8 triggerL;
    u8 triggerR;
    u8 dPad;
    u8 dPadRepeat;
    u8 unknown12[2]; /* No code names this field. */
    s8 groupSlot;
    s8 playerNumber;
    s8 character;
    s8 controllerPort;
    s8 group;
    s8 playerType;
    s8 computerDifficulty;
    s8 choiceMode;
    s8 selectedCharacter;
    u8 unknown1D[0x17]; /* No code names this field. */
    HuVecF startPosition;
    HuVecF previousPosition;
    HuVecF endPosition;
    s8 playerStatus[2];
    s16 currentValue, previousValue, pairedIndex, previousPairedIndex, updateCount;
    s16 score;
    s32 padX, padY, stickX, stickY;
    u8 unknown78[0x44]; /* No code names this field. */
} PlayerRoundState;

extern PlayerRoundState lbl_1_bss_1C04[2];

extern s16 lbl_802C0278;

void fn_1_1790(s16 mode);

void fn_1_19C(void);

void fn_1_1C0(s16 mode, s16 frameNo);

void fn_1_1F4(s16 mode, s16 frameNo);

void fn_1_1F8(s16 mode, s16 frameNo);

typedef struct Position {
    HuVecF position;
    unsigned char unk_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

extern u16 lbl_802C0010[];

extern u16 lbl_802C0008[];

extern u16 lbl_802C0000[];

extern s8 lbl_802BFFFC[];

extern u8 lbl_802BFFEC[];

extern u8 lbl_802BFFE8[];

extern u8 lbl_802BFFE4[];

extern u8 lbl_802BFFE0[];

#define PLAYER_INPUT(index_) (&lbl_1_bss_1C04[index_])

extern CameraRecord lbl_1_bss_B08[4];

extern void fn_1_1DC8(s16 cameraIndex);

void fn_1_1C40(s16 cameraIndex);

typedef struct FourHalfwords {
    u16 first;
    u16 second;
    u16 third;
    u16 fourth;
} FourHalfwords;

typedef struct LightInitState {
    HU3D_LIGHTID id;
    HuVecF position;
    HuVecF orientation;
    HuVecF aim;
    HuVecF direction;
    float range;
    GXColor color;
} LightInitState;

extern LightInitState lbl_1_bss_303C;
void fn_1_1E48(void);

typedef struct ShadowCameraState {
    HuVecF cameraPosition;
    f32 azimuth;
    f32 elevation;
    f32 unknownAfterElevation;
    HuVecF target;
    f32 unknown24[3]; /* No code names this field. */
    HuVecF cameraUp;
    f32 unknown3C[3]; /* No code names this field. */
    f32 orbitDistance;
    f32 shadowTransparency;
} ShadowCameraState;

extern ShadowCameraState lbl_1_bss_2FEC;

extern PlayerRoundState lbl_1_bss_1C04[];
extern ModelSetupRecord lbl_1_bss_1E3C[];

extern f64 sin(f64 angle);
extern f64 cos(f64 angle);

static inline void updatePlayerModels(void)
{
    s32 modelIndex;

    modelIndex = 2;
    while (modelIndex < 0x71) {
        fn_80026760(lbl_1_bss_1E3C[modelIndex].modelId,
            lbl_1_bss_1E3C[modelIndex].position.x,
            lbl_1_bss_1E3C[modelIndex].position.y,
            lbl_1_bss_1E3C[modelIndex].position.z);
        fn_80026808(lbl_1_bss_1E3C[modelIndex].modelId,
            lbl_1_bss_1E3C[modelIndex].rotation.x,
            lbl_1_bss_1E3C[modelIndex].rotation.y,
            lbl_1_bss_1E3C[modelIndex].rotation.z);
        modelIndex++;
    }

    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_1C04[modelIndex].playerStatus[0] == 0 ||
            lbl_1_bss_1C04[modelIndex].playerStatus[0] == 2) {
            fn_80026760(lbl_1_bss_1E3C[modelIndex].modelId,
                lbl_1_bss_1C04[modelIndex].startPosition.x,
                lbl_1_bss_1C04[modelIndex].startPosition.y,
                lbl_1_bss_1C04[modelIndex].startPosition.z);
            fn_80026808(lbl_1_bss_1E3C[modelIndex].modelId,
                lbl_1_bss_1C04[modelIndex].endPosition.x,
                lbl_1_bss_1C04[modelIndex].endPosition.y,
                lbl_1_bss_1C04[modelIndex].endPosition.z);
        }
        modelIndex++;
    }
}

static inline void updateShadowCamera(void)
{
    lbl_1_bss_2FEC.cameraPosition.x = (f32)((f64)lbl_1_bss_2FEC.target.x +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
          sin((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0))));
    lbl_1_bss_2FEC.cameraPosition.y = (f32)((f64)lbl_1_bss_2FEC.target.y +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         -sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0)));
    lbl_1_bss_2FEC.cameraPosition.z = (f32)((f64)lbl_1_bss_2FEC.target.z +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
          cos((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0))));
    lbl_1_bss_2FEC.cameraUp.x = (f32)(
        sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
        sin((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0));
    lbl_1_bss_2FEC.cameraUp.y = (f32)cos(
        (M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0);
    lbl_1_bss_2FEC.cameraUp.z = (f32)(
        sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
        cos((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0));
    Hu3DShadowTPLvlSet(lbl_1_bss_2FEC.shadowTransparency);
    Hu3DShadowPosSet(&lbl_1_bss_2FEC.cameraPosition,
                     &lbl_1_bss_2FEC.cameraUp, &lbl_1_bss_2FEC.target);
}

extern s8 lbl_802BFFF8[];

extern int printf();
void fn_1_335C(s16 player, s16 selection);

typedef struct ParticleSettings {
    s16 modelIds[1];
    s16 particleCounts[1];
    void *work[1];
} ParticleSettings;

extern ParticleSettings lbl_1_bss_2FE4;

typedef struct ParticleSpawnWork {
    u32 flags;
    HuVecF position;
    GXColor startColor;
    GXColor endColor;
    f32 motion[9];
} ParticleSpawnWork;

s16 fn_1_42E8(s16 particleSlot, s16 particleId, HuVecF *position, HuVecF *velocity, f32 scale, s16 lifetime, ParticleSpawnWork *initialWork);

/* Creates the object manager and its camera-view object, starts the minigame sequence,
 * and sets sequence phases 2 and 7 to last 1000 frames. */
void fn_1_A0(void) {
    lbl_1_bss_31C0.process = fn_80080418();
    fn_8003AB38(lbl_1_bss_31C0.process, 0x7FDA, 0U, 0U, -1, fn_8003D4FC);
    fn_80070788(&lbl_1_data_0);
    fn_80071CE8(2, 0x3E8);
    fn_80071CE8(7, 0x3E8);
}

void fn_1_118(void)
{
    s16 mode;

    memset(&lbl_1_bss_3078, 0, 0x148);
    memset(&lbl_1_bss_303C, 0, 0x3C);
    memset(&lbl_1_bss_2FEC, 0, 0x50);
    memset(&lbl_1_bss_2FE4, 0, 8);
    memset(lbl_1_bss_1E3C, 0, 0x11A8);
    memset(lbl_1_bss_1D7C, 0, 0xC0);
    memset(lbl_1_bss_1C04, 0, 0x178);
    memset(&lbl_1_bss_196C, 0, 0x298);
    memset(&lbl_1_bss_1960, 0, 0xC);
    memset(lbl_1_bss_1870, 0, 0xF0);
    memset(lbl_1_bss_10, 0, 0x1860);
    memset(lbl_1_bss_0, 0, 0x10);
    mode = lbl_802C0278;
    lbl_1_bss_196C.mode = mode;
    fn_1_1790(0);
    fn_1_20B4();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_88B8();
    fn_1_4D1C();
    fn_80071A9C();
}

void fn_1_26C(s16 mode, s16 frameNo)
{
    if (fn_1_8DD4() != 0) fn_80071A9C();
}

void fn_1_298(s16 mode, s16 frameNo)
{
    if (fn_1_8E14() != 0) fn_80071A9C();
}

void fn_1_2C4(s16 mode, s16 frameNo)
{
    if (fn_1_57A4() != 0) fn_80071A9C();
}

void fn_1_2F0(s16 mode, s16 frameNo)
{
    if (fn_1_BA38() != 0) fn_80071A9C();
}

void fn_1_31C(s16 mode, s16 frameNo)
{
    if (fn_1_BAA0() != 0) fn_80071A9C();
}

void fn_1_348(s16 mode, s16 frameNo)
{
    if (fn_1_BAE0() != 0) fn_80071A9C();
}

void fn_1_374(s16 mode, s16 frameNo)
{
    if (fn_1_BD20() != 0) fn_80071A9C();
}

void fn_1_3A0(s16 mode, s16 frameNo)
{
    if (fn_1_BD48() != 0) fn_80071A9C();
}

void fn_1_3CC(void)
{
}

void fn_1_3D0(void)
{

    s16 playerLookup[2] = {0xFFFF, 0xFFFF};
    HuVecF startPositions[2] = {
        { -721.5f, 10.0f, 0.0f }, { 721.5f, 10.0f, 0.0f },
    };
    HuVecF endPositions[2] = {
        { 0.0f, 0.0f, 0.0f }, { 0.0f, -90.0f, 0.0f },
    };
    s16 playerIndex;
    s16 group;

    playerIndex = 0;
    group = 0;
    for (; playerIndex < 4; playerIndex++) {
        group = lbl_80265728[playerIndex].grpNo;

        if ((group == 0 || group == 1) && playerLookup[group] == -1) {
            playerLookup[group] = playerIndex;
        }
    }

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        lbl_1_bss_1C04[playerIndex].groupSlot =
            (s8)playerLookup[playerIndex];
        lbl_1_bss_1C04[playerIndex].playerNumber =
            (s8)playerIndex;
        lbl_1_bss_1C04[playerIndex].character =
            (s8)lbl_80265728[playerLookup[playerIndex]].charNo;
        lbl_1_bss_1C04[playerIndex].controllerPort =
            (s8)lbl_80265728[playerLookup[playerIndex]].padNo;
        lbl_1_bss_1C04[playerIndex].group =
            (s8)lbl_80265728[playerLookup[playerIndex]].grpNo;
        lbl_1_bss_1C04[playerIndex].playerType =
            (s8)lbl_80265728[playerLookup[playerIndex]].type;
        lbl_1_bss_1C04[playerIndex].computerDifficulty =
            (s8)lbl_80265728[playerLookup[playerIndex]].comDif;
        lbl_1_bss_1C04[playerIndex].startPosition =
            startPositions[playerIndex];
        lbl_1_bss_1C04[playerIndex].endPosition =
            endPositions[playerIndex];
        lbl_1_bss_1C04[playerIndex].selectedCharacter = -1;
    }
}

void fn_1_71C(void)
{
    s32 index;
    s32 stickResult;
    s32 deadZoneValue;

    index = 0;
    for (; index < 2; index++) {
        if (PLAYER_INPUT(index)->playerType != 0 ||
            PLAYER_INPUT(index)->choiceMode != 0) {
            memset(PLAYER_INPUT(index), 0, 0x14);
        } else {
            PLAYER_INPUT(index)->buttons = lbl_802C0010[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->buttonsDown = lbl_802C0008[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->buttonsRepeat = lbl_802C0000[PLAYER_INPUT(index)->controllerPort];
            if (lbl_802BFFFC[PLAYER_INPUT(index)->controllerPort] > 0) {
                stickResult = lbl_802BFFFC[PLAYER_INPUT(index)->controllerPort];
            } else {
                if (lbl_802BFFFC[PLAYER_INPUT(index)->controllerPort] * -1 < 0xA) {
                    deadZoneValue = 0;
                } else {
                    deadZoneValue = lbl_802BFFFC[PLAYER_INPUT(index)->controllerPort];
                }
                stickResult = deadZoneValue;
            }
            PLAYER_INPUT(index)->inputStickX = stickResult;
            PLAYER_INPUT(index)->inputStickY = (s8)lbl_802BFFF8[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->subStickX = (s8)lbl_802BFFF4[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->subStickY = (s8)lbl_802BFFF0[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->triggerL = lbl_802BFFEC[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->triggerR = lbl_802BFFE8[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->dPad = lbl_802BFFE4[PLAYER_INPUT(index)->controllerPort];
            PLAYER_INPUT(index)->dPadRepeat = lbl_802BFFE0[PLAYER_INPUT(index)->controllerPort];
        }
    }

    index = 0;
    for (; index < 4; index++) {
        lbl_1_bss_196C.pads[index].buttons = lbl_802C0010[index];
        lbl_1_bss_196C.pads[index].buttonsDown = lbl_802C0008[index];
        lbl_1_bss_196C.pads[index].buttonsRepeat = lbl_802C0000[index];
        lbl_1_bss_196C.pads[index].stickX = (s8)lbl_802BFFFC[index];
        lbl_1_bss_196C.pads[index].stickY = (s8)lbl_802BFFF8[index];
        lbl_1_bss_196C.pads[index].subStickX = (s8)lbl_802BFFF4[index];
        lbl_1_bss_196C.pads[index].subStickY = (s8)lbl_802BFFF0[index];
        lbl_1_bss_196C.pads[index].triggerL = lbl_802BFFEC[index];
        lbl_1_bss_196C.pads[index].triggerR = lbl_802BFFE8[index];
        lbl_1_bss_196C.pads[index].dPad = lbl_802BFFE4[index];
        lbl_1_bss_196C.pads[index].dPadRepeat = lbl_802BFFE0[index];
    }
}

/* Applies the stored position and rotation of stage models 2-112, then places each
 * player model whose status is 0 or 2 at its start position, using its end-position
 * vector as the rotation. */
void fn_1_CDC(void) {
    s32 index;
    index = 2;
    while (index < 0x71)
    {
        fn_80026760(lbl_1_bss_1E3C[index].modelId, lbl_1_bss_1E3C[index].position.x, lbl_1_bss_1E3C[index].position.y, lbl_1_bss_1E3C[index].position.z);
        fn_80026808(lbl_1_bss_1E3C[index].modelId, lbl_1_bss_1E3C[index].rotation.x, lbl_1_bss_1E3C[index].rotation.y, lbl_1_bss_1E3C[index].rotation.z);
        index += 1;
    }
    index = 0;
    while (index <= 1)
    {
        if (lbl_1_bss_1C04[index].playerStatus[0] == 0 ||
            lbl_1_bss_1C04[index].playerStatus[0] == 2)
        {
            fn_80026760(lbl_1_bss_1E3C[index].modelId, lbl_1_bss_1C04[index].startPosition.x, lbl_1_bss_1C04[index].startPosition.y, lbl_1_bss_1C04[index].startPosition.z);
            fn_80026808(lbl_1_bss_1E3C[index].modelId, lbl_1_bss_1C04[index].endPosition.x, lbl_1_bss_1C04[index].endPosition.y, lbl_1_bss_1C04[index].endPosition.z);
        }
        index += 1;
    }
}

void fn_1_EB8(void)
{
    u32 motionData[5] = {
        DATANUM(0x930000, 0), DATANUM(0x930000, 2),
        DATANUM(0x930000, 6), DATANUM(0x930000, 7),
        DATANUM(0x930000, 24),
    };
    s16 playerIndex;
    s16 motionIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        for (motionIndex = 0; motionIndex < 5; motionIndex++) {
            lbl_1_bss_1D7C[playerIndex].motionIds[motionIndex] =
                fn_8005B0B8(lbl_1_bss_1C04[playerIndex].character,
                                 motionData[motionIndex]);
        }
    }
}

void fn_1_F98(void)
{
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        if (lbl_1_bss_1D7C[playerIndex].motionId !=
            lbl_1_bss_1D7C[playerIndex].appliedMotionId) {
            fn_8002C95C(lbl_1_bss_1E3C[playerIndex].modelId,
                          lbl_1_bss_1D7C[playerIndex].motionIds[
                              lbl_1_bss_1D7C[playerIndex].motionId]);
            lbl_1_bss_1D7C[playerIndex].appliedMotionId =
                lbl_1_bss_1D7C[playerIndex].motionId;
            lbl_1_bss_1D7C[playerIndex].motionDuration =
                fn_8002D530(lbl_1_bss_1E3C[playerIndex].modelId);
        }
        if (lbl_1_bss_1D7C[playerIndex].motionSpeed !=
            lbl_1_bss_1D7C[playerIndex].appliedMotionSpeed) {
            fn_8002D86C(lbl_1_bss_1E3C[playerIndex].modelId,
                               lbl_1_bss_1D7C[playerIndex].motionSpeed);
            lbl_1_bss_1D7C[playerIndex].appliedMotionSpeed =
                lbl_1_bss_1D7C[playerIndex].motionSpeed;
        }
        if (lbl_1_bss_1D7C[playerIndex].modelAttributes !=
            lbl_1_bss_1D7C[playerIndex].appliedModelAttributes) {
            fn_800269E8(lbl_1_bss_1E3C[playerIndex].modelId,
                             lbl_1_bss_1D7C[playerIndex].modelAttributes);
            lbl_1_bss_1D7C[playerIndex].appliedModelAttributes =
                lbl_1_bss_1D7C[playerIndex].modelAttributes;
        }
        if (lbl_1_bss_1D7C[playerIndex].attributesToReset != 0) {
            fn_80026A44(lbl_1_bss_1E3C[playerIndex].modelId,
                               lbl_1_bss_1D7C[playerIndex].attributesToReset);
            lbl_1_bss_1D7C[playerIndex].attributesToReset =
                ~lbl_1_bss_1D7C[playerIndex].attributesToReset;
            lbl_1_bss_1D7C[playerIndex].appliedModelAttributes =
                lbl_1_bss_1D7C[playerIndex].modelAttributes =
                    lbl_1_bss_1D7C[playerIndex].modelAttributes &
                    lbl_1_bss_1D7C[playerIndex].attributesToReset;
            lbl_1_bss_1D7C[playerIndex].attributesToReset = 0;
        }
        if (lbl_1_bss_1D7C[playerIndex].motionTime !=
            lbl_1_bss_1D7C[playerIndex].appliedMotionTime) {
            fn_8002D414(lbl_1_bss_1E3C[playerIndex].modelId,
                              lbl_1_bss_1D7C[playerIndex].motionTime);
        }
        switch (lbl_1_bss_1D7C[playerIndex].shiftState) {
        case 0:
            break;
        case 1:
            fn_8002CCC0(
                lbl_1_bss_1E3C[playerIndex].modelId,
                lbl_1_bss_1D7C[playerIndex].motionIds[
                    lbl_1_bss_1D7C[playerIndex].shiftedMotionId],
                lbl_1_bss_1D7C[playerIndex].shiftTime,
                lbl_1_bss_1D7C[playerIndex].shiftDuration,
                lbl_1_bss_1D7C[playerIndex].shiftAttributes);
            lbl_1_bss_1D7C[playerIndex].shiftMaxTime =
                fn_8002D5D4(
                    lbl_1_bss_1E3C[playerIndex].modelId);
            lbl_1_bss_1D7C[playerIndex].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_1D7C[playerIndex].shiftElapsed =
                fn_8002D508(lbl_1_bss_1E3C[playerIndex].modelId);
            if (lbl_1_bss_1D7C[playerIndex].shiftElapsed >=
                lbl_1_bss_1D7C[playerIndex].shiftDuration - 1.1f) {
                lbl_1_bss_1D7C[playerIndex].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_1D7C[playerIndex].shiftState = 0;
            break;
        }
        if (lbl_1_bss_1D7C[playerIndex].shiftState == 0) {
            lbl_1_bss_1D7C[playerIndex].motionTime =
                fn_8002D4E0(lbl_1_bss_1E3C[playerIndex].modelId);
            lbl_1_bss_1D7C[playerIndex].appliedMotionTime =
                lbl_1_bss_1D7C[playerIndex].motionTime;
        } else {
            lbl_1_bss_1D7C[playerIndex].appliedMotionTime =
                lbl_1_bss_1D7C[playerIndex].motionTime = -1.0f;
        }
        lbl_1_bss_1D7C[playerIndex].motionEnded =
            fn_8002D75C(lbl_1_bss_1E3C[playerIndex].modelId);
        if (lbl_1_bss_1D7C[playerIndex].motionEnded != 0 &&
            lbl_1_bss_1D7C[playerIndex].repeatMotion != 0) {
            lbl_1_bss_1D7C[playerIndex].motionEnded = 0;
            lbl_1_bss_1D7C[playerIndex].repeatMotion = 0;
            lbl_1_bss_1D7C[playerIndex].motionId =
                lbl_1_bss_1D7C[playerIndex].nextMotionId;
            lbl_1_bss_1D7C[playerIndex].motionTime =
                lbl_1_bss_1D7C[playerIndex].nextMotionTime;
            lbl_1_bss_1D7C[playerIndex].modelAttributes =
                lbl_1_bss_1D7C[playerIndex].nextModelAttributes;
            lbl_1_bss_1D7C[playerIndex].attributesToReset =
                lbl_1_bss_1D7C[playerIndex].transitionState;
        }
    }
}

/* Loads camera preset cameraIndex into its record and creates that camera with the
 * preset viewport and perspective, then sets the shared view center, rotation and
 * distance from the preset. */
void fn_1_1790(s16 cameraIndex) {
    f32 presets[4][13] = { { 0.0f, 357.0f, 38.0f, -20.0f, 0.0f, 0.0f, 325.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 24.0f }, { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f }, { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f }, { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f } };
    s16 cameraIds[4] = { 1, 2, 4, 8 };

    lbl_1_bss_3078.records[cameraIndex].cameraIndex = cameraIndex;
    lbl_1_bss_3078.records[cameraIndex].position.x = presets[cameraIndex][0];
    lbl_1_bss_3078.records[cameraIndex].position.y = presets[cameraIndex][1];
    lbl_1_bss_3078.records[cameraIndex].position.z = presets[cameraIndex][2];
    lbl_1_bss_3078.records[cameraIndex].rotation.x = presets[cameraIndex][3];
    lbl_1_bss_3078.records[cameraIndex].rotation.y = presets[cameraIndex][4];
    lbl_1_bss_3078.records[cameraIndex].rotation.z = presets[cameraIndex][5];
    lbl_1_bss_3078.records[cameraIndex].viewport[0] = presets[cameraIndex][6];
    lbl_1_bss_3078.records[cameraIndex].viewport[1] = presets[cameraIndex][7];
    lbl_1_bss_3078.records[cameraIndex].viewport[2] = presets[cameraIndex][8];
    lbl_1_bss_3078.records[cameraIndex].viewport[3] = presets[cameraIndex][9];
    lbl_1_bss_3078.records[cameraIndex].viewport[4] = presets[cameraIndex][10];
    lbl_1_bss_3078.records[cameraIndex].viewport[5] = presets[cameraIndex][11];
    lbl_1_bss_3078.records[cameraIndex].fieldOfView = presets[cameraIndex][12];

    Hu3DCameraCreate(cameraIds[cameraIndex]);
    Hu3DCameraViewportSet(cameraIds[cameraIndex],
                          lbl_1_bss_3078.records[cameraIndex].viewport[2],
                          lbl_1_bss_3078.records[cameraIndex].viewport[3],
                          lbl_1_bss_3078.records[cameraIndex].viewport[4],
                          lbl_1_bss_3078.records[cameraIndex].viewport[5],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds[cameraIndex],
                             lbl_1_bss_3078.records[cameraIndex].fieldOfView,
                             20.0f, 40000.0f,
                             lbl_1_bss_3078.records[cameraIndex].viewport[1]);
    lbl_80266A4C.x = presets[cameraIndex][0];
    lbl_80266A4C.y = presets[cameraIndex][1];
    lbl_80266A4C.z = presets[cameraIndex][2];
    lbl_80266A40.x = presets[cameraIndex][3];
    lbl_80266A40.y = presets[cameraIndex][4];
    lbl_80266A40.z = presets[cameraIndex][5];
    lbl_802C028C = presets[cameraIndex][6];
    fn_1_1C40(cameraIndex);
}

const FourHalfwords lbl_1_rodata_148 = {1, 2, 4, 8};

void fn_1_1C40(s16 cameraIndex)
{
    FourHalfwords values = lbl_1_rodata_148;
}

void fn_1_1C74(void)
{
    fn_800285BC();

    lbl_1_bss_303C.position.x = 0.0f;
    lbl_1_bss_303C.position.y = 0.0f;
    lbl_1_bss_303C.position.z = 0.0f;
    lbl_1_bss_303C.aim.x = 0.0f;
    lbl_1_bss_303C.aim.y = 0.0f;
    lbl_1_bss_303C.aim.z = 0.0f;
    lbl_1_bss_303C.orientation.x = -70.0f;
    lbl_1_bss_303C.orientation.y = 0.0f;
    lbl_1_bss_303C.orientation.z = 0.0f;
    lbl_1_bss_303C.range = 1000.0f;

    lbl_1_bss_303C.color.r = 0xff;
    lbl_1_bss_303C.color.g = 0xff;
    lbl_1_bss_303C.color.b = 0xff;
    lbl_1_bss_303C.color.a = 0xff;

    PSVECSubtract(&lbl_1_bss_303C.aim, &lbl_1_bss_303C.position,
                  &lbl_1_bss_303C.direction);
    lbl_1_bss_303C.id = fn_800286D0(
        &lbl_1_bss_303C.position, &lbl_1_bss_303C.direction,
        &lbl_1_bss_303C.color);
    fn_1_1E48();
    Hu3DGLightStaticSet(lbl_1_bss_303C.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_303C.id);
}

/* Places the directional light on its orbit around the aim point from the orientation
 * angles and range, then updates the light's position, normalized direction and
 * color. */
void fn_1_1E48(void) {
    lbl_1_bss_303C.position.x =
        lbl_1_bss_303C.aim.x +
        lbl_1_bss_303C.range *
            (cos(M_PI * lbl_1_bss_303C.orientation.x /
                 180.0) *
             sin(M_PI * lbl_1_bss_303C.orientation.y /
                 180.0));
    lbl_1_bss_303C.position.y =
        lbl_1_bss_303C.aim.y +
        lbl_1_bss_303C.range *
            -sin(M_PI * lbl_1_bss_303C.orientation.x /
                 180.0);
    lbl_1_bss_303C.position.z =
        lbl_1_bss_303C.aim.z +
        lbl_1_bss_303C.range *
            (cos(M_PI * lbl_1_bss_303C.orientation.x /
                 180.0) *
             cos(M_PI * lbl_1_bss_303C.orientation.y /
                 180.0));
    PSVECSubtract(&lbl_1_bss_303C.aim, &lbl_1_bss_303C.position,
                  &lbl_1_bss_303C.direction);
    PSVECNormalize(&lbl_1_bss_303C.direction, &lbl_1_bss_303C.direction);
    Hu3DGLightPosSetV(lbl_1_bss_303C.id, &lbl_1_bss_303C.position,
                      &lbl_1_bss_303C.direction);
    Hu3DGLightColorSet(lbl_1_bss_303C.id, lbl_1_bss_303C.color.r,
                       lbl_1_bss_303C.color.g, lbl_1_bss_303C.color.b,
                       lbl_1_bss_303C.color.a);
}

void fn_1_20B4(void)
{
    Hu3DShadowCreate(5.0f, 24800.0f, 25200.0f);
    Hu3DShadowSizeSet(0x1E0);
    lbl_1_bss_2FEC.azimuth = -90.0f;
    lbl_1_bss_2FEC.elevation = 0.0f;
    lbl_1_bss_2FEC.unknownAfterElevation = 0.0f;
    lbl_1_bss_2FEC.target.x = 0.0f;
    lbl_1_bss_2FEC.target.y = -100.0f;
    lbl_1_bss_2FEC.target.z = 0.0f;
    lbl_1_bss_2FEC.shadowTransparency = 0.2f;
    lbl_1_bss_2FEC.orbitDistance = 25200.0f;
    fn_1_21C4();
}

void fn_1_21C4(void)
{
    lbl_1_bss_2FEC.cameraPosition.x = (f32)((f64)lbl_1_bss_2FEC.target.x +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
          sin((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0))));
    lbl_1_bss_2FEC.cameraPosition.y = (f32)((f64)lbl_1_bss_2FEC.target.y +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         -sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0)));
    lbl_1_bss_2FEC.cameraPosition.z = (f32)((f64)lbl_1_bss_2FEC.target.z +
        ((f64)lbl_1_bss_2FEC.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
          cos((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0))));

    lbl_1_bss_2FEC.cameraUp.x = (f32)(
        sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
        sin((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0));
    lbl_1_bss_2FEC.cameraUp.y = (f32)cos(
        (M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0);
    lbl_1_bss_2FEC.cameraUp.z = (f32)(
        sin((M_PI * (f64)lbl_1_bss_2FEC.azimuth) / 180.0) *
        cos((M_PI * (f64)lbl_1_bss_2FEC.elevation) / 180.0));

    Hu3DShadowTPLvlSet(lbl_1_bss_2FEC.shadowTransparency);
    Hu3DShadowPosSet(&lbl_1_bss_2FEC.cameraPosition, &lbl_1_bss_2FEC.cameraUp,
                     &lbl_1_bss_2FEC.target);
}

void fn_1_24E8(void)
{
    Vec playerPosition;
    s16 playerIndex;
    FourHalfwords values;

    fontcolor = 0xF;
    playerIndex = 0;
    while (playerIndex < 2) {
        playerPosition.x = lbl_1_bss_1C04[playerIndex].startPosition.x;
        playerPosition.y = lbl_1_bss_1C04[playerIndex].startPosition.y;
        playerPosition.z = lbl_1_bss_1C04[playerIndex].startPosition.z;
        fn_80086B04(
            lbl_1_bss_1960.players[playerIndex]->actor,
            &playerPosition);
        lbl_1_bss_1960.players[playerIndex]->actor->rotY =
            lbl_1_bss_1C04[playerIndex].endPosition.y;
        lbl_1_bss_1E3C[playerIndex].position = lbl_1_bss_1C04[playerIndex].startPosition;
        lbl_1_bss_1E3C[playerIndex].rotation = lbl_1_bss_1C04[playerIndex].endPosition;
        playerIndex++;
    }

    updatePlayerModels();

    fn_1_F98();
    values = lbl_1_rodata_148;
    updateShadowCamera();
}

/* Feeds each active player's actor its input (zeroed in choice mode, the stored stick
 * and buttons for computer players), saves the previous position, value and pairing
 * and clears the current ones, resetting the contact count when the pairing changed.
 * Then updates all actors and copies each actor's position and facing back. */
void fn_1_2B58(void) {
    Point3d position;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        if (lbl_1_bss_1C04[playerIndex].playerStatus[0] != 0) {
            if (lbl_1_bss_1C04[playerIndex].choiceMode != 0) {
                fn_80080520(lbl_1_bss_1960.players[playerIndex], 0, 0, 0, 0);
            } else if (lbl_1_bss_1C04[playerIndex].playerType != 0) {
                fn_80080520(lbl_1_bss_1960.players[playerIndex], lbl_1_bss_1C04[playerIndex].stickX, lbl_1_bss_1C04[playerIndex].stickY, lbl_1_bss_1C04[playerIndex].padY, lbl_1_bss_1C04[playerIndex].padX);
            }
            fn_80086BC8(lbl_1_bss_1960.players[playerIndex]->actor, &position);
            lbl_1_bss_1C04[playerIndex].previousPosition.x = position.x;
            lbl_1_bss_1C04[playerIndex].previousPosition.y = position.y;
            lbl_1_bss_1C04[playerIndex].previousPosition.z = position.z;
            lbl_1_bss_1C04[playerIndex].previousValue = lbl_1_bss_1C04[playerIndex].currentValue;
            lbl_1_bss_1C04[playerIndex].currentValue = -1;
            if (lbl_1_bss_1C04[playerIndex].previousPairedIndex != lbl_1_bss_1C04[playerIndex].pairedIndex || lbl_1_bss_1C04[playerIndex].pairedIndex == -1) {
                lbl_1_bss_1C04[playerIndex].updateCount = 0;
            }
            lbl_1_bss_1C04[playerIndex].previousPairedIndex = lbl_1_bss_1C04[playerIndex].pairedIndex;
            lbl_1_bss_1C04[playerIndex].pairedIndex = -1;
        }
        playerIndex += 1;
    }
    fn_800856A8();
    playerIndex = 0;
    while (playerIndex < 2) {
        if (lbl_1_bss_1C04[playerIndex].playerStatus[0] != 0) {
            fn_80086BC8(lbl_1_bss_1960.players[playerIndex]->actor, &position);
            lbl_1_bss_1C04[playerIndex].startPosition.x = position.x;
            lbl_1_bss_1C04[playerIndex].startPosition.y = position.y;
            lbl_1_bss_1C04[playerIndex].startPosition.z = position.z;
            lbl_1_bss_1C04[playerIndex].endPosition.y = lbl_1_bss_1960.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

s32 fn_1_2EA4(f32 *value, f32 step, f32 start, f32 end)
{
    f32 startDistance;
    f32 currentDistance;
    s32 frames = 0;
    f32 increment = (end - start) / step;

    *value += increment;
    if (increment) {
        frames = (s32)((end - *value) / increment);
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

MGTIMER *fn_1_2FC4(int type, int duration, int end, float x, float y)
{
    MGTIMER *timer = MgTimerCreate(type);
    MgTimerParamSet(timer, duration * 60, end * 60, 0);
    MgTimerPosSet(timer, x, y);
    return timer;
}

void fn_1_3038(MGTIMER *timer)
{
    MgTimerModeOnSet(timer, 1);
}

int fn_1_3064(MGTIMER *timer)
{
    return MgTimerDoneCheck(timer);
}

void fn_1_308C(MGTIMER *timer)
{
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

void fn_1_30C4(s16 player, s16 minimum, s16 maximum)
{
    s16 index;

    if (lbl_802BFFEC[player] > 0x32) {
        if (lbl_802C0008[player] & 0x400) {
            lbl_1_bss_196C.debugSelection++;
        }
        if (lbl_802C0008[player] & 0x800) {
            lbl_1_bss_196C.debugSelection--;
        }
    }
    if (lbl_1_bss_196C.debugSelection < minimum) {
        lbl_1_bss_196C.debugSelection = maximum;
    }
    if (lbl_1_bss_196C.debugSelection > maximum) {
        lbl_1_bss_196C.debugSelection = minimum;
    }
    fn_1_335C(player, lbl_1_bss_196C.debugSelection);

    if ((lbl_802C0008[player] & 0x400) && (lbl_802C0008[player] & 0x800)) {
        for (index = minimum; index <= maximum; index++) {
            if (index >= 0 && index <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n", index,
                       lbl_1_bss_1C04[index].startPosition.x,
                       lbl_1_bss_1C04[index].startPosition.y,
                       lbl_1_bss_1C04[index].startPosition.z,
                       lbl_1_bss_1C04[index].endPosition.y);
            } else {
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n", index,
                       lbl_1_bss_1E3C[index].position.x,
                       lbl_1_bss_1E3C[index].position.y, lbl_1_bss_1E3C[index].position.z,
                       lbl_1_bss_1E3C[index].rotation.y);
            }
        }
    }
}

void fn_1_335C(s16 player, s16 selection)
{
    if (selection >= 0 && selection <= 1) {
        if ((lbl_802C0000[player] & 0x400) != 0) {
            if (lbl_802BFFFC[player] > 10) {
                lbl_1_bss_1C04[selection].startPosition.x += 5.0f;
            }
            if (lbl_802BFFFC[player] < -10) {
                lbl_1_bss_1C04[selection].startPosition.x -= 5.0f;
            }
            if (lbl_802BFFF8[player] > 10) {
                lbl_1_bss_1C04[selection].startPosition.z -= 5.0f;
            }
            if (lbl_802BFFF8[player] < -10) {
                lbl_1_bss_1C04[selection].startPosition.z += 5.0f;
            }
        } else if ((lbl_802C0008[player] & 0x400) != 0) {
            if (lbl_802BFFFC[player] > 10) {
                lbl_1_bss_1C04[selection].startPosition.x += 5.0f;
            }
            if (lbl_802BFFFC[player] < -10) {
                lbl_1_bss_1C04[selection].startPosition.x -= 5.0f;
            }
            if (lbl_802BFFF8[player] > 10) {
                lbl_1_bss_1C04[selection].startPosition.z -= 5.0f;
            }
            if (lbl_802BFFF8[player] < -10) {
                lbl_1_bss_1C04[selection].startPosition.z += 5.0f;
            }
        }
        if ((lbl_802C0008[player] & 0x100) != 0) {
            lbl_1_bss_1C04[selection].endPosition.y += 90.0f;
        }
        if ((lbl_802C0008[player] & 0x200) != 0) {
            lbl_1_bss_1C04[selection].endPosition.y -= 90.0f;
        }
        print8(0x19, 0x9C, 1.5f, "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f", selection,
               lbl_1_bss_1C04[selection].startPosition.x,
               lbl_1_bss_1C04[selection].startPosition.y,
               lbl_1_bss_1C04[selection].startPosition.z,
               lbl_1_bss_1C04[selection].endPosition.y);
        return;
    }

    {
        if ((lbl_802C0000[player] & 0x400) != 0) {
            if (lbl_802BFFFC[player] > 10) {
                lbl_1_bss_1E3C[selection].position.x += 5.0f;
            }
            if (lbl_802BFFFC[player] < -10) {
                lbl_1_bss_1E3C[selection].position.x -= 5.0f;
            }
            if (lbl_802BFFF8[player] > 10) {
                lbl_1_bss_1E3C[selection].position.z -= 5.0f;
            }
            if (lbl_802BFFF8[player] < -10) {
                lbl_1_bss_1E3C[selection].position.z += 5.0f;
            }
        } else if ((lbl_802C0008[player] & 0x400) != 0) {
            if (lbl_802BFFFC[player] > 10) {
                lbl_1_bss_1E3C[selection].position.x += 5.0f;
            }
            if (lbl_802BFFFC[player] < -10) {
                lbl_1_bss_1E3C[selection].position.x -= 5.0f;
            }
            if (lbl_802BFFF8[player] > 10) {
                lbl_1_bss_1E3C[selection].position.z -= 5.0f;
            }
            if (lbl_802BFFF8[player] < -10) {
                lbl_1_bss_1E3C[selection].position.z += 5.0f;
            }
        }
        if ((lbl_802C0008[player] & 0x100) != 0) {
            lbl_1_bss_1E3C[selection].rotation.y += 90.0f;
        }
        if ((lbl_802C0008[player] & 0x200) != 0) {
            lbl_1_bss_1E3C[selection].rotation.y -= 90.0f;
        }
        print8(0x19, 0x9C, 1.5f, "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f", selection,
               lbl_1_bss_1E3C[selection].position.x,
               lbl_1_bss_1E3C[selection].position.y,
               lbl_1_bss_1E3C[selection].position.z,
               lbl_1_bss_1E3C[selection].rotation.y);
    }
}

s16 fn_1_3B54(s16 modelIndex, s16 lightId)
{
    s16 result = 0;
    s16 index;
    HU3D_MODEL *model = &lbl_802C01C8[modelIndex];

    for (index = 0; index < 8; index++) {
        if (model->LLightId[index] == lightId) {
            result = -1;
            break;
        }
    }
    if (result != -1) {
        for (index = 0; index < 8; index++) {
            if (model->LLightId[index] == -1) {
                model->LLightId[index] = lightId;
                model->attr |= 0x1000;
                result = 1;
                break;
            }
        }
    }
    if (result == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", modelIndex, lightId);
        result = -1;
    }
    return result;
}

/* Removes lightId from the model's local light list; returns 1 if it was found,
 * otherwise -1. */
s32 fn_1_3C6C(HU3D_MODELID modelId, HU3D_LIGHTID lightId) {
    HU3D_MODEL *model;
    s32 result;
    s16 index;

    result = -1;
    model = &lbl_802C01C8[modelId];
    index = 0;
    while (index < 8) {
        if (model->LLightId[index] == lightId) {
            model->LLightId[index] = -1;
            result = 1;
            break;
        }
        index++;
    }
    return result;
}

f32 fn_1_3D04(f32 maximum)
{
    s32 result = maximum;
    result = frandmod(result);
    maximum = result;
    return maximum;
}

void fn_1_3D7C(void)
{
    u32 animationFiles[1] = {0x780027};
    s16 particleCounts[1] = {50};
    s16 layers[1] = {5};
    s16 cameras[1] = {1};
    s16 blendModes[1] = {1};
    s16 effectIndex;

    for (effectIndex = 0; effectIndex < 1; effectIndex++) {
        ANIMDATA *animation;
        HU3D_PARTICLE *particle;
        HU3D_PARTICLE_DATA *data;
        s16 particleIndex;

        lbl_1_bss_2FE4.particleCounts[effectIndex] = particleCounts[effectIndex];
        animation = HuSprAnimRead(fn_80008DD0(
            animationFiles[effectIndex], 0x10000000, 2));
        lbl_1_bss_2FE4.work[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, particleCounts[effectIndex] * 0x3C, 0x10000000);
        lbl_1_bss_2FE4.modelIds[effectIndex] = fn_8003216C(
            animation, particleCounts[effectIndex]);
        fn_80026B8C(lbl_1_bss_2FE4.modelIds[effectIndex], layers[effectIndex]);
        fn_800326D0(lbl_1_bss_2FE4.modelIds[effectIndex], fn_1_44F8);
        fn_80026B64(lbl_1_bss_2FE4.modelIds[effectIndex], cameras[effectIndex]);
        fn_8003269C(lbl_1_bss_2FE4.modelIds[effectIndex],
                                 (u8)blendModes[effectIndex]);

        particle = lbl_802C01C8[lbl_1_bss_2FE4.modelIds[effectIndex]].hookData;
        particle->emitCnt = 0;
        particle->work = lbl_1_bss_2FE4.work[effectIndex];
        particle->count = 0;
        particleIndex = 0;
        data = particle->data;
        for (; particleIndex < particle->maxCnt; particleIndex++, data++) {
            data->scale = 0.0f;
            data->time = 0;
        }
    }
}

void fn_1_400C(HuVecF *position)
{
    HuVecF particleOffset;
    HuVecF sourcePosition;
    HuVecF particlePosition;
    HuVecF initialVelocity;
    ParticleSpawnWork initialWork;
    s32 horizontalRandom;
    s32 verticalRandom;
    f32 randomHorizontal;
    f32 randomVertical;
    f32 scale;
    f32 centeredRandomValue;
    f32 horizontalVelocity;
    f32 baseHeight;
    f32 verticalVelocity;
    HuVecF *velocityOut;
    HuVecF *positionOut;

    initialWork.flags = 0;
    initialWork.startColor.r = 0xFF;
    initialWork.startColor.g = 0xFF;
    initialWork.startColor.b = 0xFF;
    initialWork.startColor.a = 0xFF;
    initialWork.endColor.r = 0;
    initialWork.endColor.g = 0;
    initialWork.endColor.b = 0;
    initialWork.endColor.a = 0;
    initialWork.motion[4] = 0.0f;
    initialWork.motion[7] = 0.01f;
    initialWork.motion[8] = 0.01f;
    initialWork.motion[3] = 0.0f;
    sourcePosition.x = position->x;
    sourcePosition.y = position->y;
    sourcePosition.z = position->z;
    scale = 30.0f;
    initialWork.motion[5] = -0.1f;
    initialWork.motion[6] = 1.2f;
    initialWork.motion[0] = 1.05f;
    initialWork.motion[1] = 0.95f;
    initialWork.motion[2] = 1.0f;
    randomHorizontal = 3.0f;
    horizontalRandom = (s32)randomHorizontal;
    horizontalRandom = frandmod(horizontalRandom);
    randomHorizontal = (f32)horizontalRandom;
    centeredRandomValue = randomHorizontal;
    horizontalVelocity = centeredRandomValue;
    particleOffset.x = (horizontalVelocity - 1.0f) * 0.4f;
    randomVertical = 3.0f;
    verticalRandom = (s32)randomVertical;
    verticalRandom = frandmod(verticalRandom);
    randomVertical = (f32)verticalRandom;
    baseHeight = randomVertical;
    verticalVelocity = baseHeight;
    particleOffset.y = 5.0f + verticalVelocity;
    particleOffset.z = 0.0f;
    initialVelocity = particleOffset;
    velocityOut = &initialVelocity;
    particlePosition = sourcePosition;
    positionOut = &particlePosition;
    fn_1_42E8(0, -1, positionOut, velocityOut, scale, 0x14, &initialWork);
}

s16 fn_1_42E8(s16 modelIndex, s16 parameter, HuVecF *sourceB,
              HuVecF *sourceC, f32 opacity, s16 time,
              ParticleSpawnWork *copyData)
{
    HU3D_MODEL *model;
    s16 result;
    HU3D_PARTICLE *context;
    ParticleSpawnWork *entryData;
    s16 entryIndex;
    HU3D_PARTICLE_DATA *entry;

    result = -1;
    model = &lbl_802C01C8[lbl_1_bss_2FE4.modelIds[modelIndex]];
    context = model->hookData;
    entryData = context->work;
    entryIndex = 0;
    entry = context->data;

    while (entryIndex < context->maxCnt) {
        if (entry->scale <= 0.0f) {
            entryData[entryIndex] = *copyData;
            entryData[entryIndex].position.z = entryData[entryIndex].position.y =
                entryData[entryIndex].position.x = 0.0f;

            entry->pos = *sourceB;
            entry->vel = *sourceC;
            entry->accel = *sourceB;
            entry->scaleBase = entry->colorIdx = entry->speedDecay = 0.0f;

            entry->color = entryData[entryIndex].startColor;
            entry->scale = opacity;
            entry->time = time;
            entry->parManId = parameter;
            context->emitCnt = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

void fn_1_44F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    ParticleSpawnWork *parameters = particle->work;
    HU3D_PARTICLE_DATA *entry = particle->data;
    s16 entryNo = 0;

    while (entryNo < particle->maxCnt) {
        if (entry->time != 0) {
            s16 channelValue;

            entry->vel.x *= parameters[entryNo].motion[0];
            entry->vel.y *= parameters[entryNo].motion[1];
            entry->vel.z *= parameters[entryNo].motion[2];
            PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
            entry->vel.y += parameters[entryNo].motion[3];
            entry->zRot += parameters[entryNo].motion[4];

            channelValue = (s16)((f32)entry->color.r +
                                 (parameters[entryNo].motion[8] *
                                  (f32)(parameters[entryNo].endColor.r -
                                        parameters[entryNo].startColor.r)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 0xFF) {
                channelValue = 0xFF;
            }
            entry->color.r = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.g +
                                 (parameters[entryNo].motion[8] *
                                  (f32)(parameters[entryNo].endColor.g -
                                        parameters[entryNo].startColor.g)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 0xFF) {
                channelValue = 0xFF;
            }
            entry->color.g = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.b +
                                 (parameters[entryNo].motion[8] *
                                  (f32)(parameters[entryNo].endColor.b -
                                        parameters[entryNo].startColor.b)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 0xFF) {
                channelValue = 0xFF;
            }
            entry->color.b = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.a +
                                 parameters[entryNo].motion[7]);
            if (channelValue < 0) {
                entry->scale = 0.0f;
            }
            entry->color.a = (u8)channelValue;

            parameters[entryNo].motion[5] *= parameters[entryNo].motion[6];
            entry->scale += parameters[entryNo].motion[5];
            if (--entry->time < 1) {
                entry->scale = 0.0f;
                entry->time = 0;
            }
            DCStoreRangeNoSync(particle->data, particle->maxCnt * 0x48);
        }
        entryNo++;
        entry++;
    }
}

/* The sound effects of the three sound slots. */
s16 lbl_1_data_F6[3] = { MSM_SE_MG661_INTRO_BOO_START, MSM_SE_MG661_INTRO_BOO_MOVE,
                         MSM_SE_MG661_BOO_EXIT_SCREEN };

/* Module work, cleared by fn_1_118. */
ProcessStorage lbl_1_bss_31C0;
CameraRecordStorage lbl_1_bss_3078;
LightInitState lbl_1_bss_303C;
ShadowCameraState lbl_1_bss_2FEC;
ParticleSettings lbl_1_bss_2FE4;
ModelSetupRecord lbl_1_bss_1E3C[0x71];
ModelAnimationWork lbl_1_bss_1D7C[2];
PlayerRoundState lbl_1_bss_1C04[2];
SystemInputState lbl_1_bss_196C;
PlayerActors lbl_1_bss_1960;
LightConfig lbl_1_bss_1870[3];
MotionControl lbl_1_bss_10[0x4E];
ScoreDisplay lbl_1_bss_0[2];

extern s32 lbl_1_bss_210;

extern s16 lbl_1_bss_31C8[4];

void fn_1_58DC(void);

void fn_1_695C(void);

s32 fn_1_5954(void);

typedef struct SetupValues {
    HuVecF positions[2];
} SetupValues;

extern void fn_1_20D74(void);

extern void fn_1_59F0(void);

extern void fn_1_5A7C(void);

extern void fn_1_67DC(void);

void fn_1_4918(PlayerRoundState *object, s16 mode)
{
    s16 pan;
    s16 volume;
    f32 positionDelta;
    f32 panScale;
    f32 volumeScale;

    if (mode == 0 || mode == 1 || mode == 6) {
        if (lbl_1_bss_196C.panPositionMax >= object->startPosition.x &&
            lbl_1_bss_196C.panPositionMin <= object->startPosition.x) {
            panScale = (f32)(lbl_1_bss_196C.panMax -
                             lbl_1_bss_196C.panMin) /
                       (lbl_1_bss_196C.panPositionMax -
                        lbl_1_bss_196C.panPositionMin);
            positionDelta = object->startPosition.x -
                            lbl_1_bss_196C.panPositionMin;
            positionDelta *= panScale;
            pan = positionDelta;
            pan += lbl_1_bss_196C.panMin;
        } else if (lbl_1_bss_196C.panPositionMax < object->startPosition.x) {
            pan = lbl_1_bss_196C.panMax;
        } else if (lbl_1_bss_196C.panPositionMin > object->startPosition.x) {
            pan = lbl_1_bss_196C.panMin;
        }
    }

    if (mode == 0 || mode == 2) {
        if (lbl_1_bss_196C.volumePositionMax >= object->startPosition.z &&
            lbl_1_bss_196C.volumePositionMin <= object->startPosition.z) {
            volumeScale = (f32)(lbl_1_bss_196C.volumeMax -
                                lbl_1_bss_196C.volumeMin) /
                          (lbl_1_bss_196C.volumePositionMax -
                           lbl_1_bss_196C.volumePositionMin);
            positionDelta = object->startPosition.z -
                            lbl_1_bss_196C.volumePositionMin;
            positionDelta *= volumeScale;
            volume = positionDelta;
            volume += lbl_1_bss_196C.volumeMin;
        } else if (lbl_1_bss_196C.volumePositionMax < object->startPosition.z) {
            volume = lbl_1_bss_196C.volumeMax;
        } else if (lbl_1_bss_196C.volumePositionMin > object->startPosition.z) {
            volume = lbl_1_bss_196C.volumeMin;
        }
    }

    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
        pan = (lbl_1_bss_196C.panMin +
                    (lbl_1_bss_196C.panMax -
                     lbl_1_bss_196C.panMin) / 2);
    }

    if (mode == 1 || mode == 3) {
        volume = (lbl_1_bss_196C.volumeMin +
                       (lbl_1_bss_196C.volumeMax -
                        lbl_1_bss_196C.volumeMin) / 2);
    }

    if (mode == 4) {
        volume = lbl_1_bss_196C.volumeMin;
    }

    if (mode == 5 || mode == 6) {
        volume = lbl_1_bss_196C.volumeMax;
    }

    fn_80060F14(object->character, pan);
    fn_80060F40(object->character, volume);
}

/* Sets the default sound state: no music or jingle playing, music 0x55, pan 32-96 and
 * volume 32-110 mapped over positions -800 to 800, and three idle effect slots. */
void fn_1_4D1C(void) {
    s16 index;

    lbl_1_bss_196C.bgmHandle = -1;
    lbl_1_bss_196C.jingleHandle = -1;
    lbl_1_bss_196C.bgmId = MSM_STREAM_NIGHT_DUEL;
    lbl_1_bss_196C.jingleId = -1;
    lbl_1_bss_196C.panMin = 0x20;
    lbl_1_bss_196C.panMax = 0x60;
    lbl_1_bss_196C.panPositionMin = -800.0f;
    lbl_1_bss_196C.panPositionMax = 800.0f;
    lbl_1_bss_196C.volumeMin = 0x20;
    lbl_1_bss_196C.volumeMax = 0x6E;
    lbl_1_bss_196C.volumePositionMin = -800.0f;
    lbl_1_bss_196C.volumePositionMax = 800.0f;
    index = 0;
    while (index < 3) {
        lbl_1_bss_196C.soundActive[index] = -1;
        lbl_1_bss_196C.soundHandles[index] = -1;
        index += 1;
    }
}

void fn_1_4E60(void) {
    if (lbl_1_bss_196C.jingleId != -1 && lbl_1_bss_196C.jingleHandle == -1) {
        lbl_1_bss_196C.jingleHandle = HuAudJinglePlay((s16)lbl_1_bss_196C.jingleId);
    }
}

void fn_1_4EC4(void) {
    if (lbl_1_bss_196C.bgmHandle == -1) {
        lbl_1_bss_196C.bgmHandle = HuAudBGMPlay((s16)lbl_1_bss_196C.bgmId);
    }
}

/* Fades out the music stream (streamSlot nonzero) or the jingle stream over
 * fadeFrames if it is playing, and clears its handle. */
void fn_1_4F14(s16 streamSlot, s16 fadeFrames) {
    s16 streamId = streamSlot != 0
                       ? lbl_1_bss_196C.bgmHandle
                       : lbl_1_bss_196C.jingleHandle;
    if (streamId != -1) {
        HuAudSStreamFadeOut(streamId, fadeFrames);
        if (streamSlot != 0) {
            lbl_1_bss_196C.bgmHandle = -1;
            return;
        }
        lbl_1_bss_196C.jingleHandle = -1;
    }
}

void fn_1_4FC0(s16 usePrimaryStream)
{
    s16 streamNumber;

    if (usePrimaryStream != 0) {
        streamNumber = (s16)lbl_1_bss_196C.bgmHandle;
    } else {
        streamNumber = (s16)lbl_1_bss_196C.jingleHandle;
    }
    HuAudSStreamStop(streamNumber);
}

void fn_1_5020(s16 slot, s16 mode, VoicePosition *position)
{
    s16 pan;
    s16 volume;
    f32 positionDelta;
    f32 panScale;
    f32 volumeScale;

    if (mode <= 5) {
    if (mode == 0 || mode == 1) {
        if (lbl_1_bss_196C.panPositionMax >= position->panPosition &&
            lbl_1_bss_196C.panPositionMin <= position->panPosition) {
            panScale = (f32)(lbl_1_bss_196C.panMax -
                             lbl_1_bss_196C.panMin) /
                       (lbl_1_bss_196C.panPositionMax -
                        lbl_1_bss_196C.panPositionMin);
            positionDelta = position->panPosition -
                            lbl_1_bss_196C.panPositionMin;
            positionDelta *= panScale;
            pan = positionDelta;
            pan += lbl_1_bss_196C.panMin;
        } else if (lbl_1_bss_196C.panPositionMax < position->panPosition) {
            pan = lbl_1_bss_196C.panMax;
        } else if (lbl_1_bss_196C.panPositionMin > position->panPosition) {
            pan = lbl_1_bss_196C.panMin;
        }
    }

    if (mode == 0 || mode == 2) {
        if (lbl_1_bss_196C.volumePositionMax >= position->volumePosition &&
            lbl_1_bss_196C.volumePositionMin <= position->volumePosition) {
            volumeScale = (f32)(lbl_1_bss_196C.volumeMax -
                                lbl_1_bss_196C.volumeMin) /
                          (lbl_1_bss_196C.volumePositionMax -
                           lbl_1_bss_196C.volumePositionMin);
            positionDelta = position->volumePosition -
                            lbl_1_bss_196C.volumePositionMin;
            positionDelta *= volumeScale;
            volume = positionDelta;
            volume += lbl_1_bss_196C.volumeMin;
        } else if (lbl_1_bss_196C.volumePositionMax < position->volumePosition) {
            volume = lbl_1_bss_196C.volumeMax;
        } else if (lbl_1_bss_196C.volumePositionMin > position->volumePosition) {
            volume = lbl_1_bss_196C.volumeMin;
        }
    }

    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
        pan = (lbl_1_bss_196C.panMin +
                    (lbl_1_bss_196C.panMax -
                     lbl_1_bss_196C.panMin) / 2);
    }

    if (mode == 1 || mode == 3) {
        volume = (lbl_1_bss_196C.volumeMin +
                       (lbl_1_bss_196C.volumeMax -
                        lbl_1_bss_196C.volumeMin) / 2);
    }

    if (mode == 4) {
        volume = lbl_1_bss_196C.volumeMin;
    }

    if (mode == 5) {
        volume = lbl_1_bss_196C.volumeMax;
    }
    lbl_1_bss_196C.soundActive[slot] = 1;
    lbl_1_bss_196C.soundHandles[slot] =
        HuAudFXPlayVolPan(lbl_1_data_F6[slot], volume, pan);
    }
}
