/* Module entry and exit, the camera and shadow setup, the minigame sequence callbacks, and
 * object creation. */
#include "math.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/object.h"
#include "game/process.h"
#include "REL/m607DLL/game.h"
#include "game/mg/seqman.h"
#include "game/main.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
#include "game/memory.h"
#include <string.h>

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

OMOBJMAN *lbl_1_bss_4;
void *lbl_1_bss_0;

const Point3d lbl_1_rodata_10 = { -100.0f, 8000.0f, 0.0f };

const Point3d lbl_1_rodata_1C = { 0.0f, 1.0f, 0.0f };

const Point3d lbl_1_rodata_28 = { 0.0f, 0.0f, 0.0f };

void fn_1_A0(void)
{
    Point3d shadowPosition;
    Point3d shadowUp;
    Point3d shadowTarget;

    lbl_1_bss_4 = fn_80080418();
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f,
                             15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    shadowPosition = lbl_1_rodata_10;
    shadowUp = lbl_1_rodata_1C;
    shadowTarget = lbl_1_rodata_28;
    Hu3DShadowCreate(10.0f, 10.0f, 10000.0f);
    fn_8002AAB4(0, 0, 0);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(0.3f);
    fn_8000DAC0(fn_1_238, 100, 0x3000, 0, lbl_1_bss_4);
}

void fn_1_25C(s16 mode, s16 frameNo);
void fn_1_29C(s16 mode, s16 frameNo);
void fn_1_2A0(s16 mode, s16 frameNo);
void fn_1_2A4(s16 mode, s16 frameNo);
void fn_1_2A8(s16 mode, s16 frameNo);
void fn_1_2AC(s16 mode, s16 frameNo);
void fn_1_2B0(s16 mode, s16 frameNo);
void fn_1_2B4(s16 mode, s16 frameNo);
void fn_1_2B8(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = { 30, 0, fn_1_25C, fn_1_29C, fn_1_2A0, fn_1_2A4, fn_1_2A8, fn_1_2AC, fn_1_2B0, fn_1_2B4, fn_1_2B8 };

void fn_1_238(void)
{
    fn_80070788(&lbl_1_data_0);
    for (;;) {
        fn_800856A8();
        HuPrcVSleep();
    }
}

void fn_1_25C(s16 mode, s16 frameNo)
{
    lbl_1_bss_0 = fn_1_2BC(100, 16, fn_1_B2C);
    fn_80071A9C();
}

void fn_1_29C(s16 mode, s16 frameNo)
{
}

void fn_1_2A0(s16 mode, s16 frameNo)
{
}

void fn_1_2A4(s16 mode, s16 frameNo)
{
}

void fn_1_2A8(s16 mode, s16 frameNo)
{
}

void fn_1_2AC(s16 mode, s16 frameNo)
{
}

void fn_1_2B0(s16 mode, s16 frameNo)
{
}

void fn_1_2B4(s16 mode, s16 frameNo)
{
}

void fn_1_2B8(s16 mode, s16 frameNo)
{
}

void *fn_1_2BC(s32 priority, u32 bytes, void (*update)(OMOBJ *));

void *fn_1_2BC(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = fn_8003AB38(lbl_1_bss_4, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, 0x10000000);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

void fn_1_35C(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}
