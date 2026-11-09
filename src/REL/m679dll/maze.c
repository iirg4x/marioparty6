/* Lab Brats: random maze generation, the maze, wall and goal models, player movement and wall
 * collision, the camera, the history of visited cells, and the round flow. */
#include <math.h>

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
#include "messdir_enum.h"

#include "REL/m679dll/module_state.h"

#include "dolphin/os.h"

#include "game/window.h"

#include "game/esprite.h"

#include <string.h>

#include <stdio.h>
#include "humath.h"

/* Cell edges, in the order consumed by maze generation and collision. */
typedef struct M679Cell_s {
    u8 negativeZ;
    u8 positiveZ;
    u8 negativeX;
    u8 positiveX;
} M679Cell;

typedef struct M679TilePosition_s { s16 x, z; } M679TilePosition;

typedef struct M679MazeLevel_s {
    s16 characterNo;
    s16 shadowModel;
    s16 baseModel;
    u8 sideLength;
    M679Cell cells[256];
    u8 unknownByte;
    M679TilePosition currentTile;
    s16 goalColumn;
    s16 goalRow;
    s16 startColumn;
    s16 startRow;
    s16 exitColumn;
    s16 exitRow;
} M679MazeLevel;

typedef union M679ControllerValue40_u { s32 word; s16 halves[2]; } M679ControllerValue40;

typedef struct M679Controller_s {
    s16 playerIndex;
    s16 modelId;
    s16 menuIndex;
    s16 padIndex;
    u8 unk_08[0x20]; /* No code names this field. */
    MGPLAYER *player;
    s16 frame;
    s16 unknownShort;
    f32 animationDuration;
    f32 shadowAlpha;
    s32 state;
    s32 inputActive;
    M679ControllerValue40 value40;
    s32 playerArrivalState;
    s16 currentIndex;
    s16 progress;
    s32 playerState[5];
    s16 resultIndex;
    s16 collisionTileX;
    s16 collisionTileZ;
    s16 unknownTail;
} M679Controller;

typedef struct M679Window_s {
    s16 phase;
    s16 primaryModel;
    s16 primaryMotion;
    s16 alternateMotion;
    s16 shadowModel;
    s16 mapModel;
    s16 shadowMotion;
    s16 window;
    s32 transitionState;
    s32 soundHandle;
    s32 effectIds[10];
    s16 soundIndex;
    s16 windowState;
    s32 soundStarted;
} M679Window;

typedef struct M679PlayerWork_s M679PlayerWork;

struct M679PlayerWork_s {
    s16 characterNo;
    s16 modelId;
    s16 motions[4];
    s16 effectId;
    s16 playerIndex;
    s16 eventIndex;
    s16 unknownShort;
    s32 state;
    s16 motionCounter;
    s16 phase;
    M679Controller *owner;
    s16 action;
    s16 model;
    s16 catchTarget; /* Set to -1 with the target cell when the player catches this character. */
    s32 moving;
    s16 targetColumn;
    s16 targetRow;
    Point3d targetPosition;
    s16 progress;
    s16 unknownTail;
};

typedef struct M679HistoryEntry_s { s16 first, second; } M679HistoryEntry;

typedef struct M679Queue_s {
    s32 count;
    s16 values[100];
    s16 source[100];
    u8 unknown_194[40]; /* No code names this field. */
} M679Queue;

typedef struct M679ModelPair_s { s16 first, second; } M679ModelPair;

typedef struct M679RegionPoint_s {
    Point3d position;
    Point3d rotation;
    M679HistoryEntry coordinate;
    u8 unknown_1C[4]; /* No code names this field. */
} M679RegionPoint;

extern s32 lbl_1_data_CC;

/* Builds five random mazes, then records their paths, goals, and edge states. */
typedef struct M679MazeCoordinate_s {
    s16 column;
    s16 row;
} M679MazeCoordinate;

typedef struct M679WallModel_s {
    s16 modelIds[4];
} M679WallModel;

void fn_1_1A0(HuVecF *location, M679Cell *color);

void fn_1_250(s32 column, s32 row, M679Cell *color);

s16 fn_1_2A0(s16 index);

s16 fn_1_2EC(M679Queue *queue);

void fn_1_1238(M679Queue *work, s16 modelId);

s16 fn_1_374(void);

s16 fn_1_3FC(void);

void fn_1_484(s16 value);

void fn_1_4C4(s16 value);

s16 fn_1_504(void);

s16 fn_1_58C(void);

void fn_1_614(s16 value);

void fn_1_654(s16 value);

void fn_1_694(M679Queue *transfer);

void fn_1_6E4(void);

void fn_1_77C(void);

void fn_1_7B8(void);

void fn_1_7FC(M679HistoryEntry *entry);

void fn_1_860(M679HistoryEntry *entry);

void fn_1_8C4(void);

void fn_1_95C(void);

void fn_1_998(void);

void fn_1_9DC(M679HistoryEntry *entry);

void fn_1_AD4(M679HistoryEntry *entry);

void fn_1_B38(void);

void fn_1_B78(OMOBJ *obj);

void fn_1_E1C(OMOBJ *obj);

void fn_1_11FC(OMOBJ *object);

void fn_1_1220(OMOBJ *object);

void fn_1_1434(void);

void fn_1_17E0(OMOBJ *object);

s32 fn_1_1AE8(u8 *grid, s16 column, s16 row, s16 width, s16 height, s32 option);

s32 fn_1_1BF0(u8 *grid, s32 seed, s16 startColumn, s16 startRow,
    s32 level);

void fn_1_2D7C(void);

void fn_1_3B38(void);

void fn_1_3B78(void);

void fn_1_3DD8(void);

void fn_1_3FA4(OMOBJ *object);

void fn_1_4AA4(OMOBJ *object);

void fn_1_4AC8(OMOBJ *object);

void fn_1_50D4(s32 phase);

void fn_1_5930(s32 modelSetIndex, Point3d *tilePosition);

s32 fn_1_5DF0(MGACTOR *response, Point3d *position);

void fn_1_61E8(void);

void fn_1_64DC(Point3d *position);

void fn_1_6568(void);

void fn_1_65D4(void);

void fn_1_6640(void);

void fn_1_6680(OMOBJ *obj);

void fn_1_67BC(OMOBJ *object);

void fn_1_67E0(OMOBJ *obj);

void fn_1_6BD0(MGACTOR *actor);

s32 fn_1_6E48(void);

void fn_1_6E50(OMOBJ *object);

void fn_1_6EB4(OMOBJ *object);

void fn_1_70EC(OMOBJ *object);

void fn_1_7270(OMOBJ *object);

void fn_1_732C(OMOBJ *object);

void fn_1_742C(OMOBJ *object);

void fn_1_74EC(OMOBJ *object);

s32 fn_1_7798(void);

s32 fn_1_7AF4(s32 region, Point3d *position, Point3d *rotation);

void fn_1_7D94(void);

void fn_1_7E38(OMOBJ *object);

void fn_1_8DC8(OMOBJ *object);

void fn_1_9430(OMOBJ *obj);

void fn_1_95A4(void);

void fn_1_96F4(s32 region, Point3d *position);

s32 fn_1_97CC(s32 region, M679HistoryEntry *gridCoordinate, Point3d *worldPosition);

void fn_1_9AA0(M679PlayerWork *work, s16 motionIndex, u32 attributes);

f32 fn_1_9B04(M679PlayerWork *work);

int fn_1_9C18(HuVecF *sourceDirection, HuVecF *normalizedDirection);

s32 fn_1_9D70(M679PlayerWork *movingModel,
    Point3d *targetPosition, f32 speed, f32 arrivalRadius);

void fn_1_A09C(OMOBJ *object);

void fn_1_AB08(OMOBJ *object);

s32 fn_1_AB9C(s32 column, s32 row);

void fn_1_ABFC(HuVecF *pair);

void fn_1_AC20(HuVecF *pair);

void fn_1_AC44(HuVecF *pair);

void fn_1_AC68(HuVecF *pair);

void fn_1_AC8C(HuVecF *pair);

void fn_1_ACB0(HuVecF *pair);

void fn_1_ACD4(HuVecF *pair);

void fn_1_ACF8(HuVecF *pair);

void fn_1_AD1C(HuVecF *pair);

s32 fn_1_AD40(s32 mode);

void fn_1_B1F8(M679PlayerWork *moving);

void fn_1_C100(M679PlayerWork *character);

void fn_1_C7C4(OMOBJ *object);

void fn_1_C89C(OMOBJ *object);

void fn_1_C8C0(OMOBJ *object);

void fn_1_CDD0(void);

void fn_1_D0B0(void);

void fn_1_D144(void);

extern s32 lbl_1_data_C8;

extern HUPROCESS *fn_1_A0(void);

extern const Point3d lbl_1_rodata_20;

extern const Point3d lbl_1_rodata_2C;

extern const Point3d lbl_1_rodata_38;

extern s32 lbl_1_data_D0;

extern s32 lbl_1_data_C4;

extern const Point3d lbl_1_rodata_2E8;

extern void fn_1_D37C(OMOBJ *obj);

extern void fn_1_D750(void);

s32 fn_1_D6C8(void);

extern const HuVecF lbl_1_rodata_250;

extern const Point3d lbl_1_rodata_1BC;

extern const Point3d lbl_1_rodata_1C8;

extern const Point3d lbl_1_rodata_1D4;

extern const HuVecF lbl_1_rodata_260;

extern void fn_1_D7E0(void);

extern void fn_1_D6B4(void);

extern u32 lbl_1_data_D4[14];

extern const Point3d lbl_1_rodata_70;

extern const Point3d lbl_1_rodata_7C;

extern const Point3d lbl_1_rodata_88;

extern u32 lbl_1_data_10C[16];

extern int abs(int value);

extern int rand8(void);

extern const Point3d lbl_1_rodata_22C;

extern void fn_1_D700(void);

extern void fn_1_D6A0(void);

extern const Point3d lbl_1_rodata_288;

extern unsigned int lbl_1_data_30[];

extern u32 lbl_1_data_74[5][4];

/* Copies the color stored for the grid cell at the supplied world position. */
unsigned int lbl_1_data_30[17] = { DATANUM(0x930000, 23), DATANUM(0x930000, 1), DATANUM(0x930000, 2), DATANUM(0x930000, 3), DATANUM(0x930000, 4), DATANUM(0x930000, 5), DATANUM(0x930000, 56), DATANUM(0x930000, 36), DATANUM(0x930000, 7), DATANUM(0x930000, 0), DATANUM(0x930000, 15), DATANUM(0x930000, 16), DATANUM(0x930000, 17), DATANUM(0x930000, 18), DATANUM(0x930000, 22), DATANUM(0x930000, 22), 0 };
u32 lbl_1_data_74[5][4] = { { DATANUM(0x8a0000, 29), DATANUM(0x8a0000, 30), DATANUM(0x8a0000, 31), DATANUM(0x8a0000, 32) }, { DATANUM(0x8a0000, 34), DATANUM(0x8a0000, 35), DATANUM(0x8a0000, 36), DATANUM(0x8a0000, 37) }, { DATANUM(0x8a0000, 39), DATANUM(0x8a0000, 40), DATANUM(0x8a0000, 41), DATANUM(0x8a0000, 42) }, { DATANUM(0x8a0000, 44), DATANUM(0x8a0000, 45), DATANUM(0x8a0000, 46), DATANUM(0x8a0000, 47) }, { DATANUM(0x8a0000, 49), DATANUM(0x8a0000, 50), DATANUM(0x8a0000, 51), DATANUM(0x8a0000, 52) } };
s32 lbl_1_data_C4 = 1;
s32 lbl_1_data_C8 = 1;
s32 lbl_1_data_CC = -1;
s32 lbl_1_data_D0 = 1;
u32 lbl_1_data_D4[14] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
u32 lbl_1_data_10C[16] = { MESSNUM(MESS_MG_MESS_679, 8), MESSNUM(MESS_MG_MESS_679, 9), MESSNUM(MESS_MG_MESS_679, 10), MESSNUM(MESS_MG_MESS_679, 11), MESSNUM(MESS_MG_MESS_679, 12), MESSNUM(MESS_MG_MESS_679, 13), MESSNUM(MESS_MG_MESS_679, 14), MESSNUM(MESS_MG_MESS_679, 15), MESSNUM(MESS_MG_MESS_679, 16), MESSNUM(MESS_MG_MESS_679, 17), MESSNUM(MESS_MG_MESS_679, 18), MESSNUM(MESS_MG_MESS_679, 19), MESSNUM(MESS_MG_MESS_679, 20), MESSNUM(MESS_MG_MESS_679, 21), MESSNUM(MESS_MG_MESS_679, 22), MESSNUM(MESS_MG_MESS_679, 23) };

M679Controller lbl_1_bss_7E6C;
M679PlayerWork lbl_1_bss_7D2C[5];
M679MazeLevel lbl_1_bss_68B4[5];
M679Window lbl_1_bss_686C;
M679Queue lbl_1_bss_66B0;
M679Queue lbl_1_bss_64F4;
M679Queue lbl_1_bss_6338;
M679Queue lbl_1_bss_617C;
M679ScoreState lbl_1_bss_6160;
/* Model pairs of the wall pieces. */
M679ModelPair lbl_1_bss_6148[6];
/* Six wall models, followed by four bytes the game's code never reads. */
struct {
    M679WallModel models[6];
    u8 unknown_30[4]; /* No code names this field. */
} lbl_1_bss_6114;
M679ModelSet lbl_1_bss_6100;
s16 lbl_1_bss_60EC[10];
s32 lbl_1_bss_60E8;
s32 lbl_1_bss_60E4;
M679RegionPoint lbl_1_bss_E4[6][128];
s16 lbl_1_bss_E2;
s16 lbl_1_bss_E0;
s16 lbl_1_bss_DE;
char lbl_1_bss_9E[64];
char lbl_1_bss_5E[64];
s16 lbl_1_bss_1C[33];
f32 lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
M679HistoryEntry *lbl_1_bss_C;
M679HistoryEntry *lbl_1_bss_8;

void fn_1_1A0(HuVecF *location, M679Cell *color)
{
    s32 playerIndex = lbl_1_bss_7E6C.currentIndex;
    s32 column = (s32)(location->x / 300.0f);
    s32 row = (s32)(location->z / 300.0f);

    *color = lbl_1_bss_68B4[playerIndex]
        .cells[column + row * 16];
}

void fn_1_250(s32 column, s32 row, M679Cell *color)
{
    *color = lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex]
        .cells[column + row * 16];
}

/* Returns the selected value from the five-entry choice table. */

s16 fn_1_2A0(s16 index)
{
    s16 sizes[5] = {6, 8, 10, 12, 14};

    return sizes[index];
}

/* Pops the newest value from a queue; prints a stack warning
 * instead when the count is negative. */
s16 fn_1_2EC(M679Queue *queue) {
    s32 count;

    count = queue->count;
    if (count < 0) {
        OSReport("wall info Stack Over.\n");
        return;
    }
    queue->count -= 1;
    return queue->values[count];
}

void fn_1_1238(M679Queue *work, s16 modelId)
{
    work->values[work->count] = modelId;
    work->count++;
}

/* Pops the newest value from the wall queue lbl_1_bss_64F4; prints a stack warning
 * instead when the count is negative. */
s16 fn_1_374(void) {
    s16 value;
    s32 count;

    count = lbl_1_bss_64F4.count;
    if (count < 0) {
        OSReport("wall info Stack Over.\n");
    } else {
        lbl_1_bss_64F4.count -= 1;
        value = lbl_1_bss_64F4.values[count];
    }
    return value;
}

/* Pops the newest value from the wall queue lbl_1_bss_66B0; prints a stack warning
 * instead when the count is negative. */
s16 fn_1_3FC(void) {
    s16 value;
    s32 count;

    count = lbl_1_bss_66B0.count;
    if (count < 0) {
        OSReport("wall info Stack Over.\n");
    } else {
        lbl_1_bss_66B0.count -= 1;
        value = lbl_1_bss_66B0.values[count];
    }
    return value;
}

void fn_1_484(s16 value) {
    lbl_1_bss_64F4.values[lbl_1_bss_64F4.count] = value;
    lbl_1_bss_64F4.count += 1;
}

void fn_1_4C4(s16 value) {
    lbl_1_bss_66B0.values[lbl_1_bss_66B0.count] = value;
    lbl_1_bss_66B0.count += 1;
}

/* Pops the newest value from the wall queue lbl_1_bss_617C; prints a stack warning
 * instead when the count is negative. */
