#ifndef REL_M605DLL_PLAYER_STATES_H
#define REL_M605DLL_PLAYER_STATES_H
#include "REL/m605DLL/owner_callback.h"

// Walks after the character reaches the end of the course.
class WalkState : public OwnerCallback {
public:
    virtual ~WalkState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    // Selects the walking state.
    WalkState() : OwnerCallback(STATE_WALK) {}
};
// Stops the character and scales it according to its position relative to the rock.
class DeathState : public OwnerCallback {
public:
    virtual ~DeathState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    f32 scale;
    // Selects the death state.
    DeathState() : OwnerCallback(STATE_DIE) {}
};
// Plays the hit motion before entering the death state.
class HitState : public OwnerCallback {
public:
    virtual ~HitState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    // Selects the initial callback state.
    explicit HitState(s32 state) : OwnerCallback(state) {}
};
// Stops movement during the stand-up motion, then resumes running.
class StandUpState : public OwnerCallback {
public:
    virtual ~StandUpState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    s32 framesRemaining;
    // Selects the stand-up state.
    StandUpState() : OwnerCallback(STATE_STANDUP) {}
};
// Applies a vertical impulse and waits to begin standing up.
class StumbleState : public OwnerCallback {
public:
    virtual ~StumbleState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    s32 duration;
    // Selects the stumble state.
    StumbleState() : OwnerCallback(STATE_STUMBLE) {}
};
// Runs along the course and reacts to obstacles and lost ground.
class RunState : public OwnerCallback {
public:
    virtual ~RunState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    f32 lastActiveTime;
    // Selects the running state.
    RunState() : OwnerCallback(STATE_RUN) {}
};
// Waits for landing before resuming walking or running.
class FallState : public OwnerCallback {
public:
    virtual ~FallState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    s32 framesRemaining;
    // Selects the falling state.
    FallState() : OwnerCallback(STATE_FALL) {}
};
// Plays the jump motion until vertical velocity becomes nonpositive.
class JumpState : public OwnerCallback {
public:
    virtual ~JumpState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    // Selects the jumping state.
    JumpState() : OwnerCallback(STATE_JUMP) {}
};
// Plays the idle motion and stops movement on the ground.
class IdleState : public OwnerCallback {
public:
    virtual ~IdleState() {}
    virtual void enter(PlayerCharacter *player);
    virtual void advance(PlayerCharacter *player);
    // Selects an idle callback state.
    explicit IdleState(s32 state = STATE_IDLE) : OwnerCallback(state) {}
};
// Idles facing back along the course.
class ReverseIdleState : public IdleState {
public:
    virtual ~ReverseIdleState() {}
    virtual void enter(PlayerCharacter *player);
    // Selects the reverse-facing idle state.
    ReverseIdleState() : IdleState(STATE_IDLE) {}
};
// Plays the alternate idle motion.
class AlternateIdleState : public IdleState {
public:
    virtual ~AlternateIdleState() {}
    virtual void enter(PlayerCharacter *player);
    // Selects the alternate idle state.
    AlternateIdleState() : IdleState(STATE_IDLE) {}
};
#endif
