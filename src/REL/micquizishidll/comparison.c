/* Comparison round: objects on two sides, the players' answer and the result display. */
#include "msm_stream.h"
#include "dolphin/types.h"

#include "REL/micquizishidll/module_base.h"

#include "game/hu3d.h"

#include "game/printfunc.h"

#include "game/process.h"

#include "humath.h"

#include "REL/micquizishidll.h"

#include "REL/micquizishidll/micquizishidll_message_setup_views_v2.h"

#include "game/object.h"

#include "REL/micquizishidll/shared_types.h"

#include "REL/micquizishidll/micquizishidll_callback_views_v1.h"

#include "REL/micquizishidll/micquizishidll_model_views_v1.h"
#include <math.h>

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

extern OM_CAMERA_VIEW lbl_1_data_12EC;

extern char lbl_1_data_145C[];

extern char lbl_1_data_147B[];

extern char lbl_1_data_1498[];

extern s32 lbl_1_bss_15A4;

extern s32 lbl_1_bss_15A8[2];

extern s16 lbl_1_bss_1750[1];

extern s32 lbl_1_bss_100;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_20;

extern f32 lbl_1_bss_104;

extern f32 lbl_1_bss_8C;

extern u32 lbl_1_bss_FC;

extern HUPROCESS *lbl_1_bss_114[0xB];

extern s32 lbl_1_bss_15B0[2][23];

extern s32 lbl_1_bss_1668[2][23];

extern s16 lbl_1_bss_1598;

extern s16 lbl_1_bss_159A;

extern ANIMDATA *lbl_1_bss_159C;

extern s16 lbl_1_bss_1754[2];

extern s16 lbl_1_bss_1758[2];

extern s16 lbl_1_bss_175C[2];

extern s16 lbl_1_bss_1760[2];

extern s16 lbl_1_bss_1764;

extern OMOBJ *lbl_1_bss_1768[2][23];

extern OMOBJ *lbl_1_bss_A8;

extern s16 lbl_1_bss_9C[2];

extern s16 lbl_1_bss_A0;

extern s16 lbl_1_bss_A4;

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

extern u8 lbl_1_data_488[4];

extern MicquizSelectionConfig lbl_1_data_15CC[5];

u8 MicQuizQuestionLevelGet(void);

void MicQuizCountdownInit(void);

s32 MicQuizCurrentPlayerGet(void);

void MicQuizExpectedAnswerSet(s32);

s32 MicQuizPauseInit(HUPROCESS *);

void MicQuizMicContextSet(s32);

s32 MicQuizRandRange(s32, s32);

void fn_1_22434(void);

void fn_1_22458(void);

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

extern char *lbl_1_data_1584[18];

extern s32 lbl_1_data_1608[5];

char *lbl_1_data_1584[18] = { "2", "4", "6", "8", "10", "11", "o1", "o2", "o3", "o4", "o5", "o6",
                             "o7", "o8", "o9", "o10", "o11", "o12" };
MicquizSelectionConfig lbl_1_data_15CC[5] = { { 5, 2, 3 }, { 5, 4, 6 }, { 5, 7, 9 }, { 5, 10, 12 },
                                             { 1, 13, 15 } };
s32 lbl_1_data_1608[5] = { 1, 2, 3, 4, 5 };

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
    lbl_1_bss_159C = HuSprAnimRead(fn_80008DD0(DATANUM(0xa40000, 0x5F), 0x10000000, 2));
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

extern s16 micQuizGameMes;

void fn_1_216A0(void);

s32 MicQuizResponseStateGet(void);

/* Debug value hook: does nothing in the release build. */
static inline void DebugValue(s32 value) { }

/* Runs the comparison round's mode steps: the question message, the objects, the answer, and the
   closing wipe. */
void fn_1_20804(void) {
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
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
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

typedef struct MicquizPlayerWork {
    u8 unknown_00[0x60]; /* No code names this field. */
    s32 animationState;
} MicquizPlayerWork;

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
    MicquizPlayerWork *firstPlayerData;
    MicquizPlayerWork *secondPlayerData;
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
    for (rotationFrame = 0; rotationFrame < 0x96; rotationFrame++) {
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
            firstPlayerData->animationState = 4;
        }
        if (lbl_1_bss_1768[1][playerIndex] != NULL) {
            fn_1_211C4((u8)playerNumber, 1);
            secondPlayerData = lbl_1_bss_1768[1][playerIndex]->data;
            secondPlayerData->animationState = 4;
        }
        playerNumber++;
        HuPrcSleep(0x2D);
    }
    HuPrcSleep(0x3C);
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

