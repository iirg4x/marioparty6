/* Registers and runs the minigame's sequence callbacks. */
#include "REL/m637dll.h"

MGSEQ_PARAM lbl_1_data_0 = {
    0, 1, fn_1_F0, fn_1_334, fn_1_3B8, fn_1_3DC, fn_1_418, fn_1_488, fn_1_50C, fn_1_510, fn_1_514,
};

OMOBJMAN *lbl_1_bss_4;
s32 lbl_1_bss_0;

/* Creates the object manager and starts the Slot Trot sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_4 = omInitObjMan(200, 8192);
    omGameSysInit(lbl_1_bss_4);
    MgSeqCreate(&lbl_1_data_0);
}

/* Sets up the scene camera, lighting, shadows, and initial play state. */
void fn_1_F0(s16 mode, s16 frameNo)
{
    s16 lightId;

    HuVecF shadowPosition = { 0.0f, 3000.0f, 500.0f };
    HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
    HuVecF shadowTarget = { 0.0f, 0.0f, 0.0f };
    HuVecF lightPosition = { 0.0f, 1000.0f, 5000.0f };
    HuVecF lightDirection = { 0.0f, -1.0f, -1.0f };
    GXColor lightColor = { 255, 255, 255, 255 };
    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 30000.0f, 1.2f);
    omAddObjEx(lbl_1_bss_4, 32730, 0U, 0U, -1, omOutView);
    fn_1_1110(0);
    lightId = Hu3DGLightCreateV(&lightPosition, &lightDirection, &lightColor);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 10000.0f);
    Hu3DShadowColSet(0U, 0U, 0U);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    fn_1_548();
    lbl_1_bss_0 = 0;
    MgSeqModeNext();
}

/* During fade-in, advances the scripted reel spin; update 140 starts the play-camera move
 * and lowers the central machine. The sequence manager then advances to the start message. */
void fn_1_334(s16 mode, s16 frameNo)
{
    fn_1_A54();
    lbl_1_bss_0 += 1;
    if ((s32) lbl_1_bss_0 == (s32) (240 - fn_1_1814())) {
        fn_1_1110(1);
    }
    if ((s32) lbl_1_bss_0 >= (s32) (240 - fn_1_1814())) {
        fn_1_1464(1);
    }
}

void fn_1_3B8(s16 mode, s16 frameNo)
{
    fn_1_1464(2);
}

/* Runs the round state machine during main play, then advances to FINISH when the game ends. */
void fn_1_3DC(s16 mode, s16 frameNo)
{
    if (fn_1_65C() != 0) {
        lbl_1_bss_0 = 0;
        MgSeqModeNext();
    }
}

/* Runs the result transition, counting up to the configured duration. */
void fn_1_418(s16 mode, s16 frameNo)
{
    fn_1_B94();
    lbl_1_bss_0 += 1;
    if ((s32) lbl_1_bss_0 <= fn_1_1814()) {
        fn_1_1464(-1);
        return;
    }
    lbl_1_bss_0 = fn_1_1814();
}

/* During PREWIN, starts the winner-camera move only if the finish counter reached 100, then
 * counts down to the winner/draw poses and advances to the winner message; a draw requests no move.
 */
void fn_1_488(s16 mode, s16 frameNo)
{
    if ((s32) lbl_1_bss_0 == fn_1_1814()) {
        lbl_1_bss_0 = fn_1_1110(2);
    }
    if (--lbl_1_bss_0 <= 0) {
        lbl_1_bss_0 = 0;
        fn_1_E4C();
        MgSeqModeNext();
    }
}

void fn_1_50C(s16 mode, s16 frameNo)
{

}

void fn_1_510(s16 mode, s16 frameNo)
{

}

/* Stops minigame audio and models before returning control to the caller. */
void fn_1_514(s16 mode, s16 frameNo)
{
    HuAudAllStop();
    Hu3DModelAllKill();
    Hu3DLightAllKill();
    omOvlReturnEx(1, 1);
}
