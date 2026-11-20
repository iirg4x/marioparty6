#include "REL/m611DLL/minigame_thread_owner.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include <new>

// Creates the minigame owner and starts its object manager.
extern "C" void ObjectSetup()
{
    MinigameCore *game = new MinigameThreadOwner;
    game->setup(1, 8192);
}
