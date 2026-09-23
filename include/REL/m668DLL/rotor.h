#ifndef M668_ROTOR_H
#define M668_ROTOR_H
#include "game/object.h"
typedef struct M668StageWork_s { s32 state; s16 streamHandle; } M668StageWork;
/* Observed 88-byte work allocation: model handles, angle/history, speed and FX. */
typedef struct M668RotorWork_s {
    s16 index;
    s16 model;
    s16 shadowModel;
    f32 angle;
    f32 history[16];
    f32 speed;
    f32 speedScale;
    s32 sound;
} M668RotorWork;
typedef char M668RotorWork_size[(sizeof(M668RotorWork) == 88) ? 1 : -1];
extern M668StageWork *lbl_1_bss_20;
extern void fn_1_218(OMOBJ *, OMOBJ_FUNC);
#endif
