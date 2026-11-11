#ifndef REL_MICQUIZISHIDLL_GRID_H
#define REL_MICQUIZISHIDLL_GRID_H

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

void MicQuizResponseRun(s32 mode);

typedef struct QuizGridTileView {
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ; /* the game's code never reads this field */
} QuizGridTileView;

typedef struct QuizCameraViewRecord {
    f32 centerY;
    f32 zoom;
} QuizCameraViewRecord;

typedef struct QuizGridCell_s {
    s16 positionX;
    s16 positionY;
    s16 width;
    s16 height;
} QuizGridCell;

typedef struct QuizCharacterMotionView {
    u8 unknown_00[4]; /* No code names this field. */
    s32 characterId;
    s32 state;
    u8 unknown_0C[4]; /* No code names this field. */
    s16 modelId;
    s16 motions[5];
    u8 unknown_1C[4]; /* No code names this field. */
    f32 positionX;
    f32 positionY;
    f32 positionZ;
} QuizCharacterMotionView;

typedef struct QuizCharacterMotionData {
    s32 dataNumbers[5];
} QuizCharacterMotionData;

extern s16 lbl_1_bss_1024[0x24];
extern s32 lbl_1_bss_106C;
extern s16 lbl_1_bss_1190;
extern ANIMDATA *lbl_1_bss_1194[0x24];
extern s16 lbl_1_bss_1224;
extern OMOBJ *lbl_1_bss_1228;
extern OMOBJ *lbl_1_bss_122C;
extern s32 lbl_1_bss_1230;
extern QuizGridTileView lbl_1_bss_1234[0x24];
extern s32 lbl_1_bss_1594;
extern f32 lbl_1_bss_E3C;
extern s32 lbl_1_bss_E40[6];
extern s32 lbl_1_bss_E58;
extern ANIMDATA *lbl_1_bss_E5C;
extern s16 lbl_1_bss_E60;
extern s16 lbl_1_bss_E70[0xDA];
extern QuizGridCell lbl_1_bss_1070[];
extern s16 lbl_1_bss_E62;
extern Vec lbl_1_bss_E64;
extern HUPROCESS *lbl_1_bss_E38;

struct SoundChoice {
    s16 motionSound;
    s16 finishSound;
};

#endif
