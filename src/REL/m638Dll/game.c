/* Gondola Glide scene setup, split-screen play, course events, and results. */
#include "REL/m638Dll.h"

#define M638_OVERLAY_MEMORY_ID HU_MEMNUM_OVL
#define M638_GAMEPLAY_BGM_ID 79
#define M638_PLAY_START_SFX_ID 13
#define M638_TEAM0_WIN_SFX_ID 1893
#define M638_TEAM1_WIN_SFX_ID 1894
#define M638_TEAM0_EVENT_BUTTON0_SFX_ID 1889
#define M638_TEAM0_EVENT_BUTTON1_SFX_ID 1887
#define M638_TEAM1_EVENT_BUTTON0_SFX_ID 1890
#define M638_TEAM1_EVENT_BUTTON1_SFX_ID 1888
#define M638_TEAM0_BOOST_SFX_ID 1885
#define M638_TEAM1_BOOST_SFX_ID 1886
#define M638_TEAM0_FINISH_MOTION_SFX_ID 1895
#define M638_TEAM1_FINISH_MOTION_SFX_ID 1896
#define M638_TEAM0_FINISH_END_SFX_ID 1897
#define M638_TEAM1_FINISH_END_SFX_ID 1898
#define M638_TEAM0_ACTIVITY_SFX_ID 1883
#define M638_TEAM1_ACTIVITY_SFX_ID 1884
#define M638_FLAG_SKIP_WINNER_COIN_BONUS 65551U

MGSEQ_PARAM lbl_1_data_0 = {
    300, 1, fn_1_1EC, fn_1_284, fn_1_528, fn_1_598, fn_1_77C, fn_1_A28,
    fn_1_D84, fn_1_11EC, fn_1_11F0,
};

/* Course-segment hook names, grouped by the five authored course variants. */
char lbl_1_data_28[9] = "fa_rdb00";

char lbl_1_data_31[9] = "fa_rdb01";

char lbl_1_data_3A[9] = "fa_rdb10";

char lbl_1_data_43[9] = "fa_rdb11";

char lbl_1_data_4C[9] = "fa_rdb20";

char lbl_1_data_55[9] = "fa_rdb21";

char lbl_1_data_5E[9] = "fa_rdb30";

char lbl_1_data_67[9] = "fa_rdb31";

char lbl_1_data_70[9] = "fb_rdb00";

char lbl_1_data_79[9] = "fb_rdb01";

char lbl_1_data_82[9] = "fb_rdb10";

char lbl_1_data_8B[9] = "fb_rdb11";

char lbl_1_data_94[9] = "fb_rdb20";

char lbl_1_data_9D[9] = "fb_rdb21";

char lbl_1_data_A6[9] = "fb_rdb30";

char lbl_1_data_AF[9] = "fb_rdb31";

char lbl_1_data_B8[9] = "fc_rdb00";

char lbl_1_data_C1[9] = "fc_rdb01";

char lbl_1_data_CA[9] = "fc_rdb10";

char lbl_1_data_D3[9] = "fc_rdb11";

char lbl_1_data_DC[9] = "fc_rdb20";

char lbl_1_data_E5[9] = "fc_rdb21";

char lbl_1_data_EE[9] = "fc_rdb30";

char lbl_1_data_F7[9] = "fc_rdb31";

char lbl_1_data_100[9] = "fd_rdb00";

char lbl_1_data_109[9] = "fd_rdb01";

char lbl_1_data_112[9] = "fd_rdb10";

char lbl_1_data_11B[9] = "fd_rdb11";

char lbl_1_data_124[9] = "fd_rdb20";

char lbl_1_data_12D[9] = "fd_rdb21";

char lbl_1_data_136[9] = "fd_rdb30";

char lbl_1_data_13F[9] = "fd_rdb31";

char lbl_1_data_148[9] = "fe_rdb00";

char lbl_1_data_151[9] = "fe_rdb01";

char lbl_1_data_15A[9] = "fe_rdb10";

char lbl_1_data_163[9] = "fe_rdb11";

char lbl_1_data_16C[9] = "fe_rdb20";

char lbl_1_data_175[9] = "fe_rdb21";

char lbl_1_data_17E[9] = "fe_rdb30";

char lbl_1_data_187[9] = "fe_rdb31";

char *lbl_1_data_190[40] = {
    lbl_1_data_28, lbl_1_data_31, lbl_1_data_3A, lbl_1_data_43,
    lbl_1_data_4C, lbl_1_data_55, lbl_1_data_5E, lbl_1_data_67,
    lbl_1_data_70, lbl_1_data_79, lbl_1_data_82, lbl_1_data_8B,
    lbl_1_data_94, lbl_1_data_9D, lbl_1_data_A6, lbl_1_data_AF,
    lbl_1_data_B8, lbl_1_data_C1, lbl_1_data_CA, lbl_1_data_D3,
    lbl_1_data_DC, lbl_1_data_E5, lbl_1_data_EE, lbl_1_data_F7,
    lbl_1_data_100, lbl_1_data_109, lbl_1_data_112, lbl_1_data_11B,
    lbl_1_data_124, lbl_1_data_12D, lbl_1_data_136, lbl_1_data_13F,
    lbl_1_data_148, lbl_1_data_151, lbl_1_data_15A, lbl_1_data_163,
    lbl_1_data_16C, lbl_1_data_175, lbl_1_data_17E, lbl_1_data_187,
};

/* Model-hook names for course events; this byte string marks unused event slots. */
char lbl_1_data_230[5] = { 130, 200, 130, 181, 0 };

char lbl_1_data_235[9] = "fa_sdb00";

char lbl_1_data_23E[9] = "fb_sdb00";

char lbl_1_data_247[9] = "fb_sdb01";

char lbl_1_data_250[9] = "fc_sdb00";

char lbl_1_data_259[9] = "fc_sdb01";

char lbl_1_data_262[9] = "fc_sdb02";

char lbl_1_data_26B[9] = "fd_sdb00";

char lbl_1_data_274[9] = "fd_sdb01";

char lbl_1_data_27D[9] = "fe_sdb01";

char lbl_1_data_286[10] = "fe_sdb00";

char *lbl_1_data_290[40] = {
    lbl_1_data_230, lbl_1_data_230, lbl_1_data_230, lbl_1_data_230,
    lbl_1_data_230, lbl_1_data_230, lbl_1_data_230, lbl_1_data_235,
    lbl_1_data_230, lbl_1_data_230, lbl_1_data_23E, lbl_1_data_230,
    lbl_1_data_230, lbl_1_data_247, lbl_1_data_230, lbl_1_data_230,
    lbl_1_data_230, lbl_1_data_230, lbl_1_data_250, lbl_1_data_230,
    lbl_1_data_259, lbl_1_data_230, lbl_1_data_262, lbl_1_data_230,
    lbl_1_data_26B, lbl_1_data_230, lbl_1_data_230, lbl_1_data_230,
    lbl_1_data_230, lbl_1_data_230, lbl_1_data_274, lbl_1_data_230,
    lbl_1_data_27D, lbl_1_data_230, lbl_1_data_230, lbl_1_data_230,
    lbl_1_data_286, lbl_1_data_230, lbl_1_data_230, lbl_1_data_230,
};

/* Per-stop course time and prop rotation settings, in motion frames and degrees. */
struct _struct_lbl_1_data_330_0x8 lbl_1_data_330[10] = {
    { 0.0f, 0.0f },
    { 245.0f, 10.0f },
    { 285.0f, 0.0f },
    { 287.0f, -10.0f },
    { 350.0f, 0.0f },
    { 362.0f, -10.0f },
    { 480.0f, 0.0f },
    { 505.0f, -10.0f },
    { 535.0f, 0.0f },
    { 600.0f, 0.0f },
};

/* Event prop placement angles in degrees, indexed by course stop. */
f32 lbl_1_data_380[10] = { 0.0f, 0.0f, 0.0f, 14.67f, 11.5f, 0.0f, 6.2f, 22.45f, 0.0f, 0.0f };

s32 lbl_1_data_3A8[73] = {
    DATANUM(DATA_m638, 0), DATANUM(DATA_m638, 1), DATANUM(DATA_m638, 2), DATANUM(DATA_m638, 3),
    DATANUM(DATA_m638, 4), DATANUM(DATA_m638, 5), DATANUM(DATA_m638, 6), DATANUM(DATA_m638, 7),
    DATANUM(DATA_m638, 8), DATANUM(DATA_m638, 9), DATANUM(DATA_m638, 10), DATANUM(DATA_m638, 11),
    DATANUM(DATA_m638, 12), DATANUM(DATA_m638, 13), DATANUM(DATA_m638, 14), DATANUM(DATA_m638, 15),
    DATANUM(DATA_m638, 16), DATANUM(DATA_m638, 17), DATANUM(DATA_m638, 18), DATANUM(DATA_m638, 19),
    DATANUM(DATA_m638, 20), DATANUM(DATA_m638, 21), DATANUM(DATA_m638, 22), DATANUM(DATA_m638, 23),
    DATANUM(DATA_m638, 24), DATANUM(DATA_m638, 25), DATANUM(DATA_m638, 26), DATANUM(DATA_m638, 27),
    DATANUM(DATA_m638, 28), DATANUM(DATA_m638, 29), DATANUM(DATA_m638, 30), DATANUM(DATA_m638, 31),
    DATANUM(DATA_m638, 32), DATANUM(DATA_m638, 33), DATANUM(DATA_m638, 34), DATANUM(DATA_m638, 35),
    DATANUM(DATA_m638, 36), DATANUM(DATA_m638, 37), DATANUM(DATA_m638, 38), DATANUM(DATA_m638, 39),
    DATANUM(DATA_m638, 40), DATANUM(DATA_m638, 41), DATANUM(DATA_m638, 42), DATANUM(DATA_m638, 43),
    DATANUM(DATA_m638, 44), DATANUM(DATA_m638, 45), DATANUM(DATA_m638, 46), DATANUM(DATA_m638, 47),
    DATANUM(DATA_m638, 48), DATANUM(DATA_m638, 53), DATANUM(DATA_m638, 54), DATANUM(DATA_m638, 55),
    DATANUM(DATA_m638, 56), DATANUM(DATA_m638, 57), DATANUM(DATA_m638, 58), DATANUM(DATA_m638, 59),
    DATANUM(DATA_m638, 60), DATANUM(DATA_m638, 61), DATANUM(DATA_m638, 62), DATANUM(DATA_m638, 63),
    DATANUM(DATA_m638, 64), DATANUM(DATA_m638, 65), DATANUM(DATA_m638, 66), DATANUM(DATA_m638, 67),
    DATANUM(DATA_m638, 68), DATANUM(DATA_m638, 69), DATANUM(DATA_m638, 70), DATANUM(DATA_m638, 71),
    DATANUM(DATA_m638, 72), DATANUM(DATA_m638, 73), DATANUM(DATA_m638, 74), DATANUM(DATA_m638, 75),
    DATANUM(DATA_m638, 76),
};

