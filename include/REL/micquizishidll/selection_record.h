#ifndef REL_MICQUIZISHIDLL_SELECTION_RECORD_H
#define REL_MICQUIZISHIDLL_SELECTION_RECORD_H
#include "dolphin/types.h"
typedef struct MicquizSelectionEntry {
    s32 selection;
    s32 inUse;
    s16 modelId;
    s32 state;
} MicquizSelectionEntry;
#endif
