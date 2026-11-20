#ifndef REL_M611DLL_PLAYER_STATE_H
#define REL_M611DLL_PLAYER_STATE_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include <vector>
#include "REL/framework/list.h"
struct ParticleStrip;
struct TimedFlash;
// Player models, motion sequence, input, and movement state.
struct PlayerState : ListNode<PlayerState> {
    Vec position;
    void (PlayerState::*callback)();
    u8 callbackDisabled;
    u8 active;
    s32 playerIndex;
    s32 character;
    s32 padNo;
    s32 characterMotion;
    s32 comDif;
    f32 rotationY;
    f32 unk3C; // The game's code never reads this field.
    f32 targetAngle;
    f32 movementLength;
    s32 unk48; // The game's code never reads this field.
    f32 unk4C; // The game's code never reads this field.
    OwnedModel bodyModel;
    OwnedModel craneModel;
    OwnedModel boxModel;
    OwnedModel timedTransitionModel;
    OwnedModel motionModel;
    OwnedModel resetMotionModel;
    OwnedModel firstStateModel;
    OwnedModel secondStateModel;
    Vec jointPosition;
    u8 winner;
    u8 unk6D[1]; /* No code names this field. */
    u8 resultPoseComplete;
    u8 unk6F[5]; /* No code names this field. */
    s32 updateCountdown;
    s32 currentMotion;
    u8 motionEnabled;
    s32 motionTimer;
    s32 previousCandidateState;
    s32 score;
    bool hasCandidate;
    u8 candidateChanged;
    s32 effectTimer;
    s32 collisionTimer;
    s32 unk98; // The game's code never reads this field.
    u8 unk9C; // The game's code never reads this field.
    s32 updateCount;
    // The game's code never reads this field.
    u8 unkA4;
    s32 unkA8; // The game's code never reads this field.
    s32 selectedPosition;
    Vec targetPosition;
    ParticleStrip *particleStrip;
    TimedFlash *timedFlash;
    u8 motionActive;
    u8 transitionStarted;
    f32 motionDuration;
    u8 motionState;
    u8 countdownActive;
    u8 unkCE; // The game's code never reads this field.
    u8 modelAttributeFlag;
    u8 motionResetPending;
    u8 angleEnabled;
    u8 unkD2; // The game's code never reads this field.
    s32 countdown;
    s32 transitionCountdown;
    s32 transitionState;
    s16 firstMotion;
    s16 secondMotion;
    u8 effectEnabled;
    f32 effectHeight;
    f32 elapsed;
    u8 audioEnabled;
    u8 belowSpriteThreshold;
    s32 activeAudioId;
    s32 soundId;
    u8 soundCuePlayed;
    enum { NUMBEROF_MOTION = 1 };
    std::vector<s16> m_mot_id;
    PlayerState(s32 playerNo, s32 modelVariant, bool dayMode);
    ~PlayerState();
    void chooseMotionInput();
    void advanceMotionState();
    void advanceMotionCountdown();
    void finishMotion();
    void fn_1_D748();
    void fn_1_D81C();
    void fn_1_D204();
    void fn_1_CFBC();
    void fn_1_FB24();
    void fn_1_FEE0();
    void fn_1_10EF4(Vec *direction);
    void fn_1_11554(Vec *direction);
    void fn_1_10C04();
};

void fn_1_FA64(PlayerState *player, s32 motionId, u8 queued);
bool fn_1_10848(PlayerState *player, Vec *movement);
s32 fn_1_116D8(PlayerState *player);
void fn_1_EFAC(PlayerState *player);
void fn_1_EB48(PlayerState *player);

void fn_1_CB18();
void fn_1_CBB0();
void fn_1_CDEC();
void fn_1_CE90(Vec *positions);
bool fn_1_CF20();
u8 fn_1_D5D0();

void fn_1_BD38();

#endif
