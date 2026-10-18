/* Runs the minigame sequence, selects the winning group, and controls sequence audio. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"
#include "game/flag.h"
#include "msm_stream.h"
#include "game/object.h"

#define MSM_SE_M644_OPENING 1974

void fn_1_234(s16 mode, s16 frameNo);
void fn_1_254(s16 mode, s16 frameNo);
void fn_1_2D4(s16 mode, s16 frameNo);
void fn_1_330(s16 mode, s16 frameNo);
void fn_1_334(s16 mode, s16 frameNo);
void fn_1_490(s16 mode, s16 frameNo);
void fn_1_4BC(s16 mode, s16 frameNo);
void fn_1_528(s16 mode, s16 frameNo);
void fn_1_52C(s16 mode, s16 frameNo);

/* Nonzero until a team finishes; used to select draw handling if time expires first. */
s16 lbl_1_data_0 = 1;
/* Winner character slots; -1 marks a player outside the winning team. */
s16 lbl_1_data_2[4] = {-1, -1, -1, -1};
/* Winning team, or -1 until a team reaches the finish. */
s16 lbl_1_data_A = -1;
/* Streamed minigame music handle, or -1 when stopped. */
s32 lbl_1_data_C = -1;
/* Opening sound-effect handle, or -1 before playback. */
static s32 lbl_1_data_10 = -1;
/* Five-minute game timer with the sequence phase callbacks below. */
MGSEQ_PARAM lbl_1_data_14 = {
    300, 0, fn_1_234, fn_1_254, fn_1_2D4, fn_1_330, fn_1_334, fn_1_490, fn_1_4BC, fn_1_528, fn_1_52C
};

void fn_1_1124(s32);
s32 fn_1_913C(s32);

s32 fn_1_559C(s16);
s32 fn_1_56D4(s32);

extern M644EffectMenuData lbl_1_bss_8;

typedef struct SceneModelIds {
    /* Scene model IDs in caller-defined order. */
    s32 model_ids[12];
} SceneModelIds;

extern void fn_1_11F0(void);

typedef struct M644CameraIdPair_s {
    /* Camera IDs for the two team views. */
    s32 camera_ids[2];
} M644CameraIdPair;

extern void fn_1_11F0(void);
extern f32 fn_1_9178(s32 entry_index);

extern void fn_1_94A4(s32 index);
extern s16 fn_1_9114(void);
extern void fn_1_1060(OMOBJ *object);

typedef struct M644CameraIds_s {
    /* Camera IDs for the two team views. */
    s32 camera_ids[2];
} M644CameraIds;

extern s32 fn_1_224(void);

s32 fn_1_367CC(s32 *player);

void fn_1_1690(void);
void fn_1_590(void);
/* Actor-manager process shared with module startup and player object creation. */
HUPROCESS *lbl_1_bss_0[2];

static const f64 lbl_1_rodata_0[2] = {0.5, 3.0};

/* Records the winning team's character slots when a route reaches the finish. */
void fn_1_B0(s16 group)
{
    s32 player_index = 0;
    s32 excluded_team;
    for (player_index = 0; player_index < 4; player_index++) {
        if (group == GwPlayerConf[player_index].grpNo)
            lbl_1_data_2[player_index] = GwPlayerConf[player_index].charNo;
    }
    lbl_1_data_0 = 0;
    lbl_1_data_A = group;
    if (lbl_1_data_2[0] != -1 && lbl_1_data_2[1] != -1 && lbl_1_data_2[2] != -1 &&
        lbl_1_data_2[3] != -1) {
        /* If both teams were recorded, randomly exclude one before showing the result. */
        excluded_team = rand8() % 2;
        for (player_index = 0; player_index < 4; player_index++) {
            if (excluded_team == GwPlayerConf[player_index].grpNo) lbl_1_data_2[player_index] = -1;
        }
        lbl_1_data_A = 1 - excluded_team;
    }
}