s16 fn_1_504(void) {
    s16 value;
    s32 count;

    count = lbl_1_bss_617C.count;
    if (count < 0) {
        OSReport("wall info Stack Over.\n");
    } else {
        lbl_1_bss_617C.count -= 1;
        value = lbl_1_bss_617C.values[count];
    }
    return value;
}

/* Pops the newest value from the wall queue lbl_1_bss_6338; prints a stack warning
 * instead when the count is negative. */
s16 fn_1_58C(void) {
    s16 value;
    s32 count;

    count = lbl_1_bss_6338.count;
    if (count < 0) {
        OSReport("wall info Stack Over.\n");
    } else {
        lbl_1_bss_6338.count -= 1;
        value = lbl_1_bss_6338.values[count];
    }
    return value;
}

void fn_1_614(s16 value) {
    lbl_1_bss_617C.values[lbl_1_bss_617C.count] = value;
    lbl_1_bss_617C.count += 1;
}

void fn_1_654(s16 value) {
    lbl_1_bss_6338.values[lbl_1_bss_6338.count] = value;
    lbl_1_bss_6338.count += 1;
}

void fn_1_694(M679Queue *transfer)
{
    s32 index;

    index = 0;
    for (index = 0; index < 100; index++) {
        transfer->values[index] = transfer->source[index];
        transfer->count++;
    }
}

/* Empties the cell-pair history and allocates its cleared 200-entry buffer, freeing an earlier
 * one. */
void fn_1_6E4(void) {
    lbl_1_bss_60E8 = 0;
    if (lbl_1_bss_8 != NULL) {
        HuMemDirectFree(lbl_1_bss_8);
        lbl_1_bss_8 = NULL;
    }
    lbl_1_bss_8 = HuMemDirectMallocNum(HEAP_HEAP, 0x320, 0x10000000U);
    memset(lbl_1_bss_8, 0, 0x320);
}

void fn_1_77C(void) {
    HuMemDirectFree(lbl_1_bss_8);
    lbl_1_bss_8 = NULL;
}

void fn_1_7B8(void) {
    lbl_1_bss_60E8 = -1;
    memset(lbl_1_bss_8, 0, 0x320);
}

void fn_1_7FC(M679HistoryEntry *entry)
{
    if (lbl_1_bss_60E8 < 200) {
        lbl_1_bss_60E8++;
        lbl_1_bss_8[lbl_1_bss_60E8] = *entry;
    }
}

void fn_1_860(M679HistoryEntry *entry)
{
    if (lbl_1_bss_60E8 >= 0) {
        *entry = lbl_1_bss_8[lbl_1_bss_60E8];
        lbl_1_bss_60E8--;
    }
}

/* Empties the 512-entry history and allocates its cleared buffer, freeing an earlier one. */
void fn_1_8C4(void) {
    lbl_1_bss_60E4 = 0;
    if (lbl_1_bss_C != NULL) {
        HuMemDirectFree(lbl_1_bss_C);
        lbl_1_bss_C = NULL;
    }
    lbl_1_bss_C = HuMemDirectMallocNum(HEAP_HEAP, 0x800, 0x10000000U);
    memset(lbl_1_bss_C, 0, 0x800);
}

void fn_1_95C(void) {
    HuMemDirectFree(lbl_1_bss_C);
    lbl_1_bss_C = NULL;
}

void fn_1_998(void) {
    lbl_1_bss_60E4 = -1;
    memset(lbl_1_bss_C, 0, 0x800);
}

/* Adds a pair to the history unless it already appears in the active entries. */
void fn_1_9DC(M679HistoryEntry *entry)
{
    s32 index;

    if (lbl_1_bss_60E4 < 0x200) {
        if (lbl_1_bss_60E4 != -1) {
            index = 0;
            for (index = 0; index < lbl_1_bss_60E4; index++) {
                if ((entry->first == lbl_1_bss_C[index].first) &&
                    (entry->second == lbl_1_bss_C[index].second)) {
                    return;
                }
            }
        }

        lbl_1_bss_60E4++;
        lbl_1_bss_C[lbl_1_bss_60E4] = *entry;
    }
}

void fn_1_AD4(M679HistoryEntry *entry)
{
    if (lbl_1_bss_60E4 >= 0) {
        *entry = lbl_1_bss_C[lbl_1_bss_60E4];
        lbl_1_bss_60E4--;
    }
}

void fn_1_B38(void) {
    OMOBJ *obj;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_B78);
}

/* Loads the shadow scene models and initializes their shared lighting. */
const Point3d lbl_1_rodata_20 = { 0.0f, 1000.0f, 0.0f };

const Point3d lbl_1_rodata_2C = { 0.0f, 0.0f, -1.0f };

const Point3d lbl_1_rodata_38 = { 0.0f, 0.0f, 0.0f };

void fn_1_B78(OMOBJ *obj)
{
    Point3d lightPosition;
    Point3d lookAtPosition;
    Point3d upDirection;

    lbl_1_bss_686C.primaryModel = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 53), 0x10000000U, 2));
    lbl_1_bss_686C.primaryMotion = fn_80030458(lbl_1_bss_686C.primaryModel,
        fn_80008DD0(DATANUM(0x8a0000, 54), 0x10000000U, 2));
    lbl_1_bss_686C.alternateMotion = fn_80030458(lbl_1_bss_686C.primaryModel,
        fn_80008DD0(DATANUM(0x8a0000, 55), 0x10000000U, 2));
    lbl_1_bss_686C.shadowModel = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 56), 0x10000000U, 2));
    lbl_1_bss_686C.mapModel = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 58), 0x10000000U, 2));
    lbl_1_bss_686C.shadowMotion = fn_80030458(lbl_1_bss_686C.shadowModel,
        fn_80008DD0(DATANUM(0x8a0000, 57), 0x10000000U, 2));
    fn_800269E8(lbl_1_bss_686C.primaryModel, 0x40000001U);
    fn_800269E8(lbl_1_bss_686C.shadowModel, 0x40000001U);
    fn_800269E8(lbl_1_bss_686C.mapModel, 0x40000001U);
    fn_8002C95C(lbl_1_bss_686C.primaryModel, lbl_1_bss_686C.primaryMotion);
    fn_8002C95C(lbl_1_bss_686C.shadowModel, lbl_1_bss_686C.shadowMotion);
    lbl_1_bss_686C.window = -1;
    lbl_1_bss_686C.soundIndex = 0;
    lightPosition = lbl_1_rodata_20;
    lookAtPosition = lbl_1_rodata_2C;
    upDirection = lbl_1_rodata_38;
    Hu3DShadowCreate(30.0f, 20.0f, 6000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&lightPosition, &lookAtPosition, &upDirection);
    fn_80026E44(lbl_1_bss_686C.primaryModel);
    fn_80026FFC(lbl_1_bss_686C.shadowModel);
    obj->objFunc = fn_1_E1C;
}

/* Closes the results window after its sounds and wipe have finished. */
void fn_1_E1C(OMOBJ *obj)
{
    if (lbl_1_bss_686C.window == -1) {
        lbl_1_bss_686C.window = HuWinExCreateFrame(-10000.0f,
            340.0f, 0x1E0, 0x5A, -1, 1);
        HuWinExOpen(lbl_1_bss_686C.window);
        HuWinDisablePlayerSet(lbl_1_bss_686C.window,
            (u8)(~(1 << lbl_1_bss_7E6C.padIndex) & 0xF));
        OSReport("pad no : %d, %x\n", lbl_1_bss_7E6C.padIndex,
            ~(1 << lbl_1_bss_7E6C.padIndex) & 0xF);
        HuWinInsertMesSet(lbl_1_bss_686C.window,
            lbl_1_data_D4[lbl_1_bss_7E6C.menuIndex], 0);
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 0));
        HuWinMesSpeedSet(lbl_1_bss_686C.window, 1);
        fn_8002CCC0(lbl_1_bss_686C.primaryModel,
            lbl_1_bss_686C.alternateMotion, 0.0f,
            20.0f, 0x40000001U);
        lbl_1_data_CC = HuAudSStreamPlay(MSM_STREAM_MAZE_JAM);
        lbl_1_bss_686C.soundHandle = HuAudFXPlay(MSM_SE_MG679_MESSAGE_START);
        lbl_1_bss_686C.windowState =
            HuWinStatGet(lbl_1_bss_686C.window);
        lbl_1_bss_686C.transitionState = 0;
    }
    if (lbl_1_bss_686C.windowState !=
        HuWinStatGet(lbl_1_bss_686C.window)) {
        switch (lbl_1_bss_686C.windowState) {
        case 2: {
            s32 soundIds[4] = { MSM_SE_MG679_MESSAGE_PAGE_1, MSM_SE_MG679_MESSAGE_PAGE_2,
                                MSM_SE_MG679_MESSAGE_PAGE_3, MSM_SE_MG679_MESSAGE_PAGE_4 };
            if (lbl_1_bss_686C.soundIndex < 3) {
                lbl_1_bss_686C.soundHandle =
                    HuAudFXPlay(soundIds[lbl_1_bss_686C.soundIndex]);
                lbl_1_bss_686C.soundIndex++;
            }
            OSReport("%d\n", lbl_1_bss_686C.soundIndex);
            break;
        }
        default:
            break;
        }
        lbl_1_bss_686C.windowState =
            HuWinStatGet(lbl_1_bss_686C.window);
    }
    if ((lbl_1_bss_686C.transitionState == 0) &&
        (HuWinStatGet(lbl_1_bss_686C.window) == 0)) {
        WipeCreate(2, 0, 0x3C);
        lbl_1_bss_686C.transitionState = 1;
        HuWinDispOff(lbl_1_bss_686C.window);
        HuAudSStreamFadeOut(lbl_1_data_CC, 0x3E8);
    }
    if ((lbl_1_bss_686C.transitionState == 1) && (WipeCheck() == 0)) {
        fn_800269E8(lbl_1_bss_686C.primaryModel, 1U);
        fn_800269E8(lbl_1_bss_686C.shadowModel, 1U);
        fn_800269E8(lbl_1_bss_686C.mapModel, 1U);
        WipeCreate(1, 5, 0x3C);
        lbl_1_data_D0 = 0;
        lbl_1_data_C8 = 0;
        GameMesPauseEnable(1);
        obj->objFunc = fn_1_11FC;
    }
}

void fn_1_11FC(OMOBJ *object) {
    if (lbl_1_data_C8 == 1) {
        object->objFunc = fn_1_1220;
    }
}

const Point3d lbl_1_rodata_70 = { 0.0f, 1000.0f, 0.0f };

const Point3d lbl_1_rodata_7C = { 0.0f, 0.0f, -1.0f };

const Point3d lbl_1_rodata_88 = { 0.0f, 0.0f, 0.0f };

void fn_1_1220(OMOBJ *object)
{
    Point3d shadowCameraPosition;
    Point3d shadowCameraUp;
    Point3d shadowTarget;

    lbl_1_bss_686C.phase = 0;
    lbl_1_data_D0 = 1;
    GameMesPauseEnable(0);
    fn_80026760(lbl_1_bss_686C.primaryModel,
        0.0f, 0.0f, 0.0f);
    fn_80026A44(lbl_1_bss_686C.primaryModel, 1U);
    fn_80026A44(lbl_1_bss_686C.shadowModel, 1U);
    fn_80026A44(lbl_1_bss_686C.mapModel, 1U);
    HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 1));
    HuWinInsertMesSet(lbl_1_bss_686C.window,
        lbl_1_data_D4[lbl_1_bss_7E6C.menuIndex], 0);
    shadowCameraPosition = lbl_1_rodata_70;
    shadowCameraUp = lbl_1_rodata_7C;
    shadowTarget = lbl_1_rodata_88;
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowCameraPosition, &shadowCameraUp, &shadowTarget);
    fn_80026E44(lbl_1_bss_686C.primaryModel);
    fn_80026FFC(lbl_1_bss_686C.shadowModel);
    WipeCreate(1, 5, 60);
    lbl_1_data_CC = HuAudSStreamPlay(MSM_STREAM_MAZE_JAM);
    lbl_1_bss_686C.soundStarted = 0;
    object->objFunc = fn_1_17E0;
}

/* Chooses the result-window message and its per-mode sound sequence. */
void fn_1_1434(void)
{
    s32 index;

    for (index = 0; index < 10; index++) {
        lbl_1_bss_686C.effectIds[index] = -1;
    }
    index = 0;
    switch (lbl_1_bss_7E6C.resultIndex) {
    case 0:
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 2));
        lbl_1_bss_686C.effectIds[index] = 0x909;
        index++;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 3));
        lbl_1_bss_686C.effectIds[index] = 0x90B;
        index++;
        sprintf(lbl_1_bss_9E, "%d",
                lbl_1_bss_7E6C.currentIndex + 1);
        HuWinInsertMesSet(lbl_1_bss_686C.window, (u32)lbl_1_bss_9E, 0);
        break;
    case 5:
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 4));
        lbl_1_bss_686C.effectIds[index] = 0x908;
        index++;
        break;
    case 6:
    case 7:
    case 8:
    case 9: {
        u32 choices[5] = { 0, MESSNUM(MESS_MG_MESS_679, 24), MESSNUM(MESS_MG_MESS_679, 25), MESSNUM(MESS_MG_MESS_679, 26), MESSNUM(MESS_MG_MESS_679, 27) };
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 5));
        HuWinInsertMesSet(lbl_1_bss_686C.window,
                          choices[lbl_1_bss_7E6C.progress], 0);
        lbl_1_bss_686C.effectIds[index] = 0x908;
        index++;
        lbl_1_bss_686C.effectIds[index] = 0x90C;
        index++;
        break;
    }
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15: {
        s32 remaining = 0x4650 - lbl_1_bss_6160.mode;
        HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 6));
        sprintf(lbl_1_bss_9E, "%02d", remaining / 3600);
        HuWinInsertMesSet(lbl_1_bss_686C.window, (u32)lbl_1_bss_9E, 0);
        sprintf(lbl_1_bss_5E, "%02d",
                (remaining - (remaining / 3600) * 3600) / 60);
        HuWinInsertMesSet(lbl_1_bss_686C.window, (u32)lbl_1_bss_5E, 1);
        lbl_1_bss_686C.effectIds[index] = 0x908;
        index++;
        lbl_1_bss_686C.effectIds[index] = 0x90D;
        index++;
        break;
    }
    }

    lbl_1_bss_686C.effectIds[index] = 0x903;
    index++;
    lbl_1_bss_686C.effectIds[index] = 0x905;
    index++;
    lbl_1_bss_686C.effectIds[index] = 0x906;
    index++;
}

void fn_1_17E0(OMOBJ *object)
{
    if (WipeCheckIn() == 1) {
        return;
    }
    if (lbl_1_bss_686C.phase == 0) {
        HuWinDispOn(lbl_1_bss_686C.window);
        if (lbl_1_bss_686C.soundStarted == 0) {
            lbl_1_bss_686C.soundIndex = 0;
            lbl_1_bss_686C.windowState = HuWinStatGet(lbl_1_bss_686C.window);
            lbl_1_bss_686C.soundHandle = HuAudFXPlay(MSM_SE_MG679_INSTRUCTION_START);
            lbl_1_bss_686C.soundStarted = 1;
        }
    }
    if (lbl_1_bss_686C.windowState != HuWinStatGet(lbl_1_bss_686C.window)) {
        switch (lbl_1_bss_686C.windowState) {
        case 2:
            if (lbl_1_bss_686C.effectIds[lbl_1_bss_686C.soundIndex] != -1) {
                lbl_1_bss_686C.soundHandle = HuAudFXPlay(
                    lbl_1_bss_686C.effectIds[lbl_1_bss_686C.soundIndex]);
            }
            lbl_1_bss_686C.soundIndex++;
            OSReport("%d\n", lbl_1_bss_686C.soundIndex);
            break;
        }
        lbl_1_bss_686C.windowState = HuWinStatGet(lbl_1_bss_686C.window);
    }
    if (HuWinStatGet(lbl_1_bss_686C.window) == 0) {
        switch (lbl_1_bss_686C.phase) {
        case 0:
            fn_1_1434();
            lbl_1_bss_686C.phase = 1;
            break;
        case 1:
            HuWinMesSet(lbl_1_bss_686C.window, MESSNUM(MESS_MG_MESS_679, 7));
            HuWinInsertMesSet(lbl_1_bss_686C.window, lbl_1_data_D4[lbl_1_bss_7E6C.menuIndex], 0);
            sprintf(lbl_1_bss_9E, "%d", lbl_1_bss_7E6C.resultIndex + 1);
            HuWinInsertMesSet(lbl_1_bss_686C.window, (u32)lbl_1_bss_9E, 1);
            HuWinInsertMesSet(lbl_1_bss_686C.window, lbl_1_data_10C[lbl_1_bss_7E6C.resultIndex], 2);
            lbl_1_bss_686C.phase = 2;
            break;
        case 2:
            HuAudSStreamFadeOut(lbl_1_data_CC, 1000);
            HuWinDispOff(lbl_1_bss_686C.window);
            fn_80071AC4(9);
            break;
        }
    }
}

