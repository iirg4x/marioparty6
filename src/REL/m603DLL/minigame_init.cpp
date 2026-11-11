#include "REL/framework/frame_time_inline.h"
#include "REL/m603DLL/flow.h"

extern "C" void ObjectSetup()
{
    MinigameThreadOwner *root=new MinigameThreadOwner();
    root->setup(1,8192);
}

extern "C" {
#include "REL/m603DLL.h"
}
