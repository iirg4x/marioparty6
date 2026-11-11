/* Mix quiz: the cast models, mic answers that step through the cast, and the quiz's end. */
#include "REL/micquizishidll/micquizishidll_wipe_input_views_v2.h"

#include "REL/micquizishidll/module_base.h"

#include "game/window.h"

#include "dolphin/os.h"

#include "game/process.h"

#include "game/wipe.h"

#include "game/hu3d.h"

#include "game/printfunc.h"

#include "math.h"

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

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "REL/micquizishidll/related_types.h"

#include "REL/micquizishidll/process_manager.h"

#include "REL/micquizishidll/micquizishidll_memory_button_views_v1.h"

#include "REL/micquizishidll/micquizishidll_menu_callback_views_v1.h"

#include "REL/micquizishidll/micquizishidll_message_setup_views_v2.h"
#include "REL/micquizishidll.h"
#include <game/mic.h>

typedef struct MotionFiles {
    s32 fileIds[4];
} MotionFiles;

typedef struct MotionPair {
    s16 motion[4];
} MotionPair;

u8 lbl_1_data_2370[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
u32 lbl_1_data_238C[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 0x21000C, 0x21000D, 0x21000E, 0x21000F,
                           0x210010, 0x210011 };
s32 lbl_1_data_23CC[10] = { 0x920000, 0x3A0000, 0xC40000, 0xF70000, 0xF00000, 0x180000, 0xEA0000,
                           0x340000, 0xBB0000, 0x2E0000 };
s32 lbl_1_data_23F4[10] = { 0x930000, 0x3B0000, 0xC50000, 0xF80000, 0xF10000, 0x190000, 0xEB0000,
                           0x350000, 0xBC0000, 0x2F0000 };
s32 lbl_1_data_241C[10] = { 0x920000, 0x3A0000, 0xC40000, 0xF70000, 0xF00000, 0x180000, 0xEA0000,
                           0x340000, 0xBB0000, 0x2E0000 };
MotionFiles lbl_1_data_2444[10] = { { { 0x930000, 0x930001, 0x930002, 0x930024 } },
        { { 0x3B0000, 0x3B0001, 0x3B0002, 0x3B0024 } }, { { 0xC50000, 0xC50001, 0xC50002,
                0xC50024 } }, { { 0xF80000, 0xF80001, 0xF80002, 0xF80024 } }, { { 0xF10000,
                        0xF10001, 0xF10002, 0xF10024 } }, { { 0x190000, 0x190001, 0x190002,
                                0x190024 } }, { { 0xEB0000, 0xEB0001, 0xEB0002,
                                        0xEB0024 } }, { { 0x350000, 0x350001, 0x350002,
                                                0x350024 } }, { { 0xBC0000, 0xBC0001, 0xBC0002,
                                                        0xBC0024 } }, { { 0x2F0000, 0x2F0001,
                                                                0x2F0002, 0x2F0024 } } };
MotionFiles lbl_1_data_24E4[6] = { { { 0xA40001, 0xA40002, 0xA40003, 0xA40004 } },
        { { 0xA40006, 0xA40007, 0xA40008, 0xA40009 } }, { { 0xA4000B, 0xA4000C, 0xA4000D,
                0xA4000E } }, { { 0xA40015, 0xA40016, 0xA40017, 0xA40019 } }, { { 0xA4001B,
                        0xA40020, 0xA40021, 0xA4001F } }, { { 0xA40010, 0xA40011, 0xA40012,
                                0xA40013 } } };
s32 lbl_1_data_2544[6] = { 0xA40000, 0xA40005, 0xA4000A, 0xA40014, 0xA4001A, 0xA4000F };
s16 lbl_1_data_255C[16] = { 579, 555, 699, 795, 747, 483, 771, 507, 651, 531, 960, 981, 986, 992,
                           939, 999 };
Point3d lbl_1_data_257C = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_2588 = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_2594 = { 0.0f, 0.0f, -400.0f };
OM_CAMERA_VIEW lbl_1_data_25A0 = { { 0.0f, 195.0f, 0.0f }, { -6.3f, 0.0f, 0.0f }, 6195.0f };
Vec lbl_1_data_25BC[6][3] = { { { -75.0f, 0.0f, -150.0f }, { 75.0f, 0.0f, -150.0f },
        { 0.0f, 0.0f, 0.0f } }, { { -150.0f, 0.0f, -150.0f }, { 0.0f, 0.0f, -150.0f },
                { 150.0f, 0.0f, -150.0f } }, { { -150.0f, 0.0f, -150.0f }, { 0.0f, 0.0f, -150.0f },
                        { 150.0f, 0.0f, -150.0f } }, { { -75.0f, 0.0f, -150.0f },
                                { 75.0f, 0.0f, -150.0f }, { 0.0f, 0.0f,
                                        0.0f } }, { { -150.0f, 0.0f, -150.0f },
                                                { 0.0f, 0.0f, -150.0f }, { 150.0f, 0.0f,
                                                        -150.0f } }, { { -150.0f, 0.0f, -150.0f },
                                                                { 0.0f, 0.0f, -150.0f }, { 150.0f,
                                                                        0.0f, -150.0f } } };
u8 lbl_1_data_2694[0x48] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

extern HU3D_MODELID lbl_1_bss_1C54[6];
extern HU3D_MODELID lbl_1_bss_1CB0[10];
extern MotionPair lbl_1_bss_1C24[6];
extern MotionPair lbl_1_bss_1C60[10];

/* Waits for START, then sets the mode to 2, starts the closing wipe and ends this process. */
void fn_1_2F83C(void) {
    do {
        HuPrcVSleep();
    } while ((lbl_802C0008[0] & PAD_BUTTON_START) == 0);

    lbl_1_bss_FC = 2;
    WipeCreate(2, 0, 0x14);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_301DC(void);

void fn_1_3066C(s32, s32);

void fn_1_30FD8(s32);

void fn_1_31DF0(void);

void fn_1_31F4C(void);

void fn_1_326F0(void);

extern f32 lbl_1_bss_104;

extern HUPROCESS *lbl_1_bss_114[0xB];

extern s32 lbl_1_bss_1C04;

extern s32 lbl_1_bss_1C08;

extern s32 lbl_1_bss_FC;

extern s16 lbl_1_bss_9C[2];

extern s16 lbl_1_bss_1C0C[3];

extern s16 lbl_1_bss_1C12[2];

extern s16 lbl_1_bss_1C16;

extern s32 lbl_1_bss_1CC4;

extern s32 lbl_1_bss_110;

extern s16 lbl_1_bss_A0;

extern s16 lbl_1_bss_A2;

extern s16 lbl_1_bss_A4;

extern OMOBJ *lbl_1_bss_A8;

extern u8 lbl_1_data_488[4];

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

/* Sets up the mix quiz scene: camera, lights and shadow, the scene objects and the
   /mic/ctx/micquiz_mix context. */
void fn_1_2F890(void) {
    HUWIN *window;
    s32 index;

    lbl_1_bss_104 += 1.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0U, 0U, 0x280U, 0x1E0U);
    fn_8003E488(1, &lbl_1_data_25A0);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0U, 0U, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U, 0U);
    Hu3DGLightPosAimSetV((s16) lbl_1_bss_110, &lbl_1_data_470, &lbl_1_data_47C);
    Hu3DGLightStaticSet((s16) lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet((s16) lbl_1_bss_110);
    Hu3DGLightColorSet((s16) lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10U, 0x10U, 0x10U);
    Hu3DShadowPosSet(&lbl_1_data_257C, &lbl_1_data_2588, &lbl_1_data_2594);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(0xA40022, 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(fn_80008DD0(0xA40081, 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(0xA4002B, 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(0xA4002C, 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(0xA4002D, 0x10000000, 2));
    lbl_1_bss_1CC4 = 0;
    index = 0;
    while (index < 0xA) {
        lbl_1_bss_1CB0[index] = -1;
        index += 1;
    }
    index = 0;
    while (index < 6) {
        lbl_1_bss_1C54[index] = -1;
        index += 1;
    }
    fn_1_31548(0);
    MicQuizEffectParentSetAlt();
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    lbl_1_bss_1C16 = HuMCContextCreate("/mic/ctx/micquiz_mix");
    HuMCSelWinItemSet(0x21004E, 3, 0);
    lbl_1_bss_FC = 0;
    lbl_1_bss_1C12[0] = HuWinCreate(148.0f, 150.0f, 0x79, 0x21, 1);
    window = &winData[lbl_1_bss_1C12[0]];
    window->mesRectY += 4;
    lbl_1_bss_1C12[1] = HuWinCreate(300.0f, 150.0f, 0x79, 0x21, 1);
    window = &winData[lbl_1_bss_1C12[1]];
    window->mesRectY += 4;
    index = 0;
    while (index < 2) {
        HuWinMesSpeedSet(lbl_1_bss_1C12[index], 0);
        HuWinAttrSet(lbl_1_bss_1C12[index], 0x800U);
        HuWinExOpen(lbl_1_bss_1C12[index]);
        HuWinDispOff(lbl_1_bss_1C12[index]);
        index += 1;
    }
    lbl_1_bss_1C0C[0] = HuWinCreate(148.0f, 170.0f, 0x79, 0x21, 1);
    window = &winData[lbl_1_bss_1C0C[0]];
    window->mesRectY += 4;
    lbl_1_bss_1C0C[1] = HuWinCreate(228.0f, 120.0f, 0x79, 0x21, 1);
    window = &winData[lbl_1_bss_1C0C[1]];
    window->mesRectY += 4;
    lbl_1_bss_1C0C[2] = HuWinCreate(300.0f, 170.0f, 0x79, 0x21, 1);
    window = &winData[lbl_1_bss_1C0C[2]];
    window->mesRectY += 4;
    index = 0;
    while (index < 3) {
        HuWinMesSpeedSet(lbl_1_bss_1C0C[index], 0);
        HuWinAttrSet(lbl_1_bss_1C0C[index], 0x800U);
        HuWinExOpen(lbl_1_bss_1C0C[index]);
        HuWinDispOff(lbl_1_bss_1C0C[index]);
        index += 1;
    }
}

/* Runs the mix quiz's mode steps: after the opening wipe starts the quiz processes, and at the
   end closes the mic selection window and the mic. */
void fn_1_2FFA4(void) {
    switch (lbl_1_bss_FC) {
    case 0:
        if (WipeCheck() == 0) {
            lbl_1_bss_FC = 1;
            fn_8000DAC0(fn_1_326F0, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
            fn_8000DAC0(fn_1_31F4C, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
        }
        break;
    case 1:
        if (lbl_1_bss_1C04 != 0)
            lbl_1_bss_FC = 2;
        break;
    case 2:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 3;
            fn_1_30FD8(0);
            fn_1_3066C(1, 0);
        } else {
            lbl_1_bss_104 += 1.0f;
            fn_1_301DC();
        }
        break;
    case 3:
        if (lbl_1_bss_1C08 != 0) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            OSReport("                           START2\n");
            fn_8000DAC0(fn_1_31DF0, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
            if (HuMCSelWinCheck() != 0) {
                MicQuizInstructionSet(0xA);
            } else {
                MicQuizInstructionSet(9);
            }
        }
        break;
    case 5:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

void fn_1_301DC(void)
{
    Point3d positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= 8.888889f;
    positions[1].x += 8.888889f;
    i = 0;
    while (i < 2) {
        fn_80026790(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_302A0(void) {
    GXColor windowColor;

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

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_25A0);

    printWin(0x10, 0x28, 0x180, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
}

extern s32 lbl_1_bss_1C18[3];

void fn_1_31C50(s32 playerNo, s32 motionNo, f32 startTime, u32 attr);

void fn_1_31D20(s32 playerNo, s32 motionNo, f32 startTime, u32 attr);

/* Animates the cast models while the cast changes, forward or backward, hiding the message
   windows first when asked. */
void fn_1_3066C(s32 showWindows, s32 moveForward) {
    s32 modelCount;
    s32 modelBase;
    s32 frame;
    s32 index;
    s32 windowIndex;
    s16 modelIndex;
    HuVecF modelPosition;
    f32 angle;
    f32 angleStep;
    f32 positionStep;

    lbl_1_bss_1C08 = 0;
    if (lbl_1_bss_1CC4 == 0) {
        modelCount = 2;
        modelBase = 0;
    } else if (lbl_1_bss_1CC4 == 3) {
        modelCount = 2;
        modelBase = 1;
    } else {
        modelCount = 3;
        modelBase = 1;
    }
    if (showWindows != 0) {
        for (windowIndex = 0; windowIndex < 3; windowIndex++) {
            HuWinDispOff(lbl_1_bss_1C0C[windowIndex]);
        }
        for (windowIndex = 0; windowIndex < 2; windowIndex++) {
            HuWinDispOff(lbl_1_bss_1C12[windowIndex]);
        }
    }
    angle = 0.0f;
    angleStep = 9.0f;
    for (frame = 0; (f32) frame < 10.0f; frame++) {
        HuPrcVSleep();
        if (moveForward != 0) {
            angle += angleStep;
        } else {
            angle -= angleStep;
        }
        if (lbl_1_bss_1CC4 <= 3) {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + lbl_1_bss_1CC4 * 3 - modelBase;
                fn_1_31C50(modelIndex, 1, 8.0f, 0x40000001U);
                fn_80026808(lbl_1_bss_1CB0[modelIndex], 0.0f, angle, 0.0f);
            }
        } else {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + (lbl_1_bss_1CC4 - 4) * 3;
                fn_1_31D20(modelIndex, 1, 8.0f, 0x40000001U);
                fn_80026808(lbl_1_bss_1C54[modelIndex], 0.0f, angle, 0.0f);
            }
        }
    }
    positionStep = 9.285714f;
    for (frame = 0; (f32) frame < 70.0f; frame++) {
        HuPrcVSleep();
        if (lbl_1_bss_1CC4 <= 3) {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + lbl_1_bss_1CC4 * 3 - modelBase;
                fn_1_31C50(modelIndex, 2, 16.0f, 0x40000001U);
                fn_800267CC(lbl_1_bss_1CB0[modelIndex], &modelPosition);
                if (moveForward != 0) {
                    modelPosition.x += positionStep;
                } else {
                    modelPosition.x -= positionStep;
                }
                fn_80026760(lbl_1_bss_1CB0[modelIndex], modelPosition.x,
                                modelPosition.y, modelPosition.z);
                if (moveForward != 0) {
                    fn_80026808(lbl_1_bss_1CB0[modelIndex], 0.0f, 90.0f, 0.0f);
                } else {
                    fn_80026808(lbl_1_bss_1CB0[modelIndex], 0.0f, -90.0f, 0.0f);
                }
            }
        } else {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + (lbl_1_bss_1CC4 - 4) * 3;
                fn_1_31D20(modelIndex, 2, 16.0f, 0x40000001U);
                fn_800267CC(lbl_1_bss_1C54[modelIndex], &modelPosition);
                if (moveForward != 0) {
                    modelPosition.x += positionStep;
                } else {
                    modelPosition.x -= positionStep;
                }
                fn_80026760(lbl_1_bss_1C54[modelIndex], modelPosition.x,
                                modelPosition.y, modelPosition.z);
                if (moveForward != 0) {
                    fn_80026808(lbl_1_bss_1C54[modelIndex], 0.0f, 90.0f, 0.0f);
                } else {
                    fn_80026808(lbl_1_bss_1C54[modelIndex], 0.0f, -90.0f, 0.0f);
                }
            }
        }
    }
    angleStep = 9.0f;
    for (frame = 0; (f32) frame < 10.0f; frame++) {
        HuPrcVSleep();
        if (moveForward != 0) {
            angle -= angleStep;
        } else {
            angle += angleStep;
        }
        if (lbl_1_bss_1CC4 <= 3) {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + lbl_1_bss_1CC4 * 3 - modelBase;
                fn_1_31C50(modelIndex, 1, 8.0f, 0x40000001U);
                fn_80026808(lbl_1_bss_1CB0[modelIndex], 0.0f, angle, 0.0f);
            }
        } else {
            for (index = 0; index < modelCount; index++) {
                modelIndex = index + (lbl_1_bss_1CC4 - 4) * 3;
                fn_1_31D20(modelIndex, 1, 8.0f, 0x40000001U);
                fn_80026808(lbl_1_bss_1C54[modelIndex], 0.0f, angle, 0.0f);
            }
        }
    }
    if (lbl_1_bss_1CC4 <= 3) {
        for (index = 0; index < modelCount; index++) {
            modelIndex = index + lbl_1_bss_1CC4 * 3 - modelBase;
            fn_1_31C50(modelIndex, 0, 8.0f, 0x40000001U);
        }
    } else {
        for (index = 0; index < modelCount; index++) {
            modelIndex = index + (lbl_1_bss_1CC4 - 4) * 3;
            fn_1_31D20(modelIndex, 0, 8.0f, 0x40000001U);
        }
    }
    if (showWindows != 0) {
        for (windowIndex = 0; windowIndex < 3; windowIndex++) {
            HuWinDispOff(lbl_1_bss_1C0C[windowIndex]);
        }
        for (windowIndex = 0; windowIndex < 2; windowIndex++) {
            HuWinDispOff(lbl_1_bss_1C12[windowIndex]);
        }
        for (index = 0; index < modelCount; index++) {
            if (lbl_1_bss_1CC4 == 0 || lbl_1_bss_1CC4 == 3) {
                HuWinDispOn(lbl_1_bss_1C12[index]);
                HuWinMesSet(lbl_1_bss_1C12[index],
                            lbl_1_data_238C[lbl_1_bss_1C18[index]]);
            } else {
                HuWinDispOn(lbl_1_bss_1C0C[index]);
                HuWinMesSet(lbl_1_bss_1C0C[index],
                            lbl_1_data_238C[lbl_1_bss_1C18[index]]);
            }
        }
    }
            OSReport("                           START1\n");
    lbl_1_bss_1C08 = 1;
}

/* Places the models of the given cast and starts their motions, reporting the cast number. */
void fn_1_30FD8(s32 argument) {
    f32 horizontalOffset;
    s16 modelIndex;
    s32 modelCount;
    s32 playerOffset;
    s32 index;

    if (argument != 0) {
        horizontalOffset = -650.0f;
    } else {
        horizontalOffset = 650.0f;
    }

    if (lbl_1_bss_1CC4 == 0) {
        modelCount = 2;
        playerOffset = 0;
    } else if (lbl_1_bss_1CC4 == 3) {
        modelCount = 2;
        playerOffset = 1;
    } else {
        modelCount = 3;
        playerOffset = 1;
    }

    for (index = 0; index < 3; index++) {
        lbl_1_bss_1C18[index] = -1;
    }

    if (lbl_1_bss_1CC4 <= 3) {
        for (index = 0; index < modelCount; index++) {
            if (lbl_1_bss_1CC4 == 0) {
                lbl_1_bss_1C18[index] = index + lbl_1_bss_1CC4 * 3;
            } else {
                lbl_1_bss_1C18[index] = index + lbl_1_bss_1CC4 * 3 - 1;
            }
            modelIndex = index + lbl_1_bss_1CC4 * 3 - playerOffset;
            fn_80026A44(lbl_1_bss_1CB0[modelIndex], 1);
            fn_80026760(lbl_1_bss_1CB0[modelIndex],
                            horizontalOffset + lbl_1_data_25BC[lbl_1_bss_1CC4][index].x,
                            lbl_1_data_25BC[lbl_1_bss_1CC4][index].y,
                            lbl_1_data_25BC[lbl_1_bss_1CC4][index].z);
            fn_1_31C50(index + lbl_1_bss_1CC4 * 3 - playerOffset,
                       2, 16.0f, 0x40000001);
        }
    } else {
        for (index = 0; index < modelCount; index++) {
            if (lbl_1_bss_1CC4 == 0 || lbl_1_bss_1CC4 == 3) {
                lbl_1_bss_1C18[index] = index + lbl_1_bss_1CC4 * 3;
            } else {
                lbl_1_bss_1C18[index] = index + lbl_1_bss_1CC4 * 3 - 2;
            }
            modelIndex = index + (lbl_1_bss_1CC4 - 4) * 3;
            fn_80026A44(lbl_1_bss_1C54[modelIndex], 1);
            fn_80026760(lbl_1_bss_1C54[modelIndex],
                            horizontalOffset + lbl_1_data_25BC[lbl_1_bss_1CC4][index].x,
                            lbl_1_data_25BC[lbl_1_bss_1CC4][index].y,
                            lbl_1_data_25BC[lbl_1_bss_1CC4][index].z);
            fn_1_31D20(index + (lbl_1_bss_1CC4 - 4) * 3,
                       2, 16.0f, 0x40000001);
        }
    }

    for (index = 0; index < 3; index++) {
        OSReport("NOW CAST:%d\n", lbl_1_bss_1C18[index]);
    }
}

/* Shows the models the current cast uses and hides the others. */
void fn_1_31430(void) {
    s32 modelOffset;
    s32 modelCount;
    s32 index;
    if ((lbl_1_bss_1CC4 == 0) || (lbl_1_bss_1CC4 == 3))
    {
        modelCount = 2;
        modelOffset = 0;
    }
    else
    {
        modelCount = 3;
        modelOffset = 1;
    }
    if (lbl_1_bss_1CC4 <= 3)
    {
        index = 0;
        while (index < modelCount)
        {
            fn_800269E8(lbl_1_bss_1CB0[index + (lbl_1_bss_1CC4 * 3) - modelOffset], 1U);
            index += 1;
        }
        return;
    }
    index = 0;
    while (index < modelCount)
    {
        fn_800269E8(lbl_1_bss_1C54[index + (lbl_1_bss_1CC4 - 4) * 3], 1U);
        index += 1;
    }
}

/* Creates the player and NPC models and their motions for the given mode. */
s32 fn_1_31548(s32 mode) {
    s32 slotCount;
    s32 firstSlot;
    s32 slot;
    s32 motionIndex;
    s32 modelIndex;

    if (mode == 0) {
        slotCount = 2;
        firstSlot = 0;
    } else if (mode == 3) {
        slotCount = 2;
        firstSlot = 1;
    } else {
        slotCount = 3;
        firstSlot = 1;
    }

    if (lbl_1_bss_1CC4 <= 3) {
        for (slot = 0; slot < slotCount; slot++) {
            modelIndex = mode * 3 + slot - firstSlot;
            if (lbl_1_bss_1CB0[modelIndex] == -1) {
                lbl_1_bss_1CB0[modelIndex] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_241C[modelIndex], 0x10000000, 2));
                fn_800097F4(lbl_1_data_23CC[modelIndex]);
                for (motionIndex = 0; motionIndex < 4; motionIndex++) {
                    lbl_1_bss_1C60[modelIndex].motion[motionIndex] = fn_80030458(
                        lbl_1_bss_1CB0[modelIndex],
                        fn_80008DD0(lbl_1_data_2444[modelIndex].fileIds[motionIndex],
                                             0x10000000, 2));
                }
                fn_800097F4(lbl_1_data_23F4[modelIndex]);
                fn_800269E8(lbl_1_bss_1CB0[modelIndex], 0x40000001);
                fn_800269E8(lbl_1_bss_1CB0[modelIndex], 1);
                fn_8002C95C(lbl_1_bss_1CB0[modelIndex], lbl_1_bss_1C60[modelIndex].motion[0]);
                fn_80026E44(lbl_1_bss_1CB0[modelIndex]);
            }
        }
    } else {
        for (slot = 0; slot < slotCount; slot++) {
            modelIndex = (mode - 4) * 3 + slot;
            if (lbl_1_bss_1C54[modelIndex] == -1) {
                lbl_1_bss_1C54[modelIndex] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_2544[modelIndex], 0x10000000, 2));
                if (modelIndex == 4) {
                    fn_800268B0(lbl_1_bss_1C54[modelIndex], 0.5f, 0.5f, 0.5f);
                }
                for (motionIndex = 0; motionIndex < 4; motionIndex++) {
                    lbl_1_bss_1C24[modelIndex].motion[motionIndex] = fn_80030458(
                        lbl_1_bss_1C54[modelIndex],
                        fn_80008DD0(lbl_1_data_24E4[modelIndex].fileIds[motionIndex],
                                             0x10000000, 2));
                }
                fn_800269E8(lbl_1_bss_1C54[modelIndex], 0x40000001);
                fn_800269E8(lbl_1_bss_1C54[modelIndex], 1);
                fn_8002C95C(lbl_1_bss_1C54[modelIndex], lbl_1_bss_1C24[modelIndex].motion[0]);
                fn_80026E44(lbl_1_bss_1C54[modelIndex]);
            }
        }
    }
}

/* Creates the ten player models with their motions. */
void fn_1_31904(void) {
    s32 motionIndex;
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < 0xA) {
        lbl_1_bss_1CB0[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_data_241C[modelIndex],
                0x10000000, 2));
        fn_800097F4(lbl_1_data_23CC[modelIndex]);
        motionIndex = 0;
        while (motionIndex < 4) {
            lbl_1_bss_1C60[modelIndex].motion[motionIndex] = fn_80030458(lbl_1_bss_1CB0[modelIndex],
                    fn_80008DD0(lbl_1_data_2444[modelIndex].fileIds[motionIndex], 0x10000000, 2));
            motionIndex += 1;
        }
        fn_800097F4(lbl_1_data_23F4[modelIndex]);
        fn_800269E8(lbl_1_bss_1CB0[modelIndex], 0x40000001U);
        fn_800269E8(lbl_1_bss_1CB0[modelIndex], 1U);
        fn_8002C95C(lbl_1_bss_1CB0[modelIndex], lbl_1_bss_1C60[modelIndex].motion[0]);
        fn_80026E44(lbl_1_bss_1CB0[modelIndex]);
        modelIndex += 1;
    }
}

