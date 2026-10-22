/* Actor work state used by Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_RECOVERED_H
#define REL_M666DLL_RECOVERED_H

#include "game/mg/actman.h"
#include "REL/m666dll/tables.h"

extern u16 lbl_1_bss_2;

typedef struct M666ActorWork {
    MGACTOR *actor;
    s16 modelIds[2];
    s16 state;
    s16 pointIndex;
    s16 countdown;
    s16 impactState;
    s16 motionState;
    f32 opacity;
    /* Vertical step integrated into model Y; acceleration adds 2.5 and impact scales it by -0.2. */
    f32 verticalStep;
} M666ActorWork;

extern M666ActorWork lbl_1_bss_E3C[3];

#endif
