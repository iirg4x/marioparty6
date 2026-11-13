#include "REL/framework/frame_time_inline.h"
#include "REL/m603DLL/flow.h"

/* Create and start the game-thread owner. */
extern "C" void ObjectSetup()
{
    MinigameThreadOwner *root=new MinigameThreadOwner();
    root->setup(1,8192);
}
