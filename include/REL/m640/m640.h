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
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#ifndef M640_H
#define M640_H

/* Consumed scene layout: cleared as 20 bytes at fn_1_86C, halfword state
 * consumers, floating camera progress at +4, audio handle at +16. */
typedef struct M640Scene {
    s16 state;
    s16 frame;
    float cameraProgress;
    s16 finishCount;
    s16 winnerSide;
    s16 unkC;
    s32 stream;
} M640Scene;
typedef struct M640Player {
    s16 model;
    s16 motions[5];
    s16 playerNo;
    s16 charNo;
    s16 side;
    s16 slot;
    s16 padNo;
    s16 comDif;
    s16 delay;
    s16 error;
    s16 unk1C;
    s16 model1E;
    s16 model20;
    s32 sfx24;
    s32 sfx28;
    s16 model2C;
    float speed;
    s16 state;
    s16 unk36;
    s16 unk38;
    s16 angle;
} M640Player;
typedef struct M640Team {
    s16 model0;
    s16 model1;
    s16 model2;
    s16 model3;
    s16 model4;
    s16 model5;
    s16 hookModels[4];
    s16 auxModels[4];
    s16 model6;
    M640Player *players[2];
    s16 parts[4];
    s16 activeRecord;
    s16 targetBucket;
    /* The 56-byte indexed stride leaves these four bytes with no known
     * direct consumers. Their original declaration remains unknown. */
    u8 unknown34[4];
} M640Team;
typedef struct M640Piece {
    /* No recovered direct accesses to the first twelve bytes. The 40-byte
     * indexed record and clearing extent are target-backed. */
    u8 unknown0[12];
    float speed;
    s16 model;
    s16 motionsA[2];
    s16 motionsB[4];
    s16 part;
    s16 state;
    s16 variant;
    s16 frame;
} M640Piece;
typedef struct M640MotionEntry {
    s32 file;
    u32 attr;
} M640MotionEntry;
extern M640MotionEntry lbl_1_data_44[5];
extern M640Piece lbl_1_bss_C[12][2];
extern M640Player lbl_1_bss_3F8[4];
extern M640Team lbl_1_bss_4FC[2];
extern u32 lbl_1_data_84[12][2];
extern u32 lbl_1_data_E4[12][4];
extern s32 lbl_1_data_7C[2];
extern s16 lbl_1_bss_3E4[5];
extern s16 lbl_1_bss_3CC[3][4];
extern OMOBJ *lbl_1_bss_3F0;
extern OMOBJ *lbl_1_bss_3F4;
extern void *lbl_1_bss_8;
extern M640Scene lbl_1_bss_4E8;
extern s32 lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_4;
extern MGSEQ_PARAM lbl_1_data_0;
void fn_1_270(s16 frame);
void fn_1_60C(void);
void fn_1_868(s16 frame);
void fn_1_86C(void);
void fn_1_351C(s16 side, s16 arg1);
void fn_1_4CD8(void);
void fn_1_3090(void);
void fn_1_355C(void);
s16 fn_1_112C(s16 frame);
s32 fn_1_1C48(s32 frame);
void fn_1_8D4(void);
void fn_1_15C8(void);
void fn_1_2820(void);
void fn_1_5A28(void);
void fn_1_5DF0(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
void fn_1_26B8(s16 player, s16 motion);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_52D0(OMOBJ *obj);
void fn_1_60AC(OMOBJ *obj);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
s16 fn_1_33D0(float angle);

#endif
