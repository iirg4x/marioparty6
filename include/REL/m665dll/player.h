/* Shared player records and role-dependent vehicle state for Verbal Assault. */
#ifndef REL_M665DLL_PLAYER_H
#define REL_M665DLL_PLAYER_H

#include "dolphin/types.h"

typedef void (*M665MovementCallback)(s16 playerNo, s16 *horizontal, s16 *vertical);

typedef struct M665MainModels {
    s16 motions[7]; /* Vehicle motion IDs. */
    s16 propellers[2]; /* Propeller model IDs. */
    s16 damageModels[2]; /* Damage overlay model IDs. */
    s16 hookModels[6]; /* Hook model IDs. */
    s16 hookEffects[6]; /* Hook effect model IDs. */
    s16 damage; /* Accumulated damage. */
    s16 brokenHooks; /* Number of broken hooks. */
    s16 actionCooldown; /* Frames remaining before another action. */
} M665MainModels;

typedef struct M665DetachedModel {
    s16 model; /* Detached effect model ID. */
    s16 state; /* Detached-effect state. */
    s16 blinkFrames; /* Elapsed effect updates; parity alternates visibility each update. */
    f32 velocityX; /* X movement per update. */
    f32 velocityY; /* Y movement per update. */
    f32 velocityZ; /* Z movement per update. */
} M665DetachedModel;

typedef struct M665OtherModels {
    s16 mainMotionIDs[2]; /* Overlaps the first two motions in M665MainModels through the union. */
    M665DetachedModel effects[2]; /* Detached effect models. */
    s16 motions[2]; /* Alternate vehicle motion IDs. */
    s16 damageCount; /* Number of hits recorded for this player. */
    s16 recoveryFrames; /* Recovery countdown, then elapsed updates used for elimination
                         * blinking. */
} M665OtherModels;

typedef union M665PlayerFlags {
    struct {
        u8 moving : 1; /* Enables the main vehicle's horizontal movement. */
        u8 positiveDirection : 1; /* Direction bit used by movement. */
        u8 other : 6; /* Remaining bit positions; accessed through the byte view. */
    } bits;
    u8 byte;
} M665PlayerFlags;

typedef struct M665PlayerWork {
    s16 characterModel; /* Character model ID. */
    s16 characterMotions[7]; /* Character motion IDs by motion-table index. */
    s16 vehicleModel; /* Vehicle model ID. */
    f32 position[3]; /* Vehicle world position in X, Y, Z order. */
    union {
        M665MainModels main; /* Main player's vehicle state. */
        M665OtherModels other; /* Opponents' vehicle and effect state. */
    } models;
    s16 action; /* Main-vehicle action state or opponent shot cooldown. */
    s16 actionFrames; /* Action countdown or repetition counter; also drives the winner bobbing
                       * angle. */
    union {
        struct {
            s16 effectState; /* Player effect state. */
            s16 selectionFrames; /* Updates remaining before the next CPU effect selection. */
            s32 soundHandle; /* Active sound-effect handle, or -1 when stopped. */
        } main; /* Main-player sound and effect state. */
        struct {
            s16 scoreGroup; /* Score sprite-group ID. */
            s16 spriteGroup; /* Effect sprite-group ID. */
            s16 damaged; /* Set when the opponent is eliminated. */
            s16 damageFrames; /* Set to 420 when the opponent is eliminated. */
        } other; /* Opponent sprite and damage-feedback state. */
    } effect;
    union {
        struct {
            M665PlayerFlags flags; /* Main-player movement flags. */
        } main; /* Main-player movement flags. */
        struct {
            s16 state; /* Other-player status state. */
        } other; /* Opponent status state. */
    } status;
    f32 speed; /* Main-vehicle horizontal speed, or an eliminated opponent's upward speed. */
    M665MovementCallback movement; /* Current movement update function. */
    s16 movementIndex; /* Index into the movement-function table. */
    s16 movementFrames; /* Frames before the next movement decision. */
    f32 movementAngle; /* Movement direction in degrees. */
    s16 movementOverride; /* Forces the alternate movement routine when nonzero. */
    s16 selectedStage; /* Selected forward or backward stage index. */
    u8 unreadBeforeMotionSpeed[4]; /* Unread byte storage before the vehicle-motion playback
                                    * speed. */
    f32 motionSpeed; /* Vehicle-motion playback speed used for sound timing. */
    u8 unreadBeforePlayerIndex[2]; /* Unread byte storage before the player-configuration index. */
    s16 playerIndex; /* Player-configuration index. */
    s16 groupIndex; /* Index among the three non-main players. */
    s16 character; /* Character ID from player configuration. */
    s16 team; /* Team number from player configuration. */
    s16 controller; /* Controller number from player configuration. */
    s16 difficulty; /* CPU difficulty, or -1 for a human player. */
} M665PlayerWork;

extern M665PlayerWork lbl_1_bss_10[4];

#endif
