/* Sequence hooks and object-manager setup for the minigame overlay. */
#include "REL/m621dll.h"
#include "game/mg/actman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "string.h"

#define M621_START_STREAM_ID 72

OMOBJMAN *lbl_1_bss_0;

MGSEQ_PARAM lbl_1_data_0 = {
    30, 1, fn_1_2E0, fn_1_358, fn_1_35C, fn_1_3B8, fn_1_3BC,
    fn_1_410, fn_1_414, fn_1_418, fn_1_41C
};
s32 lbl_1_data_28 = -1;

/* Add an overlay object and, when requested, allocate and clear its callback data. */
void *fn_1_A0(s32 priority, u32 dataSize, OMOBJ_FUNC callback)
{
    OMOBJ *obj;

    obj = omAddObjEx(lbl_1_bss_0, priority, 0, 0, 0, callback);
    if (dataSize != 0) {
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, dataSize, HU_MEMNUM_OVL);
        memset(obj->data, 0, dataSize);
    } else {
        obj->data = NULL;
    }
    return obj->data;
}

/* Replace an overlay object's update callback during its state transitions. */
void fn_1_140(OMOBJ *obj, OMOBJ_FUNC callback)
{
    obj->objFunc = callback;
}

/* Create the actor object manager, scene lighting and shadow, then start the minigame process. */
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

/* Create the minigame sequence and run actor updates once per process wakeup. */
void fn_1_2BC(void)
{
    MgSeqCreate(&lbl_1_data_0);
    while (TRUE) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* Sequence init hook: create the round controller before advancing to the next mode. */
void fn_1_2E0(s16 mode, s16 frameNo)
{
    fn_1_A0(100, 8, fn_1_5F0);
    MgSeqModeNext();
}

/* Sequence fade-in hook: this overlay has no additional fade-in work. */
void fn_1_358(s16 mode, s16 frameNo)
{
}

/* Sequence start hook: play the start-phase audio stream once the start announcement sets its
 * effect-play flag. */
void fn_1_35C(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY)) {
        lbl_1_data_28 = HuAudSStreamPlay(M621_START_STREAM_ID);
    }
}

/* Sequence main hook: per-frame gameplay is handled by overlay objects instead. */
void fn_1_3B8(s16 mode, s16 frameNo)
{
}

/* Sequence finish hook: request a fade for the start-phase audio stream when its handle is set,
 * then clear the stored handle. */
void fn_1_3BC(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
}

/* Sequence pre-winner hook: no extra work is needed before winner selection. */
void fn_1_410(s16 mode, s16 frameNo)
{
}

/* Sequence winner hook: winner animations are handled by each player's object. */
void fn_1_414(s16 mode, s16 frameNo)
{
}

/* Sequence fade-out hook: this overlay has no additional fade-out work. */
void fn_1_418(s16 mode, s16 frameNo)
{
}

/* Sequence close hook: no extra overlay cleanup is needed here. */
void fn_1_41C(s16 mode, s16 frameNo)
{
}
