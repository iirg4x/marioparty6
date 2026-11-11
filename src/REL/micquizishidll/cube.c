/* Cube round: the hint character, the cube grid and its fade, and the answer. */
#include "msm_stream.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "game/hu3d.h"

#include "game/object.h"

#include "game/printfunc.h"

#include "game/process.h"
#include "game/mic.h"
#include "humath.h"
#include "REL/micquizishidll.h"

/* quiz.h leaves these out (other files call them without a prototype); this file's calls take
 * them with one. */
void MicQuizCountdownInit(void);
void MicQuizExpectedAnswerSet(s32 expectedAnswer);
void MicQuizListenStart(void);
void MicQuizMicContextSet(s32 questionType);
void MicQuizQuestionReadySet(void);

static s32 lbl_1_data_1188[7] = {0xA4007A, 0xA4007B, 0xA4007C, 0xA4007D, 0xA4007E, 0xA4007F,
                                 0xA40080};

struct SoundChoice lbl_1_data_11A4[6] = {{0x3BE, 0x3C0}, {0x3D4, 0x3D5}, {0x3D9, 0x3DA},
                                         {0x3D9, 0x3E0}, {0x3AA, 0x3AB}, {0x3A5, 0x3E7}};

s32 lbl_1_data_11BC[6][2] = {{30, 60}, {20, 40}, {30, 30}, {10, 40}, {0, 0}, {6, 25}};

s32 lbl_1_data_11EC[5] = {2, 3, 4, 5, 6};

f32 lbl_1_data_1200[5] = {10.0f, 10.0f, 10.0f, 10.0f, 10.0f};

QuizCameraViewRecord lbl_1_data_1214[6] = {{60.0f, 1469.0f}, {45.0f, 1181.5f}, {70.0f, 1632.5f},
                                           {50.0f, 1334.5f}, {100.0f, 2662.0f}, {50.0f, 1330.0f}};

QuizCharacterMotionData lbl_1_data_1244[6] = {
    {{0xA40001, 0xA40002, 0xA40003, 0xA40004, 0xA40001}},
    {{0xA40006, 0xA40007, 0xA40008, 0xA40009, 0xA40006}},
    {{0xA4000B, 0xA4000C, 0xA4000D, 0xA4000E, 0xA4000B}},
    {{0xA40015, 0xA40016, 0xA40017, 0xA40018, 0xA40015}},
    {{0xA4001B, 0xA4001C, 0xA4001D, 0xA4001E, 0xA4001B}},
    {{0xA40010, 0xA40011, 0xA40012, 0xA40013, 0xA40010}}
};

s32 lbl_1_data_12BC[6] = {0xA40000, 0xA40005, 0xA4000A, 0xA40014, 0xA4001A, 0xA4000F};

f32 lbl_1_data_12D4[6] = {1.8f, 2.2f, 1.7f, 2.2f, 0.8f, 2.0f};

OM_CAMERA_VIEW lbl_1_data_12EC = {{0.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 5840.0f};

OM_CAMERA_VIEW lbl_1_data_1308 = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 800.0f};

Point3d lbl_1_data_1324 = {-100.0f, 8000.0f, -400.0f};

Point3d lbl_1_data_1330 = {0.0f, 1.0f, 0.0f};

Point3d lbl_1_data_133C = {0.0f, 0.0f, -400.0f};

f32 lbl_1_data_1348[5] = {1.2f, 0.8f, 0.6f, 0.48f, 0.4f};

s32 lbl_1_data_135C[5] = {128, 84, 64, 52, 44};

char *lbl_1_data_13A8[6] = {"v77_men1", "v77_men2", "v77_men3", "v77_men4", "v77_men5", "v77_men6"};