char lbl_1_data_4CC[] = "l";

char lbl_1_data_4CE[] = "r";

char lbl_1_data_4D0[7] = "rlever";

char lbl_1_data_4D7[7] = "llever";

char lbl_1_data_4DE[6] = "db_ab";

char lbl_1_data_4E4[7] = "tenban";

char lbl_1_data_4EB[4] = "db0";

char lbl_1_data_4EF[4] = "db1";

char lbl_1_data_4F3[4] = "db2";

char lbl_1_data_4F7[4] = "db3";

char lbl_1_data_4FB[7] = "db_sl0";

char lbl_1_data_502[7] = "db_sl1";

char lbl_1_data_509[7] = "db_sl2";

char lbl_1_data_510[7] = "db_sl3";

char lbl_1_data_517[9] = "db_sl4";

M638TimerWatch lbl_1_bss_10;

HUPROCESS *lbl_1_bss_C;

s32 lbl_1_bss_8;

OMOBJ *lbl_1_bss_4;

/* Scene-state bytes whose purpose is not used by the minigame logic here. */
u8 lbl_1_bss_0[4];

const u16 lbl_1_rodata_10[4] = { 1, 2, 4, 8 };

/* Create the scene and player objects, prepare the cameras, then start the sequence. */
void fn_1_A0(void)
{
    OMOBJ *obj;
    s32 player;
    M638SceneView *scene;

    lbl_1_bss_C = omInitObjMan(10, 4096);
    omGameSysInit(lbl_1_bss_C);
    obj = omAddObjEx(lbl_1_bss_C, 101, 0, 0, -1, fn_1_1988);
    lbl_1_bss_4 = obj;
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M638SceneView), M638_OVERLAY_MEMORY_ID);
    scene = obj->data;
    for (player = 0; player < 4; player++) {
        obj = omAddObjEx(lbl_1_bss_C, 100, 1, 5, 0, fn_1_67EC);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M638PlayerView), M638_OVERLAY_MEMORY_ID);
        obj->work[0] = player;
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DNoSyncSet(1);
    fn_1_1CD4(scene);
    fn_1_1294();
    fn_1_165C(&scene->camera);
    fn_1_13B4((M638LightingView *)scene);
    fn_1_1470((M638LightingView *)scene);
    MgSeqCreate(&lbl_1_data_0);
}

/* Sequence init hook: create the timer, read the record, and prepare both courses. */
void fn_1_1EC(s16 mode, s16 frameNo)
{
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    scene->record = GWRecordGet(GW_RECORD_M638);
    if ((s32) scene->record == 0) {
        scene->record = 18000U;
    }
    scene->timer = MgTimerCreate(1);
    MgTimerParamSet(scene->timer, 0, 18000, (s32) scene->record);
    MgTimerRecordDispOn(scene->timer);
    fn_1_3804(scene);
    fn_1_1E58(scene);
    fn_1_4E48(scene);
    MgSeqModeNext();
}

/* Sequence fade-in hook: animate the split view before the play phase begins. */
void fn_1_284(s16 mode, s16 frameNo)
{
    f32 splitProgress;
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->introState) {
    case 0:
        MgSeqModeChangeOff();
        scene->teams[0].players[0]->impulse = 10.0f;
        scene->teams[0].players[1]->impulse = 10.0f;
        scene->teams[1].players[0]->impulse = 10.0f;
        scene->teams[1].players[1]->impulse = 10.0f;
        scene->teams[0].blend = 0.3f;
        scene->teams[1].blend = 0.3f;
        scene->seqTime = 0.0f;
        scene->camera.splitX = 640.0f;
        scene->introState++;
        break;
    case 1:
        scene->seqTime += 1.0f;
        if (scene->seqTime >= 90.0f) {
            scene->seqTime = 0.0f;
            scene->introState++;
        }
        break;
    case 2:
        scene->seqTime += 1.0f;
        splitProgress = scene->seqTime / 60.0f;
        if (splitProgress > 1.0f) {
            splitProgress = 1.0f;
        }
        scene->camera.splitX = (f32) ((640.0f * (1.0f - splitProgress)) + (320.0f * splitProgress));
        if (scene->seqTime >= 60.0f) {
            scene->seqTime = 0.0f;
            scene->introState++;
        }
        break;
    case 3:
        espAttrReset(scene->sprites[0], 4U);
        espAttrReset(scene->sprites[1], 4U);
        espAttrReset(scene->sprites[2], 4U);
        scene->camera.splitX = 320.0f;
        scene->introState++;
        /* fallthrough */
    case 4:
        scene->seqTime += 1.0f;
        if (scene->seqTime >= 100.0f) {
            MgSeqModeNext();
        }
        break;
    }
}

/* Sequence start hook: start the minigame music before input is enabled. */
void fn_1_528(s16 mode, s16 frameNo)
{
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->startState) {
    case 0:
        lbl_1_bss_8 = HuAudBGMPlay(M638_GAMEPLAY_BGM_ID);
        scene->startState++;
        /* fallthrough */
    case 1:
        break;
    }
}

/* Main sequence hook: activate both teams and select a winner when either reaches ten. */
void fn_1_598(s16 mode, s16 frameNo)
{
    f32 motionTimes[2];
    s32 rightTeamFinished;
    s32 leftTeamFinished;
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->playState) {
    case 0:
        MgTimerModeOnSet(scene->timer, 0);
        HuAudFXPlay(M638_PLAY_START_SFX_ID);
        espAttrReset(scene->teams[0].sprites[0], 4U);
        espAttrReset(scene->teams[1].sprites[0], 4U);
        espAttrReset(scene->teams[0].sprites[1], 4U);
        espAttrReset(scene->teams[1].sprites[1], 4U);
        scene->teams[0].active = 1;
        scene->teams[1].active = 1;
        scene->playState++;
        break;
    case 1:
        leftTeamFinished = scene->teams[0].count == 10;
        rightTeamFinished = scene->teams[1].count == 10;
        if ((leftTeamFinished != 0) || (rightTeamFinished != 0)) {
            motionTimes[0] = Hu3DMotionTimeGet(scene->teams[0].course.model);
            motionTimes[1] = Hu3DMotionTimeGet(scene->teams[1].course.model);
            if ((leftTeamFinished != 0) && (rightTeamFinished != 0)) {
                if (motionTimes[0] > motionTimes[1]) {
                    scene->winner = 0;
                } else if (motionTimes[1] > motionTimes[0]) {
                    scene->winner = 1;
                } else {
                    scene->winner = frandmod(2);
                }
            } else if (leftTeamFinished != 0) {
                scene->winner = 0;
            } else {
                scene->winner = 1;
            }
            if (scene->winner == 0) {
                HuAudFXPlay(M638_TEAM0_WIN_SFX_ID);
            } else if (scene->winner == 1) {
                HuAudFXPlay(M638_TEAM1_WIN_SFX_ID);
            }
            MgTimerModeOffSet(scene->timer);
            scene->teams[0].active = 0;
            scene->teams[1].active = 0;
            MgSeqModeNext();
        }
        break;
    }
}

/* Finish hook: stop play, record the outcome, and begin the team finish animations. */
void fn_1_77C(s16 mode, s16 frameNo)
{
    s32 spriteIndex;
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->finishState) {
    case 0:
        HuAudSStreamFadeOut(lbl_1_bss_8, 100);
        spriteIndex = 0;
        while (spriteIndex < 5) {
            espAttrSet(scene->teams[0].sprites[spriteIndex], 4U);
            espAttrSet(scene->teams[1].sprites[spriteIndex], 4U);
            spriteIndex += 1;
        }
        scene->teams[0].active = 0;
        scene->teams[1].active = 0;
        scene->teams[0].elapsed = 0.0f;
        scene->teams[1].elapsed = 0.0f;
        scene->teams[0].base = (f32) scene->teams[0].blend;
        scene->teams[1].base = (f32) scene->teams[1].blend;
        scene->teams[0].state = 0;
        scene->teams[1].state = 0;
        if (scene->winner < 0) {
            MgSeqWinnerSet(-1, -1, -1, -1);
            fn_1_7660(fn_1_66C0, scene);
            fn_1_7660(fn_1_66C0, &scene->teams[1]);
        } else {
            espAttrSet(scene->sprites[0], 4U);
            espAttrSet(scene->sprites[1], 4U);
            espAttrSet(scene->sprites[2], 4U);
            MgSeqWinnerSet(scene->teams[scene->winner].players[0]->charNo,
                           scene->teams[scene->winner].players[1]->charNo, -1, -1);
            scene->time = MgTimerValueGet(scene->timer);
            if ((scene->time < scene->record) &&
                (_CheckFlag(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) &&
                ((scene->teams[scene->winner].players[0]->isCom == 0) ||
                 (scene->teams[scene->winner].players[1]->isCom == 0))) {
                scene->newRecord = 1;
                MgSeqRecordSet(scene->time);
                GWRecordSet(GW_RECORD_M638, (u32) scene->time);
            }
            if (fn_1_7660(fn_1_6374, scene) != 0) {
                scene->camera.elapsed = 0.0f;
            }
            fn_1_7660(fn_1_647C, &scene->teams[scene->winner]);
            fn_1_7660(fn_1_66C0, &scene->teams[scene->winner ^ 1]);
            scene->teams[scene->winner].actor.blend = scene->teams[scene->winner].actor.rot.x;
            scene->teams[scene->winner].actor.swingEnabled = 0;
        }
        scene->finishState++;
        /* fallthrough */
    case 1:
        break;
    }
}

