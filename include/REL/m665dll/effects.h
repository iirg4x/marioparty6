/* Projectile, impact, and score-icon animation storage for Verbal Assault. */
#ifndef REL_M665DLL_EFFECTS_H
#define REL_M665DLL_EFFECTS_H

#include "dolphin/types.h"

typedef struct M665HitWork {
    s16 model; /* Hit-effect model ID. */
    s16 impactModel; /* Impact model ID. */
    s16 soundPlayerIndex; /* Player-work index used to select impact sounds; cleared to zero at
                           * setup and not assigned on spawn. */
    s16 state; /* Hit-effect state. */
} M665HitWork;

struct AnimData_s;

typedef struct M665AnimationStorage {
    struct AnimData_s *animation; /* Animation loaded for the opponent score icons. */
    u8 unreadAfterAnimation[12]; /* No M665 consumer reads the storage after the animation
                                  * pointer. */
} M665AnimationStorage;

extern M665HitWork lbl_1_bss_CA8[64];
extern M665AnimationStorage lbl_1_bss_EA8;

#endif