void fn_1_1D578(void);
void fn_1_1D6BC(OMOBJ *work);
void fn_1_1D82C(OMOBJ *object);
void fn_1_1DA74(OMOBJ *object);
void fn_1_1DB00(OMOBJ *object);
void fn_1_1DB98(void);
void fn_1_1E364(void);
void fn_1_1E460(void);
void fn_1_1E524(s16 modelId);
void fn_1_1E600(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format, void *destination);
void fn_1_1E6B8(void);
extern s16 lbl_1_bss_1024[0x24];
extern s32 lbl_1_bss_106C;
extern QuizGridCell lbl_1_bss_1070[36];
extern s16 lbl_1_bss_1190;
extern ANIMDATA *lbl_1_bss_1194[0x24];
extern s16 lbl_1_bss_1224;
extern OMOBJ *lbl_1_bss_1228;
extern OMOBJ *lbl_1_bss_122C;
extern s32 lbl_1_bss_1230;
extern QuizGridTileView lbl_1_bss_1234[0x24];
extern s32 lbl_1_bss_1594;
extern HUPROCESS *lbl_1_bss_E38;
extern f32 lbl_1_bss_E3C;
extern s32 lbl_1_bss_E40[6];
extern s32 lbl_1_bss_E58;
extern ANIMDATA *lbl_1_bss_E5C;
extern s16 lbl_1_bss_E60;
extern s16 lbl_1_bss_E62;
extern Vec lbl_1_bss_E64;
extern s16 lbl_1_bss_E70[0xDA];

