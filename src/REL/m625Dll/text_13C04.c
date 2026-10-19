/* Minigame sequence callbacks for setup, play, results, and exit. */
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

s32 lbl_1_bss_818;
s16 lbl_1_bss_816;
s16 lbl_1_bss_814;
HUPROCESS *lbl_1_bss_810;
extern s16 GwMgNightF;
extern void fn_1_1B0(void);
extern void fn_1_15C8(void);
extern void fn_1_2D48(void);
extern void fn_1_1284C(void);
extern s32 fn_1_41C(s32);
extern s32 fn_1_3E8C(s32);
extern void fn_1_CB0(void);
extern void fn_1_4E60(void);
extern void fn_1_EF40(void);
extern void fn_1_12C80(void);
extern void fn_1_1BAC(void);
extern void fn_1_4E80(void);
extern s32 fn_1_EF24(s16);
extern void fn_1_12F98(s16, s32);
extern void fn_1_12EA8(void);
extern s16 fn_1_C19C(void);
extern void fn_1_C270(s16);
extern u8 fn_1_CB4(s16);
extern u8 fn_1_C438(void);

void fn_1_DE68(void);

/* Called by sequence initialization to set up the stage, shadows, water, and camera. */
void fn_1_13C04(s16 mode, s16 frameNo)
{
    s16 nightFlag;

    lbl_1_bss_816 = 0;
    fn_1_1B0();
    {
    Point3d shadowPosition = { 0.0f, 5000.0f, 1.0f };
    Point3d shadowUp = { 0.0f, 1.0f, 0.0f };
    Point3d shadowTarget = { 0.0f, 0.0f, 0.0f };
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    fn_1_15C8();
    fn_1_2D48();
    fn_1_1284C();
    nightFlag = GwMgNightF;
    if (nightFlag == 0) {
        Hu3DBGColorSet(128, 255, 128);
    } else {
        Hu3DBGColorSet(64, 128, 64);
    }
    MgSeqModeNext();
    }
}

/* Called during fade-in to advance the camera and player entrance animations. */
void fn_1_13D30(s32 mode, s32 frameNo)
{
    s32 cameraReady;
    s32 playersReady;

    lbl_1_bss_816 = 1;
    cameraReady = fn_1_41C(frameNo);
    playersReady = fn_1_3E8C(frameNo);
    if ((u8)cameraReady != 0 && (u8)playersReady != 0) {
        MgSeqModeNext();
    }
}

/* Called on the start phase to launch the music and begin the player intro. */
void fn_1_13DAC(s32 mode, s16 frameNo)
{
    lbl_1_bss_816 = 2;
    lbl_1_bss_814 = 1;
    if (frameNo == 0) {
        lbl_1_bss_818 = HuAudBGMPlay(MSM_STREAM_CAPSULE_WANWAN);
    }
    fn_1_CB0();
    fn_1_4E60();
}

/* Called at the end of play to switch to result presentation and stop gameplay input. */
void fn_1_13E14(void) {
    lbl_1_bss_816 = 3;
    lbl_1_bss_814 = 1;
    fn_1_EF40();
    fn_1_12C80();
}

/* Called each play frame to update the game and prepare the winning player sequences. */
void fn_1_13E58(void) {
    s16 playerIndex;
    s32 playerSequence;

    lbl_1_bss_816 = 3;
    fn_1_CB0();
    fn_1_1BAC();
    fn_1_4E80();
    playerIndex = 0;
    while (playerIndex < 4) {
        playerSequence = fn_1_EF24(playerIndex);
        fn_1_12F98(playerIndex, playerSequence);
        playerIndex++;
    }
    fn_1_12EA8();
    if (fn_1_C19C() == 0) {
        lbl_1_bss_814 = 0;
        MgSeqModeNext();
    }
}

/* Called during the winner phase to select the winner's pose and fade the music. */
void fn_1_13EF8(s16 mode, s16 frameNo)
{
    lbl_1_bss_816 = 4;
    fn_1_CB0();
    if (frameNo == 0) {
        fn_1_C270(lbl_1_bss_814);
        HuAudSStreamFadeOut(lbl_1_bss_818, 100);
    }
    fn_1_C438();
    fn_1_12EA8();
}

/* Called during the pre-winner phase to finish the camera and result sequence. */
void fn_1_13F64(s16 mode, s16 frameNo)
{
    u8 sequenceReady;
    u8 callbackReady;

    lbl_1_bss_816 = 5;
    callbackReady = fn_1_CB4(frameNo);
    sequenceReady = fn_1_C438();
    fn_1_12EA8();
    if ((sequenceReady != 0 && callbackReady != 0) || frameNo > 240) {
        MgSeqModeNext();
    }
}

/* Called for winner and fade-out phases to run the exit animation and result display. */
void fn_1_13FEC(s16 mode, s16 frameNo)
{
    lbl_1_bss_816 = 6;
    fn_1_DE68();
    fn_1_12EA8();
}

void fn_1_14020(void) {
    lbl_1_bss_816 = 7;
}

MGSEQ_PARAM lbl_1_data_448 = {
    30, 2, fn_1_13C04, (MGSEQ_FUNC)fn_1_13D30, (MGSEQ_FUNC)fn_1_13DAC, (MGSEQ_FUNC)fn_1_13E58,
    fn_1_13EF8, fn_1_13F64, fn_1_13FEC, fn_1_13FEC, (MGSEQ_FUNC)fn_1_14020
};
