/* Computer-player behavior: route tables, per-frame input, target choice, route
 * following, the piece-herding and other moves, and the play-area bounds test. */
#include "game/mg/actman.h"
#include "REL/m661dll/motion.h"
#include "REL/m661dll/player_actors.h"
typedef struct ModelSetupRecord {
    s16 modelId;
    HuVecF position;
    HuVecF shadowPosition;
    HuVecF rotation;
} ModelSetupRecord;
extern ModelSetupRecord lbl_1_bss_1E3C[0x71];
extern MotionControl lbl_1_bss_10[0x4E];

#include "dolphin/types.h"
#include "game/frand.h"
#include "REL/m661dll/system_state.h"
#include "REL/m661dll/routes.h"

BoardRouteGroup lbl_1_bss_31D0[16];

/* Selects a movement destination for the current player from board state. */
typedef struct SequenceChoiceState {
    SystemPadInput input;
    s8 groupSlot;
    s8 playerIndex;
    s8 character;
    s8 controllerPort;
    s8 group;
    s8 playerType;
    s8 choiceMode;
    s8 active;
    s8 selectedCharacter;
    /* Player data between character selection and position. */
    u8 unknown1D[0x17]; /* No code names this field. */
    Point3d position;
    Point3d previousPosition;
    Point3d rotation;
    s8 playerStatus[2];
    s16 currentValue;
    s16 previousValue;
    s16 pairedIndex;
    s16 previousPairedIndex;
    s16 targetState;
    s16 previousTarget;
    s32 padX;
    s32 padY;
    s32 stickX;
    s32 stickY;
    s16 inputPhase;
    s16 choicePhase;
    u8 unknown7C[2]; /* No code names this field. */
    s16 phase;
    Point3d movementTarget;
    /* Player data between movement target and chase settings. */
    u8 unknown8C[0xC]; /* No code names this field. */
    f32 chaseSpeed;
    f32 chaseRadius;
    u8 unknownA0[2]; /* No code names this field. */
    s16 target;
    s16 targetKind;
    s16 action;
    s16 targetIndex;
    s16 firstResetValue;
    s16 secondResetValue;
    s16 settleTimer;
    s16 forceStop;
    s16 selectedModel;
    s16 targetCount;
    s16 roundCount;
    s16 overrideChance;
} SequenceChoiceState;

typedef struct ChoiceTotals {
    s16 values[4];
} ChoiceTotals;

typedef struct ChoiceTable2 {
    s16 values[2];
} ChoiceTable2;

extern SequenceChoiceState lbl_1_bss_1C04[2];

#include <math.h>
#include "REL/m661dll/particle.h"

typedef HuVecF PlayerPosition;

s16 fn_1_20F90(SequenceChoiceState *object);
s16 fn_1_21F24(SequenceChoiceState *object, s16 side);

#include "humath.h"

void fn_1_238E0(SequenceChoiceState *state);

s32 fn_1_22C70(SequenceChoiceState *object);

typedef struct PlanePoint {
    f32 x;
    f32 z;
} PlanePoint;

typedef struct PlaneEdge {
    PlanePoint start;
    PlanePoint end;
} PlaneEdge;

#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"

#include "dolphin/mtx.h"
#include "string.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/main.h"
#include "game/pad.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/charman.h"
#include "game/process.h"
#include "game/hsfex.h"
#include "game/gamemes.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/mgdata.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/board/object.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/tutorial.h"
#include "game/board/masu.h"
#include "game/board/player.h"

s16 fn_1_24470(Point3d position);

typedef struct BoardPointXZ {
    f32 x;
    f32 z;
} BoardPointXZ;

extern SystemInputState lbl_1_bss_196C;

#include "REL/m661dll.h"

void fn_1_243E8(SequenceChoiceState *state);