/* Builds the quiz grid, its tile animations, and the scene's display tasks. */
void fn_1_1C210(void) {
    s32 index;
    s32 randomOffset;
    s32 quizState;
    s16 animationId;
    s32 axis;
    u32 bufferSize;
    f32 spread;
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_100 = MicQuizQuestionLevelGet();
    lbl_1_bss_1230 = lbl_1_data_11EC[lbl_1_bss_100];
    lbl_1_bss_106C = lbl_1_data_135C[lbl_1_bss_100];
    randomOffset = frandmod(6);
    for (index = 0; index < 6; ++index)
    {
        lbl_1_bss_E40[index] = randomOffset;
        randomOffset++;
        if (randomOffset >= 6)
        {
            randomOffset = 0;
        }
    }
    MicQuizCountdownInit();
    lbl_1_bss_FC = 0;
    lbl_1_bss_8C = 11.111111f;
    lbl_1_bss_E58 = frandmod(6);
    lbl_1_bss_1594 = 0;
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(2, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    fn_8003E488(2, &lbl_1_data_12EC);
    fn_8003E488(1, &lbl_1_data_1308);
    lbl_80266B18->y = lbl_1_data_1214[lbl_1_bss_E58].centerY;
    lbl_80266BD8[0] = lbl_1_data_1214[lbl_1_bss_E58].zoom;
    lbl_1_bss_A8 = fn_8003AB38(* lbl_1_bss_114, 0x7FDA, 0, 0, - 1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 2;
    fn_800285BC();
    lbl_1_bss_110 = (s32) Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U, 0U);
    Hu3DGLightPosAimSetV(lbl_1_bss_110, & lbl_1_data_470, & lbl_1_data_47C);
    Hu3DGLightStaticSet(lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_110);
    Hu3DGLightColorSet(lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10, 0x10, 0x10);
    Hu3DShadowPosSet(& lbl_1_data_1324, & lbl_1_data_1330, & lbl_1_data_133C);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x34)), 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x35)), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    fn_80026B64(lbl_1_bss_A4, 2);
    fn_80026B64(lbl_1_bss_A2, 2);
    lbl_1_bss_E60 = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x44)), 0x10000000, 2));
    fn_80026760(lbl_1_bss_E60, 0.0f, - 100.0f, - 50.0f);
    fn_80026B64(lbl_1_bss_E60, 1);
    lbl_1_bss_1224 = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x42)), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1224, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_1224, 2);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x2B)), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x2C)), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x2D)), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A0, 2);
    fn_80026B64(lbl_1_bss_9C[0], 2);
    fn_80026B64(lbl_1_bss_9C[1], 2);
    lbl_1_bss_E3C = 100.0f * lbl_1_data_1348[lbl_1_bss_100];
    for (index = 0; index < lbl_1_bss_1230 * lbl_1_bss_1230; ++index)
    {
        fn_8000C200(lbl_1_bss_1194[index] = HuSprAnimMake((s16) lbl_1_bss_106C,
                                                          (s16) lbl_1_bss_106C, 0), 0x10000000);
        lbl_1_bss_1194[index]->bmp->palData = NULL;
        lbl_1_bss_1194[index]->bmp->palNum = 0;
        bufferSize = GXGetTexBufferSize((u16) lbl_1_bss_106C, (u16) lbl_1_bss_106C, GX_TF_RGBA8, 0,
                                        0);
        lbl_1_bss_1194[index]->bmp->data = HuMemDirectMallocNum(2, bufferSize, 0x10000000);
    }
    lbl_1_bss_122C = fn_8003AB38(* lbl_1_bss_114, 0x64, 1, 5, - 1, fn_1_1D6BC);
    lbl_1_bss_122C->data = HuMemDirectMallocNum(HEAP_HEAP, 0x1C, 0x10000000);
    lbl_1_bss_122C->work[0] = lbl_1_bss_E58;
    lbl_1_bss_1228 = fn_8003AB38(* lbl_1_bss_114, 0x64, 1, 5, - 1, fn_1_1D82C);
    lbl_1_bss_1228->data = HuMemDirectMallocNum(HEAP_HEAP, 0x1C, 0x10000000);
    lbl_1_bss_1228->work[0] = lbl_1_bss_E58;
    MicQuizExpectedAnswerSet(lbl_1_bss_E58);
    lbl_1_bss_1190 = HuSprGrpCreate((s16)(lbl_1_bss_1230 * lbl_1_bss_1230));
    for (index = 0; index < lbl_1_bss_1230 * lbl_1_bss_1230; ++index)
    {
        HuSprGrpMemberSet(lbl_1_bss_1190, (s16) index, HuSprCreate(lbl_1_bss_1194[index], 0x32, 0));
        HuSprAttrSet(lbl_1_bss_1190, (s16) index, 4);
    }
    fn_1_1D578();
    lbl_1_bss_E5C = HuSprAnimRead(fn_80008DD0(DATANUM(0xa40000, 0x43), 0x10000000, 2));
    animationId = fn_80030B2C(lbl_1_bss_E5C, lbl_1_bss_E60, "v77_BG");
    Hu3DAnimBankSet(animationId, (u16) lbl_1_bss_E40[0]);
    for (index = 0; index < lbl_1_bss_1230 * lbl_1_bss_1230; ++index)
    {
        lbl_1_bss_1024[index] = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x41)), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_1024[index], 2);
        fn_80030B2C(lbl_1_bss_1194[index], lbl_1_bss_1024[index], "v77_men1");
        for (quizState = 1; quizState < 6; ++quizState)
        {
            animationId = fn_80030B2C(lbl_1_bss_E5C, lbl_1_bss_1024[index],
                                      lbl_1_data_13A8[quizState]);
            Hu3DAnimBankSet(animationId, (u16) lbl_1_bss_E40[quizState]);
        }
    }
    spread = 60.0f * lbl_1_data_1348[lbl_1_bss_100];
    for (index = 0; index < lbl_1_bss_1230; ++index)
    {
        for (quizState = 0; quizState < lbl_1_bss_1230; ++quizState)
        {
            f32 positionX;
            f32 positionY;
            f32 positionZ;
            positionX = (lbl_1_bss_E3C / 2.0f) + (((f32) quizState * (lbl_1_bss_E3C + spread))
                    - (((lbl_1_bss_E3C * (f32) lbl_1_bss_1230)
                        + (spread * (f32)(lbl_1_bss_1230 - 1))) / 2.0f));
            positionY = (f32)(lbl_1_bss_1230 - index) * (lbl_1_bss_E3C + spread);
            positionZ = - 200.0f - lbl_1_bss_E3C;
            axis = frandmod(2);
            fn_80026760(lbl_1_bss_1024[quizState + index * lbl_1_bss_1230], positionX, positionY,
                        positionZ);
            fn_800268B0(lbl_1_bss_1024[quizState + index * lbl_1_bss_1230],
                    lbl_1_data_1348[lbl_1_bss_100], lbl_1_data_1348[lbl_1_bss_100], lbl_1_data_1348[
                            lbl_1_bss_100]);
            if (axis != 0)
            {
                rotationX = (f32)(u32) frandmod(0x168);
                rotationY = 0.0f;
            rotationZ = 0.0f;
                if ((u32) frandmod(2) != 0)
                {
                    lbl_1_bss_1234[quizState + index
                            * lbl_1_bss_1230].velocityX = lbl_1_data_1200[lbl_1_bss_100] + (f32)(u32) frandmod(5);
                }
                else
                {
                    lbl_1_bss_1234[quizState + index
                            * lbl_1_bss_1230].velocityX = - (lbl_1_data_1200[lbl_1_bss_100] + (f32)(u32) frandmod(5));
                }
            }
            else
            {
                rotationX = 0.0f;
                rotationY = (f32)(u32) frandmod(0x168);
                rotationZ = 0.0f;
                if ((u32) frandmod(2) != 0)
                {
                    lbl_1_bss_1234[quizState + index
                            * lbl_1_bss_1230].velocityY = lbl_1_data_1200[lbl_1_bss_100] + (f32)(u32) frandmod(5);
                }
                else
                {
                    lbl_1_bss_1234[quizState + index
                            * lbl_1_bss_1230].velocityY = - (lbl_1_data_1200[lbl_1_bss_100] + (f32)(u32) frandmod(5));
                }
            }
            fn_80026808(lbl_1_bss_1024[quizState + index * lbl_1_bss_1230], rotationX, rotationY,
                        rotationZ);
            lbl_1_bss_1234[quizState + index * lbl_1_bss_1230].rotationX = rotationX;
            lbl_1_bss_1234[quizState + index * lbl_1_bss_1230].rotationY = rotationY;
            lbl_1_bss_1234[quizState + index * lbl_1_bss_1230].rotationZ = rotationZ;
            lbl_1_bss_1234[quizState + index * lbl_1_bss_1230].velocityZ = 0.0f;
        }
    }
    lbl_1_bss_E70[0] = fn_80025A04(fn_80008DD0((DATANUM(0xa40000, 0x32)), 0x10000000, 2));
    fn_80026760(lbl_1_bss_E70[0], 0.0f, lbl_1_data_1214[lbl_1_bss_E58].centerY, 120.0f);
    fn_800268B0(lbl_1_bss_E70[0], 3.0f, 3.0f, 0.0f);
    fn_80026B64(lbl_1_bss_E70[0], 1);
    fn_80026CBC(lbl_1_bss_E70[0], 0.0f);
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(* lbl_1_bss_114);
    MicQuizMicContextSet(4);
    fn_8002583C(2, 0, fn_1_1E524);
    fn_8000DAC0(fn_1_1E6B8, 0x64, 0x1800, 0, * lbl_1_bss_114);
    fn_8000DAC0(fn_1_1DB98, 0x64, 0x1800, 0, * lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FIFTH);
}

