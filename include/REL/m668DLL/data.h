#ifndef M668_DATA_H
#define M668_DATA_H
#include "game/mg/seqman.h"

/* The API consumes the first 0x28 bytes. Retail preserves another 20 initialized
 * bytes here; their original declaration/purpose is unknown, not padding. */
typedef struct M668SequenceDataRecord {
    MGSEQ_PARAM param;
    u8 unknown_28[20];
} M668SequenceDataRecord;
extern M668SequenceDataRecord lbl_1_data_0;
#endif