/* Always returns 0. */
s32 fn_1_1AE0(void) { return 0; }

/* Checks grid bounds and occupancy, consuming a random value for an empty cell. */
s32 fn_1_1AE8(u8 *grid, s16 column, s16 row, s16 width, s16 height, s32 option)
{
    s32 rowStride = 0x20;

    if ((column < 0) || (row < 0)) {
        return 0;
    }
    if ((column >= width) || (row >= height)) {
        return 0;
    }
    if (grid[row * rowStride + column] == 0) {
        if ((s32)(rand8() % 100) < 2) {
            option == 1; /* The original's typo: == where = was meant. */
        }
        return 1;
    }
    return 2;
}

/* Carves maze passages and saves cells with no unvisited neighbor. */
s32 fn_1_1BF0(u8 *grid, s32 seed, s16 startColumn, s16 startRow,
    s32 level)
{
    s32 rowStride = 0x20;
    u32 available = 0;
    M679HistoryEntry point;
    M679HistoryEntry candidate;
    s32 direction;
    s32 result;

    point.first = startColumn;
    point.second = startRow;
    fn_1_7B8();
    fn_1_7FC(&point);
    fn_1_998();
    while (lbl_1_bss_60E8 >= 0) {
        (grid + startRow * rowStride)[startColumn] = 0;
        if ((rand8() % 100) < 0x32) {
            if (rand8() % 2) {
                seed = (seed * 2) & 0xF;
                if (seed == 0) {
                    seed = 1;
                }
            } else {
                seed = (seed >> 1) & 0xF;
                if (seed == 0) {
                    seed = 8;
                }
            }
        }
        available = 0;
        result = fn_1_1AE8(grid, startColumn, startRow - 2,
                fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0);
        if (result == 2) {
            available |= 1;
        }
        result = fn_1_1AE8(grid, startColumn + 2, startRow,
                fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0);
        if (result == 2) {
            available |= 2;
        }
        result = fn_1_1AE8(grid, startColumn, startRow + 2,
                fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0);
        if (result == 2) {
            available |= 4;
        }
        result = fn_1_1AE8(grid, startColumn - 2, startRow,
                fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0);
        if (result == 2) {
            available |= 8;
        }
        if (available == 0) {
            fn_1_860(&point);
            startColumn = point.first;
            startRow = point.second;
        } else {
            while (available != 0) {
                s32 directionBits[4] = { 1, 2, 4, 8 };

                direction = rand8() % 4;
                if ((available & directionBits[direction]) == 0) {
                    direction = (direction + 1) % 4;
                    continue;
                }
                switch (directionBits[direction]) {
                case 1:
                    (grid + (startRow - 1) * rowStride)[startColumn] = 0;
                    (grid + (startRow - 2) * rowStride)[startColumn] = 0;
                    startRow -= 2;
                    point.first = startColumn;
                    point.second = startRow;
                    fn_1_7FC(&point);
                    break;
                case 2:
                    (grid + startRow * rowStride + startColumn)[1] = 0;
                    (grid + startRow * rowStride + startColumn)[2] = 0;
                    startColumn += 2;
                    point.first = startColumn;
                    point.second = startRow;
                    fn_1_7FC(&point);
                    break;
                case 4:
                    (grid + (startRow + 1) * rowStride)[startColumn] = 0;
                    (grid + (startRow + 2) * rowStride)[startColumn] = 0;
                    startRow += 2;
                    point.first = startColumn;
                    point.second = startRow;
                    fn_1_7FC(&point);
                    break;
                case 8:
                    (grid + startRow * rowStride + startColumn)[-1] = 0;
                    (grid + startRow * rowStride + startColumn)[-2] = 0;
                    startColumn -= 2;
                    point.first = startColumn;
                    point.second = startRow;
                    fn_1_7FC(&point);
                    break;
                }
                break;
            }
            if (fn_1_1AE8(grid, startColumn, startRow - 2,
                    fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0) != 2) {
                if (fn_1_1AE8(grid, startColumn + 2, startRow,
                        fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0) != 2) {
                    if (fn_1_1AE8(grid, startColumn, startRow + 2,
                            fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0) != 2) {
                        if (fn_1_1AE8(grid, startColumn - 2, startRow,
                                fn_1_2A0(level) * 2, fn_1_2A0(level) * 2, 0) != 2) {
                            candidate.first = startColumn;
                            candidate.second = startRow;
                            fn_1_9DC(&candidate);
                        }
                    }
                }
            }
        }
    }
}

void fn_1_2D7C(void)
{
    s32 halfLength;
    s32 columnCrosses;
    s32 rowCrosses;
    s32 pathColumn;
    s32 mazeSize;
    s32 unknownValue;
    s32 retryCount;
    s16 startColumn;
    s32 goalAttempts;
    s32 candidateCount;
    s32 pathIndex;
    s16 startRow;
    s32 level;
    s32 pathRow;
    s32 goalIndex;
    s32 cellColumn;
    s32 cellRow;
    u8 * maze;
    M679Cell edgeCell;
    s32 wallMask;
    s32 gridSideLength;
    s32 goalQuadrant;
    M679MazeCoordinate * candidates;
    level = 0;
    mazeSize = 0x20;
    maze = HuMemDirectMallocNum(HEAP_HEAP, mazeSize * mazeSize, 0x10000000U);
    fn_1_6E4();
    fn_1_8C4();
    for (level = 0; 5 > level; level++)
    {
        unknownValue = 0;
        retryCount = 0;
        {
        u32 seeds[4] = { 1, 2, 4, 8 };
        retry_maze : memset(maze, 1, mazeSize * mazeSize);
        lbl_1_bss_68B4[level].sideLength = fn_1_2A0(level);
        startColumn = rand8() % lbl_1_bss_68B4[level].sideLength;
        startRow = rand8() % lbl_1_bss_68B4[level].sideLength;
        lbl_1_bss_68B4[level].startColumn = startColumn;
        lbl_1_bss_68B4[level].startRow = startRow;
        fn_1_1BF0(maze, seeds[rand8() % 4], startColumn * 2, startRow * 2, level);
        halfLength = lbl_1_bss_68B4[level].sideLength / 2;
        goalQuadrant = 0;
        goalAttempts = 0;
        pathIndex = 0;
        columnCrosses = 0;
        rowCrosses = 0;
        candidates = HuMemDirectMallocNum(HEAP_HEAP, 0x800, 0x10000000U);
        candidateCount = 0;
        memset(candidates, 0, 0x800);
        for (pathIndex = 0; pathIndex < lbl_1_bss_60E4; pathIndex++)
        {
            pathColumn = lbl_1_bss_C[pathIndex].first >> 1;
            pathRow = lbl_1_bss_C[pathIndex].second >> 1;
            columnCrosses = 0;
            rowCrosses = 0;
            if (pathColumn < halfLength && halfLength <= startColumn)
            {
                columnCrosses = 1;
            }
            if (startColumn < halfLength && halfLength <= pathColumn)
            {
                columnCrosses = 1;
                goalQuadrant += 1;
            }
            if (pathRow < halfLength && halfLength <= startRow)
            {
                rowCrosses = 1;
            }
            if (startRow < halfLength && halfLength <= pathRow)
            {
                rowCrosses = 1;
                goalQuadrant += 2;
            }
            if (columnCrosses == 1 && rowCrosses == 1)
            {
                OSReport("area_half %d : candidate ( %d, %d )\n", halfLength, pathColumn, pathRow);
                candidates[candidateCount].column = pathColumn;
                candidates[candidateCount].row = pathRow;
                candidateCount = 1 + candidateCount;
            }
        }
        OSReport("candidate count: %d.\n", candidateCount);
        if (candidateCount <= 1)
        {
            retryCount++;
            if (retryCount < 0x14)
            {
                OSReport("dig retry.\n");
                HuMemDirectFree(candidates);
                goto retry_maze;
            }
            memcpy(candidates, lbl_1_bss_C, 0x800);
            candidateCount = lbl_1_bss_60E4;
            for (pathIndex = 0; pathIndex < lbl_1_bss_60E4; pathIndex++)
            {
                candidates[pathIndex].column = candidates[pathIndex].column >> 1;
                candidates[pathIndex].row = candidates[pathIndex].row >> 1;
            }
        }
        goalIndex = 0;
        retry_goal : goalQuadrant = 0;
        goalIndex = frandmod(candidateCount);
        lbl_1_bss_68B4[level].goalColumn = candidates[goalIndex].column;
        lbl_1_bss_68B4[level].goalRow = candidates[goalIndex].row;
        if (halfLength <= lbl_1_bss_68B4[level].goalColumn)
        {
            goalQuadrant += 1;
        }
        if (halfLength <= lbl_1_bss_68B4[level].goalRow)
        {
            goalQuadrant += 2;
        }
        if (((abs(lbl_1_bss_68B4[level].goalColumn - lbl_1_bss_68B4[level].startColumn) < level + 2) || (abs(lbl_1_bss_68B4[level].goalRow - lbl_1_bss_68B4[level].startRow) < level + 2)) && (goalAttempts < 0x32))
        {
            goalAttempts++;
            goto retry_goal;
        }
        HuMemDirectFree(candidates);
        switch (goalQuadrant)
        {
        case 0 :
        case 3 :
            if (rand8() % 2)
            {
                lbl_1_bss_68B4[level].exitColumn = halfLength + rand8() % halfLength;
                lbl_1_bss_68B4[level].exitRow = rand8() % halfLength;
            }
            else
            {
                lbl_1_bss_68B4[level].exitColumn = rand8() % halfLength;
                lbl_1_bss_68B4[level].exitRow = halfLength + rand8() % halfLength;
            }
            break;
        case 1 :
        case 2 :
            if (rand8() % 2)
            {
                lbl_1_bss_68B4[level].exitColumn = rand8() % halfLength;
                lbl_1_bss_68B4[level].exitRow = rand8() % halfLength;
            }
            else
            {
                lbl_1_bss_68B4[level].exitColumn = halfLength + rand8() % halfLength;
                lbl_1_bss_68B4[level].exitRow = halfLength + rand8() % halfLength;
            }
            break;
        }
        lbl_1_bss_68B4[level].currentTile.x = 0x3E8;
        lbl_1_bss_68B4[level].currentTile.z = 0x3E8;
        OSReport("level : %d ( %d x %d )\n", level, lbl_1_bss_68B4[level].sideLength, lbl_1_bss_68B4[level].sideLength);
        OSReport("start : ( %d , %d )\n", lbl_1_bss_68B4[level].startColumn, lbl_1_bss_68B4[level].startRow);
        OSReport("goal  : ( %d , %d )\n", lbl_1_bss_68B4[level].goalColumn, lbl_1_bss_68B4[level].goalRow);
        gridSideLength = fn_1_2A0(level);
        cellColumn = 0;
        cellRow = 0;
        for (startRow = 0; startRow < (gridSideLength * 2); ++startRow)
        {
            for (startColumn = 0; startColumn < (gridSideLength * 2); ++startColumn)
            {
                if ((maze[startRow * mazeSize + startColumn] == 1) && (rand8() % 100 < 2))
                {
                    maze[startRow * mazeSize + startColumn] = 0;
                    OSReport("broken wall : ( %d, %d )\n", startColumn, startRow);
                }
            }
        }
        for (startRow = 0; startRow < (gridSideLength * 2); startRow += 2)
        {
            cellColumn = 0;
            for (startColumn = 0; startColumn < (gridSideLength * 2); startColumn += 2)
            {
                wallMask = 0xF;
                {
                s32 wallCodes[10] = { 1, 1, 1, 1, 1, 1, 3, 3, 3, 3 };
                edgeCell.negativeZ = wallCodes[rand8() % 10];
                edgeCell.negativeX = wallCodes[rand8() % 10];
                edgeCell.positiveZ = wallCodes[rand8() % 10];
                edgeCell.positiveX = wallCodes[rand8() % 10];
                }
                if (startRow - 1 < 0)
                {
                    edgeCell.negativeZ = 2;
                    wallMask &= ~1;
                }
                else
                {
                    if (maze[(startRow - 1) * mazeSize + startColumn] == 0)
                    {
                        edgeCell.negativeZ = 0;
                        wallMask &= ~1;
                    }
                }
                if (startColumn - 1 < 0)
                {
                    edgeCell.negativeX = 2;
                    wallMask &= ~8;
                }
                else
                {
                    if (! (maze[startRow * mazeSize + startColumn - 1]))
                    {
                        edgeCell.negativeX = 0;
                        wallMask = wallMask & (~8);
                    }
                }
                if ((2 * gridSideLength) <= startColumn + 2)
                {
                    edgeCell.positiveX = 2;
                    wallMask &= ~2;
                }
                else
                {
                    if (! (maze[startColumn + 1 + startRow * mazeSize]))
                    {
                        edgeCell.positiveX = 0;
                        wallMask &= ~2;
                    }
                }
                if ((gridSideLength * 2) <= startRow + 2)
                {
                    edgeCell.positiveZ = 2;
                    wallMask &= ~4;
                }
                else
                {
                    if (0 == maze[(startRow + 1) * mazeSize + startColumn])
                    {
                        edgeCell.positiveZ = 0;
                        wallMask &= ~4;
                    }
                }
                wallMask &= 0xF;
                if (wallMask == 0)
                {
                    if (edgeCell.negativeX != 2)
                    {
                        edgeCell.negativeX = 0;
                    }
                    else
                    {
                        if (edgeCell.negativeZ != 2)
                        {
                            edgeCell.negativeZ = 0;
                        }
                        else
                        {
                            if (edgeCell.positiveX != 2)
                            {
                                edgeCell.positiveX = 0;
                            }
                            else
                            {
                                edgeCell.positiveZ = 0;
                            }
                        }
                    }
                }
                lbl_1_bss_68B4[level].cells[cellColumn + cellRow * 16] = edgeCell;
                cellColumn++;
            }
            cellRow++;
        }
        }
    }
    fn_1_77C();
    fn_1_95C();
    HuMemDirectFree(maze);
}

void fn_1_3B38(void) {
    OMOBJ *obj;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_3FA4);
}

/* Builds and links the model queues used by the result effect. */
void fn_1_3B78(void)
{
    s16 *linkedModels;
    s16 *sourceModels;
    s32 modelIndex;
    s32 linkedIndex;
    s32 sourceIndex;

    memset(&lbl_1_bss_64F4.values[0], -1, 0xC8);
    memset(lbl_1_bss_64F4.source, -1, 0xC8);
    memset(&lbl_1_bss_66B0.values[0], -1, 0xC8);
    memset(lbl_1_bss_66B0.source, -1, 0xC8);
    for (modelIndex = 0; modelIndex < 100; modelIndex++) {
        linkedModels = lbl_1_bss_66B0.source + modelIndex;
        sourceModels = lbl_1_bss_64F4.source + modelIndex;
        *linkedModels = fn_80025D4C(lbl_1_bss_6148[0].first);
        *sourceModels = fn_80025D4C(lbl_1_bss_6148[0].second);
        fn_80026B8C(*linkedModels, 4);
        fn_80026B8C(*sourceModels, 4);
        fn_800269E8(*linkedModels, 0x40000001U);
        fn_800269E8(*sourceModels, 0x40000001U);
        fn_80026A44(*linkedModels, 0x40000002U);
        fn_80026A44(*sourceModels, 0x40000002U);
    }
    linkedIndex = 0;
    for (linkedIndex = 0; linkedIndex < 100; linkedIndex++) {
        lbl_1_bss_66B0.values[linkedIndex] =
            (lbl_1_bss_66B0.source)[linkedIndex];
        lbl_1_bss_66B0.count++;
    }
    sourceIndex = 0;
    for (sourceIndex = 0; sourceIndex < 100; sourceIndex++) {
        lbl_1_bss_64F4.values[sourceIndex] =
            (lbl_1_bss_64F4.source)[sourceIndex];
        lbl_1_bss_64F4.count++;
    }
    memset(&lbl_1_bss_617C.values[0], -1, 0xC8);
    memset(lbl_1_bss_617C.source, -1, 0xC8);
    memset(&lbl_1_bss_6338.values[0], -1, 0xC8);
    memset(lbl_1_bss_6338.source, -1, 0xC8);
}