/* Lays out the cube grid: each cell's screen position and size from the grid size and the cell
   size. */
void fn_1_1D578(void) {
    s32 gridCellCount;
    s32 gridSize;
    s32 gridSizeCopy;
    s32 row;
    s32 column;

    gridCellCount = lbl_1_bss_1230 * lbl_1_bss_1230;
    gridSize = lbl_1_bss_1230;
    gridSizeCopy = gridSize;
    for (row = 0; row < lbl_1_bss_1230; row++) {
        for (column = 0; column < lbl_1_bss_1230; column++) {
            lbl_1_bss_1070[column + row * gridSize].positionX =
                (column * lbl_1_bss_106C) - 0x80;
            lbl_1_bss_1070[column + row * gridSize].positionY =
                (row * lbl_1_bss_106C) - 0x80;
            lbl_1_bss_1070[column + row * gridSize].width =
                lbl_1_bss_1070[column + row * gridSize].height =
                    (s16)lbl_1_bss_106C;
        }
    }
}

/* Creates a character model with its motions at its place. */
void fn_1_1D6BC(OMOBJ *work) {
    QuizCharacterMotionView *model;
    ANIMDATA *animation;
    s32 motionIndex;

    model = work->data;
    animation = fn_80008DD0(lbl_1_data_12BC[work->work[0]], 0x10000000, 2);
    model->modelId = fn_80025A04(animation);
    for (motionIndex = 0; motionIndex < 5; motionIndex++) {
        animation = fn_80008DD0(lbl_1_data_1244[work->work[0]].dataNumbers[motionIndex], 0x10000000,
                                2);
        model->motions[motionIndex] = fn_80030458(model->modelId, animation);
    }
    model->state = 0;
    model->characterId = work->work[0];
    fn_800269E8(model->modelId, 0x40000002);
    fn_8002C95C(model->modelId, model->motions[0]);
    fn_80026760(model->modelId, 0.0f, 0.0f, 0.0f);
    fn_80026808(model->modelId, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model->modelId, 1);
    work->objFunc = fn_1_1DA74;
}