/* Pre-winner hook: wait for player exit animations, then continue or open the result screen. */
void fn_1_A28(s16 mode, s16 frameNo)
{
    f32 durations[2] = { 240.0f, 270.0f };
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    scene->exitTime += 1.0f;
    switch (scene->resultState) {
    case 0:
        MgSeqModeChangeOff();
        if (scene->winner >= 0) {
            if (scene->newRecord != 0) {
                fn_1_7660(fn_1_6790, &lbl_1_bss_10);
                lbl_1_bss_10.count = 0;
                lbl_1_bss_10.timer = scene->timer;
            }
            scene->seqTime = 0.0f;
            scene->resultState = 1;
        } else {
            scene->resultState = 10;
        }
        break;
    case 1:
        scene->seqTime += 1.0f;
        if (scene->seqTime >= 30.0f) {
            scene->resultState++;
        }
        break;
    case 2:
        if (scene->teams[scene->winner].state == 2) {
            fn_1_7660(fn_1_6FE4, scene->teams[scene->winner].players[0]);
            fn_1_7660(fn_1_6FE4, scene->teams[scene->winner].players[1]);
            scene->resultState++;
        }
        break;
    case 3:
        if ((scene->teams[scene->winner].players[0]->exitState == 2) &&
            (scene->teams[scene->winner].players[1]->exitState == 2)) {
            scene->resultState = 100;
        }
        break;
    case 10:
        if (scene->teams[0].state == 2) {
            fn_1_7660(fn_1_6FE4, scene->teams[0].players[0]);
            fn_1_7660(fn_1_6FE4, scene->teams[0].players[1]);
            fn_1_7660(fn_1_6FE4, scene->teams[1].players[0]);
            fn_1_7660(fn_1_6FE4, scene->teams[1].players[1]);
            scene->resultState++;
        }
        break;
    case 11:
        if ((scene->teams[0].players[0]->exitState == 2) &&
            (scene->teams[0].players[1]->exitState == 2) &&
            (scene->teams[1].players[0]->exitState == 2) &&
            (scene->teams[1].players[1]->exitState == 2)) {
            scene->resultState = 100;
        }
        break;
    case 100:
        scene->seqTime = 0.0f;
        scene->resultState++;
        /* fallthrough */
    case 101:
        scene->seqTime += 1.0f;
        if (scene->seqTime >= 30.0f) {
            if (scene->winner < 0) {
                MgSeqModeNext();
            } else if (scene->exitTime > durations[scene->newRecord]) {
                MgSeqModeNext();
            } else {
                MgSeqModeChangeOn();
                scene->resultState++;
            }
        }
        break;
    case 102:
        break;
    }
}