/* Fades the result overlay while pausing and hides it after unpausing. */
void fn_1_3DD8(void)
{
    if (lbl_802C0234 + fn_8003B748()) {
        if (lbl_1_data_D0 == 0) {
            espDispOn(lbl_1_bss_1C[0]);
            espTPLvlSet(lbl_1_bss_1C[0], lbl_1_bss_18);
            if (lbl_1_bss_18 < 1.0f) {
                lbl_1_bss_18 += 0.05f;
                if (lbl_1_bss_18 > 1.0f) {
                    lbl_1_bss_18 = 1.0f;
                }
            }
        }
    } else {
        espTPLvlSet(lbl_1_bss_1C[0], lbl_1_bss_18);
        if (lbl_1_bss_18 > 0.0f) {
            lbl_1_bss_18 -= 0.05f;
            if (lbl_1_bss_18 < 0.0f) {
                lbl_1_bss_18 = 0.0f;
            }
        }
        if (0.0f == lbl_1_bss_18) {
            espDispOff(lbl_1_bss_1C[0]);
        }
    }
}

/* Creates the board models, links their queues, and starts the board actors. */
static inline void m679InitializeWindowModels(void)
{
    s16 *linkedModels;
    s16 *sourceModels;
    s32 modelIndex;
    s32 sourceIndex;
    s32 linkedIndex;

    memset(&lbl_1_bss_64F4.values[0], -1, 0xC8);
    memset(lbl_1_bss_64F4.source, -1, 0xC8);
    memset(&lbl_1_bss_66B0.values[0], -1, 0xC8);
    memset(lbl_1_bss_66B0.source, -1, 0xC8);
    for (modelIndex = 0; modelIndex < 100; modelIndex++) {
        linkedModels = lbl_1_bss_66B0.source + modelIndex;
        sourceModels = lbl_1_bss_64F4.source + modelIndex;
        *linkedModels = fn_80025D4C(lbl_1_bss_6148[0].first);
        *sourceModels = fn_80025D4C(lbl_1_bss_6148[0].second);
        fn_80026B8C(*linkedModels, 4);
        fn_80026B8C(*sourceModels, 4);
        fn_800269E8(*linkedModels, 0x40000001U);
        fn_800269E8(*sourceModels, 0x40000001U);
        fn_80026A44(*linkedModels, 0x40000002U);
        fn_80026A44(*sourceModels, 0x40000002U);
    }
    linkedIndex = 0;
    for (linkedIndex = 0; linkedIndex < 100; linkedIndex++) {
            lbl_1_bss_66B0.values[linkedIndex] =
                (lbl_1_bss_66B0.source)[linkedIndex];
        lbl_1_bss_66B0.count++;
    }
    sourceIndex = 0;
    for (sourceIndex = 0; sourceIndex < 100; sourceIndex++) {
            lbl_1_bss_64F4.values[sourceIndex] =
                (lbl_1_bss_64F4.source)[sourceIndex];
        lbl_1_bss_64F4.count++;
    }
    memset(&lbl_1_bss_617C.values[0], -1, 0xC8);
    memset(lbl_1_bss_617C.source, -1, 0xC8);
    memset(&lbl_1_bss_6338.values[0], -1, 0xC8);
    memset(lbl_1_bss_6338.source, -1, 0xC8);
}

void fn_1_3FA4(OMOBJ *object)
{
    s32 modelCount = 0;
    s16 collisionModels[6];
    Point3d shadowTop = {0.0f, 0.0f, 1.0f};
    Point3d shadowBottom = {0.0f, 0.0f, 0.0f};
    Point3d shadowCenter = {0.0f, 1000.0f, 0.0f};
    s32 index;
    Hu3DShadowCreate(45.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.6f);
    fn_8002AAB4(1U, 0U, 0U);
    Hu3DShadowPosSet(&shadowCenter, &shadowTop, &shadowBottom);

    for (index = 0; index < 5; index++) {
        u32 shadowModelData[5] = {
            DATANUM(0x8a0000, 0), DATANUM(0x8a0000, 1),
            DATANUM(0x8a0000, 2), DATANUM(0x8a0000, 3),
            DATANUM(0x8a0000, 4)
        };
        u32 baseModelData[5] = {
            DATANUM(0x8a0000, 5), DATANUM(0x8a0000, 6),
            DATANUM(0x8a0000, 7), DATANUM(0x8a0000, 8),
            DATANUM(0x8a0000, 9)
        };
        char *shadowMapData[5] = {
            "meiro-1Fsha", "meiro-2Fsha", "meiro-3Fsha",
            "meiro-4Fsha", "meiro-5Fsha"
        };

        lbl_1_bss_68B4[index].shadowModel = fn_80025A04(
            fn_80008DD0(shadowModelData[index],
                0x10000000, 2));
        lbl_1_bss_68B4[index].baseModel = fn_80025A04(
            fn_80008DD0(baseModelData[index],
                0x10000000, 2));
        {
            Point3d position = {0.0f, 0.0f, 0.0f};

            position.x += 300.0f * (f32)(fn_1_2A0(index) / 2);
            position.z += 300.0f * (f32)(fn_1_2A0(index) / 2);
            fn_80026790(lbl_1_bss_68B4[index].shadowModel, &position);
            fn_80026790(lbl_1_bss_68B4[index].baseModel, &position);
        }
        fn_80026B64(lbl_1_bss_68B4[index].shadowModel, 1U);
        fn_80027080(lbl_1_bss_68B4[index].shadowModel,
            shadowMapData[index]);
        collisionModels[modelCount] = lbl_1_bss_68B4[index].baseModel;
        modelCount++;
        fn_800269E8(lbl_1_bss_68B4[index].shadowModel, 1U);
        fn_800269E8(lbl_1_bss_68B4[index].baseModel, 1U);
        fn_80026CBC(lbl_1_bss_68B4[index].baseModel,
            0.8f);
    }

    lbl_1_bss_DE = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 23), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_DE, 1U);
    collisionModels[modelCount] = lbl_1_bss_DE;
    modelCount++;
    fn_800804E8(collisionModels, 6, 0xC);
    fn_80080D7C(0, 1U);
    fn_80080D7C(1, 2U);
    fn_80080D7C(2, 4U);
    fn_80080D7C(3, 8U);
    fn_80080D7C(4, 0x10U);
    fn_80080D7C(5, 0x1FU);
    for (index = 0; index < 0x18; index++) {
        fn_80080D7C(index + 6, 0x1FU);
    }

    for (index = 0; index < 6; index++) {
        u32 motionModelData[12] = {
            DATANUM(0x8a0000, 10), DATANUM(0x8a0000, 11),
            DATANUM(0x8a0000, 12), DATANUM(0x8a0000, 13),
            DATANUM(0x8a0000, 14), DATANUM(0x8a0000, 15),
            DATANUM(0x8a0000, 16), DATANUM(0x8a0000, 17),
            DATANUM(0x8a0000, 18), DATANUM(0x8a0000, 19),
            DATANUM(0x8a0000, 20), DATANUM(0x8a0000, 21)
        };
        lbl_1_bss_6148[index].first =
            fn_80025A04(fn_80008DD0(motionModelData[index * 2],
                0x10000000, 2));
        lbl_1_bss_6148[index].second =
            fn_80025A04(fn_80008DD0(motionModelData[index * 2 + 1],
                0x10000000, 2));
        fn_800269E8(lbl_1_bss_6148[index].first, 1U);
        fn_800269E8(lbl_1_bss_6148[index].second, 1U);
    }

    fn_1_61E8();
    fn_1_6568();
    lbl_1_bss_60EC[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 25), 0x10000000, 2));
    lbl_1_bss_60EC[6] = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 26), 0x10000000, 2));
    lbl_1_bss_60EC[7] = fn_80030458(lbl_1_bss_60EC[6],
        fn_80008DD0(DATANUM(0x8a0000, 27), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_60EC[6], lbl_1_bss_60EC[7]);
    fn_800269E8(lbl_1_bss_60EC[0], 1U);
    fn_80026A44(lbl_1_bss_60EC[6], 0x40000001U);
    fn_800269E8(lbl_1_bss_60EC[6], 0x40000002U);
    fn_800269E8(lbl_1_bss_60EC[6], 1U);
    fn_80026B8C(lbl_1_bss_60EC[6], 7);

    lbl_1_bss_E2 = fn_80025A04(
        fn_80008DD0(DATANUM(0x8a0000, 22), 0x10000000, 2));
    lbl_1_bss_E0 = fn_80030458(lbl_1_bss_E2,
        fn_80008DD0(DATANUM(0x8a0000, 24), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_E2, 2);
    fn_8002C95C(lbl_1_bss_E2, lbl_1_bss_E0);
    fn_800269E8(lbl_1_bss_E2, 0x40000002U);
    fn_800269E8(lbl_1_bss_E2, 1U);
    fn_1_2D7C();

    m679InitializeWindowModels();

    lbl_1_bss_1C[0] = espEntry(DATANUM(0x8a0000, 67), 0x50, 0);
    espPosSet(lbl_1_bss_1C[0], 320.0f, 240.0f);
    espScaleSet(lbl_1_bss_1C[0], 20.0f, 15.0f);
    espDispOff(lbl_1_bss_1C[0]);
    fn_80025918(0xF, (void (*)(s16))fn_1_3DD8);
    fn_1_6640();
    fn_1_95A4();
    fn_1_CDD0();
    fn_1_D0B0();
    fn_1_D144();
    fn_1_B38();
    object->objFunc = fn_1_4AA4;
}

void fn_1_4AA4(OMOBJ *object) {
    if (lbl_1_data_C8 == 0) {
        object->objFunc = fn_1_4AC8;
    }
}

void fn_1_4AC8(OMOBJ *object)
{
    M679TilePosition oldTile;
    Point3d actorPosition;
    s32 tileCount;
    s32 rowBase;
    s32 newTileX;
    s32 newTileZ;
    s32 row;
    s32 modelIndex;
    s32 columnBase;
    s32 firstColumn;
    s32 firstRow;
    s32 lastColumn;
    s32 lastRow;
    s16 modelIdA;
    s32 playerIndex3 = lbl_1_bss_7E6C.currentIndex;
    s16 modelIdD;
    s32 column;
    M679Cell tileState;
    oldTile = lbl_1_bss_68B4[playerIndex3].currentTile;
    tileCount = lbl_1_bss_68B4[playerIndex3].sideLength;
    if (lbl_1_bss_7E6C.player == NULL)
    {
        return;
    }
    if (lbl_1_data_C8 == 1)
    {
        s32 i;

        for (i = 0; i < 100; ++i)
        {
            fn_800269E8(lbl_1_bss_66B0.source[i], 1U);
            fn_800269E8(lbl_1_bss_64F4.source[i], 1U);
            fn_800269E8(lbl_1_bss_6338.source[i], 1U);
            fn_800269E8(lbl_1_bss_617C.source[i], 1U);
        }
        for (i = 0; i < 5; i++)
        {
            fn_800269E8(lbl_1_bss_68B4[i].shadowModel, 1U);
        }
        fn_1_6568();
        fn_800269E8(lbl_1_bss_60EC[0], 1U);
        object->objFunc = NULL;
        return;
    }
    fn_80086BC8(lbl_1_bss_7E6C.player->actor, & actorPosition);
    newTileX = (s32)(actorPosition.x / 300.0f);
    newTileZ = (s32)(actorPosition.z / 300.0f);
    if (oldTile.x == newTileX && oldTile.z == newTileZ)
    {
        return;
    }
    columnBase = 0;
    rowBase = 0;
    lbl_1_bss_68B4[playerIndex3].currentTile.x = (s16) newTileX;
    lbl_1_bss_68B4[playerIndex3].currentTile.z = (s16) newTileZ;
    for (modelIndex = 0; modelIndex < 100; modelIndex++)
    {
        fn_800269E8(lbl_1_bss_66B0.source[modelIndex], 1U);
        fn_800269E8(lbl_1_bss_64F4.source[modelIndex], 1U);
        fn_800269E8(lbl_1_bss_6338.source[modelIndex], 1U);
        fn_800269E8(lbl_1_bss_617C.source[modelIndex], 1U);
    }
    firstColumn = newTileX - 5;
    lastColumn = newTileX + 5;
    if (firstColumn < 0)
    {
        lastColumn = 10;
        firstColumn = 0;
    }
    if (lastColumn > tileCount)
    {
        firstColumn = tileCount - 10;
        lastColumn = 10;
    }
    columnBase = firstColumn;
    firstRow = newTileZ - 5;
    lastRow = newTileZ + 5;
    if (firstRow < 0)
    {
        lastRow = 10;
        firstRow = 0;
    }
    if (lastRow > tileCount)
    {
        firstRow = tileCount - 10;
        lastRow = tileCount;
    }
    rowBase = firstRow;
    for (row = 0; row < 10; row++)
    {
        for (column = 0; column < 10; column++)
        {
            modelIdA = - 1;
            modelIdD = - 1;
            tileState = lbl_1_bss_68B4[playerIndex3].cells[columnBase + column + (rowBase + row) * 16];
            switch (tileState.positiveZ)
            {
            case 1 :
                modelIdA = lbl_1_bss_64F4.source[column + row * 10];
                break;
            case 3 :
                modelIdA = lbl_1_bss_617C.source[column + row * 10];
                break;
            default :
                modelIdA = - 1;
                break;
            }
            switch (tileState.positiveX)
            {
            case 1 :
                modelIdD = lbl_1_bss_66B0.source[column + row * 10];
                break;
            case 3 :
                modelIdD = lbl_1_bss_6338.source[column + row * 10];
                break;
            default :
                modelIdD = - 1;
                break;
            }
            if (modelIdA != - 1)
            {
                fn_80026760(modelIdA, 150.0f + 300.0f * (f32)(columnBase + column), 0.0f, 300.0f + 300.0f * (f32)(rowBase + row));
                fn_80026A44(modelIdA, 1U);
                if (rowBase + row < lbl_1_bss_68B4[playerIndex3].goalRow)
                {
                    fn_80026B8C(modelIdA, 4);
                }
                else
                {
                    fn_80026B8C(modelIdA, 7);
                }
            }
            if (modelIdD != - 1)
            {
                fn_80026760(modelIdD, 300.0f + 300.0f * (f32)(columnBase + column), 0.0f, 150.0f + 300.0f * (f32)(rowBase + row));
                fn_80026A44(modelIdD, 1U);
            }
        }
    }
}

/* Places the selected result formation, then builds its paired model lists. */
const Point3d lbl_1_rodata_1BC = { 0.0f, 0.0f, 0.0f };

const Point3d lbl_1_rodata_1C8 = { 0.0f, 0.0f, 0.0f };

const Point3d lbl_1_rodata_1D4 = { 0.0f, 0.0f, 0.0f };

void fn_1_50D4(s32 phase)
{
    Point3d position;
    Point3d playerPosition;
    Point3d modelPosition;
    s16 *linkedModels;
    s16 *sourceModels;
    s16 *oldLinkedModels;
    s16 *oldSourceModels;
    s32 center;
    s32 lastGridIndex;
    s32 index;
    s32 rowCount;
    s32 linkedIndex;
    s32 sourceIndex;

    rowCount = lbl_1_bss_68B4[phase].sideLength;
    center = (16 - rowCount) / 2;
    lastGridIndex = center + rowCount - 1;
    lbl_1_bss_68B4[phase].currentTile.x = -1;
    lbl_1_bss_68B4[phase].currentTile.z = -1;
    for (index = 0; index < 5; index++) {
        fn_800269E8(lbl_1_bss_68B4[index].shadowModel, 1);
    }
    fn_80026A44(lbl_1_bss_68B4[phase].shadowModel, 1);

    position = lbl_1_rodata_1BC;
    playerPosition = lbl_1_rodata_1C8;
    position.x = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].goalColumn;
    position.z = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].goalRow;
    playerPosition.x = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].startColumn;
    playerPosition.y = 0.0f;
    playerPosition.z = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].startRow;
    fn_1_64DC(&position);
    if (phase == 4) {
        fn_80026790(lbl_1_bss_60EC[6], &position);
        fn_80026790(lbl_1_bss_60EC[0], &position);
        fn_80026A44(lbl_1_bss_60EC[0], 1);
    }
    for (index = 0; index < 5; index++) {
        if (lbl_1_bss_7E6C.playerState[index] != 0) {
            fn_800269E8(lbl_1_bss_7D2C[index].modelId, 1);
        }
    }
    OSReport("player pos ( %f, %f, %f) %d, %d \n", index, playerPosition.x, playerPosition.y,
        playerPosition.z);
    fn_80080DD4(lbl_1_bss_7E6C.player->actor,
        1 << phase);
    fn_80086F00(lbl_1_bss_7E6C.player, &playerPosition);
    fn_8008697C(lbl_1_bss_7E6C.player, &playerPosition);
    fn_1_96F4(0, &playerPosition);
    lbl_1_bss_7E6C.collisionTileX = -1;
    lbl_1_bss_7E6C.collisionTileZ = -1;

    fn_80026A44(lbl_1_bss_7E6C.modelId, 1);
    fn_80026A44(lbl_1_bss_E2, 1);
    fn_800269E8(lbl_1_bss_E2, ((1 << 30) | (1 << 1)));
    fn_8002D414(lbl_1_bss_E2, 0.0f);
    fn_80026790(lbl_1_bss_7E6C.modelId, &playerPosition);
    fn_80026790(lbl_1_bss_E2, &playerPosition);
    fn_80026790(lbl_1_bss_DE, &playerPosition);
    lbl_1_bss_7E6C.shadowAlpha = 0.6f;
    Hu3DShadowTPLvlSet(0.6f);
    modelPosition = lbl_1_rodata_1D4;
    modelPosition.x = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].exitColumn;
    modelPosition.z = 150.0f + 300.0f *
        (f32)lbl_1_bss_68B4[phase].exitRow;
    fn_80026790(lbl_1_bss_7D2C[phase].modelId, &modelPosition);
    fn_80026A44(lbl_1_bss_7D2C[phase].modelId, 1);
    lbl_1_bss_7D2C[phase].state = 1;
    lbl_1_bss_14 = 0;
    lbl_1_bss_10 = 0;
    {
    s16 modelSelection[5] = { 1, 2, 3, 4, 5 };

    for (index = 0; index < 100; index++) {
        oldLinkedModels = &lbl_1_bss_6338.source[index];
        oldSourceModels = &lbl_1_bss_617C.source[index];
        if (*oldLinkedModels != -1) {
            fn_800262D0(*oldLinkedModels);
        }
        if (*oldSourceModels != -1) {
            fn_800262D0(*oldSourceModels);
        }
    }
    memset(lbl_1_bss_617C.values, -1, (100 * sizeof(s16)));
    memset(lbl_1_bss_617C.source, -1,
        (100 * sizeof(s16)));
    memset(lbl_1_bss_6338.values, -1, (100 * sizeof(s16)));
    memset(lbl_1_bss_6338.source, -1,
        (100 * sizeof(s16)));

    for (index = 0; index < 100; index++) {
        linkedModels = &lbl_1_bss_6338.source[index];
        sourceModels = &lbl_1_bss_617C.source[index];
        *linkedModels = fn_80025D4C(
            lbl_1_bss_6148[modelSelection[phase]].first);
        *sourceModels = fn_80025D4C(
            lbl_1_bss_6148[modelSelection[phase]].second);
        fn_80026B8C(*linkedModels, 4);
        fn_80026B8C(*sourceModels, 4);
        fn_800269E8(*linkedModels, ((1 << 30) | (1 << 0)));
        fn_800269E8(*sourceModels, ((1 << 30) | (1 << 0)));
        fn_80026A44(*linkedModels, ((1 << 30) | (1 << 1)));
        fn_80026A44(*sourceModels, ((1 << 30) | (1 << 1)));
    }
    linkedIndex = 0;
    for (linkedIndex = 0; linkedIndex < 100; linkedIndex++) {
        lbl_1_bss_6338.values[linkedIndex] =
            lbl_1_bss_6338.source[linkedIndex];
        lbl_1_bss_6338.count++;
    }
    sourceIndex = 0;
    for (sourceIndex = 0; sourceIndex < 100; sourceIndex++) {
        lbl_1_bss_617C.values[sourceIndex] =
            lbl_1_bss_617C.source[sourceIndex];
        lbl_1_bss_617C.count++;
    }
    }
}

