#ifndef M661DLL_PLAYER_ACTORS_H
#define M661DLL_PLAYER_ACTORS_H

#include "dolphin/types.h"
#include "game/mg/actman.h"

typedef struct PlayerActors {
    s16 modelId;
    MGPLAYER *players[2];
} PlayerActors;

#ifdef __cplusplus
extern "C" {
#endif
extern PlayerActors lbl_1_bss_1960;
#ifdef __cplusplus
}
#endif

#endif
