#ifndef REL_M614DLL_STATE_H
#define REL_M614DLL_STATE_H

#include "game/object.h"
#include "game/mg/actman.h"

typedef struct CallbackStorage {
    void (*callback)(MGPLAYER *);
    /* The game's code never reads these bytes. */
    u8 unknown04[24]; /* No code names this field. */
} CallbackStorage;

typedef struct M614SoundQueueEntry {
    s32 count;
    s32 soundHandle;
} M614SoundQueueEntry;

typedef struct M614Obstacle {
    s32 state;
    f32 x;
    f32 z;
    f32 radius;
} M614Obstacle;

typedef struct {
    s32 values[3];
    /* The game's code never reads these bytes. */
    u8 unk_0C[8]; /* No code names this field. */
} M614SequenceState;

/* Storage whose leading field is the only one the game's code reads. */
typedef struct {
    u32 frame;
    u8 unknown04[12]; /* No code names this field. */
} M614FrameStorage;
typedef struct {
    f32 verticalOffset;
    u8 unknown04[8]; /* No code names this field. */
} M614CameraOffsetStorage;
typedef struct {
    s32 frame;
    u8 unknown04[0xA024]; /* No code names this field. */
} M614DebugStorage;

extern f32 lbl_1_bss_20;
extern f32 lbl_1_bss_24;
extern M614CameraOffsetStorage lbl_1_bss_28;
extern CallbackStorage lbl_1_bss_A060;
extern s32 lbl_1_bss_14;
extern s32 lbl_1_bss_10;
extern M614FrameStorage lbl_1_bss_0;
extern s32 lbl_1_bss_A0E4;
extern OMOBJ *lbl_1_bss_A0E8;
extern OMOBJ *lbl_1_bss_A0EC[4];
extern s16 lbl_1_bss_A0A4[4][5];
extern M614SoundQueueEntry lbl_1_bss_A104[20];
extern M614Obstacle lbl_1_bss_A1B8[5];
extern s32 lbl_1_bss_1C;
extern s32 lbl_1_bss_18;
extern s32 lbl_1_bss_34;
extern s16 lbl_1_bss_A07C[4][5];
extern s16 lbl_1_bss_A0CC[4];
extern s32 lbl_1_bss_A0D8;
extern s32 lbl_1_bss_A0D4;
extern M614SequenceState lbl_1_bss_A1A4;
extern OMOBJMAN *lbl_1_bss_A0E0;
extern s16 lbl_1_bss_A0DC;
extern OMOBJ *lbl_1_bss_A0FC;
extern OMOBJ *lbl_1_bss_A100;
extern M614DebugStorage lbl_1_bss_38;


#include "game/mg/seqman.h"
void fn_1_A38C(s16 mode, s16 frameNo);
void fn_1_6524(s16 mode, s16 frameNo);
void fn_1_6528(s16 mode, s16 frameNo);
void fn_1_652C(s16 mode, s16 frameNo);
void fn_1_6530(s16 mode, s16 frameNo);
void fn_1_6534(s16 mode, s16 frameNo);
void fn_1_6538(s16 mode, s16 frameNo);
void fn_1_653C(s16 mode, s16 frameNo);
void fn_1_A4D8(s16 mode, s16 frameNo);

#endif
