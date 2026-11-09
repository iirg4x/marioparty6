extern const int heapSize = 0x20000;

#include "math.h"
#include "REL/framework/minigame_core.h"
#include "REL/m652DLL/framework/thread.h"

void* operator new(unsigned long);
struct RootContainer;

struct ModuleRoot : MinigameCore, ThreadEntry {
    RootContainer* container;
    ModuleRoot();
    virtual void setupObjects(OMOBJMAN*);
    virtual void finishGame();
    virtual void run(void*);
};

extern "C" void ObjectSetup(void) {
    ModuleRoot* root = new ModuleRoot;
    root->setupActorManager();
}
