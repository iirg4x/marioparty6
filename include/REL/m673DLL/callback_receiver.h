// Declares the receiver that captures the scene texture.

#ifndef REL_M673DLL_CALLBACK_RECEIVER_H
#define REL_M673DLL_CALLBACK_RECEIVER_H
#include "REL/framework/game_object2.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/singleton.h"

// Handles scene events for the framebuffer capture object.
struct CallbackReceiver : MinigameSingleton<CallbackReceiver>, RenderObject {
    GXTexObj texture; // Texture object backed by the framebuffer copy.
    // Texture-copy storage must be aligned to a 32-byte boundary.
    u8 pixels[640 * 480 * 2] ATTRIBUTE_ALIGN(32);
    u8 unk96040; /* The game's code never reads this field. */
    EventHandlerManager channel1Events; // Receives scene events on channel 1.
    // Registers the receiver for scene events.
    CallbackReceiver();
    // Releases the scene event handlers and render object.
    virtual ~CallbackReceiver();
    // Leaves capture to the draw callback.
    virtual void update();
    // Configures projected texture coordinates and captures the scene texture.
    virtual void draw(Mtx *modelView);
    // Sets the receiver flag in response to a scene event.
    void fn_1_E4BC(const EventPacket &event);
    // Clears the receiver flag in response to a scene event.
    void fn_1_E4CC(const EventPacket &event);
};

#endif
