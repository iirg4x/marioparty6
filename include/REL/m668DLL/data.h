/* Burnstile sequence parameter record layout. */
#ifndef M668_DATA_H
#define M668_DATA_H
#include "game/mg/seqman.h"

/* Sequence API parameters followed by initialized bytes retained with the data. */
typedef struct M668SequenceDataRecord {
    MGSEQ_PARAM param; /* Startup and per-mode sequence callbacks. */
    u8 reservedBytes[20]; /* Initialized sequence bytes with unknown gameplay role. */
} M668SequenceDataRecord;
extern M668SequenceDataRecord lbl_1_data_0;
#endif
