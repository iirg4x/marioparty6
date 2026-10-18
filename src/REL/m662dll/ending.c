/* Ending camera sequence and computer-player decisions for Trick or Tree. */
#include "dolphin/math.h"
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
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m662dll/game.h"

extern void fn_1_4EFC(s32, s32);
extern void fn_1_984(void);
extern void fn_1_2670(void);
extern s32 fn_1_48E8(void *, s32);
extern s32 fn_1_57A8(void);
s32 fn_1_A7A4(void);
static void fn_1_ADD8(M662Player *);
extern s32 fn_1_2DA0(f32 *, f32, f32, f32);
extern void fn_1_5008(s16, s16, void *);

/* Finish-phase callback: requests the main music fade and keeps input, ending camera and scene
 * updates running. */
s32 fn_1_A430(void) {
    s16 phaseFinished = 0;

    fn_1_4EFC(1, 100);
    fn_1_984();
    fn_1_A7A4();
    fn_1_2670();
    return phaseFinished;
}

/* Pre-winner callback: updates input and scene transforms and reports when the ending camera
 * sequence is ready. */
s16 fn_1_A474(void) {
    s16 endingReady = 0;

    fn_1_984();
    endingReady = fn_1_A7A4();
    fn_1_2670();
    return endingReady;
}

/* Winner callback: starts winner and loser motions once, restarts the accompanying model motion and
 * keeps the ending scene updated. */
s32 fn_1_A4B0(void)
{
    s16 playerIndex;

    switch (lbl_1_bss_210.endingState) {
    case 0:
        playerIndex = 0;
        while (playerIndex < 2) {
            if (lbl_1_bss_2F4[playerIndex].result >= 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 4;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 5.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
            } else {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 5;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 5.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
            }
            fn_1_48E8(&lbl_1_bss_2F4[playerIndex], 5);
            playerIndex++;
        }
        lbl_1_bss_210.endingState++;
        break;
    case 1:
        if (Hu3DMotionEndCheck(lbl_1_bss_4B4[16].modelId) != 0) {
            Hu3DMotionTimeSet(lbl_1_bss_4B4[16].modelId, 0.0f);
            Hu3DMotionSpeedSet(lbl_1_bss_4B4[16].modelId, 1.0f);
            lbl_1_bss_210.endingState++;
        }
        break;
    }
    fn_1_984();
    fn_1_A7A4();
    fn_1_2670();
    return 0;
}

/* Fade-out callback: keeps the ending camera sequence and scene transforms updated. */
s32 fn_1_A758(void) {
    fn_1_A7A4();
    fn_1_2670();
    return 0;
}

/* Close callback: stops all game audio and returns zero. */
s32 fn_1_A780(void) {
    fn_1_57A8();
    return 0;
}

/* Ending updater: waits, frames the winner, starts the accompanying model motion and repeats timed
 * stereo sound cues. */