/* Creates the hint character's model, scaled, with its motions, and keeps its model for the
   hint. */
void fn_1_1D82C(OMOBJ *object) {
    QuizCharacterMotionView *work;
    ANIMDATA *animation;
    f32 modelScale;
    s32 motionIndex;

    work = object->data;
    animation = fn_80008DD0(lbl_1_data_12BC[object->work[0]],
        0x10000000, 2);
    lbl_1_bss_E62 = fn_80025A04(animation);
    motionIndex = 0;
    while (motionIndex < 5) {
        animation = fn_80008DD0(
            lbl_1_data_1244[object->work[0]].dataNumbers[motionIndex],
            0x10000000, 2);
        work->motions[motionIndex] = fn_80030458(lbl_1_bss_E62, animation);
        motionIndex += 1;
    }
    work->state = 0;
    work->characterId = object->work[0];
    fn_800269E8(lbl_1_bss_E62, 0x40000001);
    fn_800269E8(lbl_1_bss_E62, 1);
    fn_8002C95C(lbl_1_bss_E62, work->motions[0]);
    lbl_1_bss_E64.x = 0.0f;
    lbl_1_bss_E64.y = 0.0f;
    lbl_1_bss_E64.z = -300.0f;
    modelScale = lbl_1_data_12D4[object->work[0]];
    fn_800268B0(lbl_1_bss_E62, modelScale, modelScale, modelScale);
    fn_80026760(lbl_1_bss_E62, lbl_1_bss_E64.x, lbl_1_bss_E64.y,
        lbl_1_bss_E64.z);
    fn_80026808(lbl_1_bss_E62, 0.0f, 0.0f, 0.0f);
    fn_80026B64(lbl_1_bss_E62, 2);
    object->objFunc = fn_1_1DB00;
}

/* Keeps a character model at the origin, unturned, each frame. */
void fn_1_1DA74(OMOBJ *object) {
    QuizCharacterMotionView *work;

    work = object->data;
    fn_80026760(work->modelId, 0.0f,
        0.0f, 0.0f);
    fn_80026808(work->modelId, 0.0f,
        0.0f, 0.0f);
}

/* Keeps the hint character's model at its place, unturned, each frame. */
void fn_1_1DB00(OMOBJ *object) {
    void *work;

    work = object->data;
    fn_80026760(lbl_1_bss_E62, lbl_1_bss_E64.x, lbl_1_bss_E64.y, lbl_1_bss_E64.z);
    fn_80026808(lbl_1_bss_E62, 0.0f,
        0.0f, 0.0f);
}

void fn_1_1DB98(void) {
    MicQuizResponseRun(4);
}

/* Runs the cube round's mode steps: the question message, the cube's fade, the hint, the answer
   and the closing wipe. */
