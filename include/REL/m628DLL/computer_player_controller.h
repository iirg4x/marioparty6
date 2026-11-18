#ifndef REL_M628DLL_COMPUTER_PLAYER_CONTROLLER_H
#define REL_M628DLL_COMPUTER_PLAYER_CONTROLLER_H
#include "REL/m628DLL/act_player.h"
#include "REL/framework/vector3.h"
// Chooses board targets and produces input for a computer player.
struct ComputerPlayerController : public PlayerInputController
{
    ComputerPlayerController(ActPlayer *, s32);
    virtual ~ComputerPlayerController();
    virtual void invoke(StickState *);
    virtual void invoke();
    Vec position;
    s32 selectedIndex;
    s32 unk54; /* The game's code never reads this field. */
    s32 state; // Counts input updates while the player is spawned.
    f32 unk5C; /* The game's code never reads this field. */
    s32 difficulty; // Computer difficulty from comDif.
    typedef void (ComputerPlayerController::*Callback)(StickState *);
    Callback callback; // Runs the current object state.
    // Selects the current input callback.
    void setCallback(Callback value)
    {
        callback = value;
    }
    // Returns the owner player position.
    Vector3 ownerPositionValue() const
    {
        ActPlayer *source = owner;
        return source->position;
    }
    void fn_1_5D6C(StickState *);
};

#endif
