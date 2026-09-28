#ifndef REL_MDMINIDLL_PLAYER_CONFIG_H
#define REL_MDMINIDLL_PLAYER_CONFIG_H
#include "dolphin/types.h"

/* Reconstructed from four 14-byte records and their GW_PLAYER consumers. */
typedef struct MDMinidllPlayerConfig {
    s16 unknown_00;
    s16 unknown_02;
    s16 comF;
    s16 comDif;
    s16 charNo;
    s16 padNo;
    /* Observed record extent; original fields/purpose are unknown. */
    u8 unknown_0C[2];
} MDMinidllPlayerConfig;
extern MDMinidllPlayerConfig lbl_1_bss_7CC[4];
/* Retail consumes signed halfwords at +0 and +2. The symbol's declared extent
 * is two bytes; +2 is an observed adjacent unknown interval. This incomplete
 * view makes no capacity or original-type claim. */
extern s16 lbl_1_bss_804[];
#endif
