/* Declarations the micquizishidll sources share: their types, the data one file defines and
 * others use, and one prototype for each function one file defines and others call. */
#ifndef REL_MICQUIZISHIDLL_H
#define REL_MICQUIZISHIDLL_H

#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "math.h"
#include "humath.h"
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
#include "game/printfunc.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "game/process.h"
#include "game/window.h"
#include "game/mic.h"
#include "datadir_enum.h"
#include "REL/micquiz/quiz.h"

typedef struct QuizGridTileView {
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    f32 velocityX;
    f32 velocityY;
    f32 unk14; /* The game's code never reads this field. */
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
    s32 unk04; /* The game's code never reads this field. */
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

struct SoundChoice {
    s16 motionSound;
    s16 finishSound;
};

typedef struct MicQuizRecord {
    s32 motionType; /* Motion type of the object on this place; minus one when free. */
    s32 objectIndex; /* Index of the object on this place. */
    s32 unk08; /* The game's code never reads this field. */
} MicQuizRecord;

typedef struct MicQuizRecordGroup {
    MicQuizRecord entries[4];
} MicQuizRecordGroup;

typedef struct MicquizCameraRange {
    f32 width;
    f32 height;
} MicquizCameraRange;

typedef struct MicquizObjectWork_s {
    s32 initialFrames;
    s32 remainingFrames;
    f32 animationSpeed;
    HuVecF savedPosition;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    s32 motionType;
    s32 currentMotion;
    s32 state;
    s32 objectIndex;
    s16 modelId;
    s16 motionIds[4];
} MicquizObjectWork;

typedef struct QuizPairEntry {
    s32 slotIndex;
    s32 partnerIndex;
    s32 selected;
} QuizPairEntry;

typedef struct QuizCharacterModel {
    u8 unknown_00[4]; /* No code names this field. */
    s32 characterId;
    s32 state;
    u8 unknown_0C[4]; /* No code names this field. */
    s16 modelId;
    s16 motions[3];
    f32 positionX;
    f32 positionY;
    f32 positionZ;
} QuizCharacterModel;

typedef struct QuizMenuCharacterMotionData {
    s32 dataNumbers[3];
} QuizMenuCharacterMotionData;

typedef struct QuizCharacterEntry {
    s32 unknown_00;
    s32 voiceId;
    s32 unk08; /* The game's code never reads this field. */
} QuizCharacterEntry;

typedef struct Rectangle {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} Rectangle;

typedef struct TexturePair {
    ANIMDATA *animation[2];
} TexturePair;

typedef struct QuizSelectionTable {
    QuizPairEntry entries[20];
    u8 unknown_F0[4]; /* No code names this field. */
} QuizSelectionTable;

typedef struct QuizModelList {
    s16 models[20];
    u8 unknown_28[4]; /* No code names this field. */
} QuizModelList;

typedef struct QuizObjectStorage {
    OMOBJ *object;
    u8 unknown_04[4]; /* No code names this field. */
} QuizObjectStorage;

typedef struct QuizParticleWork {
    Vec position;
    Vec origin;
    Vec offset;
    Vec velocity;
    f32 velocityScale;
    f32 angle;
    s32 frame;
    Vec rotation;
    Vec scale;
    s32 side;
    s32 unk58; /* The game's code never reads this field. */
    s32 positionIndex;
    s32 state;
    s16 model;
    u8 unk66[0x2]; /* No code names this field. */
    s32 active;
} QuizParticleWork;

/* Three 32-bit fields consumed by fn_1_1FB04 and fn_1_22058. */
typedef struct {
    s32 itemCount;
    s32 minCount;
    s32 maxCountExclusive;
} MicquizSelectionConfig;

typedef struct QuizMotionWork {
    Vec position;
    Vec referencePosition;
    f32 motionSpeed;
    s32 frame;
    Vec rotation;
    Vec scale;
    Vec positionStep;
    f32 unk44; /* The game's code never reads this field. */
    f32 rotationStepY;
    f32 unk4C; /* The game's code never reads this field. */
    s32 variantIndex;
    s32 state;
    s32 modelNameIndex;
    s32 modelPairIndex;
    s32 motionState;
    s16 modelId;
    s16 jointMotionIds[2];
} QuizMotionWork;

struct QuizSelectionEntry {
    s32 count;
    s32 minCount;
    s32 maxCountExclusive;
};

/* comparison.c */
void fn_1_1FB04(void);
void fn_1_20804(OMOBJ *object);

/* comparison_moving.c */
void fn_1_229E0(void);
void fn_1_24B40(OMOBJ *object);

/* count.c */
void fn_1_14EE0(void);
void fn_1_1598C(OMOBJ *object);

/* count_second.c */
void fn_1_1706C(void);
void fn_1_17BEC(OMOBJ *object);

/* cube.c */
void fn_1_1C210(void);
void fn_1_1DBBC(OMOBJ *object);

/* hint_scene.c */
void fn_1_2A164(void);
void fn_1_2B290(OMOBJ *object);

/* main.c */
extern s32 lbl_1_bss_100;
extern f32 lbl_1_bss_104;
extern s32 lbl_1_bss_110;
extern HUPROCESS *lbl_1_bss_114[11];
extern s32 lbl_1_bss_20;
extern s32 lbl_1_bss_24[20];
extern s32 lbl_1_bss_74[6];
extern f32 lbl_1_bss_8C;
extern s32 lbl_1_bss_90;
extern s32 lbl_1_bss_94;
extern s32 lbl_1_bss_98;
extern s16 lbl_1_bss_9C[2];
extern s16 lbl_1_bss_A0;
extern s16 lbl_1_bss_A2;
extern s16 lbl_1_bss_A4;
extern OMOBJ *lbl_1_bss_A8;
extern OMOBJ *lbl_1_bss_AC[20];
extern s32 lbl_1_bss_FC;
extern OM_CAMERA_VIEW lbl_1_data_388;
extern u32 lbl_1_data_3A4[6][4];
extern s32 lbl_1_data_404[6];
extern struct QuizSelectionEntry lbl_1_data_41C[5];
extern s16 lbl_1_data_458[12];
extern Point3d lbl_1_data_470;
extern Point3d lbl_1_data_47C;
extern u8 lbl_1_data_488[4];
void fn_1_10B4(u32 message);
void fn_1_12F4(void);
void fn_1_14F0(u32 message);
void fn_1_1730(void);
void fn_1_192C(void);
void fn_1_1B28(void);
void fn_1_1D24(void);
void fn_1_1F20(u32 message);
void fn_1_2160(void);
void fn_1_235C(void);
void fn_1_2558(void);
void fn_1_2788(OMOBJ *object, s32 motion, f32 scale, u32 attribute);
void fn_1_2800(void);
s32 fn_1_2A18(Vec *position0, f32 radius0, Vec *position1, f32 radius1);
f32 fn_1_2AD8(f32 start, f32 end, f32 amount);
void fn_1_2D18(HU3D_MODELID modelId, char *objName, Vec *position);
void fn_1_83C(void);
void fn_1_A38(u32 message);
void fn_1_C78(void);
void fn_1_E74(u32 message);

/* memory.c */
void fn_1_197D4(void);
void fn_1_19FD8(OMOBJ *modeObject);

/* quiz_mix.c */
void fn_1_2F83C(void);
void fn_1_2F890(void);
void fn_1_2FFA4(OMOBJ *object);

/* selection_process.c */
void fn_1_2C624(void);
void fn_1_2CEA4(OMOBJ *object);

/* selection_scene.c */
void fn_1_26810(void);
void fn_1_27CA0(OMOBJ *object);

/* tutorial.c */
void fn_1_2EDB4(void);
void fn_1_2F2B8(OMOBJ *object);

/* Adjusts and shows the debug camera, with a preset for the current quiz round. */
static inline void QuizDebugCameraUpdate(OM_CAMERA_VIEW *defaultView, GXColor *windowColor)
{
    windowColor->r = 0;
    windowColor->g = 0;
    windowColor->b = 0x90;
    windowColor->a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].z -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].z += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, defaultView);

    printWin(0x10, 0x28, 0x180, 0x54, windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
}

