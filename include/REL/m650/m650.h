/* Shared Throw Me a Bone state and data declarations. */
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/printfunc.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#ifndef M650_H
#define M650_H

/* Sound effect resources used by Throw Me a Bone. */
enum M650EffectId {
    M650_EFFECT_1001 = 1001,
    M650_EFFECT_2028 = 2028,
    M650_EFFECT_2029 = 2029,
    M650_EFFECT_2030 = 2030,
    M650_EFFECT_2031 = 2031,
    M650_EFFECT_2032 = 2032,
    M650_EFFECT_2033 = 2033,
    M650_EFFECT_2034 = 2034,
    M650_EFFECT_2035 = 2035,
    M650_EFFECT_2036 = 2036,
    M650_EFFECT_2037 = 2037,
    M650_EFFECT_2038 = 2038,
    M650_EFFECT_2039 = 2039,
    M650_EFFECT_2040 = 2040,
    M650_EFFECT_2041 = 2041,
    M650_EFFECT_2042 = 2042,
    M650_EFFECT_2043 = 2043,
    M650_EFFECT_2044 = 2044,
    M650_EFFECT_2045 = 2045,
    M650_EFFECT_2046 = 2046,
    M650_EFFECT_2047 = 2047,
    M650_EFFECT_2048 = 2048,
    M650_EFFECT_2049 = 2049,
    M650_EFFECT_2050 = 2050,
    M650_EFFECT_2051 = 2051
};

/* Shared state used by the Throw Me a Bone sequence and callbacks. */
typedef struct M650Scene {
    MGTIMER *timer; /* Shared elapsed-time display used outside Decathlon. */
    s32 record; /* Best time, in timer ticks, loaded for this round. */
    s16 winner; /* Winning player index, or -1 when nobody wins. */
    s16 night; /* 0 for the daytime course, nonzero for the nighttime course. */
    s16 state; /* Step within the opening or results sequence. */
    s16 frame; /* Frames elapsed in the current sequence step. */
    s16 roundEnding; /* Set while player actions are being stopped for results. */
    s16 resultsScene; /* Selects the character pose used on the results screen. */
    s16 recordChanged; /* Nonzero when this round beats the saved record. */
    s16 winnerAnimationState; /* 0 idle, 1 bounce, 2 walk-off. */
    int audio; /* Handle for the minigame music stream. */
} M650Scene;

typedef struct M650Player {
    u8 unknown00[2]; /* This part of the player state is neither read nor written. */
    s16 model; /* Character model handle used by the character motion system. */
    s16 motionIDs[5]; /* Character motion handles indexed by the minigame motion table. */
    u8 unknown0E[2]; /* This part of the player state is neither read nor written. */
    s16 charNo; /* Character slot selected for this player. */
    s16 padNo; /* Controller port assigned to this player. */
    s16 computerDifficulty; /* CPU difficulty, or -1 for a human player. */
    s16 throwWait; /* Frames remaining in the spin penalty after a runner collision. */
    s16 actionFrame; /* Frame counter reused by throwing, runner rebound, and bone pickup
                      * actions. */
    s16 bounceFrameCount; /* Frames elapsed while the item is moving. */
    s16 state; /* Current player action state. */
    s16 model1E; /* Auxiliary player model shown during selected action states. */
    s16 model20; /* Player's animated body model used for movement and joint motions. */
    s16 jointMotionIDs[4]; /* Joint motion handles for the shared animated player model. */
    HuVecF direction; /* Runner's X/Z step toward the bone, also used for collision reflection. */
    HuVecF reflectedDirection; /* Unit direction after the most recent bounce. */
    float turnAngle; /* Current left/right turn angle in degrees. */
    float movementSpeed; /* Chase and results movement speed in world units per frame; rebound
                          * moves at half this value. */
    float horizontalDistance; /* Accumulated horizontal travel during the throw. */
    u8 turnTowardPositiveAngle; /* Nonzero while the turn angle increases. */
    s16 model52; /* Attached item model used by the character's item hook. */
    s16 model54; /* Per-player shadow or ground effect model. */
    u8 unknown56[14]; /* This part of the player state is neither read nor written. */
    HuVecF boneVelocity; /* Bone velocity in world units per frame during throws and the results
                          * bounce. */
    MGTIMER *timer; /* Per-player elapsed-time display used in Decathlon. */
    s16 reachedGoal; /* Nonzero after this player reaches the far end of the course. */
    s16 timerValue; /* Elapsed timer ticks recorded for this player. */
    s16 model78; /* Additional per-player effect model with motion initially stopped. */
    s16 model7A; /* Additional per-player effect model with motion initially stopped. */
    s16 model7C; /* Additional per-player effect model with motion initially stopped. */
    s16 candidateIDs[12]; /* Nearby obstacle indices considered by the computer player. */
    s16 candidateCount; /* Number of valid entries in candidateIDs. */
    s16 throwDelay; /* Randomized delay before a computer player chooses a throw. */
    s16 decisionFrameCount; /* Frames spent waiting for a computer throw decision. */
    s16 riskyThrowMode; /* CPU mode that accepts blocked aim or no obstacles before the decision
                        * timeout and ignores the lane boundary afterward. */
} M650Player;

typedef struct M650Cell {
    s16 state; /* 0 available, 1 breaking, 2 hidden, 3 record prop. */
    s16 timer; /* Frames spent blinking after the break motion ends. */
} M650Cell;

typedef struct M650Point {
    HuVecF pos; /* Obstacle location in world units. */
    s16 isRock; /* 0 selects the tree model; 1 selects the rock model. */
} M650Point;

typedef struct M650Motion {
    s32 file; /* Data resource number for this player motion. */
    u32 flags; /* Motion flags passed to the character motion system. */
} M650Motion;

extern M650Scene lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_1C;
extern M650Player lbl_1_bss_28[4];
extern M650Cell lbl_1_bss_2AC[32][4];
extern MGSEQ_PARAM lbl_1_data_0;
extern M650Motion lbl_1_data_58[5];
extern u16 lbl_1_data_150[4];
extern M650Point lbl_1_data_1F8[5];
extern M650Point lbl_1_data_248[32];
extern s32 lbl_1_data_448[2];
extern s32 lbl_1_data_450[2];

void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_6478(HU3D_MODEL *model, Mtx *mtx);
s16 fn_1_6D88(s16 player, HuVecF *position);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_5600(s16 player, float value, float *out0, float *out1);
s16 fn_1_6EE4(HuVecF *firstPosition, HuVecF *secondPosition, float radius);
s16 fn_1_7000(s16 player, HuVecF *position, float angle, s16 obstacleIndex);
void fn_1_289C(s16 player);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_62E0(void);
void fn_1_5258(void);
void fn_1_5308(void);
void fn_1_54D4(void);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *object);
void fn_1_6F54(void);
s16 fn_1_4D58(s16 frame);

#endif