/* Winner sequence hook: award the result and play the winning or tie presentation. */
void fn_1_D84(s16 mode, s16 frameNo)
{
    s32 firstWinnerPlayerNo;
    s32 secondWinnerPlayerNo;
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->winnerSequenceState) {
    case 0:
        if (scene->winner < 0) {
            CharMotionShiftSet(scene->teams[0].players[0]->charNo,
                               scene->teams[0].players[0]->motions[3],
                               0.0f, 5.0f, 0U);
            CharMotionShiftSet(scene->teams[0].players[1]->charNo,
                               scene->teams[0].players[1]->motions[3],
                               0.0f, 5.0f, 0U);
            CharMotionShiftSet(scene->teams[1].players[0]->charNo,
                               scene->teams[1].players[0]->motions[3],
                               0.0f, 5.0f, 0U);
            CharMotionShiftSet(scene->teams[1].players[1]->charNo,
                               scene->teams[1].players[1]->motions[3],
                               0.0f, 5.0f, 0U);
            CharLoseVoicePlay(scene->teams[0].players[0]->charNo,
                              scene->teams[0].players[1]->charNo,
                              scene->teams[1].players[0]->charNo,
                              scene->teams[1].players[1]->charNo);
            Hu3DModelObjPosGet(scene->teams[0].actor.modelA,
                               lbl_1_data_4CC,
                               &scene->teams[0].players[0]->pos);
            Hu3DModelObjPosGet(scene->teams[0].actor.modelA,
                               lbl_1_data_4CE,
                               &scene->teams[0].players[1]->pos);
            scene->teams[0].players[0]->rot.y = 0.0f;
            scene->teams[0].players[1]->rot.y = 0.0f;
            Hu3DModelHookObjReset(scene->teams[0].actor.modelA,
                                  lbl_1_data_4CC);
            Hu3DModelHookObjReset(scene->teams[0].actor.modelA,
                                  lbl_1_data_4CE);
            fn_1_7554(scene->teams[0].players[0]);
            fn_1_7554(scene->teams[0].players[1]);
            Hu3DModelObjPosGet(scene->teams[1].actor.modelA,
                               lbl_1_data_4CC,
                               &scene->teams[1].players[0]->pos);
            Hu3DModelObjPosGet(scene->teams[1].actor.modelA,
                               lbl_1_data_4CE,
                               &scene->teams[1].players[1]->pos);
            scene->teams[1].players[0]->rot.y = 0.0f;
            scene->teams[1].players[1]->rot.y = 0.0f;
            Hu3DModelHookObjReset(scene->teams[1].actor.modelA,
                                  lbl_1_data_4CC);
            Hu3DModelHookObjReset(scene->teams[1].actor.modelA,
                                  lbl_1_data_4CE);
            fn_1_7554(scene->teams[1].players[0]);
            fn_1_7554(scene->teams[1].players[1]);
        } else {
            CharMotionShiftSet(scene->teams[scene->winner].players[0]->charNo,
                               scene->teams[scene->winner].players[0]->motions[2],
                               0.0f, 5.0f, 0U);
            CharMotionShiftSet(scene->teams[scene->winner].players[1]->charNo,
                               scene->teams[scene->winner].players[1]->motions[2],
                               0.0f, 5.0f, 0U);
            firstWinnerPlayerNo = scene->teams[scene->winner].players[0]->playerNo;
            if (_CheckFlag(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) {
                GwPlayer[firstWinnerPlayerNo].mgCoinBonus = 10;
            }
            secondWinnerPlayerNo = scene->teams[scene->winner].players[1]->playerNo;
            if (_CheckFlag(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) {
                GwPlayer[secondWinnerPlayerNo].mgCoinBonus = 10;
            }
            Hu3DModelObjPosGet(scene->teams[scene->winner].actor.modelA,
                               lbl_1_data_4CC,
                               &scene->teams[scene->winner].players[0]->pos);
            Hu3DModelObjPosGet(scene->teams[scene->winner].actor.modelA,
                               lbl_1_data_4CE,
                               &scene->teams[scene->winner].players[1]->pos);
            scene->teams[scene->winner].players[0]->rot.y = 0.0f;
            scene->teams[scene->winner].players[1]->rot.y = 0.0f;
            Hu3DModelHookObjReset(scene->teams[scene->winner].actor.modelA,
                                  lbl_1_data_4CC);
            Hu3DModelHookObjReset(scene->teams[scene->winner].actor.modelA,
                                  lbl_1_data_4CE);
            fn_1_7554(scene->teams[scene->winner].players[0]);
            fn_1_7554(scene->teams[scene->winner].players[1]);
        }
        scene->winnerSequenceState++;
        break;
    case 1:
        break;
    }
}

/* Fade-out phase: the sequence advances without a separate scene action. */
void fn_1_11EC(s16 mode, s16 frameNo)
{

}

/* Close hook: release the timer and all sprites created for the scene and teams. */
void fn_1_11F0(s16 mode, s16 frameNo)
{
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    MgTimerKill(scene->timer);
    espKill(scene->sprites[0]);
    espKill(scene->sprites[1]);
    espKill(scene->sprites[2]);
    espKill(scene->teams[0].sprites[0]);
    espKill(scene->teams[0].sprites[1]);
    espKill(scene->teams[0].sprites[2]);
    espKill(scene->teams[0].sprites[3]);
    espKill(scene->teams[0].sprites[4]);
    espKill(scene->teams[1].sprites[0]);
    espKill(scene->teams[1].sprites[1]);
    espKill(scene->teams[1].sprites[2]);
    espKill(scene->teams[1].sprites[3]);
    espKill(scene->teams[1].sprites[4]);
}

/* Create the two gameplay cameras used by the split-screen view. */
void fn_1_1294(void)
{
    s32 cameraIndex;

    cameraIndex = 0;
    while (cameraIndex < 2) {
        Hu3DCameraCreate((s32) lbl_1_rodata_10[cameraIndex]);
        Hu3DCameraPerspectiveSet((s32) lbl_1_rodata_10[cameraIndex], 100.0f, 20.0f, 40000.0f, 0.6f);
        Hu3DCameraPosSet((s32) lbl_1_rodata_10[cameraIndex], 0.0f, -80.0f, 400.0f, 0.0f, 1.0f, 0.0f,
                         0.0f, -470.0f, -1200.0f);
        cameraIndex += 1;
    }
}

/* Build the directional light used by both gameplay cameras. */
void fn_1_13B4(M638LightingView *scene)
{
    Point3d lightDirection;
    s16 lightId;

    PSVECSubtract(&scene->lightTarget, &scene->lightPosition, &lightDirection);
    PSVECNormalize(&lightDirection, &lightDirection);
    if (scene->alternateLight != 0) {
        lightId =
            Hu3DGLightCreate(scene->lightPosition.x, scene->lightPosition.y, scene->lightPosition.z,
                             lightDirection.x, lightDirection.y, lightDirection.z, 95U, 95U, 127U);
    } else {
        lightId = Hu3DGLightCreate(scene->lightPosition.x, scene->lightPosition.y,
                                   scene->lightPosition.z, lightDirection.x, lightDirection.y,
                                   lightDirection.z, 207U, 207U, 207U);
    }
    Hu3DGLightInfinitytSet(lightId);
}

const Point3d lbl_1_rodata_68 = { 0.0f, 1.0f, 0.0f };

/* Position the shared shadow volume and set its lighting for both cameras. */
void fn_1_1470(M638LightingView *scene)
{
    Point3d shadowSide;
    Point3d shadowDirection;
    Point3d shadowRight;
    Point3d shadowPosition;
    Point3d shadowUp;

    shadowUp = lbl_1_rodata_68;
    shadowPosition.x = scene->lightPosition.x;
    shadowPosition.y = 0.3f * scene->lightPosition.y;
    shadowPosition.z = 0.3f * scene->lightPosition.z;
    PSVECSubtract(&shadowPosition, &scene->lightTarget, &shadowDirection);
    PSVECNormalize(&shadowDirection, &shadowDirection);
    PSVECCrossProduct(&shadowUp, &shadowDirection, &shadowRight);
    PSVECNormalize(&shadowRight, &shadowRight);
    PSVECCrossProduct(&shadowDirection, &shadowRight, &shadowSide);
    PSVECNormalize(&shadowSide, &shadowSide);
    Hu3DShadowMultiCreate(100.0f, 20.0f, 40000.0f, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    Hu3DShadowMultiTPLvlSet(0.5f, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    Hu3DShadowMultiPosSet(&shadowPosition, &shadowSide, &scene->lightTarget,
                          (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    if (scene->alternateLight != 0) {
        Hu3DShadowMultiColSet(15U, 15U, 95U, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
        return;
    }
    Hu3DShadowMultiColSet(63U, 63U, 63U, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
}

/* Initialize the camera split before fn_1_16AC applies its viewports and lenses. */
void fn_1_165C(M638CameraWork *cameraWork)
{
    cameraWork->elapsed = 0.0f;
    cameraWork->splitX = 320.0f;
    fn_1_16AC(cameraWork);
}

/* Apply the current split position to both gameplay camera viewports and lenses. */
void fn_1_16AC(M638CameraWork *cameraWork)
{
    int camera;
    float aspect;
    camera = 0;
    Hu3DCameraViewportSet(lbl_1_rodata_10[camera], 0.0f, 0.0f,
        16.0f + cameraWork->splitX, 480.0f, 0.0f, 1.0f);
    camera++;
    Hu3DCameraViewportSet(lbl_1_rodata_10[camera], cameraWork->splitX - 16.0f,
        0.0f, 16.0f + (640.0f - cameraWork->splitX), 480.0f, 0.0f, 1.0f);
    camera = 0;
    Hu3DCameraScissorSet(lbl_1_rodata_10[camera], 0, 0, (u32)cameraWork->splitX, 480);
    camera++;
    Hu3DCameraScissorSet(lbl_1_rodata_10[camera], (u32)cameraWork->splitX, 0,
        (u32)(640.0f - cameraWork->splitX), 480);
    camera = 0;
    aspect = (0.9f * cameraWork->splitX) / 480.0f;
    if (aspect < 0.6f) {
        aspect = 0.6f;
    }
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[camera], 100.0f, 20.0f, 40000.0f, aspect);
    camera++;
    aspect = (0.9f * (640.0f - cameraWork->splitX)) / 480.0f;
    if (aspect < 0.6f) {
        aspect = 0.6f;
    }
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[camera], 100.0f, 20.0f, 40000.0f, aspect);
}

/* First object callback: initialize the scene job list, then install its update callback. */
void fn_1_1988(OMOBJ *obj)
{
    fn_1_75BC();
    obj->objFunc = fn_1_19C0;
}

/* Scene object update: advance both teams, courses, cameras, and queued scene effects. */
void fn_1_19C0(OMOBJ *obj)
{
    M638SceneView *scene;
    int camera;
    float aspect;
    scene = lbl_1_bss_4->data;
    fn_1_2288(&scene->teams[0]);
    fn_1_2288(&scene->teams[1]);
    fn_1_4B4C(&scene->teams[0].course);
    fn_1_4B4C(&scene->teams[1].course);
    fn_1_5378((M638SceneCameraView *)scene);
    camera = 0;
    Hu3DCameraViewportSet(lbl_1_rodata_10[camera], 0.0f, 0.0f,
        16.0f + scene->camera.splitX, 480.0f, 0.0f, 1.0f);
    camera++;
    Hu3DCameraViewportSet(lbl_1_rodata_10[camera], scene->camera.splitX - 16.0f,
        0.0f, 16.0f + (640.0f - scene->camera.splitX), 480.0f, 0.0f, 1.0f);
    camera = 0;
    Hu3DCameraScissorSet(lbl_1_rodata_10[camera], 0, 0, (u32)scene->camera.splitX, 480);
    camera++;
    Hu3DCameraScissorSet(lbl_1_rodata_10[camera], (u32)scene->camera.splitX, 0,
        (u32)(640.0f - scene->camera.splitX), 480);
    camera = 0;
    aspect = (0.9f * scene->camera.splitX) / 480.0f;
    if (aspect < 0.6f) {
        aspect = 0.6f;
    }
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[camera], 100.0f, 20.0f, 40000.0f, aspect);
    camera++;
    aspect = (0.9f * (640.0f - scene->camera.splitX)) / 480.0f;
    if (aspect < 0.6f) {
        aspect = 0.6f;
    }
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[camera], 100.0f, 20.0f, 40000.0f, aspect);
    fn_1_7754();
}

/* Initialize scene state, cached model slots, team links, and the event-choice table. */
void fn_1_1CD4(M638SceneView *scene)
{
    s32 i;
    s16 night;

    scene->teams[0].players[0] = NULL;
    scene->teams[0].players[1] = NULL;
    scene->teams[1].players[0] = NULL;
    scene->teams[1].players[1] = NULL;
    for (i = 0; i < 73; i++) {
        scene->models[i] = -1;
    }
    for (i = 0; i < 1; i++) {
        scene->extraModels[i] = -1;
    }
    night = GwMgNightF;
    scene->alternateLight = night == 1;
    scene->playState = 0;
    scene->introState = 0;
    scene->startState = 0;
    scene->resultState = 0;
    scene->finishState = 0;
    scene->winnerSequenceState = 0;
    scene->winner = -1;
    scene->exitTime = 0.0f;
    scene->newRecord = 0;
    scene->vector0.x = 0.0f;
    scene->vector0.y = 500.0f;
    scene->vector0.z = 1000.0f;
    scene->vector1.x = 0.0f;
    scene->vector1.y = 0.0f;
    scene->vector1.z = 1.0f;
    fn_1_4D68((M638SceneChoicesView *)scene);
}

/* Create each team's character, course actor, and scale-effect models. */
void fn_1_1E58(M638SceneView *scene)
{
    fn_1_309C(&scene->teams[0].actor, 0);
    fn_1_309C(&scene->teams[1].actor, 1);
    Hu3DModelHookSet(scene->teams[0].actor.modelB, lbl_1_data_4D0, scene->teams[0].actor.modelD);
    Hu3DModelHookSet(scene->teams[0].actor.modelB, lbl_1_data_4D7, scene->teams[0].actor.modelC);
    Hu3DModelHookSet(scene->teams[1].actor.modelB, lbl_1_data_4D0, scene->teams[1].actor.modelD);
    Hu3DModelHookSet(scene->teams[1].actor.modelB, lbl_1_data_4D7, scene->teams[1].actor.modelC);
    fn_1_1F18(&scene->teams[0], 0);
    fn_1_1F18(&scene->teams[1], 1);
    fn_1_5528(&scene->teams[0].scale, 0);
    fn_1_5528(&scene->teams[1].scale, 1);
}

/* Bind a team's characters and props to its camera and initialize its play state. */
void fn_1_1F18(M638TeamView *team, s32 camera)
{
    s16 model;
    M638SceneView *scene;
    s32 i;

    scene = lbl_1_bss_4->data;
    Hu3DModelHookSet(team->actor.modelA, lbl_1_data_4CC, team->players[0]->model);
    Hu3DModelHookSet(team->actor.modelA, lbl_1_data_4CE, team->players[1]->model);
    team->players[0]->attachedModel = team->actor.modelC;
    team->players[1]->attachedModel = team->actor.modelD;
    team->button = 0;
    team->count = 0;
    team->active = 0;
    team->blend = 0.0f;
    team->swingHoldFrames = 0;
    team->nextPropIndex = 0;
    team->variant = camera;
    team->camera = (u16)lbl_1_rodata_10[camera];
    team->unusedFloat = 0.0f;
    team->cameraBlend = 0.0f;
    for (i = 0; i < 2; i++) {
        if (scene->models[33] < 0) {
            model = scene->models[33] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_3A8[33], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[33]);
        }
        team->props[i].model = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 5);
        Hu3DModelPosSet(model, 0.0f, -120.0f, 125.0f);
        Hu3DModelRotSet(model, 75.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(model, 1U);
        Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
        Hu3DMotionSpeedSet(model, 0.75f);
        Hu3DModelScaleSet(model, 1.5f, 1.5f, 1.5f);
    }
    if (scene->models[34] < 0) {
        model = scene->models[34] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[34], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[34]);
    }
    team->props[2].model = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 5);
    team->props[2].pos.x = 0.0f;
    team->props[2].pos.y = -120.0f;
    team->props[2].pos.z = 400.0f;
    Hu3DModelPosSetV(model, &team->props[2].pos);
    Hu3DModelRotSet(model, 75.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, 1U);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    Hu3DMotionSpeedSet(model, 7.28f);
    team->sound0 = -1;
    team->sound1 = -1;
}

