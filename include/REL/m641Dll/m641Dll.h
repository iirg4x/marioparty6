/* Shared score and route records used by Mole-it! board source files. */
#ifndef _REL_M641DLL_M641DLL_H_
#define _REL_M641DLL_M641DLL_H_

#include "dolphin/types.h"
#include "game/main.h"
#include "game/mg/score.h"

typedef struct M641ScoreEntry {
    s16 scoreValue; /* Accumulated group points, clamped to 0 through 99. */
    s16 boxHandle; /* Handle of the group score box. */
    MGSCORE *score; /* Display object for the group total. */
} M641ScoreEntry;

extern s16 lbl_1_bss_7C0[4];
extern M641ScoreEntry lbl_1_bss_7C8[2];

typedef struct M641CharacterRecord {
    s16 playerSlot; /* Player slot used to access the corresponding GwPlayer record. */
    s16 characterNo; /* Character number passed to character model and motion APIs. */
    s16 padNo; /* Pad number from GwPlayerConf; indexes the HuPad arrays. */
    s16 groupNo;
    s32 inputSamplingGate; /* Player input is sampled only while this value is zero. */
    s16 computerDifficulty; /* Route selection uses the player's computer difficulty. */
    s16 movementState; /* Movement state consumed by the per-frame character update. */
    s16 fadeInState; /* Per-player movement and pose state advanced during fade-in. */
    s16 resultSequenceState; /* Per-player end-of-round state advanced before the winner
                              * sequence. */
    Point3d velocity;
    Point3d jumpVelocity;
    s16 jumpDescentDelayFrames; /* Frames between jump-motion end and resuming downward velocity. */
    s16 collisionRecoveryState; /* Selects the collision-recovery branch after a jump. */
    s16 modelId; /* Character 3D model ID. */
    s16 motionIds[13];
    s16 currentMotion;
    s16 movementTimer; /* Movement-transition and wait timer in frames. */
    Point3d position;
    Point3d rotation;
    s16 tiltState; /* Tracks left/right tilt and return-to-center state during movement. */
    s16 unk6A; /* Never read by name in the module. */
    f32 collisionRadius; /* Collision radius in game-space units for player and board-space
                          * tests. */
    f32 characterHeight; /* Configured character height in game-space units for collision and
                          * jumps. */
    s8 stickX;
    s8 stickY;
    u16 buttons;
    u16 buttonsDown;
    u16 heldButtonFlags; /* Accumulates held-button samples; setup, airborne updates, and landing
                          * recovery clear it. */
    u16 jumpInputFlags; /* Carries a buffered jump request during landing recovery. */
    s16 routeUpdateCount; /* Counts per-frame route updates since the round reset. */
    s16 targetActionFrames; /* Counts the selected-space jump sequence; zero allows approach
                             * movement. */
    s16 targetSpaceIndex; /* Selected nearby-space index, or -1 when no target is assigned. */
    s16 autonomousMovementState; /* State for automatic direction changes and near-space
                                  * movement. */
    s16 autonomousMovementFrames; /* Counts frames spent in automatic movement states. */
    s16 directionChangeTimer; /* Frames remaining before the automatic movement direction
                               * changes. */
    u16 unk8A; /* Never read by name in the module. */
} M641CharacterRecord;

typedef struct BoardSpaceRoute {
    /* Per-space models, motions, states, timers, and flags for this board. */
    s16 spaceModel[7];
    s16 spaceMotion[7][2];
    s16 spaceState[7];
    s16 spaceTimer[7];
    s16 spaceFlags[7];
    s16 itemModel[7];
    s16 itemMotionIds[7]; /* Motion ID created for each item model. */
    s16 itemState[7];
    s16 itemTimer[7];
    s16 itemFlags[7][4];
    s32 itemSound[7];
    s16 specialModel;
    s16 specialMotion[2];
    s16 specialSecondModel;
    s16 specialState;
    s16 specialTimer;
    s16 specialSpace;
    s16 specialUsed;
    s16 scheduledTime[60];
    s16 scheduledType[60];
    s16 nextScheduled;
    s16 queuedType[60];
    s16 queueCount;
} BoardSpaceRoute;

typedef struct M641RouteState {
    BoardSpaceRoute boards[2];
    u8 unknownTail[28]; /* Never read by name in the module. */
} M641RouteState;

extern M641CharacterRecord lbl_1_bss_590[4];
extern M641RouteState lbl_1_bss_84;

#endif
