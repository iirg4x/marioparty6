// Declares the player and computer input controllers.

#ifndef REL_M673DLL_CONTROLLERS_H
#define REL_M673DLL_CONTROLLERS_H

#include "dolphin/types.h"
#include "REL/framework/pad_control.h"
#include "REL/framework/stick_state.h"

class PlayerActor;

// Stores the player's stick state and camera-relative input.
struct PlayerController {
    // Releases the pad input state.
    virtual ~PlayerController();
    // Supplies the stick state for this update.
    virtual void update(StickState *stickState) = 0;
    // Provides the default controller hook with no input change.
    virtual void fn_1_2690();
    // Clears the stick values and updates the controller.
    void fn_1_BDC();
    StickState values; // Input supplied to the player controller.
    u8 unk18[4]; /* No code names this field. */
    PlayerActor *player; // Player actor controlled by this object.
    PadControl input; // Transforms controller input using the camera.
    // Binds the player and initializes pad input for camera mask 1.
    explicit PlayerController(PlayerActor *actor) : player(actor), input(1, 0.0f)
    {
    }
};

// Supplies input from the player's controller.
struct PadController : PlayerController {
    s32 padNumber; // Index of the player's controller pad.
    // Selects the pad assigned to the player.
    explicit PadController(PlayerActor *player);
    // Releases the base controller and its pad state.
    virtual ~PadController()
    {
    }

    // Reads the configured pad's stick values and buttons.
    virtual void update(StickState *stickState);
};

// Selects destinations and steers a computer-controlled player.
struct ComController : PlayerController {
    s32 difficulty; // Difficulty index used for steering parameters.
    f32 steeringParameters[4]; // Steering parameters selected for this difficulty.
    // Speed-scaled direction sent as stick input. Without a new heading, the magnitude
    // test follows normalization, so the 140/60 threshold clears an ordinary unit vector.
    Vec direction;
    f32 speed; // Scale applied to the direction before generating input.
    u16 state; // A successful steering draw suppresses further random draws until reset.
    Vec destination; // Position selected for steering.
    u8 needsDestination; // Requests a new destination.
    void (ComController::*callback)(StickState *); // Current steering update.
    // Selects the steering callback.
    void setUpdate(void (ComController::*method)(StickState *))
    {
        callback = method;
    }

    // Loads the difficulty settings and selects the initial steering callback.
    ComController(PlayerActor *player, s32 computerDifficulty);
    // Releases the base controller and its pad state.
    virtual ~ComController();
    // Saves the player position and passes input to the steering callback.
    virtual void update(StickState *stickState);
    // Computes the desired direction and applies camera-relative stick input.
    void updateDirection(StickState *stickState);
    // Chooses the computer-controlled movement direction using the scene position table.
    Vec fn_1_1414();
};

// Returns the distance at which the computer flees the moving actor.
f32 fn_1_2450(s32 index);

// Returns the steering margin from the side walls.
f32 fn_1_2478(s32 index);

// Returns the steering margin from the end walls.
f32 fn_1_24A0(s32 index);

// Returns the steering interval in seconds for this difficulty.
f32 fn_1_24C8(s32 index);

// Stores the projected point and returns the distance to it. A projection parameter
// outside [0, 1] returns -1 without writing the output. Coincident endpoints leave an
// unguarded zero denominator.
f32 fn_1_24E0(const Vec *point, const Vec *lineStart, const Vec *lineEnd,
              Vec *nearestPoint);

// Sorts the supplied f32 values in ascending order.
void fn_1_25F8(f32 *values, s32 count);

#endif
