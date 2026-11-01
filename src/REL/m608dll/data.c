/* Course model, motion and per-player animation resource names and IDs. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

/* Camera motion resources loaded in setup, in camera-index order. */
s32 lbl_1_data_4F0[10] = {
    DATANUM(0x430000, 7), DATANUM(0x430000, 8), DATANUM(0x430000, 9), DATANUM(0x430000, 10),
    DATANUM(0x430000, 11), DATANUM(0x430000, 12), DATANUM(0x430000, 13), DATANUM(0x430000, 14),
    DATANUM(0x430000, 15), DATANUM(0x430000, 16),
};

/* Model resources for the three rider model variants, ordered by player slot. */
s32 lbl_1_data_518[4] = { DATANUM(0x430000, 46), DATANUM(0x430000, 47), DATANUM(0x430000, 48),
                          DATANUM(0x430000, 49) };

s32 lbl_1_data_528[4] = { DATANUM(0x430000, 50), DATANUM(0x430000, 51), DATANUM(0x430000, 52),
                          DATANUM(0x430000, 53) };

s32 lbl_1_data_538[4] = { DATANUM(0x430000, 54), DATANUM(0x430000, 55), DATANUM(0x430000, 56),
                          DATANUM(0x430000, 57) };

/* Character animation resources attached to each rider model. */
unsigned int lbl_1_data_548[9] = {
    DATANUM(0x8e0000, 137),  DATANUM(0x8e0000, 138),   DATANUM(0x8e0000, 139),
    DATANUM(0x8e0000, 140),  DATANUM(0x8e0000, 141),   DATANUM(0x930000, 0),
    DATANUM(0x930000, 6), DATANUM(0x930000, 40), 0
};

/* CPU roll timing: base interval divisor and random spread for each difficulty. */
u32 lbl_1_data_56C[8] = { 4, 8, 16, 6, 22, 8, 34, 6 };

char lbl_1_data_58C[17] = "608kurukuru-p1st";

char lbl_1_data_59D[17] = "608kurukuru-p2st";

char lbl_1_data_5AE[17] = "608kurukuru-p3st";

char lbl_1_data_5BF[17] = "608kurukuru-p4st";

/* Per-player board and display hook names for the four rider models. */
char *lbl_1_data_5D0[4] = { lbl_1_data_58C, lbl_1_data_59D, lbl_1_data_5AE, lbl_1_data_5BF };

char lbl_1_data_5E0[17] = "608kurukuru-p1ls";

char lbl_1_data_5F1[17] = "608kurukuru-p2ls";

char lbl_1_data_602[17] = "608kurukuru-p3ls";

char lbl_1_data_613[17] = "608kurukuru-p4ls";

char *lbl_1_data_624[4] = { lbl_1_data_5E0, lbl_1_data_5F1, lbl_1_data_602, lbl_1_data_613 };

char lbl_1_data_634[6] = "p1lsB";

char lbl_1_data_63A[6] = "p2lsB";

char lbl_1_data_640[6] = "p3lsB";

char lbl_1_data_646[6] = "p4lsB";

char *lbl_1_data_64C[4] = { lbl_1_data_634, lbl_1_data_63A, lbl_1_data_640, lbl_1_data_646 };

char lbl_1_data_65C[10] = "p1lsboard";

char lbl_1_data_666[10] = "p2lsboard";

char lbl_1_data_670[10] = "p3lsboard";

char lbl_1_data_67A[10] = "p4lsboard";

char *lbl_1_data_684[4] = { lbl_1_data_65C, lbl_1_data_666, lbl_1_data_670, lbl_1_data_67A };

/* Model and motion resources used by the 27 moving course pieces. */
s32 lbl_1_data_694[27] = {
    DATANUM(0x430000, 17), DATANUM(0x430000, 18), DATANUM(0x430000, 19), DATANUM(0x430000, 20),
    DATANUM(0x430000, 21), DATANUM(0x430000, 22), DATANUM(0x430000, 23), DATANUM(0x430000, 24),
    DATANUM(0x430000, 25), DATANUM(0x430000, 26), DATANUM(0x430000, 27), DATANUM(0x430000, 28),
    DATANUM(0x430000, 29), DATANUM(0x430000, 30), DATANUM(0x430000, 31), DATANUM(0x430000, 32),
    DATANUM(0x430000, 33), DATANUM(0x430000, 34), DATANUM(0x430000, 35), DATANUM(0x430000, 36),
    DATANUM(0x430000, 37), DATANUM(0x430000, 38), DATANUM(0x430000, 39), DATANUM(0x430000, 40),
    DATANUM(0x430000, 41), DATANUM(0x430000, 42), DATANUM(0x430000, 43),
};

M608SceneConsumedContext lbl_1_bss_3E80;