/* Scene update called from fn_1_19C0: animate team response, course events, and camera zoom. */
void fn_1_2288(M638TeamView *team)
{
    Point3d eventPosition;
    s32 sounds[2] = { M638_TEAM0_ACTIVITY_SFX_ID, M638_TEAM1_ACTIVITY_SFX_ID };
    M638Prop *eventProp;
    M638SceneView *sceneState;
    f32 targetActorRotation;
    f32 propBlend;
    f32 courseTime;
    f32 targetPropRotation;

    sceneState = lbl_1_bss_4->data;
    if (team->active != 0) {
        team->buttonIndicatorFrames += 1;
        if (team->buttonIndicatorFrames > 5) {
            espAttrSet(team->sprites[((team->button * 2) + 1) + team->buttonIndicatorIndex], 4U);
            team->buttonIndicatorFrames = 0;
            team->buttonIndicatorIndex ^= 1;
            espAttrReset(team->sprites[((team->button * 2) + 1) + team->buttonIndicatorIndex], 4U);
        }
        fn_1_2CDC(team);
    }
    if (team->actor.swingEnabled != 0) {
        if ((team->players[0]->buttonB != 0) || (team->players[1]->buttonB != 0)) {
            /* B cancels the held pose even if the other player is still pressing A. */
            team->swingHoldFrames = 0;
        } else if ((team->players[0]->buttonA != 0) || (team->players[1]->buttonA != 0)) {
            /* Refill the hold while A is down; after release, the countdown controls the pose. */
            team->swingHoldFrames = 10;
        } else if (team->swingHoldFrames > 0) {
            team->swingHoldFrames -= 1;
        }
        if (team->swingHoldFrames != 0) {
            team->actor.pos.x += 0.05f;
            if (team->actor.pos.x > 1.0f) {
                team->actor.pos.x = 1.0f;
            }
            targetActorRotation = -15.0f * team->actor.pos.x;
            team->actor.pos.y = 0.0f;
            if (team->actor.pos.z < 1.0f) {
                team->actor.pos.z += 0.1f;
            }
        } else {
            team->actor.pos.x *= 0.995f;
            if (team->actor.pos.x < 0.005f) {
                team->actor.pos.x = 0.0f;
            }
            team->actor.pos.y += 5.0f;
            while (team->actor.pos.y > 360.0f) {
                team->actor.pos.y -= 360.0f;
            }
            targetActorRotation = (f32) ((f64) (-15.0f * team->actor.pos.x) *
                                         cos((f64) ((team->actor.pos.y * acosf(-1.0f)) / 180.0f)));
            if (team->actor.pos.z > 0.5f) {
                team->actor.pos.z -= 0.1f;
            }
        }
        if (team->actor.rot.x > targetActorRotation) {
            team->actor.rot.x -= team->actor.pos.x;
            if (team->actor.rot.x < targetActorRotation) {
                team->actor.rot.x = targetActorRotation;
            }
        }
        if (team->actor.rot.x < targetActorRotation) {
            team->actor.rot.x += team->actor.pos.x;
            if (team->actor.rot.x > targetActorRotation) {
                team->actor.rot.x = targetActorRotation;
            }
        }
    }
    courseTime = Hu3DMotionTimeGet(team->course.model);
    if (courseTime >= lbl_1_data_330[team->actor.state + 1].courseTimeThreshold) {
        team->actor.state += 1;
    }
    targetPropRotation = team->actor.pos.z * lbl_1_data_330[team->actor.state].propRotationScale;
    if (team->actor.rot.z < targetPropRotation) {
        team->actor.rot.z += 0.05f;
    }
    if (team->actor.rot.z > targetPropRotation) {
        team->actor.rot.z -= 0.05f;
    }
    Hu3DMotionSpeedSet(team->course.alternateModel, team->blend);
    Hu3DMotionSpeedSet(team->course.model, team->blend);
    Hu3DMotionClusterTimeSet(team->course.alternateModel, 0, courseTime);
    if (team->blend > 0.2f) {
        propBlend = (team->blend - 0.2f) / 0.2f;
        if (propBlend > 1.0f) {
            propBlend = 1.0f;
        }
        Hu3DModelAttrReset(team->props[2].model, 1U);
    } else {
        propBlend = 0.0f;
        Hu3DModelAttrSet(team->props[2].model, 1U);
    }
    team->props[2].pos.z = 800.0f - (400.0f * propBlend);
    Hu3DModelPosSetV(team->props[2].model, &team->props[2].pos);
    fn_1_3518(&team->actor, (s32) team->active, team->blend);
    Hu3DModelRotSetV(team->actor.modelB, &team->actor.rot);
    Hu3DModelRotSetV(team->actor.modelA, &team->actor.rot);
    if (Hu3DMotionTimeGet(team->course.model) >= sceneState->eventTimes[team->count]) {
        espAttrSet(team->sprites[(team->button * 2) + 1], 4U);
        espAttrSet(team->sprites[(team->button * 2) + 2], 4U);
        team->count += 1;
        team->button = sceneState->choices[team->count];
        if (team->count < 10) {
            if (team->active != 0) {
                espAttrReset(team->sprites[((team->button * 2) + 1) + team->buttonIndicatorIndex],
                             4U);
            }
        if (fn_1_7660(fn_1_2ED0, &team->props[team->nextPropIndex]) != 0) {
            team->props[team->nextPropIndex].state = 0;
            team->nextPropIndex ^= 1;
            }
            eventProp = &team->course.pair[team->button][team->course.pairIndex[team->button]];
            if (fn_1_7660(fn_1_2F94, eventProp) != 0) {
                Hu3DModelObjPosGet(team->course.events[team->count - 1], lbl_1_data_4DE,
                                   &eventPosition);
                eventProp->pos.x =
                    (f32) (152.22000122070312 *
                           sin((f64) ((lbl_1_data_380[team->count - 1] * acosf(-1.0f)) / 180.0f)));
                eventProp->pos.y =
                    (f32) (-152.22000122070312 *
                           cos((f64) ((lbl_1_data_380[team->count - 1] * acosf(-1.0f)) / 180.0f)));
                eventProp->pos.z = eventPosition.z;
                Hu3DModelAttrSet(team->course.overlays[team->count - 1], 1U);
                Hu3DModelRotSet(eventProp->model, 0.0f, 0.0f, lbl_1_data_380[team->count - 1]);
                eventProp->state = 0;
                team->course.pairIndex[team->button] ^= 1;
                if (team->variant == 0) {
                    if (team->button != 0) {
                        HuAudFXPlay(M638_TEAM0_EVENT_BUTTON1_SFX_ID);
                    } else {
                        HuAudFXPlay(M638_TEAM0_EVENT_BUTTON0_SFX_ID);
                    }
                } else if (team->button != 0) {
                    HuAudFXPlay(M638_TEAM1_EVENT_BUTTON1_SFX_ID);
                } else {
                    HuAudFXPlay(M638_TEAM1_EVENT_BUTTON0_SFX_ID);
                }
            }
        }
        if (fn_1_7660(fn_1_368C, &team->actor) != 0) {
            team->actor.count = 0;
        }
    }
    if (team->cameraBlend < (team->blend - 0.1f)) {
        team->cameraBlend += 0.005f;
        if (team->cameraBlend > 0.4f) {
            team->cameraBlend = 0.4f;
        }
    } else if (team->cameraBlend > team->blend) {
        team->cameraBlend -= 0.01f;
        if (team->cameraBlend < 0.0f) {
            team->cameraBlend = 0.0f;
        }
    }
    Hu3DCameraPosSet((s32) team->camera, 0.0f, -80.0f, 400.0f + (100.0f * team->cameraBlend), 0.0f,
                     1.0f, 0.0f, 0.0f, -470.0f, -1200.0f);
    fn_1_56B4(&team->scale);
    if (team->blend > 0.0f) {
        if (team->sound0 == -1) {
            /* Start this team's loop once while its response blend is active. */
            team->sound0 = HuAudFXPlay((&sounds[0])[team->variant]);
            return;
        }
    } else if (team->sound0 > -1) {
        /* The sound handle is stopped and cleared when the team settles. */
        HuAudFXStop(team->sound0);
        team->sound0 = -1;
    }
}

/* Recalculate team effort from both players' input and start the shared scale effect on a B
 * press. */
void fn_1_2CDC(M638TeamView *team)
{
    f32 boostWeight;
    f32 responseWeight;
    s32 i;

    responseWeight = 0.0f;
    boostWeight = 0.0f;
    if (team->players[0]->buttonB != 0) {
        team->players[1]->impulse *= 0.8f;
    }
    if (team->players[1]->buttonB != 0) {
        team->players[0]->impulse *= 0.8f;
    }
    for (i = 0; i < 2; i++) {
        if (team->players[i]->impulse > 10.0f) {
            responseWeight += 0.5f;
            boostWeight += 0.05f * (team->players[i]->impulse - 10.0f);
        } else {
            responseWeight += 0.05f * team->players[i]->impulse;
        }
    }
    team->blend = (0.3f * responseWeight) + (0.1f * boostWeight);
    if (((team->players[0]->buttonB != 0) ||
         (team->players[1]->buttonB != 0)) &&
        (team->blend > 0.0f)) {
        fn_1_5760(&team->scale, team->blend / 0.4f);
        if (team->variant == 0) {
            HuAudFXPlay(M638_TEAM0_BOOST_SFX_ID);
            return;
        }
        HuAudFXPlay(M638_TEAM1_BOOST_SFX_ID);
    }
}

