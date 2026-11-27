/* Model settings, animation and movement state shared by the scene props. */
#ifndef REL_M676DLL_PROP_TYPES_H
#define REL_M676DLL_PROP_TYPES_H
#include "REL/minigame_prop_params.h"
#include "REL/minigame_prop_motion.h"

#include "REL/m676Dll/actor_types.h"

typedef struct M676PropState {
    M676ActorState actor;
    s32 controlState;
    s32 controlTimer;
    u8 unkB4[4]; /* No code names this field. */
    /* Desired horizontal heading in degrees and its world-space destination. */
    f32 targetAngle;
    Vec targetPosition;
    s32 sequenceState;
    s32 sequenceTimer;
    u8 unkD0[4]; /* No code names this field. */
    s32 movementState;
    s32 movementTimer;
    u8 unkDC[52]; /* No code names this field. */
    s32 motionActive;
    s32 unk114; /* The game's code never reads this field. */
    u8 unk118[4]; /* No code names this field. */
    s32 unk11C; /* The game's code never reads this field. */
    u8 unk120[44]; /* No code names this field. */
    /* Accumulated requested movement for footstep sounds; reduced by 100 after each sound. */
    f32 footstepDistance;
    s32 unk150; /* The game's code never reads this field. */
    f32 unk154; /* The game's code never reads this field. */
    Vec *unk158; /* The game's code never reads this field. */
    Vec savedPosition;
    u8 unk168[60]; /* No code names this field. */
} M676PropState;

typedef struct M676PropWork {
    M676PropState state;
    MinigamePropParams *param;
    s32 id;
    s32 timer;
    Vec startPosition;
    s32 phase;
    s32 targetChangeTimer;
    s32 frame;
    u8 unk1C8[4]; /* No code names this field. */
    s32 actionIndex;
    s32 pathIndex;
    /* Scenery uses wobble strength and angular phases. Lane props reuse wobbleAmount as
     * target yaw, bobPhase as a rolling sign, and pitchPhase/rollPhase as X/Z movement
     * directions. */
    f32 wobbleAmount;
    f32 bobPhase;
    f32 pitchPhase;
    f32 unknown_1E0; /* The game's code never reads this field. */
    f32 rollPhase;
    u8 unk1E8[140]; /* No code names this field. */
    void *records;
    s32 soundHandle;
    s32 modelId;
    s32 movementState;
    s32 contactState;
    s32 settleTimer;
    s32 finished;
    s32 selectionState;
    s32 initialized;
    s32 outOfRange;
    f32 targetX;
    f32 targetZ;
} M676PropWork;

extern OMOBJ *lbl_1_bss_EA0[128];

#endif
