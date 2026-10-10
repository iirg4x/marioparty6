/* Contains the module's shared type declarations. */
#ifndef REL_M649DLL_MODULE_TYPES_H
#define REL_M649DLL_MODULE_TYPES_H

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
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/window.h"
#include "game/printfunc.h"
#include "string.h"
#include "stdio.h"

/* Shared camera-light state used for the shadow direction, placement, and color. */
typedef struct M620LightData_s {
    HU3D_LIGHTID lightId; /* Renderer light handle; -1 before the light is created. */
    OMOBJ_FUNC callback; /* Optional object update callback for the light owner. */
    HuVecF position; /* World position used to place the directional light. */
    HuVecF target; /* World point used to derive the light direction at setup. */
    HuVecF direction; /* Stored light direction shared with the camera shadow state. */
    GXColor color; /* RGBA color sent to the renderer. */
} M620LightData;

/* Stores per-object context and the optional frame callback used by the sound manager. */
typedef void (*M620ObjectCallback)(OMOBJ *obj);

typedef struct M620ObjectState_s {
    void *userData; /* Client-owned data retained by the sound manager object. */
    M620ObjectCallback callback; /* Optional per-frame callback after sound updates. */
} M620ObjectState;

/* A movement sample stores its starting point, displacement, and calculated point. */
typedef struct M620VectorRecord_s {
    Point3d origin; /* Starting point of this particle's movement sample. */
    Point3d delta; /* Per-step displacement applied to the starting point. */
    Point3d result; /* Calculated position stored for this sample. */
} M620VectorRecord;

/* Groups particle movement settings with the batch's 32 position samples. */
typedef struct M620ParticleBatch_s {
    u32 source; /* Source particle or object index selected by the caller. */
    u32 target; /* Destination particle or object index selected by the caller. */
    u32 resource; /* Data resource used to create the particle batch. */
    f32 step; /* Movement increment used to calculate each position sample. */
    s32 active; /* Nonzero while this batch is being updated. */
    M620VectorRecord records[32]; /* Position samples used by the particle movement. */
} M620ParticleBatch;

/* Camera views, perspective, and split-screen rectangles used by the frame callback. */
typedef struct M649CameraEntry_s {
    s32 active, motion, model;
    OM_CAMERA_VIEW view, previousView;
    Vec position, up, target;
    float fov, nearPlane, farPlane, aspect;
    float viewportX, viewportY, viewportWidth, viewportHeight;
    float scissorX, scissorY, scissorWidth, scissorHeight;
    s32 phase, timer;
    /* Cleared during camera setup; no camera callback reads these bytes. */
    u8 unknown_A0_to_B7[24];
} M649CameraEntry;

/* The camera object stores sixteen views and their shared shadow settings. */
typedef struct M649CameraWork_s {
    u32 cameras;
    float shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    void (*callback)(OMOBJ *, unsigned int, int);
    M649CameraEntry entries[16];
    s32 shadowInitialized;
    /* Cleared during camera setup; no camera callback reads these bytes. */
    u8 unknown_BA4_to_BA7[4];
} M649CameraWork;

/* Character height and motion selections copied into every contestant's state at setup. */
typedef struct M649PlayerConfig_s {
    float unknown00; /* Copied at setup; no module code reads this value. */
    float height; /* Replaced at setup with the selected character's model height. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unknown_08_to_0B[4];
    /* Idle and movement motions used while turning toward the front after play. */
    s32 finishIdleMotion, moveMotion;
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unknown_14_to_1F[12];
    s32 idleMotion; /* Motion selected while waiting for a turn. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unknown_24_to_33[16];
    s32 turnMotion, winMotion, loseMotion; /* Turn and results motion selections. */
} M649PlayerConfig;

#endif
