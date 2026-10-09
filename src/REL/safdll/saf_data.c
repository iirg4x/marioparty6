/* Motion-check strings, surface distortion matrix, and shared scene state. */
#include "dolphin/math.h"
#include "REL/safdll/saf.h"

#include "dolphin/types.h"

/* Not read by this module's code; MP4's safDll has the same variable. */
s32 lbl_1_data_0 = 100;
char lbl_1_data_4[] = "******* Motion Check ObjectSetup *********\n";
char lbl_1_data_30[12] = ">>>>>>%f\n";
/* Scales the surface bump-map offsets along texture S and T. */
f32 lbl_1_data_3C[2][3] = {{0.05f, 0.0f, 0.0f}, {0.0f, 0.05f, 0.0f}};
char lbl_1_data_54[] = "m625_field-grid7";

#include "dolphin/math.h"
#include "dolphin/types.h"
#include "game/object.h"
#include "game/process.h"
#include "game/hsfex.h"

OMOBJ *lbl_1_bss_4C0;
OMOBJ *lbl_1_bss_4BC;
u32 lbl_1_bss_49C[8];
HUPROCESS *lbl_1_bss_1B8[185];
s16 lbl_1_bss_1B4;
void *lbl_1_bss_1B0;
void *lbl_1_bss_1AC;
void *lbl_1_bss_1A4[2];
void *lbl_1_bss_1A0;
void *lbl_1_bss_19C;
void *lbl_1_bss_198;
SafState lbl_1_bss_4;
f32 lbl_1_bss_0;
