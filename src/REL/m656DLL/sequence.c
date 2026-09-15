#include "REL/m656/m656.h"

MGSEQ_PARAM lbl_1_data_0 = {
    0,
    0,
    fn_1_118,
    fn_1_14C,
    fn_1_150,
    fn_1_154,
    fn_1_158,
    fn_1_15C,
    fn_1_160,
    fn_1_164,
    fn_1_168,
};

OMOBJMAN *lbl_1_bss_0;

void fn_1_A0(void)
{
    lbl_1_bss_0 = MgActorObjectSetup();
    HuPrcChildCreate(fn_1_F4, 100U, 36864U, 0, lbl_1_bss_0);
}

void fn_1_F4(void)
{
    MgSeqCreate(&lbl_1_data_0);
loop_1:
    MgActorExec();
    HuPrcVSleep();
    goto loop_1;
}

void fn_1_118(s16 mode, s16 frameNo)
{
    fn_1_16C(100, 12U, fn_1_5050);
    MgSeqModeNext();
}

void fn_1_14C(s16 mode, s16 frameNo)
{

}

void fn_1_150(s16 mode, s16 frameNo)
{

}

void fn_1_154(s16 mode, s16 frameNo)
{

}

void fn_1_158(s16 mode, s16 frameNo)
{

}

void fn_1_15C(s16 mode, s16 frameNo)
{

}

void fn_1_160(s16 mode, s16 frameNo)
{

}

void fn_1_164(s16 mode, s16 frameNo)
{

}

void fn_1_168(s16 mode, s16 frameNo)
{

}

void *fn_1_16C(s32 priority, u32 size, void (*hook)(OMOBJ *))
{
    OMOBJ *temp_r31;

    temp_r31 = omAddObjEx(lbl_1_bss_0, (s16) priority, 0U, 0U, 0, hook);
    if (size != 0) {
        temp_r31->data = HuMemDirectMallocNum(HEAP_HEAP, (s32) size, 268435456U);
        memset(temp_r31->data, 0, size);
    } else {
        temp_r31->data = NULL;
    }
    return temp_r31->data;
}

void fn_1_20C(OMOBJ *obj, void (*hook)(OMOBJ *))
{
    obj->objFunc = hook;
}

float fn_1_214(f32 start, f32 end)
{
    return start + ((end - start) * frandf());
}
