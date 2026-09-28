#define _MATH_H
#include "REL/mdminidll/camera.h"

extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_128;
extern const f32 lbl_1_rodata_268;
extern const f32 lbl_1_rodata_26C;
extern const f32 lbl_1_rodata_6C;

void fn_1_175F8(OMOBJ *obj, MDMinidllCameraState *state)
{
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;
    f32 var_f25;
    f32 var_f24;
    f32 var_f23;

    var_f29 = state->transitionFrame;
    var_f30 = state->centerTarget.y;
    if (var_f29 <= lbl_1_rodata_74) {
        var_f31 = var_f30;
    } else if (var_f29 >= lbl_1_rodata_128) {
        var_f31 = lbl_1_rodata_268;
    } else {
        var_f31 = var_f30 + ((var_f29 / lbl_1_rodata_128) * (lbl_1_rodata_268 - var_f30));
    }
    state->center.y = var_f31;

    var_f26 = state->transitionFrame;
    var_f27 = state->centerTarget.z;
    if (var_f26 <= lbl_1_rodata_74) {
        var_f28 = var_f27;
    } else if (var_f26 >= lbl_1_rodata_128) {
        var_f28 = lbl_1_rodata_26C;
    } else {
        var_f28 = var_f27 + ((var_f26 / lbl_1_rodata_128) * (lbl_1_rodata_26C - var_f27));
    }
    state->center.z = var_f28;

    var_f23 = state->transitionFrame;
    var_f24 = state->rotationTarget.x;
    if (var_f23 <= lbl_1_rodata_74) {
        var_f25 = var_f24;
    } else if (var_f23 >= lbl_1_rodata_128) {
        var_f25 = lbl_1_rodata_74;
    } else {
        var_f25 = var_f24 + ((var_f23 / lbl_1_rodata_128) * (lbl_1_rodata_74 - var_f24));
    }
    state->rotation.x = var_f25;
    state->transitionFrame += lbl_1_rodata_6C;
}
