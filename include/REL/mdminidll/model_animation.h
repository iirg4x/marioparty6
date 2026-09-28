#ifndef REL_MDMINIDLL_MODEL_ANIMATION_H
#define REL_MDMINIDLL_MODEL_ANIMATION_H
#include "dolphin/types.h"

/* Consumed layout: 0x38-byte rows, a model, four animation handles, and
 * a halfword accessed at offset 0x30.
 * The remaining target-backed bytes have no recovered field types yet. */
typedef struct MDMinidllModelAnimationRecord {
    s16 model;
    s16 animation[4];
    u8 unknown_0A[38];
    s16 unknown_30;
    u8 unknown_32[6];
} MDMinidllModelAnimationRecord;
extern MDMinidllModelAnimationRecord lbl_1_bss_230[];
#endif
