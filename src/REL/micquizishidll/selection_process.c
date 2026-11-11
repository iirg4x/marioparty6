/* Fruit round: the choices, the items' moves to the players and the result. */
#include "msm_stream.h"
#include "game/hsfformat.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "math.h"

#include "dolphin/mtx.h"

#include "string.h"

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

#include "msm_se.h"
#include "REL/micquizishidll.h"

/* quiz.h leaves these out (other files call them without a prototype); this file's calls take
 * them with one. */
void MicQuizCountdownInit(void);
void MicQuizExpectedAnswerSet(s32 expectedAnswer);
void MicQuizListenStart(void);
void MicQuizMicContextSet(s32 questionType);
void MicQuizQuestionReadySet(void);

u8 lbl_1_data_1E88[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
OM_CAMERA_VIEW lbl_1_data_1EA4 = { { 0.0f, 187.0f, -86.0f }, { -22.14f, 0.0f, 0.0f }, 1911.5f };
Point3d lbl_1_data_1EC0 = { 0.0f, 4000.0f, 0.0f };
Point3d lbl_1_data_1ECC = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_1ED8 = { 0.0f, 0.0f, -200.0f };
s32 lbl_1_data_1EE4[8] = { 0xA4006A, 0xA4006B, 0xA4006C, 0xA4006D, 0xA4006E, 0xA4006F, 0xA40070,
                          0xA40071 };
s32 lbl_1_data_1F04[5] = { 4, 5, 6, 7, 8 };
s32 lbl_1_data_1F18[5] = { 1, 2, 3, 3, 3 };
Point3d lbl_1_data_1F2C[5][8] = { { { -190.0f, 100.0f, -230.0f }, { -70.0f, 100.0f, -170.0f },
        { 70.0f, 100.0f, -170.0f }, { 190.0f, 100.0f, -230.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,
                0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,
                        0.0f } }, { { -250.0f, 100.0f, -370.0f }, { -160.0f, 100.0f, -210.0f },
                                { 0.0f, 100.0f, -150.0f }, { 160.0f, 100.0f, -210.0f }, { 250.0f,
                                        100.0f, -370.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f,
                                                0.0f }, { 0.0f, 0.0f,
                                                        0.0f } }, { { -260.0f, 100.0f, -390.0f },
                                                                { -180.0f, 100.0f,
                                                                        -220.0f }, { -70.0f, 100.0f,
                                                                                -170.0f }, { 70.0f,
                                                                                        100.0f, -170.0f }, { 180.0f, 100.0f, -220.0f }, { 260.0f, 100.0f, -390.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }, { { -260.0f, 100.0f, -410.0f }, { -200.0f, 100.0f, -260.0f }, { -110.0f, 100.0f, -180.0f }, { 0.0f, 100.0f, -150.0f }, { 110.0f, 100.0f, -180.0f }, { 200.0f, 100.0f, -260.0f }, { 260.0f, 100.0f, -410.0f }, { 0.0f, 0.0f, 0.0f } }, { { -220.0f, 100.0f, -520.0f }, { -250.0f, 100.0f, -360.0f }, { -170.0f, 100.0f, -210.0f }, { -60.0f, 100.0f, -160.0f }, { 60.0f, 100.0f, -160.0f }, { 170.0f, 100.0f, -210.0f }, { 250.0f, 100.0f, -360.0f }, { 220.0f, 100.0f, -520.0f } } };

void fn_1_2CE80(void);
void fn_1_2E074(void);
void fn_1_2E138(void);
s32 fn_1_2E240(s32 value);
s32 fn_1_2E29C(s32 key);
void fn_1_2E2EC(void);
void fn_1_2E85C(void);
void fn_1_2E89C(void);
void fn_1_2EC80(s16 modelId);
extern f32 lbl_1_bss_1B90;
extern f32 lbl_1_bss_1B94;
extern s32 lbl_1_bss_1B98;
extern s32 lbl_1_bss_1B9C;
extern s32 lbl_1_bss_1BA0;
extern s32 lbl_1_bss_1BA4;
extern s32 lbl_1_bss_1BA8;
extern s16 lbl_1_bss_1BAC[2];
extern s16 lbl_1_bss_1BB0;
extern s16 lbl_1_bss_1BB2;
extern s16 lbl_1_bss_1BB4[8];
extern s16 lbl_1_bss_1BC4[8];
extern s16 lbl_1_bss_1BD4;
extern f32 lbl_1_bss_1BD8;
extern s16 lbl_1_bss_1BDC;
extern s32 lbl_1_bss_1BE0;

/* Sets up the fruit round: picks the answer fruit and the choices, reports the answer, creates
   the camera, lights and models. */
void fn_1_2C624(void) {
    s32 index;
    Point3d position;

    lbl_1_bss_100 = (u8)MicQuizQuestionLevelGet();
    lbl_1_bss_1BE0 = lbl_1_data_1F04[lbl_1_bss_100];
    fn_1_2E138();
    lbl_1_bss_1BA0 = frandmod(lbl_1_bss_1BE0);
    MicQuizExpectedAnswerSet(micQuizChoiceRecords[lbl_1_bss_1BA0].answerIndex);

    switch (micQuizChoiceRecords[lbl_1_bss_1BA0].answerIndex) {
    case 0:
        OSReport("    ANS:STRAWBERRY\n");
        break;
    case 1:
        OSReport("    ANS:MELON\n");
        break;
    case 2:
        OSReport("    ANS:APPLE\n");
        break;
    case 3:
        OSReport("    ANS:ORANGE\n");
        break;
    case 4:
        OSReport("    ANS:GRAPE\n");
        break;
    case 5:
        OSReport("    ANS:CHERRY\n");
        break;
    case 6:
        OSReport("    ANS:BANANA\n");
        break;
    case 7:
        OSReport("    ANS:PINE\n");
        break;
    }

    lbl_1_bss_FC = 0;
    lbl_1_bss_1BD8 = 7.7777777f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f,
                             20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_1EA4);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0, 0, -1,
                              fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV((s16)lbl_1_bss_110, &lbl_1_data_470,
                         &lbl_1_data_47C);
    Hu3DGLightStaticSet((s16)lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_110);
    Hu3DGLightColorSet((s16)lbl_1_bss_110, lbl_1_data_488[0],
                       lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f,
                     8000.0f);
    fn_8002AAB4(0x40, 0x40, 0x40);
    Hu3DShadowPosSet(&lbl_1_data_1EC0, &lbl_1_data_1ECC,
                     &lbl_1_data_1ED8);
    Hu3DShadowTPLvlSet(0.3f);

    lbl_1_bss_1BDC = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x72), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1BDC, ((1 << 30) | (1 << 0)));
    fn_80026FFC(lbl_1_bss_1BDC);
    fn_80026B8C(lbl_1_bss_1BDC, 0);
    lbl_1_bss_1BD4 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x74), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1BD4, 1);

    for (index = 0; index < lbl_1_bss_1BE0; index++) {
        position.x = lbl_1_data_1F2C[lbl_1_bss_100][index].x;
        position.y = lbl_1_data_1F2C[lbl_1_bss_100][index].y;
        position.z = lbl_1_data_1F2C[lbl_1_bss_100][index].z;
        lbl_1_bss_1BC4[index] = fn_80025D4C(lbl_1_bss_1BD4);
        fn_80026A44(lbl_1_bss_1BC4[index], 1);
        fn_80026760(lbl_1_bss_1BC4[index], position.x, position.y,
                        position.z);
        fn_80026FFC(lbl_1_bss_1BC4[index]);
        lbl_1_bss_1BB4[index] = fn_80025A04(
            fn_80008DD0(DATANUM(0xa40000, 0x78), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_1BB4[index], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_1BB4[index], 1);
        fn_80026760(lbl_1_bss_1BB4[index], position.x, position.y,
                        position.z + 40.0f);
        micQuizChoiceRecords[index].modelId = fn_80025A04(
            fn_80008DD0(
                lbl_1_data_1EE4[micQuizChoiceRecords[index].answerIndex],
                0x10000000, 2));
        fn_80026760(micQuizChoiceRecords[index].modelId, position.x,
                        position.y, position.z);
        fn_80026E44(micQuizChoiceRecords[index].modelId);
    }

    MicQuizPresenterCreate();
    lbl_1_bss_1BB2 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x73), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1BB2, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1BB2, 1);
    lbl_1_bss_1BB0 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x75), 0x10000000, 2));
    lbl_1_bss_1BAC[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x76), 0x10000000, 2));
    lbl_1_bss_1BAC[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 0x77), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_1BB0, 3);
    fn_80026B8C(lbl_1_bss_1BAC[0], 2);
    fn_80026B8C(lbl_1_bss_1BAC[1], 2);
    fn_800097F4(0xa40000);
    MicQuizCountdownInit();
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(6);
    fn_8000DAC0(fn_1_2CE80, 0x64, 0x1800, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_FUN_TIMES);
}

