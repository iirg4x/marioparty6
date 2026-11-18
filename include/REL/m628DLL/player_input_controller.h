#ifndef REL_M628DLL_PLAYER_INPUT_CONTROLLER_H
#define REL_M628DLL_PLAYER_INPUT_CONTROLLER_H
#include "REL/m628DLL/player_actor.h"
#include "REL/framework/p_info.h"
#include "REL/framework/vector3.h"
#include "REL/framework/pad_control.h"
class ActPlayer;
// Provides input for a player character and owns its pad state.
class PlayerInputController
{
  public:
    virtual ~PlayerInputController();
    StickState state;
    u8 unk18[4]; /* No code names this field. */
    ActPlayer *owner;
    PadControl inputState;
    virtual void invoke(StickState *) = 0;
    virtual void invoke();
    // Associates the input controller with its player and initializes its pad state.
    PlayerInputController(ActPlayer *value) : owner(value), inputState(1, 0.0f) {}
};

// Releases the player input controller.
inline PlayerInputController::~PlayerInputController() {}

// Clears the input state and invokes the player controller.
void fn_1_52D4(PlayerInputController *object);

#endif
