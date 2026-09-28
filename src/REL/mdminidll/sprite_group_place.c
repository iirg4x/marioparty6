#define _MATH_H
#include "game/sprite.h"

extern HUSPR_GROUPID lbl_1_bss_752[9];
/* Existing target readonly storage; original declaration/name unknown. */
extern const f32 lbl_1_rodata_18C;
extern const f32 lbl_1_rodata_128;
extern const f32 lbl_1_rodata_74;

void fn_1_73B4(void)
{
    HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], lbl_1_rodata_74);
}
