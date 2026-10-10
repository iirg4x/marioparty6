/* Runs the Odd Card Out intro, round, and result sequence callbacks. */
#include "REL/m602Dll.h"

#define MSM_SE_M602_ROUND_START 1571
#define MSM_SE_M602_CARD_REVEAL 1578
#define MSM_SE_M602_RESULT_CARD 1572
#define M602_CARD_STOPPED_FRONT_FLAG (1 << 1)
#define M602_CARD_STOPPED_MASK 0x3

/* static */
MGSEQ_PARAM lbl_1_data_0 = {
    0, MGSEQ_TIMER_RIGHT, fn_1_10C, fn_1_210, fn_1_278, fn_1_2AC, fn_1_7B8, fn_1_8F0,
    fn_1_A5C, NULL, fn_1_AB8,
};

M602CameraParams lbl_1_data_28 = {
    8,
    32.0f,
    10.0f,
    5000.0f,
    1.2f,
    0.0f,
    0.0f,
    640.0f,
    480.0f,
    0.0f,
    1.0f,
    { 0.0f, 0.0f, 1000.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
};

M602CameraPose lbl_1_data_78 = { { 0.0f, 394.0f, 1990.0f },
                                 { 0.0f, 1.0f, 0.0f },
                                 { 0.0f, 394.0f, 0.0f } };

M602CameraPose lbl_1_data_9C[2] = {
    { { -50.0f, 200.0f, 980.0f }, { 0.0f, 1.0f, 0.0f }, { -500.0f, 200.0f, 0.0f } },
    { { 50.0f, 200.0f, 980.0f }, { 0.0f, 1.0f, 0.0f }, { 500.0f, 200.0f, 0.0f } },
};

static const Point3d lbl_1_rodata_10 = { 0.0f, 5000.0f, 1.0f };

/* const */
static const Point3d lbl_1_rodata_1C = { 0.0f, 1.0f, 0.0f };

/* const */
static const Point3d lbl_1_rodata_28 = { 0.0f, 0.0f, 0.0f };

/* Round phase: 0 shuffle, 1 start, 2 choice, 3 reveal, 4 reaction, 5 next-round wait. */
s16 lbl_1_bss_10;

/* Playing minigame music handle, faded out when the rounds end. */
s32 lbl_1_bss_C;

/* Object-manager process owning this minigame's objects. */
HUPROCESS *lbl_1_bss_8;

/* Exclusive frame limit for calling the winner-stage update during the pre-winner phase. */
s16 lbl_1_bss_4;

/* Remaining correct-choice effect frames before the next round. */
s16 lbl_1_bss_2;

/* Frame counter reset at each round-phase transition. */
s16 lbl_1_bss_0;

/* Module initialization registers the callbacks and lets the intro and pre-winner scenes
 * choose when their sequence phases finish. */
void fn_1_A0(void)
{
    lbl_1_bss_8 = omInitObjMan(300, 4096);
    omGameSysInit(lbl_1_bss_8);
    MgSeqCreatePrio(&lbl_1_data_0, 2000);
    MgSeqModeDelaySet(MGSEQ_MODE_FADEIN, -1);
    MgSeqModeDelaySet(MGSEQ_MODE_PREWIN, -1);
}

/* Intro callback: initialize the shadow and scene, then start the minigame music. */
void fn_1_10C(s16 mode, s16 frameNo)
{
    Point3d shadowPosition;
    Point3d shadowUp;
    Point3d shadowTarget;

    lbl_1_bss_10 = 0;
    fn_1_AD8();
    shadowPosition = lbl_1_rodata_10;
    shadowUp = lbl_1_rodata_1C;
    shadowTarget = lbl_1_rodata_28;
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    fn_1_411C();
    fn_1_179C();
    fn_1_5254();
    fn_1_9ED8();
    fn_1_A074();
    lbl_1_bss_C = HuAudBGMPlay(MSM_STREAM_MGMUS_25);
    MgSeqModeNext();
}

/* Intro update callback: advance the camera and player animations until the scene is ready. */
void fn_1_210(s16 mode, s16 frameNo)
{
    s8 playerAnimationResult;
    u8 introFinished;

    introFinished = fn_1_D54(frameNo);
    playerAnimationResult = fn_1_5C7C(frameNo);
    /* The player animation result is intentionally ignored by this callback. */
    fn_1_A648(frameNo);
    fn_1_4AAC();
    if (introFinished != 0) {
        fn_1_6078();
        MgSeqModeNext();
    }
}

/* Start-announcement callback: arm the first round and update the player interface. */
void fn_1_278(s16 mode, s16 frameNo)
{
    lbl_1_bss_10 = 1;
    fn_1_A81C();
    fn_1_4AAC();
}

/* Round callback: collect card choices, reveal the odd card, and run the result sequence. */
void fn_1_2AC(s16 mode, s16 frameNo)
{
    s16 oddColumnIndex;
    s16 playerChoiceOrScore;
    s16 columnOrPlayerIndex;

    if (lbl_1_bss_10 == 1) {
        if (frameNo == 0) {
            fn_1_A78C();
        }
        lbl_1_bss_10 = 0;
        lbl_1_bss_0 = 0;
        fn_1_2998();
        fn_1_22DC();
        fn_1_61E0();
        HuAudFXPlay(MSM_SE_M602_ROUND_START);
    } else if (lbl_1_bss_10 == 0) {
        fn_1_22DC();
        fn_1_61E0();
        if ((fn_1_2E3C(0) == M602_CARD_STOPPED_FRONT_FLAG) &&
            (fn_1_2E3C(1) == M602_CARD_STOPPED_FRONT_FLAG) &&
            (fn_1_2E3C(2) == M602_CARD_STOPPED_FRONT_FLAG)) {
            lbl_1_bss_10 = 1;
            fn_1_9F34();
            lbl_1_bss_10 = 2;
            lbl_1_bss_0 = 0;
            HuAudFXPlay(MSM_SE_M602_CARD_REVEAL);
        }
    } else if (lbl_1_bss_10 == 2) {
        fn_1_22DC();
        fn_1_647C();
        if ((fn_1_9FD8() != 0) || (fn_1_4F90() != 0)) {
            lbl_1_bss_10 = 3;
            lbl_1_bss_0 = 0;
        }
    } else if (lbl_1_bss_10 == 3) {
        if (lbl_1_bss_0 == 2) {
            fn_1_A034();
        }
        if (lbl_1_bss_0 == 60) {
            columnOrPlayerIndex = fn_1_2B28();
            if (columnOrPlayerIndex == 0) {
                HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_RESULT_CARD), 32);
            } else if (columnOrPlayerIndex == 1) {
                HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_RESULT_CARD), 64);
            } else {
                HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_RESULT_CARD), 96);
            }
        }
        if (lbl_1_bss_0 > 150) {
            lbl_1_bss_10 = 4;
            lbl_1_bss_0 = 0;
        }
        fn_1_22DC();
        fn_1_6DB8();
    } else if (lbl_1_bss_10 == 4) {
        if (((s32) (fn_1_2E3C(0) & M602_CARD_STOPPED_MASK) != 0) &&
            ((s32) (fn_1_2E3C(1) & M602_CARD_STOPPED_MASK) != 0) &&
            ((s32) (fn_1_2E3C(2) & M602_CARD_STOPPED_MASK) != 0)) {
            if (lbl_1_bss_0 == 1) {
                fn_1_6F00();
                oddColumnIndex = fn_1_2D5C();
                columnOrPlayerIndex = fn_1_5048();
                if (columnOrPlayerIndex >= 0) {
                    playerChoiceOrScore = fn_1_502C(columnOrPlayerIndex);
                    if (oddColumnIndex == playerChoiceOrScore) {
                        fn_1_8D90(columnOrPlayerIndex, 1);
                        playerChoiceOrScore = fn_1_50D4(columnOrPlayerIndex);
                        fn_1_A914(columnOrPlayerIndex, playerChoiceOrScore);
                        lbl_1_bss_2 = fn_1_4A80();
                    } else {
                        fn_1_8D90(columnOrPlayerIndex, 0);
                    }
                } else {
                    fn_1_8D90(-1, 0);
                }
            }
            fn_1_22DC();
            columnOrPlayerIndex = fn_1_6F14();
            if ((columnOrPlayerIndex < 0) && (lbl_1_bss_2 <= 0)) {
                fn_1_4A98();
                lbl_1_bss_10 = 5;
                lbl_1_bss_0 = 0;
            }
        } else {
            fn_1_22DC();
            fn_1_6DB8();
            lbl_1_bss_0 = 0;
        }
    } else if (lbl_1_bss_10 == 5) {
        if ((fn_1_178C() >= 10) || (fn_1_5170() != 0)) {
            if (lbl_1_bss_0 >= 30) {
                MgSeqModeNext();
                HuAudSStreamFadeOut(lbl_1_bss_C, 100);
                lbl_1_bss_0 = 0;
            }
            if (fn_1_51D4() >= 0) {
                fn_1_3908();
            }
        } else if (lbl_1_bss_0 >= 60) {
            lbl_1_bss_10 = 1;
            lbl_1_bss_0 = 0;
        }
    }
    fn_1_4AAC();
    fn_1_4CD0(frameNo);
    fn_1_A81C();
    fn_1_1428();
    fn_1_1778(frameNo);
    lbl_1_bss_0 += 1;
    lbl_1_bss_2 -= 1;
    if (lbl_1_bss_2 < 0) {
        lbl_1_bss_2 = 0;
    }
}

