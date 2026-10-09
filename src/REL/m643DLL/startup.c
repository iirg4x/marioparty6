/* Sets up Jump the Gun and advances its minigame sequence. */
#include "dolphin/math.h"
#include "REL/m643DLL/m643.h"
#include "game/mg/seqman.h"
#include "game/memory.h"
#include <string.h>
OMOBJMAN *lbl_1_bss_0;
extern MGSEQ_PARAM lbl_1_data_0;
void fn_1_F4(void);
void fn_1_17A0(OMOBJ *object);
void *fn_1_16C(s32 priority, u32 bytes, OMOBJ_FUNC callback);
/* Creates the actor manager and starts the minigame's actor update process. */
void fn_1_A0(void)
{
    lbl_1_bss_0 = MgActorObjectSetup();
    HuPrcChildCreate(fn_1_F4, 100, 36864, 0, lbl_1_bss_0);
}
/* Creates the phase sequence, then updates actors once per frame until shutdown. */
void fn_1_F4(void)
{
    MgSeqCreate(&lbl_1_data_0);
    for (;;) { MgActorExec(); HuPrcVSleep(); }
}
/* Creates the scene object during sequence init and requests the next phase. */
void fn_1_118(s16 phase, s16 frame)
{
    fn_1_16C(100, 12U, fn_1_17A0);
    MgSeqModeNext();
}
void fn_1_14C(s16 phase, s16 frame) {}
void fn_1_150(s16 phase, s16 frame) {}
void fn_1_154(s16 phase, s16 frame) {}
void fn_1_158(s16 phase, s16 frame) {}
void fn_1_15C(s16 phase, s16 frame) {}
void fn_1_160(s16 phase, s16 frame) {}
void fn_1_164(s16 phase, s16 frame) {}
void fn_1_168(s16 phase, s16 frame) {}

MGSEQ_PARAM lbl_1_data_0 = {
    300, 0, fn_1_118, fn_1_14C, fn_1_150, fn_1_154, fn_1_158, fn_1_15C, fn_1_160, fn_1_164, fn_1_168
};
/* Adds an object to the minigame manager and optionally gives it zeroed work data. */
void *fn_1_16C(s32 priority, u32 bytes, OMOBJ_FUNC callback)
{
    OMOBJ *object;
    object = omAddObjEx(lbl_1_bss_0, priority, 0, 0, 0, callback);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, HU_MEMNUM_OVL);
        memset(object->data, 0, bytes);
    } else object->data = NULL;
    return object->data;
}
/* Replaces an object's per-frame callback during the minigame sequence. */
void fn_1_20C(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}
