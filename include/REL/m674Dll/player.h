/* Player state, motion settings and input callbacks for the minigame. */
#ifndef REL_M674DLL_PLAYER_H
#define REL_M674DLL_PLAYER_H
#include "REL/minigame_player_motion.h"
#include "game/object.h"
#include "REL/m674Dll/collision.h"

typedef s32 (*M674PlayerCallback)(M674Entry *, M674Entry *);
typedef struct M674PlayerWork M674PlayerWork;
typedef struct M674ActorState M674ActorState;
typedef void (*M674PlayerControl)(M674PlayerWork *work);

typedef struct M674PlayerConfig {
    f32 radius; /* Collision radius. */
    f32 unk04; /* The game's code never reads this field. */
    f32 moveSpeed, jumpSpeed, gravity, dustStrength; /* Movement and dust settings. */
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
    s32 scoreIndex; /* Score/result slot. */
    s32 unk48; /* The game's code never reads this field. */
    s32 playerNo; /* Current player slot. */
    s32 controlEnabled; /* Gates input, feedback, and readiness for player-owned actors. */
    Vec position, velocity, rotation; /* World position, per-frame velocity, and Euler rotation in
                                       * degrees. */
    u32 buttonsDown, buttons; /* Buttons pressed this frame and buttons currently held. */
    f32 stickAngle, stickMagnitude; /* Input heading in degrees and normalized stick strength. */
    s32 motionIndex; /* Current motion index. */
    s32 unk8C; /* The game's code never reads this field. */
    s32 requestedMotion; /* Queued motion index. */
    f32 motionSpeed; /* Playback speed applied with the queued motion. */
    s32 phase, timer; /* Current actor-sequence phase and its frame counter. */
    s32 unreadStateA0; /* Unread state word at this layout position. */
    f32 targetAngle; /* Desired actor facing angle, in degrees. */
    Vec targetPosition; /* World-space position used by the current movement action. */
    /* Action and turn progression counters. */
    s32 actionPhase, actionFrame;
    u8 unk_BC[4]; /* No code names this field. */
    s32 turnState, turnFrame;
    s32 stateFlags; /* Actor state bits. */
    s32 unkCC; /* The game's code never reads this field. */
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
    s32 unk1E0; /* The game's code never reads this field. */
    s32 volumeEnabled; /* Character voice-volume state. */
};

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
extern MinigamePlayerMotionParams lbl_1_data_F8[8];
extern Vec lbl_1_data_158[4], lbl_1_data_198[4], lbl_1_data_1D8;
extern f32 lbl_1_data_188[4], lbl_1_data_1C8[4];
extern M674PlayerConfig lbl_1_data_1E4;
extern M674ComputerParam lbl_1_data_240[4];

/* Player setup and updates. */
s32 fn_1_7660(M674Entry *first, M674Entry *second);

#endif
