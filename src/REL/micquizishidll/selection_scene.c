/* Pairing round: the character models, the paired entries and the answer. */
#include "msm_stream.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "math.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/wipe.h"
#include "datadir_enum.h"
#include "humath.h"
#include "REL/micquizishidll.h"

/* quiz.h leaves these out (other files call them without a prototype); this file's calls take
 * them with one. */
void MicQuizCountdownInit(void);
void MicQuizExpectedAnswerSet(s32 expectedAnswer);
void MicQuizPauseInit(HUPROCESS *parentProcess);

u32 lbl_1_data_19A0[7] = {
    0x00A4007A, 0x00A4007B, 0x00A4007C, 0x00A4007D,
    0x00A4007E, 0x00A4007F, 0x00A40080
};

Rectangle lbl_1_data_19BC[20] = {
    {-90, 0, 180, 10}, {-90, 10, 180, 10}, {-90, 20, 180, 10}, {-90, 30, 180, 10},
    {-90, 40, 180, 10}, {-90, 50, 180, 10}, {-90, 60, 180, 10}, {-90, 70, 180, 10},
    {-90, 80, 180, 10}, {-90, 90, 180, 10}, {-90, 100, 180, 10}, {-90, 110, 180, 10},
    {-90, 120, 180, 10}, {-90, 130, 180, 10}, {-90, 140, 180, 10}, {-90, 150, 180, 10},
    {-90, 160, 180, 10}, {-90, 170, 180, 10}, {-90, 180, 180, 10}, {-90, 190, 180, 10}
};

QuizMenuCharacterMotionData lbl_1_data_1A5C[] = {
    {{0x930000, 0x930026, 0x930028}}, {{0x3B0000, 0x3B0026, 0x3B0028}},
    {{0xC50000, 0xC50026, 0xC50028}}, {{0xF80000, 0xF80026, 0xF80028}},
    {{0xF10000, 0xF10026, 0xF10028}}, {{0x190000, 0x190026, 0x190028}},
    {{0xEB0000, 0xEB0026, 0xEB0028}}, {{0x350000, 0x350026, 0x350028}},
    {{0xD40000, 0xD40026, 0xD40028}}, {{0xBC0000, 0xBC0026, 0xBC0028}},
    {{0x2F0000, 0x2F0026, 0x2F0028}}, {{0, 0, 0}}, {{0, 0, 0}}, {{0, 0, 0}}
};

QuizCharacterEntry lbl_1_data_1B04[] = {
    {0x930000, 0x930006, 0x930028}, {0x3B0000, 0x3B0006, 0x3B0028},
    {0xC50000, 0xC50006, 0xC50028}, {0xF80000, 0xF80006, 0xF80028},
    {0xF10000, 0xF10006, 0xF10028}, {0x190000, 0x190006, 0x190028},
    {0xEB0000, 0xEB0006, 0xEB0028}, {0x350000, 0x350006, 0x350028},
    {0xD40000, 0xD40006, 0xD40028}, {0xBC0000, 0xBC0006, 0xBC0028},
    {0x2F0000, 0x2F0006, 0x2F0028}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
};

s32 lbl_1_data_1BAC[] = { 0x920000, 0x3A0000, 0xC40000, 0xF70000, 0xF00000, 0x180000, 0xEA0000,
                         0x340000, 0xD30000, 0xBB0000, 0x2E0000, 0, 0, 0 };

s32 lbl_1_data_1BE4[5] = { 4, 5, 7, 8, 10 };

unsigned int lbl_1_data_1BF8[] = { 0x930000, 0x930006, 0x930028, 0 };

OM_CAMERA_VIEW lbl_1_data_1C08 = { { 0.0f, 200.0f, 0.0f }, { -2.04f, 0.0f, 0.0f }, 5890.0f };
OM_CAMERA_VIEW lbl_1_data_1C24 = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 800.0f };
Point3d lbl_1_data_1C40 = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_1C4C = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_1C58 = { 0.0f, 0.0f, -400.0f };

void fn_1_27740(OMOBJ *work);
void fn_1_278F4(OMOBJ *work);
void fn_1_27AA8(OMOBJ *work);
void fn_1_27C08(OMOBJ *object);
void fn_1_27C7C(void);
void fn_1_280E8(void);
void fn_1_281AC(s16 layer);
void fn_1_28288(s16 layer);
void fn_1_28364(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format, void *destination);
void fn_1_284F0(void);
void fn_1_28C98(void);
void fn_1_291E4(s32 *slotIndexOut, s32 *partnerIndexOut);
s32 fn_1_29294(void);
void fn_1_29300(s16 modelId);
void fn_1_29380(void);
void fn_1_294F4(void);
void fn_1_29A0C(void);
extern s32 lbl_1_bss_18D0;
extern s32 lbl_1_bss_18D4;
extern s32 lbl_1_bss_18D8;
extern s32 lbl_1_bss_18DC;
extern s32 lbl_1_bss_18E0[6];
extern s16 lbl_1_bss_18F8[2];
extern s16 lbl_1_bss_18FC[2];
extern s16 lbl_1_bss_1900[0x14];
extern QuizModelList lbl_1_bss_1928;
extern QuizSelectionTable lbl_1_bss_1954;
extern s32 lbl_1_bss_1A48;
extern s16 lbl_1_bss_1A4C[0x14];
extern TexturePair lbl_1_bss_1A74[20];
extern s16 lbl_1_bss_1B14;
extern s16 lbl_1_bss_1B16;
extern OMOBJ *lbl_1_bss_1B18;
extern OMOBJ *lbl_1_bss_1B1C;
extern QuizObjectStorage lbl_1_bss_1B20;

