#ifndef REL_M628DLL_PLAYER_ACTOR_H
#define REL_M628DLL_PLAYER_ACTOR_H

#include "REL/framework/stick_state.h"
#include "dolphin/types.h"
#include "REL/framework/event_hm.h"
#include <memory>
#include "REL/framework/p_chara.h"
#include "REL/framework/game_object.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"

class PlayerActor;
typedef void (PlayerActor::*ActorEventCallback)(const EventPacket &);
typedef void (PlayerActor::*ActorStateCallback)();

// Provides stick input for the lever actor.
class ActorController
{
  public:
    virtual ~ActorController();
    virtual void invoke(StickState *) = 0;
    StickState state;
    u8 unk18[4]; /* No code names this field. */
    PlayerActor *actor;
    // Associates the input controller with its lever actor.
    ActorController(PlayerActor *value) : actor(value) {}
    void fn_1_525C();
};

// Updates the lever actor, its animation and the positions used by the board tracker.
class PlayerActor : public GameObject, public PlayerConfigSelection
{
  public:
    PlayerActor(s32 playerNo, s32 groupCount, f32 *leftLever, f32 *rightLever);
    virtual ~PlayerActor();
    f32 *leftLever;
    f32 *rightLever;
    std::auto_ptr<ActorController> controller; // Owns the lever input controller.
    s32 playerNo;
    bool unk3C; /* The game's code never reads this field. */
    s32 unk40;  /* The game's code never reads this field. */
    Vec position;
    Vec displacement;
    // Returns the actor velocity.
    const Vec &getVelocity() const
    {
        return velocity;
    }
    // Returns the tracked actor displacement.
    const Vec &getDisplacement() const
    {
        return displacement;
    }
    Vec rotation;
    Vec velocity;
    u32 motion;        // Currently selected actor motion.
    s32 overlayMotion; // Motion applied over the actor base animation.
    f32 rightAngle;
    f32 leftAngle;
    f32 elapsed;
    u8 active;
    s32 state;
    s32 sound;
    EventHandlerManager callbacks; // Receives the object movement and state events.
    ActorStateCallback callback;   // Runs the current object state.
    void fn_1_155A8(const EventPacket &);
    void fn_1_155EC(const EventPacket &);
    void fn_1_15704(const EventPacket &);
    void fn_1_157E8(const EventPacket &);
    void fn_1_1503C();
    void fn_1_15040();
    void fn_1_153E8();
    void fn_1_1531C();
    void fn_1_14CF4();
    void fn_1_14D78();
    void fn_1_14E58();
    void fn_1_14F40(u8 active);
    // Selects the current update callback.
    void setCallback(ActorStateCallback value)
    {
        callback = value;
    }
    virtual void update();
};

#endif