/* Queued prop callback: play its motion and report completion to the scene job list. */
s32 fn_1_2ED0(void *data)
{
    M638Prop *prop;
    int done;
    done = 0;
    prop = data;
    switch (prop->state) {
    case 0:
        Hu3DModelAttrReset(prop->model, 1U);
        Hu3DModelAttrReset(prop->model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (Hu3DMotionEndCheck(prop->model)) {
            Hu3DModelAttrSet(prop->model, 1U);
            Hu3DModelAttrSet(prop->model, HU3D_MOTATTR_PAUSE);
            done = 1;
        }
        break;
    }
    return done;
}

/* Queued course-event callback: play motion and its cluster from the event's stored position. */
s32 fn_1_2F94(void *data)
{
    M638Prop *prop;
    int done;
    done = 0;
    prop = data;
    switch (prop->state) {
    case 0:
        Hu3DModelAttrReset(prop->model, 1U);
        Hu3DModelAttrReset(prop->model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(prop->model, HU3D_CLUSTER_ATTR_PAUSE);
        Hu3DMotionTimeSet(prop->model, 0.0f);
        Hu3DMotionClusterTimeSet(prop->model, 0, 0.0f);
        Hu3DModelPosSetV(prop->model, &prop->pos);
        prop->state++;
        /* fallthrough */
    case 1:
        if (Hu3DMotionEndCheck(prop->model)) {
            Hu3DModelAttrSet(prop->model, 1U);
            Hu3DModelAttrSet(prop->model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(prop->model, HU3D_CLUSTER_ATTR_PAUSE);
            done = 1;
        }
        break;
    }
    return done;
}

/* Called by fn_1_1E58 for each team: create its podium body and hand models, both linked from asset
 * slot 31. */
void fn_1_309C(M638TeamActorView *actor, s32 camera)
{
    s16 model;
    M638ModelCacheView *scene;
    scene = lbl_1_bss_4->data;
    actor->rot.x = 0.0f;
    actor->rot.y = 0.0f;
    actor->rot.z = 0.0f;
    actor->pos.x = 0.0f;
    actor->pos.y = 0.0f;
    actor->pos.z = 0.0f;
    actor->state = 0;
    actor->blinkCount = 0;
    actor->count = 0;
    actor->blend = 0.0f;
    actor->swingEnabled = 1;
    if (scene->models[camera + 29] < 0) {
        model = scene->models[camera + 29] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_3A8[camera + 29], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[camera + 29]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSetV(model, &actor->rot);
    Hu3DModelAttrReset(model, 1U);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    Hu3DModelShadowMapObjSet(model, lbl_1_data_4E4);
    actor->modelB = model;
    if (scene->models[28] < 0) {
        model = scene->models[28] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[28], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[28]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSetV(model, &actor->rot);
    Hu3DModelAttrReset(model, 1U);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    Hu3DModelShadowSet(model);
    actor->modelA = model;
    if (scene->models[31] < 0) {
        model = scene->models[31] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[31], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[31]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, 1U);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    actor->modelC = model;
    if (scene->models[31] < 0) {
        model = scene->models[31] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[31], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[31]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, 1U);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    actor->modelD = model;
}

/* Called by fn_1_2288 each scene update to set the actor's input reaction or idle blink pose. */
void fn_1_3518(M638TeamActorView *actor, s32 teamActive, f32 blend)
{
    f32 poseWeight;

    if (teamActive != 0) {
        poseWeight = blend / 0.3f;
        if (poseWeight > 1.0f) {
            poseWeight = 1.0f;
        }
    } else if (blend > 0.0f) {
        actor->blinkCount = (s32) (actor->blinkCount + 1);
        actor->blinkCount = (s32) (actor->blinkCount % 47);
        if ((s32) actor->blinkCount < 3) {
            poseWeight = (f32) actor->blinkCount / 3.0f;
        } else if ((s32) actor->blinkCount < 31) {
            poseWeight = 1.0f;
        } else {
            poseWeight = 0.0f;
        }
    } else {
        poseWeight = 0.0f;
    }
    Hu3DMotionTimeSet(actor->modelB, 16.0f * poseWeight);
}

/* Queued from fn_1_2288: bounce both podium models for twenty callback updates. */
s32 fn_1_368C(void *data)
{
    M638BounceView *bounce;
    f32 bounceAngle;
    f32 bounceHeight;
    int done;
    done = 0;
    bounce = data;
    bounce->count++;
    bounceAngle = 36.0f * bounce->count;
    if (bounceAngle > 180.0f) {
        bounceAngle -= 180.0f;
    }
    {
        f32 pi = acos(-1.0);
        bounceHeight = 4.0 * sin((bounceAngle * pi) / 180.0f);
    }
    if (bounce->count >= 20) {
        bounceHeight = 0.0f;
        done = 1;
    }
    Hu3DModelPosSet(bounce->modelB, 0.0f, bounceHeight, 0.0f);
    Hu3DModelPosSet(bounce->modelA, 0.0f, bounceHeight, 0.0f);
    return done;
}

/* Called by the sequence init hook fn_1_1EC: load the reflection map and both team courses. */
void fn_1_3804(void *sceneData)
{
    Hu3DReflectMapSet(HuDataSelHeapReadNum(DATANUM(DATA_m638, 52), HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_386C(&((M638SceneView *)sceneData)->teams[0].course, 0);
    fn_1_386C(&((M638SceneView *)sceneData)->teams[1].course, 1);
    fn_1_4948(&((M638SceneView *)sceneData)->teams[0].course);
    fn_1_4948(&((M638SceneView *)sceneData)->teams[1].course);
}

/* Called twice by fn_1_3804, once per team camera, to load course models and event props. */
void fn_1_386C(M638CourseView *course, s32 camera)
{
    s16 model;
    M638SceneCourseView *scene;
    s32 i;
    s32 j;
    s32 variant;

    scene = lbl_1_bss_4->data;
    if (scene->models[21] < 0) {
        model = scene->models[21] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[21], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[21]);
    }
    course->model = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelAttrReset(model, 1U);
    if (scene->alternateLight != 0) {
        if (scene->models[70] < 0) {
            model = scene->models[70] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_3A8[70], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[70]);
        }
    } else if (scene->models[24] < 0) {
        model = scene->models[24] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[24], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[24]);
    }
    course->alternateModel = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelAttrReset(model, 1U);
    Hu3DMotionClusterTimeSet(model, 0, 0.0f);
    Hu3DModelAttrReset(model, HU3D_CLUSTER_ATTR_PAUSE);
    i = 0;
    while (i < 4) {
        if (scene->alternateLight != 0) {
            if (scene->models[i + 49] < 0) {
                model = scene->models[i + 49] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(lbl_1_data_3A8[i + 49], HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model = Hu3DModelLink(scene->models[i + 49]);
            }
        } else if (scene->models[i] < 0) {
            model = scene->models[i] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[i], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[i]);
        }
        course->segments[i] = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(model, 1U);
        Hu3DModelLayerSet(model, 1);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
        Hu3DMotionSpeedSet(model, 0.2f);
        i += 1;
    }
    while (i < 20) {
        if (scene->alternateLight != 0) {
            if (scene->models[i + 49] < 0) {
                model = scene->models[i + 49] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(lbl_1_data_3A8[i + 49], HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model = Hu3DModelLink(scene->models[i + 49]);
            }
        } else if (scene->models[i] < 0) {
            model = scene->models[i] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[i], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[i]);
        }
        course->segments[i] = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(model, 1U);
        Hu3DModelLayerSet(model, 1);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
        if (i == 4) {
            Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(model, 0.2f);
        }
        if (i == 5) {
            Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(model, 0.5f);
        }
        if (i == 16) {
            Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(model, 0.3f);
        }
        if (i == 17) {
            Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(model, 0.3f);
        }
        i += 1;
    }
    if (scene->alternateLight != 0) {
        if (scene->models[69] < 0) {
            model = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_3A8[69], HU_MEMNUM_OVL, HEAP_MODEL));
            scene->models[69] = model;
        } else {
            model = Hu3DModelLink(scene->models[69]);
        }
    } else if (scene->models[20] < 0) {
        model = scene->models[20] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[20], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->models[20]);
    }
    course->lastModel = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
    Hu3DModelAttrSet(model, 1U);
    j = 1;
    variant = 0;
    i = 0;
    while (i < 40) {
        if (scene->models[variant + 22] < 0) {
            model = scene->models[variant + 22] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_3A8[variant + 22], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[variant + 22]);
        }
        course->connectors[i] = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 1);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(model, 1U);
        Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
        if (strcmp(lbl_1_data_290[i], lbl_1_data_230) != 0) {
            if (j < 10) {
                variant = scene->choices[j];
                if (scene->models[variant + 25] < 0) {
                    model = scene->models[variant + 25] = Hu3DModelCreate(HuDataSelHeapReadNum(
                        lbl_1_data_3A8[variant + 25], HU_MEMNUM_OVL, HEAP_MODEL));
                } else {
                    model = Hu3DModelLink(scene->models[variant + 25]);
                }
                course->events[j - 1] = model;
                Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
                Hu3DModelLayerSet(model, 1);
                Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset(model, 1U);
                Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
                scene->eventTimes[j - 1] = 15.0f + (15.0f * (f32) i);
                if (scene->models[variant + 39] < 0) {
                    model = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[variant + 39],
                                                                 HU_MEMNUM_OVL, HEAP_MODEL));
                    scene->models[variant + 39] = model;
                } else {
                    model = Hu3DModelLink(scene->models[variant + 39]);
                }
                course->overlays[j - 1] = model;
                Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
                Hu3DModelLayerSet(model, 5);
                Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset(model, 1U);
                Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
            } else {
                variant = scene->choices[j];
                if (scene->models[27] < 0) {
                    model = scene->models[27] = Hu3DModelCreate(
                        HuDataSelHeapReadNum(lbl_1_data_3A8[27], HU_MEMNUM_OVL, HEAP_MODEL));
                } else {
                    model = Hu3DModelLink(scene->models[27]);
                }
                course->events[j - 1] = model;
                Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
                Hu3DModelLayerSet(model, 1);
                Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset(model, 1U);
                Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
                scene->eventTimes[j - 1] = 15.0f + (15.0f * (f32) i);
            }
            j += 1;
        }
        i += 1;
    }
    scene->endTime = 601.0f;
    if (scene->alternateLight != 0) {
        i = 0;
        while (i < 5) {
            if (scene->models[71] < 0) {
                model = scene->models[71] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(lbl_1_data_3A8[71], HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model = Hu3DModelLink(scene->models[71]);
            }
            course->nightModels[i] = model;
            Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            Hu3DModelLayerSet(model, 1);
            Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(model, 1U);
            Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
            Hu3DMotionSpeedSet(model, 0.2f);
            i += 1;
        }
    }
    Hu3DModelAttrSet(course->events[0], 1U);
    fn_1_5814(&course->props, camera);
    course->current = 0;
    course->pairIndex[0] = 0;
    course->pairIndex[1] = 0;
    i = 0;
    while (i < 2) {
        j = 0;
        while (j < 2) {
            if (scene->models[41 + camera * 4 + i * 2 + j] < 0) {
                model = scene->models[41 + camera * 4 + i * 2 + j] =
                    Hu3DModelCreate(HuDataSelHeapReadNum(
                        lbl_1_data_3A8[41 + camera * 4 + i * 2 + j], HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model = Hu3DModelLink(scene->models[41 + camera * 4 + i * 2 + j]);
            }
            course->pair[i][j].model = model;
            Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(model, HU3D_CLUSTER_ATTR_PAUSE);
            Hu3DModelLayerSet(model, 5);
            Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrSet(model, 1U);
            Hu3DModelCameraSet(model, (u16) lbl_1_rodata_10[camera]);
            j += 1;
        }
        i += 1;
    }
}

/* Called by fn_1_3804 after course loading to attach segment, event, and night-model hooks. */
void fn_1_4948(M638CourseView *course)
{
    M638SceneCourseView *scene;
    s32 i;
    s32 eventCount;

    scene = lbl_1_bss_4->data;
    eventCount = 1;
    for (i = 0; i < 40; i++) {
        Hu3DModelHookSet(course->segments[i / 2], lbl_1_data_190[i], course->connectors[i]);
        if (strcmp(lbl_1_data_290[i], lbl_1_data_230) != 0) {
            Hu3DModelHookSet(course->segments[i / 2], lbl_1_data_290[i],
                             course->events[eventCount - 1]);
            if (eventCount < 10) {
                Hu3DModelHookSet(course->events[eventCount - 1], lbl_1_data_4DE,
                                 course->overlays[eventCount - 1]);
            }
            eventCount += 1;
        }
    }
    Hu3DModelHookSet(course->model, lbl_1_data_4EB, course->segments[0]);
    Hu3DModelHookSet(course->model, lbl_1_data_4EF, course->segments[1]);
    Hu3DModelHookSet(course->model, lbl_1_data_4F3, course->segments[2]);
    Hu3DModelHookSet(course->model, lbl_1_data_4F7, course->segments[3]);
    if (scene->alternateLight != 0) {
        Hu3DModelHookSet(course->lastModel, lbl_1_data_4FB, course->nightModels[0]);
        Hu3DModelHookSet(course->lastModel, lbl_1_data_502, course->nightModels[1]);
        Hu3DModelHookSet(course->lastModel, lbl_1_data_509, course->nightModels[2]);
        Hu3DModelHookSet(course->lastModel, lbl_1_data_510, course->nightModels[3]);
        Hu3DModelHookSet(course->lastModel, lbl_1_data_517, course->nightModels[4]);
    }
}

/* Called each scene update by fn_1_19C0 to advance a team's course and prop schedules. */
void fn_1_4B4C(M638CourseView *course)
{
    char *hooks[4] = {lbl_1_data_4EB, lbl_1_data_4EF, lbl_1_data_4F3, lbl_1_data_4F7};
    f32 courseMotionTime;
    int segmentHookIndex;
    courseMotionTime = Hu3DMotionTimeGet(course->model);
    if (courseMotionTime > 60.0f + 30.0f * course->current) {
        segmentHookIndex = course->current % 4;
        Hu3DModelHookObjReset(course->model, hooks[segmentHookIndex]);
        Hu3DModelAttrSet(course->segments[course->current], 1U);
        if (course->current + 4 < 20) {
            Hu3DModelHookSet(course->model, hooks[segmentHookIndex],
                             course->segments[course->current + 4]);
            Hu3DModelAttrReset(course->segments[course->current + 4], 1U);
        }
        course->current++;
    }
    if (course->current == 0 && courseMotionTime >= 30.0f &&
        (Hu3DModelAttrGet(course->events[0]) & 1)) {
        Hu3DModelAttrReset(course->events[0], 1U);
    }
    if (courseMotionTime >= 536.0f) {
        if (Hu3DModelAttrGet(course->lastModel) & 1) {
            Hu3DModelAttrReset(course->lastModel, 1U);
            Hu3DModelAttrReset(course->lastModel, HU3D_MOTATTR_PAUSE);
        } else {
            Hu3DMotionTimeSet(course->lastModel, courseMotionTime - 536.0f);
        }
    }
    fn_1_5D0C(&course->props, courseMotionTime);
}

/* Called by fn_1_1CD4 during scene initialization to build the event-choice sequence. */
void fn_1_4D68(M638SceneChoicesView *scene)
{
    int i;
    int repeat;
    repeat = 0;
    scene->choices[0] = 0;
    scene->choices[1] = 0;
    for (i = 2; i < 10; i++) {
        if (repeat) {
            /* A repeated random result is followed by the opposite event. */
            scene->choices[i] = scene->choices[i - 1] ^ 1;
            repeat = 0;
        } else {
            scene->choices[i] = frandmod(2);
            repeat = scene->choices[i] == scene->choices[i - 1];
        }
    }
    scene->choices[10] = scene->choices[9];
}

/* Called by sequence init fn_1_1EC to create shared result and per-team event sprites. */
void fn_1_4E48(M638SceneView *scene)
{
    s16 model;

    model = espEntry(DATANUM(DATA_m638, 49), 111, 0);
    scene->sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 50), 110, 0);
    scene->sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 51), 110, 0);
    scene->sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);

    model = espEntry(DATANUM(DATA_m638, 81), 111, 0);
    scene->teams[0].sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 77), 110, 0);
    scene->teams[0].sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 78), 110, 0);
    scene->teams[0].sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 79), 110, 0);
    scene->teams[0].sprites[3] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 80), 110, 0);
    scene->teams[0].sprites[4] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    scene->teams[0].buttonIndicatorFrames = 0;
    scene->teams[0].buttonIndicatorIndex = 0;

    model = espEntry(DATANUM(DATA_m638, 81), 111, 0);
    scene->teams[1].sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 77), 110, 0);
    scene->teams[1].sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 78), 110, 0);
    scene->teams[1].sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 79), 110, 0);
    scene->teams[1].sprites[3] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(DATA_m638, 80), 110, 0);
    scene->teams[1].sprites[4] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    scene->teams[1].buttonIndicatorFrames = 0;
    scene->teams[1].buttonIndicatorIndex = 0;
}