void fn_1_2CE80(void) {
    MicQuizResponseRunAlt(6);
}

/* Runs the fruit round's mode steps: the question message, the items' moves toward the players,
   the answer and the closing wipe. */
void fn_1_2CEA4(OMOBJ *object) {
    Vec startPosition;
    Vec targetPosition;
    Vec direction;
    f32 degrees;
    f32 rotation;

    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        lbl_1_bss_104 = 0.0f;
        fn_1_2558();
        break;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 2;
        }
        break;
    case 2:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            MicQuizQuestionReadySet();
        } else {
            fn_1_2E074();
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 4:
        if (MicQuizResponseStateGet() == 3) {
            lbl_1_bss_FC = 0x11;
        } else if (MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 0x12;
        } else if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 0xE;
        } else if (MicQuizResponseStateGet() == 5) {
            lbl_1_bss_FC = 5;
            lbl_1_bss_1B9C = -1;
        }
        break;
    case 5:
        lbl_1_bss_1BA8 = MicQuizRecognizedAnswerGet();
        lbl_1_bss_1BA4 = fn_1_2E29C(lbl_1_bss_1BA8);
        /* fall through */
    case 6:
        startPosition.x = 0.0f;
        startPosition.y = 0.0f;
        startPosition.z = -410.0f;
        targetPosition.x = lbl_1_data_1F2C[lbl_1_bss_100][lbl_1_bss_1BA4].x;
        targetPosition.y = lbl_1_data_1F2C[lbl_1_bss_100][lbl_1_bss_1BA4].y;
        targetPosition.z = lbl_1_data_1F2C[lbl_1_bss_100][lbl_1_bss_1BA4].z;
        direction.x = targetPosition.x - startPosition.x;
        direction.y = 0.0f;
        direction.z = targetPosition.z - startPosition.z;
        PSVECNormalize(&direction, &direction);
        degrees = (f32)(180.0 *
                         (atan2((f64)direction.x, (f64)direction.z) /
                          M_PI));
        rotation = (f32)fmod((f64)degrees, 360.0);
        if (0.0f > rotation) {
            rotation += 360.0f;
        }
        if (180.0f < rotation) {
            rotation -= 360.0f;
        }
        lbl_1_bss_1B90 = rotation / 90.0f;
        lbl_1_bss_1B94 = 0.0f;
        MicQuizPresenterMotionSet(2, 8.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_FC = 7;
        lbl_1_bss_104 = 0.0f;
        break;
    case 7:
        if (lbl_1_bss_104 >= 90.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 8;
        } else {
            lbl_1_bss_104 += 1.0f;
            lbl_1_bss_1B94 += lbl_1_bss_1B90;
            fn_80026808(micQuizPresenterModel, 0.0f, lbl_1_bss_1B94, 0.0f);
        }
        break;
    case 8:
        MicQuizPresenterMotionSet(4, 8.0f, 0);
        lbl_1_bss_104 = 0.0f;
        lbl_1_bss_FC = 9;
        break;
    case 9:
        if (lbl_1_bss_104 >= 50.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xA;
            fn_800269E8(micQuizPresenterModel, 0x40000002U);
            fn_8000DAC0(fn_1_2E2EC, 0x64, 0x2000, 0, *lbl_1_bss_114);
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 10:
        if (lbl_1_bss_104 >= 160.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xB;
            fn_80026A44(micQuizPresenterModel, 0x40000002U);
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 11:
        if (lbl_1_bss_104 >= 30.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xC;
            MicQuizPresenterMotionSet(0, 8.0f, ((1 << 30) | (1 << 0)));
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 12:
        MicQuizPresenterMotionSet(2, 8.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_FC = 0xD;
        break;
    case 13:
        if (lbl_1_bss_104 >= 90.0f) {
            lbl_1_bss_104 = 0.0f;
            if (lbl_1_bss_1B9C == -1) {
                lbl_1_bss_FC = 4;
            } else {
                lbl_1_bss_FC = lbl_1_bss_1B9C;
            }
            MicQuizPresentationDone();
            MicQuizPresenterMotionSet(0, 8.0f, ((1 << 30) | (1 << 0)));
        } else {
            lbl_1_bss_104 += 1.0f;
            lbl_1_bss_1B94 -= lbl_1_bss_1B90;
            fn_80026808(micQuizPresenterModel, 0.0f, lbl_1_bss_1B94, 0.0f);
        }
        break;
    case 14:
        MicQuizPresenterMotionSet(5, 0.0f, 0);
        lbl_1_bss_FC = 0xF;
        break;
    case 15:
        if (lbl_1_bss_104 >= 47.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0x10;
            fn_1_2E85C();
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 16:
        if (lbl_1_bss_104 >= 200.0f) {
            MicQuizPresenterMotionSet(0, 8.0f, ((1 << 30) | (1 << 0)));
            lbl_1_bss_FC = 4;
            MicQuizListenStart();
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 17:
        lbl_1_bss_FC = 0x14;
        break;
    case 18:
        fn_1_2670();
        lbl_1_bss_1BA4 = lbl_1_bss_1BA0;
        lbl_1_bss_FC = 6;
        lbl_1_bss_1B9C = 0x15;
        lbl_1_bss_1B98 = 1;
        break;
    case 21:
        if (lbl_1_bss_104 >= 60.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0x16;
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 22:
        HuAudFXPlay(MSM_SE_GUIDE_KOOPA_TALK);
        MicQuizPresenterMotionSet(3, 16.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_FC = 0x14;
        break;
    case 20:
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0x17;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
        } else {
            lbl_1_bss_104 += 1.0f;
        }
        break;
    case 23:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport(" KOOPA END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Debug view: moves the camera with the first pad, prints its center, rotation and zoom in a
   window, then its position and target. */
void fn_1_2DA80(void) {
    Vec cameraPosition;
    Vec centerPosition;
    GXColor windowColor;
    f32 rotationZ;
    f32 rotationY;
    f32 rotationX;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].z -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].z += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_1EA4);

    printWin(0x10, 0x28, 0x180, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x,
           lbl_80266B18[0].y, lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x,
           lbl_80266A58[0].y, lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);

    rotationX = lbl_80266A58[0].x;
    rotationY = lbl_80266A58[0].y;
    rotationZ = lbl_80266A58[0].z;
    cameraPosition.x = (f32)((f64)lbl_80266B18[0].x +
                             ((f64)lbl_80266BD8[0] *
                              (sin((M_PI * (f64)rotationY) / 180.0) *
                               cos((M_PI * (f64)rotationX) / 180.0))));
    cameraPosition.y = (f32)((f64)lbl_80266B18[0].y +
                             ((f64)lbl_80266BD8[0] * -sin((M_PI * (f64)rotationX) / 180.0)));
    cameraPosition.z = (f32)((f64)lbl_80266B18[0].z +
                             ((f64)lbl_80266BD8[0] *
                              (cos((M_PI * (f64)rotationY) / 180.0) *
                               cos((M_PI * (f64)rotationX) / 180.0))));

    centerPosition.x = lbl_80266B18[0].x;
    centerPosition.y = lbl_80266B18[0].y;
    centerPosition.z = lbl_80266B18[0].z;
    print8(0x10, 0x58, 1.0f, "CAMPOS X:%.02f Y:%.02f Z:%.02f\n", cameraPosition.x, cameraPosition.y,
           cameraPosition.z);
    print8(0x10, 0x64, 1.0f, "CAMINT X:%.02f Y:%.02f Z:%.02f\n", centerPosition.x, centerPosition.y,
           centerPosition.z);
}

void fn_1_2E074(void)
{
    Point3d positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_1BAC[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= lbl_1_bss_1BD8;
    positions[1].x += lbl_1_bss_1BD8;
    i = 0;
    while (i < 2) {
        fn_80026790(lbl_1_bss_1BAC[i], &positions[i]);
        i += 1;
    }
}

/* Picks eight different random choices for the round. */
void fn_1_2E138(void) {
    s32 selection;
    s32 index;
    index = 0;
    while (index < 8)
    {
        micQuizChoiceRecords[index].answerIndex = -1;
        micQuizChoiceRecords[index].answered = 0;
        micQuizChoiceRecords[index].modelId = 0;
        micQuizChoiceRecords[index].availabilityState = 1;
        index += 1;
    }
    index = 0;
    while (index < lbl_1_bss_1BE0)
    {
        do
        {
            selection = frandmod(8);
        }
        while (fn_1_2E240(selection) == 0);
        micQuizChoiceRecords[index].answerIndex = selection;
        micQuizChoiceRecords[index].availabilityState = 0;
        index += 1;
    }
}

/* Returns 1 when no choice of the round holds the value yet. */
s32 fn_1_2E240(s32 value) {
    s32 index;

    for (index = 0; index < lbl_1_bss_1BE0; index++) {
        if (value == micQuizChoiceRecords[index].answerIndex) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_2E29C(s32 key)
{
    s32 index;
    for (index = 0; index < 8; index++) {
        if (key == micQuizChoiceRecords[index].answerIndex) return index;
    }
    return -1;
}

/* Returns the index of the choice record that holds this answer, or -1. */
static inline s32 ChoiceRecordFind(s32 answer) {
    s32 row;

    for (row = 0; row < 8; row++) {
        if (answer == micQuizChoiceRecords[row].answerIndex) {
            return row;
        }
    }
    return -1;
}

/* Moves the answered item to its result place, turning it, with a sound. */
void fn_1_2E2EC(void) {
    Point3d itemPosition;
    Point3d animationTargetPosition;
    Point3d difference;
    Point3d targetPosition;
    Vec movementDifference;
    s32 selection;
    s32 index;
    s32 row;
    f32 opacity;
    f32 targetAngle;
    f32 rotation = 0.0f;
    f32 angle;

    if (lbl_1_bss_1B98 != 0) {
        row = lbl_1_bss_1BA4;
    } else {
        selection = lbl_1_bss_1BA8;
        row = ChoiceRecordFind(selection);
    }

    fn_1_2D18(micQuizPresenterModel, "g000m0-itemhook_M", &itemPosition);
    fn_1_2D18(micQuizPresenterModel, "g000m0-ns_hed", &animationTargetPosition);
    PSVECSubtract(&itemPosition, &animationTargetPosition, &difference);
    targetAngle = (f32)(180.0 *
                        (atan2((f64)difference.x, (f64)difference.z) /
                         M_PI));
    fn_80026760(lbl_1_bss_1BB2, itemPosition.x, itemPosition.y, itemPosition.z);
    fn_80026808(lbl_1_bss_1BB2, 15.0f, targetAngle, 0.0f);
    fn_800268B0(lbl_1_bss_1BB2, 0.0f, 0.0f, 0.0f);
    fn_80026A44(lbl_1_bss_1BB2, 1U);

    angle = 0.0f;
    HuAudFXPlay(0x534);
    do {
        HuPrcVSleep();
        opacity = (f32)sin((M_PI * (f64)angle) / 180.0);
        fn_800268B0(lbl_1_bss_1BB2, opacity, opacity, opacity);
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    fn_800268B0(lbl_1_bss_1BB2, 1.0f, 1.0f, 1.0f);

    targetPosition.x = lbl_1_data_1F2C[lbl_1_bss_100][row].x;
    targetPosition.y = lbl_1_data_1F2C[lbl_1_bss_100][row].y;
    targetPosition.z = lbl_1_data_1F2C[lbl_1_bss_100][row].z;
    PSVECSubtract(&itemPosition, &targetPosition, &movementDifference);
    {
    Vec step;

    step.x = movementDifference.x / 120.0f;
    step.y = movementDifference.y / 120.0f;
    step.z = movementDifference.z / 120.0f;
    angle = 90.0f;
    for (index = 0; index < 0x78; index++) {
        HuPrcVSleep();
        opacity = (f32)sin((M_PI * (f64)angle) / 180.0);
        targetPosition.x += step.x;
        targetPosition.y += step.y;
        targetPosition.z += step.z;
        angle += 0.75f;
        fn_80026760(micQuizChoiceRecords[row].modelId,
                        targetPosition.x, targetPosition.y, targetPosition.z);
        fn_800268B0(micQuizChoiceRecords[row].modelId, opacity, opacity, opacity);
        fn_80026808(micQuizChoiceRecords[row].modelId, 0.0f, rotation, 0.0f);
        rotation += 10.0f;
    }
    }
    HuAudFXPlay(0x535);
    micQuizChoiceRecords[row].availabilityState = 1;
    fn_800269E8(micQuizChoiceRecords[row].modelId, 1U);

    angle = 90.0f;
    do {
        HuPrcVSleep();
        opacity = (f32)sin((M_PI * (f64)angle) / 180.0);
        fn_800268B0(lbl_1_bss_1BB2, opacity, opacity, opacity);
        angle += 3.0f;
    } while (!(angle >= 180.0f));
    fn_800268B0(lbl_1_bss_1BB2, 0.0f, 0.0f, 0.0f);
    fn_800269E8(lbl_1_bss_1BB2, 1U);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Starts the process that swings the choice models, under the main process. */
void fn_1_2E85C(void) {
    fn_8000DAC0(fn_1_2E89C, 0x64U, 0x1800U, 0, lbl_1_bss_114[0]);
}

/* Swings random choice models, dims them and ends this process. */
void fn_1_2E89C(void) {
    s32 selectedIndices[6];
    s32 selectedCount;
    s32 index;
    s32 selection;
    s32 frame;
    f32 angle;
    f32 scale;
    f32 scaleAmplitude;

    selectedCount = lbl_1_data_1F18[lbl_1_bss_100];
    index = 0;
    while (index < selectedCount) {
        for (;;) {
            selection = frandmod(lbl_1_bss_1BE0);
            if (selection == lbl_1_bss_1BA0) {
                continue;
            }
            if (micQuizChoiceRecords[selection].answered == 0) {
                break;
            }
        }
        selectedIndices[index] = selection;
        micQuizChoiceRecords[selection].answered = 1;
        index++;
    }

    for (index = 0; index < selectedCount; index++) {
        fn_80026A44(lbl_1_bss_1BB4[selectedIndices[index]], 1);
        fn_800268B0(lbl_1_bss_1BB4[selectedIndices[index]], 0.0f, 0.0f, 0.0f);
    }

    angle = 0.0f;
    scale = 0.0f;
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(2.0 * sin((M_PI * angle) / 180.0f));
        for (index = 0; index < selectedCount; index++) {
            fn_800268B0(lbl_1_bss_1BB4[selectedIndices[index]], scale, scale, scale);
        }
        angle += 4.5f;
        if (angle >= 90.0f) {
            break;
        }
    }

    angle = 90.0f;
    scale = 2.0f;
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(2.0 * sin((M_PI * angle) / 180.0f));
        for (index = 0; index < selectedCount; index++) {
            fn_800268B0(lbl_1_bss_1BB4[selectedIndices[index]], scale, scale, scale);
        }
        angle += 2.0f;
        if (angle >= 150.0f) {
            break;
        }
    }

    HuPrcSleep(0x3C);
    for (frame = 0; frame < 0x3C; frame++) {
        HuPrcVSleep();
        for (index = 0; index < selectedCount; index++) {
            fn_1_2EC80(micQuizChoiceRecords[selectedIndices[index]].modelId);
        }
    }

    scaleAmplitude = scale;
    angle = 90.0f;
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(scaleAmplitude *
                      sin((M_PI * angle) / 180.0f));
        for (index = 0; index < selectedCount; index++) {
            fn_800268B0(lbl_1_bss_1BB4[selectedIndices[index]], scale, scale, scale);
        }
        angle += 3.0f;
        if (angle >= 180.0f) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

s32 lbl_1_bss_1BE0;
s16 lbl_1_bss_1BDC;
f32 lbl_1_bss_1BD8;
s16 lbl_1_bss_1BD4;
s16 lbl_1_bss_1BC4[8];
s16 lbl_1_bss_1BB4[8];
s16 lbl_1_bss_1BB2;
s16 lbl_1_bss_1BB0;
s16 lbl_1_bss_1BAC[2];
s32 lbl_1_bss_1BA8;
s32 lbl_1_bss_1BA4;
s32 lbl_1_bss_1BA0;
s32 lbl_1_bss_1B9C;
s32 lbl_1_bss_1B98;
f32 lbl_1_bss_1B94;
f32 lbl_1_bss_1B90;

/* Darkens every material of a model by a tenth. */
void fn_1_2EC80(s16 modelId) {
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_DATA *hsf = model->hsf;
    HSF_MATERIAL *material = hsf->material;
    s32 materialIndex;

    for (materialIndex = 0; materialIndex < hsf->materialNum;
         materialIndex++, material++) {
        material->color[0] *= 0.9f;
        material->color[1] *= 0.9f;
        material->color[2] *= 0.9f;
    }
}
