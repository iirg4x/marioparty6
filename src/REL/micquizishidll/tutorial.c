/* Tutorial: its scene, the pointer sprite and the tutorial messages. */
#include "REL/micquizishidll/module_base.h"
#include "REL/micquizishidll/process_manager.h"
#include "REL/micquizishidll/micquizishidll_sprite_position_views_v1.h"
#include "REL/micquizishidll/micquizishidll_message_setup_views_v2.h"
#include "REL/micquizishidll/micquizishidll_memory_button_views_v1.h"
#include "REL/micquizishidll/micquizishidll_menu_animation_views_v1.h"
#include "game/hsfformat.h"
#include "messdir_enum.h"
#include "REL/micquizishidll.h"
#include "REL/micquizishidll/micquizishidll_menu_callback_views_v1.h"

void MicQuizMicHudMotionSet(s32);
void fn_1_2F368(void);
void fn_1_2F57C(void);
void fn_1_2F7F8(s32);
void MicQuizPortraitShow(void);
void fn_1_2F3E4(void);

extern s32 lbl_1_bss_110;
extern s32 lbl_1_bss_1BE8;
extern s16 lbl_1_bss_1BF4;
extern ANIMDATA *lbl_1_bss_1BF8;
extern s16 lbl_1_bss_9C[2];
extern s16 lbl_1_bss_A0;
extern s16 lbl_1_bss_A2;
extern s16 lbl_1_bss_A4;
extern OMOBJ *lbl_1_bss_A8;
extern Point3d lbl_1_data_232C;
extern Point3d lbl_1_data_2338;
extern Point3d lbl_1_data_2344;
extern OM_CAMERA_VIEW lbl_1_data_2350;
extern Point3d lbl_1_data_470;
extern Point3d lbl_1_data_47C;
extern u8 lbl_1_data_488[4];

/* Sets up the tutorial: camera, lights and shadow, the models and the sprites. */
void fn_1_2EDB4(void) {
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0U, 0U, 0x280U, 0x1E0U);
    fn_8003E488(1, &lbl_1_data_2350);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0U, 0U, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = (s32)Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U, 0U);
    Hu3DGLightPosAimSetV((s16)lbl_1_bss_110, &lbl_1_data_470, &lbl_1_data_47C);
    Hu3DGLightStaticSet((s16)lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_110);
    Hu3DGLightColorSet((s16)lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10U, 0x10U, 0x10U);
    Hu3DShadowPosSet(&lbl_1_data_232C, &lbl_1_data_2338, &lbl_1_data_2344);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 34), 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 129), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 43), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 44), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 45), 0x10000000, 2));
    fn_80026760(lbl_1_bss_9C[0], -500.0f, 0.0f, 0.0f);
    fn_80026760(lbl_1_bss_9C[1], 500.0f, 0.0f, 0.0f);
    lbl_1_bss_1BF8 = HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 50), 0x10000000, 2));
    lbl_1_bss_1BF4 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_1BF4, 0, HuSprCreate(lbl_1_bss_1BF8, 0x30, 2));
    HuSprAttrSet(lbl_1_bss_1BF4, 0, 4);
    fn_8000DAC0(fn_1_2F3E4, 0x64U, 0x1000U, 0, *lbl_1_bss_114);
    lbl_1_bss_1BEC.x = 0.0f;
    lbl_1_bss_1BEC.y = 0.0f;
    lbl_1_bss_1BE8 = 2;
    MicQuizCountdownInit();
    MicQuizCurrentPlayerGet();
    MicQuizPortraitShow();
    MicQuizMarkerHide();
    MicQuizMicHudStateSet(0);
    lbl_1_bss_FC = 0;
    fn_8000DAC0(fn_1_2F3E4, 0x64U, 0x1000U, 0, *lbl_1_bss_114);
    fn_8000DAC0(fn_1_2F83C, 0x64U, 0x1000U, 0, *lbl_1_bss_114);
    MicQuizEffectParentSet();
}

/* Runs the tutorial's mode steps: after the opening wipe starts the tutorial, and at the end
   leaves it. */
