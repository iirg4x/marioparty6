#ifndef _GAME_FRAND_H
#define _GAME_FRAND_H

#include "dolphin.h"

u32 frandom(u32 seed);
u32 frand(void);
f32 frandf(void);
/* Signed by DOL authority: every frandmod result the DOL converts to float in
 * TUs without this header goes through the signed xoris 0x8000 sequence
 * (e.g. CharModelLandDustCreate 0x8005F968), and tu_declarations.json records
 * the same s32 contract for mdpartydll/stage.c. TUs that include this header
 * and need the DOL's unsigned float conversion cast the result back to u32 at
 * the use site (see actman.c/colman.c). */
s32 frandmod(s32 modulus);

#endif