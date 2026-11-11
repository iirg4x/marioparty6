#ifndef REL_MICQUIZOHDEDLL_H
#define REL_MICQUIZOHDEDLL_H

#include "dolphin/types.h"
#include "game/object.h"
#include "game/process.h"
#include "game/mic.h"
#include "REL/micquiz/quiz.h"

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
    HUPROCESS *value;
    u8 unknown_04[0x28]; /* No code names this field. */
} MicquizBss60Storage;

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

/* The response of the last microphone choice. */
typedef struct MicquizResponseStorage {
    MicquizResponse *response;
    u32 unk04; /* never read */
} MicquizResponseStorage;

/* main.c */
extern MicquizResponseStorage lbl_1_bss_1C;
extern s16 lbl_1_bss_24;
extern MicquizBss28Storage lbl_1_bss_28;
extern MicquizBss60Storage lbl_1_bss_60;
extern s32 lbl_1_data_388[0x36];
extern s32 lbl_1_data_4BC;
extern s32 lbl_1_data_52C;
s32 fn_1_760(s32 left, s32 right);

/* quiz_scene.c */
void fn_1_12D08(void);
void fn_1_1D4AC(OMOBJ *object);

/* Not declared by the game's headers. */
void HuMCSelWinMaxTimeSet(f32 maxTime);

#endif
