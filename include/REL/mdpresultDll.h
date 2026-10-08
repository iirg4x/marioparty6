#ifndef MDRESULT_H
#define MDRESULT_H

/* Shared constants and work records for the minigame result overlay. */

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
    s32 values[2]; /* Message IDs used by the result window sound callback. */
} MDRESULT_MESSAGE_NUMBERS;

typedef struct MdResultFxNumbers_s {
    s32 values[16]; /* Sound effect IDs indexed by the result-window slot. */
} MDRESULT_FX_NUMBERS;

typedef struct MdResultS16Table22_s {
    s16 values[2][11]; /* Two voice tables indexed by character ID. */
} MDRESULT_S16_TABLE_22;

typedef struct MdResultByteTable110_s {
    s8 values[55][2]; /* Unordered character pairs used by the special result message lookup. */
} MDRESULT_BYTE_TABLE_110;

typedef struct MdResultU8Table12_s {
    u8 values[12]; /* Byte values used by the corresponding result effect table. */
} MDRESULT_U8_TABLE_12;

typedef struct MdResultFloatTable11_s {
    float values[11]; /* Scalar values used by the corresponding result animation. */
} MDRESULT_FLOAT_TABLE_11;

typedef struct MdResultFloatTable8_s {
    float values[8]; /* Scalar values used by the corresponding result animation. */
} MDRESULT_FLOAT_TABLE_8;

typedef struct MdResultColorTable8_s {
    GXColor values[8]; /* Eight RGBA colors used by the result effects. */
} MDRESULT_COLOR_TABLE_8;

typedef struct MdResultColorTable7_s {
    GXColor values[7]; /* Seven RGBA colors used by the result particle effect. */
} MDRESULT_COLOR_TABLE_7;

typedef struct MdResultColorStep_s {
    s16 tick; /* Frame at which this palette step is reached. */
    s16 paletteIndex; /* Color-table entry applied at this step. */
} MDRESULT_COLOR_STEP;

typedef struct MdResultColorWork_s {
    u8 current[4]; /* Current RGBA color bytes. */
    u8 target[4]; /* Destination RGBA color bytes. */
} MDRESULT_COLOR_WORK;

typedef struct MdResultBss1278Work_s {
    s16 values[4]; /* Board number, turn limit, bonus-star flag, and team-mode flag. */
    s32 messages[6]; /* Four character-name messages followed by two pair messages. */
} MDRESULT_BSS_1278_WORK;

typedef struct MdResultVectorPair_s {
    HuVecF values[2]; /* The two edges of one rendered trail segment. */
} MDRESULT_VECTOR_PAIR;

typedef struct MdResultCharacterWork_s {
    s16 playerIndex; /* Player slot whose character is shown. */
    s16 groupNo; /* Group number from GwPlayerConf. */
    s16 playerType; /* Human or computer type from GwPlayerConf. */
    s16 computerDifficulty; /* Computer difficulty from GwPlayerConf. */
    s16 character; /* Character ID used for the displayed model and voice. */
    s16 padNo; /* Controller slot used for the result prompt. */
} MDRESULT_CHARACTER_WORK;

typedef struct MdResultSpriteInfo_s {
    s16 groupNo; /* Sprite group receiving this sprite. */
    s16 memberNo; /* Member slot within the group. */
    s16 animNo; /* Animation resource index. */
    s16 priority; /* Draw priority before the screen-wide offset. */
    s16 bank; /* Initial sprite bank. */
    HuVec2f pos; /* Screen position in pixels. */
    HuVec2f scale; /* Horizontal and vertical sprite scale. */
    float zRot; /* Rotation around the screen Z axis, in degrees. */
} MDRESULT_SPRITE_INFO;

