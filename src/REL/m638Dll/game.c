/* Gondola Glide scene setup, split-screen play, course events, and results. */
#include <math.h>
#include "REL/m638Dll.h"

#define M638_OVERLAY_MEMORY_ID 0x10000000
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
    DATANUM(0x610000, 0), DATANUM(0x610000, 1), DATANUM(0x610000, 2), DATANUM(0x610000, 3),
    DATANUM(0x610000, 4), DATANUM(0x610000, 5), DATANUM(0x610000, 6), DATANUM(0x610000, 7),
    DATANUM(0x610000, 8), DATANUM(0x610000, 9), DATANUM(0x610000, 10), DATANUM(0x610000, 11),
    DATANUM(0x610000, 12), DATANUM(0x610000, 13), DATANUM(0x610000, 14), DATANUM(0x610000, 15),
    DATANUM(0x610000, 16), DATANUM(0x610000, 17), DATANUM(0x610000, 18), DATANUM(0x610000, 19),
    DATANUM(0x610000, 20), DATANUM(0x610000, 21), DATANUM(0x610000, 22), DATANUM(0x610000, 23),
    DATANUM(0x610000, 24), DATANUM(0x610000, 25), DATANUM(0x610000, 26), DATANUM(0x610000, 27),
    DATANUM(0x610000, 28), DATANUM(0x610000, 29), DATANUM(0x610000, 30), DATANUM(0x610000, 31),
    DATANUM(0x610000, 32), DATANUM(0x610000, 33), DATANUM(0x610000, 34), DATANUM(0x610000, 35),
    DATANUM(0x610000, 36), DATANUM(0x610000, 37), DATANUM(0x610000, 38), DATANUM(0x610000, 39),
    DATANUM(0x610000, 40), DATANUM(0x610000, 41), DATANUM(0x610000, 42), DATANUM(0x610000, 43),
    DATANUM(0x610000, 44), DATANUM(0x610000, 45), DATANUM(0x610000, 46), DATANUM(0x610000, 47),
    DATANUM(0x610000, 48), DATANUM(0x610000, 53), DATANUM(0x610000, 54), DATANUM(0x610000, 55),
    DATANUM(0x610000, 56), DATANUM(0x610000, 57), DATANUM(0x610000, 58), DATANUM(0x610000, 59),
    DATANUM(0x610000, 60), DATANUM(0x610000, 61), DATANUM(0x610000, 62), DATANUM(0x610000, 63),
    DATANUM(0x610000, 64), DATANUM(0x610000, 65), DATANUM(0x610000, 66), DATANUM(0x610000, 67),
    DATANUM(0x610000, 68), DATANUM(0x610000, 69), DATANUM(0x610000, 70), DATANUM(0x610000, 71),
    DATANUM(0x610000, 72), DATANUM(0x610000, 73), DATANUM(0x610000, 74), DATANUM(0x610000, 75),
    DATANUM(0x610000, 76),
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

    lbl_1_bss_C = fn_8003A8BC(10, 4096);
    fn_80069F1C(lbl_1_bss_C);
    obj = fn_8003AB38(lbl_1_bss_C, 101, 0, 0, -1, fn_1_1988);
    lbl_1_bss_4 = obj;
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M638SceneView), M638_OVERLAY_MEMORY_ID);
    scene = obj->data;
    for (player = 0; player < 4; player++) {
        obj = fn_8003AB38(lbl_1_bss_C, 100, 1, 5, 0, fn_1_67EC);
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
    fn_80070788(&lbl_1_data_0);
}

/* Sequence init hook: create the timer, read the record, and prepare both courses. */
void fn_1_1EC(s16 mode, s16 frameNo)
{
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    scene->record = fn_8003C860(3);
    if ((s32) scene->record == 0) {
        scene->record = 18000U;
    }
    scene->timer = MgTimerCreate(1);
    MgTimerParamSet(scene->timer, 0, 18000, (s32) scene->record);
    MgTimerRecordDispOn(scene->timer);
    fn_1_3804(scene);
    fn_1_1E58(scene);
    fn_1_4E48(scene);
    fn_80071A9C();
}

