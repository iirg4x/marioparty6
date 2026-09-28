#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"

extern OMOBJ *lbl_1_bss_C;
extern ANIMDATA *lbl_1_bss_764[26];

void fn_1_765C(s16 index)
{
    OMOBJ *obj;
    obj = lbl_1_bss_C;
    Hu3DAnimAnimSet(obj->mtnId[2], lbl_1_bss_764[index * 2 + 3]);
    Hu3DAnimAnimSet(obj->mtnId[3], lbl_1_bss_764[index * 2 + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}
