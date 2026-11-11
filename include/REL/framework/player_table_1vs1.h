// The player table of a one-versus-one minigame (player_table_1vs1.cpp, shared by the minigames that have one).
#ifndef REL_FRAMEWORK_PLAYER_TABLE_1VS1_H
#define REL_FRAMEWORK_PLAYER_TABLE_1VS1_H

#include "dolphin/types.h"

// Fills table[0] and table[1] with the first player of group 0 and of group 1. The name is descriptive, not
// recovered.
void makePlayerTable1vs1(s32 *table);

#endif
