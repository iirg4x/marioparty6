template <class Time> Time secondsToFrames(Time seconds);
#include "REL/framework/minigame_core.h"
#include <new>
#include "REL/m634DLL/scene_owner_vtable.h"

// Creates the scene owner and starts its actor manager.
extern "C" void ObjectSetup(void)
{
    SceneObjectOwner *object = new SceneObjectOwner;
    object->setupActorManager();
}

extern int lbl_1_bss_68;

extern int lbl_1_bss_6C;

extern unsigned int lbl_1_bss_70;

extern const s32 heapSize = 0x100000;

struct SetupTimingInterval {
    double seconds;

    double framesPerSecond;
    u16 intervals[4];
};
extern const SetupTimingInterval lbl_1_rodata_7A8 = { 3.5, 60.0, { 1, 2, 4, 8 } };

int lbl_1_bss_68 = secondsToFrames(2L);
int lbl_1_bss_6C = (int)secondsToFrames(lbl_1_rodata_7A8.seconds);
unsigned int lbl_1_bss_70 = (unsigned int)(int)secondsToFrames(lbl_1_rodata_7A8.seconds);
