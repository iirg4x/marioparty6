// Minigame entry point and its shared memory-manager setup.

#include "REL/framework/frame_durations.h"
#include <new>
#include "REL/framework/minigame_core.h"
#include "REL/framework/event_manager.h"
extern "C" {
#include "dolphin/types.h"
}

#include "REL/m623DLL/scene_object.h"

// Backing capacity for the minigame memory manager.
extern "C" const s32 heapSize = 0x40000;

// Creates the scene root and initializes its object manager.
extern "C" void ObjectSetup()
{
    MinigameCore *root = new SceneObject;
    root->setup(1, 8192);
}
