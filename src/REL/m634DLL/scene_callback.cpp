#include "REL/m634DLL/TextureController.h"
#include "REL/m634DLL/render_owners.h"
#include "dolphin/types.h"
#include "REL/m634DLL/game_manager.h"

// Creates the scene renderers and secondary motion effect.
void GameManager::fn_1_C7F0()
{
    new TextureController();
    new Camera1Renderer();
    new Camera2Renderer();
    MotionEffect *replacement = new MotionEffect(animations[2], 0x50, 5);
    secondaryResource.reset(replacement);
    MotionEffect *replacement2 = new MotionEffect(animations[2], 0x50, 7);
    primaryResource.reset(replacement2);
    new SpriteRenderer(animations[3]);
}

// Dispatches the event to the appropriate scene handler.
void GameManager::receive(const EventPacket *event)
{
    switch (event->channel) {
    case 1:
        fn_1_CA30(event);
        return;
    case 2:
        fn_1_CCA8(event);
    }
}