s32 fn_1_A7A4(void)
{
    f32 durationFrames = 0.0f;
    s32 sequenceActive = 0;
    s32 unfinishedMoves = 0;
    /* Frame offsets and stereo pans; a negative frame ends the cue list. */
    struct {
        s16 frameNo;
        s16 pan;
    } cueFrames[6] = { { 0, 64 }, { 10, 64 }, { 40, 80 },
                       { 50, 48 }, { 60, 64 }, { -1, -1 } };
    f32 modelMotionFrame[1];

    switch (lbl_1_bss_210.sceneState) {
    case 0: {
        if (lbl_1_bss_210.timer++ > 120) {
            lbl_1_bss_210.timer = 0;
            lbl_1_bss_210.centerY = Center.y;
            lbl_1_bss_210.sceneState++;
        }
        break;
    }
    case 1:
        durationFrames = 60.0f;
        unfinishedMoves = fn_1_2DA0(&Center.x, durationFrames,
                              0.0f,
                              lbl_1_bss_2F4[lbl_1_bss_210.winnerIndex]
                                  .position.x);
        unfinishedMoves += fn_1_2DA0(
            &Center.y, durationFrames, lbl_1_bss_210.centerY,
            200.0f +
                lbl_1_bss_2F4[lbl_1_bss_210.winnerIndex].position.y);
        unfinishedMoves += fn_1_2DA0(&Center.z, durationFrames,
                               -446.0f, -326.0f);
        unfinishedMoves += fn_1_2DA0(&CRot.x, durationFrames,
                               -15.2f, -15.2f);
        unfinishedMoves += fn_1_2DA0(&CRot.y, durationFrames,
                               0.0f, 0.0f);
        unfinishedMoves += fn_1_2DA0(&CRot.z, durationFrames,
                               0.0f, 0.0f);
        unfinishedMoves += fn_1_2DA0(&CZoom, durationFrames,
                               3317.7f, 2200.0f);
        if ((s16)unfinishedMoves == 0) {
            lbl_1_bss_210.sceneState++;
        }
        break;
    case 2:
        Hu3DModelAttrReset(lbl_1_bss_4B4[16].modelId, HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(lbl_1_bss_4B4[16].modelId,
                          0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4B4[16].modelId,
                           1.0f);
        lbl_1_bss_4B4[16].position.x =
            lbl_1_bss_2F4[lbl_1_bss_210.winnerIndex].position.x;
        lbl_1_bss_4B4[16].position.y =
            lbl_1_bss_2F4[lbl_1_bss_210.winnerIndex].position.y;
        lbl_1_bss_4B4[16].position.z = -1650.0f;
        lbl_1_bss_210.timer = 0;
        lbl_1_bss_210.cueIndex = 0;
        lbl_1_bss_210.sceneState++;
        break;
    case 3:
        /* This motion-time sample is unused; the sound cues follow the sequence timer below. */
        modelMotionFrame[0] = Hu3DMotionTimeGet(lbl_1_bss_4B4[16].modelId);
        if (lbl_1_bss_210.timer ==
            cueFrames[lbl_1_bss_210.cueIndex].frameNo) {
            lbl_1_bss_210.panCenter =
                cueFrames[lbl_1_bss_210.cueIndex].pan;
            fn_1_5008(3, 7, 0);
            lbl_1_bss_210.cueIndex++;
            if (cueFrames[lbl_1_bss_210.cueIndex].frameNo < 0) {
                lbl_1_bss_210.cueIndex = 0;
            }
        }
        {
            if (lbl_1_bss_210.timer++ > 104) {
                lbl_1_bss_210.timer = 0;
            }
        }
        sequenceActive = 1;
        break;
    }
    return sequenceActive;
}

/* During setup, chooses computer lateral-move budgets and assigns one representative a budget if
 * both are zero. */
void fn_1_AC88(void)
{
    s16 playerIndex = 0;
    while (playerIndex < 2) {
        if (lbl_1_bss_2F4[playerIndex].playerType != 0) {
            lbl_1_bss_2F4[playerIndex].remainingMoves = frandmod(5);
        }
        playerIndex++;
    }
    /* This fallback can choose either representative, including a human player. */
    if (lbl_1_bss_2F4[0].remainingMoves == 0 &&
        lbl_1_bss_2F4[1].remainingMoves == 0) {
        playerIndex = frandmod(2);
        lbl_1_bss_2F4[playerIndex].remainingMoves = frandmod(3) + 2;
    }
}

/* After input capture, generates controls for each computer player before the round consumes
 * them. */
void fn_1_AD68(void)
{
    s16 playerIndex;
    M662Player *player;
    playerIndex = 0;
    while (playerIndex < 2) {
        player = &lbl_1_bss_2F4[playerIndex];
        if (player->playerType != 0) {
            fn_1_ADD8(player);
        }
        playerIndex++;
    }
}

/* Computer input helper: while choosing a tree, makes a limited number of delayed lateral choices,
 * then presses A. */
static void fn_1_ADD8(M662Player *player)
{
    s32 stickDirection;
    s16 randomStick;

    switch (player->decisionState) {
    case 0:
        if ((player->state == 4) && (player->delay > 35)) {
            if (player->remainingMoves > 0) {
                /* At an occupied outer slot, choose a direction back toward the other slots. */
                if ((player->slot == 3) ||
                    ((player->characterIndex == 1) &&
                     (lbl_1_bss_2F4[0].slot == 3))) {
                    stickDirection = -60;
                } else if ((player->slot == 0) ||
                           ((player->characterIndex == 1) &&
                            (lbl_1_bss_2F4[0].slot == 0))) {
                    stickDirection = 60;
                } else {
                    stickDirection = (s16)frandmod(100);
                    if (stickDirection < 50) {
                        randomStick = 60;
                    } else {
                        randomStick = -60;
                    }
                    stickDirection = randomStick;
                }
                player->input.stickX = stickDirection;
                player->cooldown = frandmod(30) + 30;
                player->decisionState++;
                player->remainingMoves--;
                return;
            }
            player->cooldown = frandmod(30) + 30;
            player->decisionState = 2;
        }
        break;
    case 1:
        if (player->state == 4) {
            if (player->cooldown-- < 0) {
                player->decisionState = 0;
                return;
            }
        }
        break;
    case 2:
        if (player->state == 4) {
            if (player->cooldown-- < 0) {
                player->decisionState = 0;
                player->input.buttonsDown = PAD_BUTTON_A;
            }
        }
        break;
    }
}
