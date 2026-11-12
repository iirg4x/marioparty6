#include "REL/m604DLL/manager_owner.h"
#include "REL/m604DLL/patapata.h"
#include "REL/framework/phase_durations.h"

extern "C" void ObjectSetup()
{
    CleanupSequenceOwner *root = new CleanupSequenceOwner();
    root->setup(1, 8192);
}

extern "C" const s32 heapSize = 256 * 1024;