/* Shows the debug camera in its standard window. */
static inline void QuizDebugCamera(OM_CAMERA_VIEW *defaultView)
{
    GXColor windowColor;

    QuizDebugCameraUpdate(defaultView, &windowColor);
}

/* Updates and shows the comparison rounds' debug camera. */
static inline void QuizComparisonCameraUpdate(OM_CAMERA_VIEW *defaultView, GXColor *windowColor)
{
    windowColor->r = 0;
    windowColor->g = 0;
    windowColor->b = 0x90;
    windowColor->a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].x -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].x += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, defaultView);

    printWin(0x10, 0x28, 0xF0, 0x54, windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
}

/* Spreads the two models apart along X. */
static inline void QuizModelPairSpread(s16 *models, const f32 *spread)
{
    Vec positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(models[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= *spread;
    positions[1].x += *spread;
    i = 0;
    while (i < 2) {
        fn_80026790(models[i], &positions[i]);
        i += 1;
    }
}

/* Gets a named model object's world position. */
static inline void QuizObjectWorldPosition(HU3D_MODELID modelId, char *objName, Vec *position)
{
    Mtx matrix;

    fn_80023398(modelId, objName, matrix);
    position->x = matrix[0][3];
    position->y = matrix[1][3];
    position->z = matrix[2][3];
}

/* Counts the objects for the quiz answer and records the result. */
static inline void QuizCountAnswerRecord(void)
{
    void MicQuizExpectedAnswerSet(s32 expectedAnswer);

    s32 count;
    s32 index;
    count = 0;
    lbl_1_bss_94 = lbl_1_bss_74[0];
    switch (lbl_1_bss_98)
    {
    case 0 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 == lbl_1_bss_24[index])
            {
                count += 1;
            }
            index += 1;
        }
        break;
    case 1 :
        count = lbl_1_bss_90;
        break;
    case 2 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 != lbl_1_bss_24[index])
            {
                count += 1;
            }
            index += 1;
        }
        break;
    }
    MicQuizExpectedAnswerSet(count);
    OSReport("----------------------------ANSER:%d\n", count);
}

/* Copies the specified EFB region to a texture. */
static inline void QuizTextureCopy(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format, void *destination)
{
    GXSetTexCopySrc((u16)x, (u16)y, (u16)width, (u16)height);
    GXSetTexCopyDst((u16)width, (u16)height, format, 0);
    GXCopyTex(destination, 1);
}

/* Sets every material of a model to black. */
static inline void QuizModelMaterialsBlack(s16 modelId)
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = model->hsf;
    HSF_MATERIAL *material = hsf->material;
    s32 index;

    for (index = 0; index < hsf->materialNum;) {
        material->color[0] = 0;
        material->color[1] = 0;
        material->color[2] = 0;
        index++;
        material++;
    }
}

#endif
