#include <new>
#include "dolphin/types.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/m672DLL/scene_thread_owner.h"

/* Capacity of the minigame heap used by initializeMemoryManager. */
extern const s32 heapSize = 0x100000;

// Create the minigame scene owner and initialize its actor manager.
extern "C" void ObjectSetup() {
    SceneOwner* root=new SceneOwner;
    root->setupActorManager();
}
