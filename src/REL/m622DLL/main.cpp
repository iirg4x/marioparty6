#include "REL/framework/minigame_core.h"
#include "REL/framework/phase_durations.h"
#include "REL/m622DLL/minigame_owner.h"

/* The REL entry the framework startup calls: creates the minigame and its object manager. */
extern "C" void ObjectSetup()
{
    MinigameThreadOwner *root = new MinigameThreadOwner;
    root->setup(1, 8192);
}

#include "dolphin/types.h"
/* The model-heap size framework heap.cpp reserves for this minigame's C++ objects. */
extern const s32 heapSize = 0x64000;
