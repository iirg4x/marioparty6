#include "REL/framework/minigame_core.h"
#include "REL/framework/frame_time_inline.h"

#include "REL/framework/linknode.h"
#include "REL/m648DLL/motion_lists.h"
#include "REL/m648DLL/scene.h"

void *operator new(unsigned long);

extern "C" void *fn_1_21C(void *);
extern "C" void fn_1_1AE30(void *, int, unsigned long);

extern "C" void ObjectSetup()
{
    MinigameState *state = new MinigameState;
    state->setup(1, 8192);
}
