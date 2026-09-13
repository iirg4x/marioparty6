#include "REL/m621dll.h"
#include "game/mg/actman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "string.h"

MGSEQ_PARAM lbl_1_data_0 = {
    30, 1, fn_1_2E0, fn_1_358, fn_1_35C, fn_1_3B8, fn_1_3BC,
    fn_1_410, fn_1_414, fn_1_418, fn_1_41C
};
s32 lbl_1_data_28 = -1;

void *fn_1_A0(s32 priority, u32 size, OMOBJ_FUNC callback)
{
    OMOBJ *obj;

    obj = omAddObjEx(lbl_1_bss_0, priority, 0, 0, 0, callback);
    if (size != 0) {
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
        memset(obj->data, 0, size);
    } else {
        obj->data = NULL;
    }
    return obj->data;
}

void fn_1_140(OMOBJ *obj, OMOBJ_FUNC callback)
{
    obj->objFunc = callback;
}

void fn_1_148(void)
{
    HU3D_LIGHTID light;

    lbl_1_bss_0 = MgActorObjectSetup();
    light = Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f,
        1.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(light);
    {
        HuVecF shadowPos = { -100, 8000, 0 };
        HuVecF shadowUp = { 0, 1, 0 };
        HuVecF shadowTarget = { 0, 0, 0 };

        Hu3DShadowCreate(15.0f, 10.0f, 10000.0f);
        Hu3DShadowColSet(16, 16, 16);
        Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
        Hu3DShadowTPLvlSet(0.3f);
    }
    HuPrcChildCreate(fn_1_2BC, 100, 12288, 0, lbl_1_bss_0);
}

void fn_1_2BC(void)
{
    MgSeqCreate(&lbl_1_data_0);
    while (TRUE) {
        MgActorExec();
        HuPrcVSleep();
    }
}

void fn_1_2E0(s16 mode, s16 frameNo)
{
    fn_1_A0(100, 8, fn_1_5F0);
    MgSeqModeNext();
}

void fn_1_358(s16 mode, s16 frameNo)
{
}

void fn_1_35C(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY)) {
        lbl_1_data_28 = HuAudSStreamPlay(72);
    }
}

void fn_1_3B8(s16 mode, s16 frameNo)
{
}

void fn_1_3BC(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
}

void fn_1_410(s16 mode, s16 frameNo)
{
}

void fn_1_414(s16 mode, s16 frameNo)
{
}

void fn_1_418(s16 mode, s16 frameNo)
{
}

void fn_1_41C(s16 mode, s16 frameNo)
{
}
