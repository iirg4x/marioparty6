#ifndef REL_MICQUIZOHDEDLL_SHARED_TYPES_H
#define REL_MICQUIZOHDEDLL_SHARED_TYPES_H

#include "dolphin/types.h"

/* Process pointer used by the quiz. */
struct Process_s;

/* Per-model counters and sound handles; the intervening bytes are unknown. */
typedef struct MicquizCameraState {
    s32 pendingCount;
    u8 unknown_04[0x18]; /* No code names this field. */
    s32 soundHandle;
} MicquizCameraState;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss28Storage {
    s32 value;
    u8 unknown_04[0x10]; /* No code names this field. */
} MicquizBss28Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss48Storage {
    s32 value;
    u8 unknown_04[0x10]; /* No code names this field. */
} MicquizBss48Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss60Storage {
    struct Process_s * value;
    u8 unknown_04[0x28]; /* No code names this field. */
} MicquizBss60Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss1C0Storage {
    s16 value;
    u8 unknown_02[0x8]; /* No code names this field. */
} MicquizBss1C0Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss238Storage {
    s16 value;
    u8 unknown_02[0xA]; /* No code names this field. */
} MicquizBss238Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss318Storage {
    s16 value;
    u8 unknown_02[0xE]; /* No code names this field. */
} MicquizBss318Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss334Storage {
    s32 value;
    u8 unknown_04[0x4]; /* No code names this field. */
} MicquizBss334Storage;

/* Storage with an accessed value and additional unknown bytes. */
typedef struct MicquizBss370Storage {
    f32 value;
    u8 unknown_04[0xC]; /* No code names this field. */
} MicquizBss370Storage;

#endif
