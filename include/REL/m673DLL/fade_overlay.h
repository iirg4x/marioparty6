// Declares the scene object that fades a follow effect.

#ifndef REL_M673DLL_FADE_OVERLAY_H
#define REL_M673DLL_FADE_OVERLAY_H
#include <memory>
#include "REL/framework/event_hm.h"
#include "REL/framework/game_object2.h"
#include "REL/m673DLL/follow_effect.h"

// Controls the scene fade quad and its event state.
struct FadeOverlay : GameObject {
    // Holds the position read by the position table. The constructor leaves it uninitialized.
    Vec position;
    std::auto_ptr<FollowEffect> followEffect; // Owns the quad used for the fade overlay.
    f32 colorScale; // Intensity applied to the three color channels.
    EventHandlerManager channel1Events; // Receives overlay events on channel 1.
    typedef void (FadeOverlay::*State)();
    State state; // Current frame update.
    // Creates the scene fade effect and registers its two event callbacks.
    FadeOverlay();
    // Releases the fade effect and its event handler.
    virtual ~FadeOverlay();
    // Runs the overlay's current state callback.
    virtual void update();
    // Shows the overlay and starts its fade-out state.
    void fn_1_DD3C(const EventPacket &);
    // Shows the overlay and starts its fade-in state.
    void fn_1_DDA8(const EventPacket &);
    // Leaves the fade effect unchanged in its idle state.
    void fn_1_DE14();
    // Increases the overlay's color intensity toward full intensity.
    void fn_1_DE18();
    // Reduces the overlay's color intensity and hides it at zero.
    void fn_1_DF58();
    // Selects the callback to run on subsequent updates.
    void setState(State method)
    {
        state = method;
    }
};

#endif
