#define _MATH_H
#include <string.h>
#include "REL/mdminidll/camera.h"

extern const f32 lbl_1_rodata_74;
void fn_1_175F8(OMOBJ *obj, MDMinidllCameraState *state);

void fn_1_17820(OMOBJ *obj, MDMinidllCameraState *state)
{
    state->transitionFrame = lbl_1_rodata_74;
    memcpy(&state->center, &state->centerTarget, 12U);
    memcpy(&state->rotation, &state->rotationTarget, 12U);
    state->zoom = state->zoomTarget;
    lbl_1_bss_808.callback = fn_1_175F8;
}
