#ifndef M668_PLAYER_H
#define M668_PLAYER_H
#include "REL/m668DLL/rotor.h"
#include "game/mg/actman.h"

/* 84-byte callback work allocation. Field names describe observed consumers. */
typedef struct M668PlayerWork_s {
    s16 playerNo, partnerNo, charNo, teamNo, formationNo;
    MGPLAYER *player;
    s16 state;
    HuVecF position[3];
    s16 playerSelector;
    s32 transitionFlag;
    s32 timer60;
    HuVecF origin;
    s32 sign;
} M668PlayerWork;
#endif
