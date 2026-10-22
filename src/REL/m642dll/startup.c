/* Loader entry points and sequence setup for the Cashapult minigame. */
#include "dolphin/math.h"
#include "REL/m642dll/work.h"
#include "game/memory.h"
#include "msm_stream.h"
#include <string.h>
#include "include/game/mg/actman.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_148(void);

/* Runs when the minigame REL is loaded, before its sequence starts. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_148();
    return 0;
}

/* Runs when the minigame REL is unloaded and releases its registered static objects. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

M642ObjectManagerState lbl_1_bss_0;

/* Creates a manager object and optionally gives it zeroed work storage. */
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = omAddObjEx(lbl_1_bss_0.manager, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, HU_MEMNUM_OVL);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

/* Replaces an object's update callback during scene setup. */
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

void fn_1_19C(void);

/* Creates the actor manager and starts the sequence process during REL startup. */
void fn_1_148(void)
{
    lbl_1_bss_0.manager = MgActorObjectSetup();
    MgActorColCylReset();
    HuPrcChildCreate(fn_1_19C, 100, 12288, 0, lbl_1_bss_0.manager);
}

/* MgSeqCreate consumes the sequence parameter at offset zero. */
typedef struct M642SequenceDataView_s {
    MGSEQ_PARAM param;
} M642SequenceDataView;
void fn_1_1C0(s16 mode, s16 frameNo);
void fn_1_1F4(void);
void fn_1_1F8(void);
void fn_1_254(s16 mode, s16 frameNo);
void fn_1_280(void);
void fn_1_2E0(s16 mode, s16 frameNo);
void fn_1_30C(void);
void fn_1_310(void);
void fn_1_314(void);

M642SequenceDataView lbl_1_data_0 = {{
    0,
    0,
    fn_1_1C0,
    (MGSEQ_FUNC)fn_1_1F4,
    (MGSEQ_FUNC)fn_1_1F8,
    fn_1_254,
    (MGSEQ_FUNC)fn_1_280,
    fn_1_2E0,
    (MGSEQ_FUNC)fn_1_30C,
    (MGSEQ_FUNC)fn_1_310,
    (MGSEQ_FUNC)fn_1_314
}};

s32 lbl_1_data_28 = -1;

extern void fn_1_5CC(void);
extern s32 fn_1_64C(void);
extern s32 fn_1_698(void);

/* Creates the minigame sequence, then runs actor updates once per process wake. */
void fn_1_19C(void)
{
    MgSeqCreate(&lbl_1_data_0.param);
    for (;;) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* Begins a sequence phase by clearing the streamed-music handle and advancing state. */
void fn_1_1C0(s16 mode, s16 frameNo)
{
    lbl_1_data_28 = -1;
    fn_1_5CC();
    MgSeqModeNext();
}

void fn_1_1F4(void)
{
}

/* Starts the Cashapult background stream once the game-message FX event is active. */
void fn_1_1F8(void)
{
    if (lbl_1_data_28 == -1 &&
        (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        lbl_1_data_28 = HuAudSStreamPlay(MSM_STREAM_CAPSULE_WANWAN);
    }
}

/* Advances the sequence when the current actor phase reports completion. */
void fn_1_254(s16 mode, s16 frameNo)
{
    if (fn_1_64C() != 0) MgSeqModeNext();
}

/* Fades out the background stream when the sequence phase reaches its first frame. */
void fn_1_280(void)
{
    if (MgSeqFrameNoGet() == 0 && lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
}

/* Advances the sequence after the result phase reports completion. */
void fn_1_2E0(s16 mode, s16 frameNo)
{
    if (fn_1_698() != 0) MgSeqModeNext();
}

void fn_1_30C(void)
{
}

void fn_1_310(void)
{
}

void fn_1_314(void)
{
}

void fn_1_318(void)
{
}

void fn_1_31C(void)
{
}