/* Sequence fade-in hook: animate the split view before the play phase begins. */
void fn_1_284(s16 mode, s16 frameNo)
{
    f32 splitProgress;
    M638SceneView *scene;

    scene = lbl_1_bss_4->data;
    switch (scene->introState) {
    case 0:
        fn_80071B08();
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
            fn_80071A9C();
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
            motionTimes[0] = fn_8002D4E0(scene->teams[0].course.model);
            motionTimes[1] = fn_8002D4E0(scene->teams[1].course.model);
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
            fn_80071A9C();
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
            fn_80071C18(-1, -1, -1, -1);
            fn_1_7660(fn_1_66C0, scene);
            fn_1_7660(fn_1_66C0, &scene->teams[1]);
        } else {
            espAttrSet(scene->sprites[0], 4U);
            espAttrSet(scene->sprites[1], 4U);
            espAttrSet(scene->sprites[2], 4U);
            fn_80071C18(scene->teams[scene->winner].players[0]->charNo,
                           scene->teams[scene->winner].players[1]->charNo, -1, -1);
            scene->time = MgTimerValueGet(scene->timer);
            if ((scene->time < scene->record) &&
                (fn_8006A124(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) &&
                ((scene->teams[scene->winner].players[0]->isCom == 0) ||
                 (scene->teams[scene->winner].players[1]->isCom == 0))) {
                scene->newRecord = 1;
                fn_80071BAC(scene->time);
                fn_8003C80C(3, (u32) scene->time);
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
        fn_80071B08();
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
                fn_80071A9C();
            } else if (scene->exitTime > durations[scene->newRecord]) {
                fn_80071A9C();
            } else {
                fn_80071B30();
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
            fn_8005C2C8(scene->teams[0].players[0]->charNo,
                               scene->teams[0].players[0]->motions[3],
                               0.0f, 5.0f, 0U);
            fn_8005C2C8(scene->teams[0].players[1]->charNo,
                               scene->teams[0].players[1]->motions[3],
                               0.0f, 5.0f, 0U);
            fn_8005C2C8(scene->teams[1].players[0]->charNo,
                               scene->teams[1].players[0]->motions[3],
                               0.0f, 5.0f, 0U);
            fn_8005C2C8(scene->teams[1].players[1]->charNo,
                               scene->teams[1].players[1]->motions[3],
                               0.0f, 5.0f, 0U);
            fn_800611E4(scene->teams[0].players[0]->charNo,
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
            fn_800275B4(scene->teams[0].actor.modelA,
                                  lbl_1_data_4CC);
            fn_800275B4(scene->teams[0].actor.modelA,
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
            fn_800275B4(scene->teams[1].actor.modelA,
                                  lbl_1_data_4CC);
            fn_800275B4(scene->teams[1].actor.modelA,
                                  lbl_1_data_4CE);
            fn_1_7554(scene->teams[1].players[0]);
            fn_1_7554(scene->teams[1].players[1]);
        } else {
            fn_8005C2C8(scene->teams[scene->winner].players[0]->charNo,
                               scene->teams[scene->winner].players[0]->motions[2],
                               0.0f, 5.0f, 0U);
            fn_8005C2C8(scene->teams[scene->winner].players[1]->charNo,
                               scene->teams[scene->winner].players[1]->motions[2],
                               0.0f, 5.0f, 0U);
            firstWinnerPlayerNo = scene->teams[scene->winner].players[0]->playerNo;
            if (fn_8006A124(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) {
                lbl_80265750[firstWinnerPlayerNo].mgCoinBonus = 10;
            }
            secondWinnerPlayerNo = scene->teams[scene->winner].players[1]->playerNo;
            if (fn_8006A124(M638_FLAG_SKIP_WINNER_COIN_BONUS) == 0) {
                lbl_80265750[secondWinnerPlayerNo].mgCoinBonus = 10;
            }
            Hu3DModelObjPosGet(scene->teams[scene->winner].actor.modelA,
                               lbl_1_data_4CC,
                               &scene->teams[scene->winner].players[0]->pos);
            Hu3DModelObjPosGet(scene->teams[scene->winner].actor.modelA,
                               lbl_1_data_4CE,
                               &scene->teams[scene->winner].players[1]->pos);
            scene->teams[scene->winner].players[0]->rot.y = 0.0f;
            scene->teams[scene->winner].players[1]->rot.y = 0.0f;
            fn_800275B4(scene->teams[scene->winner].actor.modelA,
                                  lbl_1_data_4CC);
            fn_800275B4(scene->teams[scene->winner].actor.modelA,
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
    fn_8002A3B0(100.0f, 20.0f, 40000.0f, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    fn_8002A7D8(0.5f, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    fn_8002A674(&shadowPosition, &shadowSide, &scene->lightTarget,
                          (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
    if (scene->alternateLight != 0) {
        fn_8002AA3C(15U, 15U, 95U, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
        return;
    }
    fn_8002AA3C(63U, 63U, 63U, (s16) (lbl_1_rodata_10[0] | lbl_1_rodata_10[1]));
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
    night = lbl_802C0278;
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
    fn_8002741C(scene->teams[0].actor.modelB, lbl_1_data_4D0, scene->teams[0].actor.modelD);
    fn_8002741C(scene->teams[0].actor.modelB, lbl_1_data_4D7, scene->teams[0].actor.modelC);
    fn_8002741C(scene->teams[1].actor.modelB, lbl_1_data_4D0, scene->teams[1].actor.modelD);
    fn_8002741C(scene->teams[1].actor.modelB, lbl_1_data_4D7, scene->teams[1].actor.modelC);
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
    fn_8002741C(team->actor.modelA, lbl_1_data_4CC, team->players[0]->model);
    fn_8002741C(team->actor.modelA, lbl_1_data_4CE, team->players[1]->model);
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
            model = scene->models[33] = fn_80025A04(
                fn_80008DD0(lbl_1_data_3A8[33], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[33]);
        }
        team->props[i].model = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_80026B8C(model, 5);
        fn_80026760(model, 0.0f, -120.0f, 125.0f);
        fn_80026808(model, 75.0f, 0.0f, 0.0f);
        fn_80026A44(model, 1U);
        fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
        fn_8002D86C(model, 0.75f);
        fn_800268B0(model, 1.5f, 1.5f, 1.5f);
    }
    if (scene->models[34] < 0) {
        model = scene->models[34] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[34], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[34]);
    }
    team->props[2].model = model;
    fn_80026A44(model, ((1 << 30) | (1 << 1)));
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_80026B8C(model, 5);
    team->props[2].pos.x = 0.0f;
    team->props[2].pos.y = -120.0f;
    team->props[2].pos.z = 400.0f;
    fn_80026790(model, &team->props[2].pos);
    fn_80026808(model, 75.0f, 0.0f, 0.0f);
    fn_80026A44(model, 1U);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
    fn_8002D86C(model, 7.28f);
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
    courseTime = fn_8002D4E0(team->course.model);
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
    fn_8002D86C(team->course.alternateModel, team->blend);
    fn_8002D86C(team->course.model, team->blend);
    fn_8002D340(team->course.alternateModel, 0, courseTime);
    if (team->blend > 0.2f) {
        propBlend = (team->blend - 0.2f) / 0.2f;
        if (propBlend > 1.0f) {
            propBlend = 1.0f;
        }
        fn_80026A44(team->props[2].model, 1U);
    } else {
        propBlend = 0.0f;
        fn_800269E8(team->props[2].model, 1U);
    }
    team->props[2].pos.z = 800.0f - (400.0f * propBlend);
    fn_80026790(team->props[2].model, &team->props[2].pos);
    fn_1_3518(&team->actor, (s32) team->active, team->blend);
    fn_80026838(team->actor.modelB, &team->actor.rot);
    fn_80026838(team->actor.modelA, &team->actor.rot);
    if (fn_8002D4E0(team->course.model) >= sceneState->eventTimes[team->count]) {
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
                fn_800269E8(team->course.overlays[team->count - 1], 1U);
                fn_80026808(eventProp->model, 0.0f, 0.0f, lbl_1_data_380[team->count - 1]);
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
        fn_80026A44(prop->model, 1U);
        fn_80026A44(prop->model, ((1 << 30) | (1 << 1)));
        fn_8002D414(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (fn_8002D75C(prop->model)) {
            fn_800269E8(prop->model, 1U);
            fn_800269E8(prop->model, ((1 << 30) | (1 << 1)));
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
        fn_80026A44(prop->model, 1U);
        fn_80026A44(prop->model, ((1 << 30) | (1 << 1)));
        fn_80026A44(prop->model, ((s32)((3U << 30) | (1U << 1))));
        fn_8002D414(prop->model, 0.0f);
        fn_8002D340(prop->model, 0, 0.0f);
        fn_80026790(prop->model, &prop->pos);
        prop->state++;
        /* fallthrough */
    case 1:
        if (fn_8002D75C(prop->model)) {
            fn_800269E8(prop->model, 1U);
            fn_800269E8(prop->model, ((1 << 30) | (1 << 1)));
            fn_800269E8(prop->model, ((s32)((3U << 30) | (1U << 1))));
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
        model = scene->models[camera + 29] = fn_80025A04(
            fn_80008DD0(lbl_1_data_3A8[camera + 29], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[camera + 29]);
    }
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 3);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026838(model, &actor->rot);
    fn_80026A44(model, 1U);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
    fn_80027080(model, lbl_1_data_4E4);
    actor->modelB = model;
    if (scene->models[28] < 0) {
        model = scene->models[28] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[28], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[28]);
    }
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 3);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026838(model, &actor->rot);
    fn_80026A44(model, 1U);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
    fn_80026E44(model);
    actor->modelA = model;
    if (scene->models[31] < 0) {
        model = scene->models[31] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[31], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[31]);
    }
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 3);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026A44(model, 1U);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
    actor->modelC = model;
    if (scene->models[31] < 0) {
        model = scene->models[31] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[31], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[31]);
    }
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 3);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026A44(model, 1U);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
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
    fn_8002D414(actor->modelB, 16.0f * poseWeight);
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
    fn_80026760(bounce->modelB, 0.0f, bounceHeight, 0.0f);
    fn_80026760(bounce->modelA, 0.0f, bounceHeight, 0.0f);
    return done;
}

/* Called by the sequence init hook fn_1_1EC: load the reflection map and both team courses. */
void fn_1_3804(void *sceneData)
{
    Hu3DReflectMapSet(fn_80008DD0(DATANUM(0x610000, 52), 0x10000000, 2));
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
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[21], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[21]);
    }
    course->model = model;
    fn_80026A44(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 0);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
    fn_8002D86C(model, 0.0f);
    fn_80026A44(model, 1U);
    if (scene->alternateLight != 0) {
        if (scene->models[70] < 0) {
            model = scene->models[70] = fn_80025A04(
                fn_80008DD0(lbl_1_data_3A8[70], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[70]);
        }
    } else if (scene->models[24] < 0) {
        model = scene->models[24] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[24], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[24]);
    }
    course->alternateModel = model;
    fn_80026A44(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 0);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
    fn_8002D86C(model, 0.0f);
    fn_80026A44(model, 1U);
    fn_8002D340(model, 0, 0.0f);
    fn_80026A44(model, ((s32)((3U << 30) | (1U << 1))));
    i = 0;
    while (i < 4) {
        if (scene->alternateLight != 0) {
            if (scene->models[i + 49] < 0) {
                model = scene->models[i + 49] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_3A8[i + 49], 0x10000000, 2));
            } else {
                model = fn_80025D4C(scene->models[i + 49]);
            }
        } else if (scene->models[i] < 0) {
            model = scene->models[i] =
                fn_80025A04(fn_80008DD0(lbl_1_data_3A8[i], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[i]);
        }
        course->segments[i] = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_80026A44(model, 1U);
        fn_80026B8C(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
        fn_8002D86C(model, 0.2f);
        i += 1;
    }
    while (i < 20) {
        if (scene->alternateLight != 0) {
            if (scene->models[i + 49] < 0) {
                model = scene->models[i + 49] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_3A8[i + 49], 0x10000000, 2));
            } else {
                model = fn_80025D4C(scene->models[i + 49]);
            }
        } else if (scene->models[i] < 0) {
            model = scene->models[i] =
                fn_80025A04(fn_80008DD0(lbl_1_data_3A8[i], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[i]);
        }
        course->segments[i] = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_800269E8(model, 1U);
        fn_80026B8C(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
        if (i == 4) {
            fn_80026A44(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
            fn_8002D86C(model, 0.2f);
        }
        if (i == 5) {
            fn_80026A44(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
            fn_8002D86C(model, 0.5f);
        }
        if (i == 16) {
            fn_80026A44(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
            fn_8002D86C(model, 0.3f);
        }
        if (i == 17) {
            fn_80026A44(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
            fn_8002D86C(model, 0.3f);
        }
        i += 1;
    }
    if (scene->alternateLight != 0) {
        if (scene->models[69] < 0) {
            model = fn_80025A04(
                fn_80008DD0(lbl_1_data_3A8[69], 0x10000000, 2));
            scene->models[69] = model;
        } else {
            model = fn_80025D4C(scene->models[69]);
        }
    } else if (scene->models[20] < 0) {
        model = scene->models[20] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[20], 0x10000000, 2));
    } else {
        model = fn_80025D4C(scene->models[20]);
    }
    course->lastModel = model;
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
    fn_800269E8(model, 1U);
    j = 1;
    variant = 0;
    i = 0;
    while (i < 40) {
        if (scene->models[variant + 22] < 0) {
            model = scene->models[variant + 22] = fn_80025A04(
                fn_80008DD0(lbl_1_data_3A8[variant + 22], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[variant + 22]);
        }
        course->connectors[i] = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_80026B8C(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026A44(model, 1U);
        fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
        if (strcmp(lbl_1_data_290[i], lbl_1_data_230) != 0) {
            if (j < 10) {
                variant = scene->choices[j];
                if (scene->models[variant + 25] < 0) {
                    model = scene->models[variant + 25] = fn_80025A04(fn_80008DD0(
                        lbl_1_data_3A8[variant + 25], 0x10000000, 2));
                } else {
                    model = fn_80025D4C(scene->models[variant + 25]);
                }
                course->events[j - 1] = model;
                fn_800269E8(model, ((1 << 30) | (1 << 1)));
                fn_80026B8C(model, 1);
                fn_80026760(model, 0.0f, 0.0f, 0.0f);
                fn_80026808(model, 0.0f, 0.0f, 0.0f);
                fn_80026A44(model, 1U);
                fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
                scene->eventTimes[j - 1] = 15.0f + (15.0f * (f32) i);
                if (scene->models[variant + 39] < 0) {
                    model = fn_80025A04(fn_80008DD0(lbl_1_data_3A8[variant + 39],
                                                                 0x10000000, 2));
                    scene->models[variant + 39] = model;
                } else {
                    model = fn_80025D4C(scene->models[variant + 39]);
                }
                course->overlays[j - 1] = model;
                fn_800269E8(model, ((1 << 30) | (1 << 1)));
                fn_80026B8C(model, 5);
                fn_80026760(model, 0.0f, 0.0f, 0.0f);
                fn_80026808(model, 0.0f, 0.0f, 0.0f);
                fn_80026A44(model, 1U);
                fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
            } else {
                variant = scene->choices[j];
                if (scene->models[27] < 0) {
                    model = scene->models[27] = fn_80025A04(
                        fn_80008DD0(lbl_1_data_3A8[27], 0x10000000, 2));
                } else {
                    model = fn_80025D4C(scene->models[27]);
                }
                course->events[j - 1] = model;
                fn_800269E8(model, ((1 << 30) | (1 << 1)));
                fn_80026B8C(model, 1);
                fn_80026760(model, 0.0f, 0.0f, 0.0f);
                fn_80026808(model, 0.0f, 0.0f, 0.0f);
                fn_80026A44(model, 1U);
                fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
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
                model = scene->models[71] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_3A8[71], 0x10000000, 2));
            } else {
                model = fn_80025D4C(scene->models[71]);
            }
            course->nightModels[i] = model;
            fn_80026A44(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
            fn_80026B8C(model, 1);
            fn_80026760(model, 0.0f, 0.0f, 0.0f);
            fn_80026808(model, 0.0f, 0.0f, 0.0f);
            fn_80026A44(model, 1U);
            fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
            fn_8002D86C(model, 0.2f);
            i += 1;
        }
    }
    fn_800269E8(course->events[0], 1U);
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
                    fn_80025A04(fn_80008DD0(
                        lbl_1_data_3A8[41 + camera * 4 + i * 2 + j], 0x10000000, 2));
            } else {
                model = fn_80025D4C(scene->models[41 + camera * 4 + i * 2 + j]);
            }
            course->pair[i][j].model = model;
            fn_800269E8(model, ((1 << 30) | (1 << 1)));
            fn_800269E8(model, ((s32)((3U << 30) | (1U << 1))));
            fn_80026B8C(model, 5);
            fn_80026760(model, 0.0f, 0.0f, 0.0f);
            fn_80026808(model, 0.0f, 0.0f, 0.0f);
            fn_800269E8(model, 1U);
            fn_80026B64(model, (u16) lbl_1_rodata_10[camera]);
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
        fn_8002741C(course->segments[i / 2], lbl_1_data_190[i], course->connectors[i]);
        if (strcmp(lbl_1_data_290[i], lbl_1_data_230) != 0) {
            fn_8002741C(course->segments[i / 2], lbl_1_data_290[i],
                             course->events[eventCount - 1]);
            if (eventCount < 10) {
                fn_8002741C(course->events[eventCount - 1], lbl_1_data_4DE,
                                 course->overlays[eventCount - 1]);
            }
            eventCount += 1;
        }
    }
    fn_8002741C(course->model, lbl_1_data_4EB, course->segments[0]);
    fn_8002741C(course->model, lbl_1_data_4EF, course->segments[1]);
    fn_8002741C(course->model, lbl_1_data_4F3, course->segments[2]);
    fn_8002741C(course->model, lbl_1_data_4F7, course->segments[3]);
    if (scene->alternateLight != 0) {
        fn_8002741C(course->lastModel, lbl_1_data_4FB, course->nightModels[0]);
        fn_8002741C(course->lastModel, lbl_1_data_502, course->nightModels[1]);
        fn_8002741C(course->lastModel, lbl_1_data_509, course->nightModels[2]);
        fn_8002741C(course->lastModel, lbl_1_data_510, course->nightModels[3]);
        fn_8002741C(course->lastModel, lbl_1_data_517, course->nightModels[4]);
    }
}

/* Called each scene update by fn_1_19C0 to advance a team's course and prop schedules. */
void fn_1_4B4C(M638CourseView *course)
{
    char *hooks[4] = {lbl_1_data_4EB, lbl_1_data_4EF, lbl_1_data_4F3, lbl_1_data_4F7};
    f32 courseMotionTime;
    int segmentHookIndex;
    courseMotionTime = fn_8002D4E0(course->model);
    if (courseMotionTime > 60.0f + 30.0f * course->current) {
        segmentHookIndex = course->current % 4;
        fn_800275B4(course->model, hooks[segmentHookIndex]);
        fn_800269E8(course->segments[course->current], 1U);
        if (course->current + 4 < 20) {
            fn_8002741C(course->model, hooks[segmentHookIndex],
                             course->segments[course->current + 4]);
            fn_80026A44(course->segments[course->current + 4], 1U);
        }
        course->current++;
    }
    if (course->current == 0 && courseMotionTime >= 30.0f &&
        (fn_80026A90(course->events[0]) & 1)) {
        fn_80026A44(course->events[0], 1U);
    }
    if (courseMotionTime >= 536.0f) {
        if (fn_80026A90(course->lastModel) & 1) {
            fn_80026A44(course->lastModel, 1U);
            fn_80026A44(course->lastModel, ((1 << 30) | (1 << 1)));
        } else {
            fn_8002D414(course->lastModel, courseMotionTime - 536.0f);
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

    model = espEntry(DATANUM(0x610000, 49), 111, 0);
    scene->sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 50), 110, 0);
    scene->sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 51), 110, 0);
    scene->sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 288.0f, 240.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);

    model = espEntry(DATANUM(0x610000, 81), 111, 0);
    scene->teams[0].sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 77), 110, 0);
    scene->teams[0].sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 78), 110, 0);
    scene->teams[0].sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 79), 110, 0);
    scene->teams[0].sprites[3] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 80), 110, 0);
    scene->teams[0].sprites[4] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 50.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    scene->teams[0].buttonIndicatorFrames = 0;
    scene->teams[0].buttonIndicatorIndex = 0;

    model = espEntry(DATANUM(0x610000, 81), 111, 0);
    scene->teams[1].sprites[0] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 77), 110, 0);
    scene->teams[1].sprites[1] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 78), 110, 0);
    scene->teams[1].sprites[2] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 79), 110, 0);
    scene->teams[1].sprites[3] = model;
    espDrawNoSet(model, 0);
    espPosSet(model, 526.0f, 150.0f);
    espAttrSet(model, 1U);
    espAttrSet(model, 4U);
    model = espEntry(DATANUM(0x610000, 80), 110, 0);
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
    viewProgress = (fn_8002D4E0(scene->team0Model) - 120.0f) / 435.0f;
    if (viewProgress < 0.0f) {
        viewProgress = 0.0f;
    }
    if (viewProgress > 1.0f) {
        viewProgress = 1.0f;
    }
    y = 400.0f * (1.0f - viewProgress) + 120.0f * viewProgress;
    espPosSet(scene->sprite0, 276.0f, y);
    viewProgress = (fn_8002D4E0(scene->team1Model) - 120.0f) / 435.0f;
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
            fn_80025A04(fn_80008DD0(lbl_1_data_3A8[32], 0x10000000, 2));
    } else {
        model = fn_80025D4C(modelCache->models[32]);
    }
    scale->model = model;
    fn_800269E8(model, ((1 << 30) | (1 << 1)));
    fn_80026B8C(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
    fn_800269E8(model, 1U);
    fn_8002D86C(model, 9.74f);
    fn_800268B0(model, 1.75f, 1.75f, 1.75f);
}

