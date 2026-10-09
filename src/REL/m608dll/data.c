/* Course model, motion and per-player animation resource names and IDs. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

/* Camera motion resources loaded in setup, in camera-index order. */
s32 lbl_1_data_4F0[10] = {
    DATANUM(DATA_m608, 7), DATANUM(DATA_m608, 8), DATANUM(DATA_m608, 9), DATANUM(DATA_m608, 10),
    DATANUM(DATA_m608, 11), DATANUM(DATA_m608, 12), DATANUM(DATA_m608, 13), DATANUM(DATA_m608, 14),
    DATANUM(DATA_m608, 15), DATANUM(DATA_m608, 16),
};

/* Model resources for the three rider model variants, ordered by player slot. */
s32 lbl_1_data_518[4] = { DATANUM(DATA_m608, 46), DATANUM(DATA_m608, 47), DATANUM(DATA_m608, 48),
                          DATANUM(DATA_m608, 49) };

s32 lbl_1_data_528[4] = { DATANUM(DATA_m608, 50), DATANUM(DATA_m608, 51), DATANUM(DATA_m608, 52),
                          DATANUM(DATA_m608, 53) };

s32 lbl_1_data_538[4] = { DATANUM(DATA_m608, 54), DATANUM(DATA_m608, 55), DATANUM(DATA_m608, 56),
                          DATANUM(DATA_m608, 57) };

/* Character animation resources attached to each rider model. */
unsigned int lbl_1_data_548[9] = {
    DATANUM(DATA_mario, 137),  DATANUM(DATA_mario, 138),   DATANUM(DATA_mario, 139),
    DATANUM(DATA_mario, 140),  DATANUM(DATA_mario, 141),   DATANUM(DATA_mariomot, 0),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40), 0
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
    DATANUM(DATA_m608, 17), DATANUM(DATA_m608, 18), DATANUM(DATA_m608, 19), DATANUM(DATA_m608, 20),
    DATANUM(DATA_m608, 21), DATANUM(DATA_m608, 22), DATANUM(DATA_m608, 23), DATANUM(DATA_m608, 24),
    DATANUM(DATA_m608, 25), DATANUM(DATA_m608, 26), DATANUM(DATA_m608, 27), DATANUM(DATA_m608, 28),
    DATANUM(DATA_m608, 29), DATANUM(DATA_m608, 30), DATANUM(DATA_m608, 31), DATANUM(DATA_m608, 32),
    DATANUM(DATA_m608, 33), DATANUM(DATA_m608, 34), DATANUM(DATA_m608, 35), DATANUM(DATA_m608, 36),
    DATANUM(DATA_m608, 37), DATANUM(DATA_m608, 38), DATANUM(DATA_m608, 39), DATANUM(DATA_m608, 40),
    DATANUM(DATA_m608, 41), DATANUM(DATA_m608, 42), DATANUM(DATA_m608, 43),
};

M608SceneConsumedContext lbl_1_bss_3E80;
