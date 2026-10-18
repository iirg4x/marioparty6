/* Microphone mode setup, player selection and scene transitions. */
#include "math.h"
#include "REL/mdmicdll/object_state.h"
#include "REL/mdmicdll/menu_state.h"
#include "REL/mdmicdll/data_state.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "REL/mdmicdll/vector_state.h"
#include "game/charman.h"
#include "game/gamemes.h"
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
#include "game/process.h"
#include "game/armem.h"
#include "game/flag.h"
#include "game/saveload.h"
#include "REL/mdmicdll/player_state.h"
#include "dolphin/types.h"
#include "game/window.h"
#include "REL/mdmicdll/camera_state.h"
#include "REL/mdmicdll/audio_state.h"
#include "REL/mdmicdll/sprite_state.h"
#include "REL/mdmicdll/model_state.h"
#include "REL/mdmicdll/motion_state.h"
#include "dolphin.h"
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "humath.h"
#include "string.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/board/main.h"
#include "game/printfunc.h"
#include "REL/mdmicdll/rodata_state.h"
#include "messdir_enum.h"
#include "msm_stream.h"
#include "msm_se.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include "game/animdata.h"
#include "game/mgdata.h"
#include "dolphin/mic.h"
#include "msm.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/board/masu.h"
#include "game/board/player.h"

/* Audio cues used while choosing players and changing setup screens. */
int HuAudFXPlayPan(int seId, s16 pan);
void HuAudFXStop(int seNo);
s32 CharFXPlay(s16 charNo, s16 seId);

#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)

#define MDMIC_BANK_ACTIVITY_THREE_FLAG 52
#define MDMIC_LOAD_PROCESS_PRIORITY 256U
#define MDMIC_LOAD_PROCESS_STACK_SIZE 16384U
#define MDMIC_MAIN_PROCESS_PRIORITY 12288U
#define MDMIC_MAIN_PROCESS_STACK_SIZE 12288U
#define MDMIC_OBJECT_CAPACITY 27
#define MDMIC_MANAGER_PRIORITY 8192
#define MDMIC_OBJECT_PRIORITY 4096
#define MDMIC_RECORD_OBJECT_PRIORITY 8192
#define MDMIC_CAMERA_OBJECT_PRIORITY 32730
#define MDMIC_RECORD_MAX_DISPLAY_FRAMES 35999
#define MDMIC_ENTRY_MUSIC_FADE_SPEED 3000
#define MDMIC_PLAYER_COUNT_MODEL_BASE 17
#define MDMIC_PLAYER_COUNT_POSITION_BASE 33
#define MDMIC_PLAYER_INACTIVE_IMAGE 18
#define MDMIC_PLAYER_ACTIVE_IMAGE 19
#define MDMIC_MINIGAME_POSITION_BASE 67
#define MDMIC_MINIGAME_FOCUS_POSITION_BASE 72

/* Each mode has a separate sequence of setup steps in the menu process. */
#define MDMIC_MODE_STEP_STRIDE 100
#define MDMIC_STEP_QUIZ_PLAYER_COUNT 700
#define MDMIC_STEP_QUIZ_CHARACTERS 701
#define MDMIC_STEP_QUIZ_CONFIRM 703
#define MDMIC_STEP_ACTIVITY_CHARACTERS 800
#define MDMIC_STEP_ACTIVITY_CHOICE 801
#define MDMIC_STEP_ACTIVITY_CONFIRM 802
#define MDMIC_STEP_MINIGAME_PLAYER_COUNT 900
#define MDMIC_STEP_MINIGAME_CHARACTERS 901
#define MDMIC_STEP_MINIGAME_CHOICE 902
#define MDMIC_STEP_MINIGAME_ORDER 903
#define MDMIC_STEP_MINIGAME_CONFIRM 904

typedef struct { s8 values[55][2]; } MDRESULT_BYTE_TABLE_110;

typedef struct MDMIC_AUDIO_TABLE {
    s32 value[16];
} MDMIC_AUDIO_TABLE;

typedef struct MDMicdllMotionDirectories {
    s32 value[3];
} MDMicdllMotionDirectories;

typedef struct {
    s16 rows[12][3];
} MDMinNeighborTable;

typedef void (*VoidFunc)(void);

typedef struct MdMicMessageTable_s {
    s32 message[5];
} MDMIC_MESSAGE_TABLE;

typedef void (*MDMinidllVoidFunc)(void);

typedef struct MDMICDLL_MESSAGE_TABLE {
    u32 message[5];
} MDMICDLL_MESSAGE_TABLE;

enum {
    MDSEL_DLL_RETURN_REPORT_SIZE = 60,
    MDSEL_ITEM_HOOK_NAME_SIZE = 19,
    MDSEL_OBJECT_PRIORITY = 4096,
    MDSEL_OBJECT_CAPACITY = 16,
    MDSEL_LARGE_OBJECT_CAPACITY = 64,
    MDSEL_OBJECT_MANAGER_PRIORITY = 8192,
    MDSEL_MAIN_PROCESS_PRIORITY = 12288,
    MDSEL_MAIN_PROCESS_STACK_SIZE = 12288,
    MDSEL_WINDOW_WIDTH = 544,
    MDSEL_WINDOW_HEADER_HEIGHT = 42,
    MDSEL_WINDOW_MENU_HEIGHT = 68,
};

enum {
    MDRESULT_GROUP_CURSOR_X = 329,
    MDRESULT_GROUP_CURSOR_Y = 330,
    MDRESULT_GROUP_GRAPH_INDEX = 331,
    MDRESULT_GROUP_VIEW_MODE = 332,
    MDRESULT_GROUP_TABLE_COUNT = 333,
};

enum {
    MDRESULT_OBJECT_MANAGER_PRIORITY = 8192,
    MDRESULT_OBJECT_PRIORITY = 4096,
    MDRESULT_MAIN_PROCESS_PRIORITY = 12288,
    MDRESULT_MAIN_PROCESS_STACK_SIZE = 12288,
    MDRESULT_CHARACTER_MOTION_COUNT = 80,
    MDRESULT_MODEL_ARRAY_COUNT = 16,
    MDRESULT_MOTION_ARRAY_COUNT = 16,
    MDRESULT_LARGE_MODEL_ARRAY_COUNT = 32,
    MDRESULT_WIN_MOTION_OFFSET = 32,
    MDRESULT_OTHER_MOTION_OFFSET = 36,
};

enum {
    MDRESULT_MESSAGE_GUIDE_START = MESSNUM(MESS_SYS_GUIDE, 0),
    MDRESULT_MESSAGE_GUIDE_RETURN = MESSNUM(MESS_SYS_GUIDE, 8),
    MDRESULT_MESSAGE_PARTY_GRAPH_STAR = MESSNUM(MESS_PARTY_RESULTS, 52),
    MDRESULT_MESSAGE_PARTY_GRAPH_COIN = MESSNUM(MESS_PARTY_RESULTS, 53),
    MDRESULT_MESSAGE_TEAM_GRAPH_STAR = MESSNUM(MESS_PARTY_RESULTS, 64),
    MDRESULT_MESSAGE_TEAM_GRAPH_COIN = MESSNUM(MESS_PARTY_RESULTS, 65),
};

enum {
    MDRESULT_GRAPH_GROUP_CAPACITY = 344,
    MDRESULT_GRAPH_MODE_GROUP_CAPACITY = 37,
    MDRESULT_TEAM_RESULT_GROUP_CAPACITY = 11,
    MDRESULT_GRAPH_GRID_SPRITE_PRIORITY = 65,
    MDRESULT_GRAPH_WIDE_SPRITE_PRIORITY = 70,
    MDRESULT_GRAPH_SELECTED_SPRITE_PRIORITY = 95,
    MDRESULT_GRAPH_MODE_SPRITE_PRIORITY = 60,
    MDRESULT_GRAPH_VALUE_SPRITE_PRIORITY = 61,
    MDRESULT_GRAPH_WIDE_SPRITE_ANIM = 54,
    MDRESULT_GRAPH_LINE_SPRITE_ANIM = 48,
    MDRESULT_GRAPH_SELECTED_SPRITE_ANIM = 28,
    MDRESULT_GRAPH_MODE_LEFT_SPRITE_ANIM = 34,
    MDRESULT_GRAPH_MODE_RIGHT_SPRITE_ANIM = 35,
    MDRESULT_GRAPH_VALUE_SPRITE_ANIM = 33,
    MDRESULT_GRAPH_GRID_END = 180,
    MDRESULT_GRAPH_LINE_START = 240,
    MDRESULT_GRAPH_WIDE_END = 240,
    MDRESULT_TEAM_GRAPH_WIDE_END = 210,
    MDRESULT_GRAPH_LINE_END = 255,
    MDRESULT_GRAPH_SELECTED_END = 259,
    MDRESULT_GRAPH_BANK_BASE = 240,
    MDRESULT_GRAPH_GRID_X_STEP = 20,
    MDRESULT_GRAPH_COLUMN_X_STEP = 76,
    MDRESULT_GRAPH_GRID_X_START = 142,
    MDRESULT_GRAPH_ROW_Y_STEP = 100,
    MDRESULT_GRAPH_GRID_Y_START = 204,
    MDRESULT_GRAPH_COLUMN_X_START = 162,
    MDRESULT_GRAPH_VALUE_X_STEP = 54,
    MDRESULT_GRAPH_VALUE_X_START = 153,
    MDRESULT_GRAPH_VALUE_Y_STEP = 50,
    MDRESULT_GRAPH_VALUE_Y_START = 131,
    MDRESULT_GRAPH_DRAW_NO = 64,
    MDRESULT_GRAPH_SCISSOR_X = 138,
    MDRESULT_GRAPH_SCISSOR_Y = 90,
    MDRESULT_GRAPH_SCISSOR_WIDTH = 425,
    MDRESULT_GRAPH_SCISSOR_HEIGHT = 300,
    MDRESULT_GRAPH_ALPHA_DIM = 32,
    MDRESULT_COLOR_MAX = 255,
};

enum {
    MDRESULT_PLAYER_WINDOW_WIDTH = 240,
    MDRESULT_PLAYER_WINDOW_HEIGHT = 42,
    MDRESULT_PARTICLE_COLOR_RED_GREEN = 136,
    MDRESULT_PARTICLE_PAIRED_ALPHA = 64,
};

extern const HuVecF lbl_1_rodata_108;
extern const HuVecF lbl_1_rodata_114;

extern const MDMinNeighborTable lbl_1_rodata_278;
extern const MDMinidllVoidFunc _ctors[];
extern const MDMinidllVoidFunc _dtors[];
extern int HuAudFXPlay(int seId);

extern const Point3d lbl_1_rodata_25C;

extern void _ClearFlag(u32 flag);
extern void CharDataClose(s16 charNo);
extern void HuAMemDump(void);
extern void omOvlReturnEx(s16 overlay, s16 event);

extern int HuAudSStreamPlay(s16 streamId);

