#include "REL/m624DLL/sequence_owner.h"
#include "REL/framework/phase_durations.h"

void *operator new(unsigned long);
extern "C" void ObjectSetup(void)
{
    SequenceOwner *root = new SequenceOwner;
    root->setupActorManager();
}
