// The player table shared by one-versus-one minigames.
#ifndef REL_FRAMEWORK_PLAYER_TABLE_1VS1_H
#define REL_FRAMEWORK_PLAYER_TABLE_1VS1_H

#include "dolphin/types.h"

// Fills table[0] and table[1] with the first players found in groups zero and one.
void makePlayerTable1vs1(s32 *table);

#endif
