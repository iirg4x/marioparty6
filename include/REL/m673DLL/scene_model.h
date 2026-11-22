// Declares the animated scene model and its update states.

#ifndef REL_M673DLL_SCENE_MODEL_H
#define REL_M673DLL_SCENE_MODEL_H
#include <memory>
#include "REL/framework/event_hm.h"
#include "REL/framework/game_object2.h"
#include "REL/framework/model.h"

// Controls the position and animation of a scene model.
struct SceneModel : GameObject {
    Vec unk10; /* The game's code never reads this field. */
    std::auto_ptr<OwnedModel> model; // Owns the model drawn by the scene object.
    u8 active; // A nonzero value selects the larger initial scale and allows the ending callback.
    f32 scale; // Drives the current model animation time.
    f32 scaleLimit; // Caps the scale during ordinary growth.
    f32 motionTime; // Frame applied to the model on update.
    EventHandlerManager channel1Events; // Receives the model's scene events.
    // Binds the scene model to its position and animation events.
    SceneModel(const OwnedModel *model, Vec *position, u8 active);
    // Releases the scene model instance and its event handlers.
    virtual ~SceneModel();
    // Advances the scene model's state and sets its animation time.
    virtual void update();
    typedef void (SceneModel::*State)();
    State state; // Current frame update.
    // Starts the scene model's shrinking animation.
    void fn_1_CE04(const EventPacket &event);
    // Starts the scene model's growth animation.
    void fn_1_CE48(const EventPacket &event);
    // Installs the ending animation callback when active is nonzero.
    void fn_1_CE8C(const EventPacket &event);
    // Leaves the scene model unchanged in its idle state.
    void fn_1_CEDC();
    // Advances ordinary growth and animation time only when active differs from 1.
    void fn_1_CEE0();
    // Shrinks the scene model to zero and updates its animation time.
    void fn_1_CF9C();
    // Grows the scene model to its ending scale and updates its animation time.
    void fn_1_D05C();
    // Selects the callback to run on subsequent updates.
    void setState(State method)
    {
        state = method;
    }
};

#endif
