// Writes the one-versus-three player ordering into the supplied table.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

extern "C" {
#include "game/gamework.h"
}

#include "REL/m623DLL/player_table_1vs3.h"

// Each group must have its expected number of players.
void fn_1_E5DC(int *table)
{
    int *end;
    int player;
    int groupPlayer;

    end = table + 1;
    for (player = 0; player < 4; player++) {
        if (lbl_80265728[player].grpNo == 0) {
/* Retail fixes the next assert's line; the earlier lines are unknown: 1 line is left blank. */

            assert(table < end);
            *table++ = player;
        }
    }
    assert(table == end);

    end = table + 3;
    for (groupPlayer = 0; groupPlayer < 4; groupPlayer++) {
        if (lbl_80265728[groupPlayer].grpNo != 0) {
/* Retail fixes the next assert's line; the earlier lines are unknown: 0 lines are left blank. */
            assert(table < end);
            *table++ = groupPlayer;
        }
    }
    assert(table == end);
}
