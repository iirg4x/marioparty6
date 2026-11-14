/* Module entry points and lifecycle callbacks for the microphone activity. */
#include "REL/mikeactdll/module.h"
#include "REL/mikeactdll/model_hook_setup.h"
#include "REL/mikeactdll/event_callback.h"
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "REL/mikeactdll/module_types.h"

#include "dolphin.h"

#include "game/armem.h"

#include "game/board/audio.h"

#include "game/hu3d.h"

#include "game/process.h"

#include "game/window.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

/* Run static constructors and start the activity processes. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

extern const VoidFunc _dtors[];

/* Run static destructors when the module is unloaded. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

typedef struct Process_s HUPROCESS;

HUPROCESS *fn_80080418(void);

HUPROCESS *fn_8000DAC0(void (*func)(void), u16 priority, u32 stackSize, s32 extraSize, HUPROCESS *parent);

extern HUPROCESS *lbl_1_bss_6C;

extern u16 lbl_1_bss_34;

extern u16 lbl_1_bss_6A;

extern u16 lbl_1_bss_68;

extern void (*lbl_1_bss_24)(void);

extern void (*lbl_1_bss_20)(void);

void fn_1_320(void);

void ObjectSetup(void)
{
    u16 normalizedMode;
    s16 gameMode;

    lbl_1_bss_6C = (HUPROCESS *) fn_80080418();
    fn_8000DAC0(fn_1_534, 0x0BB8, 0x4000, 0, lbl_1_bss_6C);
    fn_8000DAC0(fn_1_320, 0x07D0, 0x4000, 0, lbl_1_bss_6C);
    gameMode = lbl_80265B70.subGameNo;
    lbl_1_bss_34 = (u16) gameMode;
    if (lbl_1_bss_34 > 2) {
        lbl_1_bss_34 = 0;
    }
    normalizedMode = lbl_1_bss_34;
    lbl_80265B70.subGameNo = (s16) normalizedMode;
    lbl_1_bss_6A = 0;
    lbl_1_bss_68 = 0;
    lbl_1_bss_24 = 0;
    lbl_1_bss_20 = 0;
}

/* Initialize the camera, players, interface, and microphone state for a new activity. */
void fn_1_1C0(void) {
    MikeActActivityInit();
}

extern MGTIMER *lbl_1_bss_3C;

void fn_1_220(void) {
    fn_1_1AE4();
    MgTimerKill(lbl_1_bss_3C);
}

extern u16 lbl_1_bss_3A;

extern u16 lbl_1_bss_38;

void fn_1_250(OMOBJ *object)
{
    void (*callback)(void);

    if (lbl_1_bss_24 != 0) {
        callback = lbl_1_bss_24;
        callback();
    }
    fn_800856A8();
    if (lbl_1_bss_20 != 0) {
        callback = lbl_1_bss_20;
        callback();
    }
    lbl_802C0010[0] = lbl_1_bss_3A;
    lbl_802C0008[0] = lbl_1_bss_38;
}