/* Sets up the quiz selection scene, models, answer animations, and scene tasks. */
void fn_1_26810(void) {
    u32 textureSize;
    s32 randomChoice;
    s32 side;
    s32 row;

    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_100 = MicQuizQuestionLevelGet();
    MicQuizCountdownInit();
    lbl_1_bss_FC = 2;
    lbl_1_bss_8C = 11.111111f;
    do {
        randomChoice = frandmod(0xB);
    } while (randomChoice == 8);
    MicQuizExpectedAnswerSet(randomChoice);
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());

    Hu3DCameraCreate(7);
    Hu3DCameraPerspectiveSet(4, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(4, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(4, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(2, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    fn_8003E488(4, &lbl_1_data_1C08);
    fn_8003E488(1, &lbl_1_data_1C24);
    fn_8003E488(2, &lbl_1_data_1C24);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0, 0, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 3;
    lbl_80266B18->y = 375.0f;
    lbl_80266BD8[0] = 5172.0f;
    lbl_80266B18[1].y = 375.0f;
    lbl_80266BD8[1] = 5172.0f;

    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_110, &lbl_1_data_470, &lbl_1_data_47C);
    Hu3DGLightStaticSet(lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_110);
    Hu3DGLightColorSet(lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1],
                       lbl_1_data_488[2], lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10, 0x10, 0x10);
    Hu3DShadowPosSet(&lbl_1_data_1C40, &lbl_1_data_1C4C, &lbl_1_data_1C58);
    Hu3DShadowTPLvlSet(0.3f);

    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x34), 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x35), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    fn_80026B64(lbl_1_bss_A4, 4);
    fn_80026B64(lbl_1_bss_A2, 4);
    lbl_1_bss_1B16 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x42), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1B16, 0x40000001);
    fn_80026B64(lbl_1_bss_1B16, 4);
    lbl_1_bss_1B14 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x49), 0x10000000, 2));
    fn_800268B0(lbl_1_bss_1B14, 1.5f, 1.5f, 1.5f);
    fn_80026760(lbl_1_bss_1B14, 0.0f, 0.0f, -200.0f);
    fn_80026B64(lbl_1_bss_1B14, 4);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2B), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2C), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2D), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A0, 4);
    fn_80026B64(lbl_1_bss_9C[0], 4);
    fn_80026B64(lbl_1_bss_9C[1], 4);
    lbl_1_bss_18FC[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x47), 0x10000000, 2));
    lbl_1_bss_18FC[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x47), 0x10000000, 2));
    fn_80026760(lbl_1_bss_18FC[0], 0.0f, 0.0f, -300.0f);
    fn_80026B64(lbl_1_bss_18FC[0], 1);
    fn_80026808(lbl_1_bss_18FC[0], 0.0f, 180.0f, 0.0f);
    fn_80026760(lbl_1_bss_18FC[1], 0.0f, 0.0f, -300.0f);
    fn_80026B64(lbl_1_bss_18FC[1], 2);
    lbl_1_bss_18F8[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x48), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_18F8[0], 1);
    fn_800268B0(lbl_1_bss_18F8[0], 1.5f, 1.5f, 1.5f);
    lbl_1_bss_18F8[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x48), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_18F8[1], 2);
    fn_800268B0(lbl_1_bss_18F8[1], 1.5f, 1.5f, 1.5f);

    for (side = 0; side < 2; ++side) {
        for (row = 0; row < 0x14; ++row) {
            fn_8000C200(lbl_1_bss_1A74[row].animation[side] =
                                  HuSprAnimMake(0xB4, 0xA, 0),
                              0x10000000);
            lbl_1_bss_1A74[row].animation[side]->bmp->palData = 0;
            lbl_1_bss_1A74[row].animation[side]->bmp->palNum = 0;
            textureSize = GXGetTexBufferSize(0xB4, 0xA, 6, 0, 0);
            lbl_1_bss_1A74[row].animation[side]->bmp->data = HuMemDirectMallocNum(2,
                textureSize, 0x10000000);
        }
    }

    lbl_1_bss_1B20.object = fn_8003AB38(*lbl_1_bss_114, 0x64, 1, 3, -1, fn_1_27740);
    lbl_1_bss_1B20.object->data = HuMemDirectMallocNum(HEAP_HEAP, 36, 0x10000000);
    lbl_1_bss_1B20.object->work[0] = randomChoice;
    lbl_1_bss_1B1C = fn_8003AB38(*lbl_1_bss_114, 0x64, 1, 3, -1, fn_1_278F4);
    lbl_1_bss_1B1C->data = HuMemDirectMallocNum(HEAP_HEAP, 36, 0x10000000);
    lbl_1_bss_1B1C->work[0] = randomChoice;
    lbl_1_bss_1B18 = fn_8003AB38(*lbl_1_bss_114, 0x64, 1, 3, -1, fn_1_27AA8);
    lbl_1_bss_1B18->data = HuMemDirectMallocNum(HEAP_HEAP, 36, 0x10000000);
    lbl_1_bss_1B18->work[0] = randomChoice;

    for (row = 0; row < 0x14; ++row) {
        lbl_1_bss_1928.models[row] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x45), 0x10000000,
                2));
        fn_80026B64(lbl_1_bss_1928.models[row], 4);
        lbl_1_bss_1A4C[row] = fn_80030B2C(lbl_1_bss_1A74[row].animation[0],
                                             lbl_1_bss_1928.models[row], "v77_front");
        fn_80026760(lbl_1_bss_1928.models[row], 0.0f,
            75.0f + (375.0f - (18.75f * (f32)row) - 9.375f), -200.0f);
        fn_800268B0(lbl_1_bss_1928.models[row], 1.5f, 1.5f, 1.5f);
        lbl_1_bss_1900[row] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x46), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_1900[row], 4);
        fn_80026760(lbl_1_bss_1900[row], 0.0f,
            75.0f + (375.0f - (18.75f * (f32)row) - 9.375f), -200.0f);
        fn_800268B0(lbl_1_bss_1900[row], 1.5f, 1.5f, 1.5f);
        fn_800269E8(lbl_1_bss_1900[row], 1);
        lbl_1_bss_1954.entries[row].slotIndex = row;
        lbl_1_bss_1954.entries[row].partnerIndex = -1;
        lbl_1_bss_1954.entries[row].selected = 0;
    }

    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(3);
    fn_8002583C(4, 0, fn_1_281AC);
    fn_8002583C(2, 0, fn_1_28288);
    lbl_1_bss_18D8 = 1;
    fn_8000DAC0(fn_1_27C7C, 0x64, 0x1800, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FIFTH);
}

