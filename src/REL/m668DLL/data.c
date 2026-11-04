/* Burnstile sequence parameters, resource IDs, formation positions, and shared callback state. */
#include <math.h>
#include "REL/m668DLL/player.h"
#include "REL/m668DLL/data.h"
#include "datadir_enum.h"
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/m668DLL/rotor.h"

M668SequenceDataRecord lbl_1_data_0 = {
    { 0, 0, fn_1_118, fn_1_158, fn_1_15C, fn_1_160, fn_1_164,
      fn_1_168, fn_1_16C, fn_1_170, fn_1_174 },
    { 0, 127, 0, 0, 0, 127, 0, 2,
      0, 127, 0, 4, 0, 127, 0, 3,
      0, 127, 0, 5 },
};

/* Character motion resource IDs loaded by the stage and player callbacks. */
unsigned int lbl_1_data_3C[10] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 2),
    DATANUM(0x930000, 3), DATANUM(0x930000, 4), DATANUM(0x8e0000, 23),
    DATANUM(0x930000, 6), DATANUM(0x930000, 7), DATANUM(0x930000, 20), 0,
};

/* World-space starting positions for the four player formations. */
HuVecF lbl_1_data_64[4] = {
    { -650.0f, 0.0f, 0.0f },
    { -450.0f, 0.0f, 0.0f },
    {  450.0f, 0.0f, 0.0f },
    {  650.0f, 0.0f, 0.0f },
};

/* World-space reference points for team steering and course-edge checks. */
HuVecF lbl_1_data_94[2] = {
    { -550.0f, 0.0f, 0.0f },
    {  550.0f, 0.0f, 0.0f },
};

s32 lbl_1_data_AC = 1;

void *lbl_1_bss_644;
void *lbl_1_bss_640;
HuVecF lbl_1_bss_340[64];
HuVecF lbl_1_bss_40[64];
ANIMDATA *lbl_1_bss_3C;
M668PlayerWork *lbl_1_bss_2C[4];
M668RotorWork *lbl_1_bss_28[1];
s32 lbl_1_bss_24;
M668StageWork *lbl_1_bss_20;
s32 lbl_1_bss_18[2];
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
OMOBJMAN *lbl_1_bss_4;
void *lbl_1_bss_0;
