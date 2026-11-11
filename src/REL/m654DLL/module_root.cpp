#include "REL/framework/phase_durations.h"
#include "REL/m654DLL/scene_owner.h"

extern "C" void ObjectSetup() {
    SceneObjectOwner* root=new SceneObjectOwner;
    root->setupActorManager();
}

// Heap capacity read by the framework memory-manager initializer at .text 0xC0E0.
extern const s32 heapSize = 0x20000;
