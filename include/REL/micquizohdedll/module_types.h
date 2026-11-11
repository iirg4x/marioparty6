#ifndef REL_MICQUIZOHDEDLL_MODULE_TYPES_H
#define REL_MICQUIZOHDEDLL_MODULE_TYPES_H

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

typedef struct MicQuizResponse {
    s16 status;
    u16 reserved;
    u32 contextData[2];
    s32 choice;
} MicQuizResponse;

typedef struct MicQuizResponseStorage {
    MicQuizResponse *response;
    u32 unknown; /* Unknown word. */
} MicQuizResponseStorage;

typedef struct MicquizResultCopy {
    s32 key;
    s32 result;
    u8 unknown_08[4]; /* No code names this field. */
    s32 state0C;
} MicquizResultCopy;
extern ANIMDATA *lbl_1_bss_30;
extern ANIMDATA *lbl_1_bss_34;
extern void *lbl_1_bss_568;
extern void *lbl_1_bss_55C[2];
void fn_1_548(OMOBJ *obj);
void fn_1_6D8(void);
void fn_1_19AE4(void);
void fn_1_558(OMOBJ *obj);
void fn_1_14F00(s32 sourceLeftInput, s32 sourceTopInput, s32 sourceWidthInput,
                s32 sourceHeightInput, s32 textureFormatInput, f32 speed);
void fn_1_15F60(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 speed);
void fn_1_16804(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 strength);
void fn_1_17478(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                enum _GXTexFmt textureFormat, f32 strength);
void fn_1_187F0(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                GXTexFmt textureFormat, f32 strength);
void fn_1_195DC(s16 unused);
void fn_1_19B24(s16 level);
void fn_1_1A550(void);
void fn_1_1AA0C(s16 unused);
void fn_1_1ADAC(void);
void fn_1_1BD68(void *hookContext, f32 (*positionMatrix)[4]);
void fn_1_1C088(HU3D_MODEL *model, f32 (*positionMatrix)[4]);
void ObjectSetup(void);

#endif
