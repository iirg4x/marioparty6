/* Shared actor and callback state for each Burnstile player. */
#ifndef M668_PLAYER_H
#define M668_PLAYER_H
#include "REL/m668DLL/rotor.h"
#include "game/mg/actman.h"
#include "game/charman.h"

/* Per-player state shared by the Burnstile object callbacks. */
typedef struct M668PlayerWork_s {
    s16 playerNo; /* Player configuration index, 0 through 3. */
    s16 partnerNo; /* Other player assigned to this player's team. */
    s16 charNo; /* Character ID copied from the player configuration. */
    s16 teamNo; /* Team index, 0 or 1. */
    s16 formationNo; /* Starting-position slot, 0 through 3. */
    MGPLAYER *player; /* Actor and minigame player state. */
    s16 state; /* Callback phase: 0 setup, 1 in play, 2 hit, 3 out. */
    HuVecF position[3]; /* Current/previous positions and launch displacement. */
    s16 hitRotorIndex; /* Rotor work index that hit this player. */
    s32 transitionFlag; /* Nonzero when rotor contact is detected or predicted. */
    s32 timer60; /* Active-frame delay before starting the winner motion. */
    HuVecF origin; /* Initial formation position retained in player work. */
    s32 sign; /* Randomized side choice, either -1 or 1. */
} M668PlayerWork;
#endif
