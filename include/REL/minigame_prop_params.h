#ifndef REL_MINIGAME_PROP_PARAMS_H
#define REL_MINIGAME_PROP_PARAMS_H

#include "REL/minigame_prop_motion.h"
#include "game/object.h"

/* Model, motion and placement settings for a group of prop instances. */
typedef struct MinigamePropParams {
    s32 count; /* Number of instances. A negative count ends the table. */
    s32 linkModel; /* Reuse an existing model when nonzero. */
    s32 type; /* Prop category used for lookup. */
    s16 firstId; /* ID of the first instance. */
    s16 layer; /* 3D render layer. */
    u32 attr; /* Model attributes set at creation. */
    s32 shadow; /* 1 enables shadow casting and 2 enables shadow-map reception. */
    s32 modelResource; /* Model resource index. -1 creates no model. */
    MinigamePropMotionParams *motions; /* Motion slots, terminated by resource -2. */
    Vec *position; /* Optional initial world position. */
    f32 rotationY; /* Initial yaw in degrees. */
    OMOBJ_FUNC update; /* Per-frame object callback. */
} MinigamePropParams;

#endif