s32 lbl_1_bss_A50[4];
HU3D_LIGHTID lbl_1_bss_A4C[2];
HUWINID lbl_1_bss_A44[4];
MDMIC_CAMERA_WORK lbl_1_bss_9F4;
MDMIC_MENU_STATE lbl_1_bss_9DC;
MDMIC_PLAYER_WORK lbl_1_bss_9A4[4];
ANIMDATA *lbl_1_bss_924[32];
HUSPR_GROUPID lbl_1_bss_900[18];
HUSPRID lbl_1_bss_888[60];
ANIMDATA *lbl_1_bss_814[29];
MDMIC_MODEL_WORK lbl_1_bss_29C[25];
s16 lbl_1_bss_298;
MDMIC_TRAJECTORY lbl_1_bss_238[2];
MDMIC_TRAJECTORY lbl_1_bss_1D8[2];
MDMIC_TRAJECTORY lbl_1_bss_118[4];
HUWINID lbl_1_bss_114[2];
HuVecF lbl_1_bss_E4[4];
HuVecF lbl_1_bss_B4[4];
HuVecF lbl_1_bss_9C[2];
f32 lbl_1_bss_90[3];
HuVecF lbl_1_bss_60[4];
f32 lbl_1_bss_48[6] ATTRIBUTE_ALIGN(8);
s16 lbl_1_bss_44;
s16 lbl_1_bss_42;
s16 lbl_1_bss_40;
OMOBJ *lbl_1_bss_3C;
OMOBJ *lbl_1_bss_38;
s16 lbl_1_bss_30[4];
s16 lbl_1_bss_2E;
s16 lbl_1_bss_2C;
s16 lbl_1_bss_2A;
s16 lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_1C[2];
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_C[2];
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
HUPROCESS *lbl_1_bss_0;
s32 lbl_1_data_0 = -1;
s16 lbl_1_data_4[4][4] = {{0, 1, 2, 3}, {1, 0, 2, 3}, {2, 0, 1, 3}, {3, 0, 1, 2}};
/* Sprite animation archives used by Mic Mode's menu and player indicators. */
u32 lbl_1_data_24[32] = {
    DATANUM(DATA_mdmic, 21), DATANUM(DATA_mdmic, 4),
    DATANUM(DATA_mdmic, 5), DATANUM(DATA_mdmic, 6),
    DATANUM(DATA_mdmic, 7), DATANUM(DATA_mdmic, 8),
    DATANUM(DATA_mdmic, 9), DATANUM(DATA_mdmic, 10),
    DATANUM(DATA_mdmic, 11), DATANUM(DATA_mdmic, 16),
    DATANUM(DATA_mdmic, 17), DATANUM(DATA_mdmic, 18),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 25),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 23),
    DATANUM(DATA_mdmic, 24), DATANUM(DATA_mdmic, 25),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 25),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 20),
    DATANUM(DATA_mdmic, 13), DATANUM(DATA_mdmic, 12),
    DATANUM(DATA_mdmic, 14), DATANUM(DATA_mdmic, 15),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 25),
    DATANUM(DATA_mdmic, 25), DATANUM(DATA_mdmic, 25),
    DATANUM(DATA_mdmic, 54), DATANUM(DATA_mdmic, 22)
};
s16 lbl_1_data_A4[18] = {2, 8, 6, 4, 2, 2, 2, 2, 4, 4, 4, 4, 5, 2, 4, 4, 1, 9};
MDMIC_SPRITE_DESC lbl_1_data_C8[60] = {
    {0, 0, 0, 10, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 0, 15, 0, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 1, 15, 0, 1, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 2, 15, 0, 2, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 3, 16, 1, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 4, 16, 1, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 5, 16, 1, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 0, 21, 0, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 1, 21, 0, 2, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 2, 21, 0, 5, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 3, 21, 0, 7, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 4, 21, 0, 4, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {2, 5, 21, 0, 6, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {3, 0, 22, 0, 0, {0.0f, 0.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {3, 1, 22, 0, 0, {0.0f, 0.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {3, 2, 22, 0, 0, {0.0f, 0.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {3, 3, 22, 0, 0, {0.0f, 0.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {4, 0, 22, 9, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {4, 1, 23, 10, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {5, 0, 22, 9, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {5, 1, 23, 10, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {6, 0, 22, 9, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {6, 1, 23, 10, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {7, 0, 22, 9, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {7, 1, 23, 10, 0, {22.0f, -24.0f}, {1.0f, 1.0f}, 0.0f},
    {8, 0, 25, 9, 0, {-44.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {8, 1, 24, 9, 10, {-25.0f, 25.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {8, 2, 24, 9, 0, {-4.0f, 24.0f}, {0.8999999761581421f, 0.8999999761581421f}, 0.0f},
    {8, 3, 23, 10, 1, {-22.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {9, 0, 25, 9, 0, {-44.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {9, 1, 24, 9, 10, {-25.0f, 25.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {9, 2, 24, 9, 0, {-4.0f, 24.0f}, {0.8999999761581421f, 0.8999999761581421f}, 0.0f},
    {9, 3, 23, 10, 1, {-22.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {10, 0, 25, 9, 0, {-44.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {10, 1, 24, 9, 10, {-25.0f, 25.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {10, 2, 24, 9, 0, {-4.0f, 24.0f}, {0.8999999761581421f, 0.8999999761581421f}, 0.0f},
    {10, 3, 23, 10, 1, {-22.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {11, 0, 25, 9, 0, {-44.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {11, 1, 24, 9, 10, {-25.0f, 25.0f}, {0.800000011920929f, 0.800000011920929f}, 0.0f},
    {11, 2, 24, 9, 0, {-4.0f, 24.0f}, {0.8999999761581421f, 0.8999999761581421f}, 0.0f},
    {11, 3, 23, 10, 1, {-22.0f, 24.0f}, {1.0f, 1.0f}, 0.0f},
    {12, 0, 27, 9, 0, {-125.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {12, 1, 27, 9, 0, {-85.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {12, 2, 27, 9, 0, {-5.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {12, 3, 27, 9, 0, {35.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {12, 4, 26, 10, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {13, 0, 28, 10, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 0, 9, 11, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 1, 11, 10, 0, {-56.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 2, 10, 10, 0, {-40.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 3, 10, 10, 10, {-24.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 4, 10, 10, 0, {-8.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 5, 10, 10, 0, {8.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 6, 10, 10, 11, {24.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 7, 10, 10, 0, {40.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {17, 8, 10, 10, 0, {56.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {16, 0, 30, 0, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {0, 1, 31, 0, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 6, 16, 1, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
    {1, 7, 16, 1, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f}
};
/* Character model animation archives used when the selection scene is built. */
s32 lbl_1_data_848[29] = {
    DATANUM(DATA_mdmic, 29), DATANUM(DATA_mdmic, 30),
    DATANUM(DATA_mdmic, 31), DATANUM(DATA_mdmic, 32),
    DATANUM(DATA_mdmic, 33), DATANUM(DATA_mdmic, 34),
    DATANUM(DATA_mdmic, 35), DATANUM(DATA_mdmic, 36),
    DATANUM(DATA_mdmic, 37), DATANUM(DATA_mdmic, 38),
    DATANUM(DATA_mdmic, 39), DATANUM(DATA_mdmic, 40),
    DATANUM(DATA_mdmic, 41), DATANUM(DATA_mdmic, 42),
    DATANUM(DATA_mdmic, 43), DATANUM(DATA_mdmic, 44),
    DATANUM(DATA_mdmic, 45), DATANUM(DATA_mdmic, 46),
    DATANUM(DATA_mdmic, 27), DATANUM(DATA_mdmic, 28),
    DATANUM(DATA_mdmic, 47), DATANUM(DATA_mdmic, 48),
    DATANUM(DATA_mdmic, 49), DATANUM(DATA_mdmic, 50),
    DATANUM(DATA_mdmic, 51), DATANUM(DATA_mdmic, 52),
    DATANUM(DATA_mdmic, 53), DATANUM(DATA_mdmic, 55),
    DATANUM(DATA_mdmic, 56)
};
HuVecF lbl_1_data_8BC[80] = {
    {0.0f, 275.0f, 0.0f},
    {-140.0f, 275.0f, 0.0f},
    {0.0f, 275.0f, 0.0f},
    {140.0f, 275.0f, 0.0f},
    {40.0f, 225.0f, 0.0f},
    {120.0f, 325.0f, 0.0f},
    {200.0f, 225.0f, 0.0f},
    {-140.0f, 275.0f, 0.0f},
    {0.0f, 275.0f, 0.0f},
    {140.0f, 275.0f, 0.0f},
    {40.0f, 275.0f, 0.0f},
    {120.0f, 275.0f, 0.0f},
    {200.0f, 275.0f, 0.0f},
    {0.0f, 275.0f, -800.0f},
    {-195.0f, 380.0f, 0.0f},
    {-65.0f, 380.0f, 0.0f},
    {65.0f, 380.0f, 0.0f},
    {195.0f, 380.0f, 0.0f},
    {-130.0f, 240.0f, 0.0f},
    {0.0f, 240.0f, 0.0f},
    {130.0f, 240.0f, 0.0f},
    {-195.0f, 100.0f, 0.0f},
    {-65.0f, 100.0f, 0.0f},
    {65.0f, 100.0f, 0.0f},
    {195.0f, 100.0f, 0.0f},
    {-180.0f, 65.0f, 0.0f},
    {-60.0f, 65.0f, 0.0f},
    {60.0f, 65.0f, 0.0f},
    {180.0f, 65.0f, 0.0f},
    {-180.0f, 125.0f, 0.0f},
    {-60.0f, 125.0f, 0.0f},
    {60.0f, 125.0f, 0.0f},
    {180.0f, 125.0f, 0.0f},
    {-180.0f, 275.0f, 0.0f},
    {-60.0f, 275.0f, 0.0f},
    {60.0f, 275.0f, 0.0f},
    {180.0f, 275.0f, 0.0f},
    {-320.0f, 375.0f, 0.0f},
    {-340.0f, 240.0f, 0.0f},
    {340.0f, 375.0f, 0.0f},
    {340.0f, 240.0f, 0.0f},
    {-225.0f, 300.0f, 0.0f},
    {-75.0f, 300.0f, 0.0f},
    {75.0f, 300.0f, 0.0f},
    {225.0f, 300.0f, 0.0f},
    {-180.0f, 175.0f, 0.0f},
    {-60.0f, 175.0f, 0.0f},
    {60.0f, 175.0f, 0.0f},
    {180.0f, 175.0f, 0.0f},
    {-220.0f, 175.0f, 0.0f},
    {-20.0f, 175.0f, 0.0f},
    {100.0f, 175.0f, 0.0f},
    {220.0f, 175.0f, 0.0f},
    {-220.0f, 65.0f, 0.0f},
    {-20.0f, 65.0f, 0.0f},
    {100.0f, 65.0f, 0.0f},
    {220.0f, 65.0f, 0.0f},
    {-220.0f, 125.0f, 0.0f},
    {-20.0f, 125.0f, 0.0f},
    {100.0f, 125.0f, 0.0f},
    {220.0f, 125.0f, 0.0f},
    {-160.0f, 0.0f, 0.0f},
    {160.0f, 0.0f, 0.0f},
    {-225.0f, 275.0f, 0.0f},
    {-75.0f, 275.0f, 0.0f},
    {75.0f, 275.0f, 0.0f},
    {225.0f, 275.0f, 0.0f},
    {-160.0f, 325.0f, 0.0f},
    {-80.0f, 225.0f, 0.0f},
    {0.0f, 325.0f, 0.0f},
    {80.0f, 225.0f, 0.0f},
    {160.0f, 325.0f, 0.0f},
    {-160.0f, 275.0f, 0.0f},
    {-80.0f, 275.0f, 0.0f},
    {0.0f, 275.0f, 0.0f},
    {80.0f, 275.0f, 0.0f},
    {160.0f, 275.0f, 0.0f},
    {-120.0f, 175.0f, 0.0f},
    {0.0f, 175.0f, 0.0f},
    {120.0f, 175.0f, 0.0f}
};
s32 lbl_1_data_C7C = -1;
char lbl_1_data_C80[34] = "# ========== win callback :: %d\n";
s16 lbl_1_data_CA2[3] = {0};
s16 lbl_1_data_CA8[6] = {0};
u32 lbl_1_data_CB4[3] = {0};
char lbl_1_data_CC0[] = "\n>>>>>>>>>> mdmicdll :: READ INIT!! <<<<<<<<<<\n";
char lbl_1_data_CF0[] = "0x%x :: _effect\n";
char lbl_1_data_D01[] = "0x%x :: _gamemes\n";
char lbl_1_data_D13[] = "0x%x :: _mgconst\n";
char lbl_1_data_D25[] = "0x%x :: _win\n";
char lbl_1_data_D33[3] = "\n";
s16 lbl_1_data_D36[5] = {665, 666, 667, 669, 670};
char lbl_1_data_D40[] = "\n>>>>>>>>>> mdmicdll :: READ END!! <<<<<<<<<<\n";
char lbl_1_data_D6F[] = ">>>>>>>>>> mdmicdll :: objsetup!! <<<<<<<<<<\n";
char lbl_1_data_D9D[] = "\n>>>>>>>>>> mdmicdll :: dllreturn!! <<<<<<<<<<\n";
char lbl_1_data_DCD[] = "-----===== CAMERA TEST =====-----";
char lbl_1_data_DEF[] = "CENTER : %.2f, %.2f, %.2f";
char lbl_1_data_E09[] = "ROT    : %.2f, %.2f, %.2f";
char lbl_1_data_E23[15] = "ZOOM   : %.2f";
s16 lbl_1_data_E32[3] = {-1, -1};
s32 lbl_1_data_E38[3] = {-1, -1, -1};
char lbl_1_data_E44[] = "gN00m1-itemhook_R";
char lbl_1_data_E56[] = "dpic00";
char lbl_1_data_E5D[] = "dpic01";
s16 lbl_1_data_E64[6] = {0};
char lbl_1_data_E70[] = "ys77120";
char lbl_1_data_E78[] = "ys77121";
char lbl_1_data_E80[] = "ys77060";
char lbl_1_data_E88[] = "ys77061";
MDMIC_CHARACTER_CHOICE lbl_1_data_E90[11] = {
    {{-1, -1, 1, 5, 4, -1, -1, -1}, -1},
    {{-1, -1, 2, 6, 5, 4, 0, -1}, -1},
    {{-1, -1, 3, -1, 6, 5, 1, -1}, -1},
    {{-1, -1, -1, -1, -1, 6, 2, -1}, -1},
    {{0, 1, 5, 9, 8, 7, -1, -1}, -1},
    {{1, 2, 6, 10, 9, 8, 4, 0}, -1},
    {{2, 3, 10, 10, 10, 9, 5, 1}, -1},
    {{-1, 4, 8, -1, -1, -1, -1, -1}, -1},
    {{4, 5, 9, -1, -1, -1, 7, -1}, -1},
    {{5, 6, 10, -1, -1, -1, 8, 4}, -1},
    {{6, -1, -1, -1, -1, -1, 9, 5}, -1}
};
s16 lbl_1_data_F56[5] = {665, 666, 667, 669, 670};
char lbl_1_data_F60[64] = "\n-----===== MARIO PARTY 6 :: MICROPHONE MODE =====-----\n\n";

void fn_1_1FAC(
    HU3D_MODELID modelId, const HuVecF *a, const HuVecF *b, const HuVecF *c,
    float time, float duration);
float fn_1_1ED8(float startValue, float peakValue, float elapsedFrames, float durationFrames);
void fn_1_1A58(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);
void fn_1_1918(HU3D_MODELID modelId, float rotY);
void fn_1_15B0(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);
float fn_1_156C(float startValue, float endValue, float time, float duration);
float fn_1_1484(float startValue, float endValue, float time, float duration);
void fn_1_1340(Vec *current, const Vec *target, float weight);
float fn_1_1310(float current, float target, float weight);
void fn_1_1108(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t);
float fn_1_10AC(float start, float middle, float end, float time);
void fn_1_FC0(void);
void fn_1_EDC(void);
void fn_1_C78(void);
void fn_1_824(void);
void fn_1_758(void);
void fn_1_44C(void);
void fn_1_2FC(void);
void fn_1_1A4(void);
void fn_1_0(HUWINID winId, u32 messageNo, s16 cueNo);
void fn_1_2564(MDMIC_CAMERA_WORK *work);
void fn_1_2514(MDMIC_CAMERA_WORK *work);
void fn_1_33C8(void);
void fn_1_329C(void);
void fn_1_3240(void);
void fn_1_4434(void);
void fn_1_4380(void);
void fn_1_4788(void);
void fn_1_4848(s32 recordFrames, s16 modelId);
void fn_1_4AE8(OMOBJ *obj);
void fn_1_6DC4(void);
void fn_1_79E4(void);
void fn_1_77B8(s16 memberNo);
void fn_1_7A94(OMOBJ *obj);
void fn_1_7F14(OMOBJ *obj);
void fn_1_A030(void);
void fn_1_9F0C(void);
void fn_1_9E98(s16 memberNo);
void fn_1_9E3C(s16 index);
s16 fn_1_AED8(s16 current, MDMIC_CHARACTER_CHOICE *entries,
              s16 direction, s16 count);
s16 fn_1_AAF0(s16 current, s16 direction, s16 fallbackDirection, s16 lastDirection,
              MDMIC_CHARACTER_CHOICE *entries, s16 count);
void fn_1_AA68(OMOBJ *obj);
s16 fn_1_CE54(void);
void fn_1_CE3C(s16 state);
void fn_1_D248(OMOBJ *obj);
void fn_1_EF08(OMOBJ *obj);
void fn_1_EE70(OMOBJ *obj);
void fn_1_EE40(OMOBJ *obj);
void fn_1_F9E0(OMOBJ *obj);
void fn_1_14E70(OMOBJ *obj);
void fn_1_13870(OMOBJ *obj);
void fn_1_12D38(OMOBJ *obj);
void fn_1_12744(OMOBJ *obj);
void fn_1_15D8C(OMOBJ *obj);
void fn_1_16784(OMOBJ *obj);
void fn_1_18874(OMOBJ *obj);
void fn_1_19134(OMOBJ *obj);
void fn_1_18C58(OMOBJ *obj);
void fn_1_1A878(OMOBJ *obj);
void fn_1_1A1D8(OMOBJ *obj);
void fn_1_19B28(OMOBJ *obj);
void fn_1_1AE7C(OMOBJ *obj);
void fn_1_1A914(OMOBJ *obj);
void fn_1_1B5F0(OMOBJ *obj);
void fn_1_1C308(OMOBJ *obj);
void fn_1_1C778(OMOBJ *obj);
void fn_1_1CC48(OMOBJ *obj);
void fn_1_1D3B8(OMOBJ *obj);
void fn_1_1DB64(OMOBJ *obj);
void fn_1_1FE30(s16 modelNo);
void fn_1_1F864(OMOBJ *obj);
void fn_1_1F070(OMOBJ *obj);
void fn_1_2124C(OMOBJ *obj);
void fn_1_203E8(OMOBJ *obj);
void fn_1_219F0(OMOBJ *obj);
void fn_1_216D4(OMOBJ *obj);
void fn_1_23F2C(void);
void fn_1_23B18(OMOBJ *obj);
void fn_1_22D54(OMOBJ *obj);
void fn_1_2299C(OMOBJ *obj);
void fn_1_24420(void);
void _epilog(void);
int _prolog(void);
void fn_1_25B50(void);
void fn_1_25ADC(OMOBJ *obj, MDMIC_CAMERA_WORK *state);
void fn_1_258B4(OMOBJ *obj, MDMIC_CAMERA_WORK *state);
s16 fn_1_2666C(void);
s16 fn_1_26364(void);
s32 fn_1_27158(void);
s16 fn_1_276E0(void);
s32 fn_1_272D4(void);
s16 fn_1_29224(s16 returningFromPreviousStep);
s16 fn_1_29FA4(s16 unused);
s16 fn_1_2B844(s16 returningFromPreviousStep);
s16 fn_1_2BBBC(s16 unused);
s16 fn_1_34C78(s16 mode);
s16 fn_1_33AD8(s16 returningToSelection);
s16 fn_1_32934(s16 returningToSelection);
s16 fn_1_31770(s16 returningToSelection);
void fn_1_3107C(void);
s16 fn_1_30510(void);
s16 fn_1_2F9A4(void);
s16 fn_1_2EE38(void);
s16 fn_1_2DD78(s16 returningFromPreviousStep);
s16 fn_1_2D06C(s16 returningFromPreviousStep);
s16 fn_1_2CD28(s16 returningFromPreviousStep);
void fn_1_4538(void);
void fn_1_4C78(OMOBJ *obj);
void fn_1_539C(OMOBJ *obj);
void fn_1_63F8(OMOBJ *obj);
void fn_1_84B0(OMOBJ *obj);
void fn_1_C820(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction, s16 attributeEnabled);
void fn_1_F4D8(OMOBJ *obj);
void fn_1_2460(void);
void fn_1_2868(MDMIC_CAMERA_CALLBACK callback);
void fn_1_481C(void);
void fn_1_51CC(void);
s32 fn_1_6028(void);
void fn_1_60D4(OMOBJ *obj);
void fn_1_6364(OMOBJ *obj);
void fn_1_70B4(OMOBJ *obj);
void fn_1_7588(OMOBJ *obj);
void fn_1_7DCC(s16 animationNo);
void fn_1_7EE4(void);
void fn_1_9244(void);
void fn_1_92DC(void);
void fn_1_202DC(s16 gameNo);
void fn_1_251FC(void);
s32 fn_1_262E8(void);
void fn_1_2400(void);
void fn_1_25B4(MDMIC_CAMERA_WORK *camera, float weight);
void fn_1_2878(OMOBJ *obj, MDMIC_CAMERA_WORK *camera);
void fn_1_2F88(OMOBJ *obj, MDMIC_CAMERA_WORK *state);
void fn_1_2FD4(OMOBJ *obj);
void fn_1_30A4(MDMIC_CAMERA_CALLBACK callback);
void fn_1_3404(s16 winNo);
void fn_1_3474(s16 winNo);
void fn_1_34E4(s16 winNo);
s16 fn_1_3520(s16 winNo, s16 mode);
void fn_1_35F4(s16 winNo, s32 messNum, s16 speed);
void fn_1_36B0(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_3720(void);
void fn_1_3948(void);
void fn_1_39A4(s16 winNo);
void fn_1_3B10(void);
void fn_1_3BCC(void);
s16 fn_1_3C2C(s16 mode);
void fn_1_3D24(s16 winNo, s32 messNum, s16 speed);
void fn_1_3F5C(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_4150(s32 messNum);
void fn_1_42D4(void);
void fn_1_4438(HUSPR_GROUPID groupId, s32 attr);
void fn_1_44B8(HUSPR_GROUPID groupId, s32 attr);
void fn_1_4784(void);
void fn_1_4AC0(void);
void fn_1_4C08(void);
void fn_1_4C74(void);
void fn_1_50C8(OMOBJ *obj);
void fn_1_5328(void);
void fn_1_5280(OMOBJ *obj);
void fn_1_5918(void);
void fn_1_5A90(OMOBJ *obj);
void fn_1_6BF8(OMOBJ *obj);
void fn_1_69D8(void);
void fn_1_7254(OMOBJ *obj);
void fn_1_7304(OMOBJ *obj);
void fn_1_7498(void);
void fn_1_7554(void);
void fn_1_7728(OMOBJ *obj);
void fn_1_7A34(void);
void fn_1_7D28(s16 index);
void fn_1_7E70(void);
void fn_1_86F4(OMOBJ *obj);
s32 fn_1_87F0(s16 first, s16 second);
void fn_1_88F0(s16 winNo, u32 messNum);
void fn_1_897C(u32 messNum1, u32 messNum2);
void fn_1_8A9C(void);
void fn_1_8B3C(void);
void fn_1_8CBC(void);
void fn_1_8D14(s16 playerNo, s16 bank);
void fn_1_8E24(s16 playerNo, s16 bank);
void fn_1_8F38(s16 playerNo, s16 bank);
void fn_1_8F10(s16 playerNo);
void fn_1_8F84(s16 playerNo, s16 bank);
void fn_1_9098(s16 playerNo, s16 bank);
void fn_1_9070(s16 playerNo);
void fn_1_90E4(s16 playerNo, s16 bank);
void fn_1_91F8(s16 playerNo, s16 bank);
void fn_1_91D0(s16 playerNo);
void fn_1_92F8(OMOBJ *obj);
void fn_1_9980(OMOBJ *obj);
void fn_1_9C80(s16 memberNo, Point3d *pos, f32 xOffset, f32 yOffset);
void fn_1_9C28(void);
void fn_1_9D18(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);
void fn_1_A0BC(s16 modelNo, Vec *pos);
void fn_1_A144(s32 modelNo, HuVecF *pos, s16 bank);
void fn_1_A318(s32 modelNo);
void fn_1_A3B0(OMOBJ *obj);
void fn_1_A870(OMOBJ *obj);
s16 fn_1_AF80(s16 padNo, s16 *current,
    MDMIC_CHARACTER_CHOICE *entries, s16 count);
void fn_1_B5A8(s16 modelNo, s16 dataNo);
void fn_1_B500(s16 modelNo, s16 animNo, s16 dataNo, s16 bank);
void fn_1_B634(s16 modelNo, Point3d *position, f32 scale);
void fn_1_B6CC(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_BAD8(s16 modelNo, s16 time, s16 duration);
void fn_1_BEA8(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_C498(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);
void fn_1_CBD4(s16 modelNo, float angle, s16 time, s16 duration);
s16 fn_1_CE6C(void);
void fn_1_CEA4(OMOBJ_FUNC callback);
void fn_1_CF3C(void);
void fn_1_D16C(void);
void fn_1_F8DC(s16 modelNo);
void fn_1_F978(s16 modelNo);
void fn_1_1018C(OMOBJ *obj);
void fn_1_103F0(OMOBJ *obj);
void fn_1_1053C(OMOBJ *obj);
void fn_1_1064C(OMOBJ *obj);
void fn_1_11534(OMOBJ *obj);
void fn_1_119B8(OMOBJ *obj);
void fn_1_11B84(OMOBJ *obj);
void fn_1_12334(OMOBJ *obj);
void fn_1_125D8(OMOBJ *obj);
void fn_1_15994(OMOBJ *obj);
void fn_1_16390(s16 modelNo);
void fn_1_166B0(s16 modeNo);
void fn_1_170D4(OMOBJ *obj);
void fn_1_17340(OMOBJ *obj);
void fn_1_17E8C(OMOBJ *obj);
void fn_1_17D90(OMOBJ *obj);
void fn_1_17648(OMOBJ *obj);
void fn_1_199B4(OMOBJ *obj);
void fn_1_198CC(OMOBJ *obj);
void fn_1_1BD7C(OMOBJ *obj);
void fn_1_1C2E4(OMOBJ *obj);
void fn_1_1CC00(OMOBJ *obj);
void fn_1_1CC24(OMOBJ *obj);
void fn_1_1DB40(OMOBJ *obj);
void fn_1_1DF74(OMOBJ *obj);
void fn_1_1E1B0(OMOBJ *obj);
void fn_1_1E9D8(OMOBJ *obj);
void fn_1_1E248(OMOBJ *obj);
void fn_1_24614(void);
void fn_1_243C8(OMOBJ *obj);
void fn_1_2438C(OMOBJ *obj);
void fn_1_25550(OMOBJ *obj, MDMIC_CAMERA_WORK *camera);
s32 fn_1_29184(void);
s16 fn_1_29E70(s16 *activePlayer, s16 *firstComputerPlayer, s16 *lastPlayerSlot);
s16 fn_1_2B590(s16 returningFromPreviousStep);
s16 fn_1_2BA88(s16 *activePlayer, s16 *firstComputerPlayer, s16 *lastPlayerSlot);
float fn_1_35A10(float startValue, float endValue, float time, float duration);
float fn_1_35AF8(float startValue, float endValue, float time, float duration);
float fn_1_35B3C(float current, float target, float weight);
void fn_1_35B6C(s16 layerNo);
void fn_1_35BD8(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_36058(OMOBJ *obj);
void fn_1_36158(s16 textureSlot, HuVecF *position);
void fn_1_3624C(OMOBJ *obj);
void fn_1_36430(OMOBJ *obj);
void fn_1_364E0(void);
void fn_1_36530(void);
void fn_1_3660C(Point3d *position);
void fn_1_366CC(s16 layerNo);
void fn_1_37088(void);
void fn_1_370B4(void);
void fn_1_370D8(
    s16 particleNo, float lifetime, float targetScale, HuVecF *position, GXColor color);
void fn_1_371D8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_37498(void);
void fn_1_375AC(void);
void fn_1_375D8(s16 effectNo, float duration, HuVecF *position, GXColor color);
void fn_1_376C0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_37C08(void);
void fn_1_37DC8(void);
void fn_1_37E20(void);
void fn_1_37EC0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3864C(void);
void fn_1_3874C(void);
void fn_1_38778(s16 effectNo, float duration, HuVecF *position);
void fn_1_38844(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3912C(void);
void fn_1_392C0(void);
void fn_1_39318(s16 index, s16 visible);
void fn_1_39394(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_39498(s16 effectNo);
void fn_1_39520(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_39998(void);
void fn_1_39B04(void);
void fn_1_39B5C(s16 index, s16 visible);
void fn_1_39BD8(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_39D10(s16 effectNo);
void fn_1_39D64(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_3A364(void);
void fn_1_3A4D0(void);
void fn_1_3A528(s16 modelNo, HuVecF *pos, s16 display);
void fn_1_3A8EC(s16 groupNo, s16 show);
void fn_1_3A9A0(s16 groupNo, HuVecF *spawnPosition, GXColor *color);
void fn_1_3AB08(s16 groupNo);
void fn_1_3AB88(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_3B134(void);
void fn_1_3B360(void);
void fn_1_3B3E4(HUPROCESS *process);
void fn_1_3BE34(void);
void fn_1_3C060(HuVecF *position);
void fn_1_3C258(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_3C554(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_3C9F8(void);
void fn_1_3CAB8(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo);

static inline void fn_1_1404_inlined_mdmicdll_text_00000c(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t);
static inline float fn_1_13A8_inlined_mdmicdll_text_00000c(float startValue, float controlValue,
                                                           float endValue, float progress);
static inline float fn_1_620_inlined_mdmicdll_text_00000c(float a, float b, float c, float t);
static inline void fn_1_89E0_inlined_inlined_mdmicdll_text_07F14c(OMOBJ *obj);
static inline void fn_1_14E70_move_player_mdmicdll_text_12744c(s16 playerNo, s16 animationFrame);
static inline void fn_1_1A1D8_move_player_mdmicdll_text_1A1D8c(
    MDMIC_MODEL_WORK *entry, const HuVecF *targetPosition, s16 time);
static inline void MdmicPlayerExitTransition_mdmicdll_text_1A914c(MDMIC_PLAYER_WORK *player,
                                                                  s16 frame);
static inline void MdmicExitTransition_mdmicdll_text_1A914c(MDMIC_MODEL_WORK *entry, s16 frame);
static inline void MdmicPlayerTransition_mdmicdll_text_1B5F0c(MDMIC_PLAYER_WORK *player, s16 frame);
static inline void MdmicModelTransition_mdmicdll_text_1B5F0c(MDMIC_MODEL_WORK *entry, s16 frame);
static inline void MdmicPlayerTransition_mdmicdll_text_1CC48c(MDMIC_PLAYER_WORK *player, s16 frame);
static inline void MdmicModelTransition_mdmicdll_text_1CC48c(MDMIC_MODEL_WORK *entry, s16 frame);
static inline void MdmicPlayerTransition_mdmicdll_text_1D3B8c(MDMIC_PLAYER_WORK *player, s16 frame);
static inline void MdmicModelTransition_mdmicdll_text_1D3B8c(MDMIC_MODEL_WORK *entry, s16 frame);
static inline int CurrentSubGameGet_mdmicdll_text_216D4c(void);
static inline int CurrentMinigameGet_mdmicdll_text_216D4c(void);
static inline void DebugValue_mdmicdll_text_2299Cc(s32 value);
static inline s16 fn_1_26364_for_2666C_mdmicdll_text_26364c(void);
static inline s16 fn_1_381C_inlined_mdmicdll_text_26364c(s16 winNo, s16 mode);
static inline s16 fn_1_3F28_inlined_mdmicdll_text_26364c(s16 mode);
static inline void fn_1_3474_inlined_mdmicdll_text_27158c(s16 windowNo);
static inline void fn_1_3B10_inlined_mdmicdll_text_27158c(void);
static inline s32 fn_1_5B48_inlined_mdmicdll_text_27158c(void);
static inline s16 fn_1_26364_for_276E0_mdmicdll_text_272D4c(void);
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_29224c(void);
static inline s16 fn_1_32904_inlined_mdmicdll_text_29224c(void);
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_29FA4c(void);
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_2BBBCc(void);
static inline s32 fn_1_272D4_for_34C78_mdmicdll_text_2CD28c(void);
static inline void fn_1_3F5C_mdmicdll_text_2CD28c(s16 windowNo, s32 messageNo,
                             s16 insertPosition);
static inline s16 fn_1_26364_for_2D06C_mdmicdll_text_2CD28c(void);
static inline void fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(s32 messageNo);
static inline f32 fn_1_4C78_ease180_mdmicdll_text_4C78c(f32 start, f32 end, f32 elapsed,
                                                        f32 duration);
static inline f32 fn_1_4C78_ease90_mdmicdll_text_4C78c(f32 start, f32 end, f32 elapsed,
                                                       f32 duration);
static inline f32 fn_1_10AC_inlined_mdmicdll_text_63F8c(f32 start, f32 control, f32 end,
                                                        f32 progress);
static inline void fn_1_1108_inlined_mdmicdll_text_63F8c(Point3d *position, const Point3d *start,
                                                         const Point3d *control, const Point3d *end,
                                                         f32 progress);
static inline void fn_1_8950_inlined_07E70_fn_1_7E70c(void);
/* Window text callbacks use this to play a cue once per message ID. */
void fn_1_0(HUWINID winId, u32 messageNo, s16 cueNo)
{
    s32 sentinel[1] = {-1};
    MDMIC_AUDIO_TABLE table = { { 949, 950, 951, 952, 953, 954, 955, -1, 941, 942, 943, 944, 945,
                                  946, 947, -1 } };
    s16 messageIndex;

    cueNo -= 1;
    OSReport(lbl_1_data_C80, (s16) cueNo);
    if ((u32) lbl_1_data_C7C != messageNo) {
        lbl_1_data_C7C = messageNo;
        messageIndex = 0;
checkMessage:
        if (sentinel[messageIndex] == -1) {
            HuAudFXPlay(table.value[cueNo]);
            return;
        }
        if (messageNo == (u32) sentinel[messageIndex]) {
            if (cueNo >= 8) {
                HuAudFXPlayPan(table.value[cueNo], 80);
                return;
            }
            HuAudFXPlayPan(table.value[cueNo], 48);
            return;
        }
        messageIndex += 1;
        goto checkMessage;
    }
}

/* Caches saved bank options and microphone activity records for Mic Mode. */
void fn_1_1A4(void) {
    lbl_1_bss_2C = 0;
    lbl_1_bss_2E = 0;
    if (GWBankFlagGet(2) != 0) {
        lbl_1_bss_2C = 1;
    }
    if (GWBankFlagGet(3) != 0) {
        lbl_1_bss_2E = 1;
    }
    lbl_1_data_CA2[0] = 1;
    lbl_1_data_CA2[1] = 1;
    lbl_1_data_CA2[2] = 0;
    if (GWBankFlagGet(MDMIC_BANK_ACTIVITY_THREE_FLAG) != 0) {
        lbl_1_data_CA2[2] = 1;
    }
    lbl_1_data_CB4[0] = GWMikeActRecordGet(0);
    lbl_1_data_CB4[1] = GWMikeActRecordGet(1);
    lbl_1_data_CB4[2] = GWMikeActRecordGet(2);
    lbl_1_data_CA8[0] = 1;
    lbl_1_data_CA8[1] = 1;
    lbl_1_data_CA8[2] = 1;
    lbl_1_data_CA8[3] = 1;
    lbl_1_data_CA8[4] = 1;
}

/* Restores missing player character motions before their models are used. */
void fn_1_2FC(void)
{
    s16 playerNo;
    s32 readStatus;

    playerNo = 0;
    while (playerNo < 4) {
        if ((void *) CharMotionAMemPGet((s16) GwPlayer[playerNo].charNo) == NULL) {
            break;
        }
        playerNo += 1;
    }
    if (playerNo != 4) {
        CharDataClose(-1);
        playerNo = 0;
        while (playerNo < 4) {
            readStatus = HuDataDirReadAsync(CharDataDirTbl[GwPlayer[playerNo].charNo][4]);
            if (readStatus != -1) {
                while (HuDataGetAsyncStat(readStatus) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[playerNo].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[playerNo].charNo][4]);
            playerNo += 1;
        }
    }
}

/* Loads player and Mic Mode motions in the child process created by Mic Mode setup. */
void fn_1_44C(void)
{
    MDMicdllMotionDirectories motionDirectories = {
        {DATA_micquiz, DATA_mikeact, DATA_inst}};
    s16 playerNo;
    s32 motionPlayerNo;
    s32 characterReadStatus;
    s32 modeReadStatus;

    playerNo = 0;
    while (playerNo < 4) {
        if ((void *) CharMotionAMemPGet((s16) GwPlayer[playerNo].charNo) == NULL) {
            break;
        }
        playerNo += 1;
    }
    if (playerNo != 4) {
        CharDataClose(-1);
        playerNo = 0;
        while (playerNo < 4) {
            characterReadStatus = HuDataDirReadAsync(CharDataDirTbl[GwPlayer[playerNo].charNo][4]);
            if (characterReadStatus != -1) {
                while (HuDataGetAsyncStat(characterReadStatus) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[playerNo].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[playerNo].charNo][4]);
            playerNo += 1;
        }
    }

    modeReadStatus = HuDataDirReadAsync(motionDirectories.value[lbl_1_bss_9DC.modeSelection]);
    if (modeReadStatus != -1) {
        while (HuDataGetAsyncStat(modeReadStatus) == 0) {
            HuPrcVSleep();
        }
    }
    if (lbl_1_bss_9DC.modeSelection == 0) {
        HuAR_MRAMtoARAM(motionDirectories.value[lbl_1_bss_9DC.modeSelection]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(motionDirectories.value[lbl_1_bss_9DC.modeSelection]);
    }
    if (lbl_1_bss_9DC.modeSelection == 1) {
        motionPlayerNo = 0;
        while (motionPlayerNo < 4) {
            modeReadStatus = HuDataDirReadAsync(CharDataDirTbl[GwPlayer[motionPlayerNo].charNo][2]);
            if (modeReadStatus != -1) {
                while (HuDataGetAsyncStat(modeReadStatus) == 0) {
                    HuPrcVSleep();
                }
            }
            HuAR_MRAMtoARAM(CharDataDirTbl[GwPlayer[motionPlayerNo].charNo][2]);
            while (HuARDMACheck() != 0) {
                HuPrcVSleep();
            }
            HuDataDirClose(CharDataDirTbl[GwPlayer[motionPlayerNo].charNo][2]);
            motionPlayerNo += 1;
        }
    }
    lbl_1_bss_30[0] = 1;
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

/* fn_1_3107C calls this after confirming a selection to start motion loading. */
void fn_1_758(void) {
    lbl_1_bss_30[0] = 0;
    OSReport(lbl_1_data_CC0);
    OSReport(lbl_1_data_CF0, _effect);
    OSReport(lbl_1_data_D01, _gamemes);
    OSReport(lbl_1_data_D13, _mgconst);
    OSReport(lbl_1_data_D25, _win);
    HuAMemDump();
    OSReport(lbl_1_data_D33);
    HuDataDirClose(DATA_mdmic);
    HuPrcChildCreate(fn_1_44C, MDMIC_LOAD_PROCESS_PRIORITY, MDMIC_LOAD_PROCESS_STACK_SIZE, 0,
                     lbl_1_bss_0);
}

/* Called before game entry to copy the menu's player choices into game records. */
void fn_1_824(void)
{
    s16 playerNo;

    if (lbl_1_bss_9DC.modeSelection == 2) {
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][0]].reservedSetup = 0;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][1]].reservedSetup = 1;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][2]].reservedSetup = 1;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][3]].reservedSetup = 1;
    } else {
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][0]].reservedSetup = 0;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][1]].reservedSetup = 0;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][2]].reservedSetup = 0;
        lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][3]].reservedSetup = 0;
    }

    for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
        GwPlayer[playerNo].comF = lbl_1_bss_9A4[playerNo].isComputer;
        GwPlayer[playerNo].comDif = lbl_1_bss_9A4[playerNo].computerDifficulty;
        GwPlayer[playerNo].charNo = lbl_1_bss_9A4[playerNo].chrSel;
        GwPlayer[playerNo].padNo = lbl_1_bss_9A4[playerNo].padNo;
        GwPlayer[playerNo].team = lbl_1_bss_9A4[playerNo].reservedSetup;
    }

    for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
        GwPlayerConf[playerNo].type = GwPlayer[playerNo].comF;
        GwPlayerConf[playerNo].comDif = GwPlayer[playerNo].comDif;
        GwPlayerConf[playerNo].charNo = GwPlayer[playerNo].charNo;
        GwPlayerConf[playerNo].padNo = GwPlayer[playerNo].padNo;
        GwPlayerConf[playerNo].grpNo = GwPlayer[playerNo].team;
    }
}

/* fn_1_24420 calls this after the exit wipe to hand off to the selected game or menu. */
void fn_1_C78(void)
{
    OMOVLHIS *history;
    s32 minigameNo;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_30[0] == 0);
    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_9DC.modeSelection);
    OSReport(lbl_1_data_D40);
    OSReport(lbl_1_data_CF0, _effect);
    OSReport(lbl_1_data_D01, _gamemes);
    OSReport(lbl_1_data_D13, _mgconst);
    OSReport(lbl_1_data_D25, _win);
    HuAMemDump();
    OSReport(lbl_1_data_D33);
    if (lbl_1_bss_9DC.modeSelection == 0) {
        omOvlCallEx(DLL_micquizdll, 1, 0, 0);
    } else if (lbl_1_bss_9DC.modeSelection == 1) {
        switch (lbl_1_bss_9DC.gameSelection) {
        case 0:
            GwSystem.subGameNo = 0;
            break;
        case 1:
            GwSystem.subGameNo = 1;
            break;
        case 2:
            GwSystem.subGameNo = 2;
            break;
        }
        omOvlCallEx(DLL_mikeactdll, 1, 0, 0);
    } else if (lbl_1_bss_9DC.modeSelection == 2) {
        MgInstExitF = 1;
        MgPauseExitF = 1;
        _SetFlag(FLAG_INST_MG_MODE);
        _SetFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
        GwMgNightF = 0;
        GwSystem.mgInstDispF = 1;
        minigameNo = lbl_1_data_D36[lbl_1_bss_9DC.gameSelection];
        GWMgUnlockSet(minigameNo);
        GwSystem.mgNo = minigameNo - 601;
        omOvlCallEx(4, 1, 0, 0);
    }
}

/* Scene initialization releases mode archives and cached character motion archives. */
void fn_1_EDC(void) {
    s16 characterNo;

    HuARDirFree(DATA_micquiz);
    HuDataDirClose(DATA_mikeact);
    HuDataDirClose(DATA_inst);
    characterNo = 0;
    while (characterNo < GW_CHARA_MAX) {
        HuARDirFree((&CharDataDirTbl[characterNo][0])[2]);
        characterNo += 1;
    }
    OSReport(lbl_1_data_D6F);
    OSReport(lbl_1_data_CF0, _effect);
    OSReport(lbl_1_data_D01, _gamemes);
    OSReport(lbl_1_data_D13, _mgconst);
    OSReport(lbl_1_data_D25, _win);
    HuAMemDump();
    OSReport(lbl_1_data_D33);
}

/* Overlay exit closes character data and releases mode and motion archives. */
void fn_1_FC0(void) {
    s16 characterNo;

    CharDataClose(-1);
    HuARDirFree(DATA_micquiz);
    HuDataDirClose(DATA_mikeact);
    HuDataDirClose(DATA_inst);
    characterNo = 0;
    while (characterNo < GW_CHARA_MAX) {
        HuARDirFree((&CharDataDirTbl[characterNo][0])[2]);
        characterNo += 1;
    }
    OSReport(lbl_1_data_D9D);
    OSReport(lbl_1_data_CF0, _effect);
    OSReport(lbl_1_data_D01, _gamemes);
    OSReport(lbl_1_data_D13, _mgconst);
    OSReport(lbl_1_data_D25, _win);
    HuAMemDump();
    OSReport(lbl_1_data_D33);
}

/* Path helpers evaluate one coordinate of a quadratic curve at normalized progress. */
float fn_1_10AC(float start, float middle, float end, float time)
{
    float inverse = 1.0f - time;

    return end * (time * time)
        + (start * (inverse * inverse)
        + 2.0f * (middle * (inverse * time)));
}

/* The vector path helper evaluates one coordinate between its endpoints and control point. */
static inline float fn_1_620_inlined_mdmicdll_text_00000c(float a, float b, float c, float t)
{
    float inv = 1.0f - t;

    return (c * (t * t))
        + ((a * (inv * inv)) + ((b * (inv * t)) * 2.0f));
}

/* Movement helpers evaluate a quadratic path from the supplied endpoints and control point. */
void fn_1_1108(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_620_inlined_mdmicdll_text_00000c(a->x, b->x, c->x, t);
    dst->y = fn_1_620_inlined_mdmicdll_text_00000c(a->y, b->y, c->y, t);
    dst->z = fn_1_620_inlined_mdmicdll_text_00000c(a->z, b->z, c->z, t);
}

/* Scene updates use this weighted step to approach a target. */
float fn_1_1310(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

/* Camera and scene updates ease each coordinate toward the supplied target. */
void fn_1_1340(Vec *current, const Vec *target, float weight)
{
    current->x = fn_1_1310(current->x, target->x, weight);
    current->y = fn_1_1310(current->y, target->y, weight);
    current->z = fn_1_1310(current->z, target->z, weight);
}

/* Scene transitions ease from the starting value to the endpoint over their duration. */
float fn_1_1484(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Scene-motion helpers use this clamped linear interpolation each frame. */
float fn_1_156C(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((time / duration) * (endValue - startValue));
}

/* Movement callbacks advance a model along a line and ease its heading toward the movement. */
void fn_1_15B0(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = fn_1_156C(start->x, end->x, time, duration);
    pos.y = fn_1_156C(start->y, end->y, time, duration);
    pos.z = fn_1_156C(start->z, end->z, time, duration);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    rot.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_1310(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

/* After movement ends, transition callbacks ease the model toward its final heading. */
void fn_1_1918(HU3D_MODELID modelId, float rotY)
{
    HuVecF modelRot;
    HuVecF rot;

    Hu3DModelRotGet(modelId, &modelRot);
    rot.y = rotY;
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_1310(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelRotSetV(modelId, &rot);
}

/* Movement callbacks follow the path until its duration ends, then settle the final heading. */
void fn_1_1A58(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        fn_1_15B0(modelId, start, end, time, duration);
    } else {
        fn_1_1918(modelId, rotY);
    }
}

/* Transition callbacks pulse from the starting value toward the midpoint value and back. */
float fn_1_1ED8(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sind((180.0f / durationFrames) * elapsedFrames));
}

/* The curved movement helper evaluates one coordinate from its three control values. */
static inline float fn_1_13A8_inlined_mdmicdll_text_00000c(float startValue, float controlValue,
                                                           float endValue, float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

/* The model path helper evaluates each coordinate of its quadratic curve. */
static inline void fn_1_1404_inlined_mdmicdll_text_00000c(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_13A8_inlined_mdmicdll_text_00000c(a->x, b->x, c->x, t);
    dst->y = fn_1_13A8_inlined_mdmicdll_text_00000c(a->y, b->y, c->y, t);
    dst->z = fn_1_13A8_inlined_mdmicdll_text_00000c(a->z, b->z, c->z, t);
}

/* Movement callbacks follow a curved path while easing the model to face its direction of
 * travel. */
void fn_1_1FAC(
    HU3D_MODELID modelId, const HuVecF *a, const HuVecF *b, const HuVecF *c,
    float time, float duration)
{
    HuVecF pos;
    HuVecF modelPos;
    HuVecF modelRot;
    float value;

    value = fn_1_156C(0.0f, 1.0f, time, duration);
    fn_1_1404_inlined_mdmicdll_text_00000c(&pos, a, b, c, value);
    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    modelPos.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - modelPos.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - modelPos.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    modelRot.y = fn_1_1310(modelRot.y, modelPos.y, 10.0f);
    Hu3DModelPosSet(modelId, pos.x, pos.y, pos.z);
    Hu3DModelRotSet(modelId, 0.0f, modelRot.y, 0.0f);
}

/* The player setup helper resets selection progress, team and difficulty, and assigns default
 * slots. */
void fn_1_2400(void)
{
    MDMIC_PLAYER_WORK *entry;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        entry->characterChoiceState = 0;
        entry->reservedSetup = 0;
        entry->computerDifficulty = 0;
        entry->chrSel = i;
        entry->padNo = i;
    }
}

/* Player setup keeps player one human and fills the requested human slots from connected pads. */
void fn_1_2460(void)
{
    MDMIC_PLAYER_WORK *entry;
    s16 playerCount = lbl_1_bss_9DC.participantCount;
    s16 i;

    lbl_1_bss_9A4[0].isComputer = 0;
    playerCount--;
    for (i = 1, entry = &lbl_1_bss_9A4[1]; i < 4; i++, entry++) {
        if (playerCount > 0 && HuPadStatGet(i) == 0) {
            entry->isComputer = 0;
            playerCount--;
        } else {
            entry->isComputer = 1;
        }
    }
}

/* Camera transition setup can snap the current view directly to its target settings. */
void fn_1_2514(MDMIC_CAMERA_WORK *work)
{
    memcpy(&work->center, &work->targetCenter, sizeof(HuVecF));
    memcpy(&work->rot, &work->targetRot, sizeof(HuVecF));
    work->zoom = work->targetZoom;
}

/* Camera transition setup copies the current view into the targets before further adjustments. */
void fn_1_2564(MDMIC_CAMERA_WORK *work)
{
    memcpy(&work->targetCenter, &work->center, sizeof(HuVecF));
    memcpy(&work->targetRot, &work->rot, sizeof(HuVecF));
    work->targetZoom = work->zoom;
}

/* Camera callbacks ease the current center, rotation and zoom toward their targets each frame. */
void fn_1_25B4(MDMIC_CAMERA_WORK *camera, float weight)
{
    fn_1_1340(&camera->center, &camera->targetCenter, weight);
    fn_1_1340(&camera->rot, &camera->targetRot, weight);
    camera->zoom = fn_1_1310(camera->zoom, camera->targetZoom, weight);
}

void fn_1_2868(MDMIC_CAMERA_CALLBACK callback)
{
    lbl_1_bss_9F4.callback = callback;
}

/* The camera test callback reads controller one to move the view and displays its current
 * settings. */
void fn_1_2878(OMOBJ *obj, MDMIC_CAMERA_WORK *camera)
{
    float stickX = HuPadStkX[0];
    float stickY = -HuPadStkY[0];
    HuVec2f subStick;
    float moveX;
    float moveZ;

    subStick.y = -HuPadSubStkY[0];
    subStick.x = HuPadSubStkX[0];
    camera->targetRot.x += 0.02f * subStick.y;
    if (camera->targetRot.x > 85.0f) {
        camera->targetRot.x = 85.0f;
    }
    if (camera->targetRot.x < -85.0f) {
        camera->targetRot.x = -85.0f;
    }
    camera->targetRot.y += 0.02f * subStick.x;

    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        camera->targetCenter.y -= 0.25f * stickY;
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        camera->targetZoom += 0.25f * stickY;
        if (camera->targetZoom < 5.0f) {
            camera->targetZoom = 5.0f;
        }
    } else {
        moveX = stickX * sind(90.0f + camera->rot.y)
            + stickY * sind(camera->rot.y);
        moveZ = stickX * cosd(90.0f + camera->rot.y)
            + stickY * cosd(camera->rot.y);
        camera->targetCenter.x += 0.25f * moveX;
        camera->targetCenter.z += 0.25f * moveZ;
    }
    fn_1_25B4(camera, 15.0f);

    print8(16, 110, 1.0f, lbl_1_data_DCD);
    print8(16, 120, 1.0f, lbl_1_data_DEF,
        camera->center.x, camera->center.y, camera->center.z);
    print8(16, 130, 1.0f, lbl_1_data_E09,
        camera->rot.x, camera->rot.y, camera->rot.z);
    print8(16, 140, 1.0f, lbl_1_data_E23, camera->zoom);
}

/* The camera dispatcher invokes the current screen callback when one is installed. */
void fn_1_2F88(OMOBJ *obj, MDMIC_CAMERA_WORK *state)
{
    if (state->callback) {
        state->callback(obj, state);
    }
}

/* The camera object runs its screen callback each frame, then applies the resulting view to camera
 * one. */
void fn_1_2FD4(OMOBJ *obj)
{
    MDMIC_CAMERA_WORK *state;

    state = &lbl_1_bss_9F4;
    if (state->callback) {
        state->callback(obj, state);
    }
    Center.x = state->center.x;
    Center.y = state->center.y;
    Center.z = state->center.z;
    CRot.x = state->rot.x;
    CRot.y = state->rot.y;
    CRot.z = state->rot.z;
    CZoom = state->zoom;
    omOutView(obj);
}

/* Scene setup creates the perspective camera and its update object at the initial menu
 * viewpoint. */
void fn_1_30A4(MDMIC_CAMERA_CALLBACK callback)
{
    MDMIC_CAMERA_WORK *camera = &lbl_1_bss_9F4;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(
        1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    memset(camera, 0, sizeof(MDMIC_CAMERA_WORK));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_28 == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj =
        omAddObjEx(lbl_1_bss_0, MDMIC_CAMERA_OBJECT_PRIORITY, 0, 0, -1, fn_1_2FD4);
}

/* Scene teardown disables the menu camera and deletes its update object. */
void fn_1_3240(void)
{
    MDMIC_CAMERA_WORK *state;
    state = &lbl_1_bss_9F4;
    Hu3DCameraKill(1);
    if (state->obj) {
        omDelObjEx(lbl_1_bss_0, state->obj);
    }
    state->obj = NULL;
}

/* Scene setup creates the pair of white global lights used to illuminate the menu models. */
void fn_1_329C(void)
{
    lbl_1_bss_A4C[0] = Hu3DGLightCreate(0.0f, 1.0f, 1.0f,
                                        0.0f, (-1.0f), (-1.0f),
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A4C[0]);
    Hu3DGLightStaticSet(lbl_1_bss_A4C[0], TRUE);
    lbl_1_bss_A4C[1] = Hu3DGLightCreate((-1.0f), 1.0f, (-1.0f),
                                        1.0f, (-1.0f), (-1.0f),
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A4C[1]);
    Hu3DGLightStaticSet(lbl_1_bss_A4C[1], TRUE);
}

/* Scene teardown disables the pair of menu lights. */
void fn_1_33C8(void)
{
    Hu3DGLightKill(lbl_1_bss_A4C[0]);
    Hu3DGLightKill(lbl_1_bss_A4C[1]);
}

/* Message helpers show the guide immediately or animate a dialogue window opening. */
void fn_1_3404(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_A44[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_A44[winNo]);
    }
}

/* Message helpers hide the guide immediately or animate a dialogue window closing. */
void fn_1_3474(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_A44[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_A44[winNo]);
    }
}

void fn_1_34E4(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_A44[winNo]);
}

/* Dialogue prompts wait for a choice; mode one forbids cancel and mode two maps cancel to choice
 * one. */
s16 fn_1_3520(s16 windowNo, s16 mode)
{
    s16 choice = 0;
    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_A44[windowNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_A44[windowNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_A44[windowNo], -1);
    if (mode == 2 && choice == -1) choice = 1;
    return choice;
}

/* Dialogue helpers start centered text and reset the sound-cue cache when the message changes. */
void fn_1_35F4(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_A44[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_A44[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_A44[winNo], speed);
    if (lbl_1_data_C7C != messNum) {
        lbl_1_data_C7C = -1;
    }
}

/* Dialogue helpers clear the current page before setting a message insertion slot. */
void fn_1_36B0(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_A44[winNo]);
    HuWinInsertMesSet(lbl_1_bss_A44[winNo], messNum, insertPos);
}

/* Scene setup creates the guide and three dialogue windows with controller-one input and text
 * cues. */
void fn_1_3720(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_A44[0] = HuWinExCreateFrame(16.0f, 337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_A44[0]);
    HuWinBGTPLvlSet(lbl_1_bss_A44[0], 0.0f);
    lbl_1_bss_A44[1] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_A44[1]);
    HuWinBGTPLvlSet(lbl_1_bss_A44[1], 0.9f);
    lbl_1_bss_A44[2] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_A44[2]);
    HuWinBGTPLvlSet(lbl_1_bss_A44[2], 0.9f);
    lbl_1_bss_A44[3] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_A44[3]);
    HuWinBGTPLvlSet(lbl_1_bss_A44[3], 0.9f);
    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_A44[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_A44[i], fn_1_0);
    }
}

/* Scene teardown destroys its dialogue windows, then releases the shared window resources. */
void fn_1_3948(void)
{
    s16 i;

    i = 0;
    while (i < 4) {
        HuWinExKill(lbl_1_bss_A44[i]);
        i += 1;
    }
    HuWinAllKill();
}

/* Dialogue updates switch the active window and clear cached message IDs when it changes. */
void fn_1_39A4(s16 winNo)
{
    if (lbl_1_data_E32[0] != -1 && lbl_1_data_E32[0] != winNo) {
        fn_1_3474(lbl_1_data_E32[0]);
    }
    if (lbl_1_data_E32[0] == -1 || lbl_1_data_E32[0] != winNo) {
        lbl_1_data_E32[0] = winNo;
        lbl_1_data_E38[0] = -1;
        lbl_1_data_E38[1] = -1;
        fn_1_3404(lbl_1_data_E32[0]);
    }
}

/* Selection flows close the active dialogue window and clear its cached messages. */
void fn_1_3B10(void)
{
    if (lbl_1_data_E32[0] != -1) {
        fn_1_3474(lbl_1_data_E32[0]);
    }
    lbl_1_data_E32[0] = -1;
    lbl_1_data_E38[0] = -1;
    lbl_1_data_E38[1] = -1;
}

/* Dialogue flows wait for the active message to finish before continuing. */
void fn_1_3BCC(void)
{
    if (lbl_1_data_E32[0] != -1) {
        fn_1_34E4(lbl_1_data_E32[0]);
    }
}

/* Dialogue flows read a choice from the active window, returning zero when no window is active. */
s16 fn_1_3C2C(s16 mode)
{
    if (lbl_1_data_E32[0] != -1) {
        return fn_1_3520(lbl_1_data_E32[0], mode);
    }
    return 0;
}

/* Selection flows show a dialogue message only when its ID differs from the cached one. */
void fn_1_3D24(s16 windowNo, s32 messageId, s16 speed)
{
    fn_1_39A4(windowNo);
    if (lbl_1_data_E38[0] != messageId) {
        lbl_1_data_E38[0] = messageId;
        lbl_1_data_E38[1] = -1;
        fn_1_35F4(lbl_1_data_E32[0], lbl_1_data_E38[0], speed);
    }
}

/* Selection flows update an insertion message and invalidate the cached main message. */
void fn_1_3F5C(s16 winNo, s32 messNum, s16 insertPos)
{
    fn_1_39A4(winNo);
    if (lbl_1_data_E38[1] != messNum) {
        lbl_1_data_E38[0] = -1;
        lbl_1_data_E38[1] = messNum;
        fn_1_36B0(lbl_1_data_E32[0], lbl_1_data_E38[1], insertPos);
    }
}

/* Selection flows show the guide window and replace its text only when the guide message
 * changes. */
void fn_1_4150(s32 messNum)
{
    if (lbl_1_data_E32[1] == -1) {
        lbl_1_data_E32[1] = 0;
        lbl_1_data_E38[2] = -1;
        fn_1_3404(lbl_1_data_E32[1]);
    }
    if (lbl_1_data_E38[2] != messNum) {
        lbl_1_data_E38[2] = messNum;
        fn_1_35F4(lbl_1_data_E32[1], lbl_1_data_E38[2], 0);
    }
}

/* Selection flows hide the guide and clear its cached message. */
void fn_1_42D4(void)
{
    if (lbl_1_data_E32[1] != -1) {
        fn_1_3474(lbl_1_data_E32[1]);
    }
    lbl_1_data_E32[1] = -1;
    lbl_1_data_E38[2] = -1;
}

/* Scene setup places the shadow camera above the menu models, looking toward the origin. */
void fn_1_4380(void)
{
    Point3d shadowPosition = {0.0f, 3000.0f, 600.0f};
    Point3d shadowDirection = {0.0f, 1.0f, 0.0f};
    Point3d shadowTarget = {0.0f, 0.0f, 0.0f};

    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowDirection, &shadowTarget);
}

void fn_1_4434(void)
{
}

/* Menu effects set the requested attributes on every sprite in a group. */
void fn_1_4438(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Menu effects clear the requested attributes on every sprite in a group. */
void fn_1_44B8(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

/* Mic Mode setup loads sprite animations and creates the menu's grouped sprites. */
void fn_1_4538(void)
{
    MDMIC_SPRITE_DESC *desc;
    s16 i;

    for (i = 0; i < 32; i++) {
        lbl_1_bss_924[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_24[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 18; i++) {
        lbl_1_bss_900[i] = HuSprGrpCreate(lbl_1_data_A4[i]);
    }
    for (i = 0, desc = lbl_1_data_C8; i < 60; i++, desc++) {
        lbl_1_bss_888[i] = HuSprCreate(
            lbl_1_bss_924[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_900[desc->groupNo], desc->memberNo,
            lbl_1_bss_888[i]);
        HuSprPosSet(lbl_1_bss_900[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_900[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_900[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 18; i++) {
        fn_1_4438(lbl_1_bss_900[i], HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_4784(void)
{
}

/* Scene setup creates the transparent heading window above the game-selection display. */
void fn_1_4788(void)
{
    lbl_1_bss_298 = HuWinExCreateFrame(
        134.0f, 40.0f, 300, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_298);
    HuWinBGTPLvlSet(lbl_1_bss_298, 0.0f);
    HuWinPriSet(lbl_1_bss_298, 0);
}

void fn_1_481C(void) {
    HuWinExKill(lbl_1_bss_298);
}

/* Activity selection formats the saved frame count as time digits and attaches them to its
 * model. */
void fn_1_4848(s32 recordFrames, s16 modelId)
{
    OMOBJ *obj;
    s32 minutes;
    s32 seconds;
    s32 hundredths;

    obj = lbl_1_bss_38;
    if (lbl_1_bss_9DC.modeSelection != 1) {
        return;
    }
    obj->work[0] = modelId;
    obj->work[1] = 1;
    minutes = recordFrames / 3600;
    seconds = (recordFrames % 3600) / 60;
    hundredths = (s32)(1.6666666f *
                    (f32)((recordFrames % 3600) % 60));
    HuSprBankSet(lbl_1_bss_900[17], 2, (s16)(minutes % 10));
    HuSprBankSet(lbl_1_bss_900[17], 4, (s16)(seconds / 10));
    HuSprBankSet(lbl_1_bss_900[17], 5, (s16)(seconds % 10));
    HuSprBankSet(lbl_1_bss_900[17], 7, (s16)(hundredths / 10));
    if (recordFrames == MDMIC_RECORD_MAX_DISPLAY_FRAMES) {
        /* This stored value displays a final hundredths digit of nine. */
        HuSprBankSet(lbl_1_bss_900[17], 8, 9);
    } else {
        HuSprBankSet(lbl_1_bss_900[17], 8, (s16)(hundredths % 10));
    }
}

/* Activity selection requests that the record-time indicator hide on its next update. */
void fn_1_4AC0(void)
{
    OMOBJ *obj = lbl_1_bss_38;

    obj->work[1] = 0;
}

/* The indicator object shows record digits 55 screen units below its model's projected origin
 * or hides the entire sprite group. */
void fn_1_4AE8(OMOBJ *obj)
{
    HUSPR_GROUP *groupToHide;
    HUSPR_GROUPID groupToHideId;
    s16 memberToHide;
    HUSPR_GROUP *groupToShow;
    HUSPR_GROUPID groupToShowId;
    s16 memberToShow;
    HuVecF position;
    HuVecF screenPosition;

    if (obj->work[1] == 0) {
        groupToHideId = lbl_1_bss_900[17];
        groupToHide = &HuSprGrpData[groupToHideId];
        for (memberToHide = 0; memberToHide < groupToHide->sprNum;
             memberToHide++) {
            HuSprAttrSet(groupToHideId, memberToHide, HUSPR_ATTR_DISPOFF);
        }
        return;
    }
    groupToShowId = lbl_1_bss_900[17];
    groupToShow = &HuSprGrpData[groupToShowId];
    for (memberToShow = 0; memberToShow < groupToShow->sprNum;
         memberToShow++) {
        HuSprAttrReset(groupToShowId, memberToShow, HUSPR_ATTR_DISPOFF);
    }
    Hu3DModelPosGet((s16)obj->work[0], &position);
    Hu3D3Dto2D(&position, 1, &screenPosition);
    HuSprGrpPosSet(lbl_1_bss_900[17], screenPosition.x,
                   55.0f + screenPosition.y);
}

/* Scene setup creates the screen-space model indicator, initially hidden. */
void fn_1_4C08(void) {
    OMOBJ *indicatorObj;

    lbl_1_bss_38 = omAddObjEx(lbl_1_bss_0, MDMIC_RECORD_OBJECT_PRIORITY, 0U, 0U, -1, fn_1_4AE8);
    indicatorObj = lbl_1_bss_38;
    indicatorObj->work[1] = 0;
}

void fn_1_4C74(void)
{
}

/* The prop-exit effect eases its horizontal position toward the edge of the screen. */
static inline f32 fn_1_4C78_ease90_mdmicdll_text_4C78c(f32 start, f32 end, f32 elapsed,
                                                       f32 duration)
{
    if (elapsed <= 0.0f) {
        return start;
    }
    if (elapsed >= duration) {
        return end;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((90.0f / duration) * elapsed)) / 180.0));
}

/* The prop-exit effect lifts its vertical position midway through the path, then returns it. */
static inline f32 fn_1_4C78_ease180_mdmicdll_text_4C78c(f32 start, f32 end, f32 elapsed,
                                                        f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return start;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((180.0f / duration) * elapsed)) / 180.0));
}

/* After the host confirmation motion, the effect callback moves its prop sparkle along an arcing
 * exit. */
void fn_1_4C78(OMOBJ *obj)
{
    HuVecF position;
    MDMIC_TRAJECTORY *first;
    MDMIC_TRAJECTORY *second;
    s16 completed;
    OMOBJ *firstObj;
    OMOBJ *secondObj;
    f32 duration;

    completed = 0;
    duration = 60.0f;
    firstObj = lbl_1_bss_C[0];
    /* The second prop is disabled, so this callback cannot reach two completions. */
    secondObj = NULL;
    first = lbl_1_bss_238;
    second = &lbl_1_bss_238[1];
    if (firstObj != NULL) {
        position.x = fn_1_4C78_ease90_mdmicdll_text_4C78c(first->startPosition.x, 500.0f,
            first->elapsed, duration);
        position.y = fn_1_4C78_ease180_mdmicdll_text_4C78c(first->startPosition.y, 400.0f,
            first->elapsed, duration);
        position.z = first->startPosition.z;
        if ((first->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_3A528(0, NULL, 0);
        } else {
            fn_1_3A528(0, &position, 1);
        }
    }
    if (secondObj != NULL) {
        position.x = fn_1_4C78_ease90_mdmicdll_text_4C78c(second->startPosition.x, -500.0f,
            second->elapsed, duration);
        position.y = fn_1_4C78_ease180_mdmicdll_text_4C78c(second->startPosition.y, 400.0f,
            second->elapsed, duration);
        position.z = second->startPosition.z;
        if ((second->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_3A528(1, NULL, 0);
        } else {
            fn_1_3A528(1, &position, 1);
        }
    }
    if (completed >= 2) {
        obj->objFunc = NULL;
    }
}

/* The confirmation effect tracks the host prop for 31 updates before starting its curved exit. */
void fn_1_50C8(OMOBJ *obj)
{
    OMOBJ *obj0 = lbl_1_bss_C[0];
    OMOBJ *obj1 = NULL;
    MDMIC_TRAJECTORY *work0 = &lbl_1_bss_238[0];
    MDMIC_TRAJECTORY *work1 = &lbl_1_bss_238[1];

    if (obj0 != NULL) {
        Hu3DModelObjPosGet(obj0->mdlId[0], lbl_1_data_E44, &work0->startPosition);
        fn_1_3A528(0, &work0->startPosition, 1);
        work0->elapsed = 0.0f;
    }
    if (obj1 != NULL) {
        Hu3DModelObjPosGet(obj1->mdlId[0], lbl_1_data_E44, &work1->startPosition);
        fn_1_3A528(1, &work1->startPosition, 1);
        work1->elapsed = 0.0f;
    }
    if (++obj->work[0] > 30) {
        obj->work[0] = 0;
        obj->objFunc = fn_1_4C78;
    }
}

/* Starts the confirmation motion and sound after a successful setup choice; three result flows call
 * it. */
void fn_1_51CC(void) {
    Hu3DMotionShiftSet(*lbl_1_bss_C[0]->mdlId, lbl_1_bss_C[0]->mtnId[4], 0.0f, 5.0f, 0U);
    lbl_1_bss_3C = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 16U, 16U, -1, fn_1_50C8);
    lbl_1_bss_3C->work[0] = 0;
    HuAudFXPlay(MSM_SE_MENU_16);
}

/* After fn_1_5328 starts the reaction, its object callback restores idle motion. */
void fn_1_5280(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(*obj->mdlId, 2.0f);
    if (obj->work[3]++ > 30U) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(*obj->mdlId, 1.0f);
        Hu3DMotionShiftSet(*obj->mdlId, *obj->mtnId, 0.0f,
                           15.0f, HU3D_MOTATTR_LOOP);
    }
}

/* Dialogue flows start the host reaction and install the callback that returns it to idle. */
void fn_1_5328(void)
{
    OMOBJ *obj = lbl_1_bss_C[0];

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3], 0.0f,
                       10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_5280;
}

/* During game entry, the host callback rises briefly, follows its exit curve and carries a particle
 * trail. */
void fn_1_539C(OMOBJ *obj)
{
    MDMIC_TRAJECTORY *work = &lbl_1_bss_1D8[0];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->elapsed += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->startPosition);
            work->state = 1;
            work->elapsed = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_1FAC(obj->mdlId[0], &work->startPosition, &work->controlPosition,
            &work->endPosition, work->elapsed, work->duration);
        if ((work->elapsed += 1.0f) > work->duration) {
            work->state = 2;
            HuAudFXStop(lbl_1_bss_A50[0]);
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_3CAB8(4, &pos, 1, 5);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

/* Game entry starts the host departure animation, its curved path and its accompanying sound. */
void fn_1_5918(void)
{
    MDMIC_TRAJECTORY *work = &lbl_1_bss_1D8[0];
    OMOBJ *obj = lbl_1_bss_C[0];

    work->state = 0;
    work->elapsed = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->startPosition);
    work->startPosition.x = (-325.0f);
    work->startPosition.y = 0.0f;
    work->startPosition.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->controlPosition);
    work->controlPosition.x = 100.0f;
    work->controlPosition.y = 600.0f;
    work->controlPosition.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->endPosition);
    work->endPosition.x = 100.0f;
    work->endPosition.y = 225.0f;
    work->endPosition.z = (-1000.0f);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[2],
        0.0f, 0.0f, 0);
    lbl_1_bss_A50[0] = HuAudFXPlay(MSM_SE_BRD00_146);
    obj->objFunc = fn_1_539C;
}

/* The host return callback walks back to its menu position, settles its heading and resumes
 * idle. */
void fn_1_5A90(OMOBJ *obj)
{
    HuVecF path[2] = {
        { 0.0f, 0.0f, 0.0f },
        { -325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1A58(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, 15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Selection flows start the host return only when it is not already at the left side of the
 * scene. */
s32 fn_1_6028(void) {
    Point3d position;
    OMOBJ *obj;

    obj = lbl_1_bss_C[0];
    Hu3DModelPosGet(*obj->mdlId, &position);
    if (position.x < (-100.0f)) {
        return 0;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_5A90;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    return 1;
}

/* The object manager calls this to load a character model and its five motions. */
void fn_1_60D4(OMOBJ *obj) {
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    *obj->mdlId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 57), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] =
        Hu3DJointMotion(*obj->mdlId,
            HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 58), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[1] =
        Hu3DJointMotion(*obj->mdlId,
            HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 59), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[2] =
        Hu3DJointMotion(*obj->mdlId,
            HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 60), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[3] =
        Hu3DJointMotion(*obj->mdlId,
            HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 61), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[4] =
        Hu3DJointMotion(*obj->mdlId,
            HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 62), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_28 == 0) {
        Hu3DModelPosSet(*obj->mdlId, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(*obj->mdlId, 0.0f, 0.0f, 0.0f);
    } else {
        Hu3DModelPosSet(*obj->mdlId, (-325.0f), 0.0f, 100.0f);
        Hu3DModelRotSet(*obj->mdlId, 0.0f, 15.0f, 0.0f);
    }
    Hu3DModelScaleSet(*obj->mdlId, 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(*obj->mdlId, 1);
    Hu3DMotionShiftSet(*obj->mdlId, obj->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(*obj->mdlId);
    obj->objFunc = NULL;
}

/* Scene teardown releases the host model and its motions; the object must be valid. */
void fn_1_6364(OMOBJ *obj) {
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DModelKill(*obj->mdlId);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj->objFunc = NULL;
}

/* The character departure callback evaluates its curved path at the current progress. */
static inline void fn_1_1108_inlined_mdmicdll_text_63F8c(Point3d *position, const Point3d *start,
                                                         const Point3d *control, const Point3d *end,
                                                         f32 progress)
{
    position->x = fn_1_10AC_inlined_mdmicdll_text_63F8c(start->x, control->x, end->x, progress);
    position->y = fn_1_10AC_inlined_mdmicdll_text_63F8c(start->y, control->y, end->y, progress);
    position->z = fn_1_10AC_inlined_mdmicdll_text_63F8c(start->z, control->z, end->z, progress);
}

/* The departure path helper evaluates one coordinate of a quadratic curve. */
static inline f32 fn_1_10AC_inlined_mdmicdll_text_63F8c(f32 start, f32 control, f32 end,
                                                        f32 progress)
{
    f32 remaining;

    remaining = 1.0f - progress;
    return (end * (progress * progress)) +
        ((start * (remaining * remaining)) +
         (2.0f * (control * (remaining * progress))));
}

/* After fn_1_69D8 starts departure, move each player along the exit path every frame. */
void fn_1_63F8(OMOBJ *obj)
{
    Point3d effectPosition;
    Point3d rotation;
    Point3d heading;
    Point3d position;
    MDMIC_PLAYER_WORK *player;
    MDMIC_TRAJECTORY *trajectory;
    s16 playerNo;
    s16 playerCount;
    s16 modelId;
    f32 pathProgress;
    f32 progress;
    f32 elapsed;
    f32 duration;

    if (lbl_1_bss_9DC.modeSelection == 0) {
        playerCount = 4;
    } else if (lbl_1_bss_9DC.modeSelection == 1) {
        playerCount = 4;
    } else if (lbl_1_bss_9DC.modeSelection == 2) {
        playerCount = 4;
    }
    playerNo = 0;
    player = &lbl_1_bss_9A4[playerNo];
    while (playerNo < playerCount) {
        trajectory = &lbl_1_bss_118[playerNo];
        if (trajectory->state == 1) {
            duration = trajectory->duration;
            elapsed = trajectory->elapsed;
            modelId = obj->mdlId[player->chrSel];
            progress = fn_1_156C(0.0f, 1.0f, elapsed, duration);
            pathProgress = progress;
            fn_1_1108_inlined_mdmicdll_text_63F8c(&position, &trajectory->startPosition,
                                                  &trajectory->controlPosition,
                                                  &trajectory->endPosition, pathProgress);
            Hu3DModelPosGet(modelId, &heading);
            Hu3DModelRotGet(modelId, &rotation);
            heading.x -= position.x;
            heading.z -= position.z;
            heading.y = (f32) -(180.0 * (atan2((f64) heading.x, -heading.z) / M_PI));
            if ((rotation.y - heading.y) > 180.0f) {
                rotation.y -= 360.0f;
            } else if ((rotation.y - heading.y) < -180.0f) {
                rotation.y += 360.0f;
            }
            rotation.y = fn_1_1310(rotation.y, heading.y, 10.0f);
            Hu3DModelPosSet(modelId, position.x, position.y, position.z);
            Hu3DModelRotSet(modelId, 0.0f, rotation.y, 0.0f);
            if ((trajectory->elapsed += 1.0f) > trajectory->duration) {
                trajectory->state = 2;
                Hu3DModelAttrSet(obj->mdlId[player->chrSel], 1U);
            }
            Hu3DModelPosGet(obj->mdlId[player->chrSel], &effectPosition);
            if (player->isComputer == 0) {
                fn_1_3CAB8(playerNo, &effectPosition, 1, player->padNo);
            } else {
                fn_1_3CAB8(playerNo, &effectPosition, 1, 4);
            }
        }
        playerNo += 1;
        player += 1;
    }
    playerNo = 0;
    while (playerNo < playerCount) {
        trajectory = &lbl_1_bss_118[playerNo];
        if (trajectory->state == 2) {
            playerNo += 1;
            continue;
        }
        break;
    }
    if (playerNo == playerCount) {
        obj->objFunc = NULL;
    }
}

/* Game entry starts player departure curves, excluding computer slots in the first mode. */
void fn_1_69D8(void)
{

    extern OMOBJ *lbl_1_bss_14;

    void fn_1_63F8(OMOBJ *obj);
    OMOBJ *object;
    MDMIC_PLAYER_WORK *player;
    MDMIC_TRAJECTORY *motion;
    s16 index;
    s16 count;
    s16 includeSelected;

    includeSelected = 0;
    object = lbl_1_bss_14;
    if (lbl_1_bss_9DC.modeSelection == 0) {
        count = 4;
        includeSelected = 1;
    } else if (lbl_1_bss_9DC.modeSelection == 1) {
        count = 4;
    } else if (lbl_1_bss_9DC.modeSelection == 2) {
        count = 4;
    }
    for (index = 0, player = &lbl_1_bss_9A4[index]; index < count;
         index++, player++) {
        motion = &lbl_1_bss_118[index];
        if (includeSelected != 1 || player->isComputer != 1) {
            motion->state = 1;
            motion->elapsed = 0.0f;
            motion->duration = 90.0f;
            Hu3DModelPosGet(object->mdlId[player->chrSel], &motion->startPosition);
            Hu3DModelPosGet(object->mdlId[player->chrSel], &motion->controlPosition);
            motion->controlPosition.x += 0.5f * motion->controlPosition.x;
            motion->controlPosition.y = 550.0f;
            motion->controlPosition.z = 100.0f;
            Hu3DModelPosGet(object->mdlId[player->chrSel], &motion->endPosition);
            motion->endPosition.y = 225.0f;
            motion->endPosition.z = (-800.0f);
            Hu3DMotionShiftSet(object->mdlId[player->chrSel],
                               object->mtnId[(player->chrSel * 2) + 1],
                               0.0f, 10.0f,
                               HU3D_MOTATTR_LOOP);
        }
    }
HuAudFXPlay(MSM_SE_MENU_19);
    object->objFunc = fn_1_63F8;
}

/* The character appearance callback scales all four selected models up over its timer. */
void fn_1_6BF8(OMOBJ *obj)
{
    s16 i;
    MDMIC_PLAYER_WORK *desc;
    float scale;

    for (i = 0, desc = &lbl_1_bss_9A4[i]; i < 4; i++, desc++) {
        scale = fn_1_156C(0.0f, 1.0f, obj->work[0], obj->work[1]);
        Hu3DModelScaleSet(
            obj->mdlId[desc->chrSel], scale, scale, scale);
    }
    if ((obj->work[0] += 1.0f) > obj->work[1]) {
        obj->objFunc = NULL;
    }
}

/* Selection transitions reveal the chosen character models with timed particle bursts and a
 * scale-in. */
void fn_1_6DC4(void)
{
    HuVecF position;
    MDMIC_PLAYER_WORK *player;
    OMOBJ *obj;
    s16 playerNo;
    s16 playerCount;
    s16 humanPlayersOnly;

    obj = lbl_1_bss_14;
    humanPlayersOnly = 0;
    if (lbl_1_bss_9DC.modeSelection == 0) {
        playerCount = 4;
        humanPlayersOnly = 1;
    } else if (lbl_1_bss_9DC.modeSelection == 1) {
        playerCount = 4;
    } else if (lbl_1_bss_9DC.modeSelection == 2) {
        playerCount = 4;
    }
    playerNo = 0;
    player = &lbl_1_bss_9A4[playerNo];
    while (playerNo < playerCount) {
        if (humanPlayersOnly != 1 || player->isComputer != 1) {
            Hu3DModelPosGet(lbl_1_bss_29C[player->chrSel].modelId, &position);
            if (player->isComputer != 0) {
                fn_1_3C554(playerNo, &position, 4, 0);
            } else {
                fn_1_3C554(playerNo, &position, player->padNo, 0);
            }
        }
        playerNo++;
        player++;
    }
    playerNo = 0;
    player = &lbl_1_bss_9A4[playerNo];
    while (playerNo < playerCount) {
        if (humanPlayersOnly != 1 || player->isComputer != 1) {
            Hu3DModelPosGet(lbl_1_bss_29C[player->chrSel].modelId, &position);
            position.y -= 70.0f;
            Hu3DModelPosSetV(obj->mdlId[player->chrSel], &position);
            Hu3DModelScaleSet(obj->mdlId[player->chrSel], 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(obj->mdlId[player->chrSel], 1U);
        }
        playerNo++;
        player++;
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_6BF8;
    HuPrcSleep(15);
HuAudFXPlay(MSM_SE_MENU_18);
    playerNo = 0;
    player = &lbl_1_bss_9A4[playerNo];
    while (playerNo < playerCount) {
        if (humanPlayersOnly != 1 || player->isComputer != 1) {
            Hu3DModelPosGet(lbl_1_bss_29C[player->chrSel].modelId, &position);
            if (player->isComputer != 0) {
                fn_1_3C554(playerNo, &position, 4, 1);
            } else {
                fn_1_3C554(playerNo, &position, player->padNo, 1);
            }
        }
        playerNo++;
        player++;
    }
}

/* The object manager calls this to load the Mic Mode model and motion set. */
void fn_1_70B4(OMOBJ *obj) {
    s16 characterNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    characterNo = 0;
    while (characterNo < 11) {
        obj->mdlId[characterNo] =
            Hu3DModelCreate(HuDataSelHeapReadNum(
                characterNo + DATANUM(DATA_mdmic, 69), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[characterNo * 2] = Hu3DJointMotion(
            obj->mdlId[characterNo], HuDataSelHeapReadNum(
                characterNo + DATANUM(DATA_mdmic, 80), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[(characterNo * 2) + 1] = Hu3DJointMotion(
            obj->mdlId[characterNo], HuDataSelHeapReadNum(
                characterNo + DATANUM(DATA_mdmic, 91), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(obj->mdlId[characterNo], 1U);
        Hu3DModelLayerSet(obj->mdlId[characterNo], 1);
        Hu3DMotionShiftSet(obj->mdlId[characterNo], obj->mtnId[characterNo * 2], 0.0f, 0.0f,
                           HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(obj->mdlId[characterNo]);
        characterNo += 1;
    }
    obj->objFunc = NULL;
}

/* Scene teardown releases the eleven character models and their paired motions. */
void fn_1_7254(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        i = 0;
        while (i < 11) {
            Hu3DMotionKill(obj->mtnId[i * 2]);
            Hu3DMotionKill(obj->mtnId[(i * 2) + 1]);
            Hu3DModelKill(obj->mdlId[i]);
            i += 1;
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* The mode-heading callback fades its sprite group out over ten frames, then hides it. */
void fn_1_7304(OMOBJ *obj)
{
    float alpha = fn_1_156C(1.0f, 0.0f, obj->work[0], 10.0f);

    HuSprGrpTPLvlSet(lbl_1_bss_900[0], alpha);
    if (++obj->work[0] > 10) {
        fn_1_4438(lbl_1_bss_900[0], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

/* Initial menu entry positions and reveals the mode heading; returning entry leaves it alone. */
void fn_1_7498(void)
{
    s16 member;
    HUSPR_GROUP *group;
    HUSPR_GROUPID groupId;

    if (lbl_1_bss_28 == 0) {
        HuSprGrpPosSet(lbl_1_bss_900[0], 288.0f, 120.0f);
        groupId = lbl_1_bss_900[0];
        group = &HuSprGrpData[groupId];
        member = 0;
        while (member < group->sprNum) {
            HuSprAttrReset(groupId, member, 4);
            member += 1;
        }
    }
}

/* Selection flows reset the heading timer and install its fade-out callback. */
void fn_1_7554(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->work[0] = 0;
    obj->objFunc = fn_1_7304;
}

/* Object initialization loads the three mode models and reveals the heading on first entry. */
void fn_1_7588(OMOBJ *obj) {
    HUSPR_GROUP *group;
    s16 groupId;
    s16 memberNo;
    s16 modeNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    modeNo = 0;
    while (modeNo < 3) {
        obj->mdlId[modeNo] =
            Hu3DModelCreate(HuDataSelHeapReadNum(
                modeNo + DATANUM(DATA_mdmic, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[modeNo] = Hu3DMotionIDGet(obj->mdlId[modeNo]);
        Hu3DModelLayerSet(obj->mdlId[modeNo], 1);
        Hu3DMotionShiftSet(obj->mdlId[modeNo], obj->mtnId[modeNo], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[modeNo]);
        modeNo += 1;
    }
    if (lbl_1_bss_28 == 0) {
        HuSprGrpPosSet(lbl_1_bss_900[0], 288.0f, 120.0f);
        groupId = lbl_1_bss_900[0];
        group = &HuSprGrpData[groupId];
        memberNo = 0;
        while (memberNo < group->sprNum) {
            HuSprAttrReset(groupId, memberNo, 4);
            memberNo += 1;
        }
    }
    obj->objFunc = NULL;
}

/* Scene teardown releases the three mode models and their motions, then deletes their object. */
void fn_1_7728(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

const MDMIC_MESSAGE_IDS lbl_1_rodata_19C = {
    {MESSNUM(MESS_MG_NAME, 64), MESSNUM(MESS_MG_NAME, 65), MESSNUM(MESS_MG_NAME, 66),
     MESSNUM(MESS_MG_NAME, 68), MESSNUM(MESS_MG_NAME, 69)}};

/* Selection changes show the selected heading and, for minigames, the game name or locked
 * message. */
void fn_1_77B8(s16 memberNo)
{
    OMOBJ *selectionObj = lbl_1_bss_8;
    MDMIC_MESSAGE_IDS messages = lbl_1_rodata_19C;
    HuSprGrpPosSet(lbl_1_bss_900[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_900[1], 0.0f);
    fn_1_4438(lbl_1_bss_900[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_900[1], memberNo, HUSPR_ATTR_DISPOFF);
    selectionObj->work[2] = 1;
    selectionObj->work[3] = 0;
    if (memberNo > 2) {
        lbl_1_bss_40 = 1;
        HuWinAttrSet(lbl_1_bss_298, HUWIN_ATTR_ALIGN_CENTER);
        if (lbl_1_data_CA8[memberNo - 3] == 0) {
    HuWinMesSet(lbl_1_bss_298, MESSNUM(MESS_MGM_FREE, 2));
        } else {
            HuWinMesSet(lbl_1_bss_298, messages.ids[memberNo - 3]);
        }
        HuWinMesSpeedSet(lbl_1_bss_298, 0);
        HuWinDispOn(lbl_1_bss_298);
        HuSprGrpTPLvlSet(lbl_1_bss_900[1], 1.0f);
        selectionObj->work[2] = 0;
        selectionObj->work[3] = 0;
    } else {
        HuWinDispOff(lbl_1_bss_298);
        lbl_1_bss_40 = 0;
    }
}

/* Selection transitions request a heading fade-out and immediately hide the game-name window. */
void fn_1_79E4(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
    HuWinDispOff(lbl_1_bss_298);
}

/* Selection setup positions the heading group and starts it fully transparent. */
void fn_1_7A34(void)
{
    HuSprGrpPosSet(lbl_1_bss_900[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_900[1], 0.0f);
}

/* The heading update callback advances a requested fade, using a shorter duration for game
 * names. */
void fn_1_7A94(OMOBJ *obj)
{
    float duration;
    float alpha;

    duration = 10.0f;
    if (lbl_1_bss_40 == 1) {
        duration = 5.0f;
    }
    switch (obj->work[2]) {
    case 1:
        alpha = fn_1_156C(0.0f, 1.0f, obj->work[3], duration);
        HuSprGrpTPLvlSet(lbl_1_bss_900[1], alpha);
        if (++obj->work[3] > duration) {
            obj->work[2] = 0;
        }
        break;
    case 2:
        alpha = fn_1_156C(1.0f, 0.0f, obj->work[3], duration);
        HuSprGrpTPLvlSet(lbl_1_bss_900[1], alpha);
        if (++obj->work[3] > duration) {
            obj->work[2] = 0;
        }
        break;
    }
}

/* Selection changes replace the second preview model's textures and fade the first preview away. */
void fn_1_7D28(s16 index)
{
    OMOBJ *obj;
    obj = lbl_1_bss_8;
    Hu3DAnimAnimSet(obj->mtnId[2], lbl_1_bss_924[index * 2 + 3]);
    Hu3DAnimAnimSet(obj->mtnId[3], lbl_1_bss_924[index * 2 + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

/* Selection changes replace the second preview model's textures and request an immediate preview
 * swap. */
void fn_1_7DCC(s16 animationNo)
{
    OMOBJ *object = lbl_1_bss_8;

    Hu3DAnimAnimSet(object->mtnId[2], lbl_1_bss_924[animationNo * 2 + 3]);
    Hu3DAnimAnimSet(object->mtnId[3], lbl_1_bss_924[animationNo * 2 + 4]);
    object->work[0] = 3;
    object->work[1] = 0;
}

/* The preview reset helper requests that the selection heading fade out from the start of its
 * timer. */
static inline void fn_1_8950_inlined_07E70_fn_1_7E70c(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

/* Selection transitions restore the first preview while fading the heading and hiding the game
 * name. */
void fn_1_7E70(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    extern s16 lbl_1_bss_298;

    obj->work[0] = 2;
    obj->work[1] = 0;
    fn_1_8950_inlined_07E70_fn_1_7E70c();
    HuWinDispOff(lbl_1_bss_298);
}

/* Selection transitions request that the first preview fade back in. */
void fn_1_7EE4(void)
{
    OMOBJ *object = lbl_1_bss_8;

    object->work[0] = 2;
    object->work[1] = 0;
}

/* The preview object uses this helper each frame to advance the selection heading fade. */
static inline void fn_1_89E0_inlined_inlined_mdmicdll_text_07F14c(OMOBJ *obj)
{
    float alpha;
    float duration;

    duration = 10.0f;
    if (lbl_1_bss_40 == 1) {
        duration = 5.0f;
    }
    switch (obj->work[2]) {
    case 1:
        alpha = fn_1_156C(0.0f, 1.0f, obj->work[3], duration);
        HuSprGrpTPLvlSet(lbl_1_bss_900[1], alpha);
        if ((float)++obj->work[3] > duration) {
            obj->work[2] = 0;
            return;
        }
        return;
    case 2:
        alpha = fn_1_156C(1.0f, 0.0f, obj->work[3], duration);
        HuSprGrpTPLvlSet(lbl_1_bss_900[1], alpha);
        if ((float)++obj->work[3] > duration) {
            obj->work[2] = 0;
        }
        break;
    }
}

/* The preview object rotates both image models each frame and advances preview and heading
 * fades. */
void fn_1_7F14(OMOBJ *obj)
{
    HuVecF rot;
    float modelAlpha;
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelRotGet(obj->mdlId[i], &rot);
        rot.z -= 0.1f;
        if (rot.z < 0.0f) {
            rot.z += 360.0f;
        }
        Hu3DModelRotSetV(obj->mdlId[i], &rot);
    }
    switch (obj->work[0]) {
    case 1:
        modelAlpha = fn_1_156C(1.0f, 0.0f, obj->work[1], 30.0f);
        Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
        if (++obj->work[1] > 30) {
            obj->work[0] = 0;
        }
        break;
    case 2:
        modelAlpha = fn_1_156C(0.0f, 1.0f, obj->work[1], 30.0f);
        Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
        if (++obj->work[1] > 30) {
            obj->work[0] = 0;
        }
        break;
    case 3:
        Hu3DModelTPLvlSet(obj->mdlId[0], 0.0f);
        obj->work[0] = 0;
        break;
    }
    fn_1_89E0_inlined_inlined_mdmicdll_text_07F14c(obj);
}

/* Scene setup loads the paired preview models, attaches their texture animations and starts
 * updates. */
void fn_1_84B0(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdmic, 3), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelPosSet(
            obj->mdlId[i], 0.0f, -100.0f, -850.0f - i);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[i], 0);
        obj->mtnId[2 * i] = Hu3DAnimCreate(
            lbl_1_bss_924[1], obj->mdlId[i], lbl_1_data_E56);
        obj->mtnId[(2 * i) + 1] = Hu3DAnimCreate(
            lbl_1_bss_924[2], obj->mdlId[i], lbl_1_data_E5D);
    }
    HuSprGrpPosSet(lbl_1_bss_900[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_900[1], 0.0f);
    obj->objFunc = fn_1_7F14;
}

/* Scene teardown releases preview texture animations and models before deleting their object. */
void fn_1_86F4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DAnimKill(obj->mtnId[2 * i]);
            Hu3DAnimKill(obj->mtnId[(2 * i) + 1]);
            obj->mtnId[2 * i] = -1;
            obj->mtnId[(2 * i) + 1] = -1;
        }
        for (i = 1; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* The pair-image helper selects an asset for an unordered pair of characters, or the fallback
 * image. */
s32 fn_1_87F0(s16 first, s16 second)
{
    MDRESULT_BYTE_TABLE_110 table = { {
        { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 },
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 },
        { 1, 7 }, { 1, 8 }, { 1, 9 }, { 1, 10 }, { 2, 3 },
        { 2, 4 }, { 2, 5 }, { 2, 6 }, { 2, 7 }, { 2, 8 },
        { 2, 9 }, { 2, 10 }, { 3, 4 }, { 3, 5 }, { 3, 6 },
        { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 }, { 4, 5 },
        { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 },
        { 5, 6 }, { 5, 7 }, { 5, 8 }, { 5, 9 }, { 5, 10 },
        { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 }, { 7, 8 },
        { 7, 9 }, { 7, 10 }, { 8, 9 }, { 8, 10 }, { 9, 10 },
    } };
    s8 i;

    for (i = 0; i < 55; i++) {
        if ((first == table.values[i][0] && second == table.values[i][1])
            || (first == table.values[i][1] && second == table.values[i][0])) {
            break;
        }
    }
    if (i == 55) {
        return DATANUM(DATA_blast5, 55);
    }
    return DATANUM(DATA_blast5, 0) + i;
}

/* Paired-choice prompts place centered text in a heading window with immediate text reveal. */
void fn_1_88F0(s16 winNo, u32 messNum)
{
    HuWinAttrSet(lbl_1_bss_114[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_114[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_114[winNo], 0);
}

/* Paired-choice prompts show both heading windows and the sprites backing their labels. */
void fn_1_897C(u32 messNum1, u32 messNum2)
{
    HuWinDispOn(lbl_1_bss_114[0]);
    HuWinDispOn(lbl_1_bss_114[1]);
    fn_1_88F0(0, messNum1);
    fn_1_88F0(1, messNum2);
    fn_1_44B8(lbl_1_bss_900[13], HUSPR_ATTR_DISPOFF);
}

/* Paired-choice prompts hide both heading windows and their backing sprites when leaving the
 * choice. */
void fn_1_8A9C(void)
{
    HuWinDispOff(lbl_1_bss_114[0]);
    HuWinDispOff(lbl_1_bss_114[1]);
    fn_1_4438(lbl_1_bss_900[13], HUSPR_ATTR_DISPOFF);
}

/* Player-interface setup creates the two transparent headings and aligns their backing sprites. */
void fn_1_8B3C(void)
{
    HuVecF pos[2] = {
        { 150.0f, 120.0f, 0.0f },
        { 426.0f, 120.0f, 0.0f },
    };
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_114[i] = HuWinExCreateFrame(
            pos[i].x - 124.0f, 6.0f + pos[i].y, 240, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_114[i]);
        HuWinBGTPLvlSet(lbl_1_bss_114[i], 0.0f);
        HuWinPriSet(lbl_1_bss_114[i], 0);
        HuSprPosSet(lbl_1_bss_900[13], i, pos[i].x, pos[i].y);
    }
}

/* Player-interface teardown releases the two paired-choice heading windows. */
void fn_1_8CBC(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        HuWinExKill(lbl_1_bss_114[i]);
    }
}

/* Player-selection setup reveals a player icon group with its requested animation bank. */
void fn_1_8D14(s16 playerNo, s16 bank) {
    s16 memberNo;
    HUSPR_GROUP *group;
    HUSPR_GROUPID groupId;

    lbl_1_bss_E4[playerNo].x = 0.85f;
    lbl_1_bss_E4[playerNo].y = 0.85f;
    groupId = lbl_1_bss_900[playerNo + 4];
    group = &HuSprGrpData[groupId];
    memberNo = 0;
    while (memberNo < group->sprNum) {
        HuSprAttrReset(groupId, memberNo, 4);
        memberNo += 1;
    }
    HuSprBankSet(lbl_1_bss_900[playerNo + 4], 0, bank);
}

/* Character selection reveals a player badge and chooses its controller or computer-setting
 * bank. */
void fn_1_8E24(s16 playerNo, s16 bank)
{
    lbl_1_bss_E4[playerNo].x = 0.85f;
    fn_1_44B8(
        lbl_1_bss_900[playerNo + 4], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_900[playerNo + 4], 0, bank);
}

void fn_1_8F10(s16 playerNo)
{
    lbl_1_bss_E4[playerNo].x = 0.0f;
}

void fn_1_8F38(s16 playerNo, s16 bank)
{
    HuSprBankSet(lbl_1_bss_900[playerNo + 4], 0, bank);
}

/* Player-setting updates reveal the right-side badge and choose its animation bank. */
void fn_1_8F84(s16 playerNo, s16 bank)
{
    lbl_1_bss_B4[playerNo].x = 0.85f;
    fn_1_44B8(
        lbl_1_bss_900[playerNo + 8], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_900[playerNo + 8], 2, bank);
}

void fn_1_9070(s16 playerNo)
{
    lbl_1_bss_B4[playerNo].x = 0.0f;
}

void fn_1_9098(s16 playerNo, s16 bank)
{
    HuSprBankSet(lbl_1_bss_900[playerNo + 8], 2, bank);
}

/* Paired-player setting updates reveal the lower badge and choose its animation bank. */
void fn_1_90E4(s16 playerNo, s16 bank)
{
    lbl_1_bss_9C[playerNo].x = 0.85f;
    fn_1_44B8(
        lbl_1_bss_900[playerNo + 14], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_900[playerNo + 14], 2, bank);
}

void fn_1_91D0(s16 playerNo)
{
    ((HuVecF *)lbl_1_bss_9C)[playerNo].x = 0.0f;
}

void fn_1_91F8(s16 playerNo, s16 bank)
{
    HuSprBankSet(lbl_1_bss_900[playerNo + 14], 2, bank);
}

/* Selection updates reveal the shared indicator group at half scale. */
void fn_1_9244(void) {
    HUSPR_GROUP *group;
    s16 groupId;
    s16 memberNo;

    lbl_1_bss_90[0] = 0.5f;
    groupId = lbl_1_bss_900[16];
    group = &HuSprGrpData[groupId];
    memberNo = 0;
    while (memberNo < group->sprNum) {
        HuSprAttrReset(groupId, memberNo, 4);
        memberNo += 1;
    }
}

void fn_1_92DC(void) {
    lbl_1_bss_90[0] = 0.0f;
}

/* The player-interface callback projects character positions to screen and eases each badge
 * scale. */
void fn_1_92F8(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    HuVecF pos3D;
    HuVecF pos2D;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        Hu3DModelPosGet(
            lbl_1_bss_29C[lbl_1_bss_9A4[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_900[i + 4],
            pos2D.x - 18.0f, (s16)(pos2D.y - 23.0f));
        HuSprAttrReset(
            lbl_1_bss_900[i + 4], 0, HUSPR_ATTR_LINEAR);
        lbl_1_bss_E4[i].y = fn_1_1310(
            lbl_1_bss_E4[i].y, lbl_1_bss_E4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_900[i + 4],
            lbl_1_bss_E4[i].y, lbl_1_bss_E4[i].y);

        Hu3DModelPosGet(
            lbl_1_bss_29C[lbl_1_bss_9A4[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_900[i + 8],
            pos2D.x + 18.0f, pos2D.y + 23.0f);
        lbl_1_bss_B4[i].y = fn_1_1310(
            lbl_1_bss_B4[i].y, lbl_1_bss_B4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_900[i + 8],
            lbl_1_bss_B4[i].y, lbl_1_bss_B4[i].y);
    }

    for (i = 0; i < 2; i++) {
        Hu3DModelPosGet(
            lbl_1_bss_29C[lbl_1_bss_9A4[i].chrSel].modelId,
            &pos3D);
        pos3D.x = lbl_1_data_8BC[i + 61].x;
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_900[i + 14],
            pos2D.x, pos2D.y + 23.0f);
        lbl_1_bss_9C[i].y = fn_1_1310(
            lbl_1_bss_9C[i].y, lbl_1_bss_9C[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_900[i + 14],
            lbl_1_bss_9C[i].y, lbl_1_bss_9C[i].y);
    }

    Hu3DModelPosGet(
        lbl_1_bss_29C[lbl_1_bss_9A4[i].chrSel].modelId,
        &pos3D);
    pos3D.x = -120.0f;
    Hu3D3Dto2D(&pos3D, 1, &pos2D);
    HuSprGrpPosSet(lbl_1_bss_900[16], pos2D.x, pos2D.y);
    lbl_1_bss_90[1] = fn_1_1310(
        lbl_1_bss_90[1], lbl_1_bss_90[0], 3.0f);
    HuSprGrpScaleSet(
        lbl_1_bss_900[16], lbl_1_bss_90[1], lbl_1_bss_90[1]);
}

/* Scene setup clears player badge scales, creates paired headings and starts the interface
 * callback. */
void fn_1_9980(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_E4[i].x = lbl_1_bss_E4[i].y = 0.0f;
        lbl_1_bss_B4[i].x = lbl_1_bss_B4[i].y = 0.0f;
    }
    for (i = 0; i < 2; i++) {
        lbl_1_bss_9C[i].x = lbl_1_bss_9C[i].y = 0.0f;
    }
    lbl_1_bss_90[0] = lbl_1_bss_90[1] = 0.0f;
    obj->objFunc = fn_1_92F8;
    fn_1_8B3C();
}

/* Player-interface teardown releases the two paired-choice heading windows. */
void fn_1_9C28(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        HuWinExKill(lbl_1_bss_114[i]);
    }
}

/* Selection updates position a menu sprite at an optional world point plus a screen-space
 * offset. */
void fn_1_9C80(s16 memberNo, Point3d *pos, f32 xOffset, f32 yOffset)
{
    Point3d position = {0.0f, 0.0f, 0.0f};

    if (pos) {
        Hu3D3Dto2D(pos, 1, &position);
    }
    HuSprPosSet(lbl_1_bss_900[2], memberNo, position.x + xOffset,
                position.y + yOffset);
}

/* Selection setup reveals a menu item at double scale and enables its selection sound cue. */
void fn_1_9D18(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset)
{
    fn_1_9C80(memberNo, pos3D, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_900[2], memberNo, 2.0f, 2.0f);
    HuSprAttrReset(lbl_1_bss_900[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_48[memberNo] = 2.0f;
    lbl_1_data_E64[memberNo] = 1;
}

/* Selection transitions hide one menu item and disable its sound cue. */
void fn_1_9E3C(s16 index)
{
    HuSprAttrSet(lbl_1_bss_900[2], index, 4);
    lbl_1_data_E64[index] = 0;
}

/* Selection callbacks enlarge a menu item and play its enabled-item cue. */
void fn_1_9E98(s16 memberNo)
{
    lbl_1_bss_48[memberNo] = 2.0f;
    if (lbl_1_data_E64[memberNo] == 1) {
        HuAudFXPlay(MSM_SE_CMN_01);
    }
}

/* The menu update eases each selection item back to its normal scale after a highlight pulse. */
void fn_1_9F0C(void)
{
    s16 memberNo;

    for (memberNo = 0; memberNo < 6; memberNo++) {
        lbl_1_bss_48[memberNo] = fn_1_1310(lbl_1_bss_48[memberNo], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_900[2], memberNo,
            lbl_1_bss_48[memberNo], lbl_1_bss_48[memberNo]);
    }
}

/* Selection transitions hide all six menu items and disable their selection sound cues. */
void fn_1_A030(void)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        fn_1_9E3C(i);
        lbl_1_data_E64[i] = 0;
    }
}

/* Marker setup saves a target offset from the selected model position. */
void fn_1_A0BC(s16 modelNo, Vec *pos)
{
    lbl_1_bss_60[modelNo].x = pos->x - 50.0f;
    lbl_1_bss_60[modelNo].y = 50.0f + pos->y;
    lbl_1_bss_60[modelNo].z = 50.0f + pos->z;
}

/* Character selection reveals a player marker and badge at the chosen character, then eases it into
 * place. */
void fn_1_A144(s32 modelNo, HuVecF *pos, s16 bank)
{
    OMOBJ *obj = lbl_1_bss_1C[0];

    Hu3DModelPosSet(
        obj->mdlId[(s16)modelNo], pos->x, pos->y, pos->z);
    Hu3DModelRotSet(
        obj->mdlId[(s16)modelNo], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(
        obj->mdlId[(s16)modelNo], 1.0f, 1.0f, 1.0f);
    fn_1_A0BC(modelNo, pos);
    Hu3DModelAttrReset(
        obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_900[3], modelNo, 0.8f, 0.8f);
    HuSprAttrReset(lbl_1_bss_900[3], modelNo, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_900[3], modelNo, bank);
}

/* Character selection hides the player's marker model and its attached badge. */
void fn_1_A318(s32 modelNo)
{
    OMOBJ *obj = lbl_1_bss_1C[0];

    Hu3DModelAttrSet(obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_900[3], modelNo, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_900[3], modelNo, HUSPR_ATTR_DISPOFF);
}

/* The marker callback eases each model toward its target and keeps its badge aligned on screen. */
void fn_1_A3B0(OMOBJ *obj)
{
    HuVecF pos3D;
    HuVecF pos2D;
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos3D);
        fn_1_1340(&pos3D, &lbl_1_bss_60[i], 3.0f);
        Hu3DModelPosSetV(obj->mdlId[i], &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprPosSet(lbl_1_bss_900[3], (s16)i, pos2D.x, pos2D.y);
        HuSprScaleSet(lbl_1_bss_900[3], (s16)i, 0.8f, 0.8f);

        Hu3DModelRotGet(obj->mdlId[i], &pos3D);
        pos3D.z = fn_1_1310(pos3D.z, 0.0f, 3.0f);
        Hu3DModelRotSetV(obj->mdlId[i], &pos3D);

        Hu3DModelScaleGet(obj->mdlId[i], &pos3D);
        pos3D.x = pos3D.y = pos3D.z =
            fn_1_1310(pos3D.x, 0.4f, 3.0f);
        Hu3DModelScaleSetV(obj->mdlId[i], &pos3D);
    }
    fn_1_9F0C();
}

/* Scene setup creates four marker models, hides them and installs their movement callback. */
void fn_1_A870(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdmic, 19), HU_MEMNUM_OVL,
                    HEAP_MODEL));
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[0]);
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        fn_1_A318(i);
    }
    fn_1_A030();
    obj->objFunc = fn_1_A3B0;
}

/* Scene teardown releases the marker motion and models before deleting their object. */
void fn_1_AA68(OMOBJ *obj)
{
    s16 modelIndex;
    if (obj) {
        Hu3DMotionKill(*obj->mtnId);
        modelIndex = 3;
        while (modelIndex >= 0) {
            Hu3DModelKill(obj->mdlId[modelIndex]);
            modelIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Character input searches directional neighbors, skipping entries already selected by others. */
s16 fn_1_AAF0(s16 current, s16 direction, s16 fallbackDirection, s16 lastDirection,
              MDMIC_CHARACTER_CHOICE *entries, s16 count)
{
    s16 next;
    s16 i;

    if (direction == -1) {
        return -1;
    }
    next = entries[current].neighborCharacter[direction];
    if (next == -1) {
        next = fn_1_AAF0(current, fallbackDirection, lastDirection, -1, entries, count);
    }
    i = 0;
    while (i < count) {
        if ((current != i) && (entries[i].selectionState != -1) && (next == i)) {
            next = fn_1_AAF0(next, direction, fallbackDirection, lastDirection, entries, count);
            if (next == -1) {
                next = fn_1_AAF0(current, fallbackDirection, lastDirection, -1, entries, count);
            }
        }
        i += 1;
    }
    return next;
}

/* Character input chooses its primary and fallback directions before searching for a free entry. */
s16 fn_1_AED8(s16 current, MDMIC_CHARACTER_CHOICE *entries,
              s16 direction, s16 count)
{
    MDMinNeighborTable neighbors = { { { 2, 1, 3 },
                                       { 6, 7, 5 },
                                       { 0, 7, 1 },
                                       { 4, 5, 3 },
                                       { 1, 2, 0 },
                                       { 3, 2, 4 },
                                       { 7, 6, 0 },
                                       { 5, 6, 4 },
                                       { 1, 0, 2 },
                                       { 7, 0, 6 },
                                       { 3, 4, 2 },
                                       { 5, 4, 6 } } };
    return fn_1_AAF0(current, neighbors.rows[direction][0],
                     neighbors.rows[direction][1], neighbors.rows[direction][2],
                     entries, count);
}

/* Character-selection callbacks turn repeated pad directions into the next unoccupied character
 * choice. */
s16 fn_1_AF80(s16 padNo, s16 *current,
    MDMIC_CHARACTER_CHOICE *entries, s16 count)
{
    s16 next = 0;
    s16 directionNo = 0;
    HuVecF direction = { 0.0f, 0.0f, 0.0f };

    if (HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
        direction.y = 1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
        direction.y = -1.0f;
    }
    if (HuPadDStkRep[padNo] & PAD_BUTTON_LEFT) {
        direction.x = -1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_RIGHT) {
        direction.x = 1.0f;
    }
    if (HuAbs(direction.x) < 0.25f
        && HuAbs(direction.y) < 0.25f) {
        return FALSE;
    }

    direction.z = sqrtf(
        (direction.x * direction.x) + (direction.y * direction.y));
    direction.x /= direction.z;
    direction.y /= direction.z;
    if (HuAbs(direction.y) < 0.25f) {
        if (direction.x > 0.0f) {
            directionNo = 0;
        } else {
            directionNo = 1;
        }
    } else if (HuAbs(direction.x) < 0.25f) {
        if (direction.y > 0.0f) {
            directionNo = 2;
        } else {
            directionNo = 3;
        }
    } else if (HuAbs(direction.x) > HuAbs(direction.y)) {
        if (direction.x > 0.0f) {
            if (direction.y > 0.0f) {
                directionNo = 4;
            } else {
                directionNo = 5;
            }
        } else if (direction.y > 0.0f) {
            directionNo = 6;
        } else {
            directionNo = 7;
        }
    } else if (direction.y > 0.0f) {
        if (direction.x > 0.0f) {
            directionNo = 8;
        } else {
            directionNo = 9;
        }
    } else if (direction.x > 0.0f) {
        directionNo = 10;
    } else {
        directionNo = 11;
    }

    if ((next = fn_1_AED8(*current, entries, directionNo, count)) != -1) {
        entries[*current].selectionState = -1;
        *current = next;
        entries[*current].selectionState = 1;
        HuAudFXPlay(MSM_SE_CMN_01);
        return TRUE;
    }
    return FALSE;
}

/* Selection updates replace one model texture animation and optionally select its animation
 * bank. */
void fn_1_B500(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];

    Hu3DAnimAnimSet(entry->animId[animNo], lbl_1_bss_814[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animId[animNo], bank);
    }
}

/* Selection updates replace all four texture animations on a menu model with the same image. */
void fn_1_B5A8(s16 modelNo, s16 dataNo)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_B500(modelNo, i, dataNo, -1);
    }
}

/* Scene setup places and reveals a model with the requested horizontal and vertical scale. */
void fn_1_B634(s16 modelNo, Point3d *position, f32 scale)
{
    MDMIC_MODEL_WORK *entry;

    entry = &lbl_1_bss_29C[modelNo];
    Hu3DModelPosSet(entry->modelId, position->x, position->y, position->z);
    Hu3DModelScaleSet(entry->modelId, scale, scale, 1.0f);
    Hu3DModelAttrReset(entry->modelId, 1U);
}

/* Screen entrance callbacks reveal a menu model by spinning it in and growing it to the requested
 * scale. */
void fn_1_B6CC(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosSet(entry->modelId, pos->x, pos->y, pos->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(-540.0f, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(0.0f, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

/* Screen exit callbacks spin a menu model away while shrinking its saved scale to zero. */
void fn_1_BAD8(s16 modelNo, s16 time, s16 duration)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(
        entry->rot.y, entry->rot.y - 540.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (time <= duration) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Screen transitions move and scale a menu model from its saved transform to the requested
 * target. */
void fn_1_BEA8(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    HuVecF modelPos;
    float value;

    if (time == 0) {
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    }
    modelPos.x = fn_1_156C(entry->pos.x, pos->x, time, duration);
    modelPos.y = fn_1_156C(entry->pos.y, pos->y, time, duration);
    modelPos.z = fn_1_156C(entry->pos.z, pos->z, time, duration);
    Hu3DModelPosSet(entry->modelId, modelPos.x, modelPos.y, modelPos.z);
    value = fn_1_156C(entry->scale.x, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

/* Selection callbacks flip a menu card, changing its image once its turn reaches ninety degrees. */
void fn_1_C498(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    HuVecF rot;

    if (time == 0 || time >= duration) {
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    }
    if (time >= duration) {
        fn_1_B5A8(modelNo, dataOffset + dataNo);
    } else {
        Hu3DModelRotGet(entry->modelId, &rot);
        rot.y = fn_1_156C(
            0.0f, 180.0f * direction, time, duration);
        if (((rot.y < 0.0f) ? -rot.y : rot.y) >= 90.0f) {
            fn_1_B5A8(modelNo, dataOffset + dataNo);
        }
        Hu3DModelRotSetV(entry->modelId, &rot);
    }
}

/* Player-count updates flip a card and can change its visibility when the new image turns into
 * view. */
void fn_1_C820(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction, s16 attributeEnabled)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    HuVecF rot;

    if (time == 0 || time >= duration) {
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    }
    if (time >= duration) {
        fn_1_B5A8(modelNo, dataOffset + dataNo);
    } else {
        Hu3DModelRotGet(entry->modelId, &rot);
        rot.y = fn_1_156C(
            0.0f, 180.0f * direction, time, duration);
        if (((rot.y < 0.0f) ? -rot.y : rot.y) >= 90.0f) {
            fn_1_B5A8(modelNo, dataOffset + dataNo);
            if (attributeEnabled == 0) {
                Hu3DModelAttrReset(entry->modelId, 1U);
            } else {
                Hu3DModelAttrSet(entry->modelId, 1U);
            }
        }
        Hu3DModelRotSetV(entry->modelId, &rot);
    }
}

/* Selection transitions straighten a model to zero heading; the angle only chooses the initial
 * wrap. */
void fn_1_CBD4(s16 modelNo, float angle, s16 time, s16 duration)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];
    float rotY;

    if (time == 0) {
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        if (entry->rot.y - angle > 180.0f) {
            entry->rot.y -= 360.0f;
        } else if (entry->rot.y - angle < -180.0f) {
            entry->rot.y += 360.0f;
        }
    }
    rotY = fn_1_156C(entry->rot.y, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, rotY, 0.0f);
}

void fn_1_CE3C(s16 state)
{
    *lbl_1_bss_24->mtnId = state;
}

s16 fn_1_CE54(void)
{
    return *lbl_1_bss_24->mtnId;
}

/* Menu scripts check this input counter before accepting another selection action. */
s16 fn_1_CE6C(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

/* The selection process installs a screen callback and waits for its completion signal. */
void fn_1_CEA4(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

/* Mic Mode setup loads character animations and prepares the selectable models. */
void fn_1_CF3C(void)
{
    MDMIC_BITMAP_NAMES bitmapName = { { lbl_1_data_E70, lbl_1_data_E78, lbl_1_data_E80,
                                        lbl_1_data_E88 } };
    s16 i;
    s16 j;

    for (i = 0; i < 29; i++) {
        lbl_1_bss_814[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_848[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 25; i++) {
        memset(&lbl_1_bss_29C[i], 0, sizeof(MDMIC_MODEL_WORK));
        if (i == 0) {
            lbl_1_bss_29C[i].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdmic, 26), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            lbl_1_bss_29C[i].modelId =
                Hu3DModelLink(lbl_1_bss_29C[0].modelId);
        }
        for (j = 0; j < 4; j++) {
            lbl_1_bss_29C[i].animId[j] = Hu3DAnimCreate(
                lbl_1_bss_814[0], lbl_1_bss_29C[i].modelId,
                bitmapName.name[j]);
        }
        Hu3DModelLayerSet(lbl_1_bss_29C[i].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_29C[i].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 4096, 0, 1, -1, NULL);
}

/* Scene teardown releases the selection models and texture animations, then clears their controller
 * reference. */
void fn_1_D16C(void)
{
    s16 i;
    s16 j;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 24; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_29C[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_29C[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}

/* Player-count input wraps within the connected-pad count and flips cards to show the requested
 * participants. */
void fn_1_D248(OMOBJ *obj)
{
    s16 availablePlayers = 0;
    s16 minimumPlayers;
    s16 playerIndex;

    if (lbl_1_bss_9DC.modeSelection == 2) {
        minimumPlayers = 1;
    } else {
        minimumPlayers = 1;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (HuPadStatGet(playerIndex) == 0) {
            availablePlayers++;
        }
    }
    if (availablePlayers < minimumPlayers) {
        availablePlayers = minimumPlayers;
    }

    if (availablePlayers >= lbl_1_bss_9DC.participantCount) {
        if (obj->work[0]++ > 10) {
            if (HuPadDStkRep[0] & 1) {
                lbl_1_bss_9DC.participantCount--;
                if (lbl_1_bss_9DC.participantCount < minimumPlayers) {
                    lbl_1_bss_9DC.participantCount = availablePlayers;
                }
                obj->work[0] = 0;
                obj->work[1] = 0;
                lbl_1_bss_48[4] = 2.0f;
                if (lbl_1_data_E64[4] == 1) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
            } else if (HuPadDStkRep[0] & 2) {
                lbl_1_bss_9DC.participantCount++;
                if (lbl_1_bss_9DC.participantCount >= availablePlayers + 1) {
                    lbl_1_bss_9DC.participantCount = minimumPlayers;
                }
                obj->work[0] = 0;
                obj->work[1] = 1;
                lbl_1_bss_48[5] = 2.0f;
                if (lbl_1_data_E64[5] == 1) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
            }

            if (availablePlayers == minimumPlayers) {
                obj->work[0] = 12;
                if (obj->work[3] == 1) {
                    obj->work[3] = 0;
                    HuSprAttrSet(lbl_1_bss_900[2], 4, HUSPR_ATTR_DISPOFF);
                    lbl_1_data_E64[4] = 0;
                    HuSprAttrSet(lbl_1_bss_900[2], 5, HUSPR_ATTR_DISPOFF);
                    lbl_1_data_E64[5] = 0;
                }
            } else if (obj->work[3] == 0) {
                obj->work[3] = 1;
                fn_1_9D18(4, lbl_1_data_8BC, -180.0f, 0.0f);
                fn_1_9D18(5, lbl_1_data_8BC, 180.0f, 0.0f);
            }
        }
    } else {
        lbl_1_bss_9DC.participantCount = availablePlayers;
        obj->work[0] = 0;
        obj->work[1] = 0;
    }

    if (lbl_1_bss_9DC.modeSelection == 2) {
        if (obj->work[0] <= 10) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (obj->work[1] != 0) {
                    if (playerIndex < lbl_1_bss_9DC.participantCount) {
                        fn_1_C498(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                                  MDMIC_PLAYER_INACTIVE_IMAGE, 1, obj->work[0], 10, 1);
                    } else {
                        fn_1_C498(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                                  MDMIC_PLAYER_INACTIVE_IMAGE, 0, obj->work[0], 10, 1);
                    }
                } else if (playerIndex < lbl_1_bss_9DC.participantCount) {
                    fn_1_C498(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                              MDMIC_PLAYER_INACTIVE_IMAGE, 1, obj->work[0], 10, -1);
                } else {
                    fn_1_C498(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                              MDMIC_PLAYER_INACTIVE_IMAGE, 0, obj->work[0], 10, -1);
                }
            }
        }
    } else if (obj->work[0] <= 10) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (obj->work[1] != 0) {
                if (playerIndex < lbl_1_bss_9DC.participantCount) {
                    fn_1_C820(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                    MDMIC_PLAYER_INACTIVE_IMAGE, 1, obj->work[0], 10, 1, 0);
                } else {
                    fn_1_C820(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                    MDMIC_PLAYER_INACTIVE_IMAGE, 0, obj->work[0], 10, 1, 1);
                }
            } else if (playerIndex < lbl_1_bss_9DC.participantCount) {
                fn_1_C820(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                    MDMIC_PLAYER_INACTIVE_IMAGE, 1, obj->work[0], 10, -1, 0);
            } else {
                fn_1_C820(playerIndex + MDMIC_PLAYER_COUNT_MODEL_BASE,
                    MDMIC_PLAYER_INACTIVE_IMAGE, 0, obj->work[0], 10, -1, 1);
            }
        }
    }
}

/* Player-count setup enables its input callback and signals that the screen is ready. */
void fn_1_EE40(OMOBJ *obj)
{
    obj->work[0] = 10;
    obj->objFunc = fn_1_D248;
    *lbl_1_bss_24->mtnId = 1;
}

/* Leaving player-count input hides both adjustment arrows and signals that the screen is
 * stopped. */
void fn_1_EE70(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_900[2], 4, 4);
    lbl_1_data_E64[4] = 0;
    HuSprAttrSet(lbl_1_bss_900[2], 5, 4);
    lbl_1_data_E64[5] = 0;
    obj->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* Player-count entrance counts connected pads and spins in cards showing active and inactive
 * slots. */
void fn_1_EF08(OMOBJ *obj)
{
    f32 value;
    s16 playerIndex;
    s16 activeMotionIndex;
    s16 inactiveMotionIndex;
    s16 activeAnimationNo;
    s16 inactiveAnimationNo;
    s16 time;
    MDMIC_MODEL_WORK *entry;
    MDMIC_MODEL_WORK *activeEntry;
    MDMIC_MODEL_WORK *inactiveEntry;
    s16 activePlayers;

    if (lbl_1_bss_9DC.modeSelection == 2) {
        activePlayers = 1;
    } else {
        activePlayers = 1;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        time = obj->work[0];
        entry = &lbl_1_bss_29C[(s16)(playerIndex + 17)];

        if (time == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosSet(entry->modelId,
                lbl_1_data_8BC[playerIndex + MDMIC_PLAYER_COUNT_POSITION_BASE].x,
                lbl_1_data_8BC[playerIndex + MDMIC_PLAYER_COUNT_POSITION_BASE].y,
                lbl_1_data_8BC[playerIndex + MDMIC_PLAYER_COUNT_POSITION_BASE].z);
            Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_42 = 0;
        }

        value = fn_1_156C(-540.0f, 0.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);

        value = fn_1_156C(0.0f, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }

    if (obj->work[0] == 0) {
        playerIndex = 0;
        lbl_1_bss_9DC.participantCount = 0;
        while (playerIndex < 4) {
            if (HuPadStatGet(playerIndex) == 0) {
                lbl_1_bss_9DC.participantCount++;
            }
            playerIndex++;
        }
        if (lbl_1_bss_9DC.participantCount < activePlayers) {
            lbl_1_bss_9DC.participantCount = activePlayers;
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (playerIndex < lbl_1_bss_9DC.participantCount) {
                for (activeMotionIndex = 0; activeMotionIndex < 4; activeMotionIndex++) {
                    activeAnimationNo = activeMotionIndex;
                    fn_1_B500(playerIndex + 17, activeAnimationNo, MDMIC_PLAYER_ACTIVE_IMAGE, -1);
                }
            } else {
                for (inactiveMotionIndex = 0; inactiveMotionIndex < 4; inactiveMotionIndex++) {
                    inactiveAnimationNo = inactiveMotionIndex;
                    fn_1_B500(playerIndex + 17, inactiveAnimationNo, MDMIC_PLAYER_INACTIVE_IMAGE,
                              -1);
                }
                if (lbl_1_bss_9DC.modeSelection == 0) {
                    Hu3DModelAttrSet(lbl_1_bss_29C[(s16) playerIndex + 17].modelId,
                                     HU3D_ATTR_DISPOFF);
                }
            }
        }
    }

    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Player-count exit spins and shrinks its four cards, preserving hidden inactive slots in the first
 * mode. */
void fn_1_F4D8(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *entry;
    float value;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_29C[(s16)(i + 17)];
        if (time == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_42 = 0;
        }
        value = fn_1_156C(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
        if (i >= lbl_1_bss_9DC.participantCount) {
            if (lbl_1_bss_9DC.modeSelection == 0) {
                Hu3DModelAttrSet(lbl_1_bss_29C[i + 17].modelId, HU3D_ATTR_DISPOFF);
            }
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Character-selection input rotates a currently browsed character model by six degrees per
 * update. */
void fn_1_F8DC(s16 modelNo)
{
    HuVecF rot;
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];

    Hu3DModelRotGet(entry->modelId, &rot);
    rot.y += 6.0f;
    if (rot.y > 360.0f) {
        rot.y -= 360.0f;
    }
    Hu3DModelRotSetV(entry->modelId, &rot);
}

/* Character-selection input restores an unhighlighted character model to its forward-facing
 * rotation. */
void fn_1_F978(s16 modelNo)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo];

    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
}

/* Human character selection reads each player's pad to browse, confirm or cancel, respecting the
 * unlock. */
void fn_1_F9E0(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 0) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[i]++ > 10) {
                    if (fn_1_AF80(entry->padNo, &entry->chrSel,
                            lbl_1_data_E90, 11)) {
                        fn_1_A0BC(
                            i, &lbl_1_data_8BC[entry->chrSel + 14]);
                        obj->work[i] = 0;
                    } else if (HuPadBtnDown[entry->padNo]
                        & PAD_BUTTON_A) {
                        if (lbl_1_bss_2C == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            continue;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        fn_1_A318(i);
                        fn_1_8E24(i, entry->padNo);
                        fn_1_B500(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_B500(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_3C258(i,
                            &lbl_1_data_8BC[entry->chrSel + 14],
                            entry->padNo);
                    }
                }
            } else if (HuPadBtnDown[entry->padNo] & PAD_BUTTON_B) {
                entry->characterChoiceState = 0;
                fn_1_A144(i, &lbl_1_data_8BC[entry->chrSel + 14],
                    entry->padNo);
                fn_1_8F10(i);
                fn_1_B500((s16)entry->chrSel, 0,
                    (s16)entry->chrSel, 0);
                fn_1_B500((s16)entry->chrSel, 2,
                    (s16)entry->chrSel, 0);
            }
        }
    }
    for (i = 0; i < 11; i++) {
        for (j = 0, entry = lbl_1_bss_9A4; j < 4;
             j++, entry++) {
            if (entry->isComputer == 0 && entry->chrSel == i
                && entry->characterChoiceState == 0) {
                break;
            }
        }
        if (j == 4) {
            fn_1_F978(i);
        } else {
            fn_1_F8DC(i);
        }
    }
}

/* Human character-selection setup reserves the unconfirmed choices and enables their player
 * markers. */
void fn_1_1018C(OMOBJ *obj)
{
    s16 i;
    MDMIC_PLAYER_WORK *entry;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 0 && entry->characterChoiceState == 0) {
            lbl_1_data_E90[entry->chrSel].selectionState = 1;
            fn_1_A144(
                i, &lbl_1_data_8BC[entry->chrSel + 14], entry->padNo);
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_F9E0;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving human character input hides unconfirmed markers, straightens the characters and stops
 * updates. */
void fn_1_103F0(OMOBJ *obj)
{
    s16 i;
    MDMIC_PLAYER_WORK *entry;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 0 && entry->characterChoiceState == 0) {
            fn_1_A318(i);
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_F978(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Backing out of player one's character confirmation clears its badge and restores its unselected
 * images. */
void fn_1_1053C(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry = &lbl_1_bss_9A4[0];

    entry->characterChoiceState = 0;
    lbl_1_bss_E4[0].x = 0.0f;
    fn_1_B500((s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
    fn_1_B500((s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Computer setup uses controller one to choose each computer character and difficulty, with
 * backtracking. */
void fn_1_1064C(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState != 2) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_AF80(
                            0, &entry->chrSel, lbl_1_data_E90, 11)) {
                        fn_1_A0BC(
                            i, &lbl_1_data_8BC[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2C == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            break;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        entry->computerDifficulty = 1;
                        fn_1_A318(i);
                        fn_1_8E24(i, entry->computerDifficulty + 5);
                        fn_1_B500(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_B500(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        Hu3DModelPosGet(
                            lbl_1_bss_29C[entry->chrSel].modelId, &pos);
                        fn_1_9D18(4, &pos, -40.0f, -45.0f);
                        fn_1_9D18(5, &pos, 40.0f, -45.0f);
                        fn_1_3C258(i,
                            &lbl_1_data_8BC[entry->chrSel + 14], 4);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        lbl_1_data_E90[entry->chrSel].selectionState = -1;
                        fn_1_A318(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9A4[j].isComputer == 1
                                && lbl_1_bss_9A4[j].characterChoiceState == 2) {
                                lbl_1_bss_9A4[j].characterChoiceState = 0;
                                lbl_1_data_E90[
                                    lbl_1_bss_9A4[j].chrSel]
                                    .selectionState = -1;
                                fn_1_8F10(j);
                                fn_1_B500((s16)lbl_1_bss_9A4[j].chrSel, 0,
                                    (s16)lbl_1_bss_9A4[j].chrSel, 0);
                                fn_1_B500((s16)lbl_1_bss_9A4[j].chrSel, 2,
                                    (s16)lbl_1_bss_9A4[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_11534;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            } else if (entry->characterChoiceState == 1) {
                if (obj->work[0]++ > 10) {
                    if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                        entry->computerDifficulty--;
                        if (lbl_1_bss_2E == 0) {
                            if (entry->computerDifficulty < 0) {
                                entry->computerDifficulty += 3;
                            }
                        } else {
                            if (entry->computerDifficulty < 0) {
                                entry->computerDifficulty += 4;
                            }
                        }
                        obj->work[0] = 0;
                        fn_1_9E98(4);
                    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                        entry->computerDifficulty++;
                        if (lbl_1_bss_2E == 0) {
                            if (entry->computerDifficulty > 2) {
                                entry->computerDifficulty -= 3;
                            }
                        } else if (entry->computerDifficulty > 3) {
                            entry->computerDifficulty -= 4;
                        }
                        obj->work[0] = 0;
                        fn_1_9E98(5);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        HuAudFXPlay(MSM_SE_CMN_02);
                        entry->characterChoiceState = 2;
                        fn_1_9E3C(4);
                        fn_1_9E3C(5);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9A4[j].isComputer == 1
                                && lbl_1_bss_9A4[j].characterChoiceState == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_11534;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        entry->characterChoiceState = 0;
                        fn_1_9E3C(4);
                        fn_1_9E3C(5);
                        fn_1_B500(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
                        fn_1_B500(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
                        fn_1_A144(
                            i, &lbl_1_data_8BC[entry->chrSel + 14], 4);
                        fn_1_8F10(i);
                    }
                }
                fn_1_8F38(i, entry->computerDifficulty + 5);
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->characterChoiceState == 0) {
            fn_1_F8DC(i);
        } else {
            fn_1_F978(i);
        }
    }
}

/* Computer setup reserves a free character or restores the current difficulty arrows, then enables
 * input. */
void fn_1_11534(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1) {
            if (entry->characterChoiceState == 0) {
                for (j = 0; j < 11; j++) {
                    if (lbl_1_data_E90[j].selectionState == -1) {
                        entry->chrSel = j;
                        lbl_1_data_E90[j].selectionState = 1;
                        break;
                    }
                }
                fn_1_A144(
                    i, &lbl_1_data_8BC[entry->chrSel + 14], 4);
                break;
            } else if (entry->characterChoiceState == 1) {
                Hu3DModelPosGet(
                    lbl_1_bss_29C[entry->chrSel].modelId, &pos);
                fn_1_9D18(4, &pos, -40.0f, -45.0f);
                fn_1_9D18(5, &pos, 40.0f, -45.0f);
                break;
            }
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_1064C;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving computer setup releases an unfinished character or hides difficulty arrows, then stops
 * updates. */
void fn_1_119B8(OMOBJ *obj)
{
    s16 i;
    MDMIC_PLAYER_WORK *entry;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1) {
            if (entry->characterChoiceState == 0) {
                lbl_1_data_E90[entry->chrSel].selectionState = -1;
                fn_1_A318(i);
                break;
            } else if (entry->characterChoiceState == 1) {
                fn_1_9E3C(4);
                fn_1_9E3C(5);
                break;
            }
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_F978(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Computer character input confirms one character at a time with controller one and can revisit the
 * prior slot. */
void fn_1_11B84(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState != 1) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_AF80(
                            0, &entry->chrSel, lbl_1_data_E90, 11)) {
                        fn_1_A0BC(
                            i, &lbl_1_data_8BC[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2C == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            break;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        entry->computerDifficulty = 1;
                        fn_1_A318(i);
                        fn_1_8E24(i, 4);
                        fn_1_B500(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_B500(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_3C258(i,
                            &lbl_1_data_8BC[entry->chrSel + 14], 4);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9A4[j].isComputer == 1
                                && lbl_1_bss_9A4[j].characterChoiceState == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_12334;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        lbl_1_data_E90[entry->chrSel].selectionState = -1;
                        fn_1_A318(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9A4[j].isComputer == 1
                                && lbl_1_bss_9A4[j].characterChoiceState == 1) {
                                lbl_1_bss_9A4[j].characterChoiceState = 0;
                                lbl_1_data_E90[
                                    lbl_1_bss_9A4[j].chrSel]
                                    .selectionState = -1;
                                fn_1_8F10(j);
                                fn_1_B500((s16)lbl_1_bss_9A4[j].chrSel, 0,
                                    (s16)lbl_1_bss_9A4[j].chrSel, 0);
                                fn_1_B500((s16)lbl_1_bss_9A4[j].chrSel, 2,
                                    (s16)lbl_1_bss_9A4[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_12334;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->characterChoiceState == 0) {
            fn_1_F8DC(i);
        } else {
            fn_1_F978(i);
        }
    }
}

/* Computer character-selection setup reserves the first free character for the next unfinished
 * slot. */
void fn_1_12334(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState == 0) {
            for (j = 0; j < 11; j++) {
                if (lbl_1_data_E90[j].selectionState == -1) {
                    entry->chrSel = j;
                    lbl_1_data_E90[j].selectionState = 1;
                    break;
                }
            }
            fn_1_A144(
                i, &lbl_1_data_8BC[entry->chrSel + 14], 4);
            break;
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_11B84;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving computer character input releases its unfinished choice and hides its marker before
 * stopping. */
void fn_1_125D8(OMOBJ *obj)
{
    s16 i;
    MDMIC_PLAYER_WORK *entry;

    for (i = 0, entry = lbl_1_bss_9A4; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState == 0) {
            lbl_1_data_E90[entry->chrSel].selectionState = -1;
            fn_1_A318(i);
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_F978(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Character-screen entrance resets player choices and spins in the eleven character cards. */
void fn_1_12744(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    OMOBJ *workObj;
    float value;
    s16 i;
    s16 j;
    s16 time;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        for (i = 0; i < 11; i++) {
            fn_1_B5A8(i, i);
            fn_1_B500(i, 1, i, 2);
            fn_1_B500(i, 3, i, 2);
            lbl_1_data_E90[i].selectionState = -1;
        }
        if (lbl_1_bss_2C == 0) {
            fn_1_B5A8(10, 11);
        }
        for (j = 0, player = lbl_1_bss_9A4; j < 4;
             j++, player++) {
            player->characterChoiceState = 0;
            player->reservedSetup = 0;
            player->computerDifficulty = 0;
            player->chrSel = j;
            player->padNo = j;
        }
    }
    for (i = 0; i < 11; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_29C[i];
        if (time == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosSet(entry->modelId,
                lbl_1_data_8BC[i + 14].x,
                lbl_1_data_8BC[i + 14].y,
                lbl_1_data_8BC[i + 14].z);
            Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_42 = 0;
        }
        value = fn_1_156C(-540.0f, 0.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(0.0f, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Character-screen transitions move one chosen character into its player lineup position over ten
 * frames. */
static inline void fn_1_14E70_move_player_mdmicdll_text_12744c(s16 playerNo, s16 animationFrame)
{
    MDMIC_MODEL_WORK *characterModel;
    s32 characterNo;
    HuVecF targetPosition;
    float targetScale;

    characterNo = (s16)lbl_1_bss_9A4[playerNo].chrSel;
    characterModel = &lbl_1_bss_29C[characterNo];
    if (animationFrame == 0) {
        Hu3DModelPosGet(characterModel->modelId, &characterModel->pos);
        Hu3DModelRotGet(characterModel->modelId, &characterModel->rot);
        Hu3DModelScaleGet(characterModel->modelId, &characterModel->scale);
    }
    targetPosition.x = fn_1_156C(
        characterModel->pos.x, lbl_1_data_8BC[playerNo + 25].x,
        animationFrame, 10.0f);
    targetPosition.y = fn_1_156C(
        characterModel->pos.y, lbl_1_data_8BC[playerNo + 25].y,
        animationFrame, 10.0f);
    targetPosition.z = fn_1_156C(
        characterModel->pos.z, lbl_1_data_8BC[playerNo + 25].z,
        animationFrame, 10.0f);
    Hu3DModelPosSet(characterModel->modelId,
        targetPosition.x, targetPosition.y, targetPosition.z);
    targetScale = fn_1_156C(
        characterModel->scale.x, 1.25f, animationFrame, 10.0f);
    Hu3DModelScaleSet(characterModel->modelId, targetScale, targetScale, 1.0f);
}

/* After character choice, this transition dismisses unused cards and moves the chosen four into a
 * lineup. */
void fn_1_12D38(OMOBJ *obj)
{
        MDMIC_MESSAGE_IDS messageIds;
    s16 characterNo;
    s16 playerNo;
    s16 frame;
    s16 boardNo;
    OMOBJ * windowObject;
    s16 animationFrame;

    for (characterNo = 0; characterNo < 11; characterNo++)
    {
        for (playerNo = 0; playerNo < 4; playerNo++)
        {
            if (characterNo == lbl_1_bss_9A4[playerNo].chrSel)
            {
                break;
            }
        }
        if (playerNo == 4)
        {
            frame = obj->work[0];
            fn_1_BAD8(characterNo, frame, 10);
        }
    }
    if (obj->work[0] >= 10)
    {
        for (characterNo = 0; characterNo < 4; characterNo++)
        {
            animationFrame = obj->work[0] - 10;
            fn_1_14E70_move_player_mdmicdll_text_12744c(characterNo, animationFrame);
        }
    }
    if (obj->work[0]++ > 20)
    {
        if (lbl_1_bss_9DC.modeSelection != 2)
        {
            fn_1_77B8(lbl_1_bss_9DC.modeSelection);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After human character choice, dismiss unused cards and arrange two, three or four human
 * characters. */
void fn_1_13870(OMOBJ *obj)
{
    s16 humanCount;
    s16 playerNo;
    s16 matchNo;
    MDMIC_MODEL_WORK *model;
    MDMIC_MODEL_WORK *modelTwo;
    MDMIC_MODEL_WORK *modelThree;
    MDMIC_MODEL_WORK *modelFour;
    HuVecF position;
    float targetRotation;
    float targetScale;
    float scaleTwo;
    float scaleThree;
    float scaleFour;
    humanCount = 0;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_9A4[playerNo].isComputer == 0) {
            humanCount++;
        }
    }
    for (playerNo = 0; playerNo < 11; playerNo++) {
        for (matchNo = 0; matchNo < 4; matchNo++) {
            if (lbl_1_bss_9A4[matchNo].isComputer == 0 &&
                playerNo == lbl_1_bss_9A4[matchNo].chrSel) {
                break;
            }
        }
        if (matchNo == 4) {
            fn_1_BAD8(playerNo, (s16)obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        matchNo = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_9A4[playerNo].isComputer != 1) {
                switch (humanCount) {
                case 2:
                    position.x = lbl_1_data_8BC[matchNo + 46].x;
                    position.y = 125.0f;
                    position.z = lbl_1_data_8BC[matchNo + 46].z;
                    fn_1_BEA8((s16)lbl_1_bss_9A4[playerNo].chrSel, (s16)(obj->work[0] - 10),
                        10, &position, 1.25f);
                    matchNo++;
                    break;
                case 3:
                    position.x = lbl_1_data_8BC[matchNo + 77].x;
                    position.y = 125.0f;
                    position.z = lbl_1_data_8BC[matchNo + 77].z;
                    fn_1_BEA8((s16)lbl_1_bss_9A4[playerNo].chrSel, (s16)(obj->work[0] - 10),
                        10, &position, 1.25f);
                    matchNo++;
                    break;
                case 4:
                    position.x = lbl_1_data_8BC[matchNo + 45].x;
                    position.y = 125.0f;
                    position.z = lbl_1_data_8BC[matchNo + 45].z;
                    fn_1_BEA8((s16)lbl_1_bss_9A4[playerNo].chrSel, (s16)(obj->work[0] - 10),
                        10, &position, 1.25f);
                    matchNo++;
                    break;
                }
            }
        }
    }
    if (obj->work[0]++ > 20) {
        fn_1_77B8(lbl_1_bss_9DC.modeSelection);
        obj->objFunc = NULL;
        fn_1_CE3C(1);
    }
}

/* The lineup transition dismisses unused cards, moves the chosen characters and restores the mode
 * heading. */
void fn_1_14E70(OMOBJ *obj)
{
        s16 characterNo;
    s16 playerNo;
    MDMIC_MESSAGE_IDS messageIds;
    s16 frame;
    s16 boardNo;
    s16 animationFrame;

    for (characterNo = 0; characterNo < 11; characterNo++) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (characterNo == lbl_1_bss_9A4[playerNo].chrSel) {
                break;
            }
        }
        if (playerNo == 4) {
            frame = obj->work[0];
            fn_1_BAD8(characterNo, frame, 10);
        }
    }
    if (obj->work[0] >= 10) {
        OMOBJ *windowObject;

        fn_1_77B8(lbl_1_bss_9DC.modeSelection);
        for (characterNo = 0; characterNo < 4; characterNo++) {
            animationFrame = obj->work[0] - 10;
            fn_1_14E70_move_player_mdmicdll_text_12744c(characterNo, animationFrame);
        }
    }
    if (obj->work[0]++ > 20) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Leaving the player lineup shrinks its chosen character cards and clears the player badges. */
void fn_1_15994(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_E4[i].x = 0.0f;
        }
    }
    for (i = 0, player = lbl_1_bss_9A4; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_29C[modelNo];
        if (time == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_42 = 0;
        }
        value = fn_1_156C(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Returning from character selection dismisses all character cards and restores the appropriate
 * mode heading. */
void fn_1_15D8C(OMOBJ *obj)
{
    OMOBJ *workObj;
    HUSPR_GROUP *spriteGroup;
    MDMIC_MODEL_WORK *entry;
    MDMIC_MESSAGE_IDS messages;
    float value;
    s16 mode;
    s16 groupId;
    s16 memberNo;
    s16 i;
    s16 frame;

    if (obj->work[0] == 0) {
        if (lbl_1_bss_9DC.modeSelection != 1) {
            mode = lbl_1_bss_9DC.modeSelection;
            workObj = lbl_1_bss_8;
            messages = lbl_1_rodata_19C;
            HuSprGrpPosSet(lbl_1_bss_900[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_900[1], 0.0f);
            groupId = lbl_1_bss_900[1];
            spriteGroup = &HuSprGrpData[groupId];
            for (memberNo = 0; memberNo < spriteGroup->sprNum; memberNo++) {
                HuSprAttrSet(groupId, memberNo, 4);
            }
            HuSprAttrReset(lbl_1_bss_900[1], mode, 4);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            if (mode > 2) {
                lbl_1_bss_40 = 1;
                HuWinAttrSet(lbl_1_bss_298, HUWIN_ATTR_ALIGN_CENTER);
                if (lbl_1_data_CA8[mode - 3] == 0) {
                    HuWinMesSet(lbl_1_bss_298, MESSNUM(MESS_MGM_FREE, 2));
                } else {
                    HuWinMesSet(lbl_1_bss_298, messages.ids[mode - 3]);
                }
                HuWinMesSpeedSet(lbl_1_bss_298, 0);
                HuWinDispOn(lbl_1_bss_298);
                HuSprGrpTPLvlSet(lbl_1_bss_900[1], 1.0f);
                workObj->work[2] = 0;
                workObj->work[3] = 0;
            } else {
                HuWinDispOff(lbl_1_bss_298);
                lbl_1_bss_40 = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            lbl_1_bss_E4[i].x = 0.0f;
        }
    }

    for (i = 0; i < 11; i++) {
        frame = (s16)obj->work[0];
        entry = &lbl_1_bss_29C[i];
        if (frame == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_42 = 0;
        }
        value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }

    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Mode input enlarges the selected item, brings it forward and starts its slow spin after a short
 * delay. */
void fn_1_16390(s16 modelNo)
{
    Vec value;
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    fn_1_1340(&value, &lbl_1_data_8BC[modelNo + 7], 5.0f);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->highlightSpinFrame++ > 30) {
        entry->highlightSpinFrame = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += 3.0f;
        if (value.y >= 360.0f) {
            value.y -= 360.0f;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x =
        fn_1_1310(value.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}

/* Mode-selection updates restore a mode model's position, scale and spin timer. */
void fn_1_166B0(s16 modeNo)
{
    MDMIC_MODEL_WORK *entry = &lbl_1_bss_29C[modeNo + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_8BC[modeNo + 1]);
    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(entry->modelId, 1.25f, 1.25f, 1.0f);
    entry->highlightSpinFrame = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

/* Mode-selection input wraps left or right through three choices and updates the highlighted item
 * each frame. */
void fn_1_16784(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            s16 memberNo;

            lbl_1_bss_9DC.modeSelection--;
            if (lbl_1_bss_9DC.modeSelection < 0) {
                lbl_1_bss_9DC.modeSelection += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            memberNo = lbl_1_bss_9DC.modeSelection;
            fn_1_77B8(memberNo);
            lbl_1_bss_48[0] = 2.0f;
            if (lbl_1_data_E64[0] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            s16 memberNo;

            lbl_1_bss_9DC.modeSelection++;
            if (lbl_1_bss_9DC.modeSelection >= 3) {
                lbl_1_bss_9DC.modeSelection -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            memberNo = lbl_1_bss_9DC.modeSelection;
            fn_1_77B8(memberNo);
            lbl_1_bss_48[1] = 2.0f;
            if (lbl_1_data_E64[1] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        }
    }

    for (i = 0; i < 3; i++) {
        if (i == lbl_1_bss_9DC.modeSelection) {
            fn_1_16390(i);
        } else {
            fn_1_166B0(i);
        }
    }
}

/* Mode-selection setup resets item spin timers, reveals both arrows and enables the input
 * callback. */
void fn_1_170D4(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 3; i++) {
        lbl_1_bss_29C[i + 11].highlightSpinFrame = 0;
    }
    fn_1_9D18(0, &lbl_1_data_8BC[0],
        (-200.0f), 0.0f);
    fn_1_9D18(1, &lbl_1_data_8BC[0],
        200.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_16784;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving mode input hides both arrows and straightens the three mode models before signaling
 * completion. */
void fn_1_17340(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0] == 0) {
        fn_1_9E3C(0);
        fn_1_9E3C(1);
    }
    for (i = 0; i < 3; i++) {
        lbl_1_bss_29C[i + 11].highlightSpinFrame = 0;
        fn_1_CBD4(i + 11, 0.0f, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Mode-screen entrance resets the choice to the first mode and spins in all three mode cards. */
void fn_1_17648(OMOBJ *obj)
{
    extern s16 lbl_1_bss_40;

    s16 index;
    s16 frame;
    f32 rotation;
    f32 scale;

    lbl_1_bss_9DC.modeSelection = 0;
    if (obj->work[0] == 0) {
        for (index = 0; index < 3; index++) {
            fn_1_B5A8(index + 11, index + 12);
            fn_1_B500(index + 11, 1, index + 12, 1);
            fn_1_B500(index + 11, 3, index + 12, 1);
        }
    }

    for (index = 0; index < 3; index++) {
        frame = (s16)obj->work[0];
        fn_1_B6CC(index + 11, frame, 10, &lbl_1_data_8BC[index + 1], 1.25f);
    }

    if (obj->work[0]++ > 10U) {
        Hu3DZClearLayerSet(2);
        fn_1_77B8(lbl_1_bss_9DC.modeSelection);
        obj->objFunc = NULL;
        fn_1_CE3C(1);
    }
}

/* This transition restores the first preview and hides the heading except in the activity mode. */
void fn_1_17D90(OMOBJ *obj)
{
    OMOBJ *firstWorkObj;
    OMOBJ *fallbackWorkObj;
    OMOBJ *secondWorkObj;

    if (obj->work[0] == 0) {
        if (lbl_1_bss_9DC.modeSelection != 1) {
            firstWorkObj = lbl_1_bss_8;
            firstWorkObj->work[0] = 2;
            firstWorkObj->work[1] = 0;
            secondWorkObj = lbl_1_bss_8;
            secondWorkObj->work[2] = 2;
            secondWorkObj->work[3] = 0;
            HuWinDispOff(lbl_1_bss_298);
        } else {
            fallbackWorkObj = lbl_1_bss_8;
            fallbackWorkObj->work[0] = 2;
            fallbackWorkObj->work[1] = 0;
        }
    }
    if (obj->work[0]++ > 30U) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After mode confirmation, dismiss other models and move the chosen mode into its transition. */
void fn_1_17E8C(OMOBJ *obj)
{
    Point3d rotation;
    Point3d position;
    MDMIC_MODEL_WORK *otherMode;
    MDMIC_MODEL_WORK *selectedMode;
    s16 selectedFrame;
    s16 dismissFrame;
    OMOBJ *selectionObj;
    s16 modeNo;
    s16 selectedModeNo;
    s16 selectedModelNo;
    f32 dismissValue;
    f32 selectedScale;

    modeNo = 0;
    while (modeNo < 3) {
        if (modeNo != lbl_1_bss_9DC.modeSelection) {
            dismissFrame = (s16)obj->work[0];
            otherMode = &lbl_1_bss_29C[(s16)(modeNo + 11)];
            if (dismissFrame == 0) {
                if (lbl_1_bss_42 == 0) {
HuAudFXPlay(MSM_SE_CMN_54);
                    lbl_1_bss_42 = 1;
                }
                Hu3DModelPosGet(otherMode->modelId,
                                &otherMode->pos);
                Hu3DModelRotGet(otherMode->modelId,
                                &otherMode->rot);
                Hu3DModelScaleGet(otherMode->modelId,
                                  &otherMode->scale);
            } else {
                lbl_1_bss_42 = 0;
            }
            dismissValue = fn_1_156C(otherMode->rot.y,
                otherMode->rot.y - 540.0f, (f32)dismissFrame, 10.0f);
            Hu3DModelRotSet(otherMode->modelId, 0.0f, dismissValue,
                            0.0f);
            dismissValue = fn_1_156C(otherMode->scale.x,
                0.0f, (f32)dismissFrame, 10.0f);
            Hu3DModelScaleSet(otherMode->modelId, dismissValue, dismissValue,
                              1.0f);
            if (dismissFrame <= 10) {
                Hu3DModelAttrReset(otherMode->modelId, 1U);
            }
        }
        modeNo += 1;
    }

    if ((u32)obj->work[0] >= 10U) {
        selectedFrame = obj->work[0] - 10;
        selectedModelNo = lbl_1_bss_9DC.modeSelection + 11;
        selectedMode = &lbl_1_bss_29C[selectedModelNo];
        if (selectedFrame == 0) {
            Hu3DModelPosGet(selectedMode->modelId,
                            &selectedMode->pos);
            Hu3DModelRotGet(selectedMode->modelId,
                            &selectedMode->rot);
            Hu3DModelScaleGet(selectedMode->modelId,
                              &selectedMode->scale);
        }
        position.x = fn_1_156C(selectedMode->pos.x,
            lbl_1_data_8BC[13].x, (f32)selectedFrame, 10.0f);
        position.y = fn_1_156C(selectedMode->pos.y,
            lbl_1_data_8BC[13].y, (f32)selectedFrame, 10.0f);
        position.z = fn_1_156C(selectedMode->pos.z,
            lbl_1_data_8BC[13].z, (f32)selectedFrame, 10.0f);
        Hu3DModelPosSet(selectedMode->modelId, position.x, position.y, position.z);
        selectedScale = fn_1_156C(selectedMode->scale.x,
            1.5f, (f32)selectedFrame, 10.0f);
        Hu3DModelScaleSet(selectedMode->modelId, selectedScale, selectedScale, 1.0f);
        Hu3DModelRotGet(lbl_1_bss_29C[lbl_1_bss_9DC.modeSelection + 11].modelId,
                        &rotation);
        rotation.y += 54.0f;
        Hu3DModelRotSetV(lbl_1_bss_29C[lbl_1_bss_9DC.modeSelection + 11].modelId,
                         &rotation);
    }

    if ((u32)obj->work[0] == 20U) {
        Hu3DLayerHookReset(2);
        fn_1_3C060(&lbl_1_data_8BC[13]);
HuAudFXPlay(MSM_SE_MENU_15);
        selectedModeNo = lbl_1_bss_9DC.modeSelection;
        selectionObj = lbl_1_bss_8;
        Hu3DAnimAnimSet(selectionObj->mtnId[2], lbl_1_bss_924[(selectedModeNo * 2) + 3]);
        Hu3DAnimAnimSet(selectionObj->mtnId[3], lbl_1_bss_924[(selectedModeNo * 2) + 4]);
        selectionObj->work[0] = 1;
        selectionObj->work[1] = 0;
        Hu3DModelAttrSet(lbl_1_bss_29C[lbl_1_bss_9DC.modeSelection + 11].modelId, 1U);
    }

    if (obj->work[0]++ > 90U) {
        obj->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Leaving the mode screen fades its heading and spins the three mode cards down to zero scale. */
void fn_1_18874(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *entry;
    s16 i;
    OMOBJ *selectionObj;
    float modelValue;
    s16 frame;

    if (obj->work[0] == 0) {
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
    }
    for (i = 0; i < 3; i++) {
        frame = obj->work[0];
        entry = &lbl_1_bss_29C[(s16)(i + 11)];
        if (frame == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_42 = 0;
        }
        modelValue = fn_1_156C(
            entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
        modelValue = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handicap input selects a player horizontally; up lowers and down raises its zero-to-nine
 * value. */
void fn_1_18C58(OMOBJ *obj)
{
    HuVecF characterPosition;
    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9DC.handicapPlayer--;
            if (lbl_1_bss_9DC.handicapPlayer < 0) {
                lbl_1_bss_9DC.handicapPlayer = lbl_1_bss_9DC.participantCount - 1;
            }
            obj->work[0] = 0;
            fn_1_9E98(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9DC.handicapPlayer++;
            if (lbl_1_bss_9DC.handicapPlayer >= lbl_1_bss_9DC.participantCount) {
                lbl_1_bss_9DC.handicapPlayer = 0;
            }
            obj->work[0] = 0;
            fn_1_9E98(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap--;
            if (lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap < 0) {
                lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap += 10;
            }
            obj->work[0] = 0;
            fn_1_9E98(2);
            fn_1_9098(lbl_1_bss_9DC.handicapPlayer,
                lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap++;
            if (lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap > 9) {
                lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap -= 10;
            }
            obj->work[0] = 0;
            fn_1_9E98(3);
            fn_1_9098(lbl_1_bss_9DC.handicapPlayer,
                lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].handicap);
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_29C[lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].chrSel].modelId,
        &characterPosition);
    fn_1_9C80(2, &characterPosition, 0.0f, -80.0f);
    fn_1_9C80(3, &characterPosition, 0.0f, 80.0f);
}

/* Handicap input setup reveals value arrows around the chosen player and horizontal player-switch
 * arrows. */
void fn_1_19134(OMOBJ *obj)
{
    HuVecF characterPosition;

    Hu3DModelPosGet(
        lbl_1_bss_29C[lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].chrSel].modelId,
        &characterPosition);

    fn_1_9D18(2, &characterPosition, 0.0f, -80.0f);
    fn_1_9D18(3, &characterPosition, 0.0f, 80.0f);
    characterPosition.x = 0.0f;

    switch (lbl_1_bss_9DC.participantCount) {
    case 2:
        fn_1_9D18(4, &characterPosition, -100.0f, 0.0f);
        fn_1_9D18(5, &characterPosition, 100.0f, 0.0f);
        break;
    case 3:
        fn_1_9D18(4, &characterPosition, -140.0f, 0.0f);
        fn_1_9D18(5, &characterPosition, 140.0f, 0.0f);
        break;
    case 4:
        fn_1_9D18(4, &characterPosition, -180.0f, 0.0f);
        fn_1_9D18(5, &characterPosition, 180.0f, 0.0f);
        break;
    }

    obj->work[0] = 10;
    obj->objFunc = fn_1_18C58;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving handicap input hides all four adjustment arrows and stops its callback. */
void fn_1_198CC(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_900[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_E64[2] = 0;
    HuSprAttrSet(lbl_1_bss_900[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_E64[3] = 0;
    HuSprAttrSet(lbl_1_bss_900[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_E64[4] = 0;
    HuSprAttrSet(lbl_1_bss_900[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_E64[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Handicap setup selects player one, resets participant handicaps to zero and reveals their number
 * badges. */
void fn_1_199B4(OMOBJ *obj)
{
    lbl_1_bss_9DC.handicapPlayer = 0;
    if (obj->work[0]++ > 10) {
        s16 i;

        for (i = 0; i < lbl_1_bss_9DC.participantCount; i++) {
            s16 bank;
            HUSPR_GROUP *spriteGroup;
            HUSPR_GROUPID groupId;
            s16 memberNo;

            lbl_1_bss_9A4[i].handicap = 0;
            bank = lbl_1_bss_9A4[i].handicap;
            lbl_1_bss_B4[i].x = 0.85f;
            groupId = lbl_1_bss_900[i + 8];
            spriteGroup = &HuSprGrpData[groupId];
            for (memberNo = 0; memberNo < spriteGroup->sprNum; memberNo++) {
                HuSprAttrReset(groupId, memberNo, 4);
            }
            HuSprBankSet(lbl_1_bss_900[i + 8], 2, bank);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handicap transitions use this helper to move and resize a character over ten frames. */
static inline void fn_1_1A1D8_move_player_mdmicdll_text_1A1D8c(
    MDMIC_MODEL_WORK *entry, const HuVecF *targetPosition, s16 time)
{
    HuVecF pos;
    float value;

    pos.x = fn_1_156C(entry->pos.x, targetPosition->x, time, 10.0f);
    pos.y = fn_1_156C(entry->pos.y, targetPosition->y, time, 10.0f);
    pos.z = fn_1_156C(entry->pos.z, targetPosition->z, time, 10.0f);
    Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
    value = fn_1_156C(entry->scale.x, 1.25f, time, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

/* Handicap-screen entrance moves the participating characters into the raised lineup. */
void fn_1_19B28(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    HuVecF targetPosition;
    s16 i;
    s16 time;
    s32 modelNo;

    lbl_1_bss_9DC.handicapPlayer = 0;
    for (i = 0, player = lbl_1_bss_9A4; i < lbl_1_bss_9DC.participantCount; i++, player++) {
        switch (lbl_1_bss_9DC.participantCount) {
        case 2:
            targetPosition.x = lbl_1_data_8BC[i + 46].x;
            targetPosition.y = 175.0f;
            targetPosition.z = lbl_1_data_8BC[i + 46].z;
            break;
        case 3:
            targetPosition.x = lbl_1_data_8BC[i + 77].x;
            targetPosition.y = 175.0f;
            targetPosition.z = lbl_1_data_8BC[i + 77].z;
            break;
        case 4:
            targetPosition.x = lbl_1_data_8BC[i + 45].x;
            targetPosition.y = 175.0f;
            targetPosition.z = lbl_1_data_8BC[i + 45].z;
            break;
        }
        time = obj->work[0];
        modelNo = (s16)lbl_1_bss_9A4[i].chrSel;
        entry = &lbl_1_bss_29C[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        fn_1_1A1D8_move_player_mdmicdll_text_1A1D8c(entry, &targetPosition, time);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Leaving the raised handicap layout moves participating characters back to the lower lineup. */
void fn_1_1A1D8(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    HuVecF targetPosition;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0, player = lbl_1_bss_9A4; i < lbl_1_bss_9DC.participantCount; i++, player++) {
        switch (lbl_1_bss_9DC.participantCount) {
        case 2:
            targetPosition.x = lbl_1_data_8BC[i + 46].x;
            targetPosition.y = 125.0f;
            targetPosition.z = lbl_1_data_8BC[i + 46].z;
            break;
        case 3:
            targetPosition.x = lbl_1_data_8BC[i + 77].x;
            targetPosition.y = 125.0f;
            targetPosition.z = lbl_1_data_8BC[i + 77].z;
            break;
        case 4:
            targetPosition.x = lbl_1_data_8BC[i + 45].x;
            targetPosition.y = 125.0f;
            targetPosition.z = lbl_1_data_8BC[i + 45].z;
            break;
        }
        time = obj->work[0];
        modelNo = (s16)lbl_1_bss_9A4[i].chrSel;
        entry = &lbl_1_bss_29C[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        fn_1_1A1D8_move_player_mdmicdll_text_1A1D8c(entry, &targetPosition, time);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Leaving handicap settings requests that all participant value badges shrink away. */
void fn_1_1A878(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0] == 0) {
        for (i = 0; i < lbl_1_bss_9DC.participantCount; i++) {
            lbl_1_bss_B4[i].x = 0.0f;
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Game confirmation moves the selected game card to the center and enlarges it over ten frames. */
void fn_1_1A914(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *gameCard;
    HuVecF targetPosition;
    float targetScale;
    s16 gameCardIndex;
    s16 animationFrame;

    animationFrame = obj->work[0];
    gameCardIndex = lbl_1_bss_9DC.gameSelection + 11;
    gameCard = &lbl_1_bss_29C[gameCardIndex];
    if (animationFrame == 0) {
        Hu3DModelPosGet(gameCard->modelId, &gameCard->pos);
        Hu3DModelRotGet(gameCard->modelId, &gameCard->rot);
        Hu3DModelScaleGet(gameCard->modelId, &gameCard->scale);
    }
    targetPosition.x = fn_1_156C(gameCard->pos.x,
        lbl_1_data_8BC[0].x, animationFrame, 10.0f);
    targetPosition.y = fn_1_156C(gameCard->pos.y,
        lbl_1_data_8BC[0].y, animationFrame, 10.0f);
    targetPosition.z = fn_1_156C(gameCard->pos.z,
        lbl_1_data_8BC[0].z, animationFrame, 10.0f);
    Hu3DModelPosSet(gameCard->modelId, targetPosition.x, targetPosition.y,
        targetPosition.z);
    targetScale = fn_1_156C(gameCard->scale.x, 2.25f, animationFrame, 10.0f);
    Hu3DModelScaleSet(gameCard->modelId, targetScale, targetScale, 1.0f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* The game confirmation exit uses this helper to spin its card away while shrinking it to zero. */
static inline void MdmicExitTransition_mdmicdll_text_1A914c(MDMIC_MODEL_WORK *entry, s16 frame)
{
    float value;

    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* The game confirmation exit uses this helper to spin and shrink one player's character card. */
static inline void MdmicPlayerExitTransition_mdmicdll_text_1A914c(MDMIC_PLAYER_WORK *player,
                                                                  s16 frame)
{
    MDMIC_MODEL_WORK *entry;
    float value;
    s32 modelNo;

    modelNo = (s16)player->chrSel;
    entry = &lbl_1_bss_29C[modelNo];
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Game confirmation exit shrinks its game and character cards, then starts the animated character
 * appearance. */
void fn_1_1AE7C(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *primaryModel;
    MDMIC_PLAYER_WORK *player;
    OMOBJ *selectionObj;
    s16 index;
    s16 primaryFrame;
    s16 playerFrame;
    s16 primaryModelNo;

    if (obj->work[0] == 0) {
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        for (index = 0; index < 4; index++) {
            lbl_1_bss_E4[index].x = 0.0f;
        }
        lbl_1_bss_90[0] = 0.0f;
    }

    primaryFrame = (s16)obj->work[0];
    primaryModelNo = lbl_1_bss_9DC.gameSelection + 11;
    primaryModel = &lbl_1_bss_29C[primaryModelNo];
    MdmicExitTransition_mdmicdll_text_1A914c(primaryModel, primaryFrame);

    for (index = 0, player = lbl_1_bss_9A4; index < 4; index++, player++) {
        playerFrame = (s16)obj->work[0];
        MdmicPlayerExitTransition_mdmicdll_text_1A914c(player, playerFrame);
    }

    if (obj->work[0]++ > 10U) {
        fn_1_6DC4();
        obj->objFunc = NULL;
        ((OMOBJ *)lbl_1_bss_24)->mtnId[0] = 1;
    }
}

/* The preview-return transition uses this helper to spin and shrink its selected game card. */
static inline void MdmicModelTransition_mdmicdll_text_1B5F0c(MDMIC_MODEL_WORK *entry, s16 frame)
{
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    {
        float value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                                       (float)frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* The preview-return transition uses this helper to spin and shrink one player's character card. */
static inline void MdmicPlayerTransition_mdmicdll_text_1B5F0c(MDMIC_PLAYER_WORK *player, s16 frame)
{
    MDMIC_MODEL_WORK *entry;
    float value;
    s16 modelNo;

    modelNo = (s16)player->chrSel;
    entry = &lbl_1_bss_29C[modelNo];
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                             (float)frame, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* Returning to the preview clears badges, fades the heading and shrinks the game and character
 * cards. */
void fn_1_1B5F0(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *primaryModel;
    MDMIC_PLAYER_WORK *player;
    OMOBJ *firstObj;
    OMOBJ *selectionObj;
    s16 index;
    s16 primaryFrame;
    s16 playerFrame;
    s16 primaryModelNo;

    if (obj->work[0] == 0) {
        firstObj = lbl_1_bss_8;
        firstObj->work[0] = 2;
        firstObj->work[1] = 0;
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        for (index = 0; index < 4; index++) {
            lbl_1_bss_E4[index].x = 0.0f;
        }
        lbl_1_bss_90[0] = 0.0f;
    }

    primaryFrame = (s16)obj->work[0];
    primaryModelNo = lbl_1_bss_9DC.gameSelection + 11;
    primaryModel = &lbl_1_bss_29C[primaryModelNo];
    MdmicModelTransition_mdmicdll_text_1B5F0c(primaryModel, primaryFrame);

    for (index = 0, player = lbl_1_bss_9A4; index < 4; index++, player++) {
        playerFrame = (s16)obj->work[0];
        MdmicPlayerTransition_mdmicdll_text_1B5F0c(player, playerFrame);
    }

    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        ((OMOBJ *)lbl_1_bss_24)->mtnId[0] = 1;
    }
}

/* This selection transition moves the current game card to its smaller secondary display
 * position. */
void fn_1_1BD7C(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *gameCard;
    HuVecF targetPosition;
    float targetScale;
    s32 gameCardIndex;
    s16 animationFrame;

    animationFrame = obj->work[0];
    gameCardIndex = (s16)(lbl_1_bss_9DC.gameSelection + 11);
    gameCard = &lbl_1_bss_29C[gameCardIndex];
    if (animationFrame == 0) {
        Hu3DModelPosGet(gameCard->modelId, &gameCard->pos);
        Hu3DModelRotGet(gameCard->modelId, &gameCard->rot);
        Hu3DModelScaleGet(gameCard->modelId, &gameCard->scale);
    }
    targetPosition.x = fn_1_156C(gameCard->pos.x,
        lbl_1_data_8BC[37].x, animationFrame, 10.0f);
    targetPosition.y = fn_1_156C(gameCard->pos.y,
        lbl_1_data_8BC[37].y, animationFrame, 10.0f);
    targetPosition.z = fn_1_156C(gameCard->pos.z,
        lbl_1_data_8BC[37].z, animationFrame, 10.0f);
    Hu3DModelPosSet(gameCard->modelId, targetPosition.x, targetPosition.y,
        targetPosition.z);
    targetScale = fn_1_156C(gameCard->scale.x, 1.5f, animationFrame, 10.0f);
    Hu3DModelScaleSet(gameCard->modelId, targetScale, targetScale, 1.0f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* The first mode's confirmation process uses this callback to mark its layout ready immediately. */
void fn_1_1C2E4(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* This screen transition shrinks human character cards, clears their badges and starts
 * character-model appearance. */
void fn_1_1C308(OMOBJ *obj)
{
    OMOBJ *workObj;
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    float modelValue;
    s32 characterModelIndex;
    s16 i;
    s16 frame;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        for (i = 0, player = lbl_1_bss_9A4; i < 4; i++, player++) {
            if (player->isComputer == 0) {
                lbl_1_bss_E4[i].x = 0.0f;
                lbl_1_bss_B4[i].x = 0.0f;
            }
        }
    }
    for (i = 0, player = lbl_1_bss_9A4; i < 4; i++, player++) {
        if (player->isComputer == 0) {
            frame = obj->work[0];
            characterModelIndex = (s16)player->chrSel;
            entry = &lbl_1_bss_29C[characterModelIndex];
            if (frame == 0) {
                if (lbl_1_bss_42 == 0) {
                    HuAudFXPlay(MSM_SE_CMN_54);
                    lbl_1_bss_42 = 1;
                }
                Hu3DModelPosGet(entry->modelId, &entry->pos);
                Hu3DModelRotGet(entry->modelId, &entry->rot);
                Hu3DModelScaleGet(entry->modelId, &entry->scale);
            } else {
                lbl_1_bss_42 = 0;
            }
            modelValue = fn_1_156C(
                entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
            Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
            modelValue = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
            Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
            if (frame <= 10) {
                Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
            }
        }
    }
    if (obj->work[0]++ > 10) {
        fn_1_6DC4();
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* This screen exit restores the preview while shrinking human character cards and clearing their
 * badges. */
void fn_1_1C778(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    float modelValue;
    s32 characterModelIndex;
    s16 i;
    s16 frame;
    OMOBJ *workObj;
    OMOBJ *secondWorkObj;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        secondWorkObj = lbl_1_bss_8;
        secondWorkObj->work[2] = 2;
        secondWorkObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        for (i = 0, player = lbl_1_bss_9A4; i < 4; i++, player++) {
            if (player->isComputer == 0) {
                lbl_1_bss_E4[i].x = 0.0f;
                lbl_1_bss_B4[i].x = 0.0f;
            }
        }
    }
    for (i = 0, player = lbl_1_bss_9A4; i < 4; i++, player++) {
        if (player->isComputer == 0) {
            frame = obj->work[0];
            characterModelIndex = (s16)player->chrSel;
            entry = &lbl_1_bss_29C[characterModelIndex];
            if (frame == 0) {
                if (lbl_1_bss_42 == 0) {
                    HuAudFXPlay(MSM_SE_CMN_54);
                    lbl_1_bss_42 = 1;
                }
                Hu3DModelPosGet(entry->modelId, &entry->pos);
                Hu3DModelRotGet(entry->modelId, &entry->rot);
                Hu3DModelScaleGet(entry->modelId, &entry->scale);
            } else {
                lbl_1_bss_42 = 0;
            }
            modelValue = fn_1_156C(
                entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
            Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
            modelValue = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
            Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
            if (frame <= 10) {
                Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
            }
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Confirmation and player-order transitions use this callback to signal completion and stop
 * updates. */
void fn_1_1CC00(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* The activity confirmation process uses this callback to mark its layout ready immediately. */
void fn_1_1CC24(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* The activity confirmation exit uses this helper to spin and shrink its activity card. */
static inline void MdmicModelTransition_mdmicdll_text_1CC48c(MDMIC_MODEL_WORK *entry, s16 frame)
{
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    {
        float value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                                       (float)frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* The activity confirmation exit uses this helper to spin and shrink one player's character
 * card. */
static inline void MdmicPlayerTransition_mdmicdll_text_1CC48c(MDMIC_PLAYER_WORK *player, s16 frame)
{
    MDMIC_MODEL_WORK *entry;
    float value;
    s32 modelNo;

    modelNo = (s16)player->chrSel;
    entry = &lbl_1_bss_29C[modelNo];
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                             (float)frame, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* Activity confirmation hides the record display, shrinks the cards and starts character-model
 * appearance. */
void fn_1_1CC48(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *primaryModel;
    MDMIC_PLAYER_WORK *player;
    OMOBJ *selectionObj;
    OMOBJ *targetObj;
    s16 index;
    s16 primaryFrame;
    s16 playerFrame;
    s16 primaryModelNo;

    if (obj->work[0] == 0) {
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        targetObj = lbl_1_bss_38;
        targetObj->work[1] = 0;
        for (index = 0; index < 4; index++) {
            lbl_1_bss_E4[index].x = 0.0f;
        }
    }

    primaryFrame = (s16)obj->work[0];
    primaryModelNo = lbl_1_bss_9DC.gameSelection + 11;
    primaryModel = &lbl_1_bss_29C[primaryModelNo];
    MdmicModelTransition_mdmicdll_text_1CC48c(primaryModel, primaryFrame);

    for (index = 0, player = lbl_1_bss_9A4; index < 4; index++, player++) {
        playerFrame = (s16)obj->work[0];
        MdmicPlayerTransition_mdmicdll_text_1CC48c(player, playerFrame);
    }

    if (obj->work[0]++ > 10U) {
        fn_1_6DC4();
        obj->objFunc = NULL;
        ((OMOBJ *)lbl_1_bss_24)->mtnId[0] = 1;
    }
}

/* The activity preview-return transition spins and shrinks its activity card with this helper. */
static inline void MdmicModelTransition_mdmicdll_text_1D3B8c(MDMIC_MODEL_WORK *entry, s16 frame)
{
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    {
        float value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                                       (float)frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* The activity preview-return transition spins and shrinks each player card with this helper. */
static inline void MdmicPlayerTransition_mdmicdll_text_1D3B8c(MDMIC_PLAYER_WORK *player, s16 frame)
{
    MDMIC_MODEL_WORK *entry;
    float value;
    s16 modelNo;

    modelNo = (s16)player->chrSel;
    entry = &lbl_1_bss_29C[modelNo];
    if (frame == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(entry->rot.y, entry->rot.y - 540.0f,
                             (float)frame, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, (float)frame, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (frame <= 10) {
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
}

/* Returning to the activity preview hides its record and heading while shrinking game and character
 * cards. */
void fn_1_1D3B8(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *primaryModel;
    MDMIC_PLAYER_WORK *player;
    OMOBJ *firstObj;
    OMOBJ *selectionObj;
    OMOBJ *targetObj;
    s16 index;
    s16 primaryFrame;
    s16 playerFrame;
    s16 primaryModelNo;

    if (obj->work[0] == 0) {
        firstObj = lbl_1_bss_8;
        firstObj->work[0] = 2;
        firstObj->work[1] = 0;
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
        HuWinDispOff(lbl_1_bss_298);
        targetObj = lbl_1_bss_38;
        targetObj->work[1] = 0;
        for (index = 0; index < 4; index++) {
            lbl_1_bss_E4[index].x = 0.0f;
        }
    }

    primaryFrame = (s16)obj->work[0];
    primaryModelNo = lbl_1_bss_9DC.gameSelection + 11;
    primaryModel = &lbl_1_bss_29C[primaryModelNo];
    MdmicModelTransition_mdmicdll_text_1D3B8c(primaryModel, primaryFrame);

    for (index = 0, player = lbl_1_bss_9A4; index < 4; index++, player++) {
        playerFrame = (s16)obj->work[0];
        MdmicPlayerTransition_mdmicdll_text_1D3B8c(player, playerFrame);
    }

    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        ((OMOBJ *)lbl_1_bss_24)->mtnId[0] = 1;
    }
}

/* Returning from the activity confirmation uses this callback to finish without moving the
 * layout. */
void fn_1_1DB40(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Player-order input cycles arrangements with a human in the solo slot and animates the position
 * changes. */
void fn_1_1DB64(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *playerSetup;
    HuVecF characterRotation;
    s16 teamSlot;

    if (obj->work[0]++ > 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            do {
                lbl_1_bss_9DC.playerOrderSelection--;
                if (lbl_1_bss_9DC.playerOrderSelection < 0) {
                    lbl_1_bss_9DC.playerOrderSelection = 3;
                }
            } while (
                lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][0]].isComputer != 0);
            obj->work[0] = 0;
            obj->work[1] = 0;
            fn_1_9E98(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            do {
                lbl_1_bss_9DC.playerOrderSelection++;
                if (lbl_1_bss_9DC.playerOrderSelection >= 4) {
                    lbl_1_bss_9DC.playerOrderSelection = 0;
                }
            } while (
                lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][0]].isComputer != 0);
            obj->work[0] = 0;
            obj->work[1] = 1;
            fn_1_9E98(5);
        }
    }
    if (obj->work[0] < 10) {
        for (teamSlot = 0; teamSlot < 4; teamSlot++) {
            playerSetup = &lbl_1_bss_9A4[
                lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][teamSlot]];
            Hu3DModelRotGet(
                lbl_1_bss_29C[playerSetup->chrSel].modelId, &characterRotation);
            if (obj->work[1]) {
                characterRotation.y += 18.0f;
                if (characterRotation.y > 90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_29C[playerSetup->chrSel].modelId,
                        &lbl_1_data_8BC[teamSlot + 49]);
                }
            } else {
                characterRotation.y -= 18.0f;
                if (characterRotation.y < -90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_29C[playerSetup->chrSel].modelId,
                        &lbl_1_data_8BC[teamSlot + 49]);
                }
            }
            Hu3DModelRotSetV(
                lbl_1_bss_29C[playerSetup->chrSel].modelId, &characterRotation);
        }
    } else {
        for (teamSlot = 0; teamSlot < 4; teamSlot++) {
            playerSetup = &lbl_1_bss_9A4[
                lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][teamSlot]];
            Hu3DModelRotGet(
                lbl_1_bss_29C[playerSetup->chrSel].modelId, &characterRotation);
            characterRotation.y = 0.0f;
            Hu3DModelRotSetV(
                lbl_1_bss_29C[playerSetup->chrSel].modelId, &characterRotation);
        }
    }
}

/* Player-order setup reveals the horizontal arrows and enables arrangement input. */
void fn_1_1DF74(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_29C[lbl_1_bss_9A4[0].chrSel].modelId, &pos);
    pos.x = 0.0f;
    fn_1_9D18(4, &pos, -210.0f, 0.0f);
    fn_1_9D18(5, &pos, 210.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_1DB64;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving player-order input hides both arrows and stops its callback. */
void fn_1_1E1B0(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_900[2], 4, 4);
    lbl_1_data_E64[4] = 0;
    HuSprAttrSet(lbl_1_bss_900[2], 5, 4);
    lbl_1_data_E64[5] = 0;
    obj->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* Player-order entrance resets the arrangement to zero and moves all four characters into its
 * lineup. */
void fn_1_1E248(OMOBJ *obj)
{
    MDMIC_MODEL_WORK * entry;
    MDMIC_MODEL_WORK * secondEntry;
    MDMIC_MODEL_WORK * modelEntry;
    HUSPR_GROUP * group;
    HuVecF position;
    float scale;
    float startX;
    float startY;
    float startZ;
    float targetX;
    float targetY;
    float startScale;
    float targetZ;
    float differenceX;
    float differenceY;
    float differenceZ;
    float targetScale;
    s16 i;
    s16 time;
    s16 modelIndex1;
    MDMIC_PLAYER_WORK * player;
    s16 animationIndex1;
    s16 modelIndex2;
    s16 animationIndex2;
    s16 modelIndex3;
    s16 animationIndex3;
    s16 modelIndex4;
    s16 animationIndex4;
    s16 modelNo;
    MDMIC_MODEL_WORK * thirdEntry;
    MDMIC_MODEL_WORK *fourthEntry;
    s16 groupId;
    s16 member;

    lbl_1_bss_9DC.playerOrderSelection = 0;
    if (obj->work[0] == 0)
    {
        for (i = 0; i < 4; i++)
        {
            player = & lbl_1_bss_9A4[i];
            animationIndex1 = player->chrSel;
            modelIndex1 = player->chrSel;
            entry = & lbl_1_bss_29C[modelIndex1];
            Hu3DAnimAnimSet(entry->animId[0], lbl_1_bss_814[animationIndex1]);
            Hu3DAnimBankSet(entry->animId[0], 1U);
            animationIndex2 = player->chrSel;
            modelIndex2 = player->chrSel;
            secondEntry = & lbl_1_bss_29C[modelIndex2];
            Hu3DAnimAnimSet(secondEntry->animId[1], lbl_1_bss_814[animationIndex2]);
            Hu3DAnimBankSet(secondEntry->animId[1], 1U);
            animationIndex3 = player->chrSel;
            modelIndex3 = player->chrSel;
            thirdEntry = & lbl_1_bss_29C[modelIndex3];
            Hu3DAnimAnimSet(thirdEntry->animId[2], lbl_1_bss_814[animationIndex3]);
            Hu3DAnimBankSet(thirdEntry->animId[2], 1U);
            animationIndex4 = player->chrSel;
            modelIndex4 = player->chrSel;
            fourthEntry = & lbl_1_bss_29C[modelIndex4];
            Hu3DAnimAnimSet(fourthEntry->animId[3], lbl_1_bss_814[animationIndex4]);
            Hu3DAnimBankSet(fourthEntry->animId[3], 1U);
        }
        lbl_1_bss_90[0] = 0.5f;
        groupId = lbl_1_bss_900[16];
        group = & HuSprGrpData[groupId];
        for (member = 0; member < group->sprNum; member++)
        {
            HuSprAttrReset(groupId, member, 4);
        }
    }
    for (i = 0; i < 4; i++)
    {
        time = obj->work[0];
        modelNo = (s16) lbl_1_bss_9A4[i].chrSel;
        modelEntry = & lbl_1_bss_29C[modelNo];
        if (! time)
        {
            Hu3DModelPosGet(modelEntry->modelId, & modelEntry->pos);
            Hu3DModelRotGet(modelEntry->modelId, & modelEntry->rot);
            Hu3DModelScaleGet(modelEntry->modelId, & modelEntry->scale);
        }
        position.x = fn_1_156C(modelEntry->pos.x, lbl_1_data_8BC[i + 49].x, time, 10.0f);
        position.y = fn_1_156C(modelEntry->pos.y, lbl_1_data_8BC[i + 49].y, time, 10.0f);
        position.z = fn_1_156C(modelEntry->pos.z, lbl_1_data_8BC[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(modelEntry->modelId, position.x, position.y, position.z);
        scale = fn_1_156C(modelEntry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(modelEntry->modelId, scale, scale, 1.0f);
    }
    if (obj->work[0]++ > 10U)
    {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Returning to player-order selection restores the selected arrangement and its character
 * images. */
void fn_1_1E9D8(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    MDMIC_MODEL_WORK *secondEntry;
    MDMIC_MODEL_WORK *mainEntry;
    HuVecF pos;
    float value;
    s16 i;
    s16 time;
    s16 modelIndex1;
    s16 animationIndex1;
    s16 modelIndex2;
    s16 animationIndex2;
    s32 modelNo;
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            player = &lbl_1_bss_9A4[i];
            animationIndex1 = player->chrSel;
            modelIndex1 = player->chrSel;
            entry = &lbl_1_bss_29C[modelIndex1];
            Hu3DAnimAnimSet(entry->animId[0],
                lbl_1_bss_814[animationIndex1]);
            Hu3DAnimBankSet(entry->animId[0], 1);
            animationIndex2 = player->chrSel;
            modelIndex2 = player->chrSel;
            secondEntry = &lbl_1_bss_29C[modelIndex2];
            Hu3DAnimAnimSet(secondEntry->animId[2],
                lbl_1_bss_814[animationIndex2]);
            Hu3DAnimBankSet(secondEntry->animId[2], 1);
        }
    }
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9A4[
            lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        mainEntry = &lbl_1_bss_29C[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(mainEntry->modelId, &mainEntry->pos);
            Hu3DModelRotGet(mainEntry->modelId, &mainEntry->rot);
            Hu3DModelScaleGet(mainEntry->modelId, &mainEntry->scale);
        }
        pos.x = fn_1_156C(mainEntry->pos.x,
            lbl_1_data_8BC[i + 49].x, time, 10.0f);
        pos.y = fn_1_156C(mainEntry->pos.y,
            lbl_1_data_8BC[i + 49].y, time, 10.0f);
        pos.z = fn_1_156C(mainEntry->pos.z,
            lbl_1_data_8BC[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(mainEntry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_156C(mainEntry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(mainEntry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Player-order confirmation assigns solo-side and group-side images and moves characters into team
 * positions. */
void fn_1_1F070(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    s16 i;
    s16 time;
    s16 modelIndex1;
    s16 animationIndex1;
    s16 animationModelNo;
    s16 modelIndex2;
    s16 animationIndex2;
    s16 modelIndex3;
    s16 modelIndex4;
    s16 modelNo;
    extern s16 lbl_1_data_4[4][4];
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            player = &lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][i]];
            animationIndex1 = player->chrSel;
            modelIndex1 = player->chrSel;
            fn_1_B500(modelIndex1, 0, animationIndex1, 3);
        }
        animationModelNo =
            lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][0]].chrSel;
        fn_1_B500(animationModelNo, 2, 25, -1);
        modelIndex2 = lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][1]].chrSel;
        fn_1_B500(modelIndex2, 2, 26, -1);
        modelIndex3 = lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][2]].chrSel;
        fn_1_B500(modelIndex3, 2, 26, -1);
        modelIndex4 = lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][3]].chrSel;
        fn_1_B500(modelIndex4, 2, 26, -1);
    }

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9A4[lbl_1_data_4[lbl_1_bss_9DC.playerOrderSelection][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        fn_1_BEA8(modelNo, time, 10,
            &lbl_1_data_8BC[i + 53], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        fn_1_1CC00(obj);
    }
}

/* Leaving the order layout hides its shared indicator and restores the ordinary four-player
 * lineup. */
void fn_1_1F864(OMOBJ *obj)
{
    MDMIC_PLAYER_WORK *player;
    MDMIC_MODEL_WORK *entry;
    HuVecF pos;
    float value;
    s16 i;
    s16 time;
    s32 modelNo;

    if (obj->work[0] == 0) {
        lbl_1_bss_90[0] = 0.0f;
    }
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9A4[i];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_29C[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_156C(entry->pos.x,
            lbl_1_data_8BC[i + 25].x, time, 10.0f);
        pos.y = fn_1_156C(entry->pos.y,
            lbl_1_data_8BC[i + 25].y, time, 10.0f);
        pos.z = fn_1_156C(entry->pos.z,
            lbl_1_data_8BC[i + 25].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_156C(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Game input enlarges and brings forward the selected card, then starts its slow highlight spin. */
void fn_1_1FE30(s16 modelNo)
{
    MDMIC_MODEL_WORK *entry;
    HuVecF position;

    entry = &lbl_1_bss_29C[modelNo + 11];
    Hu3DModelPosGet(entry->modelId, &position);
    if (lbl_1_bss_9DC.modeSelection == 1) {
        fn_1_1340(&position, &lbl_1_data_8BC[modelNo + 7], 5.0f);
    } else {
        fn_1_1340(&position, &lbl_1_data_8BC[modelNo + MDMIC_MINIGAME_FOCUS_POSITION_BASE], 5.0f);
    }
    Hu3DModelPosSetV(entry->modelId, &position);
    if (entry->highlightSpinFrame++ > 30) {
        entry->highlightSpinFrame = 35;
        Hu3DModelRotGet(entry->modelId, &position);
        position.y += 3.0f;
        if (position.y >= 360.0f) {
            position.y -= 360.0f;
        }
        Hu3DModelRotSetV(entry->modelId, &position);
    }
    Hu3DModelScaleGet(entry->modelId, &position);
    position.y = position.x = fn_1_1310(position.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(entry->modelId, &position);
    Hu3DModelLayerSet(entry->modelId, 2);
}

/* Game-selection setup restores an item model at its mode-specific menu position. */
void fn_1_202DC(s16 gameNo) {
    MDMIC_MODEL_WORK *entry;

    entry = &lbl_1_bss_29C[gameNo + 11];
    if (lbl_1_bss_9DC.modeSelection == 1) {
        Hu3DModelPosSetV(entry->modelId, &lbl_1_data_8BC[gameNo + 1]);
    } else {
        Hu3DModelPosSetV(entry->modelId, &lbl_1_data_8BC[gameNo + MDMIC_MINIGAME_POSITION_BASE]);
    }
    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(entry->modelId, 1.25f, 1.25f, 1.0f);
    entry->highlightSpinFrame = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

/* Game input cycles three activities or five minigames and updates the highlighted card and
 * activity record. */
void fn_1_203E8(OMOBJ *obj)
{

    s16 gameCount;
    s16 gameNo;

    if (lbl_1_bss_9DC.modeSelection == 1) {
        gameCount = 3;
    } else {
        gameCount = 5;
    }
    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & 1) {
            lbl_1_bss_9DC.gameSelection--;
            if (lbl_1_bss_9DC.gameSelection < 0) {
                lbl_1_bss_9DC.gameSelection += gameCount;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            lbl_1_bss_48[0] = 2.0f;
            if (lbl_1_data_E64[0] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
            if (lbl_1_bss_9DC.modeSelection == 2) {
                fn_1_77B8(lbl_1_bss_9DC.gameSelection + 3);
            }
        } else if (HuPadDStkRep[0] & 2) {
            lbl_1_bss_9DC.gameSelection++;
            if (lbl_1_bss_9DC.gameSelection >= gameCount) {
                lbl_1_bss_9DC.gameSelection -= gameCount;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            lbl_1_bss_48[1] = 2.0f;
            if (lbl_1_data_E64[1] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
            if (lbl_1_bss_9DC.modeSelection == 2) {
                fn_1_77B8(lbl_1_bss_9DC.gameSelection + 3);
            }
        }
    }

    for (gameNo = 0; gameNo < gameCount; gameNo++) {
        if (gameNo == lbl_1_bss_9DC.gameSelection) {
            if (lbl_1_data_CA2[gameNo] == 0) {
                fn_1_4AC0();
            } else {
                fn_1_4848(lbl_1_data_CB4[gameNo],
                    lbl_1_bss_29C[gameNo + 11].modelId);
            }
            fn_1_1FE30(gameNo);
        } else {
            fn_1_202DC(gameNo);
        }
    }
}

/* Game-input setup resets highlight spin timers, reveals navigation arrows and enables selection
 * updates. */
void fn_1_2124C(OMOBJ *obj)
{
    extern const HuVecF lbl_1_rodata_25C;

    s16 gameCount;
    s16 i;

    if (lbl_1_bss_9DC.modeSelection == 1) {
        gameCount = 3;
    } else {
        gameCount = 5;
    }
    for (i = 0; i < gameCount; i++) {
        lbl_1_bss_29C[i + 11].highlightSpinFrame = 0;
    }

    if (lbl_1_bss_9DC.modeSelection == 1) {
        fn_1_9D18(0, &lbl_1_data_8BC[0], -200.0f, 0.0f);

        fn_1_9D18(1, &lbl_1_data_8BC[0], 200.0f, 0.0f);
    } else {
        fn_1_9D18(0, &lbl_1_data_8BC[0], -210.0f, 0.0f);

        fn_1_9D18(1, &lbl_1_data_8BC[0], 210.0f, 0.0f);
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_203E8;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Leaving game input hides the arrows and straightens each card before signaling completion. */
void fn_1_216D4(OMOBJ *obj)
{
    s16 modelCount;
    s16 i;
    if (lbl_1_bss_9DC.modeSelection == 1) {
        modelCount = 3;
    } else {
        modelCount = 5;
    }
    if (obj->work[0] == 0) {
        fn_1_9E3C(0);
        fn_1_9E3C(1);
    }
    for (i = 0; i < modelCount; i++) {
        lbl_1_bss_29C[i + 11].highlightSpinFrame = 0;
        fn_1_CBD4(i + 11, 0.0f, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

static inline int CurrentMinigameGet_mdmicdll_text_216D4c(void)
{
    return GwSystem.mgNo + 601;
}

static inline int CurrentSubGameGet_mdmicdll_text_216D4c(void)
{
    return GwSystem.subGameNo;
}

/* Game-screen entrance restores a returning game choice, applies locked images and spins in its
 * cards. */
void fn_1_219F0(OMOBJ *obj)
{
    s16 modelCount;
    s16 searchIndex;

    if (lbl_1_bss_9DC.modeSelection == 1) {
        modelCount = 3;
    } else {
        modelCount = 5;
    }
    if (obj->work[0] == 0) {
        lbl_1_bss_9DC.gameSelection = 0;
        if (lbl_1_bss_9DC.modeSelection == 2 && lbl_1_bss_28 == 1) {
            searchIndex = 0;
            while (searchIndex < 5) {
                if (lbl_1_data_F56[searchIndex] == CurrentMinigameGet_mdmicdll_text_216D4c()) {
                    break;
                }
                searchIndex++;
            }
            if (searchIndex < 5) {
                lbl_1_bss_9DC.gameSelection = searchIndex;
            }
        } else if (lbl_1_bss_9DC.modeSelection == 1 && lbl_1_bss_28 == 1) {
            lbl_1_bss_9DC.gameSelection = CurrentSubGameGet_mdmicdll_text_216D4c();
        }
    }
    if (obj->work[0] == 0) {
        for (searchIndex = 0; searchIndex < modelCount; searchIndex++) {
            if (lbl_1_bss_9DC.modeSelection == 1) {
                fn_1_B5A8(searchIndex + 11, searchIndex + 15);
                if (lbl_1_data_CA2[searchIndex] == 0) {
                    fn_1_B500(searchIndex + 11, 2, 27, -1);
                    fn_1_B500(searchIndex + 11, 3, 27, -1);
                }
            } else {
                fn_1_B5A8(searchIndex + 11, searchIndex + 20);
                if (lbl_1_data_CA8[searchIndex] == 0) {
                    fn_1_B500(searchIndex + 11, 2, 28, -1);
                    fn_1_B500(searchIndex + 11, 3, 28, -1);
                }
            }
        }
    }
    for (searchIndex = 0; searchIndex < modelCount; searchIndex++) {
        if (lbl_1_bss_9DC.modeSelection == 1) {
            fn_1_B6CC(searchIndex + 11, obj->work[0], 10,
                &lbl_1_data_8BC[searchIndex + 1], 1.25f);
        } else {
            fn_1_B6CC(searchIndex + 11, obj->work[0], 10,
                &lbl_1_data_8BC[searchIndex + 67], 1.25f);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DZClearLayerSet(2);
        if (lbl_1_bss_9DC.modeSelection == 2) {
            fn_1_77B8(lbl_1_bss_9DC.gameSelection + 3);
        } else if (lbl_1_bss_28 == 1 && lbl_1_bss_44 == 0) {
            lbl_1_bss_44 = 1;
            fn_1_77B8(lbl_1_bss_9DC.modeSelection);
        }
        lbl_1_bss_28 = 0;
        fn_1_1CC00(obj);
    }
}

static inline void DebugValue_mdmicdll_text_2299Cc(s32 value) { }

/* After game choice, this exit hides the activity record and spins the selected card down to zero
 * scale. */
void fn_1_2299C(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *entry;
    OMOBJ *targetObj;
    float value;
    s16 modelNo;
    s16 time;

    if (obj->work[0] == 0) {
        targetObj = lbl_1_bss_38;
        targetObj->work[1] = 0;
    }
    time = (s16)obj->work[0];
    modelNo = lbl_1_bss_9DC.gameSelection + 11;
    entry = &lbl_1_bss_29C[modelNo];
    if (time == 0) {
        if (lbl_1_bss_42 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_42 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_42 = 0;
    }
    value = fn_1_156C(
        entry->rot.y, entry->rot.y - 540.0f, (float)time, 10.0f);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_156C(entry->scale.x, 0.0f, (float)time, 10.0f);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (time <= 10) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After a game is chosen, dismiss the other cards and move the chosen one into its confirmation
 * position. */
void fn_1_22D54(OMOBJ *obj)
{
    s16 modelCount;
    s16 i;
    s16 frame;
    s16 singlePlayerFrame;
    s16 multiplePlayerFrame;
    if (lbl_1_bss_9DC.modeSelection == 1) {
        modelCount = 3;
    } else {
        modelCount = 5;
    }
    for (i = 0; i < modelCount; i++) {
        if (i != lbl_1_bss_9DC.gameSelection) {
            frame = (s16)obj->work[0];
            fn_1_BAD8((s16)(i + 11), frame, 10);
        }
    }
    if (obj->work[0] >= 10) {
        if (lbl_1_bss_9DC.modeSelection == 1) {
            singlePlayerFrame = (s16)(obj->work[0] - 10);
            fn_1_BEA8((s16)(lbl_1_bss_9DC.gameSelection + 11), singlePlayerFrame,
                10, &lbl_1_data_8BC[0], 2.25f);
        } else {
            multiplePlayerFrame = (s16)(obj->work[0] - 10);
            fn_1_BEA8((s16)(lbl_1_bss_9DC.gameSelection + 11), multiplePlayerFrame,
                10, &lbl_1_data_8BC[37], 1.25f);
        }
    }
    if (obj->work[0] == 20) {
        Hu3DLayerHookReset(2);
        DebugValue_mdmicdll_text_2299Cc(lbl_1_bss_9DC.modeSelection == 2);
    }
    if (obj->work[0]++ > 20) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Leaving game selection hides the record display and spins all game cards down to zero scale. */
void fn_1_23B18(OMOBJ *obj)
{
    MDMIC_MODEL_WORK *entry;
    s16 modelCount;
    s16 i;
    s16 frame;
    float value;
    OMOBJ *workObj;

    if (lbl_1_bss_9DC.modeSelection == 1) {
        modelCount = 3;
    } else {
        modelCount = 5;
    }
    if (obj->work[0] == 0) {
        DebugValue_mdmicdll_text_2299Cc((s32)(lbl_1_bss_9DC.modeSelection == 2));
        workObj = lbl_1_bss_38;
        workObj->work[1] = 0;
    }
    for (i = 0; i < modelCount; i++) {
        frame = (s16)obj->work[0];
        entry = &lbl_1_bss_29C[(s16)(i + 11)];
        if (frame == 0) {
            if (lbl_1_bss_42 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_42 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_42 = 0;
        }
        value = fn_1_156C(
            entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_156C(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Overlay exit releases the scene models, animations, effects, windows, lights and camera. */
void fn_1_23F2C(void)
{
    extern OMOBJ *lbl_1_bss_14;
    extern HUWINID lbl_1_bss_114[2];
    extern HUWINID lbl_1_bss_298;
    extern OMOBJ *lbl_1_bss_24;
    extern HUWINID lbl_1_bss_A44[4];
    extern HU3D_LIGHTID lbl_1_bss_A4C[2];
    void fn_1_3BE34(void);
    OMOBJ *firstObj;
    OMOBJ *thirdObj;
    OMOBJ *fourthObj;
    OMOBJ *lastObj;
    MDMIC_MODEL_WORK *entry;
    s16 characterModel;
    s16 finalModel;
    s16 i;
    s16 j;
    s16 finalWindow;
    s16 firstWindow;
    s16 playerModel;
    OMOBJ **objectPointer;

    firstObj = lbl_1_bss_4;
    if (firstObj) {
        for (characterModel = 0; characterModel < 3; characterModel++) {
            Hu3DMotionKill(firstObj->mtnId[characterModel]);
            Hu3DModelKill(firstObj->mdlId[characterModel]);
        }
        omDelObjEx(lbl_1_bss_0, firstObj);
    }
    firstObj = NULL;
    fn_1_86F4(lbl_1_bss_8);
    thirdObj = lbl_1_bss_C[0];
    if (thirdObj) {
        Hu3DMotionKill(thirdObj->mtnId[0]);
        Hu3DMotionKill(thirdObj->mtnId[1]);
        Hu3DMotionKill(thirdObj->mtnId[2]);
        Hu3DMotionKill(thirdObj->mtnId[3]);
        Hu3DMotionKill(thirdObj->mtnId[4]);
        Hu3DModelKill(thirdObj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, thirdObj);
    }
    thirdObj->objFunc = NULL;
    fourthObj = lbl_1_bss_14;
    if (fourthObj) {
        for (playerModel = 0; playerModel < 11; playerModel++) {
            Hu3DMotionKill(fourthObj->mtnId[playerModel * 2]);
            Hu3DMotionKill(fourthObj->mtnId[(playerModel * 2) + 1]);
            Hu3DModelKill(fourthObj->mdlId[playerModel]);
        }
        omDelObjEx(lbl_1_bss_0, fourthObj);
    }
    fourthObj = NULL;
    for (firstWindow = 0; firstWindow < 2; firstWindow++) {
        HuWinExKill(lbl_1_bss_114[firstWindow]);
    }
    lastObj = lbl_1_bss_1C[0];
    if (lastObj) {
        Hu3DMotionKill(lastObj->mtnId[0]);
        for (finalModel = 3; finalModel >= 0; finalModel--) {
            Hu3DModelKill(lastObj->mdlId[finalModel]);
        }
        omDelObjEx(lbl_1_bss_0, lastObj);
    }
    lastObj = NULL;
    HuWinExKill(lbl_1_bss_298);
    fn_1_3BE34();
    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 24; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_29C[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_29C[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
    for (finalWindow = 0; finalWindow < 4; finalWindow++) {
        HuWinExKill(lbl_1_bss_A44[finalWindow]);
    }
    HuWinAllKill();
    Hu3DGLightKill(lbl_1_bss_A4C[0]);
    Hu3DGLightKill(lbl_1_bss_A4C[1]);
    objectPointer = &lbl_1_bss_9F4.obj;
    Hu3DCameraKill(1);
    if (*objectPointer) {
        omDelObjEx(lbl_1_bss_0, *objectPointer);
    }
    *objectPointer = NULL;
}

/* The exit callback waits for the outgoing wipe to finish, cleans the scene and returns to the
 * previous overlay. */
void fn_1_2438C(OMOBJ *obj)
{
    if (!WipeCheck()) {
        fn_1_23F2C();
        omOvlReturnEx(1, 1);
    }
}

/* The scene exit watcher starts an outgoing wipe when the system requests an exit. */
void fn_1_243C8(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(2, 0, 60);
        obj->objFunc = fn_1_2438C;
    }
}

/* The menu child process runs setup, fades out and cleans the scene before entering a game or
 * returning. */
void fn_1_24420(void)
{
    s16 i;
    s16 state = 0;

    state = fn_1_34C78(0);

    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, 1000);
        lbl_1_data_0 = -1;
    }
    switch (state) {
    case 0:
    case 1:
            _ClearFlag(FLAG_INST_MG_MODE);
            _ClearFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
            WipeCreate(2, 0, 60);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
        break;
    case 2:
    case 3:
            WipeCreate(2, 0, 60);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
        break;
    }
    fn_1_23F2C();
    if (state == 2) {
        fn_1_C78();
    } else {
        MgPauseExitF = 0;
        CharDataClose(-1);
        HuARDirFree(DATA_micquiz);
        HuDataDirClose(DATA_mikeact);
        HuDataDirClose(DATA_inst);
        for (i = 0; i < 14; i++) {
            HuARDirFree(CharDataDirTbl[i][2]);
        }
        OSReport(lbl_1_data_D9D);
        OSReport(lbl_1_data_CF0, _effect);
        OSReport(lbl_1_data_D01, _gamemes);
        OSReport(lbl_1_data_D13, _mgconst);
        OSReport(lbl_1_data_D25, _win);
        HuAMemDump();
        OSReport(lbl_1_data_D33);
        omOvlReturnEx(1, 1);
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Overlay startup calls this to create the Mic Mode camera, objects and menu windows. */
void fn_1_24614(void)
{
    extern s16 lbl_1_bss_A44[4];
    extern s16 lbl_1_bss_A4C[2];

    extern OMOBJ *lbl_1_bss_14;
    extern OMOBJ *lbl_1_bss_18;
    extern OMOBJ *lbl_1_bss_24;
    extern s16 lbl_1_bss_298;
    void fn_1_0(HUWINID winId, u32 messageId, s16 cueIndex);
    void fn_1_25550(OMOBJ *obj, MDMIC_CAMERA_WORK *camera);
    void fn_1_2FD4(OMOBJ *obj);
    void fn_1_4AE8(OMOBJ *obj);
    void fn_1_7588(OMOBJ *obj);
    void fn_1_84B0(OMOBJ *obj);
    void fn_1_60D4(OMOBJ *obj);
    void fn_1_70B4(OMOBJ *obj);
    void fn_1_9980(OMOBJ *obj);
    void fn_1_A870(OMOBJ *obj);
    void fn_1_24420(void);
    void fn_1_3B3E4(HUPROCESS *process);
    OMOBJ *object;
    MDMIC_CAMERA_WORK *state;

    lbl_1_bss_0 = omInitObjMan(MDMIC_OBJECT_CAPACITY, MDMIC_MANAGER_PRIORITY);
    omGameSysInit(lbl_1_bss_0);
    state = &lbl_1_bss_9F4;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
                             10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    memset(state, 0, 80U);
    state->callback = fn_1_25550;
    state->center.x = 0.0f;
    state->center.y = 65.0f;
    state->center.z = (-800.0f);
    state->rot.x = (-7.25f);
    state->rot.y = 0.0f;
    state->rot.z = 0.0f;
    if (lbl_1_bss_28 == 0) {
        state->zoom = 2650.0f;
    } else {
        state->zoom = 2150.0f;
    }
    state->obj = omAddObjEx(lbl_1_bss_0, MDMIC_CAMERA_OBJECT_PRIORITY, 0, 0, -1,
                            fn_1_2FD4);
    lbl_1_bss_A4C[0] = Hu3DGLightCreate(
        0.0f, 1.0f, 1.0f,
        0.0f, (-1.0f), (-1.0f),
        255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_A4C[0]);
    Hu3DGLightStaticSet(lbl_1_bss_A4C[0], 1);
    lbl_1_bss_A4C[1] = Hu3DGLightCreate(
        (-1.0f), 1.0f, (-1.0f),
        1.0f, (-1.0f), (-1.0f),
        255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_A4C[1]);
    Hu3DGLightStaticSet(lbl_1_bss_A4C[1], 1);
    fn_1_3720();
    fn_1_4380();
    fn_1_4538();
    fn_1_CF3C();
    lbl_1_bss_38 = omAddObjEx(lbl_1_bss_0, MDMIC_RECORD_OBJECT_PRIORITY, 0, 0, -1,
                              fn_1_4AE8);
    object = lbl_1_bss_38;
    omObjGetWork(object, u32)[1] = 0;
    fn_1_3B3E4(lbl_1_bss_0);
    lbl_1_bss_298 = HuWinExCreateFrame(134.0f,
                                        40.0f,
                                        300, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_298);
    HuWinBGTPLvlSet(lbl_1_bss_298, 0.0f);
    HuWinPriSet(lbl_1_bss_298, 0);
    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 4, 4, -1, fn_1_7588);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 2, 4, -1, fn_1_84B0);
    lbl_1_bss_C[0] = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 1, 4, -1, fn_1_60D4);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 16, 32,
                              -1, fn_1_70B4);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 0, 0, -1, fn_1_9980);
    lbl_1_bss_1C[0] = omAddObjEx(lbl_1_bss_0, MDMIC_OBJECT_PRIORITY, 4, 1, -1, fn_1_A870);
    HuPrcChildCreate(fn_1_24420, MDMIC_MAIN_PROCESS_PRIORITY, MDMIC_MAIN_PROCESS_STACK_SIZE, 0,
                     lbl_1_bss_0);
}

/* Overlay setup releases previous mode archives and cached character motion archives. */
void fn_1_251FC(void) {
    s16 characterNo;

    OSReport(lbl_1_data_F60);
    HuARDirFree(DATA_micquiz);
    HuDataDirClose(DATA_mikeact);
    HuDataDirClose(DATA_inst);
    characterNo = 0;
    while (characterNo < GW_CHARA_MAX) {
        HuARDirFree((&CharDataDirTbl[characterNo][0])[2]);
        characterNo += 1;
    }
    OSReport(lbl_1_data_D6F);
    OSReport(lbl_1_data_CF0, _effect);
    OSReport(lbl_1_data_D01, _gamemes);
    OSReport(lbl_1_data_D13, _mgconst);
    OSReport(lbl_1_data_D25, _win);
    HuAMemDump();
    OSReport(lbl_1_data_D33);
    MgInstExitF = 0;
    MgPauseExitF = 0;
    MgExitReq = 0;
    lbl_1_bss_2C = 0;
    lbl_1_bss_2E = 0;
    if (GWBankFlagGet(2) != 0) {
        lbl_1_bss_2C = 1;
    }
    if (GWBankFlagGet(3) != 0) {
        lbl_1_bss_2E = 1;
    }
    lbl_1_data_CA2[0] = 1;
    lbl_1_data_CA2[1] = 1;
    lbl_1_data_CA2[2] = 0;
    if (GWBankFlagGet(MDMIC_BANK_ACTIVITY_THREE_FLAG) != 0) {
        lbl_1_data_CA2[2] = 1;
    }
    lbl_1_data_CB4[0] = GWMikeActRecordGet(0);
    lbl_1_data_CB4[1] = GWMikeActRecordGet(1);
    lbl_1_data_CB4[2] = GWMikeActRecordGet(2);
    lbl_1_data_CA8[0] = 1;
    lbl_1_data_CA8[1] = 1;
    lbl_1_data_CA8[2] = 1;
    lbl_1_data_CA8[3] = 1;
    lbl_1_data_CA8[4] = 1;
    lbl_1_bss_28 = (s16) omovlevtno;
    lbl_1_bss_2A = (s16) omovlstat;
    _ClearFlag(FLAG_INST_MG_MODE);
    _ClearFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
    fn_1_24614();
}

/* Overlay loading runs static initializers before starting Mic Mode setup. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_251FC();
    return 0;
}

/* Overlay unloading runs each registered static destructor. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* The normal camera callback eases the view toward the fixed menu center, rotation and zoom. */
void fn_1_25550(OMOBJ *obj, MDMIC_CAMERA_WORK *camera)
{
    camera->targetCenter.x = 0.0f;
    camera->targetCenter.y = 65.0f;
    camera->targetCenter.z = -800.0f;
    camera->targetRot.x = -7.25f;
    camera->targetRot.y = 0.0f;
    camera->targetRot.z = 0.0f;
    camera->targetZoom = 2150.0f;
    fn_1_25B4(camera, 15.0f);
}

/* The game-entry camera callback raises and moves the view back over a sixty-frame transition. */
void fn_1_258B4(OMOBJ *obj, MDMIC_CAMERA_WORK *state)
{
    state->center.y = fn_1_156C(state->targetCenter.y, 275.0f, state->transitionFrame, 60.0f);
    state->center.z = fn_1_156C(state->targetCenter.z, -2900.0f, state->transitionFrame, 60.0f);
    state->rot.x = fn_1_156C(state->targetRot.x, 0.0f, state->transitionFrame, 60.0f);
    state->transitionFrame += 1.0f;
}

/* Camera transition setup snaps to the saved target view and starts the sixty-frame game-entry
 * movement. */
void fn_1_25ADC(OMOBJ *obj, MDMIC_CAMERA_WORK *state)
{
    state->transitionFrame = 0.0f;
    memcpy(&state->center, &state->targetCenter, 12U);
    memcpy(&state->rot, &state->targetRot, 12U);
    state->zoom = state->targetZoom;
    lbl_1_bss_9F4.callback = fn_1_258B4;
}

/* Returning-game setup restores the saved mode, player choices, character cards and player
 * badges. */
void fn_1_25B50(void)
{
    s16 selectionIndex;
    s16 animationIndex;
    s16 padNumber;
    s16 difficultyBank;
    s16 characterIndex;
    MDMIC_MODEL_WORK *animationCard;
    MDMIC_MODEL_WORK *backCard;
    MDMIC_MODEL_WORK *alternateBackCard;
    MDMIC_MODEL_WORK *frontCard;
    MDMIC_MODEL_WORK *alternateFrontCard;
    MDMIC_MODEL_WORK *selectedCard;
    OMOBJ *selectionObj;
    s16 selectedMode;

    lbl_1_bss_9DC.participantCount = 0;
    lbl_1_bss_9DC.modeSelection = lbl_1_bss_2A;
    if (lbl_1_bss_9DC.modeSelection == 0) {
        lbl_1_bss_9DC.gameSelection = 0;
        return;
    }
    selectedMode = lbl_1_bss_9DC.modeSelection;
    selectionObj = lbl_1_bss_8;
    Hu3DAnimAnimSet(selectionObj->mtnId[2],
        lbl_1_bss_924[(selectedMode * 2) + 3]);
    Hu3DAnimAnimSet(selectionObj->mtnId[3],
        lbl_1_bss_924[(selectedMode * 2) + 4]);
    selectionObj->work[0] = 3;
    selectionObj->work[1] = 0;
    for (selectionIndex = 0; selectionIndex < 4; selectionIndex += 1) {
        lbl_1_bss_9A4[selectionIndex].characterChoiceState = 0;
        lbl_1_bss_9A4[selectionIndex].reservedSetup = 0;
        lbl_1_bss_9A4[selectionIndex].isComputer = GwPlayer[selectionIndex].comF;
        lbl_1_bss_9A4[selectionIndex].computerDifficulty = GwPlayer[selectionIndex].comDif;
        lbl_1_bss_9A4[selectionIndex].chrSel = GwPlayer[selectionIndex].charNo;
        lbl_1_bss_9A4[selectionIndex].padNo = GwPlayer[selectionIndex].padNo;
        if (lbl_1_bss_9A4[selectionIndex].isComputer == 0) {
            HUSPR_GROUP *group;
            s16 memberIndex;
            s16 groupId;

            padNumber = lbl_1_bss_9A4[selectionIndex].padNo;
            lbl_1_bss_E4[selectionIndex].x = 0.85000002384185791f;
            lbl_1_bss_E4[selectionIndex].y = 0.85000002384185791f;
            groupId = lbl_1_bss_900[selectionIndex + 4];
            group = &HuSprGrpData[groupId];
            memberIndex = 0;
            while (memberIndex < group->sprNum) {
                HuSprAttrReset(groupId, memberIndex, 4);
                memberIndex += 1;
            }
            HuSprBankSet(lbl_1_bss_900[selectionIndex + 4], 0, padNumber);
            lbl_1_bss_9DC.participantCount += 1;
        } else if (lbl_1_bss_9DC.modeSelection == 1) {
            HUSPR_GROUP *group;
            s16 memberIndex;
            s16 groupId;

            lbl_1_bss_E4[selectionIndex].x = 0.85000002384185791f;
            lbl_1_bss_E4[selectionIndex].y = 0.85000002384185791f;
            groupId = lbl_1_bss_900[selectionIndex + 4];
            group = &HuSprGrpData[groupId];
            memberIndex = 0;
            while (memberIndex < group->sprNum) {
                HuSprAttrReset(groupId, memberIndex, 4);
                memberIndex += 1;
            }
            HuSprBankSet(lbl_1_bss_900[selectionIndex + 4], 0, 4);
        } else {
            HUSPR_GROUP *group;
            s16 memberIndex;
            s16 groupId;

            difficultyBank = lbl_1_bss_9A4[selectionIndex].computerDifficulty + 5;
            lbl_1_bss_E4[selectionIndex].x = 0.85000002384185791f;
            lbl_1_bss_E4[selectionIndex].y = 0.85000002384185791f;
            groupId = lbl_1_bss_900[selectionIndex + 4];
            group = &HuSprGrpData[groupId];
            memberIndex = 0;
            while (memberIndex < group->sprNum) {
                HuSprAttrReset(groupId, memberIndex, 4);
                memberIndex += 1;
            }
            HuSprBankSet(lbl_1_bss_900[selectionIndex + 4], 0, difficultyBank);
        }
    }

    for (selectionIndex = 0; selectionIndex < 11; selectionIndex += 1) {
        s16 animationSlot;

        for (animationIndex = 0; animationIndex < 4; animationIndex += 1) {
            animationSlot = animationIndex;
            animationCard = &lbl_1_bss_29C[selectionIndex];
            Hu3DAnimAnimSet(animationCard->animId[animationSlot],
                            lbl_1_bss_814[selectionIndex]);
        }
        backCard = &lbl_1_bss_29C[selectionIndex];
        Hu3DAnimAnimSet(backCard->animId[1], lbl_1_bss_814[selectionIndex]);
        Hu3DAnimBankSet(backCard->animId[1], 1U);
        alternateBackCard = &lbl_1_bss_29C[selectionIndex];
        Hu3DAnimAnimSet(alternateBackCard->animId[3], lbl_1_bss_814[selectionIndex]);
        Hu3DAnimBankSet(alternateBackCard->animId[3], 1U);
        frontCard = &lbl_1_bss_29C[selectionIndex];
        Hu3DAnimAnimSet(frontCard->animId[0], lbl_1_bss_814[selectionIndex]);
        Hu3DAnimBankSet(frontCard->animId[0], 1U);
        alternateFrontCard = &lbl_1_bss_29C[selectionIndex];
        Hu3DAnimAnimSet(alternateFrontCard->animId[2], lbl_1_bss_814[selectionIndex]);
        Hu3DAnimBankSet(alternateFrontCard->animId[2], 1U);
    }

    selectionIndex = 0;
    while (selectionIndex < 4) {
        characterIndex = (s16)lbl_1_bss_9A4[selectionIndex].chrSel;
        selectedCard = &lbl_1_bss_29C[characterIndex];
        Hu3DModelPosSet(selectedCard->modelId,
                        lbl_1_data_8BC[selectionIndex + 25].x,
                        lbl_1_data_8BC[selectionIndex + 25].y,
                        lbl_1_data_8BC[selectionIndex + 25].z);
        Hu3DModelScaleSet(selectedCard->modelId, 1.2500000000000000f,
                          1.2500000000000000f, 1.0f);
        Hu3DModelAttrReset(selectedCard->modelId, 1U);
        selectionIndex += 1;
    }
}

/* The opening process restores returning-game state, starts menu music and waits for the incoming
 * wipe. */
s32 fn_1_262E8(void) {
    HuPrcSleep(5);
    if (lbl_1_bss_28 == 1) {
        fn_1_25B50();
    }
    lbl_1_data_0 = HuAudSStreamPlay(MSM_STREAM_W01);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    return 1;
}

/* The exit prompt reads the active dialogue choice, or returns zero if no window is active. */
static inline s16 fn_1_3F28_inlined_mdmicdll_text_26364c(s16 mode)
{
    if (lbl_1_data_E32[0] != -1) {
        return fn_1_381C_inlined_mdmicdll_text_26364c(lbl_1_data_E32[0], mode);
    }
    return 0;
}

/* The exit prompt configures cancellation and waits for a choice, mapping cancel to choice one in
 * mode three. */
static inline s16 fn_1_381C_inlined_mdmicdll_text_26364c(s16 winNo, s16 mode)
{
    s16 choice = 0;
    s16 retryChoice;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_A44[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_A44[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_A44[winNo], -1);
    if (mode == 3 && choice == -1) {
        choice = 1;
    }
    return choice;
}

/* Menu flows ask whether to leave Mic Mode; cancel is treated as staying. */
s16 fn_1_26364(void)
{
    fn_1_5328();
    fn_1_3D24(3, ((int)MESSNUM(MESS_MIC_SETTING, 32)), 1);
    return fn_1_3F28_inlined_mdmicdll_text_26364c(3);
}

/* The opening flow asks whether to leave Mic Mode, treating a canceled choice as staying. */
static inline s16 fn_1_26364_for_2666C_mdmicdll_text_26364c(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* First-entry setup shows the introduction and opening choice, with a confirmation before
 * leaving. */
s16 fn_1_2666C(void)
{
    s16 choice = 0;

    fn_1_5328();

    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 0), 1);
    fn_1_3BCC();

    for (;;) {
        HuPrcVSleep();
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 35), 1);
        choice = fn_1_3C2C(0);
        if (choice == -1) {
            if (!fn_1_26364_for_2666C_mdmicdll_text_26364c()) {
                choice = -1;
                break;
            }
        } else {
            if (choice == 0) {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 36), 1);
                fn_1_3BCC();
            }
            choice = 1;
            break;
        }
    }

    fn_1_7554();
    return choice;
}

/* The confirmation flow starts the host return only while the host is near the center. */
static inline s32 fn_1_5B48_inlined_mdmicdll_text_27158c(void)
{
    Point3d position;
    OMOBJ *hostObj;

    hostObj = lbl_1_bss_C[0];
    Hu3DModelPosGet(*hostObj->mdlId, &position);
    if (position.x < (-100.0f)) {
        return 0;
    }
    hostObj->work[0] = 0;
    hostObj->objFunc = fn_1_5A90;
    Hu3DMotionShiftSet(*hostObj->mdlId, hostObj->mtnId[1], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    return 1;
}

/* The host-return flow closes its active dialogue and clears cached message IDs. */
static inline void fn_1_3B10_inlined_mdmicdll_text_27158c(void)
{
    if (lbl_1_data_E32[0] != -1) {
        fn_1_3474_inlined_mdmicdll_text_27158c(lbl_1_data_E32[0]);
    }
    lbl_1_data_E32[0] = -1;
    lbl_1_data_E38[0] = -1;
    lbl_1_data_E38[1] = -1;
}

/* The host-return flow hides the guide immediately or animates another dialogue window closing. */
static inline void fn_1_3474_inlined_mdmicdll_text_27158c(s16 windowNo)
{
    if (windowNo == 0) {
        HuWinDispOff(lbl_1_bss_A44[windowNo]);
        return;
    }
    HuWinExClose(lbl_1_bss_A44[windowNo]);
}

/* Selection scripts return the host to its side position and close dialogue when it is near the
 * center. */
s32 fn_1_27158(void)
{
    Point3d position;
    Hu3DModelPosGet(*lbl_1_bss_C[0]->mdlId, &position);
    if (position.x > (-200.0f)) {
        fn_1_5B48_inlined_mdmicdll_text_27158c();
        fn_1_3B10_inlined_mdmicdll_text_27158c();
        HuPrcSleep(60);
    }
}

/* The mode-selection introduction restores the host position and waits for its opening message. */
s32 fn_1_272D4(void)
{
    Point3d startPosition;
    Point3d currentPosition;
    OMOBJ *movingObject;
    OMOBJ *menuObject;
    s16 messageWindow;
    s16 closeWindow;
    s16 closePromptWindow;
    s16 openPromptWindow;
    u32 messageId;
    s16 waitWindow;

    Hu3DModelPosGet(*lbl_1_bss_C[0]->mdlId, &startPosition);
    if (startPosition.x > (-200.0f)) {
        movingObject = lbl_1_bss_C[0];
        Hu3DModelPosGet(*movingObject->mdlId, &currentPosition);
        if (currentPosition.x < (-100.0f)) {
            /* Keep the current motion while the model remains left of this point. */
        } else {
            movingObject->work[0] = 0;
            movingObject->objFunc = fn_1_5A90;
            Hu3DMotionShiftSet(*movingObject->mdlId, movingObject->mtnId[1],
                0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        }
        if (lbl_1_data_E32[0] != -1) {
            closeWindow = lbl_1_data_E32[0];
            if (closeWindow == 0) {
                HuWinDispOff(lbl_1_bss_A44[closeWindow]);
            } else {
                HuWinExClose(lbl_1_bss_A44[closeWindow]);
            }
        }
        lbl_1_data_E32[0] = -1;
        lbl_1_data_E38[0] = -1;
        lbl_1_data_E38[1] = -1;
        HuPrcSleep(60);
    }
    menuObject = lbl_1_bss_C[0];
    Hu3DMotionShiftSet(*menuObject->mdlId, menuObject->mtnId[3],
        0.0f, 10.0f, 0U);
    menuObject->work[3] = 0;
    menuObject->objFunc = fn_1_5280;
    if ((lbl_1_data_E32[0] != -1) && (lbl_1_data_E32[0] != 3)) {
        closePromptWindow = lbl_1_data_E32[0];
        if (closePromptWindow == 0) {
            HuWinDispOff(lbl_1_bss_A44[closePromptWindow]);
        } else {
            HuWinExClose(lbl_1_bss_A44[closePromptWindow]);
        }
    }
    if ((lbl_1_data_E32[0] == -1) || (lbl_1_data_E32[0] != 3)) {
        lbl_1_data_E32[0] = 3;
        lbl_1_data_E38[0] = -1;
        lbl_1_data_E38[1] = -1;
        openPromptWindow = lbl_1_data_E32[0];
        if (openPromptWindow == 0) {
            HuWinDispOn(lbl_1_bss_A44[openPromptWindow]);
        } else {
            HuWinExOpen(lbl_1_bss_A44[openPromptWindow]);
        }
    }
    if ((u32)(lbl_1_data_E38[0] - MESSNUM(MESS_MIC_SETTING, 0)) != 1U) {
        lbl_1_data_E38[0] = MESSNUM(MESS_MIC_SETTING, 1);
        lbl_1_data_E38[1] = -1;
        messageId = (u32)lbl_1_data_E38[0];
        messageWindow = lbl_1_data_E32[0];
        HuWinAttrSet(lbl_1_bss_A44[messageWindow], 2048U);
        HuWinMesSet(lbl_1_bss_A44[messageWindow], messageId);
        HuWinMesSpeedSet(lbl_1_bss_A44[messageWindow], 1);
        if ((s32)lbl_1_data_C7C != (s32)messageId) {
            lbl_1_data_C7C = -1U;
        }
    }
    if (lbl_1_data_E32[0] != -1) {
        waitWindow = lbl_1_data_E32[0];
        HuWinMesWait(lbl_1_bss_A44[waitWindow]);
    }
    return 1;
}

/* The player-count flow asks whether to leave Mic Mode, treating cancel as staying. */
static inline s16 fn_1_26364_for_276E0_mdmicdll_text_272D4c(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* Player-count setup runs card input and handles confirmation, backtracking and exit requests. */
s16 fn_1_276E0(void) {
    s16 selectionResult = 0;
    fn_1_CEA4(fn_1_EF08);
    for (;;) {
    HuPrcVSleep();
    fn_1_CEA4(fn_1_EE40);
    fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
    if (lbl_1_bss_9DC.modeSelection == 2) {
        fn_1_3D24(3, lbl_1_bss_9DC.participantCount + (s32)MESSNUM(MESS_PARTY_SETTING, 9), 0);
    } else if (lbl_1_bss_9DC.participantCount <= 1) {
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 33), 0);
    } else {
        fn_1_3F5C(3, MESSNUM(MESS_MIC_SETTING, 13) - (lbl_1_bss_9DC.participantCount - 1), 0);
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 5), 0);
    }
    for (;;) {
    HuPrcVSleep();
    fn_1_4150(MESSNUM(MESS_SYS_GUIDE, 10));
    if (lbl_1_bss_9DC.modeSelection == 2) {
        fn_1_3D24(3, lbl_1_bss_9DC.participantCount + (s32)MESSNUM(MESS_PARTY_SETTING, 9), 0);
    } else if (lbl_1_bss_9DC.participantCount <= 1) {
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 33), 0);
    } else {
        fn_1_3F5C(3, MESSNUM(MESS_MIC_SETTING, 13) - (lbl_1_bss_9DC.participantCount - 1), 0);
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 5), 0);
    }
    if (!fn_1_CE6C()) {
        continue;
    }
    if ((s32) (HuPadBtnDown[0] & PAD_BUTTON_A) != 0) {
        if (lbl_1_bss_9DC.modeSelection == 0 && lbl_1_bss_9DC.participantCount <= 1) {
            HuAudFXPlay(MSM_SE_CMN_05);
            continue;
        }
        HuAudFXPlay(MSM_SE_CMN_03);
        fn_1_2460();
        selectionResult = 0;
    } else if ((s32) (HuPadBtnDown[0] & PAD_BUTTON_B) != 0) {
        HuAudFXPlay(MSM_SE_CMN_04);
        selectionResult = 1;
    } else if ((s32) (HuPadBtnDown[0] & PAD_BUTTON_X) != 0) {
        HuAudFXPlay(MSM_SE_CMN_04);
        selectionResult = -1;
    } else {
        continue;
    }
    break;
    }
    fn_1_42D4();
    fn_1_CEA4(fn_1_EE70);
    if (selectionResult == -1) {
        if (fn_1_26364_for_276E0_mdmicdll_text_272D4c()) continue;
        selectionResult = -1;
    }
    break;
    }
    if (selectionResult != -1) {
        fn_1_CEA4(fn_1_F4D8);
    }
    return selectionResult;
}

/* Human selection distinguishes all-ready, player-one unconfirmed and other players still
 * choosing. */
s32 fn_1_29184(void)
{
    s16 player;
    s16 count;
    player = 0;
    count = 0;
    while (player < 4) {
        if (lbl_1_bss_9A4[player].characterChoiceState == 1) {
            count += 1;
        }
        player += 1;
    }
    if (lbl_1_bss_9DC.participantCount <= count) {
        return 0;
    }
    if (lbl_1_bss_9A4[0].characterChoiceState == 0) {
        return 1;
    }
    return -1;
}

/* Human character input uses this readiness check to decide whether to advance, allow back or keep
 * waiting. */
static inline s16 fn_1_32904_inlined_mdmicdll_text_29224c(void)
{
    s16 chosenPlayerCount;
    s16 playerSlot;

    for (playerSlot = 0, chosenPlayerCount = 0; playerSlot < 4; playerSlot++) {
        if (lbl_1_bss_9A4[playerSlot].characterChoiceState == 1) {
            chosenPlayerCount++;
        }
    }
    if (lbl_1_bss_9DC.participantCount <= chosenPlayerCount) {
        return 0;
    }
    if (lbl_1_bss_9A4[0].characterChoiceState == 0) {
        return 1;
    }
    return -1;
}

/* The human-character flow asks whether to leave Mic Mode, treating cancel as staying. */
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_29224c(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* Human-character setup runs simultaneous player input and handles backtracking and exit
 * confirmation. */
s16 fn_1_29224(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_CEA4(fn_1_1053C);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_4150(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        fn_1_5328();
        fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 14)), 0);
        fn_1_CEA4(fn_1_1018C);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 14)), 0);
            result = fn_1_32904_inlined_mdmicdll_text_29224c();
            if (!fn_1_CE6C()) {
                continue;
            }
            if (result == 0) {
                result = 0;
                break;
            }
            if (result == 1 && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_103F0);
        if (result != -1) {
            break;
        }
        if (!fn_1_2EF24_inlined_mdmicdll_text_29224c()) {
            result = -1;
            break;
        }
    }
    return result;
}

/* Computer setup locates and counts slots whose character and difficulty choices remain
 * unfinished. */
s16 fn_1_29E70(s16 *activePlayer, s16 *firstComputerPlayer, s16 *lastPlayerSlot)
{
    s16 i;
    s16 count = 0;

    *activePlayer = 0;
    *firstComputerPlayer = 0;
    *lastPlayerSlot = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1) {
            *firstComputerPlayer = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1
            && lbl_1_bss_9A4[i].characterChoiceState != 2) {
            break;
        }
    }
    *activePlayer = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1
            && lbl_1_bss_9A4[i].characterChoiceState != 2) {
            count++;
        }
    }
    return count;
}

/* The computer-setup flow asks whether to leave Mic Mode, treating cancel as staying. */
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_29FA4c(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* Computer setup runs character and difficulty choices for unfinished slots, with backtracking and
 * exit confirmation. */
s16 fn_1_29FA4(s16 unused)
{
    s16 result = 0;
    s16 activePlayer = 0;
    s16 firstActivePlayer = 0;
    s16 lastPlayerSlot = 0;
    s16 count = 0;

    count = fn_1_29E70(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_4150(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        fn_1_5328();
        if (lbl_1_bss_9A4[activePlayer].characterChoiceState == 0) {
            fn_1_3F5C(3, ((int)MESSNUM(MESS_PARTY_SETTING, 19)) - (count - 1), 0);
            fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 15)), 0);
        } else {
            fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 20)), 0);
        }
        fn_1_CEA4(fn_1_11534);
        while (TRUE) {
            HuPrcVSleep();
            if (lbl_1_bss_9A4[activePlayer].characterChoiceState == 0) {
                fn_1_3F5C(3, ((int)MESSNUM(MESS_PARTY_SETTING, 19)) - (count - 1), 0);
                fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 15)), 0);
            } else {
                fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 20)), 0);
            }
            if (!fn_1_CE6C()) {
                continue;
            }
            count = fn_1_29E70(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
            if (activePlayer > lastPlayerSlot) {
                result = 0;
                break;
            }
            if (firstActivePlayer == activePlayer
                && lbl_1_bss_9A4[activePlayer].characterChoiceState == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_119B8);
        if (result == -1) {
            if (fn_1_2EF24_inlined_mdmicdll_text_29FA4c()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

/* The character-screen process runs human and computer setup, then arranges the chosen lineup or
 * returns. */
s16 fn_1_2B590(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_CEA4(fn_1_15994);
    }
    fn_1_CEA4(fn_1_12744);
    switch (lbl_1_bss_9DC.participantCount) {
    case 0:
        result = fn_1_29FA4(0);
        break;
    case 4:
        result = fn_1_29224(0);
        break;
    default:
        do {
            HuPrcVSleep();
            result = fn_1_29224(previousStepPending);
            if (result == 0) {
                result = fn_1_29FA4(0);
                if (result == 1) {
                    previousStepPending = 1;
                    continue;
                }
            }
            break;
        } while (TRUE);
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_12D38);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_15D8C);
    }
    return result;
}

/* The human-only character process shows the roster, runs player choices and arranges the chosen
 * humans. */
s16 fn_1_2B844(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_CEA4(fn_1_15994);
    }
    fn_1_CEA4(fn_1_12744);
    result = fn_1_29224(0);
    if (result == 0) {
        fn_1_CEA4(fn_1_13870);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_15D8C);
    }
    return result;
}

/* Character-only computer setup locates and counts slots still awaiting confirmation. */
s16 fn_1_2BA88(s16 *activePlayer, s16 *firstComputerPlayer, s16 *lastPlayerSlot)
{
    s16 i;
    s16 count = 0;

    *activePlayer = 0;
    *firstComputerPlayer = 0;
    *lastPlayerSlot = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1) {
            *firstComputerPlayer = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1
            && lbl_1_bss_9A4[i].characterChoiceState != 1) {
            break;
        }
    }
    *activePlayer = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9A4[i].isComputer == 1
            && lbl_1_bss_9A4[i].characterChoiceState != 1) {
            count++;
        }
    }
    return count;
}

/* The computer-character flow asks whether to leave Mic Mode, treating cancel as staying. */
static inline s16 fn_1_2EF24_inlined_mdmicdll_text_2BBBCc(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* The character-only computer process runs remaining slot choices, with backtracking and exit
 * confirmation. */
s16 fn_1_2BBBC(s16 unused)
{
    s16 result = 0;
    s16 activePlayer = 0;
    s16 firstActivePlayer = 0;
    s16 lastPlayerSlot = 0;
    s16 count = 0;

    count = fn_1_29E70(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_4150(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        fn_1_5328();
        fn_1_3F5C(3, ((int)MESSNUM(MESS_MIC_SETTING, 13)) - (count - 1), 0);
        fn_1_3D24(3, ((int)MESSNUM(MESS_MIC_SETTING, 9)), 0);
        fn_1_CEA4(fn_1_12334);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_3F5C(3, ((int)MESSNUM(MESS_MIC_SETTING, 13)) - (count - 1), 0);
            fn_1_3D24(3, ((int)MESSNUM(MESS_MIC_SETTING, 9)), 0);
            if (!fn_1_CE6C()) {
                continue;
            }
            count = fn_1_2BA88(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
            if (activePlayer > lastPlayerSlot) {
                result = 0;
                break;
            }
            if (firstActivePlayer == activePlayer
                && lbl_1_bss_9A4[activePlayer].characterChoiceState == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_125D8);
        if (result == -1) {
            if (fn_1_2EF24_inlined_mdmicdll_text_2BBBCc()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

/* Activity character setup forces one human and three computers, then runs character choices and
 * lineup transitions. */
s16 fn_1_2CD28(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;
    s16 remainingHumanPlayers;
    s16 padNo;
    MDMIC_PLAYER_WORK *player;

    lbl_1_bss_9DC.participantCount = 1;
    remainingHumanPlayers = lbl_1_bss_9DC.participantCount;
    lbl_1_bss_9A4[0].isComputer = 0;
    remainingHumanPlayers--;
    for (padNo = 1, player = &lbl_1_bss_9A4[1]; padNo < 4; padNo++, player++) {
        if (remainingHumanPlayers > 0 && HuPadStatGet(padNo) == 0) {
            player->isComputer = 0;
            remainingHumanPlayers--;
        } else {
            player->isComputer = 1;
        }
    }

    if (returningFromPreviousStep != 0) {
        fn_1_CEA4(fn_1_15994);
    }
    fn_1_CEA4(fn_1_12744);
    switch (lbl_1_bss_9DC.participantCount) {
        case 0:
            result = fn_1_2BBBC(0);
            break;
        case 4:
            result = fn_1_29224(0);
            break;
        default:
            while (TRUE) {
                HuPrcVSleep();
                result = fn_1_29224(previousStepPending);
                if (result != 0) {
                    break;
                }
                result = fn_1_2BBBC(0);
                if (result != 1) {
                    break;
                }
                previousStepPending = 1;
            }
            break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_14E70);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_15D8C);
    }
    return result;
}

/* Selection processes show the guide and update its text only when the requested message
 * changes. */
static inline void fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(s32 messageNo)
{
    extern HUWINID lbl_1_bss_A44[4];
    if (lbl_1_data_E32[1] == -1) {
        lbl_1_data_E32[1] = 0;
        lbl_1_data_E38[2] = -1;
        fn_1_3404(lbl_1_data_E32[1]);
    }
    if (lbl_1_data_E38[2] != (u32)messageNo) {
        lbl_1_data_E38[2] = messageNo;
        fn_1_35F4(lbl_1_data_E32[1],
                           lbl_1_data_E38[2], 0);
    }
}

/* Selection processes ask whether to leave Mic Mode, treating cancel as staying. */
static inline s16 fn_1_26364_for_2D06C_mdmicdll_text_2CD28c(void)
{
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 32), 1);
    return fn_1_3C2C(2);
}

/* The mode-selection process runs its entrance, input and confirmation transitions, including exit
 * confirmation. */
s16 fn_1_2D06C(s16 returningFromPreviousStep) {
    extern void fn_1_17D90(OMOBJ *obj);
    extern void fn_1_17648(OMOBJ *obj);
    extern void fn_1_170D4(OMOBJ *obj);
    extern void fn_1_17340(OMOBJ *obj);
    extern void fn_1_17E8C(OMOBJ *obj);
    extern void fn_1_18874(OMOBJ *obj);
    s16 result = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_CEA4(fn_1_17D90);
    }
    fn_1_CEA4(fn_1_17648);
    for (;;) {
        HuPrcVSleep();
        fn_1_CEA4(fn_1_170D4);
        fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(MESSNUM(MESS_SYS_GUIDE, 10));
        fn_1_3D24(3, lbl_1_bss_9DC.modeSelection + MESSNUM(MESS_MIC_SETTING, 2), 0);
        for (;;) {
            HuPrcVSleep();
            fn_1_3D24(3, lbl_1_bss_9DC.modeSelection + MESSNUM(MESS_MIC_SETTING, 2), 0);
            if (!fn_1_CE6C()) continue;
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
            } else {
                continue;
            }
            break;
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_17340);
        if (result == -1) {
            if (fn_1_26364_for_2D06C_mdmicdll_text_2CD28c()) continue;
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_17E8C);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_18874);
    }
    return result;
}

/* Selection processes update a dialogue insertion and invalidate the cached main message when it
 * changes. */
static inline void fn_1_3F5C_mdmicdll_text_2CD28c(s16 windowNo, s32 messageNo,
                             s16 insertPosition)
{
    fn_1_39A4(windowNo);
    if ((s32)lbl_1_data_E38[1] != messageNo) {
        lbl_1_data_E38[0] = -1;
        lbl_1_data_E38[1] = messageNo;
        fn_1_36B0(lbl_1_data_E32[0], lbl_1_data_E38[1], insertPosition);
    }
}

/* The handicap process initializes or restores its layout, runs value input and handles confirm,
 * back and exit. */
s16 fn_1_2DD78(s16 returningFromPreviousStep)
{
    extern HUWINID lbl_1_bss_A44[4];
    s16 result = 0;

    if (returningFromPreviousStep == 0) {
        fn_1_CEA4(fn_1_199B4);
    } else {
        fn_1_CEA4(fn_1_19B28);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_CEA4(fn_1_19134);
        fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        fn_1_3F5C_mdmicdll_text_2CD28c(3, lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].chrSel, 0);
        fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 34)), 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_3F5C_mdmicdll_text_2CD28c(3, lbl_1_bss_9A4[lbl_1_bss_9DC.handicapPlayer].chrSel,
                                           0);
            fn_1_3D24(3, ((int)MESSNUM(MESS_PARTY_SETTING, 34)), 0);
            if (!fn_1_CE6C()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_198CC);
        if (result == -1) {
            if (fn_1_26364_for_2D06C_mdmicdll_text_2CD28c()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_1A1D8);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_1A878);
    }
    return result;
}

/* The minigame confirmation process accepts the setup, returns to mode selection or revisits player
 * order. */
s16 fn_1_2EE38(void)
{
    void fn_1_1A914(OMOBJ *obj);
    void fn_1_1AE7C(OMOBJ *obj);
    void fn_1_1B5F0(OMOBJ *obj);
    void fn_1_1BD7C(OMOBJ *obj);
    s16 result = 0;
    fn_1_CEA4(fn_1_1A914);
    HuPrcVSleep();
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 27), 1);
    if ((result = fn_1_3C2C(2)) == 0) {
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 31), 1);
        HuPrcSleep(60U);
        fn_1_51CC();
        HuPrcSleep(60U);
        fn_1_3B10();
        fn_1_CEA4(fn_1_1AE7C);
        result = 0;
    } else {
        fn_1_5328();
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 28), 1);
        if ((result = fn_1_3C2C(2)) == 0) {
            fn_1_CEA4(fn_1_1B5F0);
            result = 1;
        } else {
            fn_1_CEA4(fn_1_1BD7C);
            result = 2;
        }
    }
    return result;
}

/* The first mode confirmation process accepts the setup, returns to mode selection or revisits
 * character choice. */
s16 fn_1_2F9A4(void)
{
    void fn_1_1C2E4(OMOBJ *obj);
    void fn_1_1C308(OMOBJ *obj);
    void fn_1_1C778(OMOBJ *obj);
    void fn_1_1CC00(OMOBJ *obj);
    s16 result = 0;

    fn_1_CEA4(fn_1_1C2E4);
    HuPrcVSleep();
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 27), 1);
    if ((result = fn_1_3C2C(2)) == 0) {
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 29), 1);
        HuPrcSleep(60U);
        fn_1_51CC();
        HuPrcSleep(60U);
        fn_1_3B10();
        fn_1_CEA4(fn_1_1C308);
        result = 0;
    } else {
        fn_1_5328();
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 28), 1);
        if ((result = fn_1_3C2C(2)) == 0) {
            fn_1_CEA4(fn_1_1C778);
            result = 1;
        } else {
            fn_1_CEA4(fn_1_1CC00);
            result = 2;
        }
    }
    return result;
}

/* The activity confirmation process accepts the setup, returns to mode selection or revisits the
 * activity choice. */
s16 fn_1_30510(void)
{
    void fn_1_1CC24(OMOBJ *obj);
    void fn_1_1CC48(OMOBJ *obj);
    void fn_1_1D3B8(OMOBJ *obj);
    void fn_1_1DB40(OMOBJ *obj);
    s16 result = 0;

    fn_1_CEA4(fn_1_1CC24);
    HuPrcVSleep();
    fn_1_5328();
    fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 27), 1);
    if ((result = fn_1_3C2C(2)) == 0) {
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 30), 1);
        HuPrcSleep(60U);
        fn_1_51CC();
        HuPrcSleep(60U);
        fn_1_3B10();
        fn_1_CEA4(fn_1_1CC48);
        result = 0;
    } else {
        fn_1_5328();
        fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 28), 1);
        if ((result = fn_1_3C2C(2)) == 0) {
            fn_1_CEA4(fn_1_1D3B8);
            result = 1;
        } else {
            fn_1_CEA4(fn_1_1DB40);
            result = 2;
        }
    }
    return result;
}

/* Game entry saves player choices, starts archive loading and runs character, sound and camera
 * departures. */
void fn_1_3107C(void)
{
    fn_1_824();
    fn_1_758();
    HuPrcSleep(10);
    fn_1_69D8();
    fn_1_5918();
    HuPrcSleep(10);
    fn_1_3C9F8();
    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, MDMIC_ENTRY_MUSIC_FADE_SPEED);
        lbl_1_data_0 = -1;
    }
        HuAudFXPlay(MSM_SE_MENU_20);
    HuPrcSleep(90);
    lbl_1_bss_9F4.callback = fn_1_258B4;
    HuPrcSleep(60);
}

/* The activity-choice process runs card selection, rejects locked activities and handles back or
 * exit requests. */
s16 fn_1_31770(s16 returningToSelection)
{
    void fn_1_2299C(OMOBJ *obj);
    void fn_1_219F0(OMOBJ *obj);
    void fn_1_2124C(OMOBJ *obj);
    void fn_1_216D4(OMOBJ *obj);
    void fn_1_22D54(OMOBJ *obj);
    void fn_1_23B18(OMOBJ *obj);
    s16 result = 0;

    if (returningToSelection != 0) {
        fn_1_CEA4(fn_1_2299C);
    }
    fn_1_CEA4(fn_1_219F0);
    for (;;) {
        HuPrcVSleep();
        fn_1_CEA4(fn_1_2124C);
        fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        if (lbl_1_data_CA2[lbl_1_bss_9DC.gameSelection] == 1) {
            fn_1_3D24(3, lbl_1_bss_9DC.gameSelection + MESSNUM(MESS_MIC_SETTING, 17), 0);
        } else {
            fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 38), 0);
        }
        for (;;) {
            HuPrcVSleep();
            if (lbl_1_data_CA2[lbl_1_bss_9DC.gameSelection] == 1) {
                fn_1_3D24(3, lbl_1_bss_9DC.gameSelection + MESSNUM(MESS_MIC_SETTING, 17), 0);
            } else {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 38), 0);
            }
            if (!fn_1_CE6C()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                if (lbl_1_data_CA2[lbl_1_bss_9DC.gameSelection] == 0) {
                    HuAudFXPlay(MSM_SE_CMN_05);
                    continue;
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_216D4);
        if (result == -1) {
            if (fn_1_26364_for_2D06C_mdmicdll_text_2CD28c()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_22D54);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_23B18);
    }
    return result;
}

/* The minigame-choice process runs card selection, rejects locked games and handles back or exit
 * requests. */
s16 fn_1_32934(s16 returningToSelection)
{
    void fn_1_2299C(OMOBJ *obj);
    void fn_1_219F0(OMOBJ *obj);
    void fn_1_2124C(OMOBJ *obj);
    void fn_1_216D4(OMOBJ *obj);
    void fn_1_22D54(OMOBJ *obj);
    void fn_1_23B18(OMOBJ *obj);
    void fn_1_5280(OMOBJ *obj);
    extern OMOBJ *lbl_1_bss_24;
    s16 result = 0;

    if (returningToSelection != 0) {
        fn_1_CEA4(fn_1_2299C);
    }
    fn_1_CEA4(fn_1_219F0);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_CEA4(fn_1_2124C);
        fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(MESSNUM(MESS_SYS_GUIDE, 10));
        if (lbl_1_data_CA8[lbl_1_bss_9DC.gameSelection] == 0) {
            fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 25), 0);
        } else {
            fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 34), 0);
        }
        for (;;) {
            HuPrcVSleep();
            if (lbl_1_data_CA8[lbl_1_bss_9DC.gameSelection] == 0) {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 25), 0);
            } else {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 34), 0);
            }
            if (!fn_1_CE6C()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                if (lbl_1_data_CA8[lbl_1_bss_9DC.gameSelection] == 0) {
                    HuAudFXPlay(MSM_SE_CMN_05);
                    continue;
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_216D4);
        if (result == -1) {
            if (fn_1_26364_for_2D06C_mdmicdll_text_2CD28c()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_22D54);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_23B18);
    }
    return result;
}

/* The player-order process runs solo-side selection when multiple humans play and handles confirm,
 * back and exit. */
s16 fn_1_33AD8(s16 returningToSelection)
{
    void fn_1_1DF74(OMOBJ *obj);
    void fn_1_1E9D8(OMOBJ *obj);
    void fn_1_1E248(OMOBJ *obj);
    void fn_1_1E1B0(OMOBJ *obj);
    void fn_1_1F070(OMOBJ *obj);
    void fn_1_1F864(OMOBJ *obj);
    extern HUWINID lbl_1_bss_A44[4];
    s16 result = 0;

    if (returningToSelection == 0) {
        OMOBJ *obj = lbl_1_bss_24;
        lbl_1_bss_24->mtnId[0] = 0;
        obj->work[0] = 0;
        obj->work[1] = 0;
        obj->work[2] = 0;
        obj->work[3] = 0;
        obj->objFunc = fn_1_1E248;
        while (lbl_1_bss_24->mtnId[0] == 0) HuPrcVSleep();
    } else {
        fn_1_CEA4(fn_1_1E9D8);
    }
    for (;;) {
        HuPrcVSleep();
        fn_1_4150_for_2D06C_mdmicdll_text_2CD28c(((int)MESSNUM(MESS_SYS_GUIDE, 10)));
        if (lbl_1_bss_9DC.participantCount != 1) {
            fn_1_CEA4(fn_1_1DF74);
            fn_1_5328();
            fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 15), 0);
        } else {
            fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 16), 0);
        }
        for (;;) {
            HuPrcVSleep();
            if (lbl_1_bss_9DC.participantCount != 1) {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 15), 0);
                if (!fn_1_CE6C()) continue;
            } else {
                fn_1_3D24(3, MESSNUM(MESS_MIC_SETTING, 16), 0);
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_CEA4(fn_1_1E1B0);
        if (result == -1) {
            if (fn_1_26364_for_2D06C_mdmicdll_text_2CD28c()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_CEA4(fn_1_1F070);
    } else if (result == 1) {
        fn_1_CEA4(fn_1_1F864);
    }
    return result;
}

/* The main setup process restores the host and shows the mode-selection introduction. */
static inline s32 fn_1_272D4_for_34C78_mdmicdll_text_2CD28c(void)
{
    extern void fn_1_5A90(OMOBJ *);
    Point3d currentPosition;
    Point3d startPosition;
    OMOBJ *movingObject;

    Hu3DModelPosGet(*lbl_1_bss_C[0]->mdlId, &startPosition);
    if (startPosition.x > -200.0f) {
        movingObject = lbl_1_bss_C[0];
        Hu3DModelPosGet(*movingObject->mdlId, &currentPosition);
        if (currentPosition.x < -100.0f) {
            /* Keep the current motion while the model remains left of this point. */
        } else {
            omObjGetWork(movingObject, u32)[0] = 0;
            movingObject->objFunc = fn_1_5A90;
            Hu3DMotionShiftSet(*movingObject->mdlId, movingObject->mtnId[1],
                0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        }
        fn_1_3B10();
        HuPrcSleep(60);
    }
    fn_1_5328();
    fn_1_39A4(3);
    if (lbl_1_data_E38[0] != MESSNUM(MESS_MIC_SETTING, 1)) {
        lbl_1_data_E38[0] = MESSNUM(MESS_MIC_SETTING, 1);
        lbl_1_data_E38[1] = -1;
        fn_1_35F4(lbl_1_data_E32[0], lbl_1_data_E38[0], 1);
    }
    if (lbl_1_data_E32[0] != -1) {
        fn_1_34E4(lbl_1_data_E32[0]);
    }
    return 1;
}

/* The menu child process runs the mode-specific setup sequence and returns when game entry or exit
 * is ready. */
s16 fn_1_34C78(s16 mode) {
    extern OMOBJ *lbl_1_bss_24;
    extern s16 lbl_1_bss_28;
    extern HUWINID lbl_1_bss_A44[4];
    extern void fn_1_13870(OMOBJ *), fn_1_12D38(OMOBJ *);
    extern s16 fn_1_2666C(void), fn_1_276E0(s16), fn_1_31770(s16), fn_1_32934(s16), fn_1_33AD8(s16);
    extern void fn_1_5280(OMOBJ *), fn_1_5A90(OMOBJ *);

    HuVecF position;
    s16 introResult;

    s16 messageWindowNo;
    s16 windowNo2;
    s16 windowNo3;

    s16 state = 0;
    s16 result = 0;
    s16 returningFromPreviousStep = 0;
    u32 messageId;

    HuPrcSleep(5);
    if (lbl_1_bss_28 == 1) {
        fn_1_25B50();
    }
    lbl_1_data_0 = HuAudSStreamPlay(MSM_STREAM_W01);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_28 == 0) {
        result = fn_1_2666C();
        if (result == -1) {
            return 0;
        }
        result = 0;
        state = 0;
        returningFromPreviousStep = 0;
        goto mainStateLoop;
    }
    if (lbl_1_bss_9DC.modeSelection == 0) {
        state = 1;
        returningFromPreviousStep = 0;
        lbl_1_bss_28 = 0;
    } else if (lbl_1_bss_9DC.modeSelection == 1) {
        state = MDMIC_STEP_ACTIVITY_CHOICE;
        returningFromPreviousStep = 0;
    } else {
        state = MDMIC_STEP_MINIGAME_CHOICE;
        returningFromPreviousStep = 0;
    }
mainStateLoop:
    HuPrcVSleep();
    switch (state) {
    case 0:
        fn_1_272D4_for_34C78_mdmicdll_text_2CD28c();
        state = 1;
        returningFromPreviousStep = 0;
        break;
    case 1:
        result = fn_1_2D06C(returningFromPreviousStep);
        if (result == 1) {
            state = 0;
            returningFromPreviousStep = 1;
        } else if (result == 0) {
            state = (lbl_1_bss_9DC.modeSelection * MDMIC_MODE_STEP_STRIDE) +
                    MDMIC_STEP_QUIZ_PLAYER_COUNT;
            returningFromPreviousStep = 0;
        }
        break;
    case 2:
        fn_1_3107C();
        return 2;
    case MDMIC_STEP_QUIZ_PLAYER_COUNT:
        result = fn_1_276E0(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_QUIZ_CHARACTERS;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 1;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_QUIZ_CHARACTERS:
        result =
            fn_1_2B844(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_QUIZ_CONFIRM;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = MDMIC_STEP_QUIZ_PLAYER_COUNT;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_QUIZ_CONFIRM:
        result = fn_1_2F9A4();
        if (result == 0) {
            state = 2;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 0;
            returningFromPreviousStep = 0;
        } else if (result == 2) {
            state = MDMIC_STEP_QUIZ_CHARACTERS;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_ACTIVITY_CHARACTERS:
        result = fn_1_2CD28(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_ACTIVITY_CHOICE;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 1;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_ACTIVITY_CHOICE:
        result = fn_1_31770(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_ACTIVITY_CONFIRM;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = MDMIC_STEP_ACTIVITY_CHARACTERS;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_ACTIVITY_CONFIRM:
        result = fn_1_30510();
        if (result == 0) {
            state = 2;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 0;
            returningFromPreviousStep = 0;
        } else if (result == 2) {
            state = MDMIC_STEP_ACTIVITY_CHOICE;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_MINIGAME_PLAYER_COUNT:
        result = fn_1_276E0(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_MINIGAME_CHARACTERS;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 1;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_MINIGAME_CHARACTERS:
        result = fn_1_2B590(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_MINIGAME_CHOICE;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = MDMIC_STEP_MINIGAME_PLAYER_COUNT;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_MINIGAME_CHOICE:
        result = fn_1_32934(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_MINIGAME_ORDER;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = MDMIC_STEP_MINIGAME_CHARACTERS;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_MINIGAME_ORDER:
        result = fn_1_33AD8(returningFromPreviousStep);
        if (result == 0) {
            state = MDMIC_STEP_MINIGAME_CONFIRM;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = MDMIC_STEP_MINIGAME_CHOICE;
            returningFromPreviousStep = 1;
        }
        break;
    case MDMIC_STEP_MINIGAME_CONFIRM:
        result = fn_1_2EE38();
        if (result == 0) {
            state = 2;
            returningFromPreviousStep = 0;
        } else if (result == 1) {
            state = 0;
            returningFromPreviousStep = 0;
        } else if (result == 2) {
            state = MDMIC_STEP_MINIGAME_ORDER;
            returningFromPreviousStep = 1;
        }
        break;
    }
    if (result == -1) {
        return 0;
    }
    goto mainStateLoop;
}
