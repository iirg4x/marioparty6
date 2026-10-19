/* State for the falling stage objects and their paired models. */
#ifndef REL_M625DLL_MOTION_H
#define REL_M625DLL_MOTION_H

#include "game/main.h"
#include "game/hu3d.h"

typedef struct M625MotionState {
    /* Main falling-object model id. */
    HU3D_MODELID primaryModel;
    /* Splash-effect model id paired with the falling object. */
    HU3D_MODELID secondaryModel;
    /* Lifecycle: -1 free, 0-2 falling, 3-5 splash animation, 6 retiring. */
    s16 state;
    /* Object position in world units. */
    Vec position;
    /* Pending per-frame movement in world units. */
    Vec velocity;
    /* Downward speed used directly by the falling motion, in world units per frame. */
    f32 fallSpeed;
    /* Offset from the object position used for collision queries. */
    Vec positionOffset;
    /* Collision radius in world units. */
    f32 collisionRadius;
    /* Water-effect transition state: 0 before entering water, 1 after. */
    s32 effectState;
    /* Countdown for spawning water particles; -1 means the burst has ended. */
    s16 effectTimer;
    /* Counter incremented in splash setup before the paired motion starts. */
    s16 motionTimer;
} M625MotionState;

extern M625MotionState lbl_1_bss_4B8[13];
extern HU3D_MODELID lbl_1_bss_7C4;

#endif
