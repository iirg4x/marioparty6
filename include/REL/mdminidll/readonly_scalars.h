#ifndef MD_MINI_READONLY_SCALARS_H
#define MD_MINI_READONLY_SCALARS_H

#include "dolphin/types.h"

/* Readonly f32 consumed views from verified source-selected module consumers.
 * Shared here so fresh m2c contexts preserve known scalar ownership instead
 * of losing it in function-local declarations. These allocate no storage;
 * original source names/declarations remain unknown. Implicit integer-cast
 * bias pools are compiler-owned and are deliberately not listed here. */
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_70;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_80;
extern const f32 lbl_1_rodata_90;
extern const f32 lbl_1_rodata_94;
extern const f32 lbl_1_rodata_A0;
extern const f32 lbl_1_rodata_B4;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C8;
extern const f32 lbl_1_rodata_CC;
extern const f32 lbl_1_rodata_D0;
extern const f32 lbl_1_rodata_D4;
extern const f32 lbl_1_rodata_D8;
extern const f32 lbl_1_rodata_DC;
extern const f32 lbl_1_rodata_E0;
extern const f32 lbl_1_rodata_E4;
extern const f32 lbl_1_rodata_E8;
extern const f32 lbl_1_rodata_EC;
extern const f32 lbl_1_rodata_F0;
extern const f32 lbl_1_rodata_F4;
extern const f32 lbl_1_rodata_F8;
extern const f32 lbl_1_rodata_FC;
extern const f32 lbl_1_rodata_100;
extern const f32 lbl_1_rodata_128;
extern const f32 lbl_1_rodata_130;
extern const f32 lbl_1_rodata_138;
extern const f32 lbl_1_rodata_13C;
extern const f32 lbl_1_rodata_140;
extern const f32 lbl_1_rodata_144;
extern const f32 lbl_1_rodata_148;
extern const f32 lbl_1_rodata_14C;
extern const f32 lbl_1_rodata_168;
extern const f32 lbl_1_rodata_178;
extern const f32 lbl_1_rodata_17C;
extern const f32 lbl_1_rodata_180;
extern const f32 lbl_1_rodata_184;
extern const f32 lbl_1_rodata_188;
extern const f32 lbl_1_rodata_18C;
extern const f32 lbl_1_rodata_190;
/* 0.85f, consumed by the verified player/model reset at 0x17894. */
extern const f32 lbl_1_rodata_1A0;
extern const f32 lbl_1_rodata_1BC;
extern const f32 lbl_1_rodata_1C0;
extern const f32 lbl_1_rodata_1C4;
extern const f32 lbl_1_rodata_248;
extern const f32 lbl_1_rodata_268;
extern const f32 lbl_1_rodata_26C;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_298;
extern const f32 lbl_1_rodata_2A8;
extern const f32 lbl_1_rodata_2BC;
extern const f32 lbl_1_rodata_2C0;
extern const f32 lbl_1_rodata_2C4;
extern const f32 lbl_1_rodata_2C8;
extern const f32 lbl_1_rodata_360;

#endif
