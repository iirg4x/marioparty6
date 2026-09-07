#ifndef MDRESULT_H
#define MDRESULT_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"
#include "game/window.h"
#include "messdir_enum.h"

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

typedef struct MdResultCameraWork_s MDRESULT_CAMERA_WORK;
typedef void (*MDRESULT_CAMERA_CALLBACK)(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera);

typedef struct MdResultMessageNumbers_s {
    s32 values[2];
} MDRESULT_MESSAGE_NUMBERS;

typedef struct MdResultFxNumbers_s {
    s32 values[16];
} MDRESULT_FX_NUMBERS;

typedef struct MdResultS16Table22_s {
    s16 values[2][11];
} MDRESULT_S16_TABLE_22;

typedef struct MdResultByteTable110_s {
    s8 values[55][2];
} MDRESULT_BYTE_TABLE_110;

typedef struct MdResultU8Table12_s {
    u8 values[12];
} MDRESULT_U8_TABLE_12;

typedef struct MdResultFloatTable11_s {
    float values[11];
} MDRESULT_FLOAT_TABLE_11;

typedef struct MdResultFloatTable8_s {
    float values[8];
} MDRESULT_FLOAT_TABLE_8;

typedef struct MdResultColorTable8_s {
    GXColor values[8];
} MDRESULT_COLOR_TABLE_8;

typedef struct MdResultColorTable7_s {
    GXColor values[7];
} MDRESULT_COLOR_TABLE_7;

typedef struct MdResultColorStep_s {
    s16 tick;
    s16 paletteIndex;
} MDRESULT_COLOR_STEP;

typedef struct MdResultColorWork_s {
    u8 current[4];
    u8 target[4];
} MDRESULT_COLOR_WORK;

typedef struct MdResultBss1278Work_s {
    s16 values[4];
    s32 messages[6];
} MDRESULT_BSS_1278_WORK;

typedef struct MdResultVectorPair_s {
    HuVecF values[2];
} MDRESULT_VECTOR_PAIR;

typedef struct MdResultCharacterWork_s {
    s16 unk_00;
    s16 unk_02;
    s16 unk_04;
    s16 unk_06;
    s16 character;
    s16 unk_0A;
} MDRESULT_CHARACTER_WORK;

typedef struct MdResultSpriteInfo_s {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDRESULT_SPRITE_INFO;

typedef struct MdResultPlayerSpriteInfo_s {
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDRESULT_PLAYER_SPRITE_INFO;

typedef struct MdResultPlayerSpriteTable_s {
    MDRESULT_PLAYER_SPRITE_INFO values[14];
} MDRESULT_PLAYER_SPRITE_TABLE;

typedef struct MdResultPlayerSpriteTable15_s {
    MDRESULT_PLAYER_SPRITE_INFO values[15];
} MDRESULT_PLAYER_SPRITE_TABLE_15;

typedef struct MdResultPlayerSpriteTable17_s {
    MDRESULT_PLAYER_SPRITE_INFO values[17];
} MDRESULT_PLAYER_SPRITE_TABLE_17;

typedef struct MdResultGraphRecord_s {
    s16 bank;
    s16 unk_02;
    s32 message;
} MDRESULT_GRAPH_RECORD;

typedef struct MdResultGraphTable_s {
    MDRESULT_GRAPH_RECORD values[12];
} MDRESULT_GRAPH_TABLE;

typedef struct MdResultPlayerSpriteWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    HUSPRID sprites[14];
    u32 unk_24;
} MDRESULT_PLAYER_SPRITE_WORK;


typedef struct MdResultEmitterWork_s {
    s16 active;
    float timer;
    float scale;
    void *data;
} MDRESULT_EMITTER_WORK;

typedef struct MdResultEmitterVertex_s {
    HuVecF position;
    float weight;
} MDRESULT_EMITTER_VERTEX;

typedef struct MdResultPlayerWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    float values[6];
    HUSPR_GROUPID secondGroup;
    s16 state[2];
    HUWINID winId;
} MDRESULT_PLAYER_WORK;

typedef struct MdResultPlayerAltWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    HUSPRID sprites[12];
    HUSPR_GROUPID secondGroup;
    HUSPRID secondSprites[2];
    HUWINID winId;
} MDRESULT_PLAYER_ALT_WORK;

typedef struct MdResultTrailWork_s {
    HuVecF *points;
    HuVecF base;
    HuVecF velocity;
    s16 modelIndex;
    s16 state;
    s16 pointCount;
    s16 delay;
    GXColor color;
    s16 unk_28;
    s16 unk_2A;
} MDRESULT_TRAIL_WORK;

typedef struct MdResultScoreWork_s {
    s16 playerIndex;
    s16 teamIndex;
    s16 rank;
    s16 star;
    s16 coin;
    s16 values[16];
} MDRESULT_SCORE_WORK;

typedef struct MdResultGroupWork_s {
    HUSPR_GROUPID group;
    HUSPRID sprites[3];
} MDRESULT_GROUP_WORK;

typedef struct MdResultStateWork_s {
    s16 state;
    float time;
    float delay;
    s16 score;
} MDRESULT_STATE_WORK;

typedef struct MdResultMoveWork_s {
    s16 state;
    float time;
    float duration;
    HuVecF current;
    HuVecF middle;
    HuVecF target;
    float values[4];
} MDRESULT_MOVE_WORK;

typedef struct MdResultModelEffectWork_s {
    s16 state;
    float time;
    float angle;
    float unk_0C;
    float unk_10;
    float unk_14;
    float unk_18;
    float unk_1C;
    float unk_20;
    float unk_24;
    float unk_28;
    float unk_2C;
    float unk_30;
    float unk_34;
    float unk_38;
    float unk_3C;
} MDRESULT_MODEL_EFFECT_WORK;

struct MdResultCameraWork_s {
    OMOBJ *obj;
    HuVecF center;
    HuVecF targetCenter;
    HuVecF rot;
    HuVecF targetRot;
    float zoom;
    float targetZoom;
    MDRESULT_CAMERA_CALLBACK callback;
    s16 unk_40;
    s16 mode;
    float unk_44;
};

#endif