/* Called by fn_1_19C0 each scene update to move status sprites with the course-view motion. */
void fn_1_5378(M638SceneCameraView *scene)
{
    f32 viewProgress;
    f32 y;
    viewProgress = (Hu3DMotionTimeGet(scene->team0Model) - 120.0f) / 435.0f;
    if (viewProgress < 0.0f) {
        viewProgress = 0.0f;
    }
    if (viewProgress > 1.0f) {
        viewProgress = 1.0f;
    }
    y = 400.0f * (1.0f - viewProgress) + 120.0f * viewProgress;
    espPosSet(scene->sprite0, 276.0f, y);
    viewProgress = (Hu3DMotionTimeGet(scene->team1Model) - 120.0f) / 435.0f;
    if (viewProgress < 0.0f) {
        viewProgress = 0.0f;
    }
    if (viewProgress > 1.0f) {
        viewProgress = 1.0f;
    }
    y = 400.0f * (1.0f - viewProgress) + 120.0f * viewProgress;
    espPosSet(scene->sprite1, 300.0f, y);
}

/* Called by fn_1_1E58 for each team to create its response-scale effect model. */
void fn_1_5528(M638ScaleWork *scale, s32 camera)
{
    s16 model;
    M638ModelCacheView *modelCache;
    modelCache = lbl_1_bss_4->data;
    scale->active = 0;
    scale->responseScale = 1.0f;
    if (modelCache->models[32] < 0) {
        model = modelCache->models[32] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3A8[32], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(modelCache->models[32]);
    }
    scale->model = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
    Hu3DModelAttrSet(model, 1U);
    Hu3DMotionSpeedSet(model, 9.74f);
    Hu3DModelScaleSet(model, 1.75f, 1.75f, 1.75f);
}

/* Called by fn_1_2288 each scene update to apply the team's active response-scale animation. */
void fn_1_56B4(M638ScaleWork *scale)
{
    f32 modelScale;

    if (scale->active != 0) {
        modelScale = 1.0f + ((scale->responseScale * Hu3DMotionTimeGet(scale->model)) / 60.0f);
        Hu3DModelScaleSet(scale->model, modelScale, modelScale, modelScale);
        if (Hu3DMotionEndCheck(scale->model) != 0) {
            Hu3DModelAttrSet(scale->model, 1U);
            scale->active = 0;
        }
    }
}

