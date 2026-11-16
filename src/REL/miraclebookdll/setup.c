// Miracle Book overlay startup and shared state declarations.
#include "dolphin.h"

#include "game/audio.h"

#include "game/data.h"

#include "game/esprite.h"

#include "game/gamework.h"

#include "game/hu3d.h"

#include "game/hsfex.h"

#include "game/mgdata.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/pad.h"

#include "game/process.h"

#include "game/window.h"

#include "game/wipe.h"

#include "REL/miraclebookdll/book.h"

typedef void (*VoidFunc)(void);

typedef struct {
    HuVecF entries[5];
} MIRACLEBOOK_VEC_TABLE;

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void ObjectSetup(void);

void fn_1_1CC(void);

OMOBJMAN *lbl_1_bss_4;

s32 lbl_1_bss_0;

// Runs on overlay load, invoking registered constructors before initializing the scene.
s32 _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors) {
        (*ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

// Runs on overlay unload to invoke registered destructors.
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (*dtors)();
        dtors++;
    }
}

// Called by the overlay prolog to create the scene manager, lighting and entry process.
void ObjectSetup(void)
{
    HuVecF lightPos = {0.0f, 1000.0f, 5000.0f};
    HuVecF lightDir = {0.0f, -1.0f, -1.0f};
    GXColor lightColor = {255, 255, 255, 255};
    HU3D_LIGHTID light;

    lbl_1_bss_4 = fn_8003A8BC(200, 8192);
    fn_80069F1C(lbl_1_bss_4);
    light = fn_800286D0(&lightPos, &lightDir, &lightColor);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightInfinitytSet(light);
    fn_1_2AC(lbl_1_bss_4);
    fn_8000DAC0(fn_1_1CC, 256, 12288, 0, fn_8000DD9C());
    lbl_1_bss_0 = -1;
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
}

// Runs as the overlay child process, advancing entry and exit wipes once per scheduler pass.
void fn_1_1CC(void)
{
    switch (lbl_1_bss_0) {
        case -1:
        case 0:
            if (!WipeCheck()) {
                lbl_1_bss_0 += 2;
            }
            break;
        case 1:
            if (fn_1_394() || omSysExitReq) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                lbl_1_bss_0 = 0;
            }
            break;
        case 2:
            fn_1_434();
            HuAudAllStop();
            fn_80026648();
            fn_80028D70();
            fn_8003A5E4(1, TRUE);
            break;
    }
    HuPrcVSleep();
}
