/* Strawberry Shortfuse: round sequencing, board paths and stage effects. */
#include "REL/m680Dll/work.h"
#include "game/flag.h"
#include "game/process.h"
#include "game/board/masu.h"
#include "messdir_enum.h"

#define MSM_SE_M680_PIECE_MOTION 2318
#define MSM_SE_M680_STAGE_EFFECT 2319
#define MSM_SE_M680_CURSOR_MOVE 2320
#define MSM_SE_M680_SELECTION_CONFIRM 2321
#define MSM_SE_M680_ITEM_ATTACH 2322
#define MSM_SE_M680_PIECE_LOSS 2323
#define MSM_SE_M680_LOSS_EFFECT 2324
#define MSM_SE_M680_STAGE_PROGRESS 2325
#define MSM_SE_M680_MARKER_COMPLETE 2326
#define MSM_SE_M680_STAGE_ATTACHMENT 2327
#define MSM_SE_M680_TRANSITION_FORWARD 2328
#define MSM_SE_M680_TRANSITION_REVERSE 2329
#define MSM_SE_M680_ROUND_START 2330
#define MSM_SE_M680_WIN_CUE 579
#define M680_WIN_JINGLE_ID 67
#define M680_OBJECT_MANAGER_SLOTS 10
#define M680_OBJECT_MANAGER_HEAP_SIZE 4096
#define M680_ROOT_OBJECT_PRIORITY 101
#define M680_PLAYER_OBJECT_PRIORITY 100
#define M680_PLAYER_MODEL_COUNT 10
#define M680_BACKGROUND_COLOR_LEVEL 63
#define M680_ROUND_TIMER_FRAMES 600
#define M680_GRID_NODE_COUNT 10
#define M680_SEQUENCE_DELAY_15_FRAMES 15
#define M680_SEQUENCE_DELAY_20_FRAMES 20
#define M680_RUMBLE_SETTING_20 20
#define M680_SEQUENCE_DELAY_30_FRAMES 30
#define M680_SEQUENCE_DELAY_60_FRAMES 60
#define M680_SEQUENCE_STATE_INITIALIZE 0
#define M680_SEQUENCE_STATE_START_DELAY_SETUP 1
#define M680_SEQUENCE_STATE_START_DELAY 2
#define M680_SEQUENCE_STATE_FIRST_GROUP_SETUP 10
#define M680_SEQUENCE_STATE_FIRST_GROUP_REVEAL 11
#define M680_SEQUENCE_STATE_GROUP_GAP 12
#define M680_SEQUENCE_STATE_SECOND_GROUP_REVEAL 13
#define M680_SEQUENCE_STATE_FINAL_GAP 14
#define M680_SEQUENCE_STATE_FINAL_DELAY 15
#define M680_SEQUENCE_STATE_ROUND_SETUP 20
#define M680_SEQUENCE_STATE_GRID_SELECTION 21
#define M680_SEQUENCE_STATE_PLAYER_ENTRANCE 30
#define M680_SEQUENCE_STATE_PLAYER_ENTRANCE_WAIT 31
#define M680_SEQUENCE_STATE_SELECTION_SETUP 40
#define M680_SEQUENCE_STATE_SELECTION_INPUT 41
#define M680_SEQUENCE_STATE_ITEM_PICKUP 50
#define M680_SEQUENCE_STATE_WAIT_FOR_PIECE_CLEAR 51
#define M680_SEQUENCE_STATE_WAIT_FOR_PLAYER_IDLE 52
#define M680_SEQUENCE_STATE_PLAYER_PICKUP_DELAY 53
#define M680_SEQUENCE_STATE_ITEM_ATTACH_DELAY 54
#define M680_SEQUENCE_STATE_WAIT_FOR_ITEM_ATTACH 55
#define M680_SEQUENCE_STATE_VICTORY_START 60
#define M680_SEQUENCE_STATE_VICTORY_DELAY 61
#define M680_SEQUENCE_STATE_WAIT_FOR_VICTORY_MOTION 62
#define M680_SEQUENCE_STATE_WAIT_FOR_VICTORY_END 63
#define M680_SEQUENCE_STATE_LOSS_START 70
#define M680_SEQUENCE_STATE_LOSS_DELAY 71
#define M680_SEQUENCE_STATE_PLAYER_ELIMINATION 72
#define M680_SEQUENCE_STATE_ELIMINATION_DELAY 73
#define M680_SEQUENCE_STATE_ROUND_RESULT 80
#define M680_SEQUENCE_STATE_NEXT_ROUND_SETUP 90
#define M680_SEQUENCE_STATE_WAIT_FOR_PIECE_RESET 91
#define M680_SEQUENCE_STATE_ROUND_TRANSITION_DELAY 92
#define M680_SEQUENCE_STATE_WAIT_FOR_ROUND_TRANSITION 93
#define M680_SEQUENCE_STATE_ADVANCE_MINIGAME 110
#define M680_SEQUENCE_STATE_FINAL_ROUND 120
#define M680_SEQUENCE_STATE_SET_WINNING_PIECES 100
#define M680_SEQUENCE_STATE_WAIT_FOR_FINAL_STAGE_TURN 101
#define M680_SEQUENCE_STATE_WAIT_FOR_STAGE_TURN_END 102
#define M680_SEQUENCE_STATE_START_FIRST_GROUP_REVEAL 103
#define M680_SEQUENCE_STATE_REVEAL_FIRST_GROUP 104
#define M680_SEQUENCE_STATE_WAIT_FOR_FIRST_GROUP_CLEAR 105
#define M680_SEQUENCE_STATE_START_SECOND_GROUP_REVEAL 106
#define M680_SEQUENCE_STATE_REVEAL_SECOND_GROUP 107
#define M680_SEQUENCE_STATE_WAIT_FOR_SECOND_GROUP_CLEAR 108
#define M680_SEQUENCE_STATE_FINAL_STAGE_DELAY 109
#define M680_PIECE_STATE_SELECTED 100
#define M680_PIECE_STATE_WINNER 200
#define M680_PIECE_STATE_NEXT_ROUND 300
#define M680_PIECE_STATE_FIRST_GROUP 400
#define M680_PIECE_STATE_SECOND_GROUP 500
#define M680_PICKUP_ANIMATION_FRAMES 15
#define M680_VICTORY_DELAY_FRAMES 30
#define M680_SEQUENCE_FRAME_LIMIT 31
#define M680_STAGE_REVERSE_START_FRAME 24
#define M680_STAGE_REVERSE_FRAME_LIMIT 25
#define M680_STAGE_MARKER_STEP_FRAMES 12
#define M680_STAGE_TRANSITION_FRAMES 240
#define M680_LOSS_EFFECT_START_FRAME 50
#define M680_PLAYER_ELIMINATION_START_FRAME 60
#define M680_ROUND_TRANSITION_DELAY_FRAMES 45
#define M680_LOSS_RUMBLE_SETTING 40
#define M680_LOSS_RUMBLE_DURATION 20
#define M680_SEQUENCE_STATE_PLAYER_RESULT_SETUP 121
#define M680_SEQUENCE_STATE_PLAYER_RESULT_WAIT 122
#define M680_SEQUENCE_STATE_STAGE_PROGRESS 123
#define M680_PIECE_STATE_IDLE 0
#define M680_PIECE_STATE_MOVE_START 1
#define M680_PIECE_STATE_MOVE_SELECT 2
#define M680_PIECE_STATE_MOVE 3
#define M680_PIECE_STATE_PATH_SEARCH 4
#define M680_PIECE_STATE_PATH_STEP 5
#define M680_PIECE_STATE_PATH_MOVE 6
#define M680_PIECE_STATE_PATH_RETRY 7
#define M680_PIECE_STATE_PATH_RETRY_WAIT 8
#define M680_PIECE_STATE_TURN_TO_IDLE 9
#define M680_PIECE_STATE_IDLE_WAIT 10
#define M680_PIECE_STATE_GATE_APPROACH 20
#define M680_PIECE_STATE_GATE_APPROACH_WAIT 21
#define M680_PIECE_STATE_GATE_CROSSING_START 22
#define M680_PIECE_STATE_GATE_CROSSING 23
#define M680_PIECE_STATE_GATE_EXIT 24
#define M680_PIECE_STATE_GATE_EXIT_MOVE 25
#define M680_PIECE_STATE_GATE_EXIT_WAIT 26
#define M680_PIECE_STATE_GATE_RETURN 27
#define M680_PIECE_STATE_GATE_RETURN_MOVE 28
#define M680_PIECE_STATE_GATE_RETURN_WAIT 29
#define M680_PIECE_STATE_GATE_COMPLETE 30
#define M680_PIECE_STATE_RESULT_APPROACH 31
#define M680_PIECE_STATE_RESULT_MOVE 32
#define M680_PIECE_STATE_RESULT_WAIT 33
#define M680_PIECE_STATE_RESULT_FINISH 34
#define M680_PIECE_STATE_ROUND_WIN 40
#define M680_PIECE_STATE_ROUND_WIN_WAIT 41
#define M680_PIECE_STATE_RESULT_RETRY_WAIT 35
#define M680_PIECE_STATE_WIN_RETRY_WAIT 42
#define M680_PIECE_MODEL_BASE 43
#define M680_HOOK_MODEL_BASE 24
#define M680_ITEM_MODEL_BASE 14
#define M680_STAGE_TRANSITION_START_FRAMES 30
#define M680_STAGE_TRANSITION_WAIT_FRAMES 90
#define M680_PIECE_MOVE_FRAMES 12
#define M680_PIECE_RETRY_WAIT_FRAMES 11
#define M680_PIECE_PATH_MAX_STEPS 20
#define M680_PIECE_GATE_COLUMN 10
#define M680_PIECE_GATE_END_COLUMN 11
#define M680_PIECE_STAGE_END_COLUMN 12
#define M680_PIECE_STAGE_END_ROW 13
#define M680_PIECE_COLOR_WHITE 255
#define M680_PIECE_COLOR_HIGHLIGHT 239
#define M680_PIECE_COLOR_IDLE_RED 27
#define M680_PIECE_COLOR_IDLE_GREEN 149
#define M680_PIECE_COLOR_IDLE_BLUE 206
#define M680_LIGHT_AMBIENT_COMPONENT 175
#define M680_SHADOW_COLOR_RED 47
#define M680_SHADOW_COLOR_GREEN 31
#define M680_SHADOW_COLOR_BLUE 79
#define M680_COMPUTER_CHOICE_PERCENT 100
#define M680_SEARCH_ATTEMPT_PERCENT 20
#define M680_PATH_SEGMENT_FRAMES 35
#define M680_PIECE_STATE_SELECTED_PATH_START 101
#define M680_PIECE_STATE_SELECTED_PATH_STEP 102
#define M680_PIECE_STATE_SELECTED_PATH_MOVE 103
#define M680_PIECE_STATE_SELECTED_TURN 104
#define M680_PIECE_STATE_WINNER_START 201
#define M680_PIECE_STATE_WINNER_MOVE 202
#define M680_PIECE_STATE_WINNER_END 203
#define M680_PIECE_STATE_WINNER_WAIT 204
#define M680_PIECE_STATE_WINNER_FINISH 205
#define M680_PIECE_STATE_NEXT_ROUND_START 301
#define M680_PIECE_STATE_NEXT_ROUND_MOVE 302
#define M680_PIECE_STATE_FIRST_GROUP_START 401
#define M680_PIECE_STATE_FIRST_GROUP_MOVE 402
#define M680_PIECE_STATE_FIRST_GROUP_TURN 403
#define M680_PIECE_STATE_FIRST_GROUP_EXIT 404
#define M680_PIECE_STATE_SECOND_GROUP_START 501
#define M680_PIECE_STATE_SECOND_GROUP_MOVE 502
#define M680_PIECE_STATE_SECOND_GROUP_TURN 503
#define M680_BOARD_LAST_CELL 12
#define M680_BOARD_EXIT_ROW 13
#define M680_WINNER_APPROACH_FRAMES 29
#define M680_WINNER_PAUSE_FRAMES 24
#define M680_GROUP_REVEAL_FRAMES 48
#define M680_GROUP_EXIT_FRAMES 96
#define M680_EFFECT_PHASE_THIRD 11

/* Smooths the staged attachment's heading by at most fifteen degrees per update. */
static inline void m680UpdateAngle(M680MotionWork *work)
{
    f32 difference;

    while (work->angle > 180.0f) {
        work->angle -= 360.0f;
    }
    while (work->angle < -180.0f) {
        work->angle += 360.0f;
    }
    while (work->rotation.y > 180.0f) {
        work->rotation.y -= 360.0f;
    }
    while (work->rotation.y < -180.0f) {
        work->rotation.y += 360.0f;
    }
    difference = work->angle - work->rotation.y;
    while (difference > 180.0f) {
        difference -= 360.0f;
    }
    while (difference < -180.0f) {
        difference += 360.0f;
    }
    if (difference > 0.0f) {
        if (difference > 15.0f) {
            difference = 15.0f;
        }
    } else if ((difference < 0.0f) && (difference < -15.0f)) {
        difference = -15.0f;
    }
    work->rotation.y += difference;
}

#define M680_LOAD_MODEL(index) \
    if (work->models[index] < 0) { \
        model = work->models[index] = Hu3DModelCreate(HuDataSelHeapReadNum( \
            lbl_1_data_370[index], HU_MEMNUM_OVL, HEAP_MODEL)); \
    } else { \
        model = Hu3DModelLink(work->models[index]); \
    }

#define M680_LOAD_MOTION(index) \
    if (work->motions[index] < 0) { \
        motion = work->motions[index] = Hu3DJointMotion(model, HuDataSelHeapReadNum( \
            lbl_1_data_46C[index], HU_MEMNUM_OVL, HEAP_MODEL)); \
    } else { \
        motion = work->motions[index]; \
    } \
    Hu3DMotionSet(model, motion)

#define M680_LOAD_MOTION_ONLY(index) \
    if (work->motions[index] < 0) { \
        motion = work->motions[index] = Hu3DJointMotion(model, HuDataSelHeapReadNum( \
            lbl_1_data_46C[index], HU_MEMNUM_OVL, HEAP_MODEL)); \
    } else { \
        motion = work->motions[index]; \
    }

