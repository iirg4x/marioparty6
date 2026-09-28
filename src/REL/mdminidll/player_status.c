#include "REL/mdminidll/player_config.h"

s32 fn_1_198A0(void)
{
    s16 player;
    s16 count;
    player = 0;
    count = 0;
    while (player < 4) {
        if (lbl_1_bss_7CC[player].unknown_00 == 1) {
            count += 1;
        }
        player += 1;
    }
    if (lbl_1_bss_804[0] <= count) {
        return 0;
    }
    if (lbl_1_bss_7CC[0].unknown_00 == 0) {
        return 1;
    }
    return -1;
}
