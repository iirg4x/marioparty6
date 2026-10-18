/* Player and opponent choices used by the Solo character-selection screens. */
#ifndef REL_MDSINGDLL_SELECTION_H
#define REL_MDSINGDLL_SELECTION_H

#include "dolphin/types.h"

typedef struct MdsingCharacterSelection {
    /* Zero while choosing a character; 1 after confirmation. */
    s16 choiceState;
    /* Reset to zero during setup; not otherwise read by Solo selection. */
    s16 setupValue;
    /* Zero for human selection, 1 for the computer opponent. */
    s16 isComputer;
    /* Computer difficulty copied from the player's game settings on return. */
    s16 difficulty;
    /* Character ID used to index the lineup and character-card models. */
    s16 characterNo;
    /* Controller index for input and human-player effect colors. */
    s16 padNo;
    /* Not accessed by the Solo selection functions. */
    s16 handicap;
} MDSING_CHARACTER_SELECTION;
typedef MDSING_CHARACTER_SELECTION LBL_1_BSS_9BC_ENTRY;

extern MDSING_CHARACTER_SELECTION lbl_1_bss_1308[4];

#endif
