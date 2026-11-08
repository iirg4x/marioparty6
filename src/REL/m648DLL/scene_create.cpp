#include "REL/framework/minigame_core.h"

#include "REL/m648DLL/framework/linknode.h"
#include "REL/m648DLL/motion_lists.h"

struct KuriboListSentinel : Node { ~KuriboListSentinel(); };

KuriboListSentinel::~KuriboListSentinel() {}

void *operator new(unsigned long);

extern "C" void *fn_1_21C(void *);
extern "C" void fn_1_1AE30(void *, int, unsigned long);

extern "C" { void *__ct__13MinigameStateFv(void *); }

extern "C" void ObjectSetup()
{
    void *constructed = ::operator new(880);
    if (constructed != 0) constructed = __ct__13MinigameStateFv(constructed);
    void *root = constructed;
    static_cast<MinigameCore *>(root)->setup(1, 8192);
}
