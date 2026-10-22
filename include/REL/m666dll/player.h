/* Player motion and interaction state for Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_PLAYER_H
#define REL_M666DLL_PLAYER_H
#include "dolphin/types.h"
struct MgPlayer_s;
typedef struct M666PlayerRecord {
    struct MgPlayer_s *player;
    s16 playerNumber;
    s16 padNumber;
    s16 charNo;
    s16 modelId;
    s16 comDifficulty;
    s16 zoneIndex;
    s16 motionSelection;
    /* Sine-displacement phase; each increment adds eight degrees to the phase. */
    s16 displacementPhase;
    f32 targetAngle;
    f32 secondTargetAngle;
    s16 nudgeTimer;
    /* Written with the selected avoidance direction; game code never reads this field. */
    s16 nudgeDirection;
    /* Random steering angle in degrees and the remaining steering countdown. */
    s16 wanderAngle;
    s16 wanderFrames;
    /* Written by the steering update; game code never reads this bit. */
    s8 unknownFlag80 : 1;
    /* Disables steering-angle smoothing when set. */
    s8 suppressAngleSmoothing : 1;
    /* Adds 60 degrees to the CPU steering angle when set. */
    s8 addSteeringOffset : 1;
    /* Written by player-event handling; game code never reads this bit. */
    s8 unknownFlag10 : 1;
    /* Written by wall handling; game code never reads this bit. */
    s8 unknownFlag08 : 1;
    /* Game code never accesses this bit. */
    s8 unknownFlag04 : 1;
    /* Game code never accesses this bit. */
    s8 unknownFlag02 : 1;
    /* Game code never accesses this bit. */
    s8 unknownFlag01 : 1;
    /* Decay subtracted from the initial upward push velocity of 60. */
    f32 verticalPushDecay;
    f32 pushAngle;
} M666PlayerRecord;
extern M666PlayerRecord lbl_1_bss_8[3];
#endif
