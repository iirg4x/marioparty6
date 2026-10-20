/* Model and player handles used by the stage's two active player slots. */
#ifndef REL_M660_PLAYER_SLOTS_H
#define REL_M660_PLAYER_SLOTS_H
#include "dolphin/types.h"

typedef struct M660PlayerSlots {
    s16 modelId;
    struct MgPlayer_s *players[2];
} M660PlayerSlots;

extern M660PlayerSlots lbl_1_bss_1E8;
#endif
