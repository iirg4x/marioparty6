#define _MATH_H
#include "REL/mdminidll/camera.h"
#include "game/hu3d.h"

extern HUPROCESS *lbl_1_bss_4;

void fn_1_3240(void)
{
    MDMinidllCameraState *state;
    state = &lbl_1_bss_808;
    Hu3DCameraKill(1);
    if (state->outViewObj) {
        omDelObjEx(lbl_1_bss_4, state->outViewObj);
    }
    state->outViewObj = NULL;
}
