// Defines the module heap size.

#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/frame_time.h"
#include "REL/m673DLL/minigame_thread.h"

// Heap allocation requested by the minigame.
extern const s32 heapSize = 0x100000;

// Creates the minigame thread and initializes its actor manager.
extern "C" void ObjectSetup()
{
    MinigameThread *root = new MinigameThread();
    root->setupActorManager();
}
