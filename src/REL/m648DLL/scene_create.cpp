#include "REL/framework/minigame_core.h"
#include "REL/framework/frame_time_inline.h"

#include "REL/framework/linknode.h"
#include "REL/m648DLL/scene.h"

/* Creates the minigame state and starts its main thread. */
extern "C" void ObjectSetup()
{
    MinigameState *state = new MinigameState;
    state->setup(1, 8192);
}
