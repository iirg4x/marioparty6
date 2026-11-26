#ifndef REL_MGMBINGODLL_MODULE_TYPES_H
#define REL_MGMBINGODLL_MODULE_TYPES_H
#include "REL/minigame_character_motion.h"

#include <math.h>
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

/* Window handles and rectangles for the bingo frame and text windows. */
typedef struct MgmbingoWindowWork {
    HUWINID frameWindow;
    f32 frameWidth;
    f32 frameHeight;
    HuVec2f framePosition;
    HUWINID textWindow;
    f32 textWidth;
    f32 textHeight;
    HuVec2f textPosition;
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
    u8 unknown_2C[8]; /* No code names this field. */
    f32 velocityY;
    u8 unknown_38[4]; /* No code names this field. */
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
extern char lbl_1_data_D8[3];
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
    u32 unknown_18; /* No code names this field. */
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

extern Point3d lbl_1_data_868[4];
extern MinigameCharacterMotion lbl_1_data_8B0[10];

extern MgmbingoWindowWork lbl_1_bss_C8;
extern OMOBJ *lbl_1_bss_A8;
extern OMOBJ *lbl_1_bss_A30;
extern OMOBJ *lbl_1_bss_AE4;
extern OMOBJ *lbl_1_bss_ACC[4];

extern Point3d lbl_1_bss_A3C[4];
/* Five rows of five board-cell positions. */
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

/* Window callback for message cues. */
void fn_1_3A60(HUWINID winId, u32 mess, s16 cueId);
void fn_1_4F8(void);
void fn_1_109E4(void);
void BingoShowFollowupInstructions(void);
void BingoShowLineCountHint(void);
void BingoCloseLineCountHint(void);
s16 BingoPromptTwoChoices(void);
HU3D_MODELID fn_1_0(s16 modeNo, s16 timeNo, Point3d *position);
void fn_1_2A4(s16 minigameCount, u8 *minigameOrder);
void fn_1_43C(void);
void fn_1_5E0(s16 modeNo, s16 timeNo);
void fn_1_6F4(void);
void fn_1_7F0(void);
void fn_1_824(void);
void fn_1_854(s16 layerNo);
void fn_1_898(HU3D_MODEL *model, Mtx *matrix);
void fn_1_E9C(void);
void fn_1_1088(void);
void fn_1_108C(s16 pathIndex, s16 durationFrames);
void fn_1_10CC(OMOBJ *cameraPath);
void fn_1_1214( HU3D_CAMERA *camera, HuVecF *cameraPosition, HuVecF *cameraTarget,
               const HuVecF *pathPosition, const HuVecF *pathTarget, f32 duration, f32 frame);
void fn_1_1720(void);
void fn_1_1970(void);
void fn_1_1B08(void);
void fn_1_1ED8(void);
void fn_1_2114(void);
s16 fn_1_26EC(void);
void fn_1_2B10(void);
void fn_1_2C20(void);
void fn_1_2DB4(void);
void fn_1_3074(void);
void fn_1_30C8(void);
void fn_1_311C(s16 player);
void fn_1_32CC(s16 player);
void fn_1_347C(void);
void fn_1_34B0(void);
s16 fn_1_38E4(void);
void fn_1_3B6C(void);
void fn_1_4300(s16 eventNo);
void fn_1_4FB4(s16 animationEnabled);
void fn_1_50B4(void);
void fn_1_54FC(s16 playerNo);
void fn_1_70AC(s16 playerIndex);
void fn_1_799C(s16 enable);
s16 fn_1_7AD0(s16 playerIndex);
s16 fn_1_7E6C(s16 playerIndex);
s16 fn_1_8250(s16 playerIndex);
s16 fn_1_853C(s16 playerIndex);
void fn_1_8BDC(s16 playerIndex, s16 requiredMarks, s16 *lineScores);
s16 fn_1_8FA8(s16 *scores);
s16 fn_1_901C(s16 *entryValues);
void fn_1_90F4(void);
void fn_1_9554(void);
void fn_1_A2A4(void);
void fn_1_AAC8(HuVecF *position);
void fn_1_B498(void);
void fn_1_BDB4(void);
void fn_1_C578(void);
void fn_1_C848(void);
u16 fn_1_C8B8(s16 playerIndex, s16 targetCell);
s16 fn_1_CEF0(s16 playerNo);
s16 fn_1_CF30(s16 playerNo);
s16 fn_1_CF74(s16 playerNo);
void fn_1_CFA8(void);
void fn_1_D538(void);
s16 fn_1_F018(void);
void fn_1_F704(void);
void fn_1_FCF4(void);
void fn_1_FD14(void);
void fn_1_10548(void);
void fn_1_10970(u16 enabled);
void fn_1_109FC(s16 enable);
void fn_1_10A64(s16 positionIndex, s16 durationFrames);
void fn_1_1217C(void);
void fn_1_12BE0(f32 angleX, f32 angleY, f32 angleZ, const Vec *source, Vec *destination);

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