void fn_1_5930(s32 modelSetIndex, Point3d *tilePosition)
{
    s32 playerIndex;
    s32 tileX;
    s32 tileZ;

    playerIndex = lbl_1_bss_7E6C.currentIndex;
    tileX = (s32)(tilePosition->x / 300.0f);
    tileZ = (s32)(tilePosition->z / 300.0f);
    if (lbl_1_bss_68B4[playerIndex].cells[tileX + tileZ * 16].positiveZ == 1) {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[1],
            150.0f + 300.0f * (f32)tileX,
            0.0f,
            300.0f + 300.0f * (f32)tileZ);
    } else {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[1], -200.0f,
            0.0f, -200.0f);
    }
    if (lbl_1_bss_68B4[playerIndex].cells[tileX + tileZ * 16].positiveX == 1) {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[3],
            300.0f + 300.0f * (f32)tileX,
            0.0f,
            150.0f + 300.0f * (f32)tileZ);
    } else {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[3], -200.0f,
            0.0f, -200.0f);
    }
    if (lbl_1_bss_68B4[playerIndex].cells[tileX + tileZ * 16].negativeZ == 1) {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[0],
            150.0f + 300.0f * (f32)tileX,
            0.0f,
            300.0f * (f32)tileZ);
    } else {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[0], -200.0f,
            0.0f, -200.0f);
    }
    if (lbl_1_bss_68B4[playerIndex].cells[tileX + tileZ * 16].negativeX == 1) {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[2],
            300.0f * (f32)tileX, 0.0f,
            150.0f + 300.0f * (f32)tileZ);
    } else {
        fn_80026760(lbl_1_bss_6114.models[modelSetIndex].modelIds[2], -200.0f,
            0.0f, -200.0f);
    }
}

static inline void m679ColorCellAtGrid(s32 playerIndex, s32 column, s32 row,
    M679Cell *color)
{
    *color = lbl_1_bss_68B4[playerIndex]
        .cells[column + row * 16];
}

s32 fn_1_5DF0(MGACTOR *response, Point3d *position)
{
    s32 tileX;
    s32 tileZ;
    s32 playerIndex;
    s32 collisionRow;
    M679Cell edgeStates;
    s32 changed;
    tileX = (s32)(position->x / 300.0f);
    tileZ = (s32)(position->z / 300.0f);
    playerIndex = lbl_1_bss_7E6C.currentIndex;
    collisionRow = lbl_1_bss_68B4[playerIndex].sideLength;
    m679ColorCellAtGrid(playerIndex, tileX, tileZ, &edgeStates);
    changed = 0;
    if ((edgeStates.positiveZ == 1 || edgeStates.positiveZ == 3) && position->z > 300.0f * (f32)(tileZ + 1) - 125.0f)
    {
        position->z = 300.0f * (f32)(tileZ + 1) - 125.0f;
        response->push.z = 0.0f;
        changed = 1;
    }
    if ((edgeStates.positiveX == 1 || edgeStates.positiveX == 3) && position->x > 300.0f * (f32)(tileX + 1) - 125.0f)
    {
        position->x = 300.0f * (f32)(tileX + 1) - 125.0f;
        response->push.x = 0.0f;
        changed = 1;
    }
    if ((1 == edgeStates.negativeZ || edgeStates.negativeZ == 3) && position->z < 125.0f + 300.0f * (f32) tileZ)
    {
        position->z = 125.0f + 300.0f * (f32) tileZ;
        response->push.z = 0.0f;
        changed = 1;
    }
    if ((edgeStates.negativeX == 1 || edgeStates.negativeX == 3) && position->x < 125.0f + 300.0f * (f32) tileX)
    {
        position->x = 125.0f + 300.0f * (f32) tileX;
        response->push.x = 0.0f;
        changed = 1;
    }
    return changed;
}

/* Loads the four models of lbl_1_bss_6100 with their motions and sets their attributes and
 * layer. */
void fn_1_61E8(void) {
    lbl_1_bss_6100.model0 = fn_80025A04(fn_80008DD0(DATANUM(0x8a0000, 59), 0x10000000, 2));
    lbl_1_bss_6100.model1 = fn_80025A04(fn_80008DD0(DATANUM(0x8a0000, 61), 0x10000000, 2));
    lbl_1_bss_6100.model2 = fn_80025A04(fn_80008DD0(DATANUM(0x8a0000, 63), 0x10000000, 2));
    lbl_1_bss_6100.model3 = fn_80025A04(fn_80008DD0(DATANUM(0x8a0000, 65), 0x10000000, 2));
    lbl_1_bss_6100.motion0 = fn_80030458(lbl_1_bss_6100.model0, fn_80008DD0(DATANUM(0x8a0000, 60), 0x10000000, 2));
    lbl_1_bss_6100.motion1 = fn_80030458(lbl_1_bss_6100.model1, fn_80008DD0(DATANUM(0x8a0000, 62), 0x10000000, 2));
    lbl_1_bss_6100.motion2 = fn_80030458(lbl_1_bss_6100.model2, fn_80008DD0(DATANUM(0x8a0000, 64), 0x10000000, 2));
    lbl_1_bss_6100.motion3 = fn_80030458(lbl_1_bss_6100.model3, fn_80008DD0(DATANUM(0x8a0000, 66), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_6100.model0, lbl_1_bss_6100.motion0);
    fn_8002C95C(lbl_1_bss_6100.model1, lbl_1_bss_6100.motion1);
    fn_8002C95C(lbl_1_bss_6100.model2, lbl_1_bss_6100.motion2);
    fn_8002C95C(lbl_1_bss_6100.model3, lbl_1_bss_6100.motion3);
    fn_800269E8(lbl_1_bss_6100.model0, 0x40000002U);
    fn_800269E8(lbl_1_bss_6100.model1, 0x40000002U);
    fn_800269E8(lbl_1_bss_6100.model3, 0x40000002U);
    fn_800269E8(lbl_1_bss_6100.model0, 0x40000001U);
    fn_800269E8(lbl_1_bss_6100.model1, 0x40000001U);
    fn_800269E8(lbl_1_bss_6100.model2, 0x40000001U);
    fn_800269E8(lbl_1_bss_6100.model3, 0x40000001U);
    fn_80026B8C(lbl_1_bss_6100.model0, 5);
    fn_80026B8C(lbl_1_bss_6100.model1, 5);
    fn_80026B8C(lbl_1_bss_6100.model2, 5);
    fn_80026B8C(lbl_1_bss_6100.model3, 5);
    fn_80026E44(lbl_1_bss_6100.model3);
}

/* Moves the four models of lbl_1_bss_6100 to a position and shows them. */
void fn_1_64DC(Point3d *position) {
    fn_80026790(lbl_1_bss_6100.model0, position);
    fn_80026790(lbl_1_bss_6100.model1, position);
    fn_80026790(lbl_1_bss_6100.model2, position);
    fn_80026790(lbl_1_bss_6100.model3, position);
    fn_1_65D4();
    fn_80027508(lbl_1_bss_6100.model3);
}

/* Hides the four models of lbl_1_bss_6100. */
void fn_1_6568(void) {
    fn_800269E8(lbl_1_bss_6100.model0, 1U);
    fn_800269E8(lbl_1_bss_6100.model1, 1U);
    fn_800269E8(lbl_1_bss_6100.model2, 1U);
    fn_800269E8(lbl_1_bss_6100.model3, 1U);
}

/* Shows the four models of lbl_1_bss_6100. */
void fn_1_65D4(void) {
    fn_80026A44(lbl_1_bss_6100.model0, 1U);
    fn_80026A44(lbl_1_bss_6100.model1, 1U);
    fn_80026A44(lbl_1_bss_6100.model2, 1U);
    fn_80026A44(lbl_1_bss_6100.model3, 1U);
}

void fn_1_6640(void) {
    OMOBJ *obj;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_6680);
}

void fn_1_6680(OMOBJ *obj)
{
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45.0f, 100.0f,
        8000.0f, 1.2f);
    Hu3DCameraPosSet((1 << 0), 0.0f, 180.0f,
        400.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 80.0f,
        0.0f);
    Hu3DCameraViewportSet((1 << 0), 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f, 1.0f);
    obj->objFunc = fn_1_67BC;
}

void fn_1_67BC(OMOBJ *object) {
    if (lbl_1_data_C8 == 0) {
        object->objFunc = fn_1_67E0;
    }
}

/* Camera update: a fixed view when the round ends, otherwise a view following the player,
 * kept inside the maze edge or interior bounds depending on the view mode. */
void fn_1_67E0(OMOBJ *obj) {
    s32 playerIndex;
    Point3d position;
    f32 edgeMinimumX;
    f32 edgeMaximumX;
    f32 edgeMinimumZ;
    f32 edgeMaximumZ;
    f32 interiorMinimumX;
    f32 interiorMaximumX;
    f32 interiorMinimumZ;
    f32 interiorMaximumZ;
    s32 edgeTile;
    s32 interiorTile;

    playerIndex = lbl_1_bss_7E6C.currentIndex;
    if (lbl_1_data_C8 == 1) {
        Hu3DCameraPosSet((1 << 0), 0.0f, 180.0f,
            400.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 80.0f,
            0.0f);
        obj->objFunc = NULL;
        return;
    }

    if (lbl_1_data_C4 == 1) {
        edgeTile = lbl_1_bss_68B4[playerIndex].sideLength - 1;
        edgeMinimumX = 300.0f;
        edgeMaximumX = 300.0f * (f32)edgeTile;
        edgeMinimumZ = 0.0f;
        edgeMaximumZ = edgeMaximumX;
        fn_80086BC8(lbl_1_bss_7E6C.player->actor, &position);
        if (position.x < edgeMinimumX) {
            position.x = edgeMinimumX;
        }
        if (position.x > edgeMaximumX) {
            position.x = edgeMaximumX;
        }
        if (position.z < edgeMinimumZ) {
            position.z = edgeMinimumZ;
        }
        if (position.z > edgeMaximumZ) {
            position.z = edgeMaximumZ;
        }
        Hu3DCameraPosSet((1 << 0), position.x, 1000.0f,
            600.0f + position.z, 0.0f,
            1.0f, 0.0f, position.x,
            0.0f, position.z);
        return;
    }

    interiorTile = lbl_1_bss_68B4[playerIndex].sideLength - 2;
    interiorMinimumX = 600.0f;
    interiorMaximumX = 300.0f * (f32)interiorTile;
    interiorMinimumZ = 300.0f;
    interiorMaximumZ = 300.0f * (f32)(interiorTile + 1);
    fn_80086BC8(lbl_1_bss_7E6C.player->actor, &position);
    if (position.x < interiorMinimumX) {
        position.x = interiorMinimumX;
    }
    if (position.x > interiorMaximumX) {
        position.x = interiorMaximumX;
    }
    if (position.z < interiorMinimumZ) {
        position.z = interiorMinimumZ;
    }
    if (position.z > interiorMaximumZ) {
        position.z = interiorMaximumZ;
    }
    Hu3DCameraPosSet((1 << 0), position.x, 2900.0f,
        position.z, 0.0f, 0.0f,
        -1.0f, position.x, 0.0f, position.z);
}

/* Applies the recorded tile-edge response to a player actor. */
void fn_1_6BD0(MGACTOR *actor)
{
    Point3d collisionNormal;
    Point3d actorPosition;
    u32 collisionCode;
    M679Cell edgeFlags;
    s32 column;
    s32 row;

    fn_80086BC8(actor, &actorPosition);
    column = (s32)(actorPosition.x / 300.0f);
    row = (s32)(actorPosition.z / 300.0f);
    if ((fn_80086DD0(actor, &collisionCode) != 0) &&
        ((collisionCode & 0x100U) != 0)) {
        fn_1_250(column, row, &edgeFlags);
        fn_80086D54(actor, &collisionNormal);
        if (((1.0f == collisionNormal.x) && (edgeFlags.negativeX == 0)) ||
            ((-1.0f == collisionNormal.x) && (edgeFlags.positiveX == 0))) {
            if (150.0f + 300.0f * (f32)row < actorPosition.z) {
                actor->vel.z = -4.0f;
            } else {
                actor->vel.z = 4.0f;
            }
        }
        if (((1.0f == collisionNormal.z) && (edgeFlags.negativeZ == 0)) ||
            ((-1.0f == collisionNormal.z) && (edgeFlags.positiveZ == 0))) {
            if (150.0f + 300.0f * (f32)column < actorPosition.x) {
                actor->vel.x = -4.0f;
                return;
            }
            actor->vel.x = 4.0f;
        }
    }
}

s32 fn_1_6E48(void) { return 1; }

