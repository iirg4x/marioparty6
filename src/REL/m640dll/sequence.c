/* Registers the M640 sequence modes and dispatches their intro, gameplay, and result callbacks. */
#include "REL/m640/m640.h"
#include "msm_stream.h"




void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_114(s16 mode, s16 frame);
void fn_1_13C(s16 mode, s16 frame);
void fn_1_170(s16 mode, s16 frame);
void fn_1_1B8(s16 mode, s16 frame);
void fn_1_220(s16 mode, s16 frame);
void fn_1_240(s16 mode, s16 frame);
void fn_1_268(s16 mode, s16 frame);
void fn_1_26C(s16 mode, s16 frame);
void fn_1_270(s16 frame);
void fn_1_60C(void);
void fn_1_868(s16 frame);

MGSEQ_PARAM lbl_1_data_0 = {
    300,
    0,
    fn_1_F0,
    fn_1_114,
    fn_1_13C,
    fn_1_170,
    fn_1_1B8,
    fn_1_220,
    fn_1_240,
    fn_1_268,
    fn_1_26C,
};
HUPROCESS *lbl_1_bss_4;
s32 lbl_1_bss_0;

/* Called by the module prolog to create the object manager and register the M640 sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_4 = fn_8003A8BC(256, 8192);
    fn_80069F1C(lbl_1_bss_4);
    fn_80070788(&lbl_1_data_0);
}

/* Entry callback in lbl_1_data_0; initializes the scene before moving to the next sequence mode. */
void fn_1_F0(s16 mode, s16 frame)
{
    fn_1_86C();
    fn_80071A9C();
}

/* Frame callback in lbl_1_data_0; dispatches the intro state machine once per sequence frame. */
void fn_1_114(s16 mode, s16 frame)
{
    fn_1_270(frame);
}

/* Mode callback in lbl_1_data_0; starts the background music on the first frame of its mode. */
void fn_1_13C(s16 mode, s16 frame)
{
    if (frame == 0) {
        HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
    }
}

/* Gameplay callback in lbl_1_data_0; arms both sides' first players on frame zero, then updates
 * their turns each frame. */
void fn_1_170(s16 mode, s16 frame)
{
    if (frame == 0) {
        fn_1_351C(0, 0);
        fn_1_351C(1, 0);
    }
    fn_1_4CD8();
}

/* Result callback in lbl_1_data_0; resets result state and frame, fades the scene's unchanged
 * zero-valued stream on entry, and updates the turn-animation tail. */
void fn_1_1B8(s16 mode, s16 frame)
{
    lbl_1_bss_4E8.state = 0;
    lbl_1_bss_4E8.frame = 0;
    if (frame == 0) {
        fn_1_3090();
        HuAudSStreamFadeOut(lbl_1_bss_4E8.stream, 100);
    }
    fn_1_355C();
}

/* Result callback in lbl_1_data_0; runs the winner and result state machine each frame. */
void fn_1_220(s16 mode, s16 frame)
{
    fn_1_60C();
}

/* Callback in lbl_1_data_0; forwards the mode frame to fn_1_868. */
void fn_1_240(s16 mode, s16 frame)
{
    fn_1_868(frame);
}

/* Empty callback in lbl_1_data_0; it currently performs no game update. */
void fn_1_268(s16 mode, s16 frame)
{
}

/* Empty callback in lbl_1_data_0; it currently performs no game update. */
void fn_1_26C(s16 mode, s16 frame)
{
}

/* Runs the intro state machine, demonstrating piece drops before transitioning to the split-camera
 * turn sequence. */
void fn_1_270(s16 frame)
{
    OM_CAMERA_VIEW cameraView;
    s16 pieceDropFrameByPart[] = { 40, 80, 120, 160 };
    u16 cameraBySide[] = { 1, 2 };
    int side, piecePart;

    if (frame == 30) {
        lbl_1_bss_0 = HuAudFXPlay(MSM_SE_MG640_INTRO_DEMO_LOOP);
    }
    switch (lbl_1_bss_4E8.state) {
    case 0:
        fn_80071B08();
        lbl_1_bss_4E8.state++;
        break;
    case 1:
        if (lbl_1_bss_4E8.frame++ >= 200) {
            lbl_1_bss_4E8.frame = 0;
            lbl_1_bss_4E8.state++;
        }
        for (side = 0; side < 2; side++) {
            for (piecePart = 0; piecePart < 4; piecePart++) {
                if (lbl_1_bss_4E8.frame == pieceDropFrameByPart[piecePart]) {
                    fn_1_519C(side, piecePart);
                    cameraView.center.x = side * 1300;
                    cameraView.center.y = 150.0f + piecePart * 10;
                    cameraView.center.z = 0.0f;
                    cameraView.rot.x = (-5.0f - 2.0f * side) + 0.5f * piecePart;
                    cameraView.rot.y = 0.0f;
                    cameraView.rot.z = 0.0f;
                    cameraView.zoom = 1400.0f;
                    fn_8003E8E4(cameraBySide[side], &cameraView, 40);
                }
            }
        }
        break;
    case 2:
        if (fn_1_112C(lbl_1_bss_4E8.frame++)) {
            HuAudFXPlay(MSM_SE_MG640_INTRO_DEMO_DONE);
            lbl_1_bss_4E8.state++;
        }
        break;
    case 3:
        if (fn_1_BEC()) {
            HuAudFXFadeOut(lbl_1_bss_0, 500);
            lbl_1_bss_4E8.state++;
        }
        break;
    case 4:
        if (fn_1_EC8()) {
            lbl_1_bss_4E8.state++;
        }
        break;
    case 5:
        fn_80071A9C();
        break;
    }
}

/* Runs the result sequence, announces the winner, awards the side bonus when allowed, and advances
 * the celebration. */
void fn_1_60C(void)
{
    int playerIndex;
    int firstSidePlayerNo, secondSidePlayerNo;
    switch (lbl_1_bss_4E8.state) {
    case 0:
        fn_80071B08();
        lbl_1_bss_4E8.state++;
        break;
    case 1:
        if (lbl_1_bss_4E8.finishCount == 0 || lbl_1_bss_4E8.finishCount == 2) {
            fn_80071C18(-1, -1, -1, -1);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_1_26B8(playerIndex, 3);
            }
            fn_80071A9C();
            return;
        }
        fn_80071C18(lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->charNo,
                      lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->charNo, -1, -1);
        firstSidePlayerNo = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->playerNo;
        if (!fn_8006A124(65551)) {
            lbl_80265750[firstSidePlayerNo].mgCoinBonus = 10;
        }
        secondSidePlayerNo = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->playerNo;
        if (!fn_8006A124(65551)) {
            lbl_80265750[secondSidePlayerNo].mgCoinBonus = 10;
        }
        lbl_1_bss_4E8.frame = 0;
        lbl_1_bss_4E8.state++;
        break;
    case 2:
        if (fn_1_1C48(lbl_1_bss_4E8.frame++)) {
            lbl_1_bss_4E8.state++;
        }
        break;
    case 3:
        fn_80071A9C();
        break;
    }
}

/* This frame callback currently performs no game update. */
void fn_1_868(s16 frame)
{
}
