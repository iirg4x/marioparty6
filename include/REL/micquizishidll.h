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

struct SoundChoice {
    s16 motionSound;
    s16 finishSound;
};

typedef struct MicQuizRecord {
    s32 motionType; /* Motion type of the object on this place; minus one when free. */
    s32 objectIndex; /* Index of the object on this place. */
    s32 placed; /* One while an object holds this place, minus one when cleared; never read. */
} MicQuizRecord;

typedef struct MicQuizRecordGroup {
    MicQuizRecord entries[4];
} MicQuizRecordGroup;

typedef HuVec2f MicQuizSpritePosition;

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
    s32 unknown_08;
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
    s32 modelVariant;
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
    f32 rotationStepX;
    f32 rotationStepY;
    f32 rotationStepZ;
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
extern HUPROCESS *lbl_1_bss_114[0xB];
extern s32 lbl_1_bss_20;
extern s32 lbl_1_bss_24[0x14];
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
extern OMOBJ *lbl_1_bss_AC[0x14];
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

#endif