typedef struct MdResultPlayerSpriteInfo_s {
    s16 animNo; /* Animation resource index. */
    s16 priority; /* Draw priority before the screen-wide offset. */
    s16 bank; /* Initial sprite bank. */
    HuVec2f pos; /* Screen position in pixels. */
    HuVec2f scale; /* Horizontal and vertical sprite scale. */
    float zRot; /* Rotation around the screen Z axis, in degrees. */
} MDRESULT_PLAYER_SPRITE_INFO;

typedef struct MdResultPlayerSpriteTable_s {
    MDRESULT_PLAYER_SPRITE_INFO values[14]; /* Sprite descriptions for one player result. */
} MDRESULT_PLAYER_SPRITE_TABLE;

typedef struct MdResultPlayerSpriteTable15_s {
    MDRESULT_PLAYER_SPRITE_INFO values[15]; /* Sprite descriptions for one player result variant. */
} MDRESULT_PLAYER_SPRITE_TABLE_15;

typedef struct MdResultPlayerSpriteTable17_s {
    MDRESULT_PLAYER_SPRITE_INFO values[17]; /* Sprite descriptions for one player result variant. */
} MDRESULT_PLAYER_SPRITE_TABLE_17;

typedef struct MdResultGraphRecord_s {
    s16 bank; /* Sprite bank containing the graph label. */
    s16 reserved; /* Zero in every graph record in this table. */
    s32 message; /* Message resource displayed for this statistic. */
} MDRESULT_GRAPH_RECORD;

typedef struct MdResultGraphTable_s {
    MDRESULT_GRAPH_RECORD values[12]; /* Twelve graph labels and their message resources. */
} MDRESULT_GRAPH_TABLE;

typedef struct MdResultPlayerSpriteWork_s {
    HU3D_MODELID models[3]; /* Character and result-scene model IDs. */
    HUSPR_GROUPID group; /* Main player-result sprite group. */
    HUSPRID sprites[14]; /* Individual score and label sprites. */
    u32 reservedFlags; /* Preserved per-player flags; no use is visible here. */
} MDRESULT_PLAYER_SPRITE_WORK;

typedef struct MdResultEmitterWork_s {
    s16 active; /* Nonzero while this emitter is producing particles. */
    float timer; /* Frames elapsed in the current emission. */
    float scale; /* Current particle scale. */
    void *data; /* Emitter-specific particle state. */
} MDRESULT_EMITTER_WORK;

typedef struct MdResultEmitterVertex_s {
    HuVecF position; /* Vertex position in scene coordinates. */
    float weight; /* Contribution used when blending emitter vertices. */
} MDRESULT_EMITTER_VERTEX;

typedef struct MdResultPlayerWork_s {
    HU3D_MODELID models[3]; /* Models used for this player's result display. */
    HUSPR_GROUPID group; /* Main sprite group. */
    float values[6]; /* Per-player display values used during score presentation. */
    HUSPR_GROUPID secondGroup; /* Additional sprite group used by this display. */
    s16 state[2]; /* Result-display state for the two presentation stages. */
    HUWINID winId; /* Message window associated with this player display. */
} MDRESULT_PLAYER_WORK;

typedef struct MdResultPlayerAltWork_s {
    HU3D_MODELID models[3]; /* Models used for this player's alternate display. */
    HUSPR_GROUPID group; /* Main sprite group. */
    HUSPRID sprites[12]; /* Sprite IDs in the main group. */
    HUSPR_GROUPID secondGroup; /* Additional sprite group. */
    HUSPRID secondSprites[2]; /* Sprite IDs in the additional group. */
    HUWINID winId; /* Message window associated with this display. */
} MDRESULT_PLAYER_ALT_WORK;