typedef struct MicquizNumberProcess {
    u8 unknown_000[0x12C]; /* No code names this field. */
    u32 selection;
} MicquizNumberProcess;

/* Shows the result: switches the counter models' animation banks and swings them. */
void fn_1_21230(void) {
    MicquizNumberProcess *process;
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

    process = (MicquizNumberProcess *)fn_8000DD9C();
    selection = process->selection;
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

void fn_1_216A0(void)
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

void fn_1_225AC(HU3D_MODELID modelId, char *objName, Vec *position);

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

    if (work->modelPairIndex == 0) {
        work->positionStep.x = (-350.0f - work->referencePosition.x) / 90.0f;
        work->positionStep.y = (450.0f - work->referencePosition.y) / 90.0f;
        work->positionStep.z = (-450.0f - work->referencePosition.z) / 90.0f;
    } else {
        work->positionStep.x = (350.0f - work->referencePosition.x) / 90.0f;
        work->positionStep.y = (450.0f - work->referencePosition.y) / 90.0f;
        work->positionStep.z = (-450.0f - work->referencePosition.z) / 90.0f;
    }

    work->rotationStepX = work->rotationStepZ = 0.0f;
    work->rotationStepY = ((450.0f - work->referencePosition.y) / 2.0f) / 60.0f;
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
        if (work->frame >= 0x3C) {
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
        if (work->frame >= 0x3C) {
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

typedef struct MicquizObjectWork {
    Vec position;
    Vec targetPosition;
    u8 unknown_18[0x20]; /* No code names this field. */
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 velocityX; /* the game's code never reads this field */
    f32 verticalVelocity;
    f32 velocityZ; /* the game's code never reads this field */
    u32 unknown_50;
    u32 unknown_54;
    s32 nameIndex;
    s32 groupIndex;
    s32 state;
    s16 modelId;
} MicquizObjectWork;

/* Sets each object's move from its place to a model joint's place, with a vertical speed. */
void fn_1_21E00(void) {
    s32 group;
    s32 index;
    MicquizObjectWork *work;

    group = 0;
    while (group < 2) {
        fn_80027508(lbl_1_bss_1760[group]);
        index = 0;
        while (index < lbl_1_bss_15A8[group]) {
            work = lbl_1_bss_1768[group][index]->data;
            fn_1_225AC(lbl_1_bss_1760[work->groupIndex],
                lbl_1_data_1584[work->nameIndex],
                &work->targetPosition);
            work->position = work->targetPosition;
            if (work->groupIndex == 0) {
                work->deltaX = (-350.0f - work->targetPosition.x) / 90.0f;
                work->deltaY = (450.0f - work->targetPosition.y) / 90.0f;
                work->deltaZ = (-450.0f - work->targetPosition.z) / 90.0f;
            } else {
                work->deltaX = (350.0f - work->targetPosition.x) / 90.0f;
                work->deltaY = (450.0f - work->targetPosition.y) / 90.0f;
                work->deltaZ = (-450.0f - work->targetPosition.z) / 90.0f;
            }
            work->velocityX = work->velocityZ = 0.0f;
            work->verticalVelocity = ((450.0f - work->targetPosition.y) / 2.0f) / 60.0f;
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
    Mtx matrix;

    fn_80023398(modelId, objName, matrix);
    position->x = matrix[0][3];
    position->y = matrix[1][3];
    position->z = matrix[2][3];
}

extern char lbl_1_data_16AF[];

extern char lbl_1_data_16CE[];

extern char lbl_1_data_16EB[];

extern char lbl_1_data_16AF[31];

extern char lbl_1_data_16CE[29];

extern char lbl_1_data_16EB[12];

char lbl_1_data_16AF[31] = "Center X:%.02f Y:%.02f Z:%.02f";
char lbl_1_data_16CE[29] = "CRot X:%.02f Y:%.02f Z:%.02f";
char lbl_1_data_16EB[12] = "CZoom:%.02f";

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_22604(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].x -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].x += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_150C);

    printWin(0x10, 0x28, 0xF0, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, lbl_1_data_16AF, lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, lbl_1_data_16CE, lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, lbl_1_data_16EB, lbl_80266BD8[0]);
}