/* Creates the six NPC models with their motions. */
void fn_1_31AA0(void) {
    s32 motionIndex;
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < 6) {
        lbl_1_bss_1C54[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_data_2544[modelIndex],
                0x10000000, 2));
        if (modelIndex == 4) {
            fn_800268B0(lbl_1_bss_1C54[modelIndex], 0.5f, 0.5f, 0.5f);
        }
        motionIndex = 0;
        while (motionIndex < 4) {
            lbl_1_bss_1C24[modelIndex].motion[motionIndex] = fn_80030458(lbl_1_bss_1C54[modelIndex],
                    fn_80008DD0(lbl_1_data_24E4[modelIndex].fileIds[motionIndex], 0x10000000, 2));
            motionIndex += 1;
        }
        fn_800269E8(lbl_1_bss_1C54[modelIndex], 0x40000001U);
        fn_800269E8(lbl_1_bss_1C54[modelIndex], 1U);
        fn_8002C95C(lbl_1_bss_1C54[modelIndex], lbl_1_bss_1C24[modelIndex].motion[0]);
        fn_80026E44(lbl_1_bss_1C54[modelIndex]);
        modelIndex += 1;
    }
}

/* Starts a player model's motion unless that motion is already playing. */
void fn_1_31C50(s32 playerNo, s32 motionNo, f32 startTime, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_1CB0[playerNo]);
    if (currentMotion != lbl_1_bss_1C60[playerNo].motion[motionNo]) {
        fn_8002CCC0(lbl_1_bss_1CB0[playerNo], lbl_1_bss_1C60[playerNo].motion[motionNo],
                           0.0f, startTime, attr);
    }
}