/* Called by fn_1_2288 each scene update to apply the team's active response-scale animation. */
void fn_1_56B4(M638ScaleWork *scale)
{
    f32 modelScale;

    if (scale->active != 0) {
        modelScale = 1.0f + ((scale->responseScale * fn_8002D4E0(scale->model)) / 60.0f);
        fn_800268B0(scale->model, modelScale, modelScale, modelScale);
        if (fn_8002D75C(scale->model) != 0) {
            fn_800269E8(scale->model, 1U);
            scale->active = 0;
        }
    }
}

/* Called by fn_1_2CDC on a B press to start the team's response-scale animation. */
void fn_1_5760(M638ScaleWork *scale, f32 responseAmount)
{
    if (scale->active == 0) {
        scale->responseScale = 1.5f * responseAmount;
        fn_800268B0(scale->model, 1.0f, 1.0f, 1.0f);
        fn_80026A44(scale->model, ((1 << 30) | (1 << 1)));
        fn_80026A44(scale->model, 1U);
        fn_8002D414(scale->model, 0.0f);
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
            model = scene->models[modelCacheGroup[i] + 35] = fn_80025A04(fn_80008DD0(
                lbl_1_data_3A8[modelCacheGroup[i] + 35], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[modelCacheGroup[i] + 35]);
        }
        propGroups->small[i].model = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_80026B8C(model, 5);
        propGroups->small[i].pos.x = 0.0f;
        propGroups->small[i].pos.y = -100.0f;
        propGroups->small[i].pos.z = 100.0f;
        fn_80026790(model, &propGroups->small[i].pos);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
        fn_800269E8(model, 1U);
        propGroups->small[i].state = 2;
        propGroups->small[i].moving = 0;
    }
    for (i = 0; i < 8; i++) {
        if (scene->models[37] < 0) {
            model = scene->models[37] = fn_80025A04(
                fn_80008DD0(lbl_1_data_3A8[37], 0x10000000, 2));
        } else {
            model = fn_80025D4C(scene->models[37]);
        }
        propGroups->middle[i].model = model;
        fn_800269E8(model, ((1 << 30) | (1 << 1)));
        fn_80026B8C(model, 5);
        propGroups->middle[i].pos.x = 0.0f;
        propGroups->middle[i].pos.y = -150.0f;
        propGroups->middle[i].pos.z = 100.0f;
        fn_80026790(model, &propGroups->middle[i].pos);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
        fn_800269E8(model, 1U);
        propGroups->middle[i].state = 2;
        propGroups->middle[i].moving = 0;
    }
    if (scene->alternateLight) {
        for (i = 0; i < 4; i++) {
            if (scene->models[38] < 0) {
                model = scene->models[38] = fn_80025A04(
                    fn_80008DD0(lbl_1_data_3A8[38], 0x10000000, 2));
            } else {
                model = fn_80025D4C(scene->models[38]);
            }
            propGroups->night[i].model = model;
            fn_800269E8(model, ((1 << 30) | (1 << 1)));
            fn_80026B8C(model, 5);
            propGroups->night[i].pos.x = 0.0f;
            propGroups->night[i].pos.y = -150.0f;
            propGroups->night[i].pos.z = 100.0f;
            fn_80026790(model, &propGroups->night[i].pos);
            fn_80026808(model, 0.0f, 0.0f, 0.0f);
            fn_80026B64(model, (u16)lbl_1_rodata_10[camera]);
            fn_800269E8(model, 1U);
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
                    fn_8002D86C(group->night[i].model, 0.11f);
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
                    fn_8002D86C(group->small[i].model, 0.68f);
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
                    fn_8002D86C(group->middle[i].model, 0.34f);
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
        fn_80026790(prop->model, &prop->pos);
        fn_80026A44(prop->model, 1U);
        fn_80026A44(prop->model, ((1 << 30) | (1 << 1)));
        fn_8002D414(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (fn_8002D75C(prop->model)) {
            fn_800269E8(prop->model, 1U);
            fn_800269E8(prop->model, ((1 << 30) | (1 << 1)));
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
        fn_80026790(prop->model, &prop->pos);
        fn_80026808(prop->model, -10.0f, 0.0f, 0.0f);
        fn_80026A44(prop->model, 1U);
        fn_80026A44(prop->model, ((1 << 30) | (1 << 1)));
        fn_8002D414(prop->model, 0.0f);
        prop->state++;
        /* fallthrough */
    case 1:
        if (prop->moving) {
            prop->pos.z += 1.0f;
        }
        fn_80026790(prop->model, &prop->pos);
        if (fn_8002D75C(prop->model)) {
            fn_800269E8(prop->model, 1U);
            fn_800269E8(prop->model, ((1 << 30) | (1 << 1)));
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
        motionTime = fn_8002D4E0(team->model);
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
        motionTime = fn_8002D4E0(team->model);
        weight = (600.0f - motionTime) / 30.0f;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        if (weight < 0.0f) {
            weight = 0.0f;
        }
        team->blend = 0.3f * weight;
        if (fn_8002D75C(team->model)) {
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
