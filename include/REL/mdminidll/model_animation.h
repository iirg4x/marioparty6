/* Selection-card models, texture animations and saved transition state for minigame mode. */
#ifndef REL_MDMINIDLL_MODEL_ANIMATION_H
#define REL_MDMINIDLL_MODEL_ANIMATION_H
#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"

/* Character, mode and player-count cards share these animation records. */
typedef struct MDMinidllModelAnimationRecord {
    s16 model; /* Selection-card model handle. */
    s16 animation[4]; /* Texture-animation handles for the card's four animation slots. */
    Point3d savedPosition; /* Position captured at the start of a card transition. */
    Point3d savedRotation; /* Rotation captured at the start of a card transition, in degrees. */
    Point3d savedScale; /* Scale captured at the start of a card transition. */
    s16 highlightFrame; /* Highlighted-mode updates; rotation begins after 30 and this saturates
                        * at 35. */
    u8 unusedTail[6]; /* Not accessed by selection-card callbacks. */
} MDMinidllModelAnimationRecord;
extern MDMinidllModelAnimationRecord lbl_1_bss_230[];
#endif
