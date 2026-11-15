template <class Time> Time secondsToFrames(Time seconds);
#include "REL/framework/minigame_core.h"
#include "REL/m634DLL/scene_owner_vtable.h"
void *operator new(unsigned long);

extern "C" void ObjectSetup(void)
{
    SceneObjectOwner *object = new SceneObjectOwner;
    object->setupActorManager();
}

extern int lbl_1_bss_68;

extern int lbl_1_bss_6C;

extern unsigned int lbl_1_bss_70;

/* .rodata+0x7A0: module entry fn_1_1586C consumes this capacity. */
extern const s32 heapSize = 0x100000;

struct SetupTimingInterval {
    double seconds;

    u8 unknown08[0x10]; /* No code names this field. */

};
extern const SetupTimingInterval lbl_1_rodata_7A8 = {
    3.5, {0x40, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08}
};

int lbl_1_bss_68 = secondsToFrames(2L);
int lbl_1_bss_6C = (int)secondsToFrames(lbl_1_rodata_7A8.seconds);
unsigned int lbl_1_bss_70 = (unsigned int)(int)secondsToFrames(lbl_1_rodata_7A8.seconds);
