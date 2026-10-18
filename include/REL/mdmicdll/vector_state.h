/* Model transform vectors shared by Mic Mode scenes. */
#ifndef REL_MDMICDLL_VECTOR_STATE_H
#define REL_MDMICDLL_VECTOR_STATE_H

#include "humath.h"

extern HuVecF lbl_1_bss_E4[4]; /* Left player badges: x is target scale, y is current scale. */
extern HuVecF lbl_1_bss_B4[4]; /* Right player badges: x is target scale, y is current scale. */
extern HuVecF lbl_1_bss_9C[2]; /* Paired-player badges: x is target scale, y is current scale. */
extern f32 lbl_1_bss_90[3]; /* Shared indicator: target scale, current scale, unused component. */

#endif
