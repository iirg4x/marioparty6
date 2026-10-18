/* Board, difficulty, and minigame-pack choices for Solo Mode setup. */
#ifndef REL_MDSINGDLL_SETTINGS_H
#define REL_MDSINGDLL_SETTINGS_H

#include "dolphin/types.h"

typedef struct MdsingSettings {
    /* Active settings row: 0 is difficulty, 1 is the minigame pack. */
    s16 settingsRow;
    /* Board card 0-2; the alternate opening route assigns 3. */
    s16 boardNo;
    /* Computer difficulty 0-2, or 3 when the extra difficulty is unlocked. */
    s16 difficulty;
    /* Minigame-pack menu index, normally 0-4. */
    s16 minigamePack;
} MDSING_SETTINGS;

extern MDSING_SETTINGS lbl_1_bss_1340;

#endif
