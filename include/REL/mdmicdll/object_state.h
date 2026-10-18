/* Object-manager and object references shared by Mic Mode scenes. */
#ifndef REL_MDMICDLL_OBJECT_STATE_H
#define REL_MDMICDLL_OBJECT_STATE_H

#include "game/object.h"

extern OMOBJMAN *lbl_1_bss_0; /* Scene object manager. */
extern OMOBJ *lbl_1_bss_4; /* Three mode models and the mode-heading fade callback. */
extern OMOBJ *lbl_1_bss_8; /* Paired preview models and selection-heading fade state. */
extern OMOBJ *lbl_1_bss_C[2]; /* Host model object slots. */
extern OMOBJ *lbl_1_bss_1C[2]; /* Model objects used for selection markers. */
extern OMOBJ *lbl_1_bss_38; /* Positions record-time digits below the selected activity model's
                             * projected origin. */

#endif