void fn_1_C8D4(void)
{
    s16 pathOffsets[4][2] = { { 1, 5 }, { 6, 4 }, { 10, 3 }, { 64, 1 } };
    s16 pathVariants[4] = { 30, 20, 0, 0 };
    s16 route;
    s16 dir;
    s16 point;
    s16 group;
    s16 direction;
    s16 index;
    for (group = 0; group < 2; group++) {
        if (lbl_1_bss_1C04[group].playerType != 0) {
            s16 choice = lbl_1_bss_1C04[group].choiceMode;
            lbl_1_bss_1C04[group].roundCount =
                pathOffsets[choice][0] +
                frandmod(pathOffsets[choice][1]);
            lbl_1_bss_1C04[group].overrideChance = pathVariants[choice];
        }
    }

    lbl_1_bss_196C.initialGridStep.x = 387.5f;
    lbl_1_bss_196C.initialGridStep.z = 387.5f;
    for (group = 0, index = 0; group < 4; group++) {
        for (direction = 0; direction < 4; direction++, index++) {
            lbl_1_bss_196C.initialPositions[index].x = group * lbl_1_bss_196C.initialGridStep.x - 775.0f;
            lbl_1_bss_196C.initialPositions[index].z = direction * lbl_1_bss_196C.initialGridStep.z - 775.0f;
        }
    }
    lbl_1_bss_196C.secondaryGridStep.x = 775.0f;
    lbl_1_bss_196C.secondaryGridStep.z = 775.0f;
    for (group = 0, index = 0; group < 2; group++) {
        for (direction = 0; direction < 2; direction++, index++) {
            lbl_1_bss_196C.secondaryPositions[index].x = group * lbl_1_bss_196C.secondaryGridStep.x - 775.0f;
            lbl_1_bss_196C.secondaryPositions[index].z = direction * lbl_1_bss_196C.secondaryGridStep.z - 775.0f;
        }
    }
    lbl_1_bss_196C.specialExpansion.x = 400.0f;
    lbl_1_bss_196C.specialExpansion.z = 400.0f;
    lbl_1_bss_196C.specialBounds[0].x = -775.0f;
    lbl_1_bss_196C.specialBounds[0].z = -200.0f;
    lbl_1_bss_196C.specialBounds[1].x = 775.0f - lbl_1_bss_196C.specialExpansion.x;
    lbl_1_bss_196C.specialBounds[1].z = -200.0f;

    memset(lbl_1_bss_31D0, 0, sizeof(BoardRouteGroup) * 16);

    dir = 0;
    point = 0;
    route = 0;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    dir = 0;
    point = 0;
    route = 1;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    dir = 0;
    point = 0;
    route = 2;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 3;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 4;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 5;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 6;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 720.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -658.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 600.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 7;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 600.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 8;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 9;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 10;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -420.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 11;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -420.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 12;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 13;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 14;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 15;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    dir = 1;
    point = 0;
    route = 0;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    dir = 1;
    point = 0;
    route = 1;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 200.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -200.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    dir = 1;
    point = 0;
    route = 2;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 3;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 60.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 4;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 5;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 300.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 6;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -720.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -658.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -600.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 7;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = -600.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 8;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 9;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 10;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 420.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 620.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -620.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 11;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 420.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 650.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 750.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 560.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 700.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 7.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 350.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 12;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 13;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 14;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = -400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
    point = 0;
    route = 15;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 400.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 100.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 721.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 10.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = 0;
    point++;
    lbl_1_bss_31D0[route].directions[dir].positions[point].x = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].y = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].positions[point].z = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].speeds[point] = 0.0f;
    lbl_1_bss_31D0[route].directions[dir].waitFrames[point] = -1;
    point++;
}

void fn_1_20D74(void)
{
    s16 index;
    SequenceChoiceState *state;

    fn_1_23248();
    for (index = 0; index < 2; index++) {
        state = &lbl_1_bss_1C04[index];
        if (state->playerType != 0 && (state->active == 0 || state->active == 2)) {
            state->padX = 0;
            state->padY = 0;
            state->stickX = 0;
            state->stickY = 0;
            switch (state->inputPhase) {
            case 0:
                state->inputPhase++;
                /* fall through */
            case 1:
                fn_1_243E8(state);
                break;
            }
        }
    }
}

s16 fn_1_20E50(SequenceChoiceState *move, s16 advance)
{
    HuVecF target;
    s16 result = 0;
    s16 finished = 0;

    switch (move->phase) {
    case 0:
        fn_8007E818(lbl_1_bss_1960.players[move->playerIndex]);
        move->phase++;
        break;
    case 1:
        target = move->movementTarget;
        finished = fn_8007E8B4(
            lbl_1_bss_1960.players[move->playerIndex], &target,
            move->chaseSpeed, move->chaseRadius);
        if (finished != 0) {
            if (advance != 0) {
                move->phase++;
            } else {
                result = 1;
            }
        }
        break;
    case 2:
        fn_8007E888(lbl_1_bss_1960.players[move->playerIndex]);
        move->phase = 0;
        result = 1;
        break;
    }
    return result;
}

/* Computer move that drives a piece out through this player's side exit (x -800 for
 * the first player, 800 for the second): walk to the home column and pick a roaming
 * piece at least 350 units nearer the exit whose approach point is in the play area,
 * then get behind it and follow it until it stops roaming or the angle passes 100
 * degrees. With two computer players and the target timer past 60 it stops in place
 * instead; returns 1 when the move ends. */
