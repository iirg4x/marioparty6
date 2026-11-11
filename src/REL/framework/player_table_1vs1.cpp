// The player table of a one-versus-one minigame: the first player found in each of the two groups, or -1.
// The file name and the assert text are retail's (player_table_1vs1.cpp, line 32); the function name is
// descriptive, not recovered, and the original text of the lines before the assert is unknown. NDEBUG is
// undefined because the build defines it for every object while retail kept this assert.
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
