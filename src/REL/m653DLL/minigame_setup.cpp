#include "REL/framework/phase_durations.h"
#include "REL/m653DLL/game_controller.h"
void *operator new(unsigned long);

extern "C" void ObjectSetup()
{
    M653GameController *root = new M653GameController;
    root->setup(1, 8192);
}

/* The size of the minigame's heap. */
extern const s32 heapSize = 0x100000;
