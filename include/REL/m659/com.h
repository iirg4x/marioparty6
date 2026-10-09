/* Computer-player manager state and side registration for Asteroad Rage. */
#ifndef M659_COM_H
#define M659_COM_H
#include "REL/m659/player.h"
typedef M659ComSlot M659Com;
typedef struct M659ComManager {
    M659Com *players[2]; /* Registered player-control slot for each side. */
    s16 state; /* Setup or lane-request phase selected by the manager callback. */
    int frame; /* Manager callback frame count, reset when setup completes and on each callback
                * in lane-control state 0. */
    int ready; /* Set before route planning; cleared when setup begins. */
} M659ComManager;
extern OMOBJ *lbl_1_bss_60;
#endif