void fn_1_6E50(OMOBJ *object)
{
    if (lbl_1_data_C8 == 0) {
        fn_1_50D4(0);
    lbl_1_data_CC = HuAudSStreamPlay(MSM_STREAM_MAZE_MANUFACTORY);
        object->objFunc = fn_1_7270;
    }
}

/* Stops the result motion and returns to the completion callback. */
void fn_1_6EB4(OMOBJ *object)
{
    s32 playerIndex;

    if (fn_1_D6C8() == 1) {
        if (lbl_1_data_CC != -1) {
            HuAudSStreamFadeOut(lbl_1_data_CC, 0x3E8);
            lbl_1_data_CC = -1;
        }
        WipeCreate(2, 0, 0x3C);
        object->objFunc = fn_1_70EC;
        return;
    }
    if (lbl_1_bss_7E6C.frame == 0) {
        fn_80086E3C(lbl_1_bss_7E6C.player);
        fn_8005C2C8(lbl_1_bss_7E6C.menuIndex,
            lbl_1_bss_7E6C.player->omObj->mtnId[7],
            0.0f, 30.0f, 0U);
        lbl_1_bss_7E6C.animationDuration =
            fn_8002D5D4(lbl_1_bss_7E6C.modelId);
        for (playerIndex = 0; playerIndex < 5; playerIndex++) {
            lbl_1_bss_7D2C[playerIndex].state = 5;
        }
        fn_80026A44(lbl_1_bss_60EC[6], 1U);
        fn_80026A44(lbl_1_bss_60EC[6], 0x40000002U);
    }
    if (lbl_1_bss_7E6C.animationDuration <
            (f32)lbl_1_bss_7E6C.frame &&
        (f32)lbl_1_bss_7E6C.frame >
            30.0f + lbl_1_bss_7E6C.animationDuration) {
        WipeCreate(2, 0, 0x3C);
        object->objFunc = fn_1_70EC;
    }
    lbl_1_bss_7E6C.frame++;
}

/* Advances the result index and closes the mode effects after the wipe. */
void fn_1_70EC(OMOBJ *object)
{
    s32 resultIndex;
    s32 timeIndex;
    s32 effectIndex;
    s32 remaining;

    if (WipeCheck() != 1) {
        if (lbl_1_bss_7E6C.state == 1) {
            lbl_1_bss_7E6C.currentIndex++;
        }
        lbl_1_data_C8 = 1;
        fn_1_D750();
        fn_800269E8(lbl_1_bss_7E6C.modelId, 1U);
        resultIndex = lbl_1_bss_7E6C.currentIndex;
        if (lbl_1_bss_7E6C.currentIndex == 5) {
            resultIndex += lbl_1_bss_7E6C.progress;
            if (lbl_1_bss_7E6C.progress == 5) {
                remaining = 0x4650 - lbl_1_bss_6160.mode;
                for (timeIndex = 0; timeIndex < 5; timeIndex++) {
                    if (remaining > 0x4650 - ((timeIndex + 1) * 0x708)) {
                        break;
                    }
                }
                resultIndex += timeIndex;
            }
        }
        lbl_1_bss_7E6C.resultIndex = resultIndex;
        fn_800269E8(lbl_1_bss_7E6C.modelId, 1U);
        for (effectIndex = 0; effectIndex < 5; effectIndex++) {
            fn_800269E8(lbl_1_bss_7D2C[effectIndex].modelId, 1U);
            espDispOff(lbl_1_bss_7D2C[effectIndex].effectId);
        }
        object->objFunc = NULL;
    }
}

/* Starts the player's push animation and assigns the callback that monitors it. */
const Point3d lbl_1_rodata_22C = { 0.0f, 80.0f, 0.0f };

void fn_1_7270(OMOBJ *object)
{
    Point3d pushPosition;

    pushPosition = lbl_1_rodata_22C;
    lbl_1_bss_7E6C.inputActive = 1;
    lbl_1_bss_7E6C.value40.word = 1;
    fn_1_7D94();
    fn_8007EEBC(lbl_1_bss_7E6C.player, 1);
    fn_80086C8C(lbl_1_bss_7E6C.player->actor, &pushPosition);
    fn_80026A44(lbl_1_bss_E2, 0x40000002U);
    object->objFunc = fn_1_732C;
}

/* Monitors the motion and either restores player input or sends its current pad state. */
void fn_1_732C(OMOBJ *object)
{
    Point3d actorPosition;
    Point3d modelRotation;

    if (36.0f == fn_8002D4E0(lbl_1_bss_E2)) {
        HuAudFXPlay(MSM_SE_MG679_PLAYER_PUSH_IMPACT);
    }
    if (lbl_1_bss_7E6C.player->mode != 1) {
        fn_1_D700();
        fn_1_D6A0();
        fn_8007E888(lbl_1_bss_7E6C.player);
        fn_80086BC8(lbl_1_bss_7E6C.player->actor, &actorPosition);
        fn_80026874(lbl_1_bss_7E6C.modelId, &modelRotation);
        fn_1_7AF4(0, &actorPosition, &modelRotation);
        lbl_1_bss_7E6C.value40.word = 0;
        object->objFunc = fn_1_7E38;
        return;
    }
    fn_80080520(lbl_1_bss_7E6C.player, 0, 0, 0, 0x100);
}

/* Repositions the player when the active player's state requires a respawn. */
void fn_1_742C(OMOBJ *object)
{
    Point3d position;
    s32 playerIndex = lbl_1_bss_7E6C.currentIndex;

    fn_800267CC(lbl_1_bss_7E6C.modelId, &position);
    if ((lbl_1_bss_7E6C.playerState[playerIndex] != 1) ||
        (lbl_1_bss_7D2C[playerIndex].state == 4)) {
        fn_80086F00(lbl_1_bss_7E6C.player, &position);
        fn_8008697C(lbl_1_bss_7E6C.player, &position);
        fn_1_D6A0();
        object->objFunc = fn_1_7E38;
    }
}

/* Advances the player transition and changes to the next animation when needed. */
void fn_1_74EC(OMOBJ *object)
{
    Point3d modelPosition;

    if (lbl_1_bss_7E6C.frame < 3) {
        fn_800267CC(lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].modelId, &modelPosition);
        fn_8007E8B4(lbl_1_bss_7E6C.player, &modelPosition, 1.0f, 0.0f);
    }
    if (lbl_1_bss_7E6C.frame == 3) {
        fn_8007E888(lbl_1_bss_7E6C.player);
        fn_80086E3C(lbl_1_bss_7E6C.player);
        fn_8005C2C8(lbl_1_bss_7E6C.menuIndex, lbl_1_bss_7E6C.player->omObj->mtnId[7],
            0.0f, 20.0f, 0U);
        lbl_1_bss_7E6C.animationDuration =
            fn_8002D5D4(lbl_1_bss_7E6C.modelId);
    }
    lbl_1_bss_7E6C.frame++;
    if ((lbl_1_bss_7E6C.frame >= 3) &&
        (lbl_1_bss_7E6C.animationDuration < (f32)lbl_1_bss_7E6C.frame)) {
        if (((f32)lbl_1_bss_7E6C.frame > lbl_1_bss_7E6C.animationDuration) &&
            (lbl_1_bss_7E6C.player->omObj->mtnId[0] !=
                fn_8005C284(lbl_1_bss_7E6C.menuIndex))) {
            fn_8005C2C8(lbl_1_bss_7E6C.menuIndex,
                lbl_1_bss_7E6C.player->omObj->mtnId[0],
                0.0f, 4.0f, 0U);
        }
        if ((f32)lbl_1_bss_7E6C.frame >
            10.0f + lbl_1_bss_7E6C.animationDuration) {
            object->objFunc = fn_1_742C;
        }
    }
}

/* Brings the active player to the result model and starts its return transition. */
s32 fn_1_7798(void)
{
    Point3d playerPosition;
    Point3d modelPosition;
    Point3d playerSpawnPosition;
    s32 playerTileX;
    s32 playerTileZ;
    s32 modelTileX;
    s32 modelTileZ;
    f32 distance;

    if (lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].state != 1) {
        return 0;
    }
    fn_80086BC8(lbl_1_bss_7E6C.player->actor, &playerPosition);
    playerTileX = (s32)(playerPosition.x / 300.0f);
    playerTileZ = (s32)(playerPosition.z / 300.0f);
    fn_800267CC(lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].modelId, &modelPosition);
    modelTileX = (s32)(modelPosition.x / 300.0f);
    modelTileZ = (s32)(modelPosition.z / 300.0f);
    distance = PSVECDistance(&playerPosition, &modelPosition);
    if (distance < 140.0) {
        fn_8007E818(lbl_1_bss_7E6C.player);
        fn_8007E8B4(lbl_1_bss_7E6C.player, &modelPosition,
            1.0f, 0.0f);
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].state = 2;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].targetColumn = -1;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].targetRow = -1;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].catchTarget = -1;
        lbl_1_bss_7E6C.frame = 0;
        lbl_1_bss_7E6C.inputActive = 0;
        fn_800267CC(lbl_1_bss_7E6C.modelId, &playerSpawnPosition);
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].eventIndex =
            lbl_1_bss_7E6C.progress;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].owner = &lbl_1_bss_7E6C;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].model = lbl_1_bss_7E6C.modelId;
        lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].action = 0;
        lbl_1_bss_7E6C.progress++;
        lbl_1_bss_7E6C.playerState[
            lbl_1_bss_7E6C.currentIndex] = 1;
        espDispOn(lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].effectId);
        return 1;
    }
    return 0;
}

/* Stores a new region point at the front of its 128-point history. */
s32 fn_1_7AF4(s32 region, Point3d *position, Point3d *rotation)
{
    M679RegionPoint point;
    M679HistoryEntry gridIndices;
    s32 index;

    gridIndices.first = (s32)(position->x / 300.0f);
    gridIndices.second = (s32)(position->z / 300.0f);
    point.position = *position;
    if (rotation == NULL) {
        point.rotation.x = 0.0f;
        point.rotation.y = 0.0f;
        point.rotation.z = 0.0f;
    } else {
        point.rotation = *rotation;
    }
    point.coordinate = gridIndices;
    if (fabs(point.position.x - lbl_1_bss_E4[region][0].position.x) >= 4.0 ||
        fabs(point.position.z - lbl_1_bss_E4[region][0].position.z) >= 4.0) {
        for (index = 0x7F; index >= 1; index--) {
            lbl_1_bss_E4[region][index] =
                lbl_1_bss_E4[region][index - 1];
        }
        lbl_1_bss_E4[region][0] = point;
        return 1;
    }
    return 0;
}

const HuVecF lbl_1_rodata_250 = { 0.0f, 1.0f, 0.0f };

void fn_1_7D94(void)
{
    HuVecF lightPosition;
    HuVecF actorPosition;
    HuVecF shadowPosition;

    lightPosition = lbl_1_rodata_250;
    fn_80086BC8(lbl_1_bss_7E6C.player->actor,
        &shadowPosition);
    actorPosition = shadowPosition;
    shadowPosition.z -= 100.0f;
    shadowPosition.y = 3000.0f;
    Hu3DShadowPosSet(&shadowPosition, &lightPosition, &actorPosition);
}

static inline s32 m679RecordPlayerPoint(s32 region, Point3d *position, Point3d *rotation)
{
    M679RegionPoint point;
    M679HistoryEntry gridIndices;
    s32 index;

    gridIndices.first = (s32)(position->x / 300.0f);
    gridIndices.second = (s32)(position->z / 300.0f);
    point.position = *position;
    if (rotation == NULL) {
        point.rotation.x = 0.0f;
        point.rotation.y = 0.0f;
        point.rotation.z = 0.0f;
    } else {
        point.rotation = *rotation;
    }
    point.coordinate = gridIndices;
    if (fabs(point.position.x -
            lbl_1_bss_E4[region][0].position.x) >= 4.0 ||
        fabs(point.position.z -
            lbl_1_bss_E4[region][0].position.z) >= 4.0) {
        for (index = 0x7F; index >= 1; index--) {
            lbl_1_bss_E4[region][index] =
                lbl_1_bss_E4[region][index - 1];
        }
        lbl_1_bss_E4[region][0] = point;
        return 1;
    }
    return 0;
}

const HuVecF lbl_1_rodata_260 = { 0.0f, 0.0f, -1.0f };

void fn_1_7E38(OMOBJ *object)
{
    HuVecF lightPosition;
    Point3d actorPosition;
    Point3d buttonShadowPosition;
    Point3d playerPosition;
    Point3d modelRotation;
    Point3d stickInput;
    f32 stickScale;
    s32 movementChanged = 0;
    s32 oldTileX;
    s32 currentTileX;
    s32 currentTileZ;
    s32 oldTileZ;
    Point3d modelPosition;
    lbl_1_bss_7E6C.player->actor->vel.x = 0.0f;
    lbl_1_bss_7E6C.player->actor->vel.y = 0.0f;
    lbl_1_bss_7E6C.player->actor->vel.z = 0.0f;
    if (fn_1_D6C8() == 1)
    {
        lbl_1_data_C4 = 1;
        lbl_1_bss_7E6C.inputActive = 0;
        fn_8007E818(lbl_1_bss_7E6C.player);
        object->objFunc = fn_1_6EB4;
        return;
    }
    if ((lbl_802C0008[lbl_1_bss_7E6C.playerIndex] & 0x2000) != 0)
    {
        fn_1_D7E0();
        if (1 == lbl_1_data_C4)
        {
            fn_8007E888(lbl_1_bss_7E6C.player);
            fn_80087168(lbl_1_bss_7E6C.player, 0x200U);
        }
        else
        {
            lightPosition = lbl_1_rodata_260;
            fn_800267CC(lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].shadowModel, & buttonShadowPosition);
            actorPosition = buttonShadowPosition;
            buttonShadowPosition.y = 6000.0f;
            Hu3DShadowPosSet(& buttonShadowPosition, & lightPosition, & actorPosition);
            fn_8007E818(lbl_1_bss_7E6C.player);
            fn_80087158(lbl_1_bss_7E6C.player, 0x200U);
            lbl_1_bss_7E6C.inputActive = 0;
        }
    }
    if (lbl_1_data_C4 == 0)
    {
        return;
    }
    fn_1_7D94();
    fn_1_AD40(0);
    if (lbl_1_bss_7E6C.player->motNo == 0)
    {
        lbl_1_bss_7E6C.inputActive = 0;
    }
    else
    {
        stickInput.x = lbl_8024C7D8[lbl_1_bss_7E6C.padIndex];
        stickInput.z = lbl_8024C7E8[lbl_1_bss_7E6C.padIndex];
        stickInput.y = 0.0f;
        if (PSVECMag(&stickInput) < 0.8f) {
            stickScale = 0.8f / PSVECMag(&stickInput);
            PSVECScale(&stickInput, &stickInput, stickScale);
            if (stickInput.x > 1.0f) {
                stickInput.x = 1.0f;
            }
            if (stickInput.z > 1.0f) {
                stickInput.z = 1.0f;
            }
            fn_80080520(lbl_1_bss_7E6C.player,
                (s32)(56.0f * stickInput.x),
                (s32)(56.0f * stickInput.z), 0, 0);
        }
        lbl_1_bss_7E6C.inputActive = 1;
        fn_80086BC8(lbl_1_bss_7E6C.player->actor, & playerPosition);
        fn_80026874(lbl_1_bss_7E6C.modelId, & modelRotation);
        oldTileX = (s32)(playerPosition.x / 300.0f);
        oldTileZ = (s32)(playerPosition.z / 300.0f);
        movementChanged = fn_1_5DF0(lbl_1_bss_7E6C.player->actor, &playerPosition);
        fn_80086B04(lbl_1_bss_7E6C.player->actor, & playerPosition);
        fn_1_6BD0(lbl_1_bss_7E6C.player->actor);
        if (m679RecordPlayerPoint(0, &playerPosition, &modelRotation) == 0) {
            lbl_1_bss_7E6C.inputActive = 0;
        }
    }
    fn_80086BC8(lbl_1_bss_7E6C.player->actor, & modelPosition);
    currentTileX = (s32)(modelPosition.x / 300.0f);
    currentTileZ = (s32)(modelPosition.z / 300.0f);
    if (currentTileX == lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalColumn && currentTileZ == lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalRow)
    {
        lbl_1_bss_7E6C.player->actor->vel.x = 0.0f;
        lbl_1_bss_7E6C.player->actor->vel.y = 0.0f;
        lbl_1_bss_7E6C.player->actor->vel.z = 0.0f;
        lbl_1_bss_7E6C.playerArrivalState = 0;
        lbl_1_bss_7E6C.inputActive = 0;
        fn_1_D6B4();
        fn_8007E818(lbl_1_bss_7E6C.player);
        fn_80086CBC((MGACTOR *)lbl_1_bss_7E6C.player, 450.0f);
        fn_8007EEBC(lbl_1_bss_7E6C.player, 1);
        fn_8005DFC0(lbl_1_bss_7E6C.menuIndex, 0);
        if (lbl_1_bss_7E6C.playerState[lbl_1_bss_7E6C.currentIndex] == 1)
        {
            OSReport("npc change state\n");
            lbl_1_bss_7D2C[lbl_1_bss_7E6C.currentIndex].state = 5;
        }
        if (lbl_1_bss_7E6C.currentIndex == 4)
        {
            lbl_1_bss_7E6C.state = 1;
            fn_80026A44(lbl_1_bss_60EC[6], 1U);
            fn_80026A44(lbl_1_bss_60EC[6], 0x40000002U);
            HuAudFXPlay(MSM_SE_MG679_GOAL_DOOR_STEP);
            if (lbl_1_data_CC != - 1)
            {
                HuAudSStreamFadeOut(lbl_1_data_CC, 0x3E8);
                lbl_1_data_CC = - 1;
            }
        }
        object->objFunc = fn_1_8DC8;
        return;
    }
    if (fn_1_7798() == 1)
    {
        fn_1_D6B4();
        object->objFunc = fn_1_74EC;
        return;
    }
}

