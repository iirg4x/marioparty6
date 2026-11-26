#ifndef REL_MINIGAME_CAMERA_WORK_H
#define REL_MINIGAME_CAMERA_WORK_H

#include "game/object.h"

typedef struct MinigameCameraEntry {
    s32 active, motion, model; /* Use state, active motion, and attached model ID. */
    OM_CAMERA_VIEW view, previousView; /* Current and target camera views. */
    Vec position, up, target; /* Camera position, up direction, and look target. */
    f32 fov, nearPlane, farPlane, aspect; /* Perspective projection settings. */
    f32 viewport[4]; /* Viewport x, y, width, and height in pixels. */
    f32 scissor[4];  /* Scissor x, y, width, and height in pixels. */
    s32 phase, timer; /* Camera transition phase and elapsed frames. */
    u8 unkA0[24]; /* No code names this field. */
} MinigameCameraEntry;

typedef struct MinigameCameraWork {
    u32 cameras; /* Camera mask used by this camera object. */
    f32 shadowRadius, shadowFov; /* Shadow volume radius and camera field of view. */
    Vec shadowDirection; /* Direction used for the scene shadow volume. */
    GXColor shadowColor; /* Color applied to the shadow projection. */
    void (*callback)(OMOBJ *, u32, s32); /* Per-frame camera transition callback. */
    MinigameCameraEntry entries[16]; /* Transition state indexed by camera slot. */
    s32 shadowInitialized; /* Set after shared shadow settings are installed. */
    u8 unkBA4[4]; /* No code names this field. */
} MinigameCameraWork;

#endif