/* Called by fn_1_2CDC on a B press to start the team's response-scale animation. */
void fn_1_5760(M638ScaleWork *scale, f32 responseAmount)
{
    if (scale->active == 0) {
        scale->responseScale = 1.5f * responseAmount;
        Hu3DModelScaleSet(scale->model, 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrReset(scale->model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(scale->model, 1U);
        Hu3DMotionTimeSet(scale->model, 0.0f);
        scale->active = 1;
    }
}

/* Called by fn_1_386C to create the small, middle, and night prop groups for one camera. */
void fn_1_5814(M638PropGroups *propGroups, s32 camera)
{
    int modelCacheGroup[6] = { 0, 1, 1, 0, 0, 1 };
    int i;
    s16 model;
    M638ScenePropsView *scene;
    scene = lbl_1_bss_4->data;
    for (i = 0; i < 6; i++) {
        if (scene->models[modelCacheGroup[i] + 35] < 0) {
            model = scene->models[modelCacheGroup[i] + 35] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_3A8[modelCacheGroup[i] + 35], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[modelCacheGroup[i] + 35]);
        }
        propGroups->small[i].model = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 5);
        propGroups->small[i].pos.x = 0.0f;
        propGroups->small[i].pos.y = -100.0f;
        propGroups->small[i].pos.z = 100.0f;
        Hu3DModelPosSetV(model, &propGroups->small[i].pos);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
        Hu3DModelAttrSet(model, 1U);
        propGroups->small[i].state = 2;
        propGroups->small[i].moving = 0;
    }
    for (i = 0; i < 8; i++) {
        if (scene->models[37] < 0) {
            model = scene->models[37] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_3A8[37], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->models[37]);
        }
        propGroups->middle[i].model = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 5);
        propGroups->middle[i].pos.x = 0.0f;
        propGroups->middle[i].pos.y = -150.0f;
        propGroups->middle[i].pos.z = 100.0f;
        Hu3DModelPosSetV(model, &propGroups->middle[i].pos);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
        Hu3DModelAttrSet(model, 1U);
        propGroups->middle[i].state = 2;
        propGroups->middle[i].moving = 0;
    }
    if (scene->alternateLight) {
        for (i = 0; i < 4; i++) {
            if (scene->models[38] < 0) {
                model = scene->models[38] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(lbl_1_data_3A8[38], HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model = Hu3DModelLink(scene->models[38]);
            }
            propGroups->night[i].model = model;
            Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
            Hu3DModelLayerSet(model, 5);
            propGroups->night[i].pos.x = 0.0f;
            propGroups->night[i].pos.y = -150.0f;
            propGroups->night[i].pos.z = 100.0f;
            Hu3DModelPosSetV(model, &propGroups->night[i].pos);
            Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
            Hu3DModelCameraSet(model, (u16)lbl_1_rodata_10[camera]);
            Hu3DModelAttrSet(model, 1U);
            propGroups->night[i].state = 2;
            propGroups->night[i].moving = 0;
        }
    }
    propGroups->count = 0;
    if (scene->alternateLight) {
        propGroups->state = 0;
    } else {
        propGroups->state = 3;
    }
}

/* Called by fn_1_4B4C each scene update to release props at course motion thresholds. */
void fn_1_5D0C(M638PropGroups *group, f32 time)
{
    int i;
    switch (group->state) {
    case 0:
        if (time >= 180.0f) {
            group->state++;
        }
        break;
    case 1:
        group->count = 150;
        group->state++;
        /* fallthrough */
    case 2:
        group->count++;
        if (group->count >= 150) {
            for (i = 0; i < 4; i++) {
                if (group->night[i].state == 2 && fn_1_7660(fn_1_6098, &group->night[i]) != 0) {
                    group->night[i].state = 0;
                    Hu3DMotionSpeedSet(group->night[i].model, 0.11f);
                    group->count = 0;
                    break;
                }
            }
        }
        if (time >= 340.0f) {
            group->state++;
        }
        break;
    case 3:
        if (time >= 345.0f) {
            group->state++;
        }
        break;
    case 4:
        group->count = 60;
        group->state++;
        /* fallthrough */
    case 5:
        group->count++;
        if (group->count > 30) {
            for (i = 0; i < 6; i++) {
                if (group->small[i].state == 2 && fn_1_7660(fn_1_6098, &group->small[i]) != 0) {
                    group->small[i].state = 0;
                    Hu3DMotionSpeedSet(group->small[i].model, 0.68f);
                    group->count = 0;
                    break;
                }
            }
        }
        if (time >= 455.0f) {
            group->state++;
        }
        break;
    case 6:
        if (time >= 460.0f) {
            group->state++;
        }
        break;
    case 7:
        group->count = 90;
        group->state++;
        /* fallthrough */
    case 8:
        group->count++;
        if (group->count > 45) {
            for (i = 0; i < 8; i++) {
                if (group->middle[i].state == 2 && fn_1_7660(fn_1_61D0, &group->middle[i]) != 0) {
                    group->middle[i].state = 0;
                    Hu3DMotionSpeedSet(group->middle[i].model, 0.34f);
                    group->count = 0;
                    break;
                }
            }
        }
        if (time >= 555.0f) {
            for (i = 0; i < 8; i++) {
                if (group->middle[i].state != 2) {
                    group->middle[i].moving = 1;
                }
            }
            group->state++;
        }
        break;
    }
}

/* Queued callback: reveal and position a small or night course prop. */
s32 fn_1_6098(void *data)
{
    M638Prop *prop;
    int done;
    done = 0;
    prop = data;
    switch (prop->state) {
    case 0:
        prop->pos.x = 100.0f * ((frandmod(10001) / 10000.0f) - 0.5f);
        Hu3DModelPosSetV(prop->model, &prop->pos);
        Hu3DModelAttrReset(prop->model, 1U);
        Hu3DModelAttrReset(prop->model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (Hu3DMotionEndCheck(prop->model)) {
            Hu3DModelAttrSet(prop->model, 1U);
            Hu3DModelAttrSet(prop->model, HU3D_MOTATTR_PAUSE);
            prop->state = 2;
            done = 1;
        }
        break;
    }
    return done;
}

/* Queued callback: move a middle course prop and finish its motion. */
s32 fn_1_61D0(void *data)
{
    M638Prop *prop;
    int done;
    done = 0;
    prop = data;
    switch (prop->state) {
    case 0:
        prop->pos.x = 10.0f + 100.0f * ((frandmod(10001) / 10000.0f) - 0.5f);
        Hu3DModelPosSetV(prop->model, &prop->pos);
        Hu3DModelRotSet(prop->model, -10.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(prop->model, 1U);
        Hu3DModelAttrReset(prop->model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (prop->moving) {
            prop->pos.z += 1.0f;
        }
        Hu3DModelPosSetV(prop->model, &prop->pos);
        if (Hu3DMotionEndCheck(prop->model)) {
            Hu3DModelAttrSet(prop->model, 1U);
            Hu3DModelAttrSet(prop->model, HU3D_MOTATTR_PAUSE);
            prop->state = 2;
            done = 1;
        }
        break;
    }
    return done;
}

/* Animate the post-match split from both teams to the winner's camera. */
s32 fn_1_6374(void *data)
{
    M638SceneSplitView *scene;
    f32 time;
    scene = data;
    scene->camera.elapsed += 1.0f;
    time = scene->camera.elapsed / 60.0f;
    if (time > 1.0f) {
        time = 1.0f;
    }
    if (scene->side == 0) {
        scene->camera.splitX = 320.0f * (1.0f - time) + 640.0f * time;
    } else {
        scene->camera.splitX = 320.0f * (1.0f - time);
    }
    if (scene->camera.elapsed >= 60.0f) {
        return 1;
    }
    return 0;
}

/* Advance the winning team's finish animation and return when its motion ends. */
s32 fn_1_647C(void *data)
{
    M638TeamFinishView *team;
    f32 weight;
    f32 motionTime;
    int done;
    done = 0;
    team = data;
    switch (team->state) {
    case 0:
        motionTime = Hu3DMotionTimeGet(team->model);
        weight = (570.0f - motionTime) / 15.0f;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        if (weight < 0.0f) {
            weight = 0.0f;
        }
        team->blend = 0.3f * (1.0f - weight) + team->base * weight;
        team->output = team->source * weight;
        if (motionTime >= 570.0f) {
            if (team->variant == 0) {
                HuAudFXPlay(M638_TEAM0_FINISH_MOTION_SFX_ID);
            } else {
                HuAudFXPlay(M638_TEAM1_FINISH_MOTION_SFX_ID);
            }
            team->output = 0.0f;
            team->state++;
        }
        break;
    case 1:
        motionTime = Hu3DMotionTimeGet(team->model);
        weight = (600.0f - motionTime) / 30.0f;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        if (weight < 0.0f) {
            weight = 0.0f;
        }
        team->blend = 0.3f * weight;
        if (Hu3DMotionEndCheck(team->model)) {
            if (team->variant == 0) {
                HuAudFXPlay(M638_TEAM0_FINISH_END_SFX_ID);
            } else {
                HuAudFXPlay(M638_TEAM1_FINISH_END_SFX_ID);
            }
            team->state = 2;
            team->blend = 0.0f;
            done = 1;
        }
        break;
    }
    return done;
}

/* Fade the losing team's blend to zero over thirty updates, then report completion. */
s32 fn_1_66C0(void *data)
{
    M638TeamFinishView *team;
    f32 time;
    team = data;
    team->elapsed += 1.0f;
    time = team->elapsed / 30.0f;
    if (time > 1.0f) {
        time = 1.0f;
    }
    team->blend = team->base * (1.0f - time);
    if (team->elapsed >= 30.0f) {
        team->state = 2;
        team->blend = 0.0f;
        return 1;
    }
    return 0;
}

/* After ninety updates, check for a new timer record and update its display if one qualifies. */
s32 fn_1_6790(void *data)
{
    M638TimerWatch *watch;
    watch = data;
    watch->count++;
    if (watch->count >= 90) {
        MgTimerRecordSet(watch->timer, -1);
        return 1;
    }
    return 0;
}
