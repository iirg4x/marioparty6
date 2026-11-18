#ifndef REL_M628DLL_COMPUTER_LEVER_CONTROLLER_H
#define REL_M628DLL_COMPUTER_LEVER_CONTROLLER_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m628DLL/player_actor.h"

// Selects a target position and produces stick input for the lever actor.
struct ComputerLeverController : public ActorController
{
    ComputerLeverController(PlayerActor *, s32);
    virtual ~ComputerLeverController();
    virtual void invoke(StickState *);
    f32 elapsed;
    f32 duration;
    s32 positionChoice; // Index of the current computer-controlled target.
    s32 difficulty;     // Computer-input difficulty used by target selection.
    s32 requestCount;   // Number of target choices made by the controller.
    f32 speed;
    Vec stickInput;
    typedef void (ComputerLeverController::*PositionCallback)(StickState *output);
    PositionCallback callback; // Runs the current object state.
    // Selects the current input callback.
    void setCallback(PositionCallback value)
    {
        callback = value;
    }
    void fn_1_6564(StickState *output);
    void fn_1_65F0(StickState *output);
    // Stores the three synthesized stick-input components.
    void setStickInput(f64 x, f64 y, f64 z)
    {
        stickInput.x = x;
        stickInput.y = y;
        stickInput.z = z;
    }
};

#endif
