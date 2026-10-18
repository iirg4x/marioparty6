/* Selection model object and its associated unknown storage in Solo Mode. */
#ifndef REL_MDSINGDLL_OBJECT_STATE_H
#define REL_MDSINGDLL_OBJECT_STATE_H

#include "game/object.h"

typedef struct MdsingSelectionObject {
    /* Object owning the four selection models and their settling callback. */
    OMOBJ *object;
    /* The selection callbacks do not access these bytes. */
    u8 unknownBytes[4];
} MDSING_SELECTION_OBJECT;

extern MDSING_SELECTION_OBJECT lbl_1_bss_18;

#endif
