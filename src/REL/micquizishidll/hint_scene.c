/* Hint round: the hint models and their display, the answer and its check. */
#include "msm_stream.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "math.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "dolphin.h"

#include "humath.h"

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

#include "game/frand.h"

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

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"

#include "game/mic.h"

#include "game/hsfformat.h"

#include "REL/micquizishidll/model_hint.h"

extern s32 lbl_1_bss_100;

extern f32 lbl_1_bss_104;

extern s32 lbl_1_bss_110;

extern HUPROCESS *lbl_1_bss_114[0xB];

extern s16 micQuizGameMes;

extern ANIMDATA *lbl_1_bss_1B28;

extern HUPROCESS *lbl_1_bss_1B2C;

extern f32 lbl_1_bss_1B4C;

extern f32 lbl_1_bss_1B58;

extern s32 lbl_1_bss_1B5C;

extern s16 lbl_1_bss_1B60;

extern f32 lbl_1_bss_1B64;

extern s16 lbl_1_bss_1B68;

extern ANIMDATA *lbl_1_bss_1B6C;

extern s16 lbl_1_bss_1B74;

extern s16 lbl_1_bss_1B76[2];

extern s16 lbl_1_bss_1B7A;

extern s16 lbl_1_bss_1B7C;

extern s16 lbl_1_bss_1B7E;

extern s16 lbl_1_bss_1B80[2];

extern s16 lbl_1_bss_1B84;

extern s16 lbl_1_bss_1B86;

extern s16 lbl_1_bss_1B88;

extern s16 lbl_1_bss_1B8A;

extern OMOBJ *lbl_1_bss_1B8C;

extern s32 lbl_1_bss_20;

extern f32 lbl_1_bss_8C;

extern u32 lbl_1_bss_FC;

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

extern u8 lbl_1_data_488[0x44];

u8 MicQuizQuestionLevelGet(void);

s32 MicQuizCurrentPlayerGet(void);

void MicQuizCountdownInit(void);

s32 MicQuizPortraitShow(s32 baseIndex);

void fn_1_2AEBC(s16 layerNo);

void fn_1_2B26C(void);

void fn_1_2BD60(s16 modelId);

void fn_1_2BDE0(s16 modelId, f32 ambientRed, f32 ambientGreen, f32 ambientBlue);

s32 MicQuizExpectedAnswerSet(s32 assetIndex);

s32 MicQuizPauseInit(HUPROCESS *process);

void MicQuizMicContextSet(s32 mode);

void fn_1_2AF88(void);

s32 lbl_1_data_1D18[7] = {
    0xA4007A, 0xA4007B, 0xA4007C, 0xA4007D, 0xA4007E, 0xA4007F, 0xA40080
};

OM_CAMERA_VIEW lbl_1_data_1D34 = {
    {0.0f, 268.0f, -79.0f}, {-12.5f, 0.0f, 0.0f}, 2489.45f
};

