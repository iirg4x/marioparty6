/* Fixed vectors, message IDs, and bitmap names used by Mic Mode. */
#ifndef REL_MDMICDLL_RODATA_STATE_H
#define REL_MDMICDLL_RODATA_STATE_H
#include "humath.h"
typedef struct { u32 ids[5]; } MDMIC_MESSAGE_IDS; /* Five setup-screen message IDs. */
typedef struct { char *name[4]; } MDMIC_BITMAP_NAMES; /* Four bitmap resource names. */
extern const HuVecF lbl_1_rodata_FC;
extern const f32 lbl_1_rodata_124;
extern const MDMIC_MESSAGE_IDS lbl_1_rodata_19C;
extern const MDMIC_BITMAP_NAMES lbl_1_rodata_2E8;
#endif
