#ifndef REL_M628DLL_LIGHT_SETUP_H
#define REL_M628DLL_LIGHT_SETUP_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"

// Owns the scene light and its event state.
class LightSetup : public GameObject
{
  public:
    LightSetup(s16 *modelId);
    virtual ~LightSetup();
    virtual void update();
    s16 lightId;
    Vec position;
    f32 elapsed;
    s32 unk24; /* The game's code never reads this field. */
    s32 frame; // Counts frames in the current light state.
    f32 unk2C; /* The game's code never reads this field. */
    typedef void (LightSetup::*Callback)();
    Callback callback; // Runs the current object state.
    // Selects the current update callback.
    void setCallback(Callback value)
    {
        callback = value;
    }
    EventHandlerManager firstCallbackOwner;  // Receives the first light event channel.
    EventHandlerManager secondCallbackOwner; // Receives the second light event channel.
    void fn_1_1305C(const EventPacket &argument);
    void fn_1_130A0(const EventPacket &argument);
    void fn_1_130F4(const EventPacket &argument);
    void fn_1_1312C();
    void fn_1_13130();
    void fn_1_132BC();
};

#endif
