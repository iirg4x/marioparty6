// Declares the scene billboard and its update states.

#ifndef REL_M673DLL_SCENE_BILLBOARD_H
#define REL_M673DLL_SCENE_BILLBOARD_H
#include <memory>
#include "REL/framework/event_hm.h"
#include "REL/framework/game_object2.h"
#include "REL/m673DLL/follow_effect.h"

// Controls an animated quad anchored in the scene.
struct SceneBillboard : GameObject {
    Vec position; // Anchors the effect and supplies a position to the scene table.
    std::auto_ptr<FollowEffect> followEffect; // Owns the textured quad drawn at the billboard
                                              // position.
    u8 active; // Allows the ending growth state to start.
    f32 scale; // Current size of the effect.
    f32 scaleLimit; // Caps the scale during ordinary growth.
    EventHandlerManager channel1Events; // Receives billboard events on channel 1.
    typedef void (SceneBillboard::*State)();
    State state; // Current frame update.
    // Creates a billboard at the supplied position and registers its events.
    SceneBillboard(const Vec *position, u8 active);
    // Releases the billboard effect and its event handlers.
    virtual ~SceneBillboard();
    // Runs the billboard's current state callback.
    virtual void update();
    // Starts the billboard's fade-out state.
    void fn_1_D5EC(const EventPacket &event);
    // Shows the billboard and starts its growth state.
    void fn_1_D630(const EventPacket &event);
    // Shows an active billboard and starts its ending growth state.
    void fn_1_D69C(const EventPacket &event);
    // Leaves the billboard unchanged in its idle state.
    void fn_1_D714();
    // Grows the billboard to its scale limit.
    void fn_1_D718();
    // Reduces the billboard's scale and hides it at the end of the fade.
    void fn_1_D7C8();
    // Grows the billboard toward its ending size.
    void fn_1_D8B0();
    // Selects the callback to run on subsequent updates.
    void setState(State method)
    {
        state = method;
    }
};

#endif