s16 fn_1_20F90(SequenceChoiceState *state) {
    struct ModelSetupRecord *model;
    Point3d candidatePosition;
    f32 approachAngle;
    f32 leftBoundary;
    f32 rightBoundary;
    s16 finished = 0;
    s16 motionIndex;
    s16 movementResult = 0;
    Point3d candidatePositions[2] = { { 700.0f, 0.0f, 0.0f }, { -700.0f, 0.0f, 0.0f } };
    Point3d candidateOffsets[2] = { { -800.0f, 0.0f, 0.0f }, { 800.0f, 0.0f, 0.0f } };

    if ((lbl_1_bss_1C04[0].playerType != 0) && (lbl_1_bss_1C04[1].playerType != 0) && (state->targetState > 0x3C)) {
        state->action = 5;
    }
    switch (state->action) {
    case 0:                                         /* switch 1 */
    {

        s16 modelIndex;

        state->movementTarget = candidatePositions[state->playerIndex];
        state->movementTarget.z = (f32) state->position.z;
        state->chaseSpeed = 10.0f;
        state->chaseRadius = 10.1f;
        movementResult = fn_1_20E50(state, 1);
        state->selectedModel = -1;
        for (modelIndex = 0x23, motionIndex = 0;
             modelIndex <= 0x61;
             modelIndex++, motionIndex++) {
            model = &lbl_1_bss_1E3C[modelIndex];
            if (state->playerIndex == 0) {
                leftBoundary = 50.0f + (300.0f + model->position.x);
                rightBoundary = state->position.x;
            } else {
                leftBoundary = state->position.x;
                rightBoundary = (model->position.x - 300.0f) - 50.0f;
            }
            if ((leftBoundary < rightBoundary) && (lbl_1_bss_10[motionIndex].mode == 0)) {
                candidateOffsets[state->playerIndex].z = model->position.z;
                if (candidateOffsets[state->playerIndex].z > 200.0f) {
                    candidateOffsets[state->playerIndex].z = 200.0f;
                }
                if (candidateOffsets[state->playerIndex].z < -200.0f) {
                    candidateOffsets[state->playerIndex].z = -200.0f;
                }
                candidatePosition.x = model->position.x - candidateOffsets[state->playerIndex].x;
                candidatePosition.z = model->position.z - candidateOffsets[state->playerIndex].z;
                approachAngle = (f32) (180.0 * (atan2((f64) candidatePosition.x, (f64) candidatePosition.z) / M_PI));
                candidatePosition.x = (f32) ((f64) model->position.x + (320.0 * sin((M_PI * (f64) approachAngle) / 180.0)));
                candidatePosition.z = (f32) ((f64) model->position.z +
                    (320.0 * cos((M_PI * (f64) approachAngle) / 180.0)));
                if (fn_1_24470(candidatePosition) == 0) {
                    state->selectedModel = modelIndex;
                    break;
                }
            }
        }
        if (state->selectedModel != -1) {
            state->action = 2;
        }
        break;
    }
    case 2:                                         /* switch 1 */
    {

        model = &lbl_1_bss_1E3C[state->selectedModel];
        candidateOffsets[state->playerIndex].z = model->position.z;
        if (candidateOffsets[state->playerIndex].z > 200.0f) {
            candidateOffsets[state->playerIndex].z = 200.0f;
        }
        if (candidateOffsets[state->playerIndex].z < -200.0f) {
            candidateOffsets[state->playerIndex].z = -200.0f;
        }
        candidatePosition.x = model->position.x - candidateOffsets[state->playerIndex].x;
        candidatePosition.z = model->position.z - candidateOffsets[state->playerIndex].z;
        approachAngle = (f32)(180.0 * (atan2((f64)candidatePosition.x, (f64)candidatePosition.z) / M_PI));
        leftBoundary = state->playerIndex == 0 ? 350.0f : -350.0f;
        candidatePosition.x = model->position.x + leftBoundary;
        candidatePosition.z = (f32)((f64)model->position.z +
            (300.0 * cos((M_PI * (f64)approachAngle) / 180.0)));
        if (fn_1_24470(candidatePosition) == 0) {
            state->movementTarget = candidatePosition;
            state->chaseSpeed = 10.0f;
            state->chaseRadius = 10.1f;
            movementResult = fn_1_20E50(state, 1);
            if (movementResult != 0) {
                state->action += 1;
            }
        } else {
            finished = 1;
        }
        break;
    }
    case 3:                                         /* switch 1 */
    {
        f32 playerDirection;
        model = &lbl_1_bss_1E3C[state->selectedModel];
        candidateOffsets[state->playerIndex].z = model->position.z;
        if (candidateOffsets[state->playerIndex].z > 200.0f) {
            candidateOffsets[state->playerIndex].z = 200.0f;
        }
        if (candidateOffsets[state->playerIndex].z < -200.0f) {
            candidateOffsets[state->playerIndex].z = -200.0f;
        }
        candidatePosition.x = model->position.x - candidateOffsets[state->playerIndex].x;
        candidatePosition.z = model->position.z - candidateOffsets[state->playerIndex].z;
        candidatePosition.z = candidatePosition.z == 0.0f ? 1.0f : candidatePosition.z;
        approachAngle = (f32) (180.0 * (atan2((f64) candidatePosition.x,
            (f64)candidatePosition.z) / M_PI));
        candidatePosition.x = (f32) ((f64) model->position.x + (10.0 * sin((M_PI * (f64) approachAngle) / 180.0)));
        candidatePosition.z = (f32) ((f64) model->position.z + (10.0 * cos((M_PI * (f64) approachAngle) / 180.0)));
        state->movementTarget = candidatePosition;
        state->chaseSpeed = 10.0f;
        state->chaseRadius = 10.1f;
        candidatePosition.x = state->position.x - model->position.x;
        candidatePosition.z = state->position.z - model->position.z;
        playerDirection = (f32) (180.0 * (atan2((f64) candidatePosition.x, (f64) candidatePosition.z) / M_PI));
        if (((approachAngle > 90.0f) && (playerDirection < -90.0f)) || ((approachAngle < -90.0f) && (playerDirection > 90.0f))) {
            approachAngle = approachAngle < 0.0f ? 180.0f + approachAngle : 180.0f - approachAngle;
            playerDirection = playerDirection < 0.0f ? 180.0f + playerDirection : 180.0f - playerDirection;
            approachAngle = approachAngle + playerDirection > 0.0f
                ? approachAngle + playerDirection : -1.0f * (approachAngle + playerDirection);
        } else {
            approachAngle = approachAngle - playerDirection > 0.0f
                ? approachAngle - playerDirection : -1.0f * (approachAngle - playerDirection);
        }
        if ((lbl_1_bss_10[state->selectedModel - 0x23].mode != 0) || (approachAngle > 100.0f)) {
            state->movementTarget = state->position;
        }
        movementResult = fn_1_20E50(state, 1);
        if (movementResult != 0) {
            state->action += 1;
        }
        break;
    }
    case 4:                                         /* switch 1 */
        finished = 1;
        break;
    case 5:                                         /* switch 1 */
    {
        state->movementTarget = state->position;
        state->chaseSpeed = 10.0f;
        state->chaseRadius = 10.1f;
        movementResult = fn_1_20E50(state, 1);
        if (movementResult != 0) {
            state->action = 4;
        }
        break;
    }
    }
    return finished;
}