OM_CAMERA_VIEW lbl_1_data_1D50 = {
    {76.0f, -50.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 477.0f
};

s32 lbl_1_data_1D6C[11] = {
    0xA40036, 0xA40037, 0xA40038, 0xA40039, 0xA4003A, 0xA4003B,
    0xA4003C, 0xA4003D, 0xA4003E, 0xA4003F, 0xA40040
};

MicquizCameraRange lbl_1_data_1D98[5] = {
    {2.0f, 600.0f}, {1.5f, 900.0f}, {1.0f, 1200.0f},
    {0.5f, 1500.0f}, {0.5f, 1800.0f}
};

/* Sets up the hint round: cameras, lights, the colored models and their textures. */
void fn_1_2A164(void) {
    GXColor modelColors[4] = {
        {0xFF, 0,    0,    0},
        {0,    0xFF, 0,    0},
        {0,    0xFF, 0,    0},
        {0,    0xFF, 0xFF, 0},
    };
    s32 colorIndex;
    s32 randomIndex;
    u32 textureSize;

    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 25.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(2, 25.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_1D50);
    fn_8003E488(2, &lbl_1_data_1D34);
    lbl_1_bss_1B8C = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0, 0, -1,
                                 fn_8003DAA4);
    lbl_1_bss_1B8C->work[0] = 2;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0, 0, 0);
    Hu3DGLightPosAimSetV((s16)lbl_1_bss_110, &lbl_1_data_470,
                         &lbl_1_data_47C);
    Hu3DGLightStaticSet((s16)lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_110);
    Hu3DGLightColorSet((s16)lbl_1_bss_110, lbl_1_data_488[0],
                       lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);

    lbl_1_bss_1B7E = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x32), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1B7E, 1);
    lbl_1_bss_1B8A = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x34), 0x10000000, 2));
    lbl_1_bss_1B88 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x35), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_1B8A, 0);
    fn_80026B8C(lbl_1_bss_1B88, 0);
    fn_80026B64(lbl_1_bss_1B8A, 2);
    fn_80026B64(lbl_1_bss_1B88, 2);
    lbl_1_bss_1B84 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x2B), 0x10000000, 2));
    lbl_1_bss_1B80[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x2C), 0x10000000, 2));
    lbl_1_bss_1B80[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x2D), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1B84, 2);
    fn_80026B64(lbl_1_bss_1B80[0], 2);
    fn_80026B64(lbl_1_bss_1B80[1], 2);
    fn_80026B8C(lbl_1_bss_1B84, 1);
    fn_80026B8C(lbl_1_bss_1B80[0], 1);
    fn_80026B8C(lbl_1_bss_1B80[1], 1);
    lbl_1_bss_1B86 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x33), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1B86, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_1B86, 0.0f, 0.0f, -20.0f);
    fn_80026B64(lbl_1_bss_1B86, 2);
    fn_80026B8C(lbl_1_bss_1B86, 0);
    lbl_1_bss_1B7C = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x31), 0x10000000, 2));

    do {
        randomIndex = frandmod(0xB);
    } while (randomIndex == 8);
    lbl_1_bss_1B6C = HuSprAnimRead(
        fn_80008DD0(lbl_1_data_1D6C[randomIndex], 0x10000000,
                             2));
    MicQuizExpectedAnswerSet(randomIndex);
    fn_80026B64(lbl_1_bss_1B7C, 1);
    fn_80026B8C(lbl_1_bss_1B7C, 1);
    lbl_1_bss_1B7A = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_1B7A, 0,
                      HuSprCreate(lbl_1_bss_1B6C, 0xA, 0));
    HuSprAttrSet(lbl_1_bss_1B7A, 0, 4);
    fn_80026760(lbl_1_bss_1B7C, 0.0f, 0.0f, 0.0f);
    fn_80026A44(lbl_1_bss_1B7C, 1);
    fn_80026CBC(lbl_1_bss_1B7C, 1.0f);
    lbl_1_bss_1B68 = fn_80030B2C(lbl_1_bss_1B6C, lbl_1_bss_1B7C,
                                    "mari02");
    Hu3DAnimBankSet(lbl_1_bss_1B68, 0);

    colorIndex = frandmod(4);
    fn_1_2BDE0(lbl_1_bss_1B7C,
               (f32)modelColors[colorIndex].r,
               (f32)modelColors[colorIndex].g,
               (f32)modelColors[colorIndex].b);
    lbl_1_bss_1B64 = 0.0f;
    lbl_1_bss_1B74 = fn_80025D4C(lbl_1_bss_1B7E);
    fn_80026B64(lbl_1_bss_1B74, 1);
    fn_80026B8C(lbl_1_bss_1B74, 1);
    fn_80026760(lbl_1_bss_1B74, 0.0f, 0.0f, 0.0f);
    fn_80026A44(lbl_1_bss_1B74, 1);
    fn_80026CBC(lbl_1_bss_1B74, 0.0f);

    lbl_1_bss_1B76[0] = fn_80025D4C(lbl_1_bss_1B7E);
    lbl_1_bss_1B76[1] = fn_80025D4C(lbl_1_bss_1B7E);
    fn_1_2BD60(lbl_1_bss_1B76[0]);
    fn_1_2BD60(lbl_1_bss_1B76[1]);
    fn_80026A44(lbl_1_bss_1B76[0], 1);
    fn_80026A44(lbl_1_bss_1B76[1], 1);
    fn_80026B64(lbl_1_bss_1B76[0], 1);
    fn_80026B64(lbl_1_bss_1B76[1], 1);
    fn_80026B8C(lbl_1_bss_1B76[0], 2);
    fn_80026B8C(lbl_1_bss_1B76[1], 2);

    lbl_1_bss_FC = 0;
    micQuizGameMes = -1;
    lbl_1_bss_104 = 0.0f;
    fn_8000C200(lbl_1_bss_1B28 = HuSprAnimMake(0x100, 0x100, 0),
                       0x10000000);
    lbl_1_bss_1B28->bmp->palData = NULL;
    lbl_1_bss_1B28->bmp->palNum = 0;
    textureSize = GXGetTexBufferSize(0x100, 0x100, 6, 0, 0);
    lbl_1_bss_1B28->bmp->data =
        HuMemDirectMallocNum(2, textureSize, 0x10000000);
    lbl_1_bss_1B60 = fn_80030B2C(lbl_1_bss_1B28, lbl_1_bss_1B86,
                                    "mari02");
    fn_8002583C(2, 0, fn_1_2AEBC);
    lbl_1_bss_100 = MicQuizQuestionLevelGet();
    MicQuizCountdownInit();
    lbl_1_bss_8C = 11.111111f;
    lbl_1_bss_1B58 = lbl_1_data_1D98[lbl_1_bss_100].width;
    lbl_1_bss_1B4C = 3.5f / lbl_1_data_1D98[lbl_1_bss_100].height;
    lbl_1_bss_1B5C = 1;
    fn_80026760(lbl_1_bss_1B76[0],
                    -(50.0f + lbl_1_bss_1B58), 0.0f, 5.0f);
    fn_80026760(lbl_1_bss_1B76[1],
                    50.0f + lbl_1_bss_1B58, 0.0f, 5.0f);
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(3);
    lbl_1_bss_1B2C = fn_8000DAC0(fn_1_2AF88, 0x64, 0x1800, 0,
                                     *lbl_1_bss_114);
    fn_8000DAC0(fn_1_2B26C, 0x64, 0x1800, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FIFTH);
}

