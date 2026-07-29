#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

extern Vec lbl_1_bss_CAC[];
extern const float lbl_1_rodata_1F8;

void fn_1_8F64(s16 modelNo, Vec *pos)
{
    lbl_1_bss_CAC[modelNo].x = pos->x - lbl_1_rodata_1F8;
    lbl_1_bss_CAC[modelNo].y = lbl_1_rodata_1F8 + pos->y;
    lbl_1_bss_CAC[modelNo].z = lbl_1_rodata_1F8 + pos->z;
}
