#include "REL/framework/phase_duration_statics.h"
#include <new>
#include "REL/framework/frame_time.h"
#include "REL/framework/minigame_core.h"
#include "REL/m610DLL/scene_owner.hpp"

/* Heap extent requested by this minigame. */
extern const s32 heapSize = 0x100000;

/* Create and set up the minigame scene owner. */
extern "C" void ObjectSetup()
{
    SceneObjectOwner *root = new SceneObjectOwner;
    root->setup(1, 8192);
}
