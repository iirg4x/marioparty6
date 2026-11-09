/* Player group table for the one-versus-one mode. */
extern "C" {
#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

void __msl_assertion_failed(const char *, const char *, s32);
}


/* Finds the first player entries to each of the two groups. */
void fn_1_9B98(s32 *playersByGroup) {
    const s32 entry_num = 2;
    s32 index;
    s32 player;
    s32 entries;

    index = 0;
    while (index < entry_num) {
        playersByGroup[index] = -1;
        index += 1;
    }

    entries = 0;
    player = 0;
    while (player < 4) {
        s32 group;
        group = lbl_80265728[player].grpNo;
        if ((group < entry_num) && (playersByGroup[group] == -1)) {
            playersByGroup[group] = player;
            entries += 1;
        }
        player += 1;
    }

    (entries == entry_num) ? (void)0 : (void)__msl_assertion_failed("entries == entry_num", "player_table_1vs1.cpp", 0x20);
}
