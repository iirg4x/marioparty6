#ifndef M652_PLAYER_STATE_H
#define M652_PLAYER_STATE_H

#include "game/mg/actman.h"

typedef struct PlayerAttributeState {
    unsigned char prefix[0x40];
    MGPLAYER *player;
    unsigned char stateFlag : 1;
    unsigned char attributeEnabled : 1;
    unsigned char remainingState : 6;
} PlayerAttributeState;

#endif
