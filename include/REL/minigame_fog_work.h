#ifndef REL_MINIGAME_FOG_WORK_H
#define REL_MINIGAME_FOG_WORK_H

#include "REL/minigame_fog.h"
#include "game/object.h"

typedef struct MinigameFogWork {
    MinigameFogParams *parameters; /* Per-layer fog settings. */
    OMOBJ_FUNC callback; /* Per-frame fog transition callback. */
    f32 factor; /* Current blend factor between fog states. */
} MinigameFogWork;

#endif
