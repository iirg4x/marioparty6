/* Round end check and per-frame score update for the two players. */
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
#include "REL/m661dll/system_state.h"
#include "REL/m661dll/player_state.h"
#include "REL/m661dll/score_display.h"

int fn_1_3064(MGTIMER *);

void fn_1_308C(MGTIMER *);

/* Returns 1 and marks the round finished when the timer runs out, or when the two
 * players' scores total 64 or more (the timer is then removed). */
s16 fn_1_5954(void) {
    s16 finished;

    finished = 0;
    if (fn_1_3064(lbl_1_bss_196C.action) != 0) {
        finished = 1;
    }
    if (lbl_1_bss_1C04[0].score + lbl_1_bss_1C04[1].score >= 0x40) {
        fn_1_308C(lbl_1_bss_196C.action);
        finished = 1;
    }
    if (finished != 0) {
        lbl_1_bss_196C.finished = 1;
    }
    return finished;
}

/* Sets each player's character sound pan from their position at full volume and
 * shows their current score. */
void fn_1_59F0(void) {
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        fn_1_4918(&lbl_1_bss_1C04[playerIndex], 6);
        MgScoreValueSet(lbl_1_bss_0[playerIndex].score, lbl_1_bss_1C04[playerIndex].score);
        playerIndex += 1;
    }
}
