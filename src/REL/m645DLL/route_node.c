/* Route-node and route-selection accessors for Clean Team. */
#include "REL/m645DLL/route.h"

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

#include "include/dolphin/types.h"

extern M645RouteEntry lbl_1_bss_C[25];

M645RouteEntry *fn_1_2970(s32 index) {
    return &lbl_1_bss_C[index];
}

extern s32 lbl_1_bss_138[5];

s32 fn_1_2984(s32 index) { return lbl_1_bss_138[index]; }
