#include "REL/framework/minigame_core.h"
#include "REL/m622DLL/frame_time.h"
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

s32 lbl_1_bss_B8 = secondsToFrames(2L);
s32 lbl_1_bss_BC = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_C0 = (s32)secondsToFrames(3.5);
