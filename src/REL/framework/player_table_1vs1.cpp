// Builds the player table for a one-versus-one minigame.
// Each table entry starts at -1 and receives the first player found in that group.
// The two entries represent groups zero and one.
// The build defines NDEBUG for every object; the original kept this assert.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

#include "REL/framework/player_table_1vs1.h"

#include "math.h"
extern "C" {
#include "game/gamework.h"
}
// A one-versus-one minigame needs a player in both groups.
void makePlayerTable1vs1(s32 *table)
{
    const s32 entry_num = 2;
    s32 index;
    s32 player;
    s32 entries;
    for (index = 0; index < entry_num; index++) {
        table[index] = -1;
    }
    entries = 0;
    for (player = 0; player < 4; player++) {
        s32 group = lbl_80265728[player].grpNo;
        if (group < entry_num && table[group] == -1) {
            table[group] = player;
            entries++;
        }
    }
    assert(entries == entry_num);
}
