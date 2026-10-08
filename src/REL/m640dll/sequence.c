/* Registers the M640 sequence modes and dispatches their intro, gameplay, and result callbacks. */
#include "REL/m640/m640.h"
#define M640_SEQUENCE_BGM 83
#define M640_INTRO_TRACKED_SFX 1919
#define M640_INTRO_CAMERA_SPLIT_SFX 1908

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
void fn_1_86C(void);
void fn_1_8D4(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
s16 fn_1_112C(s16 frame);
void fn_1_15C8(void);
s32 fn_1_1C48(s32 frame);
void fn_1_26B8(s16 player, s16 motion);
void fn_1_2820(void);
s16 fn_1_2FF8(void);
void fn_1_3090(void);
s16 fn_1_3274(M640Player *player, HuVecF *spinnerRotation);
s16 fn_1_33D0(float rotationDegrees);
void fn_1_345C(s16 side, s16 slot, s16 previewIndex);
void fn_1_351C(s16 side, s16 slot);
void fn_1_355C(void);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_4C90(s16 side);
void fn_1_4CD8(void);
void fn_1_4DDC(s16 side, s16 part);
void fn_1_4FFC(s16 side, s16 part);
void fn_1_519C(s16 side, s16 part);
void fn_1_52D0(OMOBJ *obj);
void fn_1_5A28(void);
void fn_1_5DF0(void);
void fn_1_60AC(OMOBJ *obj);
void fn_1_623C(void);

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
    lbl_1_bss_4 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_4);
    MgSeqCreate(&lbl_1_data_0);
}

/* Entry callback in lbl_1_data_0; initializes the scene before moving to the next sequence mode. */
void fn_1_F0(s16 mode, s16 frame)
{
    fn_1_86C();
    MgSeqModeNext();
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
        HuAudBGMPlay(M640_SEQUENCE_BGM);
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
        lbl_1_bss_0 = HuAudFXPlay(M640_INTRO_TRACKED_SFX);
    }
    switch (lbl_1_bss_4E8.state) {
    case 0:
        MgSeqModeChangeOff();
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
                    omCameraViewMoveSimpleMulti(cameraBySide[side], &cameraView, 40);
                }
            }
        }
        break;
    case 2:
        if (fn_1_112C(lbl_1_bss_4E8.frame++)) {
            HuAudFXPlay(M640_INTRO_CAMERA_SPLIT_SFX);
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
        MgSeqModeNext();
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
        MgSeqModeChangeOff();
        lbl_1_bss_4E8.state++;
        break;
    case 1:
        if (lbl_1_bss_4E8.finishCount == 0 || lbl_1_bss_4E8.finishCount == 2) {
            MgSeqWinnerSet(-1, -1, -1, -1);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_1_26B8(playerIndex, 3);
            }
            MgSeqModeNext();
            return;
        }
        MgSeqWinnerSet(lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->charNo,
                      lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->charNo, -1, -1);
        firstSidePlayerNo = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->playerNo;
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[firstSidePlayerNo].mgCoinBonus = 10;
        }
        secondSidePlayerNo = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->playerNo;
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[secondSidePlayerNo].mgCoinBonus = 10;
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
        MgSeqModeNext();
        break;
    }
}

/* This frame callback currently performs no game update. */
void fn_1_868(s16 frame)
{
}