/* Results callback: award ten coins outside practice mode and register the winner's character. */
void fn_1_7B8(s16 mode, s16 frameNo)
{
    s16 winners[4];
    s16 winningPlayer;
    s16 playerIndex;

    winningPlayer = fn_1_51D4();
    if (frameNo == 1) {
        if (winningPlayer >= 0) {
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[winningPlayer].mgCoinBonus = 10;
            }
            playerIndex = 0;
            while (playerIndex < 4) {
                (&winners[0])[playerIndex] = -1;
                playerIndex += 1;
            }
            /* Keep all player slots empty except the winner's character slot. */
            (&winners[0])[winningPlayer] = fn_1_5238(winningPlayer);
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        } else {
            playerIndex = 0;
            while (playerIndex < 4) {
                winners[playerIndex] = -1;
                playerIndex += 1;
            }
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        }
    }
    if (winningPlayer >= 0) {
        fn_1_3908();
    }
    fn_1_22DC();
    fn_1_A81C();
    fn_1_4AAC();
}

/* Pre-winner callback: clear the players' card effects and prepare the winner stage or draw. */
void fn_1_8F0(s16 mode, s16 frameNo)
{
    s16 winningPlayer;
    s16 animationResult;

    winningPlayer = fn_1_51D4();
    if (frameNo == 0) {
        lbl_1_bss_4 = 1;
    }
    fn_1_A81C();
    if (winningPlayer >= 0) {
        if (frameNo < 60) {
            /* This update has side effects; its return is replaced by the reset below. */
            animationResult = fn_1_8E64(frameNo);
        }
        animationResult = 0;
        if (frameNo < lbl_1_bss_4) {
            animationResult = fn_1_34A4(frameNo);
            if (animationResult > 0) {
                lbl_1_bss_4 = frameNo + 1;
            } else {
                lbl_1_bss_4 = frameNo + 2;
            }
        }
        if (animationResult == -1) {
            fn_1_AB68(1);
            fn_1_9480(1);
            fn_1_48B4();
            fn_1_90B4();
            MgSeqModeNext();
        }
    } else if (frameNo == 0) {
        fn_1_2BB0(135, 3);
    } else if (frameNo < 135) {
        fn_1_22DC();
    } else {
        fn_1_22DC();
        animationResult = fn_1_8E64((s16) (frameNo - 135));
        if (animationResult != 0) {
            fn_1_90B4();
            MgSeqModeNext();
        }
    }
    fn_1_4AAC();
}

