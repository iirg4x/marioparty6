extern const int heapSize = 0x20000;

#include "math.h"
#include "REL/m652DLL/module_root.h"
#include "REL/framework/phase_durations.h"

extern "C" void ObjectSetup(void) {
    ModuleRoot* root = new ModuleRoot;
    root->setupActorManager();
}
