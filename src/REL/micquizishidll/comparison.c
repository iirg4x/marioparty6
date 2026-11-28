/* Comparison round: objects on two sides, the players' answer and the result display. */
#include "msm_stream.h"
#include "dolphin/types.h"

#include "game/hu3d.h"

#include "game/printfunc.h"

#include "game/process.h"

#include "humath.h"

#include "game/object.h"

#include <math.h>
#include "REL/micquizishidll.h"

/* quiz.h leaves these out (other files call them without a prototype); this file's calls take
 * them with one. */
void MicQuizCountdownInit(void);
void MicQuizExpectedAnswerSet(s32 expectedAnswer);
void MicQuizMicContextSet(s32 questionType);

typedef struct QuizMotionFiles {
    s32 modelFileIds[2];
} QuizMotionFiles;

u8 lbl_1_data_14A8[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
QuizMotionFiles lbl_1_data_14C4[6] = { { { 0xA40001, 0xA40004 } }, { { 0xA40006, 0xA40009 } },
        { { 0xA4000B, 0xA4000E } }, { { 0xA40015, 0xA40018 } }, { { 0xA4001B,
                0xA4001E } }, { { 0xA40010, 0xA40013 } } };
s32 lbl_1_data_14F4[6] = { 0xA40000, 0xA40005, 0xA4000A, 0xA40014, 0xA4001A, 0xA4000F };
OM_CAMERA_VIEW lbl_1_data_150C = { { 0.0f, 210.0f, 0.0f }, { -8.57f, 0.0f, 0.0f }, 2294.0f };
u8 lbl_1_data_1528[0x24] = { 0xC2, 0xC8, 0x00, 0x00, 0x45, 0xFA, 0x00, 0x00, 0xC3, 0xC8, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xC8, 0x00, 0x00 };

OMOBJ *lbl_1_bss_1768[2][23];

s16 lbl_1_bss_1764;

s16 lbl_1_bss_1760[2];

s16 lbl_1_bss_175C[2];

s16 lbl_1_bss_1758[2];

s16 lbl_1_bss_1754[2];
u8 lbl_1_bss_1752[0x2];

s16 lbl_1_bss_1750[1];
s32 lbl_1_bss_1720[2][6];

s32 lbl_1_bss_1668[2][23];

s32 lbl_1_bss_15B0[2][23];

s32 lbl_1_bss_15A8[2];

s32 lbl_1_bss_15A4;

s32 lbl_1_bss_15A0;

ANIMDATA *lbl_1_bss_159C;

s16 lbl_1_bss_159A;

s16 lbl_1_bss_1598;

char *lbl_1_data_1584[18] = { "2", "4", "6", "8", "10", "11", "o1", "o2", "o3", "o4", "o5", "o6",
                             "o7", "o8", "o9", "o10", "o11", "o12" };
MicquizSelectionConfig lbl_1_data_15CC[5] = { { 5, 2, 3 }, { 5, 4, 6 }, { 5, 7, 9 }, { 5, 10, 12 },
                                             { 1, 13, 15 } };
s32 lbl_1_data_1608[5] = { 1, 2, 3, 4, 5 };

void fn_1_20B70(void);
void fn_1_211C4(u8 firstFlag, u8 secondFlag);
void fn_1_21230(void);
void fn_1_21568(void);
void fn_1_21604(void);
void fn_1_216A0(void);
void fn_1_21764(OMOBJ *object);
void fn_1_21B14(OMOBJ *object);
void fn_1_21E00(void);
void fn_1_22058(void);
s32 fn_1_2233C(s32 value, s32 groupIndex);
s32 fn_1_223B8(s32 value, s32 groupIndex);
void fn_1_22434(void);
void fn_1_22458(void);
void fn_1_225AC(HU3D_MODELID modelId, char *objName, Vec *position);

/* Sets up the comparison round: picks the left and right answers, creates the camera, lights and
   models, and reports which side is correct. */
void fn_1_1FB04(void) {
    s32 player;
    s32 slot;
    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_15A4 = 0;
    lbl_1_bss_100 = MicQuizQuestionLevelGet();
    MicQuizCountdownInit();
    lbl_1_bss_FC = 0;
    lbl_1_bss_8C = 13.333333f;
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_150C);
    lbl_1_bss_A8 = fn_8003AB38(* lbl_1_bss_114, 0x7FDA, 0, 0, - 1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_110, & lbl_1_data_470, & lbl_1_data_47C);
    Hu3DGLightStaticSet(lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_110);
    Hu3DGLightColorSet(lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x59), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A4, 1);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2B), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2C), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2D), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A0, 1);
    fn_80026B64(lbl_1_bss_9C[0], 1);
    fn_80026B64(lbl_1_bss_9C[1], 1);
    lbl_1_bss_1760[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x55), 0x10000000, 2));
    lbl_1_bss_1760[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x56), 0x10000000, 2));
    lbl_1_bss_175C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x57), 0x10000000, 2));
    lbl_1_bss_175C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x58), 0x10000000, 2));
    fn_80026760(lbl_1_bss_1760[0], - 350.0f, 0.0f, - 450.0f);
    fn_80026760(lbl_1_bss_1760[1], 350.0f, 0.0f, - 450.0f);
    fn_80026760(lbl_1_bss_175C[0], - 350.0f, 0.0f, - 450.0f);
    fn_80026760(lbl_1_bss_175C[1], 350.0f, 0.0f, - 450.0f);
    lbl_1_bss_1758[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5A), 0x10000000, 2));
    lbl_1_bss_1758[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5B), 0x10000000, 2));
    lbl_1_bss_1754[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5C), 0x10000000, 2));
    lbl_1_bss_1754[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5D), 0x10000000, 2));
    fn_80026760(lbl_1_bss_1758[0], - 350.0f, 450.0f, - 450.0f);
    fn_80026760(lbl_1_bss_1758[1], 350.0f, 450.0f, - 450.0f);
    fn_800269E8(lbl_1_bss_1758[0], ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1758[1], ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_1754[0], - 350.0f, 450.0f, - 450.0f);
    fn_80026760(lbl_1_bss_1754[1], 350.0f, 450.0f, - 450.0f);
    fn_800269E8(lbl_1_bss_1754[0], ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1754[1], ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1754[0], ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_1754[1], ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_1754[0], 1);
    fn_800269E8(lbl_1_bss_1754[1], 1);
    lbl_1_bss_1750[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5E), 0x10000000, 2));
    lbl_1_bss_1750[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x5E), 0x10000000, 2));
    fn_80026760(lbl_1_bss_1750[0], - 350.0f, 450.0f, - 450.0f);
    fn_80026760(lbl_1_bss_1750[1], 350.0f, 450.0f, - 450.0f);
    fn_800269E8(lbl_1_bss_1750[0], ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1750[1], ((1 << 30) | (1 << 0)));
    fn_800268B0(lbl_1_bss_1750[0], 0.01f, 1.0f, 1.0f);
    fn_800268B0(lbl_1_bss_1750[1], 0.01f, 1.0f, 1.0f);
    fn_800269E8(lbl_1_bss_1750[0], 1);
    fn_800269E8(lbl_1_bss_1750[1], 1);
    lbl_1_bss_15A8[0] = MicQuizRandRange(lbl_1_data_15CC[lbl_1_bss_100].minCount,
                                         lbl_1_data_15CC[lbl_1_bss_100].maxCountExclusive);
    lbl_1_bss_15A8[1] = lbl_1_bss_15A8[0];
    if (frandmod(2) != 0U)
    {
        lbl_1_bss_15A8[1]--;
        lbl_1_bss_1764 = 0;
    }
    else
    {
        lbl_1_bss_15A8[0]--;
        lbl_1_bss_1764 = 1;
    }
    MicQuizExpectedAnswerSet(lbl_1_bss_1764);
    fn_1_22058();
    for (player = 0; player < 2; ++player)
    {
        for (slot = 0; slot < 23; ++slot)
        {
            lbl_1_bss_1768[player][slot] = NULL;
        }
    }
    for (player = 0; player < 2; ++player)
    {
        for (slot = 0; slot < lbl_1_bss_15A8[player]; ++slot)
        {
            lbl_1_bss_1768[player][slot] = fn_8003AB38(* lbl_1_bss_114, 0x64, 1, 3, - 1,
                                                       fn_1_21764);
            lbl_1_bss_1768[player][slot]->data = HuMemDirectMallocNum(HEAP_HEAP, 0x6C, 0x10000000);
            lbl_1_bss_1768[player][slot]->work[0] = lbl_1_bss_1668[player][slot];
            lbl_1_bss_1768[player][slot]->work[1] = lbl_1_bss_15B0[player][slot];
            lbl_1_bss_1768[player][slot]->work[2] = slot * player;
            lbl_1_bss_1768[player][slot]->work[3] = player;
        }
    }
    if (lbl_1_bss_15A8[0] > lbl_1_bss_15A8[1])
    {
        OSReport("COMP ANSER:LEFT\n");
    }
    else
    {
        OSReport("COMP ANSER:RIGHT\n");
    }
    OSReport("COMP LEFT:%d RIGHT:%d\n", lbl_1_bss_15A8[0], lbl_1_bss_15A8[1]);
    lbl_1_bss_159C = HuSprAnimDataRead(DATANUM(0xa40000, 0x5F));
    lbl_1_bss_159A = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x60), 0x10000000, 2));
    lbl_1_bss_1598 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x61), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_159A, 1);
    fn_800269E8(lbl_1_bss_1598, 1);
    MicQuizPauseInit(* lbl_1_bss_114);
    MicQuizMicContextSet(5);
    fn_8000DAC0(fn_1_22458, 0x64, 0x1800, 0, * lbl_1_bss_114);
    fn_8000DAC0(fn_1_22434, 0x64, 0x1800, 0, * lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FOURTH);
}