/* Winner-display callback: advance the final stage motion and blinking winner displays,
 * and pause character motions that have finished. */
void fn_1_A5C(s16 mode, s16 frameNo)
{
    s16 winningPlayer;

    winningPlayer = fn_1_51D4();
    fn_1_A81C();
    fn_1_AB68(0);
    fn_1_9480(0);
    if (winningPlayer >= 0) {
        fn_1_34A4(1);
    }
    fn_1_9378();
    fn_1_4AAC();
}

/* Close and system-exit callbacks release the texture used to display the card cameras. */
void fn_1_AB8(s16 mode, s16 frameNo)
{
    fn_1_22A0();
}

/* Intro initialization sets the viewport, starts the camera at the left-side pose, and clears
 * any camera-shake countdown. */
void fn_1_AD8(void)
{
    lbl_1_data_28.pos = lbl_1_data_78.pos;
    lbl_1_data_28.up = lbl_1_data_78.up;
    lbl_1_data_28.target = lbl_1_data_78.target;
    Hu3DCameraCreate(lbl_1_data_28.cameraBit);
    Hu3DCameraViewportSet(lbl_1_data_28.cameraBit, lbl_1_data_28.viewportX, lbl_1_data_28.viewportY,
                          lbl_1_data_28.viewportW, lbl_1_data_28.viewportH, lbl_1_data_28.minZ,
                          lbl_1_data_28.maxZ);
    Hu3DCameraPerspectiveSet(lbl_1_data_28.cameraBit, lbl_1_data_28.fov, lbl_1_data_28.nearPlane,
                             lbl_1_data_28.farPlane, lbl_1_data_28.aspect);
    Hu3DCameraScissorSet(lbl_1_data_28.cameraBit, (u32) lbl_1_data_28.viewportX,
                         (u32) lbl_1_data_28.viewportY, (u32) lbl_1_data_28.viewportW,
                         (u32) lbl_1_data_28.viewportH);
    lbl_1_data_28.pos = lbl_1_data_9C[0].pos;
    lbl_1_data_28.up = lbl_1_data_9C[0].up;
    lbl_1_data_28.target = lbl_1_data_9C[0].target;
    Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                      &lbl_1_data_28.target);
    lbl_1_bss_18 = 0;
}
