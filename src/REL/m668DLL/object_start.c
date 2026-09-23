#define _MATH_H
#include "game/object.h"
#include "game/mg/seqman.h"
extern void *lbl_1_bss_0;
void fn_1_1A48(OMOBJ *obj);
void *fn_1_178(s32 priority, u32 dataSize, OMOBJ_FUNC callback);
u32 MgSeqModeNext(void);
void fn_1_118(s16 mode, s16 frameNo)
{
    lbl_1_bss_0 = fn_1_178(100, 8, fn_1_1A48);
    MgSeqModeNext();
}
