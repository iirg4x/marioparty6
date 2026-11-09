/* Controller player lookup for the two opposing groups. */
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
char lbl_1_data_578[] = "entries == entry_num";
char lbl_1_data_58D[] = "player_table_1vs1.cpp";

/* Selects the first configured player in each group. */
void fn_1_8110(s32 *playersByGroup) {
    s32 index;
    s32 player;
    s32 assigned;

    index = 0;
    while (index < 2) {
        playersByGroup[index] = -1;
        index += 1;
    }

    assigned = 0;
    player = 0;
    while (player < 4) {
        s32 group;
        group = lbl_80265728[player].grpNo;
        if ((group < 2) && (playersByGroup[group] == -1)) {
            playersByGroup[group] = player;
            assigned += 1;
        }
        player += 1;
    }

    (assigned == 2) ? (void)0 : (void)__msl_assertion_failed(lbl_1_data_578, lbl_1_data_58D, 0x20);
}
