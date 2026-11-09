#include "REL/m603DLL/frame_time.h"
#include "REL/m603DLL/flow.h"

extern "C" void ObjectSetup()
{
    MinigameThreadOwner *root=new MinigameThreadOwner();
    root->setup(1,8192);
}

extern "C" {
#include "REL/m603DLL.h"


long lbl_1_bss_30 = secondsToFrames(2L);
long lbl_1_bss_34 = (long)secondsToFrames(3.5);
long lbl_1_bss_38 = (long)secondsToFrames(3.5);
}
