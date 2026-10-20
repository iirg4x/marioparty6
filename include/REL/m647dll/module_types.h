/* Shared minigame lifecycle, player, activity and particle state. */
#ifndef REL_M647DLL_MODULE_TYPES_H
#define REL_M647DLL_MODULE_TYPES_H

#ifndef _MATH_H
#include "dolphin/math.h"
#endif
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
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
#include "game/window.h"

typedef struct M647AnimState_s {
    HU3D_MODELID backgroundModelId;
    HU3D_MODELID modelId;
    HU3D_MODELID openingModelId;
    HU3D_MODELID effectModelId;
    s16 motionModelId;
    HU3D_MODELID entranceModelId;
    HU3D_MODELID gameplayModelId;
    HU3D_MODELID shadowModelId;
    HU3D_MODELID modelIds[4];
    HU3D_ANIMID animId;
    s16 playerTurn;
    HUWINID windowId;
    s16 sequencePhase;
    s16 activityPattern;
    s16 nightMode;
    s16 finishPhaseActive;
    s16 obstacleActive;
    s16 phaseFrame;
    s32 musicHandle;
    s32 soundEffectId;
} M647AnimState;

typedef struct M647PlayerRecord_s {
    HU3D_MODELID modelId;
    HU3D_MOTIONID motionIds[9];
    s16 playerIndex;
    s16 occupancyMarker; /* Zero-valued occupancy marker copied to a claimed slot, never a player
                         * identity. */
    s16 characterId;
    s16 padIndex;
    s16 cpuDifficulty;
    s16 inputPhase;
    s16 timer;
    f32 targetX;
    f32 positionX;
    f32 rotationY;
    MGTIMER *timerId;
    void (*callback)(s16 index);
    s16 selectedSlot;
    s16 activityIndex;
    s16 cameraShifted;
    s16 motionState;
    s16 selectionCount;
    s16 finishPlace;
    s16 frame;
    s16 mountingPhase;
} M647PlayerRecord;

typedef union M647ActivityState_u {
    struct {
        u16 buttonOrder[8];
        s16 row;
        s16 frame;
        s16 inputDelay;
        HU3D_MODELID itemModelId;
    } grid;
    struct {
        HU3D_MODELID rightModelId;
        HU3D_MODELID frontModelId;
        s16 inputDelay;
        s16 phase;
        HU3D_MODELID leftModelIds[2];
        s16 counter;
        s16 frame;
        f32 savedMotionTime;
        f32 speed;
        Point3d hookPosition;
        Point3d previousHookPosition;
    } lever;
    struct {
        u16 buttonIndex;
        s16 frame;
        s16 inputDelay;
        HU3D_MODELID itemModelId;
    } tap;
    struct {
        u16 buttonIndex;
        s16 successCount;
        s16 frame;
        s16 inputDelay;
        HU3D_MODELID itemModelId;
        s16 blinkFrame;
        s16 blinkVisible;
    } row;
} M647ActivityState;

typedef struct M647ActivityEntry_s {
    HU3D_MODELID modelId;
    HU3D_MOTIONID motionIds[3];
    Point3d position;
    u8 unknown14[4]; /* Cleared with the activity table; no activity reads these bytes. */
    M647ActivityState activity;
    HUSPR_GROUPID groupId;
    s16 status;
} M647ActivityEntry;

typedef struct M647ActivityTable_s {
    M647ActivityEntry entries[4];
} M647ActivityTable;

typedef struct M647ModeState_s {
    s16 mode;                   /* +0x00 */
    s16 argument;               /* +0x02 */
} M647ModeState;

typedef struct M647ModelPair_s {
    HU3D_MODELID model0;        /* +0x00 */
    HU3D_MODELID model1;        /* +0x02 */
    f32 floorHeight;
    s32 particleCount;
} M647ModelPair;

typedef struct M647ParticleRecord_s {
    s32 active;
    s32 landed;
    Point3d position;
    Point3d velocity;
    Point3d rotation;
    Point3d angularVelocity;
    GXColor color;
    s32 landedFrames;
} M647ParticleRecord;

typedef struct M647CameraSequence_s {
    OM_CAMERA_VIEW view;
    s16 frame;
    s16 phase;
} M647CameraSequence;

extern OMOBJ *lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_4;
extern ANIMDATA *lbl_1_bss_8;
extern f32 lbl_1_bss_C;
extern s32 lbl_1_bss_10;
extern M647ModeState lbl_1_bss_14;
extern s16 lbl_1_bss_18;
extern s16 lbl_1_bss_1A[4];
extern s16 lbl_1_bss_22[5];
extern f32 lbl_1_bss_2C[2];
extern M647CameraSequence lbl_1_bss_34;
extern M647PlayerRecord lbl_1_bss_54[4];
extern M647AnimState lbl_1_bss_174;
extern ANIMDATA *lbl_1_bss_1A8;
extern ANIMDATA *lbl_1_bss_1AC[4];
extern ANIMDATA *lbl_1_bss_1BC;
extern ANIMDATA *lbl_1_bss_1C0[6];
extern ANIMDATA *lbl_1_bss_1D8[6];
extern ANIMDATA *lbl_1_bss_1F0[6];
extern ANIMDATA *lbl_1_bss_208[3];
extern ANIMDATA *lbl_1_bss_214[4];
extern M647ActivityTable lbl_1_bss_224;
extern M647ModelPair lbl_1_bss_358;
extern M647ParticleRecord lbl_1_bss_364[50];

#endif