void fn_1_A0();
void fn_1_280();
void fn_1_308();
void fn_1_3C8();
void fn_1_42C();
void fn_1_15C0();
void fn_1_16E8();
void fn_1_17E4();
void fn_1_1974();
void fn_1_1978();
void fn_1_19B4();
void fn_1_1B0C();
void fn_1_1BF8();
void fn_1_1D10();
void fn_1_1E2C();
void fn_1_1EA4();
void fn_1_1EEC();
void fn_1_33D0();
void fn_1_390C();
void fn_1_395C();
void fn_1_39D0();
void fn_1_613C();
s32 fn_1_62FC();
void fn_1_646C();
void fn_1_6538();
void fn_1_65FC();
void fn_1_6624();
void fn_1_6904();
void fn_1_6AE0();
void fn_1_6B38();
s32 fn_1_6DCC();
s32 fn_1_700C();
s32 fn_1_70BC();
s32 fn_1_7140();
void fn_1_7340();
void fn_1_73EC();
void fn_1_7434();
void fn_1_765C();
void fn_1_76E8();
void fn_1_7724();
void *fn_1_7730();
void fn_1_7788();
s32 fn_1_8288();
void fn_1_8AAC();
void fn_1_8B24();
s32 fn_1_8CAC();
void fn_1_9400();
void fn_1_9448();
void fn_1_9580();
void fn_1_95C0();
void fn_1_9600();
void fn_1_9630();
void fn_1_9754();
void fn_1_9898();
s32 fn_1_9944();
void fn_1_99B4();
s32 fn_1_9A34();
s32 fn_1_9B00();
s32 fn_1_9BDC();
s32 fn_1_9CA0();
void fn_1_9D5C();
s32 fn_1_9D9C();
s32 fn_1_9F18();
s32 fn_1_A0E4();
s32 fn_1_A234();
void fn_1_A388();
s32 fn_1_A490();
void fn_1_A5F4();
s32 fn_1_A658();
s32 fn_1_A708();
void fn_1_A7E8();
s16 fn_1_AABC(f32 position);
void fn_1_AD38();
void fn_1_AB70();
void fn_1_B844();
void fn_1_BAF8(M680CharacterState *state);

const u16 lbl_1_rodata_10[4] = {1, 2, 4, 8};
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, (MGSEQ_FUNC)fn_1_280, (MGSEQ_FUNC)fn_1_308,
    (MGSEQ_FUNC)fn_1_3C8, (MGSEQ_FUNC)fn_1_42C, (MGSEQ_FUNC)fn_1_15C0, (MGSEQ_FUNC)fn_1_16E8,
    (MGSEQ_FUNC)fn_1_17E4, (MGSEQ_FUNC)fn_1_1974, (MGSEQ_FUNC)fn_1_1978
};

s32 lbl_1_data_28[15][14] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1},
    {-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1},
    {-1, -1, -1, -1, 0, -1, 0, -1, 0, 0, 0, -1, -1, -1},
    {-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1},
    {-1, -1, 0, 0, 0, 0, 0, -1, -1, 0, -1, -1, -1, -1},
    {-1, -1, 0, 0, -1, -1, 0, -1, -1, 0, -1, -1, -1, -1},
    {-1, -1, 0, 0, -1, -1, 0, 0, 0, 0, 0, -1, -1, -1},
    {-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1},
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1},
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1},
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1},
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

s32 lbl_1_data_370[63] = {
    DATANUM(DATA_m680, 0), DATANUM(DATA_m680, 1),
    DATANUM(DATA_m680, 2), DATANUM(DATA_m680, 3),
    DATANUM(DATA_m680, 3), DATANUM(DATA_m680, 4),
    DATANUM(DATA_m680, 4), DATANUM(DATA_m680, 5),
    DATANUM(DATA_m680, 6), DATANUM(DATA_m680, 7),
    DATANUM(DATA_m680, 8), DATANUM(DATA_m680, 9),
    DATANUM(DATA_m680, 10), DATANUM(DATA_m680, 11),
    DATANUM(DATA_m680, 12), DATANUM(DATA_m680, 12),
    DATANUM(DATA_m680, 12), DATANUM(DATA_m680, 12),
    DATANUM(DATA_m680, 12), DATANUM(DATA_m680, 12),
    DATANUM(DATA_m680, 12), DATANUM(DATA_m680, 12),
    DATANUM(DATA_m680, 12), DATANUM(DATA_m680, 12),
    DATANUM(DATA_m680, 13), DATANUM(DATA_m680, 13),
    DATANUM(DATA_m680, 13), DATANUM(DATA_m680, 13),
    DATANUM(DATA_m680, 13), DATANUM(DATA_m680, 14),
    DATANUM(DATA_m680, 14), DATANUM(DATA_m680, 14),
    DATANUM(DATA_m680, 14), DATANUM(DATA_m680, 14),
    DATANUM(DATA_m680, 15), DATANUM(DATA_m680, 16),
    DATANUM(DATA_m680, 17), DATANUM(DATA_m680, 18),
    DATANUM(DATA_m680, 19), DATANUM(DATA_m680, 20),
    DATANUM(DATA_m680, 21), DATANUM(DATA_m680, 22),
    DATANUM(DATA_m680, 23), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 27),
    DATANUM(DATA_m680, 27), DATANUM(DATA_m680, 24),
    DATANUM(DATA_m680, 25), DATANUM(DATA_m680, 26),
    DATANUM(DATA_m680, 43)
};

s32 lbl_1_data_46C[15] = {
    DATANUM(DATA_m680, 28), DATANUM(DATA_m680, 29),
    DATANUM(DATA_m680, 30), DATANUM(DATA_m680, 31),
    DATANUM(DATA_m680, 32), DATANUM(DATA_m680, 33),
    DATANUM(DATA_m680, 34), DATANUM(DATA_m680, 35),
    DATANUM(DATA_m680, 36), DATANUM(DATA_m680, 37),
    DATANUM(DATA_m680, 38), DATANUM(DATA_m680, 39),
    DATANUM(DATA_m680, 40), DATANUM(DATA_m680, 41),
    DATANUM(DATA_m680, 42)
};

/* Orders players and creates the board objects when the minigame entry point starts. */
void fn_1_A0(void)
{
    M680PlayerRankEntry entries[4];
    OMOBJ *rootObject;
    M680State *boardWork;
    s32 player;

    for (player = 0; player < 4; player++) {
        entries[player].player = player;
        if (_CheckFlag(FLAG_INST_MG_MODE) || _CheckFlag(FLAG_BOARD_INIT)) {
            entries[player].rank = frandmod(4);
        } else {
            entries[player].rank = mbPlayerRankGet(player);
        }
    }
    fn_1_AB70(4, entries);
    lbl_1_bss_8.manager = omInitObjMan(M680_OBJECT_MANAGER_SLOTS, M680_OBJECT_MANAGER_HEAP_SIZE);
    omGameSysInit(lbl_1_bss_8.manager);
    rootObject = omAddObjEx(lbl_1_bss_8.manager, M680_ROOT_OBJECT_PRIORITY, 0, 0, -1, fn_1_1E2C);
    lbl_1_bss_0 = rootObject;
    rootObject->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M680State), HU_MEMNUM_OVL);
    boardWork = rootObject->data;
    for (player = 0; player < 4; player++) {
        rootObject = omAddObjEx(lbl_1_bss_8.manager, M680_PLAYER_OBJECT_PRIORITY, 1,
                                M680_PLAYER_MODEL_COUNT, 0, fn_1_AD38);
        rootObject->data =
            HuMemDirectMallocNum(HEAP_HEAP, sizeof(M680CharacterState), HU_MEMNUM_OVL);
        rootObject->work[0] = entries[player].player;
        rootObject->work[1] = player;
    }
    Hu3DBGColorSet(M680_BACKGROUND_COLOR_LEVEL, M680_BACKGROUND_COLOR_LEVEL,
                   M680_BACKGROUND_COLOR_LEVEL);
    Hu3DNoSyncSet(1);
    fn_1_1D10(boardWork);
    fn_1_19B4(boardWork);
    fn_1_1B0C(boardWork);
    fn_1_1BF8(boardWork);
    MgSeqCreate(&lbl_1_data_0);
}

/* MgSeqCreate's initialization callback creates the round timer and clears round counters. */
void fn_1_280(void) {
    M680State *sequence;

    sequence = lbl_1_bss_0->data;
    sequence->timer = MgTimerCreate(0);
    MgTimerParamSet(sequence->timer, M680_ROUND_TIMER_FRAMES, 0, 0);
    MgTimerPosSet(sequence->timer, 525.0f, 64.0f);
    sequence->round = 0;
    sequence->playerCount = 0;
    MgSeqModeNext();
}

/* MgSeqCreate calls this phase callback to place players, then wait 75 frames for their
 * entrance. */
void fn_1_308(void)
{
    M680State *sequence =
        lbl_1_bss_0->data;
    void *actor;
    s32 actorIndex;

    switch (sequence->openingPhase) {
    case 0:
        MgSeqModeChangeOff();
        sequence->frame = 0;
        for (actorIndex = 0; actorIndex < 4; actorIndex++) {
            actor = sequence->actors[actorIndex];
            fn_1_B844(actor, actorIndex, 1);
        }
        sequence->openingPhase++;
        /* fall through */
    case 1:
        sequence->frame++;
        if (sequence->frame >= 75) {
            MgSeqModeNext();
        }
        break;
    default:
        break;
    }
}

/* MgSeqCreate's music callback starts the background stream once during the start sequence. */
void fn_1_3C8(void)
{
    M680State *sequence =
        lbl_1_bss_0->data;

    switch (sequence->musicStarted) {
    case 0:
        lbl_1_bss_4 = HuAudBGMPlay(MSM_STREAM_MGMUS_26);
        sequence->musicStarted++;
        break;
    default:
        break;
    }
}