void fn_1_2ADF8(void)
{
    Point3d positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_1B80[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= lbl_1_bss_8C;
    positions[1].x += lbl_1_bss_8C;
    i = 0;
    while (i < 2) {
        fn_80026790(lbl_1_bss_1B80[i], &positions[i]);
        i += 1;
    }
}

void fn_1_2AF08(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format, void *destination);

void fn_1_2AEBC(s16 layerNo) {
    fn_1_2AF08(0, 0, 0x100, 0x100, 6, lbl_1_bss_1B28->bmp->data);
    fn_8002BF48();
}

void fn_1_2AF08(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format,
                void *destination) {
    GXSetTexCopySrc((u16)x, (u16)y, (u16)width, (u16)height);
    GXSetTexCopyDst((u16)width, (u16)height, format, 0);
    GXCopyTex(destination, 1);
}

extern f32 lbl_1_bss_1B50;

extern f32 lbl_1_bss_1B54;

/* Moves the two hint models back and forth between their places, with a sound at each turn. */
void fn_1_2AF88(void) {
    HuVecF modelPositions[2];
    f32 progress;
    s32 movingForward;
    s32 modelIndex;

    movingForward = 1;
    progress = 0.0f;
    lbl_1_bss_1B50 = 0.0f;
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        fn_800267CC(lbl_1_bss_1B76[modelIndex],
                        &modelPositions[modelIndex]);
    }
    for (;;) {
        HuPrcVSleep();
        if (lbl_1_bss_1B5C != 1) {
            progress += lbl_1_bss_1B50;
            if (movingForward != 0) {
                if (progress >= 50.0f) {
                    HuAudFXPlay(0x52E);
                    movingForward = 0;
                    progress = 50.0f;
                    lbl_1_bss_1B50 = -lbl_1_bss_1B54;
                } else {
                    lbl_1_bss_1B50 = lbl_1_bss_1B54;
                }
            } else if (progress <= -50.0f) {
                HuAudFXPlay(0x52E);
                movingForward = 1;
                progress = -50.0f;
                lbl_1_bss_1B50 = lbl_1_bss_1B54;
            } else {
                lbl_1_bss_1B50 = -lbl_1_bss_1B54;
            }
            modelPositions[0].x = progress -
                (50.0f + lbl_1_bss_1B58);
            modelPositions[1].x = progress +
                (50.0f + lbl_1_bss_1B58);
            if (lbl_1_bss_1B5C == 2) {
                lbl_1_bss_1B50 = 0.0f;
            }
        }
        if (lbl_1_bss_1B58 >= 100.0f) {
            lbl_1_bss_1B58 = 100.0f;
        }
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            fn_80026790(lbl_1_bss_1B76[modelIndex],
                             &modelPositions[modelIndex]);
        }
        if (lbl_1_bss_1B5C != 0) {
            continue;
        }
        lbl_1_bss_1B54 += lbl_1_bss_1B4C;
        if (lbl_1_bss_1B54 >= 3.5f) {
            lbl_1_bss_1B54 = 3.5f;
        }
    }
}

