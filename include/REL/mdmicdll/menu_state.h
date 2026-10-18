/* Selections and saved setup state shared by Mic Mode menus. */
#ifndef REL_MDMICDLL_MENU_STATE_H
#define REL_MDMICDLL_MENU_STATE_H

#include "dolphin/types.h"

/* Choices shared by the microphone mode setup screens. */
typedef struct {
    s16 participantCount; /* Requested human-player count; remaining player slots use computers. */
    s16 modeSelection; /* Selected mode, from 0 through 2. */
    s16 gameSelection; /* Selected game for the setup. */
    s16 handicapPlayer; /* Player selected to receive the handicap. */
    s16 playerOrderSelection; /* Player-order choice, from 0 through 3. */
    u8 unknown[14]; /* Unknown bytes following the setup choices. */
} MDMIC_MENU_STATE;

extern MDMIC_MENU_STATE lbl_1_bss_9DC;

#endif
