/* Scenery models, moving actors, and the collision limits around the stage. */
#ifndef REL_M626DLL_SCENERY_H
#define REL_M626DLL_SCENERY_H

#include "REL/m626Dll/player.h"
#include "REL/m626Dll/collision.h"
#include "REL/m626Dll/effects.h"

typedef struct M626SceneryMotion {
    s32 resource; /* Motion resource index; -1 leaves this slot unused. */
    f32 blend; /* Blend fraction, scaled to a 60-frame transition. */
    u32 attr; /* Motion playback attributes. */
} M626SceneryMotion;

typedef struct M626SceneryParameters {
    s32 count; /* Number of objects created from this entry. */
    s32 linkModel; /* Nonzero links to a shared model instead of loading one. */
    s32 type; /* Lookup category used by scenery queries. */
    s16 firstId; /* Stage-local ID assigned to the first object. */
    s16 layer; /* Render layer for the model. */
    u32 attr; /* Model and motion attributes applied at creation. */
    s32 shadow; /* 1 enables shadows; 2 enables shadow-map reception. */
    s32 modelResource; /* Model resource index. */
    M626SceneryMotion *motions; /* Motion slots, terminated by resource -2. */
    Vec *position; /* Optional initial world position. */
    f32 rotationY; /* Initial yaw in degrees. */
    OMOBJ_FUNC update; /* Object callback installed after creation. */
} M626SceneryParameters;

typedef struct M626SceneryWork {
    M626ObjectWork common;
    s32 state; /* Snowball or character action state. */
    s32 timer; /* Frames elapsed in the current scenery behavior. */
    s32 sampleIndex; /* Next authored particle position, from 0 through 2. */
    u8 unclassifiedBytesA[20]; /* Byte block not read or written by scenery behavior. */
    Vec initialPosition; /* Position captured when the character action begins. */
    Vec targetPosition; /* Latest attachment position tracked during the action. */
    u8 unclassifiedBytesB[132]; /* Byte block not read or written by scenery behavior. */
    M626EffectRecord effect;
} M626SceneryWork;

typedef union M626CollisionRecord {
    M626Entry entry; /* General actor collision entry. */
    M626RangeBounds bounds; /* Axis-aligned scenery collision bounds. */
    M626SurfaceCollider mesh; /* Transformed surface mesh collider. */
} M626CollisionRecord;

extern M626CollisionRecord lbl_1_bss_310[3];

#endif
