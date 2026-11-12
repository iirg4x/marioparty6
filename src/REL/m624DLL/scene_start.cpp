#include "REL/m624DLL/sequence_owner.h"
#include "REL/framework/phase_durations.h"

extern "C" void ObjectSetup(void)
{
    SequenceOwner *root = new SequenceOwner;
    root->setupActorManager();
}