/* Advances the character's jump and wipe transition to the next result state. */
void fn_1_8DC8(OMOBJ *object)
{
    Point3d destination;
    Point3d modelPosition;

    if ((15.0f == fn_8002D4E0(lbl_1_bss_60EC[6])) ||
        (26.0f == fn_8002D4E0(lbl_1_bss_60EC[6]))) {
        HuAudFXPlay(MSM_SE_MG679_GOAL_DOOR_STEP);
    }
    if ((lbl_1_bss_14 == 1) && (lbl_1_bss_10 == 1)) {
        Hu3DShadowTPLvlSet(lbl_1_bss_7E6C.shadowAlpha);
        lbl_1_bss_7E6C.shadowAlpha -= 0.02f;
        if (lbl_1_bss_7E6C.shadowAlpha < 0.0f) {
            lbl_1_bss_7E6C.shadowAlpha = 0.0f;
        }
    }
    if ((lbl_1_bss_14 == 1) && (lbl_1_bss_10 == 0) &&
        (fn_8002D4E0(lbl_1_bss_6100.model0) > 25.0f) &&
        (lbl_1_bss_7E6C.player->omObj->mtnId[3] !=
            fn_8005C284(lbl_1_bss_7E6C.menuIndex))) {
        fn_8005C2C8(lbl_1_bss_7E6C.menuIndex,
            lbl_1_bss_7E6C.player->omObj->mtnId[3], 0.0f,
            10.0f, 0U);
        if (lbl_1_bss_7E6C.currentIndex == 4) {
            WipeCreate(2, 0, 0x3C);
        } else {
            WipeCreate(2, 0, 0x18);
        }
        lbl_1_bss_10 = 1;
        HuAudFXPlay(MSM_SE_MG679_EXIT_WIPE);
    }
    if (lbl_1_bss_14 == 0) {
        destination.x = 150.0f + 300.0f *
            (f32)lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalColumn;
        destination.y = 135.0f;
        destination.z = 150.0f + 300.0f *
            (f32)lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalRow;
        fn_8007E8B4(lbl_1_bss_7E6C.player, &destination,
            4.0f, 10.0f);
        fn_800267CC(lbl_1_bss_7E6C.modelId, &modelPosition);
        if ((lbl_1_bss_7E6C.player->actor->velY < 0.0f) &&
            (lbl_1_bss_7E6C.player->omObj->mtnId[4] !=
            fn_8005C284(lbl_1_bss_7E6C.menuIndex))) {
            fn_8005C2C8(lbl_1_bss_7E6C.menuIndex,
                lbl_1_bss_7E6C.player->omObj->mtnId[4], 0.0f,
                8.0f, 0U);
        }
        if ((modelPosition.y < 135.0f) &&
            (lbl_1_bss_7E6C.player->actor->velY < 0.0f)) {
            fn_8005DDEC(6);
            fn_8005F968(lbl_1_bss_7E6C.menuIndex, &destination);
            fn_80086E3C(lbl_1_bss_7E6C.player);
            fn_80026760(lbl_1_bss_7E6C.modelId, 0.0f,
                0.0f, 0.0f);
            fn_8002741C(lbl_1_bss_6100.model3, "meiro-PCjamphook",
                lbl_1_bss_7E6C.modelId);
            fn_80026A44(lbl_1_bss_6100.model0, 0x40000002U);
            fn_80026A44(lbl_1_bss_6100.model1, 0x40000002U);
            fn_80026A44(lbl_1_bss_6100.model3, 0x40000002U);
            lbl_1_bss_14 = 1;
        }
    }
    if ((WipeCheck() == 0) && (lbl_1_bss_10 == 1)) {
        if (lbl_1_bss_7E6C.currentIndex == 4) {
            object->objFunc = fn_1_70EC;
        } else {
        fn_80086CBC((MGACTOR *)lbl_1_bss_7E6C.player,
            150.0f);
        fn_800269E8(lbl_1_bss_6100.model0, 0x40000002U);
        fn_800269E8(lbl_1_bss_6100.model1, 0x40000002U);
        fn_800269E8(lbl_1_bss_6100.model3, 0x40000002U);
        fn_8002D414(lbl_1_bss_6100.model0, 0.0f);
        fn_8002D414(lbl_1_bss_6100.model1, 0.0f);
        fn_8002D414(lbl_1_bss_6100.model3, 0.0f);
        fn_8005DFC0(lbl_1_bss_7E6C.menuIndex, 1);
        fn_8005DDEC(3);
        lbl_1_bss_7E6C.currentIndex++;
        fn_1_50D4(lbl_1_bss_7E6C.currentIndex);
        WipeCreate(1, 0, 0x1E);
        object->objFunc = fn_1_7270;
        }
    }
}

/* Creates the result actor and places it at the configured result-window position. */
const Point3d lbl_1_rodata_288 = { 0.0f, 0.0f, 0.0f };

void fn_1_9430(OMOBJ *obj)
{
    Point3d position;
    MGACTOR_PARAM actorParam;
    MGPLAYER *player;

    position = lbl_1_rodata_288;
    player = NULL;
    actorParam.height = 150.0f;
    actorParam.radius = 35.0f;
    actorParam.param = 0;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = NULL;
    player = fn_80080728(lbl_1_bss_7E6C.playerIndex, &actorParam, 8, 1U,
                            -0x1FU, lbl_1_data_30);
    lbl_1_bss_7E6C.player = player;
    fn_80086AA8(player->actor, &position);
    fn_8007ED60(player);
    fn_80080DD4(player->actor, 0x1FU);
    fn_80026B8C((s16)player->actor->mdlId, 1);
    fn_8005DDEC(3);
    fn_80026B64((s16)player->actor->mdlId, 1U);
    fn_800269E8((s16)player->actor->mdlId, 1U);
    fn_80026E44((s16)player->actor->mdlId);
    lbl_1_bss_7E6C.modelId = (s16)player->actor->mdlId;
    obj->objFunc = fn_1_6E50;
}

/* Selects the ungrouped player for the result actor and clears its per-result state. */
void fn_1_95A4(void)
{
    OMOBJ *obj;
    s32 playerIndex;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_9430);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_80265728[playerIndex].grpNo == 0) {
            break;
        }
    }
    lbl_1_bss_7E6C.playerIndex = playerIndex;
    lbl_1_bss_7E6C.menuIndex = lbl_80265728[playerIndex].charNo;
    lbl_1_bss_7E6C.padIndex = lbl_80265728[playerIndex].padNo;
    OSReport("pad no : %d\n", lbl_1_bss_7E6C.padIndex);
    lbl_1_bss_7E6C.player = NULL;
    lbl_1_bss_7E6C.resultIndex = 0;
    lbl_1_bss_7E6C.state = 0;
    lbl_1_bss_7E6C.progress = 0;
    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        lbl_1_bss_7E6C.playerState[playerIndex] = 0;
    }
}

/* Copies the point into each region record and stores its scaled grid coordinates. */
void fn_1_96F4(s32 region, Point3d *position)
{
    s32 index;

    for (index = 0; index < 128; index++) {
        lbl_1_bss_E4[region][index].coordinate.first =
            (s32)(position->x / 300.0f);
        lbl_1_bss_E4[region][index].coordinate.second =
            (s32)(position->z / 300.0f);
        lbl_1_bss_E4[region][index].position = *position;
    }
}

s32 fn_1_97CC(s32 region, M679HistoryEntry *gridCoordinate, Point3d *worldPosition)
{
    s32 index = 0;

    for (index = 0; index < 0x80; index++) {
        if ((gridCoordinate->first == lbl_1_bss_E4[region][index].coordinate.first) &&
            (gridCoordinate->second == lbl_1_bss_E4[region][index].coordinate.second)) {
            if (index > 0) {
                index--;
            }
            OSReport(" %d : ( %d : %d ), ( %d, %d )\n", index,
                lbl_1_bss_E4[region][index].coordinate.first,
                gridCoordinate->first,
                lbl_1_bss_E4[region][index].coordinate.second,
                gridCoordinate->second);
            worldPosition->x = 150.0f + 300.0f *
                (f32)lbl_1_bss_E4[region][index].coordinate.first;
            worldPosition->y = 0.0f;
            worldPosition->z = 150.0f + 300.0f *
                (f32)lbl_1_bss_E4[region][index].coordinate.second;
            return 1;
        }
    }
    OSReport("No History.\n");
    worldPosition->x = 150.0f + 300.0f *
        (f32)lbl_1_bss_E4[region][0].coordinate.first;
    worldPosition->y = 0.0f;
    worldPosition->z = 150.0f + 300.0f *
        (f32)lbl_1_bss_E4[region][0].coordinate.second;
    return 0;
}

void fn_1_9AA0(M679PlayerWork *work, s16 motionIndex, u32 attributes) {
    fn_8002CCC0(work->modelId, work->motions[motionIndex], 0.0f, 30.0f, attributes);
}

f32 fn_1_9B04(M679PlayerWork *work)
{
    if (lbl_1_bss_7E6C.player->motNo == 1) {
        if (work->motions[1] != fn_8002D3EC(work->modelId) &&
            work->motions[1] != fn_8002D3C4(work->modelId)) {
            fn_8002CCC0(work->modelId, work->motions[1],
                               0.0f, 30.0f, 0x40000001U);
        }
        return 6.0f;
    }
    if (work->motions[2] != fn_8002D3EC(work->modelId) &&
        work->motions[2] != fn_8002D3C4(work->modelId)) {
        fn_8002CCC0(work->modelId, work->motions[2],
                           0.0f, 30.0f, 0x40000001U);
    }
    return 10.0f;
}

int fn_1_9C18(HuVecF *sourceDirection, HuVecF *normalizedDirection)
{
    int result;

    if (PSVECSquareMag(sourceDirection) < 1e-06) {
        normalizedDirection->x = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        normalizedDirection->z = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        normalizedDirection->y = frandmod(1) != 0U ? 0.01f : -0.01f;
        PSVECNormalize(normalizedDirection, normalizedDirection);
        result = 0;
    } else {
        PSVECNormalize(sourceDirection, normalizedDirection);
        result = 1;
    }
    return result;
}

s32 fn_1_9D70(M679PlayerWork *movingModel,
    Point3d *targetPosition, f32 speed, f32 arrivalRadius)
{
    Point3d modelPosition;
    HuVecF direction;
    f32 rotationY;

    arrivalRadius = (f32)(0.001 +
        (f64)(arrivalRadius * arrivalRadius));
    fn_800267CC(movingModel->modelId, &modelPosition);
    PSVECSubtract(targetPosition, &modelPosition, &direction);
    direction.y = 0.0f;
    if (PSVECSquareMag(&direction) < arrivalRadius) {
        return 1;
    }
    fn_1_9C18(&direction, &direction);
    if (direction.z > 1.0f) {
        direction.z = 1.0f;
    } else if (direction.z < -1.0f) {
        direction.z = -1.0f;
    }
    modelPosition.x += speed * direction.x;
    modelPosition.y += speed * direction.y;
    modelPosition.z += speed * direction.z;
    rotationY = acosf(direction.z);
    if (direction.x < 0.0f) {
        rotationY = -rotationY;
    }
    rotationY = (f32)((f64)(180.0f * rotationY) / M_PI);
    fn_80026790(movingModel->modelId, &modelPosition);
    fn_80026808(movingModel->modelId, 0.0f, rotationY, 0.0f);
    return 0;
}

/* Moves the character toward its recorded point and returns it to the callback when ready. */
void fn_1_A09C(OMOBJ *object)
{
    M679PlayerWork *work;
    Point3d playerDirection;
    Point3d modelPosition;
    Point3d playerPosition;
    Point3d sourcePosition;
    f32 movementDistanceSquared;
    s32 finished;
    s32 progress;
    s16 gridIndices[2];

    finished = 0;
    work = object->data;
    movementDistanceSquared = 2756.251f;
    progress = work->progress;
    fn_800267CC(work->modelId, &modelPosition);
    fn_800267CC(work->owner->modelId, &playerPosition);
    gridIndices[0] = (s32)(modelPosition.x / 300.0f);
    gridIndices[1] = (s32)(modelPosition.z / 300.0f);
    sourcePosition = lbl_1_bss_E4[0][progress].position;
    if (fn_1_9D70(work, &sourcePosition,
            10.0f, 35.0f) == 1) {
        work->progress += 4;
        if (work->progress > 0x80) {
            work->progress = 0x7F;
            finished = 1;
        }
    }

    if (finished == 1) {
        fn_1_9D70(work, &lbl_1_bss_E4[0][0].position,
            0.0f, 0.0f);
        work->state = 4;
        object->objFunc = fn_1_C8C0;
        return;
    }

    if (work->progress > 0xE) {
        PSVECSubtract(&playerPosition, &modelPosition, &playerDirection);
        playerDirection.y = 0.0f;
        if (PSVECSquareMag(&playerDirection) > movementDistanceSquared) {
            fn_1_9D70(work, &lbl_1_bss_E4[0][0].position,
                0.0f, 0.0f);
            work->state = 4;
            object->objFunc = fn_1_C8C0;
            return;
        }
    }
}

/* Once the current motion ends, starts the third motion and continues in fn_1_C8C0. */
void fn_1_AB08(OMOBJ *object) {
    M679PlayerWork *motion;

    motion = object->data;
    if (fn_8002D75C(motion->modelId) == 1) {
        motion->state = 3;
        motion->progress = 0;
        fn_8002CCC0(motion->modelId, motion->motions[2], 0.0f, 30.0f, 0x40000001U);
        motion->state = 4;
        object->objFunc = fn_1_C8C0;
    }
}

/* Returns 1 when the cell is the current maze's goal. */
s32 fn_1_AB9C(s32 column, s32 row) {
    if ((column == lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalColumn) &&
        (row == lbl_1_bss_68B4[lbl_1_bss_7E6C.currentIndex].goalRow)) {
        return 1;
    }
    return 0;
}

void fn_1_ABFC(HuVecF *pair) {
    pair->x = -50.0f;
    pair->z = -50.0f;
}

void fn_1_AC20(HuVecF *pair) {
    pair->x = 0.0f;
    pair->z = -50.0f;
}

void fn_1_AC44(HuVecF *pair) {
    pair->x = 50.0f;
    pair->z = -50.0f;
}

void fn_1_AC68(HuVecF *pair) {
    pair->x = -50.0f;
    pair->z = 0.0f;
}

void fn_1_AC8C(HuVecF *pair) {
    pair->x = 0.0f;
    pair->z = 0.0f;
}

void fn_1_ACB0(HuVecF *pair) {
    pair->x = 50.0f;
    pair->z = 0.0f;
}

void fn_1_ACD4(HuVecF *pair) {
    pair->x = -50.0f;
    pair->z = 50.0f;
}

void fn_1_ACF8(HuVecF *pair) {
    pair->x = 0.0f;
    pair->z = 50.0f;
}

void fn_1_AD1C(HuVecF *pair) {
    pair->x = 50.0f;
    pair->z = 50.0f;
}