void fn_1_1DBBC(OMOBJ *object) {
    f32 transparency;
    QuizCharacterMotionView *work = lbl_1_bss_1228->data;
    s32 index;

    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        return;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_FC = 2;
            lbl_1_bss_104 = 0.0f;
            return;
        }
        break;
    case 2:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            fn_1_192C();
            MicQuizQuestionReadySet();
            return;
        }
        lbl_1_bss_104 += 1.0f;
        fn_1_1E460();
        return;
    case 4:
        if ((MicQuizResponseStateGet() == 3) || (MicQuizResponseStateGet() == 4)) {
            lbl_1_bss_FC = 8;
            lbl_1_bss_1594 = 3;
            return;
        }
        if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 6;
            return;
        }
        break;
    case 6:
        if (lbl_1_bss_104 >= 120.0f) {
            OSReport("                           CUBE HINT:%f\n", lbl_1_bss_104);
            lbl_1_bss_FC = 4;
            lbl_1_bss_104 = 0.0f;
            MicQuizListenStart();
            lbl_1_bss_1594 = 0;
            return;
        }
        lbl_1_bss_104 += 1.0f;
        lbl_1_bss_1594 = 2;
        return;
    case 8:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xA;
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 10:
        lbl_1_bss_FC = 0xB;
        lbl_1_bss_104 = 0.0f;
        return;
    case 11:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xC;
            fn_80026CBC(lbl_1_bss_E70[0], 1.0f);
            return;
        }
        transparency = (f32) sin((M_PI * (f64) (0.75f * lbl_1_bss_104)) / 180.0);
        fn_80026CBC(lbl_1_bss_E70[0], transparency);
        lbl_1_bss_104 += 1.0f;
        return;
    case 12:
        fn_80026A44(lbl_1_bss_E62, 1U);
        lbl_1_bss_FC = 0xD;
        fn_8002CCC0(lbl_1_bss_E62, work->motions[1], 0.0f, 8.0f, 0x40000001U);
        lbl_1_bss_E38 = fn_8000DAC0(fn_1_1E364, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
        lbl_1_bss_104 = 0.0f;
        return;
    case 13:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xE;
            fn_8002CCC0(lbl_1_bss_E62, work->motions[3], 0.0f, 8.0f, 0x40000001U);
            HuPrcKill(lbl_1_bss_E38);
            HuAudFXPlay((s32) lbl_1_data_11A4[lbl_1_bss_E58].finishSound);
            return;
        }
        lbl_1_bss_E64.z += 2.0f;
        lbl_1_bss_104 += 1.0f;
        fn_80026760(lbl_1_bss_E62, lbl_1_bss_E64.x, lbl_1_bss_E64.y, lbl_1_bss_E64.z);
        return;
    case 14:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xE;
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        /* fallthrough */
    case 15:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0x10;
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
            WipeCreate(2, 0, 0x14);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 16:
        if (WipeCheck() == 0) {
            fn_800258A8(2, 0);
            index = 0;
            while (index < (s32) (lbl_1_bss_1230 * lbl_1_bss_1230)) {
                HuSprAnimKill(lbl_1_bss_1194[index]);
                index += 1;
            }
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport(" CUBE END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Plays the hint character's motion sound each time its motion reaches the sound's frame. */
void fn_1_1E364(void) {
    s16 motionId;
    f32 motionTime;

    motionId = lbl_1_bss_E62;
    HuPrcSleep(8);
    for (;;) {
        HuPrcVSleep();
        motionTime = fn_8002D4E0(motionId);
        if (motionTime == (f32)lbl_1_data_11BC[lbl_1_bss_E58][0] ||
            motionTime == (f32)lbl_1_data_11BC[lbl_1_bss_E58][0]) {
            HuAudFXPlay(lbl_1_data_11A4[lbl_1_bss_E58].motionSound);
        }
    }
}

void fn_1_1E460(void)
{
    Vec positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= lbl_1_bss_8C;
    positions[1].x += lbl_1_bss_8C;
    i = 0;
    while (i < 2) {
        fn_80026790(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
}

/* Layer hook: draws the cube grid's cells from their images. */
void fn_1_1E524(s16 modelId) {
    s32 index;

    index = 0;
    while (index < lbl_1_bss_1230 * lbl_1_bss_1230) {
        fn_1_1E600(
            (s16)(lbl_1_bss_1070[index].positionX + 0x140),
            (s16)(lbl_1_bss_1070[index].positionY + 0xF0),
            lbl_1_bss_1070[index].width,
            lbl_1_bss_1070[index].height,
            6,
            lbl_1_bss_1194[index]->bmp->data);
        index++;
    }
    fn_8002BF48();
}

void fn_1_1E600(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format,
                void *destination) {
    GXSetTexCopySrc((u16)x, (u16)y, (u16)width, (u16)height);
    GXSetTexCopyDst((u16)width, (u16)height, format, 0);
    GXCopyTex(destination, 1);
}

/* Returns 1 when the value is zero, within 0.0001. */
s32 fn_1_1E680(f32 value) {
    if ((value < 0.0001) && (value > -0.0001)) {
        return 1;
    }
    return 0;
}

/* Updates tile rotation and spacing for the quiz board's entrance and idle states. */
void fn_1_1E6B8(void) {
    s32 row;
    s32 column;
    s32 frame;
    f32 spread;
    f32 step;
    f32 positionX;
    f32 positionY;
    f32 positionZ;

    while (1) {
        HuPrcVSleep();
        if (lbl_1_bss_1594 == 1) {
            for (row = 0; row < lbl_1_bss_1230; ++row) {
                for (column = 0; column < lbl_1_bss_1230; ++column) {
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX > 0.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX -= 0.0075f;
                        if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX <= 0.0f) {
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX = 0.0f;
                        }
                    } else {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX += 0.0075f;
                        if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX >= 0.0f) {
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX = 0.0f;
                        }
                    }
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY > 0.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY -= 0.0075f;
                        if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY <= 0.0f) {
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY = 0.0f;
                        }
                    } else {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY += 0.0075f;
                        if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY >= 0.0f) {
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY = 0.0f;
                        }
                    }
                }
            }
        }

        else if (lbl_1_bss_1594 == 3) {
            for (frame = 0; frame < 0x78; ++frame) {
                HuPrcVSleep();
                for (row = 0; row < lbl_1_bss_1230; ++row) {
                    for (column = 0; column < lbl_1_bss_1230; ++column) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX =
                            fn_1_2AD8(lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX,
                                      0.0f, 0.05f);
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY =
                            fn_1_2AD8(lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY,
                                      0.0f, 0.05f);
                        fn_80026808(lbl_1_bss_1024[column + row * lbl_1_bss_1230],
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX,
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY,
                            lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationZ);
                    }
                }
            }
            for (row = 0; row < lbl_1_bss_1230; ++row) {
                for (column = 0; column < lbl_1_bss_1230; ++column) {
                    fn_80026808(lbl_1_bss_1024[column + row * lbl_1_bss_1230],
                                    0.0f, 0.0f, 0.0f);
                }
            }
            lbl_1_bss_1594 = 4;
        }

        else if (lbl_1_bss_1594 == 4) {
            spread = 60.0f * lbl_1_data_1348[lbl_1_bss_100];
            step = (spread * (f32)(lbl_1_bss_1230 - 1)) / 60.0f;
            for (frame = 0; frame < 0x3C; ++frame) {
                HuPrcVSleep();
                for (row = 0; row < lbl_1_bss_1230; ++row) {
                    for (column = 0; column < lbl_1_bss_1230; ++column) {
                        positionX = (lbl_1_bss_E3C / 2.0f) +
                            (((f32)column * (lbl_1_bss_E3C + spread)) -
                             (((lbl_1_bss_E3C * (f32)lbl_1_bss_1230) +
                             (spread * (f32)(lbl_1_bss_1230 - 1))) / 2.0f));
                        positionY = ((step * (f32)frame) +
                            ((f32)(lbl_1_bss_1230 - row) * (lbl_1_bss_E3C + spread))) -
                            (2.3333333f * (f32)frame);
                        positionZ = (-200.0f - lbl_1_bss_E3C) + (step * (f32)frame);
                        fn_80026760(lbl_1_bss_1024[column + row * lbl_1_bss_1230],
                                        positionX, positionY, positionZ);
                    }
                }
                spread -= (60.0f * lbl_1_data_1348[lbl_1_bss_100]) / 60.0f;
            }
            spread = 0.0f;
            for (row = 0; row < lbl_1_bss_1230; ++row) {
                for (column = 0; column < lbl_1_bss_1230; ++column) {
                    positionX = (lbl_1_bss_E3C / 2.0f) +
                        (((f32)column * (lbl_1_bss_E3C + spread)) -
                         (((lbl_1_bss_E3C * (f32)lbl_1_bss_1230) +
                           (spread * (f32)(lbl_1_bss_1230 - 1))) / 2.0f));
                    positionY = ((step * (f32)frame) +
                        ((f32)(lbl_1_bss_1230 - row) * (lbl_1_bss_E3C + spread))) - 140.0f;
                    positionZ = (-200.0f - lbl_1_bss_E3C) + (step * (f32)frame);
                    fn_80026760(lbl_1_bss_1024[column + row * lbl_1_bss_1230],
                                    positionX, positionY, positionZ);
                }
            }
            HuPrcEnd();
            while (1) {
                HuPrcVSleep();
            }
        }

        if (lbl_1_bss_1594 != 3 && lbl_1_bss_1594 != 2 && lbl_1_bss_1594 != 4) {
            for (row = 0; row < lbl_1_bss_1230; ++row) {
                for (column = 0; column < lbl_1_bss_1230; ++column) {
                    lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX +=
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityX;
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX >= 360.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX = 0.0f;
                    }
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX <= -360.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX = 0.0f;
                    }
                    lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY +=
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].velocityY;
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY >= 360.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY = 0.0f;
                    }
                    if (lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY <= -360.0f) {
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY = 0.0f;
                    }
                    fn_80026808(lbl_1_bss_1024[column + row * lbl_1_bss_1230],
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationX,
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationY,
                        lbl_1_bss_1234[column + row * lbl_1_bss_1230].rotationZ);
                }
            }
        }
    }
}

