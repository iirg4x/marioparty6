/* Declares player movement, character motion and collision state. */
#ifndef REL_M619DLL_PLAYER_H
#define REL_M619DLL_PLAYER_H
#include "game/object.h"
#include "REL/m619Dll/particle.h"
#include "REL/m619Dll/motion_system.h"

typedef struct M619Player M619Player;
typedef struct M619Sway {
    const HuVecF *position; /* Player position followed by the sway model. */
    const HuVecF *rotation; /* Player rotation in degrees. */
    f32 rotationY; /* Sway model's Y rotation in degrees. */
    f32 amount; /* Expansion amount, clamped from 0 to 15. */
    HU3D_MODELID model; /* Linked model animated alongside the player. */
} M619Sway;
typedef struct M619ModelMotion {
    HuVecF position; /* Position used by the attached model motion. */
    M619ParticleEffect effect; /* Particle effect associated with the model. */
    HU3D_MODELID model; /* Model animated by this record. */
} M619ModelMotion;
typedef struct M619ModelQueueNode M619ModelQueueNode;
struct M619ModelQueueNode {
    const HuVecF *sourcePosition; /* Transform followed by the queued models. */
    HuVecF modelPositions[3]; /* Current positions of the three queued models. */
    HU3D_MODELID models[3]; /* Model IDs in the queue. */
    s32 state; /* Queue processing state. */
    s32 flags; /* Queue behavior flags. */
    M619ModelQueueNode *next; /* Next model queue entry. */
    M619ModelQueueNode *previous; /* Previous model queue entry. */
};
typedef struct M619PlayerMotionPayload {
    HuVecF position; /* Player position snapshot used by CPU steering. */
    u8 unidentifiedBytes[12]; /* Opaque bytes retained in the motion payload. */
    M619MotionNode collision; /* Collision node attached to the player motion. */
    M619Player *owner; /* Player state passed to this payload's callbacks. */
    s32 usesPlayerHeading; /* Nonzero to aim relative to the player's heading. */
} M619PlayerMotionPayload;
typedef struct M619AimResponse {
    s32 state; /* CPU heading-response phase. */
    s32 updates; /* Frames since the random heading was refreshed. */
    s32 updateLimit; /* Random refresh threshold in update frames. */
    s32 frame; /* Frames elapsed in the current response phase. */
    s32 frameLimit; /* Maximum frames before the response advances. */
    f32 randomHeading; /* Random target heading in degrees. */
} M619AimResponse;

/* Player objects retain movement, collision, character motion and held effects. */
struct M619Player {
    OMOBJ *object; /* Object-manager object that owns this player state. */
    M619Sway sway; /* Linked model that expands while the player moves. */
    M619ModelQueueNode modelQueue; /* Models queued to follow the player. */
    M619PlayerMotionPayload motionPayload; /* Position snapshot and collision callbacks. */
    M619AimResponse aimResponse; /* CPU steering-response counters and target. */
    M619ModelMotion modelMotion; /* Attached model and its particle effect. */
    HuVecF position; /* Player world position. */
    HuVecF rotation; /* Player rotation in degrees. */
    HuVecF targetPosition; /* World position used by queued movement. */
    f32 targetAngle; /* Desired heading in degrees. */
    s32 turnFrames; /* Remaining frames in the current turn. */
    s32 inputEnabled; /* Whether human or CPU steering is processed. */
    s32 movementActive; /* Whether movement and sway updates are active. */
    s32 phaseFrame; /* Frames elapsed in the current player phase. */
    s32 flashCounter; /* Hit-reaction blink counter; zero means inactive. */
    s32 excluded; /* Whether this player was excluded from the round. */
    s32 movementFinished; /* Whether queued movement reached its destination. */
    HU3D_MODELID characterModel; /* Engine model for the player's character. */
    HU3D_MOTIONID motions[8]; /* Character motions loaded for the minigame. */
    s16 playerNumber; /* Player slot number. */
    s16 character; /* Character ID used by the character manager. */
    s16 group; /* Player's configured team or group number. */
    u8 unidentifiedBytes[2]; /* Opaque bytes retained in the player record. */
    s16 pad; /* Controller assigned to this player. */
    u8 unidentifiedBytes2[4]; /* Opaque bytes retained in the player record. */
    s16 playerType; /* Configured human or CPU player type. */
    s16 difficulty; /* CPU difficulty index. */
    s16 phase; /* Player movement and reaction phase. */
    s16 enabled; /* Whether player updates run. */
    M619ParticleEffect flashA; /* First collision flash effect. */
    M619ParticleEffect flashB; /* Second collision flash effect. */
    HuVecF positionA; /* First collision flash position. */
    HuVecF positionB; /* Second collision flash position. */
    char *hookNameA; /* Character attachment name for the first flash. */
    char *hookNameB; /* Character attachment name for the second flash. */
    s32 crossingStateA; /* Set when the first hook point reaches zero; cleared below zero to trigger
                         * its flash. */
    s32 crossingStateB; /* Set when the second hook point reaches zero; cleared below zero to
                         * trigger its flash. */
    M619MotionNode collision; /* Player collision node and callbacks. */
};
#endif
