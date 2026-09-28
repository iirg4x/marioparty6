#ifndef REL_MDMINIDLL_LAYER_EFFECT_STATE_H
#define REL_MDMINIDLL_LAYER_EFFECT_STATE_H

#include "dolphin/types.h"

/* Consumed target-backed view only. The purpose, original declaration, and
 * usable record capacity remain unknown; offset 0x18..0x1B is untyped. */
typedef struct MDMinidllLayerEffectStateView {
    /* 0x00 */ f32 field_00;
    /* 0x04 */ f32 field_04;
    /* 0x08 */ f32 field_08;
    /* 0x0C */ f32 field_0C;
    /* 0x10 */ f32 field_10;
    /* 0x14 */ f32 field_14;
    /* 0x18 */ u8 unknown_18[4];
    /* 0x1C */ f32 field_1C;
    /* 0x20 */ f32 field_20;
    /* 0x24 */ f32 field_24;
} MDMinidllLayerEffectStateView;

/* Retail .bss object: 0x8E8..0x937, extent 0x50. This is not an original
 * source array declaration and does not assert a usable record count. */
extern u8 lbl_1_bss_8E8[80];

#endif
