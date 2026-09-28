#define _MATH_H
#include "game/object.h"
#include "game/audio.h"

extern OMOBJMAN *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_30;
/* Existing readonly target scalars; original declarations/names unknown. */
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_B4;

void fn_1_4BE0(OMOBJ *obj);

void fn_1_4CE4(void)
{
    Hu3DMotionShiftSet(*lbl_1_bss_10->mdlId, lbl_1_bss_10->mtnId[4],
                       lbl_1_rodata_74, lbl_1_rodata_B4, 0U);
    lbl_1_bss_30 = omAddObjEx(lbl_1_bss_4, 4096, 16, 16, -1, fn_1_4BE0);
    lbl_1_bss_30->work[0] = 0;
    HuAudFXPlay(1184);
}