s16 fn_1_21F24(SequenceChoiceState *object, s16 side)
{
    s16 result = 0;
    s16 blocked;
    s16 moveFinished = 0;
    s16 retryCount = 0;
    PlayerPosition delta;
    f32 distance;
    s16 targetLimits[2] = { 60, 30 };
    switch (object->action) {
    case 0:
        object->chaseSpeed = 10.0f;
        object->chaseRadius = 10.1f;
        object->action++;
        while (1) {
            blocked = 0;
            object->movementTarget.x = fn_1_3D04(1000.0f) - 500.0f;
            object->movementTarget.y = 0.0f;
            object->movementTarget.z = fn_1_3D04(1500.0f) - 750.0f;

            delta.x = object->movementTarget.x - object->position.x;
            delta.z = object->movementTarget.z - object->position.z;
            distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
            if (distance < 150.0f) {
                blocked = 1;
            }
            if ((side != 0) && (blocked == 0)) {
                delta.x = object->movementTarget.x - object->position.x;
                delta.z = object->movementTarget.z - object->position.z;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance > 300.0f) {
                    blocked = 1;
                }
            }
            if (blocked == 0) {
                delta.x = object->movementTarget.x - 775.0f;
                delta.x = object->movementTarget.z - 200.0f;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance < 300.0f) {
                    blocked = 1;
                }
            }
            if (blocked == 0) {
                delta.x = object->movementTarget.x - 775.0f;
                delta.x = 200.0f + object->movementTarget.z;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance < 300.0f) {
                    blocked = 1;
                }
            }
            if (blocked == 0) {
                delta.x = 775.0f + object->movementTarget.x;
                delta.x = object->movementTarget.z - 200.0f;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance < 300.0f) {
                    blocked = 1;
                }
            }
            if (blocked == 0) {
                delta.x = 775.0f + object->movementTarget.x;
                delta.x = 200.0f + object->movementTarget.z;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance < 300.0f) {
                    blocked = 1;
                }
            }
            if (blocked == 0) {
                break;
            }
            if (retryCount++ <= 0x1F4) {
                continue;
            }
            object->movementTarget = object->position;
            object->action = 2;
            break;
        }
        break;
    case 1:
        if (object->targetState > targetLimits[side]) {
            object->movementTarget = object->position;
        }
        moveFinished = fn_1_20E50(object, 1);
        if ((s16)moveFinished != 0) {
            object->action++;
        }
        break;
    case 2:
        result = 1;
        break;
    }
    return result;
}

