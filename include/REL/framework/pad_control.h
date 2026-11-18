#ifndef REL_FRAMEWORK_PAD_CONTROL_H
#define REL_FRAMEWORK_PAD_CONTROL_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include <math.h>
#include "REL/framework/range.h"
extern "C" {
#include "game/mg/actman.h"
}

/* Converts camera-relative movement into player stick and button input. */
class PadControl {
public:
    /* Initializes the camera mask and angular response. */
    PadControl(u16 cameraBit, f32 value);
    /* Releases the angular response state. */
    ~PadControl();

    u16 cameraBit; /* Camera mask used to transform world-space movement. */
    Range<f32> range; /* Current angle and its response limits. */
    f32 stickX;
    f32 stickY;
    u32 pressedButtons;
    u32 heldButtons;

    /* Resets the angular response and accumulated input. */
    void reset(f32 value);
    /* Adds buttons to the held input. */
    void holdButtons(u32 flags);
    /* Adds buttons to the pressed input. */
    void pressButtons(u32 flags);
    /* Presses the B button. */
    void pressB();
    /* Presses the A button. */
    void pressA();
    /* Holds the A button. */
    void holdA();
    /* Turns world-space movement into camera-relative stick input. */
    void updateMovement(const Vec *movement, f32 amount);
    /* Sends the accumulated input to a player, then decays it. */
    void applyToPlayer(MGPLAYER *player);
    /* Decays stick input and clears accumulated button input. */
    void decay();
};

#endif
