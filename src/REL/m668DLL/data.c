#define _MATH_H
#include "REL/m668DLL/data.h"
#include "game/mg/actman.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/m668DLL/rotor.h"

extern void fn_1_118(s16, s16);
extern void fn_1_158(s16, s16);
extern void fn_1_15C(s16, s16);
extern void fn_1_160(s16, s16);
extern void fn_1_164(s16, s16);
extern void fn_1_168(s16, s16);
extern void fn_1_16C(s16, s16);
extern void fn_1_170(s16, s16);
extern void fn_1_174(s16, s16);

M668SequenceDataRecord lbl_1_data_0 = {
    { 0, 0, fn_1_118, fn_1_158, fn_1_15C, fn_1_160, fn_1_164,
      fn_1_168, fn_1_16C, fn_1_170, fn_1_174 },
    { 0, 127, 0, 0, 0, 127, 0, 2,
      0, 127, 0, 4, 0, 127, 0, 3,
      0, 127, 0, 5 },
};

unsigned int lbl_1_data_3C[10] = {
    9633792, 9633793, 9633794, 9633795, 9633796,
    9306135, 9633798, 9633799, 9633812, 0,
};

HuVecF lbl_1_data_64[4] = {
    { -650.0f, 0.0f, 0.0f },
    { -450.0f, 0.0f, 0.0f },
    {  450.0f, 0.0f, 0.0f },
    {  650.0f, 0.0f, 0.0f },
};

HuVecF lbl_1_data_94[2] = {
    { -550.0f, 0.0f, 0.0f },
    {  550.0f, 0.0f, 0.0f },
};

s32 lbl_1_data_AC = 1;

/* MWCC allocates BSS objects in reverse declaration order. */
void *lbl_1_bss_644;
void *lbl_1_bss_640;
HuVecF lbl_1_bss_340[64];
HuVecF lbl_1_bss_40[64];
ANIMDATA *lbl_1_bss_3C;
s16 *lbl_1_bss_2C[4];
s16 *lbl_1_bss_28[1];
s32 lbl_1_bss_24;
M668StageWork *lbl_1_bss_20;
s32 lbl_1_bss_18[2];
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
OMOBJMAN *lbl_1_bss_4;
void *lbl_1_bss_0;
