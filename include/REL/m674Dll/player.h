/* Player state, motion settings and input callbacks for the minigame. */
#ifndef REL_M674DLL_PLAYER_H
#define REL_M674DLL_PLAYER_H
#include "game/object.h"
#include "REL/m674Dll/collision.h"

typedef M674Entry M674CollisionParam;
typedef s32 (*M674PlayerCallback)(M674CollisionParam *, M674CollisionParam *);
typedef struct M674PlayerWork M674PlayerWork;
typedef struct M674ActorState M674ActorState;
typedef void (*M674PlayerControl)(M674PlayerWork *work);

typedef struct M674PlayerConfig {
    /* Collision radius, character height, movement speed, jump speed, gravity, and dust scale. */
    f32 radius, characterHeight, moveSpeed, jumpSpeed, gravity, dustStrength;
    f32 blendScale, blendSpan, currentBlend; /* Partner-hit vertical scale, lateral-scale parameter,
                                              * and response reference value. */
    s32 idleMotion, walkMotion; /* Character motion indices for standing and walking. */
    f32 walkSpeed; /* Walk-motion playback multiplier applied to stick strength. */
    s32 runMotion; /* Character motion index for running. */
    f32 runSpeed; /* Run-motion playback multiplier applied to stick strength. */
    /* Selected after landing at y <= 0; the movement update has already forced input to zero. */
    s32 lowerSurfaceIdleMotion, lowerSurfaceWalkMotion; /* Idle and walk motion indices. */
    f32 lowerSurfaceWalkSpeed; /* Walk-motion playback multiplier applied to stick strength. */
    s32 lowerSurfaceRunMotion; /* Run motion index. */
    f32 lowerSurfaceRunSpeed; /* Run-motion playback multiplier applied to stick strength. */
    s32 jumpMotion, landMotion, winMotion, loseMotion; /* Motions for jumping, landing, and
                                                        * results. */
} M674PlayerConfig;

struct M674ActorState {
    M674Entry task; /* Collision entry registered for this actor. */
    /* Score/result slot and previous/current player-slot IDs; controlEnabled marks player-owned
     * actors, including CPUs, and gates input, feedback, and readiness. */
    s32 scoreIndex, previousPlayer, playerNo, controlEnabled;
    Vec position, velocity, rotation; /* World position, per-frame velocity, and Euler rotation in
                                       * degrees. */
    u32 buttonsDown, buttons; /* Buttons pressed this frame and buttons currently held. */
    f32 stickAngle, stickMagnitude; /* Input heading in degrees and normalized stick strength. */
    s32 motionIndex, motionFrame,
        requestedMotion; /* Current motion index, transform-application count since the motion
                          * change, and queued motion index. */
    f32 motionSpeed; /* Playback speed applied with the queued motion. */
    s32 phase, timer; /* Current actor-sequence phase and its frame counter. */
    s32 unreadStateA0; /* Unread state word at this layout position. */
    f32 targetAngle; /* Desired actor facing angle, in degrees. */
    Vec targetPosition; /* World-space position used by the current movement action. */
    /* Action and turn progression counters. */
    s32 actionPhase, actionFrame, unreadStateBC, turnState, turnFrame;
    s32 stateFlags, registeredFrames; /* Actor state bits and frames since registration. */
    s32 computer, difficulty, controlPhase; /* CPU-control flag, difficulty tier, and control setup
                                             * phase. */
    u8 unreadStateBytesDC[20]; /* Unread bytes in the actor state record. */
    M674PlayerControl controlUpdate; /* Active computer-control callback. */
    s32 controlState, controlFrame, controlTimer, contact; /* Control substate, frame counters, and
                                                            * contact pulse. */
    f32 headingOffset; /* Temporary computer-steering offset in degrees. */
    /* Motion and player-action state. impactJump is set on an impact bounce, cleared on a normal
     * jump or landing, and prevents held-jump gravity reduction. */
    s32 motionActive, motionTimer, motionType, action, impactJump, actionResult;
    s32 pendingImpact, blendRequest; /* Deferred impact and partner-blend requests. */
    M674ActorState *partner; /* Other actor involved in a partner impact. */
    /* Partner score/result slot and nonzero horizontal-movement lock, which is not decremented. */
    s32 partnerScoreIndex, movementLock;
    f32 requestedBlend, xzScaleGain; /* Target vertical scale and gain controlling lateral
                                      * expansion. */
    s32 blendState, blendFrame, cooldown; /* Blend phase, elapsed frames, and cooldown frames. */
    f32 blendAmount, blendValue; /* Movement/jump multiplier and vertical model scale. */
    s32 shadowEnabled, shadowModel, regionInitialized; /* Shadow selection and cached-region
                                                        * state. */
    /* Play-region edges and bounce motion. */
    f32 radialOffsetX, radialOffsetZ, bounceHeight, bounceAngle;
    s32 spareState, unreadState170; /* Frames to skip jump/gravity updates, plus an unread word. */
    M674PlayerConfig config; /* Character movement, motion, and blend settings. */
};

struct M674PlayerWork {
    M674ActorState state; /* Shared actor state updated by the player object. */
    /* Game player, character, object phase, and unread word. */
    s32 playerIndex, characterNo, processPhase, unreadWorkState1DC;
    s32 updateCount, volumeEnabled; /* Object updates completed and character voice-volume state. */
};

typedef struct M674PlayerMotionParam {
    u16 motion; /* Motion-array slot played for this player action. */
    s16 overlay; /* Overlay motion-array slot, or -1 when no overlay is used. */
    f32 blend; /* Motion-shift blend fraction, multiplied by 60 frames. */
    u32 attr; /* Motion attributes applied while the action plays. */
} M674PlayerMotionParam;
typedef struct M674ComputerParam {
    f32 speed; /* Difficulty movement factor used for search ranges and steering. */
    s32 useFixedDirection; /* Selects the radial heading nearest current facing when no airborne
                            * target is found. */
    s32 playerThreshold; /* Score threshold that halves airborne target-search angle and distance
                          * limits. */
} M674ComputerParam;

extern s32 lbl_1_bss_2E0, lbl_1_bss_2E4;
extern M674ActorState *lbl_1_bss_2E8[5];
extern OMOBJ *lbl_1_bss_2FC[4];
extern OMOBJ *lbl_1_bss_30C;
extern u32 lbl_1_data_D8[8];
extern M674PlayerMotionParam lbl_1_data_F8[8];
extern Vec lbl_1_data_158[4], lbl_1_data_198[4], lbl_1_data_1D8;
extern f32 lbl_1_data_188[4], lbl_1_data_1C8[4];
extern M674PlayerConfig lbl_1_data_1E4;
extern M674ComputerParam lbl_1_data_240[4];
#endif