s32 fn_1_22C70(SequenceChoiceState *object)
{
    MovementPath *playerState;
    f32 distanceX;
    f32 distanceZ;
    PlayerPosition distance;
    s16 region;
    s32 result = 0;
    s16 targetIndex;
    s16 moveFinished = 0;
    s16 timings[4][3] = { { 10, 13, 2 }, { 6, 9, 2 }, { 5, 8, 2 }, { 4, 7, 2 } };

    if (object->target < 16) {
        playerState = &lbl_1_bss_31D0[object->target].directions[object->playerIndex];
        targetIndex = object->firstResetValue;
        if (playerState->waitFrames[targetIndex] != -1) {
            switch (object->action) {
            case 0:
                object->movementTarget = playerState->positions[targetIndex];
                object->chaseSpeed = playerState->speeds[targetIndex];
                object->chaseRadius = object->chaseSpeed += 0.1f;
                object->secondResetValue = 0;
                object->settleTimer = 0;
                object->action++;
                /* fall through */
            case 1:
                moveFinished = fn_1_20E50(object, 1);
                if ((s16)moveFinished == 0) {
                    if ((object->position.x - object->previousPosition.x) > 0.0f) {
                        distanceX = object->position.x - object->previousPosition.x;
                    } else {
                        distanceX = -1.0f *
                                    (object->position.x - object->previousPosition.x);
                    }
                    distance.x = distanceX;
                    if ((object->position.z - object->previousPosition.z) > 0.0f) {
                        distanceZ = object->position.z - object->previousPosition.z;
                    } else {
                        distanceZ = -1.0f *
                                    (object->position.z - object->previousPosition.z);
                    }
                    distance.z = distanceZ;
                    if (distance.x < 0.5f && distance.z < 0.5f) {
                        object->settleTimer++;
                    } else {
                        object->settleTimer = 0;
                    }
                    if (object->settleTimer > 5) {
                        object->movementTarget = object->position;
                    }
                } else {
                    object->action = 0;
                    object->firstResetValue++;
                    switch (object->target) {
                    case 0:
                    case 1:
                        region = 0;
                        break;
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        region = 1;
                        break;
                    case 6:
                    case 7:
                        region = 2;
                        break;
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                        region = 3;
                        break;
                    default:
                        region = -1;
                        break;
                    }
                    if (region != -1 &&
                        object->firstResetValue >= timings[region][0] &&
                        lbl_1_bss_196C.playerChoiceCounts[object->playerIndex] < timings[region][2]) {
                        object->firstResetValue = timings[region][1];
                    }
                    if (object->previousTarget >= object->roundCount) {
                        object->firstResetValue = timings[region][1];
                    }
                }
                break;
            case 2:
                if (++object->secondResetValue >
                    playerState->waitFrames[targetIndex]) {
                    object->action = 0;
                    object->firstResetValue++;
                }
                break;
            }
            if (lbl_1_bss_1C04[0].playerType != 0 && lbl_1_bss_1C04[1].playerType != 0 && object->targetState > 0x1E) {
                object->forceStop = 1;
                result = 1;
            }
        } else {
            result = 1;
        }
    } else {
        switch (object->target) {
        case 16:
            moveFinished = fn_1_20F90(object);
            break;
        case 17:
            moveFinished = fn_1_21F24(object, 0);
            break;
        case 18:
            moveFinished = fn_1_21F24(object, 1);
            break;
        default:
            moveFinished = 1;
            break;
        }
        if ((s16)moveFinished != 0) {
            result = 1;
        }
    }
    return result;
}

