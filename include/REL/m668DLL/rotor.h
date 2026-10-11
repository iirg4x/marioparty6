/* Shared stage and rotating-bar work layouts for Burnstile callbacks. */
#ifndef M668_ROTOR_H
#define M668_ROTOR_H
#include "game/object.h"
/* Stage callback state and optional sequence music stream. */
typedef struct M668StageWork_s {
    s32 state; /* Nonzero makes callbacks that check this state skip updates. */
    s16 streamHandle; /* Audio stream handle, or -1 before playback starts. */
} M668StageWork;
/* Rotor model, angular motion history, speed, and looping sound state. */
typedef struct M668RotorWork_s {
    s16 index; /* Rotor index used to stagger its initial angle. */
    s16 model; /* Main rotor model handle. */
    s16 shadowModel; /* Linked model used for the rotor shadow. */
    f32 angle; /* Current rotor angle in radians. */
    f32 history[16]; /* Previous 16 rotor angles in radians. */
    f32 speed; /* Signed angular speed in degrees per frame. */
    f32 speedScale; /* Runtime multiplier applied to angular speed. */
    s32 sound; /* Looping rotor sound handle. */
} M668RotorWork;
typedef char M668RotorWork_size[(sizeof(M668RotorWork) == 88) ? 1 : -1];
extern M668StageWork *lbl_1_bss_20;
extern void fn_1_218(OMOBJ *, OMOBJ_FUNC);
#endif