/* Creates a character model with its motions at its place and darkens its materials. */
void fn_1_27740(OMOBJ *work) {
    QuizCharacterModel *model;
    s32 motionIndex;

    model = work->data;
    model->modelId = fn_80025A04(fn_80008DD0(lbl_1_data_1BAC[work->work[0]], 0x10000000, 2));
    fn_1_29300(model->modelId);
    for (motionIndex = 0; motionIndex < 3; motionIndex++) {
        model->motions[motionIndex] = fn_80030458(model->modelId,
                fn_80008DD0(lbl_1_data_1A5C[work->work[0]].dataNumbers[motionIndex], 0x10000000,
                            2));
    }
    model->state = 0;
    model->characterId = work->work[0];
    model->positionX = 0.0f;
    model->positionY = 0.0f;
    model->positionZ = 0.0f;
    fn_800269E8(model->modelId, 0x40000001);
    fn_8002C95C(model->modelId, model->motions[0]);
    fn_80026760(model->modelId, model->positionX, model->positionY, model->positionZ);
    fn_80026808(model->modelId, 0.0f, 0.0f, 0.0f);
    fn_800269E8(model->modelId, 1);
    fn_80026B64(model->modelId, 1);
    fn_800268B0(model->modelId, 1.5f, 1.5f, 1.5f);
    work->objFunc = fn_1_27C08;
}

/* Creates the second character model with its motions at its place and darkens its materials. */
void fn_1_278F4(OMOBJ *work) {
    QuizCharacterModel *model;
    s32 motionIndex;

    model = work->data;
    model->modelId = fn_80025A04(fn_80008DD0(lbl_1_data_1BAC[work->work[0]], 0x10000000, 2));
    fn_1_29300(model->modelId);
    for (motionIndex = 0; motionIndex < 3; motionIndex++) {
        model->motions[motionIndex] = fn_80030458(model->modelId,
                fn_80008DD0(lbl_1_data_1A5C[work->work[0]].dataNumbers[motionIndex], 0x10000000,
                            2));
    }
    model->state = 0;
    model->characterId = work->work[0];
    model->positionX = 0.0f;
    model->positionY = 0.0f;
    model->positionZ = 0.0f;
    fn_800269E8(model->modelId, 0x40000001);
    fn_8002C95C(model->modelId, model->motions[0]);
    fn_80026760(model->modelId, model->positionX, model->positionY, model->positionZ);
    fn_80026808(model->modelId, 0.0f, 0.0f, 0.0f);
    fn_800269E8(model->modelId, 1);
    fn_80026B64(model->modelId, 2);
    fn_800268B0(model->modelId, 1.5f, 1.5f, 1.5f);
    work->objFunc = fn_1_27C08;
}

/* Creates a player character's model at its place. */
void fn_1_27AA8(OMOBJ *work) {
    QuizCharacterModel *model;

    model = work->data;
    model->modelId = fn_8005C4E4((s16)work->work[0], 4, lbl_1_data_1BF8, &model->motions[0]);
    fn_8005DEB0((s16)work->work[0], (s16)lbl_1_data_1B04[work->work[0]].voiceId, 0);
    model->state = 0;
    model->characterId = work->work[0];
    model->positionX = 0.0f;
    model->positionY = 75.0f;
    model->positionZ = -200.0f;
    fn_800269E8(model->modelId, 0x40000001);
    fn_8002C95C(model->modelId, model->motions[0]);
    fn_80026760(model->modelId, model->positionX, model->positionY, model->positionZ);
    fn_80026808(model->modelId, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model->modelId, 4);
    fn_800268B0(model->modelId, 1.5f, 1.5f, 1.5f);
    work->objFunc = fn_1_27C08;
}

