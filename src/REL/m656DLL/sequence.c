/* Registers Mass Meteor's sequence callbacks and owns its object manager. */
#include "REL/m656/m656.h"

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_118, fn_1_14C, fn_1_150, fn_1_154, fn_1_158, fn_1_15C, fn_1_160, fn_1_164, fn_1_168,
};

OMOBJMAN *lbl_1_bss_0;

/* Called by _prolog to create the object manager and start the sequence process. */
void fn_1_A0(void)
{
    lbl_1_bss_0 = MgActorObjectSetup();
    HuPrcChildCreate(fn_1_F4, 100U, 36864U, 0, lbl_1_bss_0);
}

/* The child process created by fn_1_A0 advances the object manager once per frame. */
void fn_1_F4(void)
{
    MgSeqCreate(&lbl_1_data_0);
    while (1) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* Called by MgSeqCreate at the intro transition to start gameplay and advance the mode. */
void fn_1_118(s16 sequenceMode, s16 frameNumber)
{
    fn_1_16C(100, 12U, fn_1_5050);
    MgSeqModeNext();
}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_14C(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_150(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_154(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_158(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_15C(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_160(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_164(s16 sequenceMode, s16 frameNumber)
{

}

/* Unused callback slot registered with MgSeqCreate; it runs no sequence action. */
void fn_1_168(s16 sequenceMode, s16 frameNumber)
{

}

/* Called by Mass Meteor setup callbacks to register an object and allocate zeroed work data. */
void *fn_1_16C(s32 priority, u32 workSize, void (*objectCallback)(OMOBJ *))
{
    OMOBJ *object;

    object = omAddObjEx(lbl_1_bss_0, (s16) priority, 0U, 0U, 0, objectCallback);
    if (workSize != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, (s32) workSize, HU_MEMNUM_OVL);
        memset(object->data, 0, workSize);
    } else {
        object->data = NULL;
    }
    return object->data;
}

/* Called by stage callbacks to change the function run for an object's next frame. */
void fn_1_20C(OMOBJ *object, void (*objectCallback)(OMOBJ *))
{
    object->objFunc = objectCallback;
}

/* Chooses a random stage value between the supplied lower and upper bounds. */
float fn_1_214(f32 minimum, f32 maximum)
{
    return minimum + ((maximum - minimum) * frandf());
}