void fn_1_2F2B8(void) {
    OMOVLHIS *history;

    history = omOvlHisGet(-1);
    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        fn_8000DAC0(fn_1_2F368, 0x64, 0x1000, 0, lbl_1_bss_114[0]);
        lbl_1_bss_FC = 1;
        break;
    case 2:
        if (WipeCheck() == 0) {
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Plays the tutorial: the mic check message, the tutorial messages, then the closing wipe, and
   ends this process. */
void fn_1_2F368(void) {
    MicQuizMicHudMotionSet(0);
    if (HuMCSelWinCheck() != 0) {
        MicQuizInstructionSet(8);
    } else {
        MicQuizInstructionSet(7);
    }
    fn_1_2F57C();
    HuSprAttrSet(lbl_1_bss_1BF4, 0, 4);
    lbl_1_bss_FC = 2;
    WipeCreate(2, 0, 0x14);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Bobs the tutorial's pointer sprite each frame, with the image and direction of the current
   step. */
void fn_1_2F3E4(void) {
    f32 sineOffset;
    f32 angle;

    angle = 0.0f;
    for (;;) {
        HuPrcVSleep();
        sineOffset = (f32)(5.0 *
                           sin((M_PI * (f64)angle) / 180.0));
        if (lbl_1_bss_1BE8 == 0) {
            HuSprBankSet(lbl_1_bss_1BF4, 0, 0);
            HuSprPosSet(lbl_1_bss_1BF4, 0, lbl_1_bss_1BEC.x,
                        lbl_1_bss_1BEC.y - sineOffset);
        } else if (lbl_1_bss_1BE8 == 1) {
            HuSprBankSet(lbl_1_bss_1BF4, 0, 1);
            HuSprPosSet(lbl_1_bss_1BF4, 0, lbl_1_bss_1BEC.x,
                        lbl_1_bss_1BEC.y + sineOffset);
        } else if (lbl_1_bss_1BE8 == 2) {
            HuSprBankSet(lbl_1_bss_1BF4, 0, 2);
            HuSprPosSet(lbl_1_bss_1BF4, 0, lbl_1_bss_1BEC.x + sineOffset,
                        lbl_1_bss_1BEC.y - sineOffset);
        }
        angle += 18.0f;
    }
}

/* Shows the tutorial's messages one after another, waiting for each. */
void fn_1_2F57C(void) {
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 30), 1, 0);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 0;
    fn_1_2F7F8(0);
    HuSprAttrReset(lbl_1_bss_1BF4, 0, HUSPR_ATTR_DISPOFF);
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 31), 1, 1);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 0;
    fn_1_2F7F8(1);
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 32), 1, 0);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 0;
    fn_1_2F7F8(2);
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 33), 1, 1);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 0;
    fn_1_2F7F8(3);
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 35), 1, 0);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 1;
    fn_1_2F7F8(4);
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 40), 1, 2);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 1;
    fn_1_2F7F8(4);
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 41), 1, 2);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 1;
    MicQuizMicHudStateSet(1);
    fn_1_2F7F8(4);
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 42), 1, 2);
    MicQuizWinMesWait();
    lbl_1_bss_1BE8 = 1;
    fn_1_2F7F8(5);
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 43), 1, 2);
    MicQuizWinMesWait();
    MicQuizMicHudStateSet(0);
    HuPrcVSleep();
    MicQuizMarkerShow();
    MicQuizMicHudStateSet(2);
    MicQuizPortraitHide(0);
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 44), 1, 2);
    MicQuizWinMesWait();
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 45), 1, 2);
    MicQuizWinMesWait();
}

void fn_1_2F7F8(s32 positionIndex) {
    lbl_1_bss_1BEC.x = lbl_1_data_22FC[positionIndex].x;
    lbl_1_bss_1BEC.y = lbl_1_data_22FC[positionIndex].y;
}

u8 lbl_1_data_22E0[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
MicQuizSpritePosition lbl_1_data_22FC[6] = { { 279.0f, 116.0f }, { 90.0f, 113.0f },
        { 393.0f, 110.0f }, { 514.0f, 145.0f }, { 509.0f, 306.0f }, { 478.0f, 315.0f } };
Point3d lbl_1_data_232C = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_2338 = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_2344 = { 0.0f, 0.0f, -400.0f };

extern u8 lbl_1_bss_1BF6[0x2];

extern u8 lbl_1_bss_1BFC[0x4];

extern HuVec2f lbl_1_bss_1BEC;

u8 lbl_1_bss_1BFC[0x4];
ANIMDATA *lbl_1_bss_1BF8;
u8 lbl_1_bss_1BF6[0x2];
s16 lbl_1_bss_1BF4;
HuVec2f lbl_1_bss_1BEC;
s32 lbl_1_bss_1BE8;

OM_CAMERA_VIEW lbl_1_data_2350 = { { 0.0f, 245.0f, 0.0f }, { -6.3f, 0.0f, 0.0f }, 6195.0f };
