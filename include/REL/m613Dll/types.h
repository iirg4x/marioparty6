#ifndef REL_M613DLL_TYPES_H
#define REL_M613DLL_TYPES_H

#include "dolphin/types.h"
#include "REL/m613Dll/geometry.h"
#include <math.h>
#include "game/hsfformat.h"

/* Tracks the board models and the green bitmap's grid of remaining pixel references. */
typedef struct M613BitmapState {
    s16 modelIds[5];
    s16 referenceCounts[10][10];
    s32 state;
} M613BitmapState;

extern M613BitmapState lbl_1_bss_1C;
extern HSF_BITMAP *lbl_1_bss_10;
extern HSF_BITMAP *lbl_1_bss_14;
extern HSF_BITMAP *lbl_1_bss_18;

extern s32 lbl_1_bss_10C[4];
extern s32 lbl_1_bss_FC[4];

/* Per-player game state, models, drawing position, and controller input. */
typedef struct M613PlayerState {
    s16 playerIndex;
    s16 characterNo;
    s16 padNo;
    s16 groupNo;
    s32 playerType;
    s16 cpuDifficulty;
    s16 sequenceState;
    s16 state;
    Point3d displacement;
    Point3d rotationOffset;
    s32 resultPositionMode;
    s32 drawingActive;
    s32 clearedPixelCount;
    s16 model;
    s16 motionIds[5];
    s16 motionState;
    Point3d hookPosition;
    f32 hookRadius;
    f32 characterHeight;
    s16 motionTransitionTimer;
    s16 modelPart0;
    s16 modelPart1;
    Point3d position;
    Point3d rotation;
    f32 radius;
    Point3d firstObjectPosition;
    Point3d secondObjectPosition;
    s8 stickX;
    s8 stickY;
    u16 buttons;
    u16 buttonsDown;
    s16 contactFeedbackTimer;
    s32 hasTarget;
    Point3d targetPosition;
    s16 cpuCollisionDelay;
    s16 recoveryTime;
    s16 movementTime;
    s16 turnTime;
} M613PlayerState;

typedef struct M613MotionEntry {
    s16 motionIndex;
    f32 duration;
    u32 attributes;
} M613MotionEntry;

extern M613PlayerState lbl_1_bss_11C[4];
extern M613MotionEntry lbl_1_data_FC[6];

/* Model and score shown by each player's progress display. */
typedef struct M613ProgressState {
    s16 model;
    s16 effectModel;
    s32 progress;
    f32 verticalScale;
    s32 previousProgress;
} M613ProgressState;

typedef struct M613ValueKey {
    s16 value;
    s16 key;
} M613ValueKey;

extern M613ProgressState lbl_1_bss_1218[4];
extern M613ValueKey *lbl_1_bss_C;

extern s16 lbl_1_bss_1210[4];
extern s16 lbl_1_bss_D60[3][200];
extern s32 lbl_1_bss_400[3][200];
void fn_1_11F8(void);
void fn_1_18FC(void);
void fn_1_1A7C(void);
s16 fn_1_1D7C(void);
s16 fn_1_1D84(void);
s16 fn_1_1EF4(s16 row, s16 column);
void fn_1_1F54(s16 row, s16 column, Point3d *position);
s32 fn_1_20D0(const Point3d *point0, const Point3d *point1, const Point3d *point2,
              const Point3d *point3);
void fn_1_3FFC(f32 angle_x, f32 angle_y, f32 angle_z, const Vec source, Vec *destination);
void fn_1_4208(void);
void fn_1_5058(void);
f32 fn_1_99A4(Point3d movingPosition, Point3d displacement, f32 radius, u8 side);
void fn_1_9C7C(s16 playerIndex);
void fn_1_B4A8(void);
void fn_1_C2AC(void);
s32 fn_1_C850(s16 frameNo);
void fn_1_D0C8(void);
void fn_1_D61C(void);
void fn_1_DBDC(s16 playerIndex);
void fn_1_DCF4(s16 playerIndex);
void fn_1_DD54(void);
void fn_1_DE20(s16 playerIndex);
void fn_1_F3C4(void);
void fn_1_F568(void);
void fn_1_F6E0(s16 playerIndex, Point3d *position);
void fn_1_F850(void);
s16 fn_1_FA48(void);
void fn_1_FC40(void);
void fn_1_1054C(const Point3d *sourcePosition, const Point3d *targetPosition, s16 playerIndex,
                s32 progressAmount);
void fn_1_106C0(void);
s32 fn_1_10C5C(Point3d *direction, Point3d *unitDirection);
s32 fn_1_10DB4(f32 numerator, f32 denominator, f32 *ratio);
f64 fn_1_10E3C(f64 x, f64 y);

#endif
