/* Minigame sequence callbacks, constants, and scene setup for Garden Grab. */
#include "REL/m635dll.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/pad.h"

u16 lbl_1_data_0[6] = {
    PAD_BUTTON_A, PAD_BUTTON_B, PAD_BUTTON_X, PAD_BUTTON_Y,
    PAD_BUTTON_TRIGGER_L, PAD_BUTTON_TRIGGER_R
};
char lbl_1_data_C[6] = { 'A', 'B', 'X', 'Y', 'L', 'R' };
s16 lbl_1_data_12[4][2] = { { 60, 30 }, { 55, 25 }, { 40, 15 }, { 32, 6 } };
OM_CAMERA_VIEW lbl_1_data_24[3] = {
    { { 0, 180, -500 }, { -13, 0, 0 }, 1500 },
    { { -200, 180, -320 }, { -13, 0, 0 }, 1500 },
    { { 200, 180, -320 }, { -13, 0, 0 }, 1500 }
};

MGSEQ_PARAM lbl_1_data_78 = {
    300, 0, fn_1_F0, fn_1_134, fn_1_164, fn_1_1AC, fn_1_224,
    fn_1_2D0, fn_1_3D8, fn_1_3DC, fn_1_3E0
};

/* MGSEQ initialization callback: create the object manager and start the sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_64 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_64);
    MgSeqCreate(&lbl_1_data_78);
}

/* MGSEQ setup callback registered in lbl_1_data_78: initialize gameplay and create scene/player
 * objects on entry. */
void fn_1_F0(s16 mode, s16 frameNo)
{
    fn_1_10A0();
    fn_1_1774(lbl_1_bss_64);
    fn_1_195C();
    fn_1_2954();
    fn_1_4114();
    fn_1_2018();
    MgSeqModeNext();
}

/* MGSEQ fade-in callback registered in lbl_1_data_78: advance the intro and update player/team
 * animations each frame. */
void fn_1_134(s16 mode, s16 frameNo)
{
    fn_1_408(frameNo);
    fn_1_2CE8();
    fn_1_3C34();
}

/* MGSEQ start callback registered in lbl_1_data_78: start music on entry and update the intro scene
 * each frame. */
void fn_1_164(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        lbl_1_bss_4.music = HuAudBGMPlay(MSM_STREAM_MGMUS_21);
    }
    fn_1_2CE8();
    fn_1_3C34();
}

/* MGSEQ main callback registered in lbl_1_data_78: show controls on entry, then update gameplay
 * each frame. */
void fn_1_1AC(s16 mode, s16 frameNo)
{
    int teamIndex;

    if (frameNo == 0) {
        for (teamIndex = 0; teamIndex < 2; teamIndex++) {
            fn_1_25EC(teamIndex, 1, 1);
        }
    }
    fn_1_954(frameNo);
    fn_1_2CE8();
    fn_1_3C34();
    fn_1_42A0();
    fn_1_2330();
}

/* MGSEQ finish callback registered in lbl_1_data_78: hide prompts, mark ties as draws, fade music
 * on entry, and update the scene each frame. */
void fn_1_224(s16 mode, s16 frameNo)
{
    int teamIndex;
    int playerIndex;

    if (frameNo == 0) {
        for (teamIndex = 0; teamIndex < 2; teamIndex++) {
            for (playerIndex = 0; playerIndex < 2; playerIndex++) {
                fn_1_25EC(teamIndex, playerIndex, 5);
            }
        }
        if (lbl_1_bss_4.winner == -1) {
            MgSeqDrawSet();
        }
        HuAudSStreamFadeOut(lbl_1_bss_4.music, 100);
    }
    fn_1_2CE8();
    fn_1_3C34();
    fn_1_42A0();
}

/* MGSEQ pre-winner callback registered in lbl_1_data_78: wipe to the result view, then advance
 * through four states. */
void fn_1_2D0(s16 mode, s16 frameNo)
{
    switch (lbl_1_bss_4.state) {
        case 0:
            MgSeqModeChangeOff();
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            lbl_1_bss_4.state++;
            break;
        case 1:
            if (!WipeCheck()) {
                fn_1_638();
                fn_1_448C();
                lbl_1_bss_4.state++;
            }
            break;
        case 2:
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            lbl_1_bss_4.state++;
            break;
        case 3:
            if (!WipeCheck()) {
                lbl_1_bss_4.state = 0;
                fn_1_8B0();
                MgSeqModeNext();
            }
            break;
    }
    fn_1_2CE8();
    fn_1_42A0();
    fn_1_3C34();
}

/* MGSEQ winner callback; this minigame has no work in this phase. */
void fn_1_3D8(s16 mode, s16 frameNo) {}
/* MGSEQ fade-out callback; this minigame has no work in this phase. */
void fn_1_3DC(s16 mode, s16 frameNo) {}

/* MGSEQ close callback registered in lbl_1_data_78: remove the button-prompt sprites on exit. */
void fn_1_3E0(s16 mode, s16 frameNo)
{
    fn_1_2014();
    fn_1_33F8();
    fn_1_2904();
}
