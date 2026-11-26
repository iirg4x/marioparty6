#ifndef REL_M663DLL_MAIN_H
#define REL_M663DLL_MAIN_H

#include "game/process.h"
#include "game/mg/timer.h"
#include "game/hu3d.h"

void fn_1_754(void);
void fn_1_2A00(void);
void fn_1_3588(void);
void fn_1_3A68(MGTIMER *timer);
s32 fn_1_3A94(MGTIMER *timer);
f32 fn_1_46A8(f32 value);
void fn_1_56E8(HU3D_MODEL *model, Mtx *viewMatrix);

extern s16 lbl_1_bss_15A8;
extern HUPROCESS *lbl_1_bss_15A0[2];

#endif
