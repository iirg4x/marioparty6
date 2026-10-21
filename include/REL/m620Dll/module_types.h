/* Shared Trap Ease Artist records used by the module's actors and effects. */
#ifndef REL_M620DLL_MODULE_TYPES_H
#define REL_M620DLL_MODULE_TYPES_H

#include "dolphin/math.h"
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

#include "REL/m620Dll/module_data.h"

/*
 * The light object is allocated as 48 bytes by fn_1_1244. Its setup routine
 * writes the fields at +0, +4, +8, +20, +32 and +44, so the
 * ordinary ABI layout below names only those observed members.
 */
typedef struct M620LightData_s {
    HU3D_LIGHTID lightId;
    OMOBJ_FUNC callback;
    HuVecF position;
    HuVecF target;
    HuVecF direction;
    GXColor color;
} M620LightData;

/*
 * fn_1_2D7C allocates the callback state and initializes its first word from
 * the caller.  fn_1_2FA8 installs the second word, and fn_1_2E18 invokes it
 * with the OMOBJ delivered by the object manager.
 */
typedef void (*M620ObjectCallback)(OMOBJ *obj);

typedef struct M620ObjectState_s {
    void *userData;
    M620ObjectCallback callback;
} M620ObjectState;

/* The effect records are copied and processed with a 36-byte stride. */
typedef struct M620VectorRecord_s {
    Point3d origin;
    Point3d previousPosition;
    Point3d displacement;
} M620VectorRecord;

/* fn_1_C9D0 initializes this 1172-byte per-effect block. */
typedef struct M620ParticleBatch_s {
    s32 source;
    s32 target;
    s32 resource;
    f32 trailLength;
    s32 active;
    M620VectorRecord records[32];
} M620ParticleBatch;

/* This describes the first 152 bytes of the 292-byte player actor record. */
typedef struct M620ActorDataView_s {
    /* The actor helpers do not read these leading bytes. */
    u8 unreadBytesBeforePlayerId[64];
    s32 playerId;
    s32 playerIndex;
    s32 spawnIndex;
    s32 controlEnabled;
    /* World position, per-frame movement delta, and model rotation. */
    HuVecF position;
    HuVecF velocity;
    HuVecF rotation;
    /* Four bytes between the transform and input state are not read. */
    u8 unreadBytesBeforeButtons[4];
    /* Button masks sampled or synthesized for the current actor update. */
    u32 buttonsDown;
    u32 buttons;
    /* Stick heading in degrees and magnitude normalized to the range 0 through 1. */
    f32 stickAngle;
    f32 stickMagnitude;
    /* Motion-list slot, elapsed actor frames, and pending motion (-1 means none). */
    s32 motionIndex;
    s32 motionFrame;
    s32 motion;
    /* Multiplier applied when the pending motion is selected. */
    f32 motionSpeed;
} M620ActorDataView;

#endif
