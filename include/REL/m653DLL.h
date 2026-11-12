#ifndef M653DLL_H
#define M653DLL_H

#include "dolphin/types.h"

struct Car;

extern "C" {
// The player number of each of the two cars, from the 1-vs-1 player table.
s32 fn_1_696C(s32 index);

// The two players' cars and the round's state flags: the fight is on, the round is over.
extern Car *lbl_1_bss_5C[2];
extern u8 lbl_1_bss_64;
extern u8 lbl_1_bss_65;
}

#endif