void fn_1_2B26C(void)
{
    MicQuizResponseRun(2);
}

extern Point3d lbl_1_bss_1B30[2];

extern f32 lbl_1_bss_1B48;

s32 MicQuizCountdownUpdate(void);

void fn_1_1730(void);

void MicQuizQuestionReadySet(void);

void MicQuizListenStart(void);

s32 MicQuizResponseStateGet(void);

/* Runs the hint round's mode steps: the question message, the hint display (open, show, close),
   the answer and the closing wipe. */
void fn_1_2B290(void) {
    s32 currentMode;
    f32 modelAlpha;

    currentMode = MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        break;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_FC = 2;
        }
        break;
    case 2:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 3;
            fn_1_1730();
            MicQuizQuestionReadySet();
            lbl_1_bss_1B5C = 0;
        } else {
            lbl_1_bss_104 += 1.0f;
            fn_1_2ADF8();
        }
        break;
    case 3:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 9;
            lbl_1_bss_1B5C = 1;
        } else if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 4;
        }
        if (currentMode == 1) {
            lbl_1_bss_1B5C = 1;
        } else if (currentMode == 0) {
            lbl_1_bss_1B5C = 0;
        }
        break;
    case 4:
        lbl_1_bss_1B5C = 0;
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 60.0f) {
            lbl_1_bss_FC = 5;
            fn_80026CBC(lbl_1_bss_1B74, 0.0f);
            lbl_1_bss_104 = 0.0f;
            OSReport("LED_HINT_EFF\n");
        } else {
            modelAlpha = (f32) sin(
                (M_PI * (f64) (3.0f * lbl_1_bss_104)) / 180.0);
            fn_80026CBC(lbl_1_bss_1B74, modelAlpha);
        }
        break;
    case 5:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_FC = 6;
            lbl_1_bss_1B58 = 2.0f * lbl_1_data_1D98[lbl_1_bss_100].width;
            lbl_1_bss_104 = 0.0f;
            OSReport("LED_HINT_OPEN\n");
        } else {
            lbl_1_bss_1B58 +=
                (2.0f * lbl_1_data_1D98[lbl_1_bss_100].width) / 120.0f;
        }
        break;
    case 6:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_FC = 7;
            lbl_1_bss_104 = 0.0f;
            OSReport("LED_HINT\n");
        }
        break;
    case 7:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_FC = 8;
            lbl_1_bss_1B58 = lbl_1_data_1D98[lbl_1_bss_100].width;
            lbl_1_bss_104 = 0.0f;
        } else {
            lbl_1_bss_1B58 -=
                lbl_1_data_1D98[lbl_1_bss_100].width / 120.0f;
            OSReport("LED_HINT_CLOSE\n");
        }
        break;
    case 8:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 60.0f) {
            lbl_1_bss_FC = 3;
            fn_80026CBC(lbl_1_bss_1B74, 0.0f);
            lbl_1_bss_104 = 0.0f;
            MicQuizListenStart();
            lbl_1_bss_1B5C = 0;
        } else {
            modelAlpha = (f32) sin(
                (M_PI * (f64) (3.0f * lbl_1_bss_104)) / 180.0);
            fn_80026CBC(lbl_1_bss_1B74, modelAlpha);
        }
        break;
    case 9:
        HuPrcKill(lbl_1_bss_1B2C);
        fn_800267CC(lbl_1_bss_1B76[0], &lbl_1_bss_1B30[0]);
        fn_800267CC(lbl_1_bss_1B76[1], &lbl_1_bss_1B30[1]);
        lbl_1_bss_1B48 =
            ((-50.0f + lbl_1_bss_1B58) - lbl_1_bss_1B30[0].x) / 30.0f;
        lbl_1_bss_FC = 0xA;
        break;
    case 10:
        if (lbl_1_bss_104 >= 30.0f) {
            lbl_1_bss_FC = 0xB;
            HuAudFXPlay(0x533);
            lbl_1_bss_104 = 0.0f;
        } else {
            lbl_1_bss_1B30[0].x += lbl_1_bss_1B48;
            lbl_1_bss_1B30[1].x += lbl_1_bss_1B48;
            fn_80026790(lbl_1_bss_1B76[0], &lbl_1_bss_1B30[0]);
            fn_80026790(lbl_1_bss_1B76[1], &lbl_1_bss_1B30[1]);
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 11:
        if (lbl_1_bss_104 >= 60.0f) {
            lbl_1_bss_FC = 0xC;
            lbl_1_bss_104 = 0.0f;
        } else {
            lbl_1_bss_1B30[0].x -= 2.5f;
            lbl_1_bss_1B30[1].x += 2.5f;
            fn_80026790(lbl_1_bss_1B76[0], &lbl_1_bss_1B30[0]);
            fn_80026790(lbl_1_bss_1B76[1], &lbl_1_bss_1B30[1]);
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 12:
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_FC = 0xD;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
            lbl_1_bss_104 = 0.0f;
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 13:
        if (WipeCheck() == 0) {
            fn_800258A8(2, 0);
            HuSprAnimKill(lbl_1_bss_1B28);
            HuMCSelWinContextKill();
            HuMCClose();
            fn_8003A5E4(1, 1);
        }
    }
}

