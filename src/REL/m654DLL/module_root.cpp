#include "REL/framework/phase_durations.h"
#include "REL/m654DLL/scene_owner.h"

// Creates the scene owner for the minigame framework.
extern "C" void ObjectSetup() {
    SceneObjectOwner *root = new SceneObjectOwner;
    root->setupActorManager();
}

// Heap capacity for the framework allocator.
extern const s32 heapSize = 0x20000;
