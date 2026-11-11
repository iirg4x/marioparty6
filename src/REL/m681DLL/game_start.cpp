#include "REL/m681DLL.h"
#include "dolphin/types.h"
#include "REL/framework/phase_durations.h"
#include "REL/m681DLL/game_sequence.h"
extern "C" {
static const s16 panByPlayer[4] = { 0x20, 0x30, 0x50, 0x60 };
extern const s32 heapSize = 0x40000;

}
s16 fn_1_2508(s32 index) {
    s32 clampedIndex;

    if (index < 0) {
        clampedIndex = 0;
    } else if (index > 4) {
        clampedIndex = 4;
    } else {
        clampedIndex = index;
    }
    return (s16)panByPlayer[clampedIndex];
}
extern "C" {
}


void *operator new(unsigned long);

/* Creates the module root object and installs it as the primary manager. */
extern "C" void ObjectSetup()
{
    GameSequenceController *root = new GameSequenceController;
    root->setup(1, 8192);
}