/* Sets every material of a model to black. */
void fn_1_2BD60(s16 modelId) {
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

/* Sets a model's ambient color and gives every material that color. */
void fn_1_2BDE0(s16 modelId, f32 ambientRed, f32 ambientGreen, f32 ambientBlue) {
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    HSF_ATTRIBUTE *attribute;
    s32 materialIndex;
    s32 attributeIndex;

    hsf = model->hsf;
    material = hsf->material;
    fn_800273EC(modelId, ambientRed, ambientGreen, ambientBlue);

    for (materialIndex = 0; materialIndex < hsf->materialNum; materialIndex++, material++) {
        for (attributeIndex = 0; attributeIndex < material->attrNum; attributeIndex++) {
            attribute = &hsf->attribute[material->attr[attributeIndex]];
            strcmp("vq_kao", attribute->bitmap->name);
            material->color[0] = (u8)ambientRed;
            material->color[1] = (u8)ambientGreen;
            material->color[2] = (u8)ambientBlue;
        }
    }
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_2BEF8(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;
    lbl_80266BD8[0] += (f32) (lbl_802BFFEC[1] / 40);
    lbl_80266BD8[0] -= (f32) (lbl_802BFFE8[1] / 40);
    if ((s32) (lbl_802C0010[1] & 4) != 0) {
        lbl_80266B18->y -= 0.5f;
    }
    if ((s32) (lbl_802C0010[1] & 8) != 0) {
        lbl_80266B18->y += 0.5f;
    }
    if ((s32) (lbl_802C0010[1] & 1) != 0) {
        lbl_80266B18->x -= 0.5f;
    }
    if ((s32) (lbl_802C0010[1] & 2) != 0) {
        lbl_80266B18->x += 0.5f;
    }
    lbl_80266A58->x += 5.0f * ((f32) lbl_802BFFF0[1] / 44.0f);
    lbl_80266A58->y -= 5.0f * ((f32) lbl_802BFFF4[1] / 44.0f);
    lbl_80266BD8[1] += (f32) (lbl_802BFFEC[0] / 5);
    lbl_80266BD8[1] -= (f32) (lbl_802BFFE8[0] / 5);
    if ((s32) (lbl_802C0010[0] & 4) != 0) {
        lbl_80266B18[1].y -= 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 8) != 0) {
        lbl_80266B18[1].y += 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 1) != 0) {
        lbl_80266B18[1].x -= 5.0f;
    }
    if ((s32) (lbl_802C0010[0] & 2) != 0) {
        lbl_80266B18[1].x += 5.0f;
    }
    lbl_80266A58[1].x += 5.0f * ((f32) lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[1].y -= 5.0f * ((f32) (s8) lbl_802BFFF4[0] / 44.0f);
    if ((s32) (lbl_802C0008[0] & 0x800) != 0) {
        fn_8003E488(2, &lbl_1_data_1D34);
    }
    if ((s32) (lbl_802C0008[1] & 0x800) != 0) {
        fn_8003E488(1, &lbl_1_data_1D50);
    }
    printWin(0x10, 0x28, 0xF0, 0x54, &windowColor);
    fontcolor = 0xE;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18->x, lbl_80266B18->y,
           lbl_80266B18->z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58->x, lbl_80266A58->y,
           lbl_80266A58->z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
    print8(0x10, 0x58, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[1].x, lbl_80266B18[1].y,
           lbl_80266B18[1].z);
    print8(0x10, 0x64, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[1].x, lbl_80266A58[1].y,
           lbl_80266A58[1].z);
    print8(0x10, 0x70, 1.0f, "CZoom:%.02f", lbl_80266BD8[1]);
}

OMOBJ *lbl_1_bss_1B8C;
s16 lbl_1_bss_1B8A;
s16 lbl_1_bss_1B88;
s16 lbl_1_bss_1B86;
s16 lbl_1_bss_1B84;
s16 lbl_1_bss_1B80[2];
s16 lbl_1_bss_1B7E;
s16 lbl_1_bss_1B7C;
s16 lbl_1_bss_1B7A;
s16 lbl_1_bss_1B76[2];
s16 lbl_1_bss_1B74;
u8 lbl_1_bss_1B70[0x4];
ANIMDATA *lbl_1_bss_1B6C;
s16 lbl_1_bss_1B68;
f32 lbl_1_bss_1B64;
s16 lbl_1_bss_1B60;
s32 lbl_1_bss_1B5C;
f32 lbl_1_bss_1B58;
f32 lbl_1_bss_1B54;
f32 lbl_1_bss_1B50;
f32 lbl_1_bss_1B4C;
f32 lbl_1_bss_1B48;
Point3d lbl_1_bss_1B30[2];
HUPROCESS *lbl_1_bss_1B2C;
ANIMDATA *lbl_1_bss_1B28;
