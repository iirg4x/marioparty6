#include <new>
#include "REL/framework/minigame_core.h"
#include "REL/framework/frame_durations.h"
#include "REL/m628DLL/scene_thread.h"
#include "dolphin/types.h"

// Creates the game object manager and the minigame scene.
extern "C" void ObjectSetup(void)
{
    SceneThreadOwner *root = new SceneThreadOwner;
    root->setupActorManager();
}

extern "C" const s32 heapSize = 0x100000;
