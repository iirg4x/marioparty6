/* Stopping the module's sound effects and all audio. */
#include "math.h"
#include "REL/m663dll/tutorial.h"

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
#include "REL/m663dll/score.h"

#include "REL/m663dll/input.h"
#include "REL/m663dll/player.h"
#include "REL/m663dll.h"

/* Stops a sound effect whose slot is active and whose status is 2 or 3, and marks the slot
 * inactive. */
void M663SoundStop(s16 soundIndex) {
    if (lbl_1_bss_424.soundActive[soundIndex] == 1 &&
        (HuAudFXStatusGet(lbl_1_bss_424.soundHandles[soundIndex]) == 2 ||
         HuAudFXStatusGet(lbl_1_bss_424.soundHandles[soundIndex]) == 3)) {
        lbl_1_bss_424.soundActive[soundIndex] = -1;
        HuAudFXStop(lbl_1_bss_424.soundHandles[soundIndex]);
    }
}

/* Same as M663SoundStop: stops an active sound effect with status 2 or 3 and marks its slot
 * inactive. */
void M663SoundStopCopy(s16 soundIndex) {
    if (lbl_1_bss_424.soundActive[soundIndex] == 1 &&
        (HuAudFXStatusGet(lbl_1_bss_424.soundHandles[soundIndex]) == 2 ||
         HuAudFXStatusGet(lbl_1_bss_424.soundHandles[soundIndex]) == 3)) {
        lbl_1_bss_424.soundActive[soundIndex] = -1;
        HuAudFXStop(lbl_1_bss_424.soundHandles[soundIndex]);
    }
}

void fn_1_67E4(void) {
    HuAudAllStop();
}

extern s16 lbl_1_bss_15A8;