/* Starts an NPC model's motion unless that motion is already playing. */
void fn_1_31D20(s32 playerNo, s32 motionNo, f32 startTime, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_1C54[playerNo]);
    if (currentMotion != lbl_1_bss_1C24[playerNo].motion[motionNo]) {
        fn_8002CCC0(lbl_1_bss_1C54[playerNo], lbl_1_bss_1C24[playerNo].motion[motionNo],
                           0.0f, startTime, attr);
    }
}

void fn_1_31FA0(s32 playerId);
s32 fn_1_3232C(void);
void fn_1_325D8(void);
void fn_1_32664(void);

/* The quiz loop: reads each mic answer and moves the cast forward or back, until the quiz is
   left through a wipe. */
void fn_1_31DF0(void) {
    s32 result;

    for (;;) {
        HuPrcVSleep();
        result = fn_1_3232C();
        if (HuMCSelWinCheck() != 0) {
            if (result == 1) {
                MicQuizMicHudStateSet(0);
                fn_1_325D8();
                continue;
            }
            if (result == 2) {
                MicQuizMicHudStateSet(0);
                fn_1_32664();
                continue;
            }
            if (result == 3) {
                MicQuizMicHudStateSet(0);
                WipeCreate(2, 0, 0x14);
                lbl_1_bss_FC = 5;
            } else {
                continue;
            }
        } else {
            if (result == -2) {
                MicQuizMicHudStateSet(0);
                HuPrcSleep(0x3C);
                continue;
            }
            if (result == -3) {
                MicQuizMicHudStateSet(0);
                HuPrcSleep(0x3C);
                continue;
            }
            if (result == -4) {
                MicQuizMicHudStateSet(0);
                HuPrcSleep(0x3C);
                continue;
            }
            if (result == 0) {
                MicQuizMicHudStateSet(0);
                fn_1_325D8();
                continue;
            }
            if (result == 1) {
                MicQuizMicHudStateSet(0);
                fn_1_32664();
                continue;
            }
            if (result == 2) {
                MicQuizMicHudStateSet(0);
                WipeCreate(2, 0, 0x14);
                lbl_1_bss_FC = 5;
            } else {
                fn_1_31FA0(result - 3);
                continue;
            }
        }
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
}

/* Waits for START, then sets the mode, starts the closing wipe and ends this process. */
void fn_1_31F4C(void) {
    do {
        HuPrcVSleep();
    } while ((lbl_802C0008[0] & PAD_BUTTON_START) == 0);

    WipeCreate(2, 0, 0x14);
    lbl_1_bss_FC = 5;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Plays a player's answer reaction: the player's and the NPC's motions and sounds, then their
   end motions. */
void fn_1_31FA0(s32 playerId) {
    s16 npcMotionId;
    s16 playerMotionId;
    s16 npcEndMotionId;
    s16 playerEndMotionId;
    s32 matched;
    s32 searchIndex;
    s32 motionIndex;

    matched = 0;
    OSReport("plnum:%d\n", playerId);
    for (searchIndex = 0; searchIndex < 3; searchIndex++) {
        if (playerId == lbl_1_bss_1C18[searchIndex]) {
            if (playerId >= 10) {
                matched = 1;
            }
            MicQuizMicHudStateSet(0);
            if (matched != 0) {
                motionIndex = playerId - 10;
                OSReport("ActNpc:%d\n", motionIndex);
                HuAudFXPlay(lbl_1_data_255C[playerId]);
                npcMotionId = fn_8002D3C4(lbl_1_bss_1C54[motionIndex]);
                if (npcMotionId != lbl_1_bss_1C24[motionIndex].motion[3]) {
                    fn_8002CCC0(lbl_1_bss_1C54[motionIndex],
                                       lbl_1_bss_1C24[motionIndex].motion[3],
                                       0.0f, 8.0f, 0);
                }
            } else {
                motionIndex = playerId;
                OSReport("ActPl:%d\n", motionIndex);
                HuAudFXPlay(lbl_1_data_255C[playerId]);
                playerMotionId = fn_8002D3C4(lbl_1_bss_1CB0[playerId]);
                if (playerMotionId != lbl_1_bss_1C60[playerId].motion[3]) {
                    fn_8002CCC0(lbl_1_bss_1CB0[playerId],
                                       lbl_1_bss_1C60[playerId].motion[3],
                                       0.0f, 0.0f, 0);
                }
            }

            HuPrcSleep(8);
            while (1) {
                HuPrcVSleep();
                if (matched != 0) {
                    if (fn_8002D75C(lbl_1_bss_1C54[motionIndex]) != 0) {
                        npcEndMotionId = fn_8002D3C4(lbl_1_bss_1C54[motionIndex]);
                        if (npcEndMotionId != lbl_1_bss_1C24[motionIndex].motion[0]) {
                            fn_8002CCC0(lbl_1_bss_1C54[motionIndex],
                                                lbl_1_bss_1C24[motionIndex].motion[0],
                                               0.0f, 8.0f, 0x40000001);
                        }
                        return;
                    }
                } else if (fn_8002D75C(lbl_1_bss_1CB0[motionIndex]) != 0) {
                    playerEndMotionId = fn_8002D3C4(lbl_1_bss_1CB0[playerId]);
                    if (playerEndMotionId != lbl_1_bss_1C60[playerId].motion[0]) {
                        fn_8002CCC0(lbl_1_bss_1CB0[playerId],
                                            lbl_1_bss_1C60[playerId].motion[0],
                                           0.0f, 0.0f, 0x40000001);
                    }
                    return;
                }
            }
        }
    }

    OSReport("Nothing \n");
    MicQuizMicHudStateSet(0);
    HuPrcSleep(0x3C);
}

extern void fn_1_32470(void);

typedef struct MicquizResponse {
    s16 result;
    u16 score;
    s16 wordCount;
    s16 *words;
    s32 recognizedWord;
} MicquizResponse;

extern MicquizResponse *lbl_1_bss_1C00;

/* Opens a mic selection, waits for the choice and returns the recognized response. */
s32 fn_1_3232C(void) {
    u32 selectionData[1];
    HUPROCESS *childProcess;
    s32 selectedValue;

    childProcess = 0;
    selectionData[0] = 0x490056;
    childProcess = fn_8000DAC0(fn_1_32470, 0x64, 0x1800, 0, lbl_1_bss_114[0]);
    MicQuizMicHudStateSet(1);
    HuMCSelWinContextSet(lbl_1_bss_1C16, 0, 0);
    HuMCSelWinChoiceGet();
    HuPrcKill(childProcess);
    lbl_1_bss_1C00 = (MicquizResponse *)HuMCCurResponseGet();
    if (lbl_1_bss_1C00->result == 0) {
        selectedValue = lbl_1_bss_1C00->recognizedWord;
    } else if (lbl_1_bss_1C00->result == -1) {
        return -2;
    } else if (lbl_1_bss_1C00->result == -2) {
        return -2;
    } else if (lbl_1_bss_1C00->result == -4) {
        return -3;
    }
    OSReport("MIC RET:%d \n", selectedValue);
    return selectedValue;
}

/* Handles the mic selection window's buttons each frame while it is open. */
void fn_1_32470(void) {
    s32 handledSelectionMode = 0;

    for (;;) {
        HuPrcVSleep();
        if (HuMCSelWinCheck() != 0) {
            if ((s16)HuMCSelModeGet() == 1) {
                if (handledSelectionMode == 0) {
                    MicQuizRingsStart();
                    MicQuizMicHudStateSet(3);
                    handledSelectionMode = 1;
                }
            } else {
                handledSelectionMode = 0;
            }
            continue;
        }

        if (HuMCButtonGet() != 0) {
            MicQuizRingsStart();
            MicQuizMicHudStateSet(3);
        } else {
            MicQuizRingsStop();
        }

        if (HuMCButtonDownGet() != 0) {
            MicQuizRingsStart();
            MicQuizMicHudStateSet(3);
            continue;
        }
        handledSelectionMode = 0;
    }
}

/* Shows a single message window for a while, then closes it. */
void fn_1_3250C(void) {
    HuVec2f messageSize;
    s32 windowId;

    HuWinMesMaxSizeGet(1, &messageSize, 0x1C003A);
    windowId = HuWinExCreateFrame(-10000.0f, 200.0f,
        (s16)messageSize.x, (s16)messageSize.y, -1, 0);
    HuWinExOpen((s16)windowId);
    HuWinMesSpeedSet((s16)windowId, 0);
    HuWinAttrSet((s16)windowId, 0x800);
    HuWinMesSet((s16)windowId, 0x1C003A);
    HuPrcSleep(0x3C);
    HuWinExClose((s16)windowId);
    HuWinKill((s16)windowId);
}

/* Moves to the next cast (after the last, back to the first) and rebuilds its models. */
void fn_1_325D8(void) {
    fn_1_3066C(1, 0);
    lbl_1_bss_1CC4 += 1;
    if (lbl_1_bss_1CC4 > 5) {
        lbl_1_bss_1CC4 = 0;
    }
    fn_1_31548(lbl_1_bss_1CC4);
    fn_1_30FD8(0);
    fn_1_3066C(1, 0);
}

/* Moves to the previous cast (before the first, to the last) and rebuilds its models. */
void fn_1_32664(void) {
    fn_1_3066C(1, 1);
    lbl_1_bss_1CC4 -= 1;
    if (lbl_1_bss_1CC4 < 0) {
        lbl_1_bss_1CC4 = 5;
    }
    fn_1_31548(lbl_1_bss_1CC4);
    fn_1_30FD8(1);
    fn_1_3066C(1, 1);
}

/* Ends the quiz: runs its closing steps, marks it finished and ends this process. */
void fn_1_326F0(void) {
    fn_1_32724();
    MicQuizWaitTwoSeconds();
    fn_1_3275C();
    lbl_1_bss_1C04 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_32724(void) {
    MicQuizWinOpen(2);
    MicQuizWinMesSet(0x220000, 1, 2);
    MicQuizWinMesWait();
}

void fn_1_3275C(void) {
    MicQuizWinOpen(2);
    MicQuizWinMesSet(0x220001, 1, 2);
    MicQuizWinMesWait();
}

void fn_1_32798(void) {
    MicQuizWinOpen(2);
    MicQuizWinMesSet(0x1C0030, 1, 2);
    MicQuizWinMesWait();
}

void fn_1_327D4(void) {
    MicQuizWinOpen(2);
    MicQuizWinMesSet(0x1C0037, 1, 2);
    MicQuizWinMesWait();
}

s32 lbl_1_bss_1CC4;
HU3D_MODELID lbl_1_bss_1CB0[10];
MotionPair lbl_1_bss_1C60[10];
HU3D_MODELID lbl_1_bss_1C54[6];
MotionPair lbl_1_bss_1C24[6];
s32 lbl_1_bss_1C18[3];
s16 lbl_1_bss_1C16;
/* fn_1_2F890 consumes the signed-halfword pair at 0x1C12 and 0x1C14. */
s16 lbl_1_bss_1C12[2];
s16 lbl_1_bss_1C0C[3];
s32 lbl_1_bss_1C08;
s32 lbl_1_bss_1C04;
MicquizResponse *lbl_1_bss_1C00;
