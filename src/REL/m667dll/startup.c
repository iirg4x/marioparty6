/* Talkie Walkie loader callbacks and object-manager process state. */
#include "game/process.h"
#include <math.h>
#include "REL/m667dll/recovered.h"
#include "game/memory.h"
#include <string.h>
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/object.h"

HUPROCESS *lbl_1_bss_0; /* Object-manager process used by the minigame's scene objects. */

typedef void (*M667VoidFunc)(void);
extern const M667VoidFunc _ctors[];
extern const M667VoidFunc _dtors[];
void fn_1_148(void);

/* The loader runs constructors, then initializes the minigame module. */
int _prolog(void)
{
    const M667VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_148();
    return 0;
}

/* The loader runs registered destructors when the minigame module shuts down. */
void _epilog(void)
{
    const M667VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

#define MSM_STREAM_M667_TALKIE_WALKIE 73

/* Scene setup creates object-manager work records with this helper. */
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *))
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

/* Object setup uses this helper to install each object's update callback. */
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Minigame startup creates the actor manager, selects spherical collision positioning, and
 * starts its sequence process. */
void fn_1_148(void)
{
    lbl_1_bss_0 = MgActorObjectSetup();
    MgActorColCylReset();
    HuPrcChildCreate(fn_1_19C, 100, 12288, 0, lbl_1_bss_0);
}

/* The child creates the sequence process, then advances actor simulation once per frame. */
void fn_1_19C(void)
{
    MgSeqCreate(&lbl_1_data_0);
    while (1) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* The sequence transition callback resets the scene and advances to the next mode. */
void fn_1_1C0(s16 mode, s16 frameNo)
{
    lbl_1_data_28 = -1;
    fn_1_768();
    MgSeqModeNext();
}

void fn_1_1F4(s16 mode, s16 frameNo) {}

/* The sequence starts its minigame stream when the start message reaches FX-play. */
void fn_1_1F8(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 == -1 &&
        (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        lbl_1_data_28 = HuAudSStreamPlay(MSM_STREAM_M667_TALKIE_WALKIE);
    }
}

/* The main sequence advances after the scene determines the outcome and enters result mode 3. */
void fn_1_254(s16 mode, s16 frameNo)
{
    if (fn_1_B90() != 0) MgSeqModeNext();
}

/* The finish sequence fades the minigame stream at frame zero, then waits for scene mode 4. */
void fn_1_280(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0 && lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
    if (fn_1_BDC() != 0) MgSeqModeNext();
}

/* The pre-winner sequence advances after the ending scene reaches mode 5. */
void fn_1_2F0(s16 mode, s16 frameNo)
{
    if (fn_1_C28() != 0) MgSeqModeNext();
}

void fn_1_31C(s16 mode, s16 frameNo) {}
void fn_1_320(s16 mode, s16 frameNo) {}

/* Sequence shutdown attempts response retrieval if microphone input setup was requested,
 * discards the result, and closes microphone input. */
void fn_1_324(s16 mode, s16 frameNo)
{
    fn_1_7A4();
}

MGSEQ_PARAM lbl_1_data_0 = {
    60, 2, fn_1_1C0, fn_1_1F4, fn_1_1F8, fn_1_254, fn_1_280, fn_1_2F0, fn_1_31C, fn_1_320, fn_1_324
};
s32 lbl_1_data_28 = -1;
