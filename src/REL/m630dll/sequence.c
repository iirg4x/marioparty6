/* Sets up the minigame camera, runs its frame callbacks, and cleans up on exit. */
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "REL/m630dll.h"

void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_3BC(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_3FC(s16 mode, s16 frameNo);
void fn_1_428(s16 mode, s16 frameNo);
void fn_1_448(s16 mode, s16 frameNo);
void fn_1_474(s16 mode, s16 frameNo);
void fn_1_494(s16 mode, s16 frameNo);
void fn_1_4B4(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    30, 2, fn_1_F0, fn_1_3BC, fn_1_3DC, fn_1_3FC, fn_1_428,
    fn_1_448, fn_1_474, fn_1_494, fn_1_4B4
};
HUPROCESS *lbl_1_bss_0;

/* The module prolog calls this after constructors to create the object manager and start the
 * sequence. */

void fn_1_A0(void)
{
    lbl_1_bss_0 = fn_8003A8BC(200, 8192);
    fn_80069F1C(lbl_1_bss_0);
    fn_80070788(&lbl_1_data_0);
}

/* Opening callback: creates camera 1, sets the course view and lighting, then initializes play. */

void fn_1_F0(s16 mode, s16 frameNo)
{
    OM_CAMERA_VIEW view;
    HuVecF lightPos = { 0.0f, 1000.0f, 5000.0f };
    HuVecF lightDir = { 0.0f, -1.0f, -1.0f };
    s16 light;
    GXColor color = { 255, 255, 255, 255 };
    HuVecF shadowPos = { 0.0f, 5000.0f, -2500.0f };
    HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
    HuVecF shadowTarget = { 0.0f, 0.0f, -2800.0f };

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 10.0f, 20.0f, 30000.0f, 1.2f);
    fn_8003AB38(lbl_1_bss_0, 32730, 0, 0, -1, fn_8003D4FC);
    view.center = (HuVecF){ 0.0f, 349.7182f, -451.8600f };
    view.rot = (HuVecF){ -22.5770f, 0.0f, 0.0f };
    view.zoom = 8000.0f;
    fn_8003E5A4(&view);
    light = fn_800286D0(&lightPos, &lightDir, &color);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    Hu3DShadowCreate(-60.0f, 20.0f, 10000.0f);
    fn_8002AAB4(0, 0, 0);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    fn_1_4E8();
    fn_80071A9C();
}

/* Fade-in callback: updates player and moving-row state. */

void fn_1_3BC(s16 mode, s16 frameNo)
{
    fn_1_5D0();
}

/* Start callback: resets result timing and starts the gameplay music if needed. */

void fn_1_3DC(s16 mode, s16 frameNo)
{
    fn_1_AF0();
}

/* Main-play callback advances to finish when player 0 is hit by a moving object. */

void fn_1_3FC(s16 mode, s16 frameNo)
{
    if (fn_1_594() != 0) {
        fn_80071A9C();
    }
}

/* Finish callback records the result and coin bonuses, slides player 0 on a loss, and fades out
 * moving objects. */

void fn_1_428(s16 mode, s16 frameNo)
{
    fn_1_B50();
}

/* Pre-winner callback advances to the winner stage after all three opponent rows reach their end
 * marks. */

void fn_1_448(s16 mode, s16 frameNo)
{
    if (fn_1_1670() != 0) {
        fn_80071A9C();
    }
}

/* Pre-winner callback: advances alternate motions on the decorative models. */

void fn_1_474(s16 mode, s16 frameNo)
{
    fn_1_1E70();
}

/* Fade-out callback: advances alternate motions on the decorative models. */

void fn_1_494(s16 mode, s16 frameNo)
{
    fn_1_1E90();
}

/* Stops this minigame's audio and models before returning to the caller. */

void fn_1_4B4(s16 mode, s16 frameNo)
{
    HuAudAllStop();
    fn_80026648();
    fn_80028D70();
    fn_8003A5E4(1, 1);
}
