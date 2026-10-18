/* Question-grid handles, screen positions, and recognized-answer records. */
#ifndef REL_MICQUIZDLL_SOURCE_TYPES_H
#define REL_MICQUIZDLL_SOURCE_TYPES_H

#include "REL/micquizdll/module_types.h"

typedef struct MicquizModelSlots_s {
    HU3D_MODELID model[5]; /* Handles for one strip of grid models or texture animations. */
} MicquizModelSlots;

typedef struct MicquizPosition_s {
    f32 x; /* Horizontal sprite coordinate, in pixels. */
    f32 y; /* Vertical sprite coordinate, in pixels. */
} MicquizPosition;

typedef struct MicquizChoiceRecord_s {
    s32 answerIndex; /* Zero-based recognized answer index. */
    s32 alternateFollowup; /* Nonzero chooses the alternate recognized-answer follow-up. */
    u8 opaqueBeforeState[4]; /* Bytes not read by these answer lookup helpers. */
    s32 availabilityState; /* Zero allows this record to supply recognized-answer feedback. */
} MicquizChoiceRecord;

#ifdef __cplusplus
extern "C" {
#endif

extern MicquizModelSlots lbl_1_bss_198[5];
extern MicquizPosition lbl_1_bss_304;
extern HU3D_MODELID lbl_1_bss_2A6[3];
extern HUPROCESS *lbl_1_bss_548;
extern MicquizChoiceRecord lbl_1_bss_54C[8];
extern HUSPR_GROUPID lbl_1_bss_460[3];
extern s16 lbl_1_bss_386;

void fn_1_31FCC(void);
void fn_1_320F8(void);
void fn_1_32250(const Vec *position, s32 pieceCount);

#ifdef __cplusplus
}
#endif

#endif