void fn_1_23248(void)
{
    s16 objectId;
    s16 denominator;
    s16 objectIndex;
    s16 index;
    MotionControl *state;
    HuVecF inputPosition;
    HuVecF expandedPosition;

    for (index = 0; index < 16; index++) {
        lbl_1_bss_196C.boardChoiceCounts[index] = 0;
    }
    for (index = 0; index < 4; index++) {
        lbl_1_bss_196C.targetChoiceCounts[index] = 0;
    }
    for (index = 0; index < 2; index++) {
        lbl_1_bss_196C.playerChoiceCounts[index] = 0;
    }

    denominator = (0x40 - lbl_1_bss_1C04[0].previousTarget) + lbl_1_bss_1C04[1].previousTarget;
    objectId = 0x23;
    objectIndex = 0;
    for (; objectId <= 0x62; objectId++, objectIndex++) {
        ModelSetupRecord *bounds = &lbl_1_bss_1E3C[objectId];
        state = &lbl_1_bss_10[objectIndex];

        if (state->mode == 0) {
            for (index = 0; index < 16; index++) {
                inputPosition.x = lbl_1_bss_196C.initialPositions[index].x;
                inputPosition.z = lbl_1_bss_196C.initialPositions[index].z;
                expandedPosition.x = lbl_1_bss_196C.initialPositions[index].x +
                                     lbl_1_bss_196C.initialGridStep.x;
                expandedPosition.z = lbl_1_bss_196C.initialPositions[index].z +
                                     lbl_1_bss_196C.initialGridStep.z;
                if (inputPosition.x <= bounds->position.x && inputPosition.z <= bounds->position.z &&
                    expandedPosition.x >= bounds->position.x &&
                    expandedPosition.z >= bounds->position.z) {
                    lbl_1_bss_196C.boardChoiceCounts[index]++;
                }
            }
            for (index = 0; index < 2; index++) {
                inputPosition.x = lbl_1_bss_196C.specialBounds[index].x;
                inputPosition.z = lbl_1_bss_196C.specialBounds[index].z;
                expandedPosition.x = lbl_1_bss_196C.specialBounds[index].x +
                                     lbl_1_bss_196C.specialExpansion.x;
                expandedPosition.z = lbl_1_bss_196C.specialBounds[index].z +
                                     lbl_1_bss_196C.specialExpansion.z;
                if (inputPosition.x <= bounds->position.x && inputPosition.z <= bounds->position.z &&
                    expandedPosition.x >= bounds->position.x &&
                    expandedPosition.z >= bounds->position.z) {
                    lbl_1_bss_196C.playerChoiceCounts[index]++;
                }
            }
        }
    }

    lbl_1_bss_196C.targetChoiceCounts[0] =
        lbl_1_bss_196C.boardChoiceCounts[0] + lbl_1_bss_196C.boardChoiceCounts[1] +
        lbl_1_bss_196C.boardChoiceCounts[4] + lbl_1_bss_196C.boardChoiceCounts[5];
    lbl_1_bss_196C.targetChoiceCounts[1] =
        lbl_1_bss_196C.boardChoiceCounts[2] + lbl_1_bss_196C.boardChoiceCounts[3] +
        lbl_1_bss_196C.boardChoiceCounts[6] + lbl_1_bss_196C.boardChoiceCounts[7];
    lbl_1_bss_196C.targetChoiceCounts[2] =
        lbl_1_bss_196C.boardChoiceCounts[8] + lbl_1_bss_196C.boardChoiceCounts[9] +
        lbl_1_bss_196C.boardChoiceCounts[12] + lbl_1_bss_196C.boardChoiceCounts[13];
    lbl_1_bss_196C.targetChoiceCounts[3] =
        lbl_1_bss_196C.boardChoiceCounts[10] + lbl_1_bss_196C.boardChoiceCounts[11] +
        lbl_1_bss_196C.boardChoiceCounts[14] + lbl_1_bss_196C.boardChoiceCounts[15];

    for (index = 0; index < 16; index++) {
        lbl_1_bss_196C.playerOverlapRatios[index] = 100.0f *
            ((f32)lbl_1_bss_196C.boardChoiceCounts[index] / (f32)denominator);
    }
    for (index = 0; index < 4; index++) {
        lbl_1_bss_196C.regionOverlapRatios[index] = 100.0f *
            ((f32)lbl_1_bss_196C.targetChoiceCounts[index] / (f32)denominator);
    }
    for (index = 0; index < 2; index++) {
        lbl_1_bss_196C.specialOverlapRatios[index] = 100.0f *
            ((f32)lbl_1_bss_196C.playerChoiceCounts[index] / (f32)denominator);
    }
}

