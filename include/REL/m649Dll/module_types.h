/* Contains the module's shared type declarations. */
#ifndef REL_M649DLL_MODULE_TYPES_H
#define REL_M649DLL_MODULE_TYPES_H
#include "REL/minigame_camera_work.h"
#include "REL/minigame_prop_params.h"
#include "REL/minigame_prop_motion.h"

#include "REL/minigame_light.h"
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

/* Stores per-object context and the optional frame callback used by the sound manager. */
typedef void (*M620ObjectCallback)(OMOBJ *obj);

typedef struct {
    void *userData; /* Client-owned data retained by the sound manager object. */
    M620ObjectCallback callback; /* Optional per-frame callback after sound updates. */
} M620ObjectState;

/* A movement sample stores its starting point, displacement, and calculated point. */
typedef struct {
    Point3d origin; /* Starting point of this particle's movement sample. */
    Point3d delta; /* Per-step displacement applied to the starting point. */
    Point3d result; /* Calculated position stored for this sample. */
} M620VectorRecord;

/* Groups particle movement settings with the batch's 32 position samples. */
typedef struct {
    u32 source; /* Source particle or object index selected by the caller. */
    u32 target; /* Destination particle or object index selected by the caller. */
    u32 resource; /* Data resource used to create the particle batch. */
    f32 step; /* Movement increment used to calculate each position sample. */
    s32 active; /* Nonzero while this batch is being updated. */
    M620VectorRecord records[32]; /* Position samples used by the particle movement. */
} M620ParticleBatch;

/* Character height and motion selections copied into every contestant's state at setup. */
typedef struct {
    float unk0; /* The game's code never reads this field. */
    float height; /* Replaced at setup with the selected character's model height. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk8[4]; /* No code names this field. */
    /* Idle and movement motions used while turning toward the front after play. */
    s32 finishIdleMotion, moveMotion;
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk14[12]; /* No code names this field. */
    s32 idleMotion; /* Motion selected while waiting for a turn. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk24[16]; /* No code names this field. */
    s32 turnMotion, winMotion, loseMotion; /* Turn and results motion selections. */
} M649PlayerConfig;

/* A lane marker stores its world position and whether it has already scored a hit. */
typedef struct {
    Vec position;
    s16 active, hit;
} M649Point;

typedef struct {
    s32 id, timer; /* Prop identity and its startup delay in frames. */
    float position, speed, targetSpeed; /* Travel distance and current/desired speed. */
    s32 mode, group; /* Marker mode and color group. */
    s32 unk1C; /* The game's code never reads this field. */
    s32 nextPoint;
    M649Point points[64];
} M649Motion;

#endif
