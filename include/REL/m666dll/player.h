/* Player motion and interaction state for Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_PLAYER_H
#define REL_M666DLL_PLAYER_H
#include "dolphin/types.h"
struct MgPlayer_s;
typedef struct M666PlayerRecord {
    struct MgPlayer_s *player;
    s16 playerNumber;
    s16 unk06; /* The game's code never reads this field. */
    s16 charNo;
    s16 modelId;
    s16 comDifficulty;
    s16 zoneIndex;
    s16 motionSelection;
    /* Sine-displacement phase; each increment adds eight degrees to the phase. */
    s16 displacementPhase;
    f32 targetAngle;
    f32 unk18; /* The game's code never reads this field. */
    s16 nudgeTimer;
    s16 unk1E; /* The game's code never reads this field. */
    /* Random steering angle in degrees and the remaining steering countdown. */
    s16 wanderAngle;
    s16 wanderFrames;

    s8 unk24_0 : 1; /* The game's code never reads this field. */
    /* Disables steering-angle smoothing when set. */
    s8 suppressAngleSmoothing : 1;
    /* Adds 60 degrees to the CPU steering angle when set. */
    s8 addSteeringOffset : 1;

    s8 unk24_3 : 1; /* The game's code never reads this field. */

    s8 unk24_4 : 1; /* The game's code never reads this field. */
    /* Decay subtracted from the initial upward push velocity of 60. */
    f32 verticalPushDecay;
    f32 pushAngle;
} M666PlayerRecord;
extern M666PlayerRecord lbl_1_bss_8[3];
#endif
