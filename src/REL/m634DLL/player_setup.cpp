#include "REL/m634DLL/game_manager.h"
#include "REL/m634DLL/act_player.h"
extern "C" {
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

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
}
// Returns the player position within its configured group.
s32 fn_1_C68C(s32 playerNo)
{
    s32 groupPosition;
    s32 candidate;
    groupPosition = 0;
    if (lbl_80265728[playerNo].grpNo == 0) {
        groupPosition = 0;
    } else {
        candidate = 0;
        while (candidate <= playerNo) {
            if (lbl_80265728[candidate].grpNo != 0) {
                groupPosition += 1;
            }
            candidate += 1;
        }
    }
    return groupPosition;
}

// Creates the player objects for their group positions.
void fn_1_C708(GameManager *)
{
    s32 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        s32 groupPosition;
        s32 mode;

        groupPosition = fn_1_C68C(playerNo);

        mode = groupPosition == 0 ? 0x30 : 0x40;
        new ActPlayer(playerNo, groupPosition, mode);
    }
}