/* Updates a character model's position each frame and keeps it unturned. */
void fn_1_27C08(OMOBJ *object) {
    QuizCharacterModel *work;

    work = object->data;
    fn_80026760(work->modelId,
        work->positionX, work->positionY,
        work->positionZ);
    fn_80026808(work->modelId, 0.0f,
        0.0f, 0.0f);
}

void fn_1_27C7C(void) {
    MicQuizResponseRun(3);
}

/* Runs the pairing round's mode steps: the question message, the cubes, the answer and the
   closing wipe. */
void fn_1_27CA0(OMOBJ *object) {
    s32 currentMode;
    HUPROCESS *childProcess = NULL;

    currentMode = MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 2:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            fn_1_1B28();
        } else {
            lbl_1_bss_104 += 1.0f;
            fn_1_280E8();
        }
        break;
    case 3:
        break;
    case 4:
        childProcess = fn_8000DAC0(fn_1_284F0, 0x64, 0x1800, 0, lbl_1_bss_114[0]);
        childProcess->property = (void *)lbl_1_data_1BE4[lbl_1_bss_100];
        lbl_1_bss_1A48 = 0;
        lbl_1_bss_FC = 5;
        break;
    case 5:
        if (lbl_1_bss_1A48 != 0) {
            lbl_1_bss_FC = 0;
        }
        break;
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        break;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_FC = 6;
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_18DC = (s32)fn_8000DAC0(fn_1_28C98, 0x64, 0x1800, 0, lbl_1_bss_114[0]);
            lbl_1_bss_18E0[0] = 1;
            MicQuizQuestionReadySet();
        }
        break;
    case 6:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 10;
        } else if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 8;
        }
        if (currentMode == 1) {
            lbl_1_bss_18E0[0] = 0;
        } else if (currentMode == 0) {
            lbl_1_bss_18E0[0] = 1;
        }
        break;
    case 7:
        break;
    case 8:
        fn_8000DAC0(fn_1_29380, 0x64, 0x1800, 0, lbl_1_bss_114[0]);
        lbl_1_bss_FC = 9;
        lbl_1_bss_18E0[0] = 0;
        break;
    case 9:
        if (lbl_1_bss_18D0 != 0) {
            MicQuizListenStart();
            lbl_1_bss_FC = 6;
            lbl_1_bss_18E0[0] = 1;
        }
        break;
    case 10:
        lbl_1_bss_18E0[0] = 2;
        lbl_1_bss_FC = 11;
        break;
    case 11:
        if (lbl_1_bss_1A48 != 0) {
            fn_8000DAC0(fn_1_294F4, 0x64, 0x1800, 0, lbl_1_bss_114[0]);
            lbl_1_bss_FC = 12;
        }
        break;
    case 12:
        if (lbl_1_bss_18D4 != 0) {
            WipeCreate(2, 0, 20);
            HuAudSStreamFadeOut(lbl_1_bss_20, 500);
            lbl_1_bss_FC = 13;
        }
        break;
    case 13:
        if (WipeCheck() == 0) {
            fn_800258A8(4, 0);
            fn_800258A8(2, 0);
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport(" CUBE END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

void fn_1_280E8(void)
{
    Point3d positions[2];
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

/* Layer hook: draws the twenty entries from their first image while the entries are shown. */
void fn_1_281AC(s16 layer) {
    s32 index;

    if (lbl_1_bss_18D8 == 0) {
        fn_8002BF48();
        return;
    }

    for (index = 0; index < 0x14; index++) {
        fn_1_28364(
            (s16)(lbl_1_data_19BC[index].x + 0x140),
            (s16)(lbl_1_data_19BC[index].y + 0xF0),
            lbl_1_data_19BC[index].width,
            lbl_1_data_19BC[index].height,
            6,
            lbl_1_bss_1A74[index].animation[0]->bmp->data);
    }
    fn_8002BF48();
}

/* Layer hook: draws the twenty entries from their second image while the entries are shown. */
void fn_1_28288(s16 layer) {
    s32 index;

    if (lbl_1_bss_18D8 == 0) {
        fn_8002BF48();
        return;
    }

    for (index = 0; index < 0x14; index++) {
        fn_1_28364(
            (s16)(lbl_1_data_19BC[index].x + 0x140),
            (s16)(lbl_1_data_19BC[index].y + 0xF0),
            lbl_1_data_19BC[index].width,
            lbl_1_data_19BC[index].height,
            6,
            lbl_1_bss_1A74[index].animation[1]->bmp->data);
    }
    fn_8002BF48();
}

void fn_1_28364(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format,
                void *destination) {
    GXSetTexCopySrc((u16)x, (u16)y, (u16)width, (u16)height);
    GXSetTexCopyDst((u16)width, (u16)height, format, 0);
    GXCopyTex(destination, 0);
}

/* Picks two different random entries that are still free. */
void fn_1_283E4(s32 *firstIndexOut, s32 *secondIndexOut) {
    s32 firstIndex;
    s32 secondIndex;

    for (;;) {
        firstIndex = frandmod(20);
        secondIndex = frandmod(20);
        if (firstIndex == secondIndex) {
            continue;
        }
        if (lbl_1_bss_1954.entries[firstIndex].slotIndex == -1) {
            continue;
        }
        if (lbl_1_bss_1954.entries[secondIndex].slotIndex == -1) {
            continue;
        }
        if (lbl_1_bss_1954.entries[firstIndex].selected != 0) {
            continue;
        }
        if (lbl_1_bss_1954.entries[secondIndex].selected != 0) {
            continue;
        }
        break;
    }

    *firstIndexOut = firstIndex;
    *secondIndexOut = secondIndex;
    lbl_1_bss_1954.entries[firstIndex].selected = 1;
    lbl_1_bss_1954.entries[secondIndex].selected = 1;
}

/* Flip selected answer cards through a half turn, then reveal the full set. */
void fn_1_284F0(void) {
    Point3d firstPosition;
    Point3d secondPosition;
    s32 firstIndex;
    s32 secondIndex;
    s32 secondCandidate;
    s32 firstCandidate;
    s32 cardIndex;
    u32 pairCount;
    s32 cardId;
    f32 alpha;
    f32 angle;
    HUPROCESS *process;
    QuizCharacterModel *secondWork;
    QuizCharacterModel *firstWork;

    process = fn_8000DD9C();
    pairCount = (u32)process->property;
    cardIndex = 0;
    while ((u32)cardIndex < pairCount) {
        for (;;) {
            firstCandidate = frandmod(0x14);
            secondCandidate = frandmod(0x14);
            if (firstCandidate == secondCandidate) {
                continue;
            }
            if (lbl_1_bss_1954.entries[firstCandidate].slotIndex == -1) {
                continue;
            }
            if (lbl_1_bss_1954.entries[secondCandidate].slotIndex == -1) {
                continue;
            }
            if (lbl_1_bss_1954.entries[firstCandidate].selected != 0) {
                continue;
            }
            if (lbl_1_bss_1954.entries[secondCandidate].selected == 0) {
                break;
            }
        }
        firstIndex = firstCandidate;
        secondIndex = secondCandidate;
        lbl_1_bss_1954.entries[firstCandidate].selected = 1;
        lbl_1_bss_1954.entries[secondCandidate].selected = 1;
        fn_800267CC(lbl_1_bss_1928.models[firstIndex], &firstPosition);
        fn_800267CC(lbl_1_bss_1928.models[secondIndex], &secondPosition);
        angle = 0.0f;
        alpha = 0.0f;
        fn_80026A44(lbl_1_bss_1900[firstIndex], 1);
        fn_80026A44(lbl_1_bss_1900[secondIndex], 1);
        fn_80026CBC(lbl_1_bss_1900[firstIndex], alpha);
        fn_80026CBC(lbl_1_bss_1900[secondIndex], alpha);
        HuAudFXPlay(0x52F);
        do {
            HuPrcVSleep();
            alpha = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1900[firstIndex], alpha);
            fn_80026CBC(lbl_1_bss_1900[secondIndex], alpha);
            angle += 10.0f;
        } while (!(angle >= 90.0f));

        fn_80026790(lbl_1_bss_1928.models[firstIndex], &secondPosition);
        fn_80026790(lbl_1_bss_1928.models[secondIndex], &firstPosition);
        fn_80026790(lbl_1_bss_1900[firstIndex], &secondPosition);
        fn_80026790(lbl_1_bss_1900[secondIndex], &firstPosition);
        lbl_1_bss_1954.entries[firstIndex].partnerIndex =
                lbl_1_bss_1954.entries[secondIndex].slotIndex;
        lbl_1_bss_1954.entries[secondIndex].partnerIndex =
                lbl_1_bss_1954.entries[firstIndex].slotIndex;

        cardId = lbl_1_bss_1954.entries[firstIndex].slotIndex;
        fn_80031010(lbl_1_bss_1A4C[cardId]);
        lbl_1_bss_1A4C[cardId] = fn_80030B2C(
            lbl_1_bss_1A74[cardId].animation[1],
            lbl_1_bss_1928.models[cardId],
            "v77_front");

        cardId = lbl_1_bss_1954.entries[secondIndex].slotIndex;
        fn_80031010(lbl_1_bss_1A4C[cardId]);
        lbl_1_bss_1A4C[cardId] = fn_80030B2C(
            lbl_1_bss_1A74[cardId].animation[1],
            lbl_1_bss_1928.models[cardId],
            "v77_front");

        angle = 90.0f;
        alpha = 1.0f;
        fn_80026A44(lbl_1_bss_1900[firstIndex], 1);
        fn_80026A44(lbl_1_bss_1900[secondIndex], 1);
        fn_80026CBC(lbl_1_bss_1900[firstIndex], alpha);
        fn_80026CBC(lbl_1_bss_1900[secondIndex], alpha);
        do {
            HuPrcVSleep();
            alpha = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1900[firstIndex], alpha);
            fn_80026CBC(lbl_1_bss_1900[secondIndex], alpha);
            angle -= 10.0f;
        } while (!(angle <= 0.0f));
        cardIndex += 1;
    }

    angle = 0.0f;
    alpha = 0.0f;
    cardIndex = 0;
    while (cardIndex < 0x14) {
        fn_80026A44(lbl_1_bss_1900[cardIndex], 1);
        fn_80026CBC(lbl_1_bss_1900[cardIndex], alpha);
        cardIndex += 1;
    }
    HuAudFXPlay(0x530);
    do {
        HuPrcVSleep();
        alpha = (f32)sin((M_PI * angle) / 180.0);
        cardIndex = 0;
        while (cardIndex < 0x14) {
            fn_80026CBC(lbl_1_bss_1900[cardIndex], alpha);
            cardIndex += 1;
        }
        angle += 3.0f;
    } while (!(angle >= 90.0f));

    fn_800269E8(lbl_1_bss_18F8[0], 1);
    fn_800269E8(lbl_1_bss_18F8[1], 1);
    secondWork = lbl_1_bss_1B20.object->data;
    firstWork = lbl_1_bss_1B1C->data;
    fn_80026A44(secondWork->modelId, 1);
    fn_80026A44(firstWork->modelId, 1);

    angle = 90.0f;
    alpha = 1.0f;
    cardIndex = 0;
    while (cardIndex < 0x14) {
        fn_80026A44(lbl_1_bss_1900[cardIndex], 1);
        fn_80026CBC(lbl_1_bss_1900[cardIndex], alpha);
        cardIndex += 1;
    }
    do {
        HuPrcVSleep();
        alpha = (f32)sin((M_PI * angle) / 180.0);
        cardIndex = 0;
        while (cardIndex < 0x14) {
            fn_80026CBC(lbl_1_bss_1900[cardIndex], alpha);
            cardIndex += 1;
        }
        angle -= 3.0f;
    } while (!(angle <= 0.0f));

    cardIndex = 0;
    while (cardIndex < 0x14) {
        fn_80026CBC(lbl_1_bss_1900[cardIndex], 0.0f);
        cardIndex += 1;
    }
    lbl_1_bss_1A48 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Swaps the matched pair's positions and motions, then waits for the next match. */
void fn_1_28C98(void) {
    Point3d firstPosition;
    Point3d secondPosition;
    s32 firstIndex;
    s32 secondIndex;
    s32 slotIndex;
    f32 opacity;
    f32 angle;

    for (;;) {
        fn_1_291E4(&firstIndex, &secondIndex);
        fn_800267CC(lbl_1_bss_1928.models[firstIndex], &firstPosition);
        fn_800267CC(lbl_1_bss_1928.models[secondIndex], &secondPosition);
        angle = 0.0f;
        opacity = 0.0f;
        fn_80026A44(lbl_1_bss_1900[firstIndex], 1U);
        fn_80026A44(lbl_1_bss_1900[secondIndex], 1U);
        fn_80026CBC(lbl_1_bss_1900[firstIndex], opacity);
        fn_80026CBC(lbl_1_bss_1900[secondIndex], opacity);
        HuAudFXPlay(0x531);
        do {
            HuPrcVSleep();
            opacity = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1900[firstIndex], opacity);
            fn_80026CBC(lbl_1_bss_1900[secondIndex], opacity);
            angle += 0.75f;
        } while (!(angle >= 90.0f));

        fn_80026790(lbl_1_bss_1928.models[firstIndex], &secondPosition);
        fn_80026790(lbl_1_bss_1928.models[secondIndex], &firstPosition);
        fn_80026790(lbl_1_bss_1900[firstIndex], &secondPosition);
        fn_80026790(lbl_1_bss_1900[secondIndex], &firstPosition);
        lbl_1_bss_1954.entries[firstIndex].partnerIndex =
                lbl_1_bss_1954.entries[firstIndex].slotIndex;
        lbl_1_bss_1954.entries[secondIndex].partnerIndex =
                lbl_1_bss_1954.entries[secondIndex].slotIndex;

        slotIndex = lbl_1_bss_1954.entries[firstIndex].slotIndex;
        fn_80031010(lbl_1_bss_1A4C[slotIndex]);
        lbl_1_bss_1A4C[slotIndex] = fn_80030B2C(lbl_1_bss_1A74[slotIndex].animation[0],
                                                   lbl_1_bss_1928.models[slotIndex], "v77_front");
        slotIndex = lbl_1_bss_1954.entries[secondIndex].slotIndex;
        fn_80031010(lbl_1_bss_1A4C[slotIndex]);
        lbl_1_bss_1A4C[slotIndex] = fn_80030B2C(lbl_1_bss_1A74[slotIndex].animation[0],
                                                   lbl_1_bss_1928.models[slotIndex], "v77_front");

        angle = 90.0f;
        opacity = 1.0f;
        fn_80026A44(lbl_1_bss_1900[firstIndex], 1U);
        fn_80026A44(lbl_1_bss_1900[secondIndex], 1U);
        fn_80026CBC(lbl_1_bss_1900[firstIndex], opacity);
        fn_80026CBC(lbl_1_bss_1900[secondIndex], opacity);
        do {
            HuPrcVSleep();
            opacity = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1900[firstIndex], opacity);
            fn_80026CBC(lbl_1_bss_1900[secondIndex], opacity);
            angle = angle - 0.75f;
        } while (!(angle <= 0.0f));
        fn_80026CBC(lbl_1_bss_1900[firstIndex], 0.0f);
        fn_80026CBC(lbl_1_bss_1900[secondIndex], 0.0f);

        do {
            HuPrcVSleep();
        } while (lbl_1_bss_18E0[0] == 0);
        if (lbl_1_bss_18E0[0] != 2 && fn_1_29294() == 0) {
            HuPrcSleep(0x78);
            continue;
        }
        lbl_1_bss_1A48 = 1;
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
}

/* Picks a random entry that has no partner yet and returns it with its partner. */
void fn_1_291E4(s32 *slotIndexOut, s32 *partnerIndexOut) {
    s32 slotIndex;

    for (;;) {
        slotIndex = frandmod(20);
        if (lbl_1_bss_1954.entries[slotIndex].partnerIndex == -1) {
            continue;
        }
        if (lbl_1_bss_1954.entries[slotIndex].slotIndex
            == lbl_1_bss_1954.entries[slotIndex].partnerIndex) {
            continue;
        }
        break;
    }

    *slotIndexOut = slotIndex;
    *partnerIndexOut = lbl_1_bss_1954.entries[slotIndex].partnerIndex;
}

/* Returns 1 once every paired entry points at a partner other than itself. */
s32 fn_1_29294(void) {
    s32 index;

    for (index = 0; index < 20; index++) {
        if (lbl_1_bss_1954.entries[index].partnerIndex != -1 &&
            index != lbl_1_bss_1954.entries[index].partnerIndex) {
            return 0;
        }
    }
    return 1;
}

/* Sets every material of a model to black. */
void fn_1_29300(s16 modelId) {
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

/* Plays random idle motions on the two character models for a while, then ends this process. */
void fn_1_29380(void) {
    QuizCharacterModel *firstWork = lbl_1_bss_1B20.object->data;
    QuizCharacterModel *secondWork = lbl_1_bss_1B1C->data;
    s32 randomValue;
    s16 motion;

    randomValue = frandmod(2);
    if (randomValue == 0) {
        motion = 1;
    } else {
        motion = 2;
    }
    fn_8002CCC0(firstWork->modelId,
                       firstWork->motions[motion],
                       0.0f, 8.0f, 0);
    fn_8002CCC0(secondWork->modelId,
                       firstWork->motions[motion],
                       0.0f, 8.0f, 0);
    HuPrcSleep(8);
    HuPrcSleep(120);
    HuPrcSleep(120);

    motion = 0;
    fn_8002CCC0(firstWork->modelId,
                       firstWork->motions[motion],
                       0.0f, 8.0f, 0x40000001);
    fn_8002CCC0(secondWork->modelId,
                       firstWork->motions[motion],
                       0.0f, 8.0f, 0x40000001);
    HuPrcSleep(18);
    lbl_1_bss_18D0 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Re-centers the revealed answers, then starts their return animations. */
void fn_1_294F4(void) {
    s32 selected[20];
    QuizCharacterModel *soundWork;
    s32 slotIndex;
    HUPROCESS *process;
    f32 opacity;
    f32 angle;
    s32 index;

    process = NULL;
    soundWork = lbl_1_bss_1B18->data;
    for (index = 0; index < 20; index++) {
        selected[index] = -1;
    }
    angle = 0.0;
    opacity = 0.0f;
    HuAudFXPlay(0x531);
    do {
        HuPrcVSleep();
        opacity = (f32)sin((M_PI * angle) / 180.0);
        for (index = 0; index < 20; index++) {
            if (lbl_1_bss_1954.entries[index].partnerIndex != -1 &&
                lbl_1_bss_1954.entries[index].slotIndex != lbl_1_bss_1954.entries[
                        index].partnerIndex) {
                fn_80026CBC(lbl_1_bss_1900[index], opacity);
                selected[index] = 1;
            }
        }
        angle += 0.75f;
    } while (!(angle >= 90.0f));

    for (index = 0; index < 20; index++) {
        fn_80026760(lbl_1_bss_1928.models[index], 0.0f,
                        75.0f + (375.0f - (18.75f * (f32)index) - 9.375f), -200.0f);
        fn_80026760(lbl_1_bss_1900[index], 0.0f,
                        75.0f + (375.0f - (18.75f * (f32)index) - 9.375f), -200.0f);
        slotIndex = lbl_1_bss_1954.entries[index].slotIndex;
        fn_80031010(lbl_1_bss_1A4C[slotIndex]);
        lbl_1_bss_1A4C[index] = fn_80030B2C(lbl_1_bss_1A74[slotIndex].animation[0],
                                              lbl_1_bss_1928.models[slotIndex], "v77_front");
        lbl_1_bss_1954.entries[index].partnerIndex = lbl_1_bss_1954.entries[index].slotIndex;
    }

    angle = 90.0;
    opacity = 1.0f;
    do {
        HuPrcVSleep();
        opacity = (f32)sin((M_PI * angle) / 180.0);
        for (index = 0; index < 20; index++) {
            if (selected[index] == 1) {
                fn_80026CBC(lbl_1_bss_1900[index], opacity);
            }
        }
        angle += 0.75f;
    } while (!(angle >= 180.0f));

    for (index = 0; index < 20; index++) {
        process = fn_8000DAC0(fn_1_29A0C, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
        process->property = (void *)index;
        HuPrcSleep(5);
    }
    HuPrcSleep(0x3C);
    lbl_1_bss_18D8 = 0;
    for (index = 0; index < 20; index++) {
        fn_800262D0(lbl_1_bss_1928.models[index]);
    }
    Hu3DCameraScissorSet(1, 0U, 0U, 0U, 0U);
    Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
    HuAudFXPlay(lbl_1_data_458[soundWork->characterId]);
    fn_8002CCC0(soundWork->modelId, soundWork->motions[1],
                       0.0f, 8.0f, 0U);
    HuPrcSleep(0x78);
    HuPrcSleep(0x3C);
    lbl_1_bss_18D4 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Raises the chosen model out of the scene with a sound, hides it and ends this process. */
void fn_1_29A0C(void) {
    HUPROCESS *process = fn_8000DD9C();
    s32 current = (s32)process->property;
    HuVecF position;

    fn_800267CC(lbl_1_bss_1928.models[current], &position);
    {
        f32 angle = 0.0f;

        HuAudFXPlay(0x532);
        for (;;) {
            f32 offset;

            HuPrcVSleep();
            offset = (f32)(1000.0 *
                           sin((M_PI * (f64)angle) /
                               180.0));
            fn_80026760(lbl_1_bss_1928.models[current], position.x,
                            position.y + offset, position.z);
            angle += 3.0f;
            if (angle >= 90.0f) {
                break;
            }
        }
    }
    fn_800269E8(lbl_1_bss_1928.models[current], (1 << 0));
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_29B20(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;
    lbl_80266BD8[2] += (f32) (lbl_802BFFEC[0] / 5);
    lbl_80266BD8[2] -= (f32) (lbl_802BFFE8[0] / 5);
    if ((s32) (lbl_802C0010[0] & 4) != 0) {
        lbl_80266B18[2].y -= 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 8) != 0) {
        lbl_80266B18[2].y += 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 1) != 0) {
        lbl_80266B18[2].x -= 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 2) != 0) {
        lbl_80266B18[2].x += 5.0f;
    }
    lbl_80266A58[2].x += 5.0f * ((f32) lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[2].y -= 5.0f * ((f32) (s8) lbl_802BFFF4[0] / 44.0f);
    if ((s32) (lbl_802C0008[0] & 0x800) != 0) {
        fn_8003E488(4, &lbl_1_data_1C08);
    }
    lbl_80266BD8[2] += (f32) (lbl_802BFFEC[1] / 5);
    lbl_80266BD8[2] -= (f32) (lbl_802BFFE8[1] / 5);
    if ((s32) (lbl_802C0010[1] & 4) != 0) {
        lbl_80266B18[2].y -= 5.0f;
    }
    if ((s32) (lbl_802C0010[1] & 8) != 0) {
        lbl_80266B18[2].y += 5.0f;
    }
    if ((s32) (lbl_802C0010[1] & 1) != 0) {
        lbl_80266B18[2].x -= 5.0f;
    }
    if ((s32) (lbl_802C0010[1] & 2) != 0) {
        lbl_80266B18[2].x += 5.0f;
    }
    lbl_80266A58[2].x += 5.0f * ((f32) (s8) lbl_802BFFF0[1] / 44.0f);
    lbl_80266A58[2].y -= 5.0f * ((f32) (s8) lbl_802BFFF4[1] / 44.0f);
    if ((s32) (lbl_802C0008[1] & 0x800) != 0) {
        fn_8003E488(1, &lbl_1_data_1C24);
    }
    printWin(0x10, 0x28, 0xF0, 0x54, &windowColor);
    fontcolor = 0xE;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f",
           lbl_80266B18[2].x, lbl_80266B18[2].y, lbl_80266B18[2].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f",
           lbl_80266A58[2].x, lbl_80266A58[2].y, lbl_80266A58[2].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[2]);
}

QuizObjectStorage lbl_1_bss_1B20;

OMOBJ *lbl_1_bss_1B1C;

OMOBJ *lbl_1_bss_1B18;

s16 lbl_1_bss_1B16;

s16 lbl_1_bss_1B14;

TexturePair lbl_1_bss_1A74[20];

s16 lbl_1_bss_1A4C[0x14];

s32 lbl_1_bss_1A48;

QuizSelectionTable lbl_1_bss_1954;

QuizModelList lbl_1_bss_1928;

s16 lbl_1_bss_1900[0x14];
s16 lbl_1_bss_18FC[2];
s16 lbl_1_bss_18F8[2];
s32 lbl_1_bss_18E0[6];
s32 lbl_1_bss_18DC;
s32 lbl_1_bss_18D8;
s32 lbl_1_bss_18D4;
s32 lbl_1_bss_18D0;