/* Returns the negated value divided by the divisor. */
f32 fn_1_1F644(s32 divisor, f32 value) {
    f32 quotient;

    quotient = -value / (f32) divisor;
    return quotient;
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_1F690(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;

    lbl_80266BD8[1] += (f32)(lbl_802BFFEC[1] / 5);
    lbl_80266BD8[1] -= (f32)(lbl_802BFFE8[1] / 5);
    if (lbl_802C0010[1] & 0x4) lbl_80266B18[1].y -= 5.0f;
    if (lbl_802C0010[1] & 0x8) lbl_80266B18[1].y += 5.0f;
    if (lbl_802C0010[1] & 0x1) lbl_80266B18[1].x -= 5.0f;
    if (lbl_802C0010[1] & 0x2) lbl_80266B18[1].x += 5.0f;
    lbl_80266A58[1].x += 5.0f * ((f32)lbl_802BFFF0[1] / 44.0f);
    lbl_80266A58[1].y -= 5.0f * ((f32)lbl_802BFFF4[1] / 44.0f);

    if (lbl_802C0008[1] & 0x800) fn_8003E488(2, &lbl_1_data_12EC);

    printWin(0x10, 0x28, 240, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
    print8(0x10, 0x58, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[1].x, lbl_80266B18[1].y,
           lbl_80266B18[1].z);
    print8(0x10, 0x64, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[1].x, lbl_80266A58[1].y,
           lbl_80266A58[1].z);
    print8(0x10, 0x70, 1.0f, "CZoom:%.02f", lbl_80266BD8[1]);
}

s32 lbl_1_bss_1594;
QuizGridTileView lbl_1_bss_1234[0x24];
s32 lbl_1_bss_1230;
OMOBJ *lbl_1_bss_122C;
OMOBJ *lbl_1_bss_1228;
s16 lbl_1_bss_1224;
ANIMDATA *lbl_1_bss_1194[0x24];
s16 lbl_1_bss_1190;
QuizGridCell lbl_1_bss_1070[36];
s32 lbl_1_bss_106C;
s16 lbl_1_bss_1024[0x24];
s16 lbl_1_bss_E70[0xDA];
Vec lbl_1_bss_E64;
s16 lbl_1_bss_E62;
s16 lbl_1_bss_E60;
ANIMDATA *lbl_1_bss_E5C;
s32 lbl_1_bss_E58;
s32 lbl_1_bss_E40[6];
f32 lbl_1_bss_E3C;
HUPROCESS *lbl_1_bss_E38;