void fn_1_238E0(SequenceChoiceState *state)
{
    s16 choices12[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    s16 choices8[8] = { 2, 3, 4, 5, 8, 9, 10, 11 };
    s16 choices6a[6] = { 0, 1, 2, 3, 6, 7 };
    s16 choices6b[6] = { 0, 2, 4, 6, 8, 10 };
    s16 choices6c[6] = { 1, 3, 5, 7, 9, 11 };
    ChoiceTotals totals;
    s16 choices3a[3] = { 0, 2, 6 };
    s16 choices3b[3] = { 1, 3, 7 };
    ChoiceTable2 previousTotals;
    s16 choices2a[2] = { 2, 8 };
    s16 choices2b[2] = { 3, 9 };
    s16 choices3c[3] = { 4, 8, 10 };
    s16 choices3d[3] = { 5, 9, 11 };
    s16 selectionRule;
    s16 randomValue;

    state->target = -1;
    if (state->choiceMode == 0) {
        state->target = 0x10;
    } else if (state->choiceMode == 1) {
        if (state->targetCount == 0) {
            state->target = choices8[frandmod(8)];
        } else {
            randomValue = frandmod(0x64);
            if (randomValue < 0x32 &&
                state->roundCount - state->previousTarget >= 5) {
                state->target = choices8[frandmod(8)];
            } else {
                state->target = 0x10;
            }
        }
    } else if (state->targetCount == 0) {
        if (state->choiceMode == 2) {
            if (lbl_1_bss_1C04[state->playerIndex ^ 1].targetKind == 2) {
                state->target = choices12[frandmod(0xC)];
            } else if (lbl_1_bss_1C04[state->playerIndex ^ 1].targetKind == 0) {
                state->target = choices6c[frandmod(6)];
            } else {
                state->target = choices6b[frandmod(6)];
            }
        } else if (lbl_1_bss_1C04[state->playerIndex ^ 1].targetKind == 2) {
            state->target = choices6a[frandmod(6)];
        } else if (lbl_1_bss_1C04[state->playerIndex ^ 1].targetKind == 0) {
            state->target = choices3b[frandmod(3)];
        } else {
            state->target = choices3a[frandmod(3)];
        }
    } else {
        if (lbl_1_bss_196C.frameCounter <= 0x366) {
            if (state->playerIndex == 0) {
                totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[0] +
                    lbl_1_bss_196C.boardChoiceCounts[8] + lbl_1_bss_196C.boardChoiceCounts[9];
                totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[1] +
                    lbl_1_bss_196C.boardChoiceCounts[11] + lbl_1_bss_196C.boardChoiceCounts[10];
            } else {
                totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[2] +
                    lbl_1_bss_196C.boardChoiceCounts[4] + lbl_1_bss_196C.boardChoiceCounts[5];
                totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[3] +
                    lbl_1_bss_196C.boardChoiceCounts[6] + lbl_1_bss_196C.boardChoiceCounts[7];
            }
            selectionRule = 0;
            previousTotals.values[0] = totals.values[0];
            previousTotals.values[1] = totals.values[1];
            if (totals.values[0] > totals.values[1]) {
                state->target = choices3a[frandmod(3)];
            } else {
                state->target = choices3b[frandmod(3)];
            }
        }
        if (state->playerIndex == 0) {
            totals.values[0] = lbl_1_bss_196C.boardChoiceCounts[0] + lbl_1_bss_196C.boardChoiceCounts[1];
            totals.values[1] = lbl_1_bss_196C.boardChoiceCounts[2] + lbl_1_bss_196C.boardChoiceCounts[3];
        } else {
            totals.values[0] = lbl_1_bss_196C.boardChoiceCounts[12] + lbl_1_bss_196C.boardChoiceCounts[13];
            totals.values[1] = lbl_1_bss_196C.boardChoiceCounts[14] + lbl_1_bss_196C.boardChoiceCounts[15];
        }
        if (totals.values[0] > 6 || totals.values[1] > 6) {
            selectionRule = 1;
            previousTotals.values[0] = totals.values[0];
            previousTotals.values[1] = totals.values[1];
            if (totals.values[0] > totals.values[1]) {
                state->target = choices3c[frandmod(3)];
            } else {
                state->target = choices3d[frandmod(3)];
            }
        }
        if (state->playerIndex == 0) {
            totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[0];
            totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[1];
        } else {
            totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[2];
            totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[3];
        }
        if (totals.values[0] > 8 || totals.values[1] > 8) {
            selectionRule = 2;
            previousTotals.values[0] = totals.values[0];
            previousTotals.values[1] = totals.values[1];
            if (totals.values[0] > totals.values[1]) {
                state->target = choices2a[frandmod(2)];
            } else {
                state->target = choices2b[frandmod(2)];
            }
        }
        if (lbl_1_bss_196C.frameCounter >= 0x3FC &&
            (totals.values[0] > 3 || totals.values[1] > 3)) {
            selectionRule = 3;
            previousTotals.values[0] = totals.values[0];
            previousTotals.values[1] = totals.values[1];
            if (totals.values[0] > totals.values[1]) { state->target = 0xA; }
            else { state->target = 0xB; }
        }
        if (lbl_1_bss_196C.frameCounter <= 0x366) {
            if (state->playerIndex == 0) {
                totals.values[0] = lbl_1_bss_196C.boardChoiceCounts[0] +
                    lbl_1_bss_196C.boardChoiceCounts[1] + lbl_1_bss_196C.boardChoiceCounts[2] + lbl_1_bss_196C.boardChoiceCounts[3];
            } else {
                totals.values[0] = lbl_1_bss_196C.boardChoiceCounts[12] +
                    lbl_1_bss_196C.boardChoiceCounts[13] + lbl_1_bss_196C.boardChoiceCounts[14] + lbl_1_bss_196C.boardChoiceCounts[15];
            }
            if (totals.values[0] < 5) {
                selectionRule = 4;
                previousTotals.values[0] = totals.values[0];
                previousTotals.values[1] = totals.values[1];
                if (state->playerIndex == 0) {
                    totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[0] +
                        lbl_1_bss_196C.boardChoiceCounts[8] + lbl_1_bss_196C.boardChoiceCounts[9];
                    totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[1] +
                        lbl_1_bss_196C.boardChoiceCounts[10] + lbl_1_bss_196C.boardChoiceCounts[11];
                } else {
                    totals.values[0] = lbl_1_bss_196C.targetChoiceCounts[2] +
                        lbl_1_bss_196C.boardChoiceCounts[4] + lbl_1_bss_196C.boardChoiceCounts[5];
                    totals.values[1] = lbl_1_bss_196C.targetChoiceCounts[3] +
                        lbl_1_bss_196C.boardChoiceCounts[6] + lbl_1_bss_196C.boardChoiceCounts[7];
                }
                if (totals.values[0] > totals.values[1]) { state->target = 0; }
                else { state->target = 1; }
            }
        }
        if (state->choiceMode == 2 && state->roundCount - state->previousTarget <= 3) {
            state->target = 0x10;
        }
        if (lbl_1_bss_196C.playerChoiceCounts[state->playerIndex] > 3 ||
            lbl_1_bss_196C.frameCounter > 0x438 || state->target == -1) {
            state->target = 0x10;
        }
    }
    if (state->forceStop != 0) {
        state->forceStop = 0;
        state->target = 0x12;
    }
    randomValue = frandmod(0x64);
    if (state->previousTarget >= state->roundCount || randomValue < state->overrideChance) {
        state->target = 0x11;
    }
    switch (state->target) {
        case 0: case 2: case 4: case 6: case 8: case 10: case 12: case 14:
            state->targetKind = 0;
            break;
        case 1: case 3: case 5: case 7: case 9: case 11: case 13: case 15:
            state->targetKind = 1;
            break;
        default:
            state->targetKind = 2;
            break;
    }
    state->firstResetValue = 0;
    state->action = 0;
    state->secondResetValue = 0;
    state->targetState = 0;
    state->targetCount++;
}

/* Computer-player choice loop: chooses a new target with fn_1_238E0, then runs
 * fn_1_22C70 each frame until it reports done, and chooses again. */
void fn_1_243E8(SequenceChoiceState *self) {
    s32 result = 0;

    switch (self->choicePhase) {
    case 0:
        fn_1_238E0(self);
        ++self->choicePhase;
        break;
    case 1:
        result = fn_1_22C70(self);
        if ((s16)result != 0) {
            self->choicePhase = 0;
        }
        break;
    }
}

s16 fn_1_24470(HuVecF position)
{
    PlaneEdge edges[4];
    f32 distanceSquared;
    HuVecF delta;
    f32 contactRadiusSquared;
    f32 amount;
    HuVecF projected;
    f32 centerDistanceSquared;
    f32 projectionNumerator;
    s16 outside;
    s16 edgeIndex;

    outside = 0;
    {
    HuVecF signs[2] = {{1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 1.0f}};
    HuVecF side;

    edges[0].start.x = -600.0f;
    edges[0].start.z = -800.0f;
    edges[0].end.x = -800.0f;
    edges[0].end.z = -600.0f;
    edges[1].start.x = 600.0f;
    edges[1].start.z = 800.0f;
    edges[1].end.x = -800.0f;
    edges[1].end.z = -600.0f;
    edges[2].start.x = -1160.0f;
    edges[2].start.z = 860.0f;
    edges[3].start.x = 1160.0f;
    edges[3].start.z = 860.0f;
    contactRadiusSquared = 500.0f;
    contactRadiusSquared *= contactRadiusSquared;
    edgeIndex = 0;
    for (; edgeIndex < 2; edgeIndex++) {
        delta.x = edges[edgeIndex].start.x - edges[edgeIndex].start.z;
        delta.z = edges[edgeIndex].end.x - edges[edgeIndex].end.z;
        distanceSquared = delta.x * delta.x + delta.z * delta.z;
        projectionNumerator = -1.0f *
                              (delta.x * (edges[edgeIndex].start.x - position.x) +
                                delta.z * (edges[edgeIndex].end.x - position.z));
        amount = distanceSquared ? projectionNumerator / distanceSquared : 0.0f;
        projected.x = delta.x * amount + edges[edgeIndex].start.x;
        projected.z = delta.z * amount + edges[edgeIndex].end.x;
        delta.x = position.x - projected.x;
        delta.z = position.z - projected.z;
        side.x = 0.0f != delta.x
                    ? (delta.x > 0.0f ? 1.0f : -1.0f)
                    : signs[edgeIndex].x;
        side.z = 0.0f != delta.z
                    ? (delta.z > 0.0f ? 1.0f : -1.0f)
                    : signs[edgeIndex].z;
        if (side.x != signs[edgeIndex].x ||
            side.z != signs[edgeIndex].z) {
            outside = 1;
        }
    }
    }

    edgeIndex = 2;
    for (; edgeIndex < 4; edgeIndex++) {
        delta.x = position.x - edges[edgeIndex].start.x;
        delta.z = position.z - edges[edgeIndex].start.z;
        centerDistanceSquared = delta.x * delta.x + delta.z * delta.z;
        if (centerDistanceSquared < contactRadiusSquared) {
            outside = 1;
        }
    }
    if (position.x > 780.0f || position.x < -780.0f ||
        position.z > 780.0f || position.z < -780.0f) {
        outside = 1;
    }
    return outside;
}