s32 fn_1_AD40(s32 mode)
{
    s32 currentQuadrant;
    s32 targetQuadrant;
    f32 interactionRadiusSquared;
    Point3d modelPosition;
    Point3d playerPosition;
    Point3d difference;
    Point3d corner[4];
    Point3d nextPosition;
    s32 gridXIndex;
    s32 gridZIndex;
    f32 gridCenterX;
    f32 gridCenterZ;
    s32 playerIndex;

    playerIndex = lbl_1_bss_7E6C.currentIndex;
    interactionRadiusSquared = 87.501f;
    interactionRadiusSquared *= interactionRadiusSquared;
    if (lbl_1_bss_7E6C.playerState[playerIndex] == 0) {
        return 0;
    }

    fn_800267CC(lbl_1_bss_7E6C.modelId, &modelPosition);
    fn_800267CC(lbl_1_bss_7D2C[playerIndex].modelId, &playerPosition);
    PSVECSubtract(&modelPosition, &playerPosition, &difference);
    difference.y = 0.0f;
    if (PSVECSquareMag(&difference) >= interactionRadiusSquared) {
        return 0;
    }
    if (lbl_1_bss_7D2C[playerIndex].moving == 1) {
        return 0;
    }

    gridXIndex = (s32)(modelPosition.x / 300.0f);
    gridZIndex = (s32)(modelPosition.z / 300.0f);
    gridCenterX = 150.0f + 300.0f * (f32)gridXIndex;
    gridCenterZ = 150.0f + 300.0f * (f32)gridZIndex;
    currentQuadrant = 0;
    targetQuadrant = 0;

    corner[0].x = gridCenterX - 75.0f;
    corner[0].z = gridCenterZ - 75.0f;
    corner[1].x = gridCenterX + 75.0f;
    corner[1].z = gridCenterZ - 75.0f;
    corner[2].x = gridCenterX - 75.0f;
    corner[2].z = gridCenterZ + 75.0f;
    corner[3].x = gridCenterX + 75.0f;
    corner[3].z = gridCenterZ + 75.0f;

    if (gridCenterX < playerPosition.x) {
        currentQuadrant++;
    }
    if (gridCenterZ < playerPosition.z) {
        currentQuadrant += 2;
    }
    if (gridCenterX < modelPosition.x) {
        targetQuadrant++;
    }
    if (gridCenterZ < modelPosition.z) {
        targetQuadrant += 2;
    }
    OSReport("pc index : %d, npc index : %d\n", targetQuadrant, currentQuadrant);
    switch (currentQuadrant) {
    case 0:
    case 3:
        switch (targetQuadrant) {
        case 3:
        case 0:
            nextPosition = rand8() % 100 < 0x32 ? corner[1] : corner[2];
            break;
        case 1:
            nextPosition = corner[2];
            break;
        case 2:
            nextPosition = corner[1];
            break;
        default:
            break;
        }
        break;
    case 1:
    case 2:
        switch (targetQuadrant) {
        case 1:
        case 2:
            nextPosition = rand8() % 100 < 0x32 ? corner[0] : corner[3];
            break;
        case 0:
            nextPosition = corner[3];
            break;
        case 3:
            nextPosition = corner[0];
            break;
        }
        break;
    }

    nextPosition.y = 0.0f;
    lbl_1_bss_7D2C[playerIndex].moving = 1;
    lbl_1_bss_7D2C[playerIndex].targetPosition = nextPosition;
}

void fn_1_B1F8(M679PlayerWork *moving)
{
    Point3d playerPosition;
    Point3d startPosition;
    Point3d rotation;
    f32 step = 0.0f;
    f32 historyDistance;
    s32 result = 0;
    s32 historyDelay;

    historyDelay = 10;
    fn_800267CC(moving->modelId, &startPosition);
    fn_800267CC(lbl_1_bss_7E6C.modelId, &playerPosition);
    if (moving->moving == 1) {
        if (((s32)(playerPosition.x / 300.0f) !=
             (s32)(startPosition.x / 300.0f) ||
             (s32)(playerPosition.z / 300.0f) !=
             (s32)(startPosition.z / 300.0f)) &&
            lbl_1_bss_7E6C.inputActive == 1) {
            moving->moving = 0;
            goto update_history;
        }

        step = fn_1_9B04(moving);
        if (fn_1_9D70(moving,
                &moving->targetPosition, step, 8.75f) == 1) {
            fn_1_9D70(moving,
                &playerPosition, 0.0f, 0.0f);
            moving->moving = 0;
        }
        return;
    }

update_history:
    if (lbl_1_bss_7E6C.inputActive == 1) {
        historyDistance = PSVECDistance(
            &lbl_1_bss_E4[moving->action][historyDelay].position,
            &startPosition);
        if (historyDistance > 84.0f) {
        step = fn_1_9B04(moving);
        fn_1_9D70(moving,
            &lbl_1_bss_E4[moving->action][historyDelay].position,
            step, 35.0f);

        fn_800267CC(moving->modelId, &startPosition);
        fn_80026874(moving->modelId, &rotation);
        m679RecordPlayerPoint(moving->characterNo + 1, &startPosition, &rotation);
        }
        moving->moving = 0;
        return;
    }

    if (moving->motions[0] != fn_8002D3EC(moving->modelId) &&
        moving->motions[0] != fn_8002D3C4(moving->modelId)) {
        fn_8002CCC0(moving->modelId, moving->motions[0],
            0.0f, 30.0f, 0x40000001U);
    }
}

/* Chooses a clear adjacent tile, then moves the model to that tile. */
void fn_1_C100(M679PlayerWork *character)
{
    Point3d targetPosition;
    Point3d direction;
    Point3d modelPosition;
    M679Cell cell;
    u8 availableCell[4];
    s32 targetX;
    s32 targetZ;
    s32 playerIndex;
    s32 side;
    s32 attempts;
    f32 movementDistanceSquared;
    f32 directionZ;
    f32 angle;
    f32 signedAngle;
    f32 rotationY;
    s32 reached;

    playerIndex = lbl_1_bss_7E6C.currentIndex;
    fn_800267CC(character->modelId, &targetPosition);
    if ((character->targetColumn == -1) || (character->targetRow == -1)) {
        fn_1_1A0(&targetPosition, &cell);
choose_tile:
        targetX = (s32)(targetPosition.x / 300.0f);
        targetZ = (s32)(targetPosition.z / 300.0f);
        side = rand8() % 4;
        attempts = 0;
        while (attempts < 4) {
            availableCell[0] = cell.negativeZ;
            availableCell[1] = cell.positiveZ;
            availableCell[2] = cell.negativeX;
            availableCell[3] = cell.positiveX;
            if (availableCell[side] == 0) {
                break;
            }
            side = (side + 1) % 4;
            attempts++;
        }
        switch (side) {
        case 0:
            targetZ--;
            break;
        case 1:
            targetZ++;
            break;
        case 2:
            targetX--;
            break;
        case 3:
            targetX++;
            break;
        }
        if (abs(lbl_1_bss_68B4[playerIndex].exitColumn - targetX) > 3) {
            goto choose_tile;
        }
        if (abs(lbl_1_bss_68B4[playerIndex].exitRow - targetZ) > 3) {
            goto choose_tile;
        }
        if ((targetX == lbl_1_bss_68B4[playerIndex].goalColumn) &&
            (targetZ == lbl_1_bss_68B4[playerIndex].goalRow)) {
            goto choose_tile;
        }
        character->targetColumn = targetX;
        character->targetRow = targetZ;
        character->motionCounter = 0x32;
        return;
    }

    targetPosition.x = 150.0f + 300.0f * (f32)character->targetColumn;
    targetPosition.y = 0.0f;
    targetPosition.z = 150.0f + 300.0f * (f32)character->targetRow;
    if ((character->motions[1] != fn_8002D3EC(character->modelId)) &&
        (character->motions[1] != fn_8002D3C4(character->modelId))) {
        fn_8002CCC0(character->modelId, character->motions[1],
            0.0f, 30.0f, 0x40000001U);
    }
    character->motionCounter--;
    movementDistanceSquared = 87.5f;
    movementDistanceSquared = (f32)(0.001 +
        (f64)(movementDistanceSquared * movementDistanceSquared));
    fn_800267CC(character->modelId, &modelPosition);
    PSVECSubtract(&targetPosition, &modelPosition, &direction);
    direction.y = 0.0f;
    if (PSVECSquareMag(&direction) < movementDistanceSquared) {
        reached = 1;
    } else {
        fn_1_9C18(&direction, &direction);
        if (direction.z > 1.0f) {
            direction.z = 1.0f;
        } else if (direction.z < -1.0f) {
            direction.z = -1.0f;
        }
        modelPosition.x += 6.0f * direction.x;
        modelPosition.y += 6.0f * direction.y;
        modelPosition.z += 6.0f * direction.z;
        directionZ = direction.z;
        angle = (f32)acos((f64)directionZ);
        signedAngle = angle;
        rotationY = signedAngle;
        if (direction.x < 0.0f) {
            rotationY = -rotationY;
        }
        rotationY = (f32)((f64)(180.0f * rotationY) / M_PI);
        fn_80026790(character->modelId, &modelPosition);
        fn_80026808(character->modelId, 0.0f, rotationY, 0.0f);
        reached = 0;
    }
    if ((reached == 1) || (character->motionCounter == 0)) {
        character->targetColumn = -1;
        character->targetRow = -1;
    }
}

/* Places the character model, resets its work state, and advances its callback. */
const Point3d lbl_1_rodata_2E8 = { 0.0f, 0.0f, 0.0f };

void fn_1_C7C4(OMOBJ *object)
{
    M679PlayerWork *work = object->data;
    Point3d position = lbl_1_rodata_2E8;

    position.x = 300.0f;
    position.z = 300.0f;
    fn_80026790(work->modelId, &position);
    fn_80026B8C(work->modelId, 1);
    fn_8005DDEC(3);
    fn_80026B64(work->modelId, 1U);
    fn_800269E8(work->modelId, 1U);
    fn_80026E44(work->modelId);
    work->targetColumn = -1;
    work->targetRow = -1;
    work->phase = 0;
    object->objFunc = fn_1_C89C;
}

void fn_1_C89C(OMOBJ *object) {
    if (lbl_1_data_C8 == 0) {
        object->objFunc = fn_1_C8C0;
    }
}

void fn_1_C8C0(OMOBJ *object)
{
    M679PlayerWork *work = object->data;
    Point3d playerPosition;

    if (lbl_1_data_C8 == 1) {
        fn_800269E8(work->modelId, 1U);
        object->objFunc = NULL;
        return;
    }

    if (work->state == 0) {
        return;
    }

    if (work->state == 1 &&
        work->playerIndex < lbl_1_bss_7E6C.currentIndex) {
        fn_800269E8(work->modelId, 1U);
        object->objFunc = NULL;
        return;
    }

    switch (work->state) {
    case 0:
        return;

    case 1:
        if (work->playerIndex < lbl_1_bss_7E6C.currentIndex) {
            fn_800269E8(work->modelId, 1U);
            object->objFunc = NULL;
            return;
        }
        fn_1_C100(work);
        return;

    case 2:
        work->motionCounter = 0;
        fn_80086BC8(lbl_1_bss_7E6C.player->actor, &playerPosition);
        fn_1_9D70(work, &playerPosition, 0.0f, 0.0f);
        fn_8002CCC0(work->modelId, work->motions[3],
            0.0f, 30.0f, 0U);
        {
            s32 soundIds[5] = { MSM_SE_GUIDE_KURIBO_TALK, MSM_SE_GUIDE_COIN_COLLECTOR_TALK,
                                MSM_SE_GUIDE_SHOP_NIGHT_SUCCESS, MSM_SE_GUIDE_MAZE_CHARACTER_CHEER,
                                MSM_SE_GUIDE_SHOP_DAY_SUCCESS };
            HuAudFXPlay(soundIds[work->characterNo]);
        }
        object->objFunc = fn_1_AB08;
        return;

    case 3:
        break;

    case 4:
        fn_1_B1F8(work);
        return;

    case 5:
        fn_8002CCC0(work->modelId, work->motions[3],
            0.0f, 30.0f, 0x40000001U);
        object->objFunc = NULL;
        break;
    }
}

/* Creates the five player models and their per-player effect and motions. */
void fn_1_CDD0(void)
{
    u32 modelDataIds[5] = { DATANUM(0x8a0000, 28), DATANUM(0x8a0000, 33), DATANUM(0x8a0000, 38), DATANUM(0x8a0000, 43), DATANUM(0x8a0000, 48) };
    u32 effectDataIds[5] = { DATANUM(0x8a0000, 68), DATANUM(0x8a0000, 69), DATANUM(0x8a0000, 70), DATANUM(0x8a0000, 71), DATANUM(0x8a0000, 72) };
    s32 playerIndex;
    s32 motionIndex;
    OMOBJ *object;

    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        object = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_C7C4);
        object->data = &lbl_1_bss_7D2C[playerIndex];
        lbl_1_bss_7D2C[playerIndex].characterNo = playerIndex;
        lbl_1_bss_7D2C[playerIndex].playerIndex = playerIndex;
        lbl_1_bss_7D2C[playerIndex].state = 0;
        lbl_1_bss_7D2C[playerIndex].modelId = fn_80025A04(
            fn_80008DD0(modelDataIds[playerIndex], 0x10000000U, 2));
        lbl_1_bss_7D2C[playerIndex].effectId =
            espEntry(effectDataIds[playerIndex], 0x51, 0);
        espPosSet(lbl_1_bss_7D2C[playerIndex].effectId,
            368.0f + 42.0f * (f32)playerIndex,
            64.0f);
        espDispOff(lbl_1_bss_7D2C[playerIndex].effectId);
        for (motionIndex = 0; motionIndex < 4; motionIndex++) {
            lbl_1_bss_7D2C[playerIndex].motions[motionIndex] = fn_80030458(
                lbl_1_bss_7D2C[playerIndex].modelId,
                fn_80008DD0(lbl_1_data_74[playerIndex][motionIndex],
                    0x10000000U, 2));
            fn_8002C95C(lbl_1_bss_7D2C[playerIndex].modelId,
                lbl_1_bss_7D2C[playerIndex].motions[0]);
            fn_800269E8(lbl_1_bss_7D2C[playerIndex].modelId, 0x40000001U);
        }
        fn_800269E8(lbl_1_bss_7D2C[playerIndex].modelId, 1U);
    }
}

void fn_1_D0B0(void)
{
    HU3D_LIGHTID lightId;

    lightId = Hu3DGLightCreate(0.0f, 2000.0f, 1000.0f,
        0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightStaticSet(lightId, TRUE);
    Hu3DGLightInfinitytSet(lightId);
}

/* Creates the timer process and the score box and digits of the round timer, then resets the
 * timer color pulse. */
void fn_1_D144(void) {
    OMOBJ *obj;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_D37C);
    lbl_1_bss_6160.scoreBox = MgScoreBoxCreate(0x5A, 0x30);
    MgScoreBoxDispSet(lbl_1_bss_6160.scoreBox, 1);
    MgScoreBoxPosSet(lbl_1_bss_6160.scoreBox, 286.0f, 64.0f);
    lbl_1_bss_6160.mode = 0x4650;
    lbl_1_bss_6160.state = 0;
    lbl_1_bss_6160.scoreA = MgScoreCreate(DATANUM(0x9b0000, 63), DATANUM(0x9b0000, 63), 0);
    MgScoreUnitBankSet(lbl_1_bss_6160.scoreA, 0xA);
    MgScoreMaxDigitSet(lbl_1_bss_6160.scoreA, 1);
    MgScorePosSet(lbl_1_bss_6160.scoreA, 264.0f, 64.0f);
    MgScoreColorSet(lbl_1_bss_6160.scoreA, 0xFFU, 0xFFU, 0xFFU);
    lbl_1_bss_6160.scoreB = MgScoreCreate(DATANUM(0x9b0000, 63), -1, 1);
    MgScoreMaxDigitSet(lbl_1_bss_6160.scoreB, 2);
    MgScorePosSet(lbl_1_bss_6160.scoreB, 294.0f, 64.0f);
    MgScoreColorSet(lbl_1_bss_6160.scoreB, 0xFFU, 0xFFU, 0xFFU);
    MgScoreValueSet(lbl_1_bss_6160.scoreA, 0);
    MgScoreValueSet(lbl_1_bss_6160.scoreB, 5);
    fn_1_D750();
    lbl_1_bss_6160.colorInterval = 4;
    lbl_1_bss_6160.colorCountdown = 4;
    lbl_1_bss_6160.colorValue = 0xFF;
    lbl_1_bss_6160.colorIncreasing = 0;
    lbl_1_bss_6160.colorStep = 4;
}