typedef struct MdResultTrailWork_s {
    HuVecF *points; /* Ordered centerline points used to build the trail strip. */
    HuVecF base; /* Offset added while trailing points follow the head. */
    HuVecF velocity; /* Direction and speed for a moving trail head. */
    s16 modelIndex; /* Index into the trail model array. */
    s16 state; /* 1 fades the trail in; 0 fades it out. */
    s16 pointCount; /* Number of allocated trail points. */
    s16 delay; /* Half-width of the rendered trail, in scene units. */
    GXColor color; /* Trail RGBA color. */
    s16 moving; /* 0 uses a fading stationary trail; 1 advances its head. */
    s16 reserved; /* Unused by the visible trail routines. */
} MDRESULT_TRAIL_WORK;

typedef struct MdResultScoreWork_s {
    s16 playerIndex; /* First player represented by this result row. */
    s16 teamIndex; /* Team index, or zero for individual results. */
    s16 rank; /* Placement in the result screen. */
    s16 star; /* Star total shown for this row. */
    s16 coin; /* Coin total shown for this row. */
    s16 values[16]; /* Board statistics, with handicap in slot 15. */
} MDRESULT_SCORE_WORK;

typedef struct MdResultGroupWork_s {
    HUSPR_GROUPID group; /* Sprite group containing the three displayed digits. */
    HUSPRID sprites[3]; /* Sprite IDs for the hundreds, tens, and ones digits. */
} MDRESULT_GROUP_WORK;

typedef struct MdResultStateWork_s {
    s16 state; /* Current phase of a result-screen sequence. */
    float time; /* Frames elapsed in the current phase. */
    float delay; /* Frames to wait before advancing the phase. */
    s16 score; /* Score value used by the sequence. */
} MDRESULT_STATE_WORK;

typedef struct MdResultMoveWork_s {
    s16 state; /* Movement phase. */
    float time; /* Frames elapsed in the current movement. */
    float duration; /* Total movement duration in frames. */
    HuVecF current; /* Starting or current position. */
    HuVecF middle; /* Intermediate control point or position. */
    HuVecF target; /* Destination position. */
    float values[4]; /* Motion-specific speed, angle, and phase values. */
} MDRESULT_MOVE_WORK;

typedef struct MdResultModelEffectWork_s {
    s16 state; /* Travel direction selected for the effect model. */
    float time; /* Frames elapsed in the vertical bobbing cycle. */
    float angle; /* Random frame count before the next bobbing cycle. */
    float rotationSpeedX; /* Per-frame X rotation change, in degrees. */
    float rotationSpeedY; /* Per-frame Y rotation change, in degrees. */
    float rotationSpeedZ; /* Per-frame Z rotation change, in degrees. */
    float horizontalSpeed; /* X movement per frame. */
    float verticalDrift; /* Y movement per frame, plus the shared drift value. */
    float depthSpeed; /* Z movement per frame. */
    float horizontalLimit; /* X boundary that restarts the model's travel. */
    float reservedFloat28; /* Unused by the visible effect routines. */
    float reservedFloat2C; /* Unused by the visible effect routines. */
    float reservedFloat30; /* Unused by the visible effect routines. */
    float reservedFloat34; /* Unused by the visible effect routines. */
    float reservedFloat38; /* Unused by the visible effect routines. */
    float reservedFloat3C; /* Unused by the visible effect routines. */
} MDRESULT_MODEL_EFFECT_WORK;

struct MdResultCameraWork_s {
    OMOBJ *obj; /* Result-camera object updated by its callback. */
    HuVecF center; /* Current camera look-at point in world units. */
    HuVecF targetCenter; /* Look-at point approached by the camera. */
    HuVecF rot; /* Current camera rotation in radians. */
    HuVecF targetRot; /* Rotation approached by the camera, in radians. */
    float zoom; /* Current camera distance/zoom value. */
    float targetZoom; /* Camera distance/zoom value approached over time. */
    MDRESULT_CAMERA_CALLBACK callback; /* Active camera motion callback. */
    s16 reserved; /* Unused by the visible camera routines. */
    s16 cameraMode; /* Mode selected by the result sequence for camera motion. */
    float reservedParam; /* Unused by the visible camera routines. */
};

#endif