/* Runs the comparison round's mode steps: the question message, the objects, the answer, and the
   closing wipe. */
void fn_1_20804(OMOBJ *object) {
    MicQuizCountdownUpdate();

    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        return;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_FC = 3;
            lbl_1_bss_104 = 0.0f;
            fn_1_2160();
        }
        break;
    case 3:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            MicQuizQuestionReadySet();
        } else {
            lbl_1_bss_104 += 1.0f;
            fn_1_216A0();
        }
        /* fall through */
    case 4:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 7;
            lbl_1_bss_15A0 = 0;
            fn_8000DAC0(fn_1_20B70, 0x64, 0x1800, 0,
                             fn_8000DD9C());
            return;
        }
        if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 5;
            return;
        }
        DebugValue(MicQuizResponseStateGet() == 0);
        return;
    case 5:
        lbl_1_bss_FC = 6;
        fn_1_21568();
        return;
    case 6:
        if (lbl_1_bss_104 >= 300.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            fn_1_21604();
            MicQuizListenStart();
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 7:
        if (lbl_1_bss_15A0 != 0) {
            lbl_1_bss_FC = 8;
            return;
        }
        break;
    case 8:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 9;
            WipeCreate(2, 0, 20);
            HuAudSStreamFadeOut(lbl_1_bss_20, 500);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 9:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport(" COMP END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Plays the answer: the players' motions and sounds, moves the objects, then starts the result
   display. */
void fn_1_20B70(void) {
    Point3d secondRotation;
    Point3d firstRotation;
    f32 scale;
    s16 modelId0;
    s16 modelId1;
    s16 angle;
    s32 soundId;
    QuizMotionWork *firstPlayerData;
    QuizMotionWork *secondPlayerData;
    s32 maxPlayerCount;
    s32 playerIndex;
    s32 rotationFrame;
    s32 playerNumber;

    modelId0 = lbl_1_bss_1758[0];
    modelId1 = lbl_1_bss_1758[1];
    angle = 0;
    scale = 0.0f;
    lbl_1_bss_15A4 = 2;
    fn_80026874(lbl_1_bss_1760[0], &firstRotation);
    fn_80026874(lbl_1_bss_1760[1], &secondRotation);
    for (rotationFrame = 0; rotationFrame < 150; rotationFrame++) {
        HuPrcVSleep();
        firstRotation.y = fn_1_2AD8(firstRotation.y, 0.0f, 0.04f);
        secondRotation.y = fn_1_2AD8(secondRotation.y, 0.0f, 0.04f);
        fn_80026808(lbl_1_bss_1760[0], firstRotation.x, firstRotation.y,
                        firstRotation.z);
        fn_80026808(lbl_1_bss_1760[1], secondRotation.x, secondRotation.y,
                        secondRotation.z);
    }
    firstRotation.y = 0.0f;
    secondRotation.y = 0.0f;
    fn_80026808(lbl_1_bss_1760[0], firstRotation.x, firstRotation.y,
                    firstRotation.z);
    fn_80026808(lbl_1_bss_1760[1], secondRotation.x, secondRotation.y,
                    secondRotation.z);
    fn_1_21E00();
    fn_80026A44(modelId0, ((1 << 30) | (1 << 0)));
    fn_80026A44(modelId1, ((1 << 30) | (1 << 0)));
    do {
        HuPrcVSleep();
    } while (fn_8002D75C(modelId0) == 0 ||
             fn_8002D75C(modelId1) == 0);
    fn_8002D414(modelId0, 0.0f);
    fn_8002D414(modelId1, 0.0f);
    soundId = HuAudFXPlay(0x52C);
    fn_80026A44(lbl_1_bss_1750[0], 1);
    fn_80026A44(lbl_1_bss_1750[1], 1);
    angle = 0;

    while (1) {
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)angle) / 180.0);
        angle += 1.0f;
        if (angle >= 90.0f) {
            break;
        }
        fn_800268B0(lbl_1_bss_1750[0], scale, 1.0f, 1.0f);
        fn_800268B0(lbl_1_bss_1750[1], scale, 1.0f, 1.0f);
    }

    if (lbl_1_bss_15A8[0] > lbl_1_bss_15A8[1]) {
        maxPlayerCount = lbl_1_bss_15A8[0];
    } else {
        maxPlayerCount = lbl_1_bss_15A8[1];
    }
    playerNumber = 1;
    for (playerIndex = 0; playerIndex < maxPlayerCount; playerIndex++) {
        HuAudFXPlay(0x52D);
        if (lbl_1_bss_1768[0][playerIndex] != NULL) {
            fn_1_211C4((u8)playerNumber, 0);
            firstPlayerData = lbl_1_bss_1768[0][playerIndex]->data;
            firstPlayerData->motionState = 4;
        }
        if (lbl_1_bss_1768[1][playerIndex] != NULL) {
            fn_1_211C4((u8)playerNumber, 1);
            secondPlayerData = lbl_1_bss_1768[1][playerIndex]->data;
            secondPlayerData->motionState = 4;
        }
        playerNumber++;
        HuPrcSleep(45);
    }
    HuPrcSleep(60);
    angle = 90;
    while (1) {
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)angle) / 180.0);
        angle -= 1.0f;
        if (angle <= 0.0f) {
            break;
        }
        fn_800268B0(lbl_1_bss_1750[0], scale, 1.0f, 1.0f);
        fn_800268B0(lbl_1_bss_1750[1], scale, 1.0f, 1.0f);
    }
    fn_800269E8(lbl_1_bss_1750[0], 1);
    fn_800269E8(lbl_1_bss_1750[1], 1);
    HuAudFXStop(soundId);
    fn_800269E8(lbl_1_bss_1758[0], 1);
    fn_800269E8(lbl_1_bss_1758[1], 1);
    fn_80026A44(lbl_1_bss_1754[0], 1);
    fn_80026A44(lbl_1_bss_1754[1], 1);
    fn_80026A44(lbl_1_bss_1754[0], ((1 << 30) | (1 << 1)));
    fn_80026A44(lbl_1_bss_1754[1], ((1 << 30) | (1 << 1)));
    lbl_1_bss_15A0 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Starts the result display process with two flags packed into its property. */
