#ifndef OPTIONDLL_OPTION_H
#define OPTIONDLL_OPTION_H

#include "dolphin/types.h"

typedef struct OptionAudioEntry {
    s8 type; /* Zero selects streamed music, one selects sound effects, and -1 ends a list. */
    s32 marker; /* -1 for audio rows; zero for the terminating row. */
    s32 message; /* Message displayed for this sound test entry. */
    s32 audioId; /* Stream or sound effect identifier. */
} OptionAudioEntry;

void fn_1_EA6C(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_E688(struct OptionAudioEntry *entry);

#endif
