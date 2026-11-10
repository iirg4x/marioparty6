/* Note to Self: module entry and exit, and the camera and shadow setup. */
#include "math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/object.h"
#include "game/process.h"
#include "REL/m615DLL.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
void ObjectSetup(void);

/* Runs the module's static constructors, then sets up the module. */
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

/* Runs the module's static destructors. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

extern OMOBJMAN *lbl_1_bss_4;

void ObjectSetup(void)
{
    s16 night;

    lbl_1_bss_4 = fn_80080418();
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 1.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f, 1.0f);
    {
        Point3d shadowPosition = {1000.0f, 2000.0f, 4000.0f};
        Point3d shadowUp = {0.0f, 0.0f, 1.0f};
        Point3d shadowTarget = {0.0f, 0.0f, 0.0f};

        Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
        fn_8002AAB4(0, 0, 0);
        Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    }
    night = lbl_802C0278;
    if (night == 0) {
        Hu3DShadowTPLvlSet(0.4f);
    } else {
        Hu3DShadowTPLvlSet(0.8f);
    }
    fn_8000DAC0(fn_1_26C, 0x64, 0x3000, 0, lbl_1_bss_4);
}

OMOBJMAN *lbl_1_bss_4;
void *lbl_1_bss_0;