void fn_1_211C4(u8 firstFlag, u8 secondFlag) {
    HUPROCESS *process = 0;

    process = fn_8000DAC0(fn_1_21230, 0x64, 0x1800, 0,
        fn_8000DD9C());
    process->property = (void *)((firstFlag << 16) | (u16)secondFlag);
}

/* Shows the result: switches the counter models' animation banks and swings them. */
void fn_1_21230(void) {
    HUPROCESS *process;
    u32 selection;
    s16 model;
    u16 onesDigit;
    s16 tensAnimation;
    s16 onesAnimation;
    s32 frame;
    s32 tensValue;
    u8 tensDigit;
    Vec position;
    f32 angle;
    f32 scaleX;
    f32 scaleY;
    f32 transparency;
    f32 risePerFrame;
    f32 initialValue;
    s32 unitsDigit;

    process = fn_8000DD9C();
    selection = (u32)process->property;
    tensDigit = (u8)(selection >> 16);
    onesDigit = (u16)selection;
    initialValue = 178.0f;
    scaleX = scaleY = 0.0f;
    transparency = 1.0f;
    risePerFrame = 2.2222223f;
    frame = 0;
    angle = 0.0f;

    if (tensDigit >= 10) {
        model = fn_80025D4C(lbl_1_bss_1598);
        fn_80026A44(model, 1);
        tensAnimation = fn_80030B2C(lbl_1_bss_159C, model, "10fig");
        onesAnimation = fn_80030B2C(lbl_1_bss_159C, model, "1fig");
        tensValue = tensDigit / 10;
        unitsDigit = tensDigit - (tensValue * 10);
        Hu3DAnimBankSet(tensAnimation, (u16)tensValue);
        Hu3DAnimBankSet(onesAnimation, (u16)unitsDigit);
    } else {
        model = fn_80025D4C(lbl_1_bss_159A);
        fn_80026A44(model, 1);
        onesAnimation = fn_80030B2C(lbl_1_bss_159C, model, "1fig");
        Hu3DAnimBankSet(onesAnimation, tensDigit);
    }

    if (onesDigit == 0) {
        position.x = -300.0f;
        position.y = 350.0f;
        position.z = 0.0f;
    } else {
        position.x = 300.0f;
        position.y = 350.0f;
        position.z = 0.0f;
    }

    fn_80026790(model, &position);
    fn_800268B0(model, scaleX, scaleY, 1.0f);
    fn_80026CBC(model, transparency);
    for (;;) {
        HuPrcVSleep();
        position.y += risePerFrame;
        scaleX = scaleY =
            (f32)sin((M_PI * angle) / 180.0f);
        transparency = (f32)sin((M_PI * angle) / 180.0f);
        angle += 2.0f;
        fn_80026790(model, &position);
        fn_800268B0(model, scaleX, scaleY, 1.0f);
        fn_80026CBC(model, transparency);
        frame++;
        if ((f32)frame >= 90.0f) {
            break;
        }
    }
    fn_800262D0(model);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Puts every object of both sides into state 1. */
void fn_1_21568(void) {
    s32 index;
    s32 group;
    QuizMotionWork *work;
    s32 count;

    count = lbl_1_data_1608[lbl_1_bss_100];

    for (group = 0; group < 2; group++) {
        for (index = 0; index < count; index++) {
            work = lbl_1_bss_1768[group][index]->data;
            work->motionState = 1;
        }
    }
}

/* Puts every object of both sides into state 2. */
void fn_1_21604(void) {
    s32 index;
    s32 group;
    QuizMotionWork *work;
    s32 count;

    count = lbl_1_data_1608[lbl_1_bss_100];

    for (group = 0; group < 2; group++) {
        for (index = 0; index < count; index++) {
            work = lbl_1_bss_1768[group][index]->data;
            work->motionState = 2;
        }
    }
}

/* Spreads the two comparison models apart along X. */
void fn_1_216A0(void)
{
    QuizModelPairSpread(lbl_1_bss_9C, &lbl_1_bss_8C);
}

/* Sets the position and turn steps for a comparison object. */
static inline void QuizComparisonMotionStepsSet(QuizMotionWork *work)
{
    if (work->modelPairIndex == 0) {
        work->positionStep.x = (-350.0f - work->referencePosition.x) / 90.0f;
        work->positionStep.y = (450.0f - work->referencePosition.y) / 90.0f;
        work->positionStep.z = (-450.0f - work->referencePosition.z) / 90.0f;
    } else {
        work->positionStep.x = (350.0f - work->referencePosition.x) / 90.0f;
        work->positionStep.y = (450.0f - work->referencePosition.y) / 90.0f;
        work->positionStep.z = (-450.0f - work->referencePosition.z) / 90.0f;
    }
    work->unk44 = work->unk4C = 0.0f;
    work->rotationStepY = ((450.0f - work->referencePosition.y) / 2.0f) / 60.0f;
}

/* Creates one comparison object: its model, joint motions and place, then hands it to its update
   step. */
void fn_1_21764(OMOBJ *object) {
    QuizMotionWork *work;
    s32 motionIndex;

    work = object->data;
    work->modelId = fn_80025A04(
        fn_80008DD0(lbl_1_data_14F4[object->work[0]], 0x10000000, 2));
    for (motionIndex = 0; motionIndex < 2; motionIndex++) {
        work->jointMotionIds[motionIndex] = fn_80030458(
            work->modelId,
            fn_80008DD0(lbl_1_data_14C4[object->work[0]].modelFileIds[motionIndex],
                                 0x10000000, 2));
    }

    work->state = 0;
    work->variantIndex = object->work[0];
    work->modelNameIndex = object->work[1];
    work->modelPairIndex = object->work[3];
    work->motionState = 0;

    fn_8002741C(lbl_1_bss_1760[work->modelPairIndex],
                     lbl_1_data_1584[work->modelNameIndex], work->modelId);
    fn_1_225AC(lbl_1_bss_1760[work->modelPairIndex],
               lbl_1_data_1584[work->modelNameIndex], &work->referencePosition);
    fn_800269E8(work->modelId, 0x40000001);

    if (work->variantIndex == 4) {
        work->scale.x = work->scale.y = work->scale.z = 0.5f;
    } else {
        work->scale.x = work->scale.y = work->scale.z = 1.0f;
    }

    work->position.x = work->position.y = work->position.z = 0.0f;
    work->rotation.x = work->rotation.y = work->rotation.z = 0.0f;
    work->frame = 0;

    QuizComparisonMotionStepsSet(work);
    if (work->variantIndex == 4) {
        work->motionSpeed = 0.0055555557f;
    } else {
        work->motionSpeed = 0.011111111f;
    }

    fn_800268B0(work->modelId, work->scale.x, work->scale.y, work->scale.z);
    fn_80026760(work->modelId, work->position.x, work->position.y, work->position.z);
    fn_80026808(work->modelId, work->rotation.x, work->rotation.y, work->rotation.z);
    fn_8002C95C(work->modelId, work->jointMotionIds[0]);
    object->objFunc = fn_1_21B14;
}

/* Updates a comparison object each frame: its motion, place and turn. */
void fn_1_21B14(OMOBJ *object) {
    QuizMotionWork *work;

    work = object->data;
    if (work->motionState == 4) {
        work->position.x += work->positionStep.x;
        work->position.y += work->positionStep.y;
        work->position.z += work->positionStep.z;
        work->scale.x -= work->motionSpeed;
        work->scale.y -= work->motionSpeed;
        work->scale.z -= work->motionSpeed;
        work->rotation.y += 5.0f;
        work->frame++;
        if ((f32)work->frame >= 90.0f) {
            fn_800269E8(work->modelId, 1);
            work->motionState = 0;
            work->frame = 0;
        }
    } else if (work->motionState == 1) {
        if (work->frame >= 60) {
            work->frame = 0;
            work->motionState = 3;
        } else {
            work->position.y += work->rotationStepY;
            work->rotation.y += 5.0f;
            work->scale.x -= 0.01f;
            work->scale.y -= 0.01f;
            work->scale.z -= 0.01f;
            work->frame++;
        }
    } else if (work->motionState == 2) {
        if (work->frame >= 60) {
            work->position.y = 0.0f;
            work->frame = 0;
            work->motionState = 0;
        } else {
            work->position.y -= work->rotationStepY;
            work->rotation.y += 5.0f;
            work->scale.x += 0.01f;
            work->scale.y += 0.01f;
            work->scale.z += 0.01f;
            work->frame++;
        }
    } else if (work->motionState == 3) {
        work->rotation.y += 5.0f;
    }

    fn_800268B0(work->modelId, work->scale.x, work->scale.y, work->scale.z);
    fn_80026760(work->modelId, work->position.x, work->position.y,
                    work->position.z);
    fn_80026808(work->modelId, work->rotation.x, work->rotation.y,
                    work->rotation.z);
}

/* Sets each object's move from its place to a model joint's place, with a vertical speed. */
void fn_1_21E00(void) {
    s32 group;
    s32 index;
    QuizMotionWork *work;

    group = 0;
    while (group < 2) {
        fn_80027508(lbl_1_bss_1760[group]);
        index = 0;
        while (index < lbl_1_bss_15A8[group]) {
            work = lbl_1_bss_1768[group][index]->data;
            fn_1_225AC(lbl_1_bss_1760[work->modelPairIndex],
                lbl_1_data_1584[work->modelNameIndex],
                &work->referencePosition);
            work->position = work->referencePosition;
            QuizComparisonMotionStepsSet(work);
            fn_80026760(work->modelId, work->position.x,
                work->position.y, work->position.z);
            index += 1;
        }
        group += 1;
    }
}

/* Picks different random objects for both sides. */
void fn_1_22058(void) {
    s32 player;
    s32 slot;
    s32 selection;
    s32 character;

    for (player = 0; player < 2; ++player) {
        for (slot = 0; slot < 6; slot++) {
            lbl_1_bss_1720[player][slot] = -1;
        }
    }
    for (player = 0; player < 2; ++player) {
        for (slot = 0; slot < 23; slot++) {
            lbl_1_bss_1668[player][slot] =
                lbl_1_bss_15B0[player][slot] = -1;
        }
    }
    for (player = 0; player < 2; ++player) {
        slot = 0;
        while (slot < lbl_1_data_15CC[lbl_1_bss_100].itemCount) {
            do {
                selection = frandmod(6);
            } while (fn_1_2233C(selection, player) == 0);
            lbl_1_bss_1720[player][slot] = selection;
            slot++;
        }
    }
    for (player = 0; player < 2; ++player) {
        slot = 0;
        while (slot < lbl_1_data_15CC[lbl_1_bss_100].itemCount) {
            lbl_1_bss_1668[player][slot] =
                lbl_1_bss_1720[player][slot];
            slot++;
        }
        while (slot < lbl_1_bss_15A8[player]) {
            lbl_1_bss_1668[player][slot] =
                lbl_1_bss_1720[player][frandmod(
                    lbl_1_data_15CC[lbl_1_bss_100].itemCount)];
            slot++;
        }
    }
    for (player = 0; player < 2; ++player) {
        slot = 0;
        while (slot < lbl_1_bss_15A8[player]) {
            do {
                character = frandmod(0x12);
            } while (fn_1_223B8(character, player) == 0);
            lbl_1_bss_15B0[player][slot] = character;
            slot++;
        }
    }
}

/* Returns 1 when the first side's group does not hold the value yet. */
s32 fn_1_2233C(s32 value, s32 groupIndex) {
    s32 index;

    for (index = 0; index < 6; index++) {
        if (lbl_1_bss_1720[groupIndex][index] != -1 &&
            value == lbl_1_bss_1720[groupIndex][index]) {
            return 0;
        }
    }
    return 1;
}

/* Returns 1 when the second side's group does not hold the value yet. */
s32 fn_1_223B8(s32 value, s32 groupIndex) {
    s32 index;

    for (index = 0; index < 18; index++) {
        if (lbl_1_bss_15B0[groupIndex][index] != -1 &&
            value == lbl_1_bss_15B0[groupIndex][index]) {
            return 0;
        }
    }
    return 1;
}

void fn_1_22434(void) {
    MicQuizResponseRun(5);
}

/* Turns the stage models every frame until the round asks it to stop, then ends this process. */
void fn_1_22458(void) {
    f32 angleStorage[1];
    f32 firstAngle;
    f32 secondAngle;
    firstAngle = angleStorage[0] = secondAngle = 0.0f;
    for (;;)
    {
        HuPrcVSleep();
        if (lbl_1_bss_15A4 == 0)
        {
            firstAngle += 1.0f;
            secondAngle += 0.5f;
        }
        else
        {
            if (lbl_1_bss_15A4 == 1)
            {
            }
            else
            {
                if (lbl_1_bss_15A4 != 2)
                {
                }
                else
                {
                    break;
                }
            }
        }
        fn_80026808(lbl_1_bss_1760[0], 0.0f, firstAngle, 0.0f);
        fn_80026808(lbl_1_bss_1760[1], 0.0f, firstAngle, 0.0f);
        fn_80026808(lbl_1_bss_175C[0], 0.0f, -secondAngle, 0.0f);
        fn_80026808(lbl_1_bss_175C[1], 0.0f, -secondAngle, 0.0f);
    }
    HuPrcEnd();
    for (;;)
    {
        HuPrcVSleep();
    }
}

/* Gives the world position of a model's named object. */
void fn_1_225AC(HU3D_MODELID modelId, char *objName, Vec *position) {
    QuizObjectWorldPosition(modelId, objName, position);
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_22604(void) {
    GXColor windowColor;

    QuizComparisonCameraUpdate(&lbl_1_data_150C, &windowColor);
}
