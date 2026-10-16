/* Creates the scene and runs the minigame's sequence and actor updates. */
#include <math.h>
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "msm_stream.h"
#include <string.h>

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

void fn_1_148(void);
void fn_1_2CC(void);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_368(s16 mode, s16 frameNo);
void fn_1_36C(s16 mode, s16 frameNo);
void fn_1_3C8(s16 mode, s16 frameNo);
void fn_1_3CC(s16 mode, s16 frameNo);
void fn_1_420(s16 mode, s16 frameNo);
void fn_1_424(s16 mode, s16 frameNo);
void fn_1_428(s16 mode, s16 frameNo);
void fn_1_42C(s16 mode, s16 frameNo);
void fn_1_798(OMOBJ *object);

MGSEQ_PARAM lbl_1_data_0 = {
    300, MGSEQ_TIMER_TOP,
    fn_1_2F0, fn_1_368, fn_1_36C, fn_1_3C8, fn_1_3CC,
    fn_1_420, fn_1_424, fn_1_428, fn_1_42C
};
s32 lbl_1_data_28 = -1;
OMOBJMAN *lbl_1_bss_0;

/* Runs module constructors, then starts the minigame's shared scene setup. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_148();
    return 0;
}

/* Runs module destructors when the minigame module is unloaded. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

/* Module setup helper: creates an update object and returns its zeroed work data, or NULL
 * when no work bytes are requested. */
void *fn_1_A0(s32 priority, u32 bytes, OMOBJ_FUNC update)
{
    OMOBJ *object;

    object = omAddObjEx(lbl_1_bss_0, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, HU_MEMNUM_OVL);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Called by the module prolog to set lighting and shadows before starting the sequence process. */
void fn_1_148(void)
{
    HU3D_LIGHTID directionalLight;

    lbl_1_bss_0 = MgActorObjectSetup();
    directionalLight = Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f,
        -1.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(directionalLight);
    {
        HuVecF shadowCameraPosition = { -100, 8000, 0 };
        HuVecF shadowUp = { 0, 1, 0 };
        HuVecF shadowCameraTarget = { 0, 0, 0 };

        Hu3DShadowMultiCreate(24.0f, 10.0f, 10000.0f, 3);
        Hu3DShadowMultiColSet(16, 16, 16, 3);
        Hu3DShadowMultiPosSet(&shadowCameraPosition, &shadowUp,
                              &shadowCameraTarget, 3);
        Hu3DShadowMultiTPLvlSet(0.3f, 3);
    }
    HuPrcChildCreate(fn_1_2CC, 100, 12288, 0, lbl_1_bss_0);
}

/* Runs as the child process created during setup, advancing actors once per scheduler pass. */
void fn_1_2CC(void)
{
    MgSeqCreate(&lbl_1_data_0);
    for (;;) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* Sequence initialization hook: creates the update object and advances into the opening phase. */
void fn_1_2F0(s16 mode, s16 frameNo)
{
    fn_1_A0(100, 8, fn_1_798);
    MgSeqModeNext();
}

void fn_1_368(s16 mode, s16 frameNo)
{
}

/* MGSEQ_PARAM start hook: starts the streamed minigame track when its announcement requests an
 * effect. */
void fn_1_36C(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 == -1 &&
        (s32)(GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        lbl_1_data_28 = HuAudSStreamPlay(MSM_STREAM_MGMUS_21);
    }
}

void fn_1_3C8(s16 mode, s16 frameNo)
{
}

/* Sequence finish hook: fades the minigame music and clears its channel handle. */
void fn_1_3CC(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
}

void fn_1_420(s16 mode, s16 frameNo)
{
}

void fn_1_424(s16 mode, s16 frameNo)
{
}

void fn_1_428(s16 mode, s16 frameNo)
{
}

void fn_1_42C(s16 mode, s16 frameNo)
{
}
