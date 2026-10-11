/* Burnstile REL startup, sequence callbacks, and object work setup. */
#include "dolphin/math.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "REL/m668DLL/data.h"

enum { M668_ACTOR_STACK_SIZE = 36864 };

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void ObjectSetup(void);

/* Runs module constructors and starts the minigame actor manager when the REL loads. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* Runs module destructors when the REL is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

extern OMOBJMAN *lbl_1_bss_4;
void fn_1_F4(void);

/* Creates the minigame actor manager and its continuously scheduled update process. */
void ObjectSetup(void)
{
    lbl_1_bss_4 = MgActorObjectSetup();
    HuPrcChildCreate(fn_1_F4, 100, M668_ACTOR_STACK_SIZE, 0, lbl_1_bss_4);
}

/* Starts Burnstile's sequence, then updates actors once per scheduler pass. */
void fn_1_F4(void)
{
    MgSeqCreate(&lbl_1_data_0.param);
    for (;;) {
        MgActorExec();
        HuPrcVSleep();
    }
}

#include "game/object.h"
#include "game/mg/seqman.h"
extern void *lbl_1_bss_0;
void fn_1_1A48(OMOBJ *obj);
void *fn_1_178(s32 priority, u32 dataSize, OMOBJ_FUNC callback);
u32 MgSeqModeNext(void);
/* Sequence initialization callback creates the stage object and requests the next phase. */
void fn_1_118(s16 mode, s16 frameNo)
{
    lbl_1_bss_0 = fn_1_178(100, 8, fn_1_1A48);
    MgSeqModeNext();
}

#include "dolphin/types.h"
void fn_1_158(s16 mode, s16 frameNo) {}
void fn_1_15C(s16 mode, s16 frameNo) {}
void fn_1_160(s16 mode, s16 frameNo) {}
void fn_1_164(s16 mode, s16 frameNo) {}
void fn_1_168(s16 mode, s16 frameNo) {}
void fn_1_16C(s16 mode, s16 frameNo) {}
void fn_1_170(s16 mode, s16 frameNo) {}
void fn_1_174(s16 mode, s16 frameNo) {}

#include "game/object.h"
#include "game/memory.h"
#include "string.h"

/* Creates an object and zeroed callback work storage; null results are unchecked. */
void *fn_1_178(s32 priority, u32 dataSize, OMOBJ_FUNC callback)
{
    OMOBJ *obj;
    obj = omAddObjEx(lbl_1_bss_4, (s16)priority, 0, 0, 0, callback);
    if (dataSize != 0) {
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, dataSize, HU_MEMNUM_OVL);
        memset(obj->data, 0, dataSize);
    } else {
        obj->data = NULL;
    }
    return obj->data;
}

void fn_1_218(OMOBJ *obj, OMOBJ_FUNC callback)
{
    obj->objFunc = callback;
}
