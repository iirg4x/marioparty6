/* Menu card models, their texture animations, and saved transition transforms. */
#ifndef REL_MDSINGDLL_MODEL_H
#define REL_MDSINGDLL_MODEL_H

#include "game/hu3d.h"

typedef struct MdsingModel {
    /* Linked menu card model and the four texture surfaces animated on it. */
    HU3D_MODELID modelId;
    HU3D_ANIMID animationId[4];
    /* No field accesses by the menu card callbacks. */
    s16 unknownValue;
    /* Starting position, rotation in degrees, and scale captured for a transition. */
    HuVecF position;
    HuVecF rotation;
    HuVecF scale;
    /* Updates since board highlighting began; rotation starts after thirty. */
    s16 highlightFrame;
    /* No field accesses by the menu card callbacks. */
    u8 trailingBytes[6];
} MDSING_MODEL;

typedef MDSING_MODEL LBL_1_BSS_288_ENTRY;
extern MDSING_MODEL lbl_1_bss_E74[16];

#endif