/* MgSeqCreate calls this phase callback each frame to advance board, turn and result states. */
void fn_1_42C(void) {
    int count;
    int index;
    M680CharacterState *player;
    M680State *work;
    M680PlayerNode *node;

    work = lbl_1_bss_0->data;
    switch (work->state) {
        case 0:
            fn_1_7434(work);
            work->gridIndex = 0;
            work->repath = 0;
            work->round++;
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_START_DELAY_SETUP:
            work->frame = 0;
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_START_DELAY:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_30_FRAMES) {
                work->state = M680_SEQUENCE_STATE_FIRST_GROUP_SETUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_FIRST_GROUP_SETUP:
            work->frame = 0;
            for (index = 0; index < M680_GRID_NODE_COUNT; index++) {
                Hu3DModelPosSet(work->grid[index].itemModel, 0.0f, 800.0f, 0.0f);
            }
            HuAudFXPlay(MSM_SE_M680_ROUND_START);
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_FIRST_GROUP_REVEAL:
            work->frame++;
            fn_1_9630(work, 0);
            if (work->frame >= M680_SEQUENCE_DELAY_30_FRAMES) {
                fn_1_9898(work, 0);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_GROUP_GAP:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_20_FRAMES) {
                HuAudFXPlay(MSM_SE_M680_ROUND_START);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_SECOND_GROUP_REVEAL:
            work->frame++;
            fn_1_9630(work, 5);
            if (work->frame >= M680_SEQUENCE_DELAY_30_FRAMES) {
                fn_1_9898(work, 5);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_FINAL_GAP:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_20_FRAMES) {
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_FINAL_DELAY:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_20_FRAMES) {
                work->state = M680_SEQUENCE_STATE_ROUND_SETUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ROUND_SETUP:
            work->roundWins = 0;
            work->state++;
            fn_1_95C0(work);
            fn_1_A388(&work->effect);
            for (index = 0; index < M680_GRID_NODE_COUNT; index++) {
                work->grid[index].state = 1;
            }
            /* fallthrough */
        case M680_SEQUENCE_STATE_GRID_SELECTION:
            if (work->gridIndex >= M680_GRID_NODE_COUNT) {
                work->state = M680_SEQUENCE_STATE_PLAYER_ENTRANCE;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_PLAYER_ENTRANCE:
            player = work->players->player;
            fn_1_B844(player, 4, 0);
            work->frame = 0;
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_PLAYER_ENTRANCE_WAIT:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_60_FRAMES) {
                fn_1_9580(work);
                work->state = M680_SEQUENCE_STATE_SELECTION_SETUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_SELECTION_SETUP:
            MgTimerParamSet(work->timer, M680_ROUND_TIMER_FRAMES, 0, 0);
            MgTimerRecordDispOn(work->timer);
            MgTimerModeOnSet(work->timer, 1);
            work->selected = fn_1_7730(work);
            fn_1_A7E8(work);
            Hu3DModelAttrReset(work->cursorModel, HU3D_ATTR_DISPOFF);
            player = work->players->player;
            omVibrate(player->playerIndex, M680_RUMBLE_SETTING_20, 7, 3);
            if (player->playerType != 0) {
                fn_1_9400(work);
            }
            work->state += 1;
            break;
        case M680_SEQUENCE_STATE_SELECTION_INPUT:
            if (MgTimerDoneCheck(work->timer) != 0) {
                Hu3DModelAttrSet(work->cursorModel, HU3D_ATTR_DISPOFF);
                work->state = M680_SEQUENCE_STATE_ITEM_PICKUP;
                break;
            }
            if (fn_1_8CAC(work)) {
                MgTimerRecordDispOff(work->timer);
                Hu3DModelAttrSet(work->cursorModel, HU3D_ATTR_DISPOFF);
                work->state = M680_SEQUENCE_STATE_ITEM_PICKUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ITEM_PICKUP:
            work->selected->state = M680_PIECE_STATE_SELECTED;
            player = work->players->player;
            fn_1_B844(player, 5, 0);
            work->state += 1;
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_PIECE_CLEAR:
            if (work->selected->state == 0) {
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_PLAYER_IDLE:
            player = work->players->player;
            if (player->motionIds[0] == Hu3DMotionIDGet(player->modelId)) {
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_PLAYER_PICKUP_DELAY:
            work->frame++;
            if (work->frame >= M680_PICKUP_ANIMATION_FRAMES) {
                player = work->players->player;
                CharMotionShiftSet(player->character, player->motionIds[3], 0.0f, 5.0f, 0U);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ITEM_ATTACH_DELAY:
            work->frame++;
            if (work->frame >= M680_PICKUP_ANIMATION_FRAMES) {
                HuAudFXPlay(MSM_SE_M680_ITEM_ATTACH);
                player = work->players->player;
                Hu3DModelHookReset(work->selected->hookModel);
                Hu3DModelHookSet(player->modelId, CharModelItemHookGet(player->character, 4, 0),
                                 work->selected->itemModel);
                fn_1_BE08(fn_1_9BDC, work->selected);
                work->selected->handTimer = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_ITEM_ATTACH:
            player = work->players->player;
            if (Hu3DMotionEndCheck(player->modelId) != 0) {
                Hu3DModelHookObjReset(player->modelId,
                                      CharModelItemHookGet(player->character, 4, 0));
                Hu3DModelHookSet(work->selected->hookModel, "hook_futa", work->selected->itemModel);
                Hu3DModelAttrSet(work->selected->itemModel, HU3D_ATTR_DISPOFF);
                if (work->selected->result == 0) {
                    work->result = 0;
                    work->state = M680_SEQUENCE_STATE_LOSS_START;
                } else {
                    work->result = 1;
                    work->state = M680_SEQUENCE_STATE_VICTORY_START;
                }
                fn_1_7724(work->selected);
                break;
            }
            break;
        case M680_SEQUENCE_STATE_VICTORY_START:
            work->frame = 0;
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_VICTORY_DELAY:
            work->frame++;
            if (work->frame >= M680_VICTORY_DELAY_FRAMES) {
                player = work->players->player;
                CharMotionShiftSet(player->character, player->motionIds[6], 0.0f, 5.0f, 0U);
                fn_1_BE08(fn_1_9CA0, work->selected);
                work->selected->winTimer = 0;
                HuAudFXPlay(MSM_SE_GUIDE_75);
                CharFXPlay(player->character, MSM_SE_M680_WIN_CUE);
                Hu3DMotionShiftSet(work->selected->model, work->selected->motions[5], 0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
                work->roundWins++;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_VICTORY_MOTION:
            player = work->players->player;
            if (player->motionIds[6] == Hu3DMotionIDGet(player->modelId)) {
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_VICTORY_END:
            player = work->players->player;
            if (Hu3DMotionEndCheck(player->modelId) != 0) {
                fn_1_76E8(work, 0);
                work->state = M680_SEQUENCE_STATE_ROUND_RESULT;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_LOSS_START:
            HuAudFXPlay(MSM_SE_M680_PIECE_LOSS);
            fn_1_BE08(fn_1_9F18, work->selected);
            node = work->players->next;
            while (node) {
                player = node->player;
                fn_1_BCBC(player);
                node = node->next;
            }
            work->selected->loseTimer = 0;
            work->frame = 0;
            work->state += 1;
            /* fallthrough */
        case M680_SEQUENCE_STATE_LOSS_DELAY:
            work->frame++;
            if (work->frame == M680_LOSS_EFFECT_START_FRAME) {
                HuAudFXPlay(MSM_SE_M680_LOSS_EFFECT);
                fn_1_9D5C(&work->phaseEffect);
            }
            if (work->frame >= M680_PLAYER_ELIMINATION_START_FRAME) {
                player = work->players->player;
                omVibrate(player->playerIndex, M680_LOSS_RUMBLE_SETTING,
                          M680_LOSS_RUMBLE_DURATION, 0);
                CharMotionShiftSet(player->character, player->motionIds[5], 0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftSet(work->selected->model, work->selected->motions[7], 0.0f, 5.0f,
                                   0U);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_PLAYER_ELIMINATION:
            work->frame++;
            player = work->players->player;
            player->position.y += 60.0f;
            player->position.x -= 20.0f;
            player->position.z += 10.0f;
            work->selected->position.y += 60.0f;
            work->selected->position.x += 20.0f;
            work->selected->position.z += 10.0f;
            if (work->frame >= M680_PICKUP_ANIMATION_FRAMES) {
                node = work->players->next;
            while (node) {
                    player = node->player;
                    CharMotionShiftSet(player->character, player->motionIds[0], 0.0f, 15.0f,
                                       HU3D_MOTATTR_LOOP);
                    node = node->next;
                }
                player = work->players->player;
                Hu3DModelAttrSet(work->selected->model, HU3D_ATTR_DISPOFF);
                work->selected->state = 0;
                player->eliminated = 1;
                work->eliminatedPlayers[work->eliminatedCount] = (s32) player->playerIndex;
                work->eliminatedCount++;
                CharModelAttrSet(player->character, 1U);
                fn_1_76E8(work, 1);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ELIMINATION_DELAY:
            work->frame++;
            if (work->frame >= M680_PICKUP_ANIMATION_FRAMES) {
                work->state = M680_SEQUENCE_STATE_ROUND_RESULT;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ROUND_RESULT:
            count = 0;
            for (node = work->players; node;
                 node = node->next) {
                count++;
            }
            work->playerCount = count;
            if (count == 1) {
                work->state = M680_SEQUENCE_STATE_ADVANCE_MINIGAME;
                break;
            }
            if (work->roundWins >= 5) {
                work->state = M680_SEQUENCE_STATE_FINAL_ROUND;
                break;
            }
            work->state = M680_SEQUENCE_STATE_NEXT_ROUND_SETUP;
            break;
        case M680_SEQUENCE_STATE_NEXT_ROUND_SETUP:
            node = work->players;
            player = node->player;
            fn_1_B844(player, 4, 0);
            node = node->next;
            count = work->playerCount;
            for (index = 1; index < count; index++) {
                player = node->player;
                fn_1_B844(player, index, 0);
                node = node->next;
            }
            work->selected->state = M680_PIECE_STATE_NEXT_ROUND;
            work->state += 1;
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_PIECE_RESET:
            if (work->selected->state == 0) {
                Hu3DModelAttrSet(work->selected->model, HU3D_ATTR_DISPOFF);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ROUND_TRANSITION_DELAY:
            work->frame++;
            if (work->frame >= M680_ROUND_TRANSITION_DELAY_FRAMES) {
                if (work->result != 0) {
                    HuAudFXPlay(MSM_SE_M680_STAGE_PROGRESS);
                    fn_1_BE08(fn_1_A0E4, work);
                    work->transitionTimer = 0;
                    work->frame = 0;
                    work->state += 1;
                    break;
                }
                work->state = M680_SEQUENCE_STATE_SELECTION_SETUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_ROUND_TRANSITION:
            work->frame++;
            if (work->frame >= M680_VICTORY_DELAY_FRAMES) {
                if (work->roundWins >= 5) {
                    work->state = M680_SEQUENCE_STATE_FINAL_ROUND;
                    break;
                }
                work->state = M680_SEQUENCE_STATE_SELECTION_SETUP;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_SET_WINNING_PIECES:
            for (index = 0; index < M680_GRID_NODE_COUNT; index++) {
                if (work->grid[index].ready == 0) {
                    work->grid[index].state = M680_PIECE_STATE_WINNER;
                }
            }
            fn_1_BE08(fn_1_8288, &work->stage);
            work->stage.state = 0;
            work->frame = 0;
            work->state += 1;
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_FINAL_STAGE_TURN:
            work->frame++;
            if (work->frame == M680_SEQUENCE_DELAY_60_FRAMES) {
                HuAudFXPlay(MSM_SE_M680_STAGE_PROGRESS);
                fn_1_BE08(fn_1_A234, work);
                work->transitionTimer = 0;
                for (index = 0; index < 5; index++) {
                    Hu3DModelAttrReset(work->markers[index], HU3D_MOTATTR_PAUSE);
                    Hu3DModelAttrSet(work->markers[index], HU3D_MOTATTR_LOOP);
                    Hu3DMotionTimeSet(work->markers[index], 0.0f);
                }
            }
            for (index = 0; index < M680_GRID_NODE_COUNT; index++) {
                if (work->grid[index].state != 0) {
                    break;
                }
            }
            if (index == M680_GRID_NODE_COUNT) {
                Hu3DModelAttrSet(work->models[5], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(work->models[6], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(work->models[3], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(work->models[4], HU3D_ATTR_DISPOFF);
                if (work->frame < M680_SEQUENCE_DELAY_60_FRAMES) {
                    HuAudFXPlay(MSM_SE_M680_STAGE_PROGRESS);
                    fn_1_BE08(fn_1_A234, work);
                    work->transitionTimer = 0;
                    for (index = 0; index < 5; index++) {
                        Hu3DModelAttrReset(work->markers[index], HU3D_MOTATTR_PAUSE);
                        Hu3DModelAttrSet(work->markers[index], HU3D_MOTATTR_LOOP);
                        Hu3DMotionTimeSet(work->markers[index], 0.0f);
                    }
                }
                fn_1_6904(work);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_STAGE_TURN_END:
            if (work->transitionTimer >= M680_STAGE_TRANSITION_FRAMES) {
                fn_1_99B4(&work->transitions[0], M680_SEQUENCE_DELAY_15_FRAMES, 0);
                work->gridIndex = 0;
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_START_FIRST_GROUP_REVEAL:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_15_FRAMES) {
                fn_1_99B4(&work->transitions[1], M680_GRID_NODE_COUNT, 0);
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_REVEAL_FIRST_GROUP:
            work->frame++;
            if (work->frame >= M680_STAGE_MARKER_STEP_FRAMES) {
                count = 9 - work->gridIndex;
                work->grid[count].state = M680_PIECE_STATE_FIRST_GROUP;
                Hu3DModelAttrReset(work->grid[count].model, HU3D_ATTR_DISPOFF);
                work->gridIndex += 1;
                work->frame = 0;
                if (work->gridIndex >= M680_GRID_NODE_COUNT) {
                    work->state += 1;
                    break;
                }
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_FIRST_GROUP_CLEAR:
            if (work->frame < M680_SEQUENCE_FRAME_LIMIT) {
                work->frame++;
            }
            if (work->frame == M680_VICTORY_DELAY_FRAMES) {
                fn_1_99B4(&work->transitions[0], M680_SEQUENCE_DELAY_15_FRAMES, 1);
            }
            if ((s16) !work->grid[0].state) {
                fn_1_99B4(&work->transitions[1], M680_GRID_NODE_COUNT, 1);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_START_SECOND_GROUP_REVEAL:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_15_FRAMES) {
                Hu3DModelAttrReset(work->models[5], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(work->models[6], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(work->models[3], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(work->models[4], HU3D_ATTR_DISPOFF);
                fn_1_99B4(&work->transitions[2], M680_GRID_NODE_COUNT, 0);
                work->frame = 0;
                work->gridIndex = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_REVEAL_SECOND_GROUP:
            work->frame++;
            if (work->frame >= M680_STAGE_MARKER_STEP_FRAMES) {
                count = 9 - work->gridIndex;
                work->grid[count].state = M680_PIECE_STATE_SECOND_GROUP;
                work->gridIndex += 1;
                work->frame = 0;
                if (work->gridIndex >= M680_GRID_NODE_COUNT) {
                    work->state += 1;
                    break;
                }
            }
            break;
        case M680_SEQUENCE_STATE_WAIT_FOR_SECOND_GROUP_CLEAR:
            if (work->frame < M680_STAGE_REVERSE_FRAME_LIMIT) {
                work->frame++;
            }
            if (work->frame == M680_STAGE_REVERSE_START_FRAME) {
                fn_1_99B4(&work->transitions[2], M680_GRID_NODE_COUNT, 1);
            }
            for (index = 0; index < M680_GRID_NODE_COUNT; index++) {
                if (work->grid[index].state != 0) {
                    break;
                }
            }
            if (index == M680_GRID_NODE_COUNT) {
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_FINAL_STAGE_DELAY:
            work->frame++;
            if (work->frame >= M680_SEQUENCE_DELAY_60_FRAMES) {
                work->state = 0;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_ADVANCE_MINIGAME:
            MgSeqModeNext();
            break;
        case M680_SEQUENCE_STATE_FINAL_ROUND:
            node = work->players;
            player = node->player;
            fn_1_B844(player, 0, 0);
            node = node->next;
            count = work->playerCount;
            for (index = 1; index < count; index++) {
                player = node->player;
                fn_1_B844(player, index, 0);
                node = node->next;
            }
            work->selected->state = M680_PIECE_STATE_NEXT_ROUND;
            work->state += 1;
            break;
        case M680_SEQUENCE_STATE_PLAYER_RESULT_SETUP:
            if (work->selected->state == 0) {
                Hu3DModelAttrSet(work->selected->model, HU3D_ATTR_DISPOFF);
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_PLAYER_RESULT_WAIT:
            work->frame++;
            if (work->frame >= M680_STAGE_TRANSITION_START_FRAMES) {
                HuAudFXPlay(MSM_SE_M680_STAGE_PROGRESS);
                fn_1_BE08(fn_1_A0E4, work);
                work->transitionTimer = 0;
                work->frame = 0;
                work->state += 1;
                break;
            }
            break;
        case M680_SEQUENCE_STATE_STAGE_PROGRESS:
            work->frame++;
            if (work->frame >= M680_STAGE_TRANSITION_WAIT_FRAMES) {
                if (work->round >= 3) {
                    work->state = M680_SEQUENCE_STATE_ADVANCE_MINIGAME;
                    break;
                }
                work->state = M680_SEQUENCE_STATE_SET_WINNING_PIECES;
            }
            break;
    }
}

/* MgSeqCreate calls this results callback to award placement bonuses and fade the background
 * stream. */
void fn_1_15C0(void)
{
    M680State *sequence =
        lbl_1_bss_0->data;
    M680CharacterState *actor;
    s32 actorIndex;
    s32 playerNo;
    s32 activePlayerNo;
    s32 placement;

    switch (sequence->fadeState) {
    case 0:
        for (actorIndex = 0; actorIndex < 4; actorIndex++) {
            actor = sequence->actors[actorIndex];
            if (actor->eliminated == 0) {
                playerNo = actor->playerIndex;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerNo].mgCoinBonus = 0;
                }
            }
        }
        placement = 1;
        for (actorIndex = sequence->eliminatedCount - 1; actorIndex >= 0;
             placement++, actorIndex--) {
            activePlayerNo = sequence->eliminatedPlayers[actorIndex];
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[activePlayerNo].mgCoinBonus = placement;
            }
        }
        HuAudSStreamFadeOut(lbl_1_bss_4, 100);
        sequence->fadeState++;
        break;
    case 1:
    default:
        break;
    }
}

/* MgSeqCreate calls this results callback to animate the remaining players before advancing. */
void fn_1_16E8(void)
{
    M680State *sequence =
        lbl_1_bss_0->data;
    M680CharacterState *actor;
    s32 actorIndex;

    switch (sequence->winnerPhase) {
    case 0:
        MgSeqModeChangeOff();
        sequence->modeTimer = 0;
        for (actorIndex = 0; actorIndex < 4; actorIndex++) {
            actor = sequence->actors[actorIndex];
            if (actor->eliminated == 0) {
                fn_1_BAF8(actor);
            }
        }
        sequence->winnerPhase++;
        break;
    case 1:
        if (sequence->modeTimer == sequence->playerCount) {
            sequence->frame = 0;
            sequence->winnerPhase++;
        }
        break;
    case 2:
        sequence->frame++;
        if (sequence->frame >= 5) {
            MgSeqModeNext();
        }
        break;
    default:
        break;
    }
}

/* MgSeqCreate calls this winner callback each frame to run the final character and piece
 * animations. */
void fn_1_17E4(void)
{
    M680State *sequence =
        lbl_1_bss_0->data;
    M680CharacterState *character;
    s32 index;

    switch (sequence->finishPhase) {
    case 0:
        for (index = 0; index < 4; index++) {
            character = sequence->actors[index];
            if (character->eliminated == 0) {
                CharMotionShiftSet(character->character, character->motionIds[7],
                                   0.0f, 5.0f, 0);
            }
        }
        for (index = 0; index < 10; index++) {
            if (sequence->grid[index].ready == 0) {
                Hu3DMotionShiftSet(sequence->grid[index].model,
                                   sequence->grid[index].motions[5],
                                   0.0f, 5.0f, HU3D_MOTATTR_LOOP);
                fn_1_BE08((M680Callback)fn_1_9BDC,
                          &sequence->grid[index]);
                sequence->grid[index].handTimer = 0;
            }
        }
                HuAudJinglePlay(M680_WIN_JINGLE_ID);
        sequence->frame = 0;
        sequence->finishPhase++;
        break;
    case 1:
        sequence->frame++;
        fn_1_9754(sequence);
        if (sequence->frame >= 30) {
            sequence->finishPhase++;
        }
        break;
    case 2:
        break;
    default:
        break;
    }
}

/* The fade-out sequence has no additional action. */
void fn_1_1974(void) { }

/* MgSeqCreate calls this exit callback to remove the round timer when the minigame closes. */
void fn_1_1978(void) {
    M680State *sequence;

    sequence = lbl_1_bss_0->data;
    MgTimerKill(sequence->timer);
}

/* Sets the perspective camera and its orthogonal up direction during stage setup. */
void fn_1_19B4(void)
{
    Vec eye = {0.0f, 766.39f, 2312.93f};
    Vec target = {0.0f, -165.4f, -42.98f};
    Vec direction;
    Vec correctedUp;
    Vec right;
    Vec up = {0.0f, 1.0f, 0.0f};

    Hu3DCameraCreate(lbl_1_rodata_10[0]);
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[0], 45.0f,
                             20.0f, 20000.0f, 1.2f);
    PSVECSubtract(&target, &eye, &direction);
    PSVECNormalize(&direction, &direction);
    PSVECCrossProduct(&up, &direction, &right);
    PSVECNormalize(&right, &right);
    PSVECCrossProduct(&direction, &right, &correctedUp);
    PSVECNormalize(&correctedUp, &correctedUp);
    Hu3DCameraPosSet(lbl_1_rodata_10[0], eye.x, eye.y, eye.z, correctedUp.x,
                     correctedUp.y, correctedUp.z, target.x, target.y, target.z);
}

/* Creates the stage's directional light and ambient illumination. */
void fn_1_1B0C(void)
{
    Vec target = {0.0f, 2000.0f, 8000.0f};
    Vec position = {0.0f, 0.0f, 0.0f};
    Vec direction;
    HU3D_LIGHTID light;

    PSVECSubtract(&position, &target, &direction);
    PSVECNormalize(&direction, &direction);
    Hu3DLighInit();
    light = Hu3DGLightCreate(target.x, target.y, target.z,
                             direction.x, direction.y, direction.z,
                             M680_LIGHT_AMBIENT_COMPONENT, M680_LIGHT_AMBIENT_COMPONENT,
                             M680_LIGHT_AMBIENT_COMPONENT);
    Hu3DAmbColorSet(1.0f, 1.0f, 1.0f);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
}

/* Builds the shadow view and sets its color during stage setup. */
void fn_1_1BF8(void)
{
    Vec correctedUp;
    Vec direction;
    Vec right;
    Vec up = {0.0f, 1.0f, 0.0f};
    Vec position = {0.0f, 2000.0f, 1000.0f};
    Vec target = {0.0f, 0.0f, 1000.1f};

    PSVECSubtract(&target, &position, &direction);
    PSVECNormalize(&direction, &direction);
    PSVECCrossProduct(&up, &direction, &right);
    PSVECNormalize(&right, &right);
    PSVECCrossProduct(&direction, &right, &correctedUp);
    PSVECNormalize(&correctedUp, &correctedUp);
    Hu3DShadowCreate(45.0f, 20.0f, 20000.0f);
    Hu3DShadowPosSet(&position, &correctedUp, &target);
    Hu3DShadowColSet(M680_SHADOW_COLOR_RED, M680_SHADOW_COLOR_GREEN, M680_SHADOW_COLOR_BLUE);
}

/* Initializes model and motion handles, actor pointers, phase flags, input buttons and elimination
 * records before board setup. */
void fn_1_1D10(M680State *state)
{
    s32 index;
    s16 nightMode;

    for (index = 0; index < 63; index++) {
        state->models[index] = -1;
    }
    for (index = 0; index < 15; index++) {
        state->motions[index] = -1;
    }
    for (index = 0; index < 4; index++) {
        state->actors[index] = 0;
    }
    nightMode = GwMgNightF;
    state->nightMode = (nightMode == 1);
    state->state = 0;
    state->openingPhase = 0;
    state->musicStarted = 0;
    state->winnerPhase = 0;
    state->fadeState = 0;
    state->finishPhase = 0;
    state->buttons.pressed = state->buttons.previous = state->buttons.current = 0;
    for (index = 0; index < 4; index++) {
        state->eliminatedPlayers[index] = -1;
    }
    state->eliminatedCount = 0;
}

/* The root object callback initializes the board and callback pool, then installs its frame
 * update. */
void fn_1_1E2C(OMOBJ *state)
{
    M680State *sequence = lbl_1_bss_0->data;

    fn_1_65FC(sequence);
    fn_1_33D0(sequence);
    fn_1_1EEC(sequence);
    fn_1_7788(sequence);
    fn_1_6624(sequence);
    fn_1_BD64();
    state->objFunc = fn_1_1EA4;
}

/* The stage object's per-frame callback updates pieces, background motion and active callbacks. */
void fn_1_1EA4(void) {
    M680State *sequence;

    sequence = lbl_1_bss_0->data;
    fn_1_390C(sequence);
    fn_1_8AAC(sequence);
    fn_1_BEFC();
}

/* Board setup calls this once to load stage models and prepare their material colors. */
void fn_1_1EEC(M680State *state) {
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    s16 modelId;
    s32 index;
    s32 materialCount;
    if (state->models[42] < 0) {
        modelId = state->models[42] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[42], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[42]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[0] < 0) {
        modelId = state->models[0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[0], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[0]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[1] < 0) {
        modelId = state->models[1] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[1], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[1]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[41] < 0) {
        modelId = state->models[41] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[41], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[41]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(modelId);
    if (state->models[2] < 0) {
        modelId = state->models[2] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[2], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[2]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(modelId);
    if (state->models[3] < 0) {
        modelId = state->models[3] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[3], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[3]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[4] < 0) {
        modelId = state->models[4] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[4], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[4]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(modelId);
    if (state->models[5] < 0) {
        modelId = state->models[5] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[5], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[5]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[6] < 0) {
        modelId = state->models[6] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[6], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[6]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(modelId);
    if (state->models[7] < 0) {
        modelId = state->models[7] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[7], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[7]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[8] < 0) {
        modelId = state->models[8] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[8], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[8]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[9] < 0) {
        modelId = state->models[9] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[9], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[9]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    state->gate.motionId = modelId;
    state->gate.busy = 0;
    if (state->models[11] < 0) {
        modelId = state->models[11] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[11], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[11]);
    }
    state->transitions[2].motionId = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[10] < 0) {
        modelId = state->models[10] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[10], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[10]);
    }
    state->transitions[1].motionId = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[12] < 0) {
        modelId = state->models[12] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[12], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[12]);
    }
    state->transitions[0].motionId = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[13] < 0) {
        modelId = state->models[13] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[13], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[13]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[62] < 0) {
        modelId = state->models[62] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[62], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[62]);
    }
    state->cursorModel = modelId;
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelScaleSet(modelId, 0.6f, 0.6f, 0.6f);
    hsf = Hu3DData[modelId].hsf;
    material = hsf->material;
    materialCount = hsf->materialNum;
    for (index = 0; index < materialCount; index++, material++) {
        material->color[0] = M680_PIECE_COLOR_WHITE;
        material->color[1] = M680_PIECE_COLOR_HIGHLIGHT;
        material->color[2] = M680_PIECE_COLOR_HIGHLIGHT;
    }
    if (state->models[36] < 0) {
        modelId = state->models[36] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[36], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[36]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[37] < 0) {
        modelId = state->models[37] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[37], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[37]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 263.0781f, -772.693f);
    Hu3DModelRotSet(modelId, 0.0f, -30.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    for (index = 0; index < 5; index++) {
        if (state->models[38] < 0) {
            modelId = state->models[38] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_370[38], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            modelId = Hu3DModelLink(state->models[38]);
        }
        state->markers[index] = modelId;
        Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(modelId, 0);
        Hu3DModelPosSet(modelId, 230.0f - (115.0f * (f32) index), 0.0f, 0.0f);
        Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    }
    if (state->models[59] < 0) {
        modelId = state->models[59] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[59], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[59]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[61] < 0) {
        modelId = state->models[61] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[61], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[61]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    if (state->models[60] < 0) {
        modelId = state->models[60] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_370[60], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        modelId = Hu3DModelLink(state->models[60]);
    }
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(modelId, 0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    for (index = 0; index < 3; index++) {
        if (state->models[34] < 0) {
            modelId = state->models[34] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_370[34], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            modelId = Hu3DModelLink(state->models[34]);
        }
        state->phaseEffect.models[index] = modelId;
        Hu3DModelAttrReset(modelId, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(modelId, 6);
        Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Creates each moving piece, its motions and its attached item models. */
void fn_1_33D0(M680State *state)
{
    s16 motionIndex[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    s32 objectIndex;
    s32 motion;
    s16 motionId;
    s16 model;

    state->gridIndex = 10;
    state->repath = 0;

    for (objectIndex = 0; objectIndex < 10; objectIndex++) {
        state->grid[objectIndex].position.x = 0.0f;
        state->grid[objectIndex].position.y = 0.0f;
        state->grid[objectIndex].position.z = 0.0f;
        state->grid[objectIndex].rotation.x = 0.0f;
        state->grid[objectIndex].rotation.y = 0.0f;
        state->grid[objectIndex].rotation.z = 0.0f;
        state->grid[objectIndex].targetRotation = 0.0f;
        if (state->models[objectIndex + M680_PIECE_MODEL_BASE] < 0) {
            model = state->models[objectIndex + M680_PIECE_MODEL_BASE] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_370[objectIndex + M680_PIECE_MODEL_BASE],
                                     HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[objectIndex + M680_PIECE_MODEL_BASE]);
        }
        for (motion = 0; motion < 8; motion++) {
            if (state->motions[motionIndex[motion]] < 0) {
                motionId = state->motions[motionIndex[motion]] =
                    Hu3DJointMotion(model,
                        HuDataSelHeapReadNum(lbl_1_data_46C[motionIndex[motion]],
                                             HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                motionId = state->motions[motionIndex[motion]];
            }
            state->grid[objectIndex].motions[motion] = motionId;
        }
        Hu3DMotionSet(model, state->grid[objectIndex].motions[0]);
        state->grid[objectIndex].model = model;
        Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSetV(model, &state->grid[objectIndex].position);
        Hu3DModelRotSetV(model, &state->grid[objectIndex].rotation);
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        Hu3DModelShadowSet(model);
        fn_1_395C(&state->grid[objectIndex]);

        if (state->models[objectIndex + M680_HOOK_MODEL_BASE] < 0) {
            model = state->models[objectIndex + M680_HOOK_MODEL_BASE] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_370[objectIndex + M680_HOOK_MODEL_BASE],
                                     HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[objectIndex + M680_HOOK_MODEL_BASE]);
        }
        state->grid[objectIndex].hookModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        Hu3DModelHookSet(state->grid[objectIndex].model,
                         "itemhook_R", model);

        if (state->models[objectIndex + M680_ITEM_MODEL_BASE] < 0) {
            model = state->models[objectIndex + M680_ITEM_MODEL_BASE] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_370[objectIndex + M680_ITEM_MODEL_BASE],
                                     HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[objectIndex + M680_ITEM_MODEL_BASE]);
        }
        state->grid[objectIndex].itemModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, 800.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        Hu3DModelHookSet(state->grid[objectIndex].hookModel,
                         "hook_futa", model);
    }
    fn_1_73EC(state);
}

/* Updates all ten moving pieces once per frame. */
void fn_1_390C(M680State *state) {
    s32 index = 0;
    while (index < 10) {
        fn_1_39D0(&state->grid[index]);
        index++;
    }
}

/* Starts a piece at a random idle-motion frame and clears its motion state. */
void fn_1_395C(M680GridNode *work)
{
    work->state = 0;
    Hu3DMotionTimeSet(work->model, 12.0f * (f32)frandmod(5));
}

/* Advances one piece through movement, path searches and its round-result animations. */
void fn_1_39D0(M680GridNode *work) {
    f32 progress;
    s32 previousColumn;
    s32 previousRow;
    M680State *board;

    board = lbl_1_bss_0->data;
    switch (work->state) {
    case 0:
        break;
    case M680_PIECE_STATE_MOVE_START:
        board->gate.busy = 0;
        work->steps = 0;
        work->searchAttempts = 5;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_MOVE_SELECT:
        previousColumn = work->column;
        previousRow = work->row;
        if (fn_1_62FC(work) != 0) {
            Hu3DMotionShiftSet(work->model, work->motions[4], 0.0f, 5.0f, 0U);
            board->cells[work->column][work->row].state = 1;
            work->start = work->position;
            work->destination = board->cells[work->column][work->row].position;
            work->frames = 0;
            work->state = M680_PIECE_STATE_ROUND_WIN;
        } else {
            board->cells[work->column][work->row].state = 0;
            fn_1_6B38(work);
            board->cells[work->column][work->row].state = 1;
            if ((work->column == previousColumn) && (work->row == previousRow)) {
                if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
                    (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
                    Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f,
                                       HU3D_MOTATTR_LOOP);
                }
            } else if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
                       (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
            work->start = work->position;
            work->destination = board->cells[work->column][work->row].position;
            work->frames = 0;
            work->state++;
        case M680_PIECE_STATE_MOVE:
            work->frames++;
            progress = (f32) work->frames / 12.0f;
            work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
            work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
            work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
            if (work->frames >= M680_PIECE_MOVE_FRAMES) {
                work->steps++;
                if (work->steps >= M680_PIECE_PATH_MAX_STEPS) {
                    work->state++;
                    } else if ((work->column == 9) && (work->row == 7) &&
                               (work->steps < M680_PIECE_MOVE_FRAMES)) {
                    if ((board->gate.busy == 0) && (board->cells[10][7].state == 0) &&
                        (board->cells[10][8].state == 0)) {
                        board->cells[10][7].state = 1;
                        board->gate.busy = 1;
                        work->state = M680_PIECE_STATE_GATE_APPROACH;
                    } else {
                        work->state = 2;
                    }
                } else {
                    work->state = 2;
                }
            }
        }
        break;
    case M680_PIECE_STATE_PATH_SEARCH:
        if (work->searchAttempts >= 5) {
            work->pathLength = fn_1_7140(work);
        } else {
            work->pathLength = fn_1_6DCC(work);
        }
        if (work->pathLength > 0) {
            if (work->pathLength == 1) {
                work->state = 9;
                break;
            } else {
                work->searchAttempts = 0;
                work->pathIndex = 0;
                work->state++;
            }
        } else {
            work->state = 7;
            break;
        }
        /* fallthrough */
    case M680_PIECE_STATE_PATH_STEP:
        if (fn_1_70BC(work) < 0) {
            work->state = 7;
            break;
        }
            board->cells[work->column][work->row].state = 0;
            work->targetRotation = 90.0f * (f32) work->path[work->pathIndex].direction;
            work->pathIndex++;
            work->column = work->path[work->pathIndex].column;
            work->row = work->path[work->pathIndex].row;
            board->cells[work->column][work->row].state = 1;
            work->start = work->position;
            work->destination = board->cells[work->column][work->row].position;
            work->frames = 0;
            work->state++;
            if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
        /* fallthrough */
    case M680_PIECE_STATE_PATH_MOVE:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            if ((work->column == work->targetColumn) && (work->row == work->targetRow)) {
                work->state = 9;
            } else if (board->repath != 0) {
                board->repath = 0;
                work->state = 4;
            } else if (work->pathIndex == work->pathLength - 1) {
                work->state = 4;
            } else {
                work->state = 5;
            }
        }
        break;
    case M680_PIECE_STATE_PATH_RETRY:
        if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->searchAttempts++;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_PATH_RETRY_WAIT:
        work->frames++;
        if (work->frames >= M680_PIECE_RETRY_WAIT_FRAMES) {
            work->state = 4;
        }
        break;
    case M680_PIECE_STATE_TURN_TO_IDLE:
        board->gridIndex++;
        if (board->gridIndex == 9) {
            board->repath = 1;
        }
        work->targetRotation = 0.0f;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_IDLE_WAIT:
        if ((work->rotation.y > -1.0f) && (work->rotation.y < 1.0f)) {
            if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
            work->state = 0;
        }
        break;
    case M680_PIECE_STATE_GATE_APPROACH:
        if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->frames = 0;
        work->direction = 1;
        work->targetRotation = 90.0f * (f32)work->direction;
        fn_1_A5F4(&board->gate.elapsedFrames, 0);
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_GATE_APPROACH_WAIT:
        work->frames++;
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->steps++;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_GATE_CROSSING_START:
        board->cells[work->column][work->row].state = 0;
        work->column = M680_PIECE_GATE_COLUMN;
        work->row = 7;
        work->searchAttempts = 0;
        if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_GATE_CROSSING:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->steps++;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_GATE_EXIT:
        work->column = M680_PIECE_GATE_END_COLUMN;
        work->row = 7;
        work->direction = 1;
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_GATE_EXIT_MOVE:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->steps++;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_GATE_EXIT_WAIT:
        work->column = M680_PIECE_STAGE_END_COLUMN;
        work->row = 7;
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        fn_1_A5F4(&board->gate.elapsedFrames, 1);
        work->frames = 0;
        work->state++;
        break;
    case M680_PIECE_STATE_GATE_RETURN:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->steps++;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_GATE_RETURN_MOVE:
        if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->direction = 3;
        work->targetRotation = 270.0f;
        work->rotation.y = work->targetRotation;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_GATE_RETURN_WAIT:
        work->frames++;
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->steps++;
            if (board->cells[10][7].state == 0) {
                board->cells[10][7].state = 1;
                work->state++;
            } else {
                work->state = M680_PIECE_STATE_GATE_RETURN_MOVE;
            }
        }
        break;
    case M680_PIECE_STATE_GATE_COMPLETE:
        work->column = M680_PIECE_GATE_COLUMN;
        work->row = 7;
        if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        fn_1_A5F4(&board->gate.elapsedFrames, 0);
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_RESULT_APPROACH:
        work->frames++;
        progress = (f32) work->frames / 24.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= 24) {
            work->steps += 2;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_RESULT_MOVE:
        if ((board->cells[9][7].state == 0) && (board->cells[10][8].state == 0)) {
            if (frandmod(2) == 0) {
                work->column = 9;
                work->row = 7;
                work->direction = 3;
            } else {
                work->column = M680_PIECE_GATE_COLUMN;
                work->row = 8;
                work->direction = 0;
            }
        } else if (board->cells[9][7].state == 0) {
            work->column = 9;
            work->row = 7;
            work->direction = 3;
        } else if (board->cells[10][8].state == 0) {
            work->column = M680_PIECE_GATE_COLUMN;
            work->row = 8;
            work->direction = 0;
        } else {
            work->state = M680_PIECE_STATE_RESULT_FINISH;
            break;
        }
        board->cells[work->column][work->row].state = 1;
        if ((work->column == M680_PIECE_GATE_COLUMN) && (work->row == 7)) {
            if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
        } else if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
                   (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        work->targetRotation = 90.0f * (f32) work->direction;
        work->frames = 0;
        work->state++;
    case M680_PIECE_STATE_RESULT_WAIT:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            fn_1_A5F4(&board->gate.elapsedFrames, 1);
            work->steps++;
            work->state = 2;
        }
        break;
    case M680_PIECE_STATE_RESULT_FINISH:
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_RESULT_RETRY_WAIT:
        work->frames++;
        if (work->frames >= M680_PIECE_RETRY_WAIT_FRAMES) {
            work->steps++;
            work->state = M680_PIECE_STATE_RESULT_MOVE;
        }
        break;
    case M680_PIECE_STATE_ROUND_WIN:
        work->frames++;
        progress = (f32) work->frames / 29.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= 29) {
            fn_1_646C(work);
            Hu3DModelAttrSet(work->model, HU3D_MOTATTR_PAUSE);
            work->frames = 0;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_ROUND_WIN_WAIT:
        work->frames++;
        if (work->frames >= 24) {
            Hu3DModelAttrReset(work->model, HU3D_MOTATTR_PAUSE);
            work->frames = 0;
            work->state++;
        }
        break;
    case M680_PIECE_STATE_WIN_RETRY_WAIT:
        work->frames++;
        if (work->frames >= M680_STAGE_TRANSITION_START_FRAMES) {
            work->steps += 7;
            work->state = 2;
        }
        break;
    case M680_PIECE_STATE_SELECTED:
        work->targetColumn = 7;
        work->targetRow = M680_BOARD_EXIT_ROW;
        board->cells[work->column][work->row].state = 0;
        work->pathLength = fn_1_6DCC(work);
        work->pathIndex = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_SELECTED_PATH_START:
        work->targetRotation = 90.0f * (f32) work->path[work->pathIndex].direction;
        work->pathIndex++;
        work->column = work->path[work->pathIndex].column;
        work->row = work->path[work->pathIndex].row;
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        work->frames = 0;
        work->state++;
        if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        /* fallthrough */
    case M680_PIECE_STATE_SELECTED_PATH_STEP:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            if ((work->column == work->targetColumn) && (work->row == work->targetRow)) {
                work->state++;
            } else {
                work->state = M680_PIECE_STATE_SELECTED_PATH_START;
            }
        }
        break;
    case M680_PIECE_STATE_SELECTED_PATH_MOVE:
        work->targetRotation = -90.0f;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_SELECTED_TURN:
        if ((work->rotation.y > -91.0f) && (work->rotation.y < -89.0f)) {
            if ((work->motions[6] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[6] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[6], 0.0f, 5.0f, 0U);
            }
            work->state = 0;
        }
        break;
    case M680_PIECE_STATE_WINNER:
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_WINNER_START:
        if ((work->row < M680_BOARD_LAST_CELL) &&
            (board->cells[work->column][work->row + 1].state == 0)) {
            board->cells[work->column][work->row].state = 0;
            work->row++;
            board->cells[work->column][work->row].state = 1;
            work->start = work->position;
            work->destination = board->cells[work->column][work->row].position;
            work->targetRotation = 0.0f;
            work->frames = 0;
            work->state++;
            if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
        } else if ((work->column < M680_BOARD_LAST_CELL) &&
                   (board->cells[work->column + 1][work->row].state == 0)) {
            board->cells[work->column][work->row].state = 0;
            work->column++;
            board->cells[work->column][work->row].state = 1;
            work->start = work->position;
            work->destination = board->cells[work->column][work->row].position;
            work->targetRotation = 90.0f;
            work->frames = 0;
            work->state++;
            if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
                (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
                Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            }
        } else {
            work->state = M680_PIECE_STATE_WINNER_END;
            break;
        }
        /* fallthrough */
    case M680_PIECE_STATE_WINNER_MOVE:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            if ((work->column == M680_BOARD_LAST_CELL) &&
                (work->row == M680_BOARD_LAST_CELL)) {
                work->state = M680_PIECE_STATE_WINNER_FINISH;
            } else {
                work->state = M680_PIECE_STATE_WINNER_START;
            }
        }
        break;
    case M680_PIECE_STATE_WINNER_END:
        if ((work->motions[0] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[0] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_WINNER_WAIT:
        work->frames++;
        if (work->frames >= M680_PIECE_RETRY_WAIT_FRAMES) {
            work->state = M680_PIECE_STATE_WINNER_START;
        }
        break;
    case M680_PIECE_STATE_WINNER_FINISH:
        board->cells[work->column][work->row].state = 0;
        work->state = 0;
        break;
    case M680_PIECE_STATE_NEXT_ROUND:
        work->column++;
        work->start = work->position;
        work->destination = board->cells[work->column][work->row].position;
        work->targetRotation = 90.0f;
        work->frames = 0;
        work->state++;
        if ((work->motions[2] != Hu3DMotionIDGet(work->model)) &&
            (work->motions[2] != Hu3DMotionShiftIDGet(work->model))) {
            Hu3DMotionShiftSet(work->model, work->motions[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        /* fallthrough */
    case M680_PIECE_STATE_NEXT_ROUND_START:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            if (work->column == M680_BOARD_LAST_CELL) {
                work->state = 0;
            } else {
                work->state = M680_PIECE_STATE_NEXT_ROUND;
            }
        }
        break;
    case M680_PIECE_STATE_FIRST_GROUP:
        work->position.x = 570.37006f;
        work->position.y = 225.41634f;
        work->position.z = -852.6945f;
        work->start = work->position;
        work->destination.x = 570.37006f;
        work->destination.y = 225.41634f;
        work->destination.z = -286.39212f;
        work->targetRotation = 0.0f;
        work->rotation.y = 0.0f;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_FIRST_GROUP_START:
        work->frames++;
        progress = (f32) work->frames / 48.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_GROUP_REVEAL_FRAMES) {
            work->state++;
        }
        break;
    case M680_PIECE_STATE_FIRST_GROUP_MOVE:
        work->start = work->position;
        work->destination.x = -548.4001f;
        work->destination.y = 225.41634f;
        work->destination.z = -286.39212f;
        work->targetRotation = -90.0f;
        work->frames = 0;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_FIRST_GROUP_TURN:
        work->frames++;
        progress = (f32) work->frames / 96.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_GROUP_EXIT_FRAMES) {
            Hu3DModelAttrSet(work->model, HU3D_ATTR_DISPOFF);
            work->state++;
        }
        break;
    case M680_PIECE_STATE_FIRST_GROUP_EXIT:
        work->state = 0;
        break;
    case M680_PIECE_STATE_SECOND_GROUP:
        work->position.x = -548.4001f;
        work->position.y = 0.0f;
        work->position.z = -286.39212f;
        work->start = work->position;
        work->destination.x = -412.5f;
        work->destination.y = 0.0f;
        work->destination.z = -286.775f;
        work->targetRotation = 90.0f;
        work->rotation.y = 90.0f;
        Hu3DModelAttrReset(work->model, HU3D_ATTR_DISPOFF);
        work->frames = 0;
        work->state++;
        break;
    case M680_PIECE_STATE_SECOND_GROUP_START:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->state++;
        }
        break;
    case M680_PIECE_STATE_SECOND_GROUP_MOVE:
        work->start = work->position;
        work->destination.x = -412.5f;
        work->destination.y = 0.0f;
        work->destination.z = -46.27499f;
        work->frames = 0;
        work->targetRotation = 0.0f;
        work->state++;
        /* fallthrough */
    case M680_PIECE_STATE_SECOND_GROUP_TURN:
        work->frames++;
        progress = (f32) work->frames / 12.0f;
        work->position.x = work->destination.x * progress + work->start.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->start.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->start.z * (1.0f - progress);
        if (work->frames >= M680_PIECE_MOVE_FRAMES) {
            work->column = 3;
            work->row = 4;
            work->direction = 0;
            work->searchAttempts = 0;
            board->cells[work->column][work->row].state = 1;
            work->state = 4;
        }
        break;
    }
    fn_1_613C(work);
    Hu3DModelPosSetV(work->model, &work->position);
    Hu3DModelRotSetV(work->model, &work->rotation);
}

/* Turns a piece toward its target heading by at most fifteen degrees each frame. */
void fn_1_613C(M680GridNode *state) {
    f32 difference;

    while (state->targetRotation > 180.0f) {
        state->targetRotation -= 360.0f;
    }
    while (state->targetRotation < -180.0f) {
        state->targetRotation += 360.0f;
    }
    while (state->rotation.y > 180.0f) {
        state->rotation.y -= 360.0f;
    }
    while (state->rotation.y < -180.0f) {
        state->rotation.y += 360.0f;
    }
    difference = state->targetRotation - state->rotation.y;
    while (difference > 180.0f) {
        difference -= 360.0f;
    }
    while (difference < -180.0f) {
        difference += 360.0f;
    }
    if (difference > 0.0f) {
        if (difference > 15.0f) {
            difference = 15.0f;
        }
    } else if ((difference < 0.0f) && (difference < -15.0f)) {
        difference = -15.0f;
    }
    state->rotation.y += difference;
}

/* Tests a forward step and occasionally moves a piece into the adjacent free cell. */
s32 fn_1_62FC(M680GridNode *piece)
{
    M680State *boardData = lbl_1_bss_0->data;
    s32 rowOffsets[4] = {0, 1, 0, -1};
    s32 columnOffsets[4] = {1, 0, -1, 0};
    s32 column;
    s32 row;

    if (piece->steps < 15 && piece->searchAttempts == 3) {
        column = piece->column + rowOffsets[piece->direction];
        row = piece->row + columnOffsets[piece->direction];
        if (boardData->cells[column][row].state == 0 && frandmod(100) < 30) {
            piece->column = column;
            piece->row = row;
            piece->targetRotation = 90.0f * (f32)piece->direction;
            (piece->searchAttempts)++;
            return 1;
        }
    }
    return 0;
}

/* Clears the board cell immediately behind a piece after its step. */
void fn_1_646C(M680GridNode *piece)
{
    M680State *sequence =
        lbl_1_bss_0->data;
    s32 rowOffsets[4] = {0, 1, 0, -1};
    s32 columnOffsets[4] = {1, 0, -1, 0};
    s32 row;
    s32 column;

    row = piece->column - rowOffsets[piece->direction];
    column = piece->row - columnOffsets[piece->direction];
    sequence->cells[row][column].state = 0;
}

/* Registers a player and sets its starting position and heading. */
void fn_1_6538(M680CharacterState *character, s32 turnOrder)
{
    M680State *sequence =
        lbl_1_bss_0->data;

    sequence->actors[turnOrder] = character;
    character->position.x =
        100.0f + -150.0f * (f32)turnOrder - 700.0f;
    character->position.y = 0.0f;
    character->position.z = 1350.0f;
    character->targetAngle = 180.0f;
    character->rotation.y = 180.0f;
}

/* Builds the player rotation list during stage setup. */
void fn_1_65FC(M680State *state) { fn_1_765C(state); }

/* Builds the board coordinates, marks occupied cells and places the ten pieces. */
void fn_1_6624(M680State *board)
{
    s32 x;
    s32 y;

    for (x = 0; x < 14; x++) {
        for (y = 0; y < 15; y++) {
            board->cells[x][y].position.x = 137.5f * (f32)(x - 1) - 687.5f;
            board->cells[x][y].position.y = 0.0f;
            board->cells[x][y].position.z = 137.5f * (f32)(y - 1) - 458.775f;
            board->cells[x][y].state = lbl_1_data_28[y][x];
        }
    }
    board->cells[7][8].position.y = 3.59992f;
    board->cells[7][9].position.y = 3.59992f;
    board->cells[8][8].position.y = 3.59992f;
    board->cells[8][9].position.y = 3.59992f;
    board->cells[11][7].position.y = 22.0f;

    for (x = 0; x < 10; x++) {
        board->grid[x].column = (x % 5) * 2 + 2;
        board->grid[x].row = (x / 5) * 2 + 9;
        board->grid[x].direction = 0;
        board->grid[x].searchAttempts = 5;
        board->grid[x].position =
            board->cells[board->grid[x].column][board->grid[x].row].position;
        board->cells[board->grid[x].column][board->grid[x].row].state = 1;
        board->grid[x].targetColumn = board->grid[x].column;
        board->grid[x].targetRow = board->grid[x].row;
    }
}

/* Resets the item models, motion times and material colors for the next board transition. */
void fn_1_6904(M680State *work)
{
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    s32 index;

    for (index = 0; index < 10; index++) {
        work->grid[index].targetColumn = (index % 5) * 2 + 2;
        work->grid[index].targetRow = (index / 5) * 2 + 9;
        Hu3DModelAttrReset(work->grid[index].itemModel, HU3D_ATTR_DISPOFF);
        Hu3DModelPosSet(work->grid[index].itemModel, 0.0f, 800.0f, 0.0f);
        if (index == 3) {
            Hu3DMotionTimeSet(work->grid[index].model, 12.0f);
        } else {
            Hu3DMotionTimeSet(work->grid[index].model, 0.0f);
        }
        Hu3DMotionTimeSet(work->grid[index].hookModel, 0.0f);
        Hu3DModelScaleSet(work->grid[index].itemModel, 1.0f, 1.0f, 1.0f);
        if (index < 5) {
            hsf = Hu3DData[work->grid[index].hookModel].hsf;
            material = hsf->material;
            material += 2;
            material->color[0] = M680_PIECE_COLOR_IDLE_RED;
            material->color[1] = M680_PIECE_COLOR_IDLE_GREEN;
            material->color[2] = M680_PIECE_COLOR_IDLE_BLUE;
        }
    }
}

/* Copies the board position at a requested grid coordinate. */
void fn_1_6AE0(Vec *position, const M680PathPoint *pathPoint)
{
    M680State *table =
        lbl_1_bss_0->data;

    *position = table->cells[pathPoint->column][pathPoint->row].position;
}

/* Chooses a free neighboring cell and updates a piece's direction and run length. */
void fn_1_6B38(M680GridNode *work)
{
    M680State *boardData = lbl_1_bss_0->data;
    s32 xOffsets[4] = {0, 1, 0, -1};
    s32 yOffsets[4] = {1, 0, -1, 0};
    s32 candidateDirections[4];
    s32 candidateCount;
    s32 direction;
    s32 offsetIndex;

    candidateCount = 0;
    direction = work->direction;
    for (offsetIndex = 0; offsetIndex < 4; offsetIndex++) {
        s32 column;
        s32 row;

        column = work->column + xOffsets[direction];
        row = work->row + yOffsets[direction];
        if (boardData->cells[column][row].state == 0) {
            candidateDirections[candidateCount] = direction;
            candidateCount++;
        }
        direction++;
        direction %= 4;
    }
    if (candidateCount != 0) {
        if (candidateCount == 1) {
            work->column += xOffsets[candidateDirections[0]];
            work->row += yOffsets[candidateDirections[0]];
            if (work->direction == candidateDirections[0]) {
                work->searchAttempts++;
            } else {
                work->direction = candidateDirections[0];
                work->searchAttempts = 0;
            }
        } else {
            if (candidateDirections[0] != work->direction ||
                frandmod(M680_COMPUTER_CHOICE_PERCENT) <
                    work->searchAttempts * M680_SEARCH_ATTEMPT_PERCENT) {
                offsetIndex = frandmod(candidateCount);
            } else {
                offsetIndex = 0;
            }
            work->column += xOffsets[candidateDirections[offsetIndex]];
            work->row += yOffsets[candidateDirections[offsetIndex]];
            if (work->direction == candidateDirections[offsetIndex]) {
                work->searchAttempts++;
            } else {
                work->direction = candidateDirections[offsetIndex];
                work->searchAttempts = 0;
            }
        }
    }
    work->targetRotation = 90.0f * (f32)work->direction;
}

/* Searches free cells for a path to the piece's target, then clears the search marks. */
s32 fn_1_6DCC(M680GridNode *work)
{
    M680State *boardData = lbl_1_bss_0->data;
    M680PathPoint *path;
    s32 direction;
    s32 nextDirection;
    s32 pathLength;
    s32 steps;
    s32 x;
    s32 y;
    s32 xOffsets[4] = {0, 1, 0, -1};
    s32 yOffsets[4] = {1, 0, -1, 0};

    path = work->path;
    pathLength = 1;
    path->column = work->column;
    path->row = work->row;
    nextDirection = fn_1_700C(work, path->column, path->row);
    direction = (nextDirection < 0) ? 0 : nextDirection;
    while (pathLength > 0) {
        x = path->column;
        y = path->row;
        if (boardData->cells[x][y].state == 0) {
            boardData->cells[x][y].state = 2;
        }
        if (boardData->cells[x][y].state == 1) {
            boardData->cells[x][y].state = 3;
        }
        if (x == work->targetColumn && y == work->targetRow) {
            fn_1_7340(boardData);
            return pathLength;
        }
        nextDirection = fn_1_700C(work, path->column, path->row);
        direction = (nextDirection < 0) ? direction : nextDirection;
        steps = 0;
        while (steps < 4) {
            s32 nextX;
            s32 nextY;

            nextX = x + xOffsets[direction];
            nextY = y + yOffsets[direction];
            if (boardData->cells[nextX][nextY].state == 0) {
                path->direction = direction;
                pathLength++;
                path++;
                path->column = nextX;
                path->row = nextY;
                break;
            }
            direction++;
            direction %= 4;
            steps++;
        }
        if (steps == 4) {
            pathLength--;
            path--;
        }
    }
    fn_1_7340(boardData);
    return 0;
}

/* Chooses the dominant grid direction toward the target, returning -1 on a diagonal tie. */
s32 fn_1_700C(M680GridNode *work, s32 column, s32 row)
{
    s32 horizontalDistance;
    s32 verticalDistance;

    column -= work->targetColumn;
    row -= work->targetRow;
    horizontalDistance = column < 0 ? -column : column;
    verticalDistance = row < 0 ? -row : row;

    if (horizontalDistance == verticalDistance) {
        return -1;
    }
    if (horizontalDistance > verticalDistance) {
        if (column < 0) {
            return 1;
        }
        return 3;
    }
    if (row < 0) {
        return 0;
    }
    return 2;
}

/* Reports whether the next cell in a piece's stored path is occupied. */
s32 fn_1_70BC(M680GridNode *piece)
{
    M680State *sequence = lbl_1_bss_0->data;
    s32 column = piece->path[piece->pathIndex + 1].column;
    s32 row = piece->path[piece->pathIndex + 1].row;

    if (sequence->cells[column][row].state == 0) {
        return 0;
    }
    return -1;
}

/* Chooses a possible first step and records it for the piece's path. */
s32 fn_1_7140(M680GridNode *work)
{
    M680State *boardData = lbl_1_bss_0->data;
    s32 xOffsets[4] = {0, 1, 0, -1};
    s32 yOffsets[4] = {1, 0, -1, 0};
    s32 candidateDirections[4];
    s32 offsetIndex;
    s32 direction;
    s32 candidateCount;

    candidateCount = 0;
    direction = work->direction;
    for (offsetIndex = 0; offsetIndex < 4; offsetIndex++) {
        s32 column;
        s32 row;

        column = work->column + xOffsets[direction];
        row = work->row + yOffsets[direction];
        if (boardData->cells[column][row].state == 0) {
            candidateDirections[candidateCount] = direction;
            candidateCount++;
        }
        direction++;
        direction %= 4;
    }
    if (candidateCount == 0) {
        return 0;
    }
    if (candidateCount == 1) {
        work->path[0].column = work->column;
        work->path[0].row = work->row;
        work->path[0].direction = candidateDirections[0];
        work->path[1].column = work->column + xOffsets[candidateDirections[0]];
        work->path[1].row = work->row + yOffsets[candidateDirections[0]];
        return 2;
    } else {
        offsetIndex = frandmod(candidateCount);
        work->path[0].column = work->column;
        work->path[0].row = work->row;
        work->path[0].direction = candidateDirections[offsetIndex];
        work->path[1].column = work->column + xOffsets[candidateDirections[offsetIndex]];
        work->path[1].row = work->row + yOffsets[candidateDirections[offsetIndex]];
        return 2;
    }
}

/* Restores the occupied/free cell states after a path search. */
void fn_1_7340(M680State *board)
{
    s32 row;
    s32 column;

    for (row = 1; row < 13; row++) {
        for (column = 1; column < 14; column++) {
            if (board->cells[row][column].state == 2) {
                board->cells[row][column].state = 0;
            }
            if (board->cells[row][column].state == 3) {
                board->cells[row][column].state = 1;
            }
        }
    }
}

/* Assigns the ten pieces to their two five-piece groups. */
void fn_1_73EC(M680State *board)
{
    int index = 0;

    while (index < 10) {
        board->grid[index].result = index >= 5;
        index++;
    }
}

/* Shuffles the pieces and links them into the two-row selection grid. */
void fn_1_7434(M680State *work)
{
    M680GridNode *node;
    s32 index;
    s32 column;
    s32 randomStart;
    s32 row;
    s32 candidateIndex;
    s32 temporary;

    for (index = 0; index < 10; index++) {
        work->nodeOrder[index] = index;
    }
    for (index = 0; index < 20; index++) {
        randomStart = frandmod(10);
        candidateIndex = (randomStart + frandmod(9)) % 10;
        temporary = work->nodeOrder[randomStart];
        work->nodeOrder[randomStart] = work->nodeOrder[candidateIndex];
        work->nodeOrder[candidateIndex] = temporary;
    }

    work->first = &work->grid[work->nodeOrder[0]];
    for (index = 0; index < 10; index++) {
        node = &work->grid[work->nodeOrder[index]];
        column = index % 5;
        row = index / 5;
        node->targetColumn = column * 2 + 2;
        node->targetRow = row * 2 + 9;
        if (column > 0) {
            node->left = &work->grid[work->nodeOrder[index - 1]];
        } else {
            node->left = 0;
        }
        if (column < 4) {
            node->right = &work->grid[work->nodeOrder[index + 1]];
        } else {
            node->right = 0;
        }
        if (row == 0) {
            node->up = 0;
            node->down = &work->grid[work->nodeOrder[index + 5]];
        } else {
            node->up = &work->grid[work->nodeOrder[index - 5]];
            node->down = 0;
        }
        node->ready = 0;
    }
}

/* Links the four player records into the order used for successive turns. */
void fn_1_765C(M680State *state) {
    int i;
    for (i = 0; i < 3; i++) {
        state->playerNodes[i].player = state->actors[i];
        state->playerNodes[i].next = &state->playerNodes[i + 1];
    }
    state->playerNodes[i].player = state->actors[i];
    state->playerNodes[i].next = NULL;
    state->players = &state->playerNodes[0];
    state->tail = &state->playerNodes[3];
}

/* Removes the first player from the turn list, optionally moving it to the end. */
void fn_1_76E8(M680State *list, s32 mode) {
    M680PlayerNode *node;

    node = list->players;
    list->players = node->next;
    if (mode == 0) {
        node->next = 0;
        list->tail->next = node;
        list->tail = node;
    }
}

/* Marks a piece ready for the round sequence. */
void fn_1_7724(M680GridNode *work) {
    work->ready = 1;
}

/* Finds the first unready piece in the linked selection grid. */
void *fn_1_7730(M680State *board)
{
    M680GridNode *node;

    node = board->first;
    while (node) {
        if (node->ready == 0) {
            return node;
        }
        if ((node->down)->ready == 0) {
            return node->down;
        }
        node = node->right;
    }
    return node;
}

/* Creates the background, attached models and fixed stage effects during initialization. */
void fn_1_7788(M680State *work)
{
    s16 model;
    s16 motion;

    {
    M680_LOAD_MODEL(53);
    M680_LOAD_MOTION(8);
    work->attachedMotion = motion;
    work->attachedModel = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 608.0f, 320.0f, 207.0f);
    Hu3DModelRotSet(model, 0.0f, 90.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    }

    {
    M680_LOAD_MODEL(40);
    Hu3DModelHookSet(work->attachedModel, (void *)"itemhook_L", model);
    }

    {
    M680_LOAD_MODEL(54);
    M680_LOAD_MOTION(11);
    work->effectHookMotion = motion;
    work->effectHookModel = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 628.0f, 194.0f, 560.0f);
    Hu3DModelRotSet(model, 0.0f, 270.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    }

    {
    M680_LOAD_MODEL(54);
    M680_LOAD_MOTION(11);
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 273.0f, -615.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    }

    {
    M680_LOAD_MODEL(55);
    M680_LOAD_MOTION(12);
    work->motionIfNonzero = motion;
    work->currentModel = model;
    M680_LOAD_MOTION_ONLY(14);
    work->motionIfZero = motion;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, -213.0f, 231.0f, 291.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    }

    {
    M680_LOAD_MODEL(56);
    M680_LOAD_MOTION(10);
    work->effect.model_b = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 270.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(model);
    }

    {
    M680_LOAD_MODEL(57);
    M680_LOAD_MOTION(10);
    work->effect.model_c = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 270.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(model);
    }

    {
    M680_LOAD_MODEL(35);
    work->effect.model_a = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 270.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_SHAPE_LOOP);
    Hu3DModelAttrReset(model, HU3D_MOTATTR_SHAPE_PAUSE);
    }

    {
    M680_LOAD_MODEL(58);
    M680_LOAD_MOTION(9);
    work->stage.endMotion = motion;
    work->stage.model = model;
    M680_LOAD_MOTION_ONLY(13);
    work->stage.motion = motion;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    work->stage.position.x = -825.0f;
    work->stage.position.y = 0.0f;
    work->stage.position.z = 1191.225f;
    work->stage.rotation.x = 0.0f;
    work->stage.rotation.y = 0.0f;
    work->stage.rotation.z = 0.0f;
    Hu3DModelPosSetV(model, &work->stage.position);
    Hu3DModelRotSetV(model, &work->stage.rotation);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(model);
    }

    {
    M680_LOAD_MODEL(39);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelHookSet(work->stage.model, (void *)"itemhook_R", model);
    }
}

/* Moves the staged attachment through its fixed path and updates its motion and heading. */
s32 fn_1_8288(void *argument)
{
    M680MotionWork *work = argument;
    s32 done = 0;
    s32 pathX[8] = {0, 3, 3, 5, 5, 7, 7, 12};
    s32 pathZ[8] = {13, 13, 10, 10, 12, 12, 13, 13};
    s32 pathRate[8] = {1, 2, 1, 0, 1, 0, 1, 1};
    s32 pathFlags[8] = {0, 0, 0, 1, 0, 0, 1, 0};
    s32 pathDuration[8] = {3, 3, 2, 2, 2, 1, 5, 0};
    f32 progress;

    switch (work->state) {
    case 0:
        work->step = 0;
        work->position.y = 0.0f;
        work->rotation.y = 90.0f;
        Hu3DModelAttrReset(work->model, HU3D_ATTR_DISPOFF);
        Hu3DMotionSet(work->model, work->motion);
        Hu3DMotionSpeedSet(work->model, 1.3f);
        work->state++;
        /* Fall through to initialize the first path segment. */
    case 1:
        work->position.x = 137.5f * (f32)(pathX[work->step] - 1) - 687.5f;
        work->position.z = 137.5f * (f32)(pathZ[work->step] - 1) - 458.775f;
        work->origin = work->position;
        work->target.x = 137.5f * (f32)(pathX[work->step + 1] - 1) - 687.5f;
        work->target.z = 137.5f * (f32)(pathZ[work->step + 1] - 1) - 458.775f;
        work->angle = 90.0f * (f32)pathRate[work->step];
        work->frame = 0;
        work->state++;
        /* Fall through to update the active path segment. */
    case 2:
        work->frame++;
        progress = (f32)work->frame /
                   (35.0f * (f32)pathDuration[work->step]);
        work->position.x = work->target.x * progress + work->origin.x * (1.0f - progress);
        work->position.z = work->target.z * progress + work->origin.z * (1.0f - progress);
        if (work->frame < pathDuration[work->step] * M680_PATH_SEGMENT_FRAMES) {
            break;
        }
        work->step++;
        if (work->step == 7) {
            work->state = 5;
            break;
        }
        if (pathFlags[work->step] != 0) {
            work->state++;
        } else {
            work->state = 1;
        }
        break;
    case 3:
            HuAudFXPlayPan(MSM_SE_M680_STAGE_ATTACHMENT, fn_1_AABC(work->position.x));
        Hu3DMotionShiftSet(work->model, work->endMotion, 0.0f,
                           5.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSpeedSet(work->model, 1.0f);
        work->frame = 0;
        work->state++;
        /* Fall through to wait for the motion. */
    case 4:
        work->frame++;
        if (work->frame < 60) {
            break;
        }
        Hu3DMotionShiftSet(work->model, work->motion, 0.0f,
                           5.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSpeedSet(work->model, 1.3f);
        work->state = 1;
        break;
    case 5:
        Hu3DModelAttrSet(work->model, HU3D_ATTR_DISPOFF);
        done = 1;
        break;
    default:
        break;
    }
    m680UpdateAngle(work);
    Hu3DModelPosSetV(work->model, (Vec *)work);
    Hu3DModelRotSetV(work->model, (Vec *)&work->rotation.x);
    return done;
}

/* Chooses another background motion whenever the current one finishes. */
void fn_1_8AAC(M680State *sequence) {
    if (Hu3DMotionEndCheck(sequence->currentModel) != 0) {
        if (frandmod(5) == 0) {
            Hu3DMotionSet(sequence->currentModel, sequence->motionIfZero);
        } else {
            Hu3DMotionSet(sequence->currentModel, sequence->motionIfNonzero);
        }
        Hu3DMotionTimeSet(sequence->currentModel, 0.0f);
    }
}

/* Updates directional and confirm-button edges from a player or the computer controller. */
void fn_1_8B24(M680State *work, M680CharacterState *controllerWork)
{
    M680PadButtons *buttons;
    s32 controller;

    if (controllerWork->playerType != 0) {
        buttons = &work->buttons;
        buttons->previous = buttons->current;
        buttons->current = 0;
        fn_1_9448(work);
        buttons->pressed = buttons->current & (buttons->previous ^ buttons->current);
        return;
    }

    controller = controllerWork->padNo;
    buttons = &work->buttons;
    buttons->previous = buttons->current;
    buttons->current = 0;
    if (HuPadStkDirGet((s16)controller) & 1) {
        buttons->current = 1;
    }
    if (HuPadStkDirGet((s16)controller) & 2) {
        buttons->current |= 2;
    }
    if (HuPadStkDirGet((s16)controller) & 8) {
        buttons->current |= 4;
    }
    if (HuPadStkDirGet((s16)controller) & 4) {
        buttons->current |= 8;
    }
    if (HuPadBtn[controller] & 0x100) {
        buttons->current |= 0x10;
    }
    buttons->pressed = buttons->current & (buttons->previous ^ buttons->current);
}

/* Moves the selection among available pieces and reports confirmation. */
s32 fn_1_8CAC(M680State *argument)
{
    M680CharacterState *controller;
    M680PadButtons *buttons;
    M680PadButtons *input;
    M680GridNode *candidate;
    M680GridNode *fallback;
    s16 pad;
    s32 selectedCount;
    s32 candidateCount;

    controller = argument->players->player;
    fn_1_8B24(argument, controller);
    input = &argument->buttons;
    if (input->pressed & 0x10) {
        HuAudFXPlay(MSM_SE_M680_SELECTION_CONFIRM);
        return 1;
    }

    if (input->pressed & 1) {
        if ((input->pressed & 8) && argument->selected->left &&
            argument->selected->left->down && argument->selected->left->down->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->left->down;
            fn_1_A7E8(argument);
            return 0;
        }
        if ((input->pressed & 4) && argument->selected->left &&
            argument->selected->left->up && argument->selected->left->up->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->left->up;
            fn_1_A7E8(argument);
            return 0;
        }
        candidate = argument->selected->left;
        while (candidate) {
            if (candidate->ready == 0) {
                HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                argument->selected = candidate;
                fn_1_A7E8(argument);
                return 0;
            }
            candidate = candidate->left;
        }
        if (candidate == NULL) {
            if (argument->selected->up) {
                candidate = argument->selected->up->left;
            } else {
                candidate = argument->selected->down->left;
            }
            while (candidate) {
                if (candidate->ready == 0) {
                    HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                    argument->selected = candidate;
                    fn_1_A7E8(argument);
                    return 0;
                }
                candidate = candidate->left;
            }
        }
    }

check_down:
    if (input->pressed & 2) {
        if ((input->pressed & 8) && argument->selected->right &&
            argument->selected->right->down && argument->selected->right->down->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->right->down;
            fn_1_A7E8(argument);
            return 0;
        }
        if ((input->pressed & 4) && argument->selected->right &&
            argument->selected->right->up && argument->selected->right->up->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->right->up;
            fn_1_A7E8(argument);
            return 0;
        }
        candidate = argument->selected->right;
        while (candidate) {
            if (candidate->ready == 0) {
                HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                argument->selected = candidate;
                fn_1_A7E8(argument);
                return 0;
            }
            candidate = candidate->right;
        }
        if (candidate == NULL) {
            if (argument->selected->up) {
                candidate = argument->selected->up->right;
            } else {
                candidate = argument->selected->down->right;
            }
            while (candidate) {
                if (candidate->ready == 0) {
                    HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                    argument->selected = candidate;
                    fn_1_A7E8(argument);
                    return 0;
                }
                candidate = candidate->right;
            }
        }
    }

check_side:
    if ((input->pressed & 8) && argument->selected->down) {
        if (argument->selected->down->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->down;
            fn_1_A7E8(argument);
            return 0;
        }
        selectedCount = 0;
        candidateCount = 0;
        fallback = NULL;
        candidate = argument->selected->down->left;
        while (candidate) {
            selectedCount++;
            if (candidate->ready == 0) {
                fallback = candidate;
            } else {
                candidate = candidate->left;
                continue;
            }
            break;
        }
        candidate = argument->selected->down->right;
        while (candidate) {
            candidateCount++;
            if (candidate->ready == 0) {
                if (fallback && selectedCount <= candidateCount) {
                    HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                    argument->selected = fallback;
                    fn_1_A7E8(argument);
                    return 0;
                }
                HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                argument->selected = candidate;
                fn_1_A7E8(argument);
                return 0;
            }
            candidate = candidate->right;
        }
        if (fallback) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = fallback;
            fn_1_A7E8(argument);
            return 0;
        }
    }
    if ((input->pressed & 4) && argument->selected->up) {
        if (argument->selected->up->ready == 0) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = argument->selected->up;
            fn_1_A7E8(argument);
            return 0;
        }
        selectedCount = 0;
        candidateCount = 0;
        fallback = NULL;
        candidate = argument->selected->up->left;
        while (candidate) {
            selectedCount++;
            if (candidate->ready == 0) {
                fallback = candidate;
            } else {
                candidate = candidate->left;
                continue;
            }
            break;
        }
        candidate = argument->selected->up->right;
        while (candidate) {
            candidateCount++;
            if (candidate->ready == 0) {
                if (fallback && selectedCount <= candidateCount) {
                    HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                    argument->selected = fallback;
                    fn_1_A7E8(argument);
                    return 0;
                }
                HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
                argument->selected = candidate;
                fn_1_A7E8(argument);
                return 0;
            }
            candidate = candidate->right;
        }
        if (fallback) {
            HuAudFXPlay(MSM_SE_M680_CURSOR_MOVE);
            argument->selected = fallback;
            fn_1_A7E8(argument);
            return 0;
        }
    }
    return 0;
}

/* Resets the computer selection delay and chooses how many actions it will consider. */
void fn_1_9400(M680State *sequence) {
    sequence->choiceFrames = 0;
    sequence->choiceCount = 0;
    sequence->choiceLimit = frandmod(10) + 5;
}

/* Produces the computer player's next selection action after its delay. */
void fn_1_9448(M680State *state)
{
    u16 *action;
    s32 randomValue;

    action = &state->buttons.current;
    state->choiceFrames++;
    if (state->choiceFrames >= 30) {
        state->choiceCount++;
        if (state->choiceCount == state->choiceLimit) {
            *action = 16;
            return;
        }
        state->choiceFrames = 0;
        randomValue = frandmod(100);
        if (randomValue < 10) {
            *action = 0;
            return;
        }
        if (randomValue < 35) {
            if (state->selected->down != 0) {
                *action = 8;
                return;
            }
            *action = 4;
            return;
        }
        if (state->selected->left == NULL) {
            *action = 2;
            return;
        }
        if (state->selected->right == NULL) {
            *action = 1;
            return;
        }
        if (frandmod(100) < 35) {
            *action = 1;
            return;
        }
        *action = 2;
    }
}

/* Clears the cells along the gate's board route. */
void fn_1_9580(M680State *sequence) {
    s32 index;

    for (index = 7; index < 12; index++) {
        sequence->cells[index][13].state = 0;
    }
    sequence->cells[12][12].state = 0;
}

/* Marks the cells along the gate's board route occupied. */
void fn_1_95C0(M680State *sequence) {
    s32 index;

    for (index = 7; index < 12; index++) {
        sequence->cells[index][13].state = 1;
    }
    sequence->cells[12][12].state = 1;
}

/* Counts a completed player result animation for the sequence. */
void fn_1_9600(void) {
    M680State *sequence;

    sequence = lbl_1_bss_0->data;
    sequence->modeTimer += 1;
}

/* Lowers one group of item models into view during the board transition. */
void fn_1_9630(M680State *state, s32 firstIndex)
{
    f32 progress;
    f32 height;
    s32 index;

    progress = (f32)state->frame / 30.0f;
    height = 800.0f * (1.0f - progress * progress);
    for (index = firstIndex; index < firstIndex + 5; index++) {
        Hu3DModelPosSet(state->grid[index].itemModel, 0.0f, height, 0.0f);
        if (state->frame >= 30) {
            Hu3DMotionTimeSet(state->grid[index].hookModel, 5.0f);
        }
    }
}

/* Raises and shrinks the item models selected for the winner transition. */
void fn_1_9754(M680State *state)
{
    f32 progress;
    f32 height;
    s32 index;

    progress = (f32)state->frame / 30.0f;
    height = (f32)(150.0 * sin(1.5707963267948966 * (f64)progress));
    for (index = 0; index < 10; index++) {
        if (state->grid[index].ready == 0) {
            Hu3DModelPosSet(state->grid[index].itemModel, 0.0f, height, 0.0f);
            Hu3DModelScaleSet(state->grid[index].itemModel, 1.0f - progress, 1.0f - progress,
                              1.0f - progress);
        }
    }
}

/* Starts the item motion and completion callback for one five-piece group. */
void fn_1_9898(M680State *work, s32 firstIndex)
{
    s32 index;

    for (index = firstIndex; index < firstIndex + 5; index++) {
        HuAudFXPlay(MSM_SE_M680_PIECE_MOTION);
        Hu3DMotionShiftSet(
            work->grid[index].model,
            work->grid[index].motions[3], 0.0f,
            5.0f, 0);
        fn_1_BE08((M680Callback)fn_1_9944, &work->grid[index]);
    }
}

/* Restores a piece's idle motion when its current motion finishes. */
s32 fn_1_9944(M680GridNode *work) {
    if (Hu3DMotionEndCheck(work->model) != 0) {
        Hu3DMotionShiftSet(work->model, work->motions[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        return 1;
    }
    return 0;
}

/* Starts a fifteen-frame forward or reverse motion-time transition. */
void fn_1_99B4(M680MotionDurationState *durationWork, s32 duration, s32 reverse)
{
    durationWork->elapsedFrames = 0;
    durationWork->duration = duration;
    if (reverse != 0) {
        HuAudFXPlay(MSM_SE_M680_TRANSITION_REVERSE);
        fn_1_BE08(fn_1_9B00, durationWork);
    } else {
        HuAudFXPlay(MSM_SE_M680_TRANSITION_FORWARD);
        fn_1_BE08(fn_1_9A34, durationWork);
    }
}

/* Advances a model's motion time toward its requested endpoint over fifteen frames. */
s32 fn_1_9A34(void *durationData)
{
    M680MotionDurationState *durationWork;
    f32 progress;

    durationWork = durationData;
    durationWork->elapsedFrames++;
    progress = (f32)durationWork->elapsedFrames / 15.0f;
    Hu3DMotionTimeSet(durationWork->motionId, progress * (f32)durationWork->duration);
    if (durationWork->elapsedFrames >= 15) {
        return 1;
    }
    return 0;
}

/* Runs a model's motion time backward from its requested endpoint over fifteen frames. */
s32 fn_1_9B00(void *durationData)
{
    M680MotionDurationState *durationWork;
    f32 progress;

    durationWork = durationData;
    durationWork->elapsedFrames++;
    progress = (f32)durationWork->elapsedFrames / 15.0f;
    Hu3DMotionTimeSet(durationWork->motionId,
                      (1.0f - progress) * (f32)durationWork->duration);
    if (durationWork->elapsedFrames >= 15) {
        return 1;
    }
    return 0;
}

/* Advances the attached model's motion from frame five to fifteen over ten frames. */
s32 fn_1_9BDC(void *pieceData)
{
    M680GridNode *piece;
    f32 progress;

    piece = pieceData;
    piece->handTimer++;
    progress = (f32)piece->handTimer / 10.0f;
    Hu3DMotionTimeSet(piece->hookModel, (f32)(5.0 + (f64)(10.0f * progress)));
    if (piece->handTimer >= 10) {
        return 1;
    }
    return 0;
}

/* Shrinks an item model to zero over ten frames. */
s32 fn_1_9CA0(void *pieceData)
{
    M680GridNode *piece;
    f32 scale;

    piece = pieceData;
    piece->winTimer++;
    scale = 1.0f - ((f32)piece->winTimer / 10.0f);
    Hu3DModelScaleSet(piece->itemModel, scale, scale, scale);
    if (piece->winTimer >= 10) {
        return 1;
    }
    return 0;
}

/* Starts the callback that reveals the three effect models in successive phases. */
void fn_1_9D5C(M680PhaseState *phase_state) {
    phase_state->phase = 0;
    fn_1_BE08(fn_1_9D9C, phase_state);
}

/* Reveals and positions the three effect models on successive callback phases. */
s32 fn_1_9D9C(void *state) {
    Point3d position;
    M680PhaseState *effect;

    effect = state;
    effect->phase += 1;
    if (effect->phase == 1) {
        position.x = 30.0f;
        position.y = 150.0f;
        position.z = 1300.0f;
        Hu3DModelAttrReset(effect->models[0], HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(effect->models[0], 0.0f);
        Hu3DModelPosSetV(effect->models[0], &position);
    }
    if (effect->phase == 6) {
        position.x = -70.0f;
        position.y = 100.0f;
        position.z = 1300.0f;
        Hu3DModelAttrReset(effect->models[1], HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(effect->models[1], 0.0f);
        Hu3DModelPosSetV(effect->models[1], &position);
    }
    if (effect->phase == M680_EFFECT_PHASE_THIRD) {
        position.x = 105.0f;
        position.y = 180.0f;
        position.z = 1350.0f;
        Hu3DModelAttrReset(effect->models[2], HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(effect->models[2], 0.0f);
        Hu3DModelPosSetV(effect->models[2], &position);
        return 1;
    }
    return 0;
}

/* Pulses the attached model's material color for sixty frames. */
s32 fn_1_9F18(void *pieceData)
{
    M680GridNode *piece;
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    f32 phase;
    f32 progress;

    piece = pieceData;
    piece->loseTimer++;
    phase = (f32)piece->loseTimer / 60.0f;
    progress = (f32)(0.5 * (1.0 + sin(25.132741228718345 * (f64)phase)));
    hsf = Hu3DData[piece->hookModel].hsf;
    material = hsf->material;
    material += 2;
    material->color[0] = 27.0f * (1.0f - progress) + 255.0f * progress;
    material->color[1] = 149.0f * (1.0f - progress);
    material->color[2] = 206.0f * (1.0f - progress);
    if (piece->loseTimer >= 60) {
        Hu3DMotionTimeSet(piece->hookModel, 5.0f);
        return 1;
    }
    return 0;
}

/* Turns the stage model through one sector and reveals the completed marker. */
s32 fn_1_A0E4(void *boardData)
{
    M680State *board;
    f32 progress;
    f32 rotation;
    f32 phaseAngle;

    board = boardData;
    board->transitionTimer++;
    progress = (f32)board->transitionTimer / 90.0f;
    phaseAngle = 30.0f * (f32)board->roundWins;
    rotation = phaseAngle + 30.0f * progress;
    Hu3DModelRotSet(board->models[37], 0.0f, -rotation, 0.0f);
    if (board->transitionTimer >= 90) {
        HuAudFXPlay(MSM_SE_M680_MARKER_COMPLETE);
        Hu3DModelAttrReset(board->markers[board->roundWins - 1], HU3D_MOTATTR_PAUSE);
        return 1;
    }
    return 0;
}

/* Turns the stage model through the end transition, then pauses and resets all five marker
 * motions. */
s32 fn_1_A234(void *boardData)
{
    M680State *board;
    f32 progress;
    f32 rotation;
    s32 index;

    board = boardData;
    board->transitionTimer++;
    progress = (f32)board->transitionTimer / 240.0f;
    rotation = 180.0f + 210.0f * progress;
    Hu3DModelRotSet(board->models[37], 0.0f, -rotation, 0.0f);
    if (board->transitionTimer >= 240) {
        for (index = 0; index < 5; index++) {
            Hu3DModelAttrSet(board->markers[index], HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrReset(board->markers[index], HU3D_MOTATTR_LOOP);
            Hu3DMotionTimeSet(board->markers[index], 0.0f);
        }
        return 1;
    }
    return 0;
}

/* Shows the three-model moving effect and starts its positional sound and callback. */
void fn_1_A388(M680EffectState *effect) {
    Hu3DModelAttrReset(effect->model_a, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(effect->model_b, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(effect->model_c, HU3D_ATTR_DISPOFF);
    effect->x = 1000.0f;
    effect->y = 0.0f;
    effect->z = 1200.0f;
    effect->progress = 0;
    Hu3DModelPosSet(effect->model_a, effect->x, effect->y, effect->z);
    Hu3DModelPosSet(effect->model_b, effect->x, effect->y, effect->z - 43.0f);
    Hu3DModelPosSet(effect->model_c, effect->x, effect->y, 43.0f + effect->z);
    effect->audio_handle = HuAudFXPlayPan(MSM_SE_M680_STAGE_EFFECT, fn_1_AABC(effect->x));
    fn_1_BE08(fn_1_A490, effect);
}

/* Moves the effect across the stage, updating its sound pan each frame. */
s32 fn_1_A490(void *effectData)
{
    M680EffectState *effect;
    f32 progress;

    effect = effectData;
    effect->progress++;
    progress = (f32)effect->progress / 240.0f;
    effect->x = (-800.0f * progress) + (1000.0f * (1.0f - progress));
    Hu3DModelPosSet(effect->model_a, effect->x, effect->y, effect->z);
    Hu3DModelPosSet(effect->model_b, effect->x, effect->y, effect->z - 43.0f);
    Hu3DModelPosSet(effect->model_c, effect->x, effect->y, 43.0f + effect->z);
    HuAudFXPanning(effect->audio_handle, fn_1_AABC(effect->x));
    if (effect->progress >= 240) {
        Hu3DModelAttrSet(effect->model_a, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(effect->model_b, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(effect->model_c, HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Starts the ten-frame gate motion in the requested direction. */
void fn_1_A5F4(M680MotionTimerState *gateTimer, s32 reverse)
{
    gateTimer->elapsedFrames = 0;
    if (reverse) {
        fn_1_BE08(fn_1_A708, gateTimer);
        return;
    }
    fn_1_BE08(fn_1_A658, gateTimer);
}

/* Advances the gate motion from frame zero to fifteen over ten frames. */
s32 fn_1_A658(void *timerData)
{
    M680MotionTimerState *timer;
    f32 progress;

    timer = timerData;
    timer->elapsedFrames = timer->elapsedFrames + 1;
    progress = (f32)timer->elapsedFrames / 10.0f;
    Hu3DMotionTimeSet(timer->motionId, 15.0f * progress);
    return timer->elapsedFrames >= 10;
}

/* Runs the gate motion backward for ten frames, then frees its board cell. */
s32 fn_1_A708(void *timerData)
{
    M680MotionTimerState *timer;
    M680State *owner;
    f32 progress;

    timer = timerData;
    owner = lbl_1_bss_0->data;
    timer->elapsedFrames++;
    progress = (f32)timer->elapsedFrames / 10.0f;
    Hu3DMotionTimeSet(timer->motionId, 15.0f * (1.0f - progress));
    if (timer->elapsedFrames >= 10) {
        owner->cells[10][7].state = 0;
        return 1;
    }
    return 0;
}

/* Positions and turns the selection marker beside the selected piece. */
void fn_1_A7E8(M680State *board)
{
    Vec offsets[10] = {
        {-40.0f, 230.0f, 100.0f}, {-40.0f, 230.0f, 100.0f},
        {-58.0f, 230.0f, 100.0f}, {-58.0f, 230.0f, 100.0f},
        {-76.0f, 230.0f, 100.0f}, {-76.0f, 230.0f, 100.0f},
        {-94.0f, 230.0f, 100.0f}, {-94.0f, 230.0f, 100.0f},
        {-112.0f, 230.0f, 100.0f}, {-112.0f, 230.0f, 100.0f}
    };
    s32 direction;

    if (board->selected->position.x < -300.0) {
        direction = 0;
        Hu3DModelRotSet(board->cursorModel, 0.0f, 15.0f, 0.0f);
        if (board->selected->position.z > 900.0f) {
            direction++;
        }
    } else if (board->selected->position.x < -100.0) {
        Hu3DModelRotSet(board->cursorModel, 0.0f, 7.5f, 0.0f);
        direction = 2;
        if (board->selected->position.z > 900.0f) {
            direction++;
        }
    } else if (board->selected->position.x > 300.0) {
        direction = 8;
        Hu3DModelRotSet(board->cursorModel, 0.0f, -15.0f, 0.0f);
        if (board->selected->position.z > 900.0f) {
            direction++;
        }
    } else if (board->selected->position.x > 100.0) {
        direction = 6;
        Hu3DModelRotSet(board->cursorModel, 0.0f, -7.5f, 0.0f);
        if (board->selected->position.z > 900.0f) {
            direction++;
        }
    } else {
        direction = 4;
        Hu3DModelRotSet(board->cursorModel, 0.0f, 0.0f, 0.0f);
        if (board->selected->position.z > 900.0f) {
            direction++;
        }
    }

    board->cursorPosition.x = board->selected->position.x + offsets[direction].x;
    board->cursorPosition.y = board->selected->position.y + offsets[direction].y;
    board->cursorPosition.z = board->selected->position.z + offsets[direction].z;
    Hu3DModelPosSetV(board->cursorModel, (Point3d *)&board->cursorPosition);
}

/* Maps horizontal stage position to a sound pan value from 32 to 96. */
s16 fn_1_AABC(f32 stagePosition)
{
    f32 angle = (687.5f + stagePosition) / 1375.0f;
    s16 result;

    if (angle > 1.0f) {
        angle = 1.0f;
    }
    if (angle < 0.0f) {
        angle = 0.0f;
    }
    result = (s16)(32.0f + 64.0f * angle);
    return result;
}

M680ObjectManagerState lbl_1_bss_8;
s32 lbl_1_bss_4;
OMOBJ *lbl_1_bss_0;
