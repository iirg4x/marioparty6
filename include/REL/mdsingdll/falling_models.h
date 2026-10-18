/* Reusable falling reward models and their per-model movement state. */
#ifndef REL_MDSINGDLL_FALLING_MODELS_H
#define REL_MDSINGDLL_FALLING_MODELS_H
#include "game/object.h"

typedef struct MdsingFallingModelWork {
    /* Not accessed by the falling-model callbacks. */
    u32 unknownValue;
    /* Extra downward world units per update, added to the base speed of 15. */
    f32 fallSpeed;
    /* Y-axis rotation in degrees per update. */
    f32 rotationSpeed;
} MDSING_FALLING_MODEL_WORK;

extern MDSING_FALLING_MODEL_WORK lbl_1_bss_58[64];
extern s16 lbl_1_bss_358[64];
extern s16 lbl_1_bss_3D8;
extern OMOBJ *lbl_1_bss_3DC;

#endif
