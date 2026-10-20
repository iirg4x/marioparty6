/* Shared actor collision, transform, input and animation state. */
#ifndef REL_M676DLL_ACTOR_TYPES_H
#define REL_M676DLL_ACTOR_TYPES_H
#include "REL/m676Dll/collision_types.h"

typedef struct M676ActorState {
    M676CollisionEntry collision;
    s32 playerSlot;
    s32 workValue;
    s32 player;
    s32 active;
    Vec initialPosition;
    Vec position;
    Vec velocity;
    Vec rotation;
    f32 rotationYSource;
    u32 buttonsDown;
    u32 buttons;
    f32 stickAngle;
    f32 stickMagnitude;
    s32 currentMotion;
    s32 motionTime;
    s32 requestedMotion;
    f32 requestedMotionSpeed;
} M676ActorState;

void fn_1_6CA4(M676ActorState *state);
void fn_1_6CC0(M676ActorState *state, s32 status, f32 value);
void fn_1_8690(M676ActorState *state);
void fn_1_86C4(M676ActorState *record, s32 modelId);
#endif