/* Returns the selected winning team, or -1 before a winner is recorded. */
s32 fn_1_224(void) {
    return lbl_1_data_A;
}

/* Advances immediately from sequence initialization to the opening phase. */
void fn_1_234(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

/* Plays and fades the opening sound during fade-in, then advances the sequence at frame 200. */
void fn_1_254(s16 mode, s16 frameNo) {
    if (MgSeqFrameNoGet() == 0) {
        lbl_1_data_10 = HuAudFXPlay(MSM_SE_M644_OPENING);
    }
    if ((MgSeqFrameNoGet() == 170) && (lbl_1_data_10 != -1)) {
        HuAudFXFadeOut(lbl_1_data_10, 500);
    }
    if (MgSeqFrameNoGet() == 200) {
        MgSeqModeNext();
    }
}

/* Starts the minigame music once the start announcement reaches its sound cue. */
void fn_1_2D4(s16 mode, s16 frameNo) {
    if ((lbl_1_data_C == -1) &&
        ((GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0)) {
        lbl_1_data_C = HuAudSStreamPlay(MSM_STREAM_MGMUS_22);
    }
}

/* Leaves the main play phase to the route and player object callbacks. */
void fn_1_330(s16 mode, s16 frameNo)
{
}

/* At finish, fades the music, slows losing teams, selects results and awards non-practice winner
 * coins. */
void fn_1_334(s16 mode, s16 frameNo) {
    s32 player_index;

    if (MgSeqFrameNoGet() == 0) {
        if ((s32) lbl_1_data_C != -1) {
            HuAudSStreamFadeOut(lbl_1_data_C, 100);
            lbl_1_data_C = -1;
        }
        if (lbl_1_data_0 != 0) {
            MgSeqDrawSet();
            fn_1_913C(0);
            fn_1_913C(1);
            fn_1_1124(0);
            fn_1_1124(1);
            return;
        }
        fn_1_913C(1 - lbl_1_data_A);
        fn_1_1124(1 - lbl_1_data_A);
        player_index = 0;
        while (player_index < 4) {
            if ((lbl_1_data_2[player_index] != -1) && (_CheckFlag(FLAG_MG_PRACTICE) == 0)) {
                GwPlayer[player_index].mgCoinBonus = 10;
            }
            player_index += 1;
        }
        MgSeqWinnerSet(lbl_1_data_2[0], lbl_1_data_2[1], lbl_1_data_2[2], lbl_1_data_2[3]);
    }
}

/* Ends the pre-winner pause after frame 120. */
void fn_1_490(s16 mode, s16 frameNo) {
    if (MgSeqFrameNoGet() > 120U) {
        MgSeqModeNext();
    }
}

/* On winner-phase frame zero, starts the winning team's hip-drop pose. A timeout draw requests
 * team 2, so no player's pose changes. */
void fn_1_4BC(s16 mode, s16 frameNo) {
    s32 status;

    status = 0;
    if (MgSeqFrameNoGet() == 0) {
        if (lbl_1_data_0 != 0) {
            fn_1_56D4(1 - lbl_1_data_A);
            return;
        }
        fn_1_559C(lbl_1_data_A);
    }
}

/* Adds no extra work during the sequence's fade-out phase. */
void fn_1_528(s16 mode, s16 frameNo)
{
}

/* Adds no extra work during the sequence's closing phase. */
void fn_1_52C(s16 mode, s16 frameNo)
{
}

/* Creates Rocky Road's actor manager, sequence and scene objects when the module starts. */
void fn_1_530(void) {
    lbl_1_bss_0[0] = MgActorObjectSetup();
    MgSeqCreate(&lbl_1_data_14);
    fn_1_1690();
    HuPrcChildCreate(fn_1_590, 544U, 8192U, 0, lbl_1_bss_0[0]);
}

/* Runs actor updates once per scheduler pass throughout the minigame. */
void fn_1_590(void) {
    for (;;) {
        MgActorExec();
        HuPrcVSleep();
    }
}
