#ifndef REL_MGMBINGODLL_MODULE_TYPES_H
#define REL_MGMBINGODLL_MODULE_TYPES_H

/* Canonical module context; no generated M2C declarations are included. */
#ifndef _MATH_H
#include "dolphin/math.h"
#endif
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/mgdata.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "datadir_enum.h"
#include "string.h"

/* The fields below are the target-backed members used by this module's
 * window helpers. */
typedef struct MgmbingoWindowWork {
    HUWINID frameWindow;       /* +0x00 */
    f32 frameWidth;            /* +0x04 */
    f32 frameHeight;           /* +0x08 */
    f32 frameX;                /* +0x0C */
    f32 frameY;                /* +0x10 */
    HUWINID textWindow;        /* +0x14 */
    f32 textWidth;             /* +0x18 */
    f32 textHeight;            /* +0x1C */
    f32 textX;                 /* +0x20 */
    f32 textY;                 /* +0x24 */
} MgmbingoWindowWork;

/* Character, controls and animation state for each bingo player. */
typedef struct MgmbingoPlayerData {
    s16 playerIndex;
    s16 characterNo;
    s16 padIndex;
    s16 groupNo;
    s32 inputMode;
    s16 difficulty;
    s16 state;
    s16 currentMotion;
    Point3d position;
    Point3d rotation;
    u8 unknown_2C[8]; /* Cleared when the player is created; remains unread during play. */
    f32 velocityY;
    u8 unknown_38[4]; /* Cleared when the player is created; remains unread during play. */
    Point3d targetPosition;
} MgmbingoPlayerData;

typedef struct {
    u32 messageId;
    s16 cueId;
    u16 value_6;
    s32 soundEffectId;
} MgmbingoMessageSound;

extern s32 lbl_1_data_2A8[71];
extern s32 lbl_1_data_3C4[3];
extern char *lbl_1_data_400[4];
extern char *lbl_1_data_440[4];
extern char *lbl_1_data_51C[30];
extern char *lbl_1_data_5A0;
extern Point3d lbl_1_data_6F4;
extern s32 lbl_1_data_700[5];
extern s32 lbl_1_data_714[14];
extern Point3d lbl_1_data_74C[5][4];
extern s16 lbl_1_data_83C[4];

extern u32 lbl_1_data_B0[10];
extern u8 lbl_1_data_D8[3];
extern MgmbingoMessageSound lbl_1_data_114[33];
extern s16 lbl_1_data_2A0;
extern u32 lbl_1_data_848[8];
extern Point3d lbl_1_data_898[2];
extern s32 lbl_1_data_9D0[8];
extern Point3d lbl_1_data_9F0;
extern Point3d lbl_1_data_9FC[8];
extern Point3d lbl_1_data_A60[7];
extern s32 lbl_1_data_AD0;
extern s32 lbl_1_data_AD4;
extern int lbl_1_data_AD8;
extern s32 lbl_1_data_AC8;
extern s32 lbl_1_data_ACC;
extern s32 lbl_1_data_90[8];
extern u32 lbl_1_data_DC[14];

typedef struct MgmbingoInputEvent {
    s16 frame;
    u16 button;
} MgmbingoInputEvent;

typedef struct MgmbingoCameraPath {
    HuVecF position;
    HuVecF target;
} MgmbingoCameraPath;

typedef struct MgmbingoCameraState {
    MgmbingoCameraPath path;
    u32 unknown_18;
} MgmbingoCameraState;

extern MgmbingoCameraState lbl_1_bss_AC;
extern MgmbingoCameraPath lbl_1_data_0[6];

typedef struct MgmbingoBoardSnapshot {
    s16 cells[25];
} MgmbingoBoardSnapshot;

extern MgmbingoBoardSnapshot lbl_1_bss_2B6[4];
extern s16 lbl_1_bss_37E[4];
extern s16 lbl_1_bss_39E[4][30];
extern s16 lbl_1_bss_48E[4][30];
extern s16 lbl_1_bss_57E[4][30];
extern HSF_OBJECT *lbl_1_bss_670[4][30];
extern HSF_OBJECT *lbl_1_bss_850[4][30];
extern s16 lbl_1_bss_F0;

typedef struct BingoMotionSetting {
    s16 motionIndex;
    f32 duration;
    u32 modelAttributes;
    s32 voiceFlag;
    s32 setMotionAttribute;
} BingoMotionSetting;

extern Point3d lbl_1_data_868[4];
extern BingoMotionSetting lbl_1_data_8B0[10];

extern MgmbingoWindowWork lbl_1_bss_C8;
extern OMOBJ *lbl_1_bss_A8;
extern OMOBJ *lbl_1_bss_A30;
extern OMOBJ *lbl_1_bss_AE4;
extern OMOBJ *lbl_1_bss_ACC[4];

/* The target data object is 0x12C bytes: 25 contiguous Point3d values.  All
 * scoped consumers read it and none writes it. */
extern Point3d lbl_1_bss_A3C[4];
extern Point3d lbl_1_data_5A4[5][5];

extern HU3D_MODELID lbl_1_bss_39C;
extern HUSPR_GROUPID lbl_1_bss_A2;
extern ANIMDATA *lbl_1_bss_98;
extern ANIMDATA *lbl_1_bss_9C;
extern void *lbl_1_bss_94;
extern ANIMDATA *lbl_1_bss_84;
extern ANIMDATA **lbl_1_bss_8C;
extern s16 lbl_1_bss_0;
extern MGDATA *lbl_1_bss_90;
extern s16 lbl_1_bss_A0;
extern s16 lbl_1_bss_AE0;
extern s16 lbl_1_bss_AF8;

/* Target/provider contracts.  fn_1_3A60 is installed through HUWIN_CALLBACK;
 * fn_1_4F8 is called at ten sites with no arguments and every caller ignores
 * its return value.  fn_1_109E4 is the callback's target-side state provider. */
void fn_1_3A60(HUWINID winId, u32 mess, s16 cueId);
void fn_1_4F8(void);
void fn_1_109E4(void);

extern HU3D_MODELID lbl_1_bss_A4;
extern s16 lbl_1_bss_88;
extern u8 lbl_1_bss_2[130];
extern s16 lbl_1_bss_A34[4];
extern OMOBJ *lbl_1_bss_398;
extern ANIMDATA *lbl_1_bss_388[4];
extern s16 lbl_1_bss_2B4;
extern ANIMDATA *lbl_1_bss_28C[10];
extern s16 lbl_1_bss_288;
extern s16 lbl_1_bss_274[10];
extern s16 lbl_1_bss_24C[20];
extern OMOBJ *lbl_1_bss_248;
extern Point3d lbl_1_bss_1AC[13];
extern Point3d lbl_1_bss_110[13];
extern s16 lbl_1_bss_F4[14];
extern s16 lbl_1_bss_F2;
extern MgmbingoInputEvent lbl_1_bss_AA4[10];
extern ANIMDATA *lbl_1_bss_A84[8];
extern s16 lbl_1_bss_A82;
extern s16 lbl_1_bss_A72[8];
extern s16 lbl_1_bss_A70;
extern u32 lbl_1_bss_AF4;
extern Point3d lbl_1_bss_AE8;
extern OMOBJMAN *lbl_1_bss_B00;
extern s16 lbl_1_bss_AFC;
extern s16 lbl_1_bss_AFA;

#endif /* REL_MGMBINGODLL_MODULE_TYPES_H */
