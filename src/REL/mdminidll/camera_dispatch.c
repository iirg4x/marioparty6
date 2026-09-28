#define _MATH_H
#include "REL/mdminidll/camera.h"

void fn_1_2F88(OMOBJ *obj, MDMinidllCameraState *state)
{
    if (state->callback) {
        state->callback(obj, state);
    }
}

void fn_1_2FD4(OMOBJ *obj)
{
    MDMinidllCameraState *state;

    state = &lbl_1_bss_808;
    if (state->callback) {
        state->callback(obj, state);
    }
    Center.x = state->center.x;
    Center.y = state->center.y;
    Center.z = state->center.z;
    CRot.x = state->rotation.x;
    CRot.y = state->rotation.y;
    CRot.z = state->rotation.z;
    CZoom = state->zoom;
    omOutView(obj);
}
