#include "dolphin/math.h"
#include "game/board/masu.h"
#include "game/board/audio.h"
#include "game/board/branch.h"
#include "game/board/capsule.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/tutorial.h"
#include "game/board/camera.h"
#include "game/board/effect.h"
#include "game/board/window.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/hsfload.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/sprite.h"

#include "humath.h"
#include "messdir_enum.h"
#include "float.h"
#include "string.h"

#define M_PI 3.141592653589793

#define CAPSULE_OBJ_COLOR_MAX 128

enum {
    CAPSULE_BOMHEI = 24,
    CAPSULE_HONE = 30,
    CAPSULE_KETTOU = 41,
    CAPSULE_KOOPA = 43,
    CAPSULE_MAX = 53,
};

enum {
    CAPSULE_DATA_KINOKO = 0,
    CAPSULE_DATA_S_KINOKO = 1,
    CAPSULE_DATA_P_KINOKO = 2,
    CAPSULE_DATA_M_KINOKO = 3,
    CAPSULE_DATA_KILLER = 4,
    CAPSULE_DATA_DOKAN = 5,
    CAPSULE_DATA_HANACHAN = 6,
    CAPSULE_DATA_N_KINOKO = 7,
    CAPSULE_DATA_TOGEZO = 8,
    CAPSULE_DATA_KURIBO = 9,
    CAPSULE_DATA_PAKKUN = 10,
    CAPSULE_DATA_JANGO = 11,
    CAPSULE_DATA_KOKAMEKKU = 12,
    CAPSULE_DATA_KAMEKKU = 13,
    CAPSULE_DATA_THROWMAN = 14,
    CAPSULE_DATA_BOBLE = 15,
    CAPSULE_DATA_BIRIQ = 16,
    CAPSULE_DATA_TUMUJIKUN = 17,
    CAPSULE_DATA_DOSSUN = 18,
    CAPSULE_DATA_BOMUHEI = 19,
    CAPSULE_DATA_PATAPATA = 20,
    CAPSULE_DATA_HONE = 21,
    CAPSULE_DATA_LIGHT = 22,
    CAPSULE_DATA_BORDER_0 = 24,
    CAPSULE_DATA_BORDER_1 = 25,
    CAPSULE_DATA_BORDER_2 = 26,
    CAPSULE_DATA_BORDER_3 = 27,
    CAPSULE_DATA_OBJ_TYPE_0 = 28,
    CAPSULE_DATA_OBJ_TYPE_1 = 29,
    CAPSULE_DATA_OBJ_TYPE_2 = 30,
    CAPSULE_DATA_OBJ_TYPE_3 = 31,
    CAPSULE_DATA_OBJ_TYPE_4 = 32,
    CAPSULE_DATA_ENTRY_33 = 33,
    CAPSULE_DATA_YAMERU = 34,
    CAPSULE_DATA_HILITE_0 = 49,
    CAPSULE_DATA_HILITE_1 = 50,
    CAPSULE_DATA_HILITE_2 = 51,
    CAPSULE_DATA_INIT_MODEL_0 = 61,
    CAPSULE_DATA_INIT_MODEL_1 = 67,
    CAPSULE_DATA_LIST_0 = 74,
    CAPSULE_DATA_LIST_1 = 75,
    CAPSULE_DATA_LIST_2 = 76,
    CAPSULE_DATA_LIST_3 = 77,
    CAPSULE_DATA_LIST_4 = 78,
    CAPSULE_DATA_LIST_5 = 79,
    CAPSULE_DATA_LIST_6 = 80,
    CAPSULE_DATA_LIST_7 = 81,
    CAPSULE_DATA_LIST_8 = 82,
    CAPSULE_DATA_LIST_9 = 83,
    CAPSULE_DATA_LIST_10 = 84,
    BOARD_DATA_DICE = 23,
};

enum {
    CAPSULE_MESSAGE_KINOKO = 0,
    CAPSULE_MESSAGE_S_KINOKO = 1,
    CAPSULE_MESSAGE_P_KINOKO = 2,
    CAPSULE_MESSAGE_M_KINOKO = 3,
    CAPSULE_MESSAGE_KILLER = 4,
    CAPSULE_MESSAGE_DOKAN = 5,
    CAPSULE_MESSAGE_HANACHAN = 6,
    CAPSULE_MESSAGE_N_KINOKO = 7,
    CAPSULE_MESSAGE_TOGEZO = 8,
    CAPSULE_MESSAGE_KURIBO = 9,
    CAPSULE_MESSAGE_PAKKUN = 10,
    CAPSULE_MESSAGE_JANGO = 11,
    CAPSULE_MESSAGE_PATAPATA = 12,
    CAPSULE_MESSAGE_KOKAMEKKU = 13,
    CAPSULE_MESSAGE_KAMEKKU = 14,
    CAPSULE_MESSAGE_THROWMAN = 15,
    CAPSULE_MESSAGE_BOBLE = 16,
    CAPSULE_MESSAGE_BIRIQ = 17,
    CAPSULE_MESSAGE_TUMUJIKUN = 18,
    CAPSULE_MESSAGE_DOSSUN = 19,
    CAPSULE_MESSAGE_BOMUHEI = 20,
    CAPSULE_MESSAGE_HONE = 21,
    CAPSULE_MESSAGE_LIGHT = 22,
    CAPSULE_MESSAGE_TARU = 23,
    CAPSULE_MESSAGE_KILLER_MOVE = 24,
    CAPSULE_MESSAGE_KETTOU = 25,
    CAPSULE_MESSAGE_MIRACLE = 26,
    CAPSULE_MESSAGE_KOOPA = 27,
    CAPSULE_MESSAGE_DONKEY = 28,
    CAPSULE_MESSAGE_VS = 29,
    CAPSULE_MESSAGE_R_TERESA = 30,
    CAPSULE_EX99_MESSAGE_DICE = 31,
    CAPSULE_EX99_MESSAGE_YAMERU = 32,
    CAPSULE_EX99_MESSAGE_DEBUG_CAM = 33,
    CAPSULE_EX99_MESSAGE_DEBUG_WARP = 34,
    CAPSULE_EX99_MESSAGE_DEBUG_SETPOS = 35,
    CAPSULE_EX99_MESSAGE_END_SPECIAL = 37,
    CAPSULE_EX99_MESSAGE_USE_CHOICE = 39,
    CAPSULE_EX98_MESSAGE_DEBUG_CAM = 31,
    CAPSULE_EX98_MESSAGE_DEBUG_WARP = 32,
    CAPSULE_EX98_MESSAGE_DEBUG_SETPOS = 33,
    CAPSULE_EX98_MESSAGE_NONE = 34,
};

enum {
    CAPSULE_VALUE_TYPE_BITS = 8,
    CAPSULE_VALUE_TYPE_MASK = (1 << CAPSULE_VALUE_TYPE_BITS) - 1,
    CAPSULE_VALUE_PLAYER_SHIFT = CAPSULE_VALUE_TYPE_BITS,
    CAPSULE_VALUE_NONE = CAPSULE_VALUE_TYPE_MASK,
    CAPSULE_MASU_SELECT_BLOCKED = 1 << 15,
    CAPSULE_EFF_COLOR_RANGE = 32768,
    CAPSULE_VALID_LIST_MAX = 33,
};

enum {
    MSM_SE_BOARD_25 = 1029,
    MSM_SE_BOARD_26 = 1030,
    MSM_SE_BOARD_27 = 1031,
    MSM_SE_BOARD_28 = 1032,
    MSM_SE_BOARD_30 = 1034,
    MSM_SE_BOARD_36 = 1040,
};

static GXColor capsuleCrackEffColor;
static GXColor capsuleCrackEffAmbColor;
static GXColor capsuleCrackEffMatColor;

typedef void (*CAPSULE_THROW_HOOK)(BOOL startF);

typedef struct CapsuleObjColor_s {
    s16 flag;
    MBMODELID mdlId;
    MBMODELID mdlId2;
    u8 layer;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
} CAPSULE_OBJ_COLOR;

typedef struct CapEffMasuOkWork_s {
    s32 modelId;
    s32 state;
    s32 masuId;
    s32 unk0C;
    float scale;
} CAP_EFF_MASU_OK_WORK;

typedef struct CapEffRemoveWork_s {
    int modelId;
    int activeCount;
    void *anim;
} CAP_EFF_REMOVE_WORK;

typedef struct CapEffCrackData_s {
    BOOL flag;
    float scale;
    float angle;
    float angleSpeed;
    float scaleSpeed;
    int delay;
    HuVecF prevVel;
    HuVecF prevPos[3];
    HuVecF accel;
    HuVecF vel;
    HuVecF pos[3];
    HuVec2f uv[3];
    GXColor color;
} CAP_EFF_CRACK_DATA;

typedef struct CapEffCrackWork_s {
    s32 modelId;
    s32 state;
    s32 time;
    s32 num;
    s32 vtxNum;
    s32 segNum;
    HuVecF *vtx;
    HuVec2f *st;
    CAP_EFF_CRACK_DATA *data;
    ANIMDATA *animP;
    GXColor color;
    u32 dlSize;
    void *dl;
} CAP_EFF_CRACK_WORK;

typedef struct CapEffTrailWork_s {
    HuVecF prevPos[12];
} CAP_EFF_TRAIL_WORK;

typedef struct CapEffTrailPoint_s {
    HuVecF start;
    HuVecF end;
    float mag;
    float totalMag;
} CAP_EFF_TRAIL_POINT;

#define CAP_EFF_ATTR_NONE 0
#define CAP_EFF_ATTR_COUNTER_RESET (1 << 0)
#define CAP_EFF_ATTR_COUNTER_UPDATE (1 << 1)

#define CAP_EFF_DISPATTR_NONE 0
#define CAP_EFF_DISPATTR_ZBUF_OFF (1 << 0)
#define CAP_EFF_DISPATTR_NOANIM (1 << 1)
#define CAP_EFF_DISPATTR_CAMERA_ROT (1 << 2)
#define CAP_EFF_DISPATTR_ROT3D (1 << 3)
#define CAP_EFF_DISPATTR_ALL 15

#define CAP_EFF_GLOW_Z_OFFSET (30.0f + (16.0f * FLT_EPSILON))

typedef struct CapEffect_s CAP_EFFECT;
typedef void (*CAP_EFF_HOOK)(HU3D_MODEL *modelP, CAP_EFFECT *effP, Mtx *matrix);

typedef struct CapEffData_s {
    s16 time;
    s16 work;
    s16 mode;
    s16 cameraBit;
    HuVecF vel;
    float baseAlpha;
    float tpLvl;
    float speed;
    float unk20;
    float gravity;
    float rotSpeed;
    float animTime;
    float animSpeed;
    float scale;
    HuVecF rot;
    HuVecF pos;
    GXColor color;
    int no;
} CAP_EFF_DATA;

struct CapEffect_s {
    s16 mode;
    s16 time;
    HuVecF vel;
    s16 work[8];
    u8 blendMode;
    u8 attr;
    u8 dispAttr;
    u8 unk23;
    HU3D_MODELID modelId;
    s16 num;
    u32 count;
    u32 prevCounter;
    u32 prevCount;
    u32 dlSize;
    ANIMDATA *anim;
    CAP_EFF_DATA *data;
    HuVecF *vertex;
    HuVec2f *st;
    void *dl;
    CAP_EFF_HOOK hook;
    HU3D_MODEL *hookMdlP;
};

typedef struct CapEffUseWork_s {
    int playerNo;
    int capsuleNo;
} CAP_EFF_USE_WORK;

typedef struct CapSelectMasuWork_s {
    int unk00;
    int unk04;
    int unk08;
    int unk0C;
    int objId;
    int winId1;
    int winId2;
} CAP_SELECT_MASU_WORK;

typedef struct CapEffThrowWork_s {
    int modelId;
    int playerNo;
    int maxTime;
    BOOL endF;
    int minNo;
    int no;
    int initF;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF pos;
    float yOfs;
    u32 tick;
    u32 delay;
    float x[3];
    float y[3];
    float z[3];
} CAP_EFF_THROW_WORK;

typedef struct CapPlayerThrowFrame_s {
    float speed;
    float yOfs;
    float radius;
    int dir;
} CAP_PLAYER_THROW_FRAME;

typedef struct CapGuideWork_s {
    int objId;
    int state;
    int rotY;
    float scale;
} CAP_GUIDE_WORK;

typedef struct CapsuleObjData_s {
    int objId;
    void *anim;
    HU3D_ANIMID animId0;
    HU3D_ANIMID animId1;
} CAPSULE_OBJ_DATA;

typedef struct CapPlayerThrowWork_s {
    int playerNo;
    int masuId;
    int capsuleNo;
    int capObjId0;
    int capObjId1;
    int objColorId;
    int jumpMotId;
    int time;
    int maxTime;
    float yOfs;
    HuVecF pos;
    HuVecF masuPos;
    HuVecF unused;
} CAP_PLAYER_THROW_WORK;

typedef struct CapAutoThrowWork_s {
    int playerNo;
    int capsuleNo;
    int masuId;
    int maxTime;
    float startT;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF masuPos;
} CAP_AUTO_THROW_WORK;

typedef struct CapUseDeleteWork_s {
    int unk00;
    int unk04;
    int unk08;
    int capObjId;
    int objId;
    int winId0;
    int winId1;
} CAP_USE_DELETE_WORK;

typedef struct CapUseWork_s {
    int playerNo;
    int capsuleNo;
} CAP_USE_WORK;

typedef struct CapsuleList_s {
    s8 id;
    s8 cost[3];
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u8 unk08;
    u8 unk09;
    u8 unk0A;
    u8 unk0B;
    u8 unk0C;
    u8 unk0D;
    u8 unk0E;
    u8 unk0F;
} CAPSULE_LIST;

typedef struct CapsuleListFile_s {
    s32 boardNo;
    s32 dataNo;
} CAPSULE_LIST_FILE;

typedef struct CapsuleData_s {
    u32 file;
    u32 objFile;
    u32 descMes;
    u32 useMes;
    int masuPat;
    int color;
    int cost;
    s8 code;
    u16 useMode;
    char *debugName;
    u8 listFlag;
} CAPSULE_DATA;

typedef struct CapsuleTurnData_s {
    s8 code;
    int chance;
} CAPSULE_TURN_DATA;

typedef struct CapsuleComChoice_s {
    int index;
    int capsuleNo;
    int chance;
} CAPSULE_COM_CHOICE;

typedef struct CapsuleComChanceRank_s {
    s16 chance;
    s8 code;
    s8 unk03;
} CAPSULE_COM_CHANCE_RANK;

typedef struct CapsuleComChance_s {
    s16 capsuleNo;
    CAPSULE_COM_CHANCE_RANK rank[11];
} CAPSULE_COM_CHANCE;

static HUPROCESS *capsulePlayerThrowProc;
static CAPSULE_THROW_HOOK capsuleThrowHook;
static OMOBJ *capsuleGuideOMObj;
static int capsuleObjId = MB_MODEL_NONE;
static int capsuleColObjId = MB_MODEL_NONE;
static int capsuleColMdlId = MB_MODEL_NONE;
static int capEffThrowMdlId = MB_MODEL_NONE;
static BOOL capsuleMasuSelectComF[4] = { FALSE, FALSE, FALSE, FALSE };
static CAPSULE_DATA capsuleData[] = {
    { DATANUM(DATA_capsule, CAPSULE_DATA_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KINOKO), 0, 0, 5, 'A', 0, "KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_S_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_S_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_S_KINOKO), 0, 0, 10, 'B', 0, "S KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_P_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_P_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_P_KINOKO), 0, 0, 10, 'A', 0, "P KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_M_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_M_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_M_KINOKO), 0, 0, 20, 'A', 0, "M KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KILLER), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KILLER), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KILLER), 0, 0, 20, 'B', 0, "KILLER", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_DOKAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DOKAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DOKAN), 0, 0, 15, 'C', 0, "DOKAN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_HANACHAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_HANACHAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_HANACHAN), 0, 0, 30, 'E', 0, "HANACHAN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_N_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_N_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_N_KINOKO), 0, 0, 30, 'E', 0, "N KINOKO", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NULL", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NASI", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_TOGEZO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TOGEZO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TOGEZO), 1, 1, 10, 'A', 1, "TOGEZO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KURIBO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KURIBO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KURIBO), 1, 1, 15, 'A', 1, "KURIBO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_PAKKUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_PAKKUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_PAKKUN), 1, 1, 20, 'B', 1, "PAKKUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_JANGO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_JANGO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_JANGO), 1, 1, 5, 'D', 1, "JANGO", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "HANUKE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KOKAMEKKU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KOKAMEKKU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KOKAMEKKU), 1, 1, 10, 'C', 1, "KOKAMEKKU", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KAMEKKU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KAMEKKU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KAMEKKU), 1, 1, 5, 'B', 1, "KAMEKKU", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_THROWMAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_THROWMAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_THROWMAN), 1, 1, 10, 'A', 1, "THROWMAN", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "SUKA", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "KARA", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BOBLE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BOBLE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BOBLE), 2, 2, 10, 'B', 2, "BOBLE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BIRIQ), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BIRIQ), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BIRIQ), 2, 2, 15, 'C', 2, "BIRIQ", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_TUMUJIKUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TUMUJIKUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TUMUJIKUN), 2, 2, 15, 'B', 2, "TUMUJIKUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_DOSSUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DOSSUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DOSSUN), 2, 2, 15, 'C', 2, "DOSSUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BOMUHEI), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BOMUHEI), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BOMUHEI), 2, 2, 10, 'C', 2, "BOMUHEI", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_PATAPATA), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_PATAPATA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_PATAPATA), 2, 2, 10, 'E', 2, "PATAPATA", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NETA", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "GA  ", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "TUKI", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "TA  ", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_HONE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_HONE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_HONE), 3, 3, 20, 'D', 3, "HONE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_LIGHT), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_LIGHT), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_LIGHT), 3, 3, 10, 'D', 3, "LIGHT", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TARU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TARU), 3, 3, 15, 'D', 3, "TARU", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KILLER_MOVE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KILLER_MOVE), 10, 3, 1, 'C', 3, "KILLER MOVE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KETTOU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KETTOU), 10, 3, 1, 'C', 3, "KETTOU", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_4), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_MIRACLE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_MIRACLE), 10, 3, 1, 'C', 3, "MIRACLE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KOOPA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KOOPA), 9, 3, 1, 'E', 3, "KOOPA", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DONKEY), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DONKEY), 10, 3, 1, 'E', 3, "DONKEY", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_VS), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_VS), 10, 3, 1, 'Z', 3, "VS", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_R_TERESA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_R_TERESA), 10, 3, 1, 'Z', 3, "R_TERESA", 0 },
    { DATANUM(DATA_board, BOARD_DATA_DICE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DICE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "DICE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_YAMERU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_YAMERU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "YAMERU", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_CAM), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_CAM), 10, 0, 1, 'Z', 0, "DEBUG CAM TEST", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_WARP), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_WARP), 10, 0, 1, 'Z', 0, "DEBUG WARP TEST", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_SETPOS), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_SETPOS), 10, 0, 1, 'Z', 0, "DEBUG SETPOS TEST", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
};

static CAPSULE_LIST_FILE capsuleListFileTbl[] = {
    { 0, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_0) },
    { 1, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_1) },
    { 2, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_2) },
    { 3, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_3) },
    { 4, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_4) },
    { 5, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_5) },
    { 6, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_6) },
    { 7, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_7) },
    { 8, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_8) },
    { 9, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_9) },
    { 10, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_10) },
    { -1, -1 },
};
static CAPSULE_COM_CHANCE capsuleChanceTbl[] = {
    { 0, { { 50, ' ' }, { 50, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 1, { { 50, '*' }, { 50, '*' }, { 10, '*' }, { 30, '*' }, { 30, ' ' }, { 30, '*' }, { 10, ' ' }, { 30, '*' }, { 30, ' ' }, { 30, ' ' }, { 30, '*' } } },
    { 2, { { 5, 'b' }, { 5, 'b' }, { 10, 'b' }, { 10, 'b' }, { 10, ' ' }, { 5, 'a' }, { 10, ' ' }, { 5, 'a' }, { 30, ' ' }, { 10, ' ' }, { 5, 'a' } } },
    { 3, { { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 30, ' ' }, { 0, '*' }, { 0, '*' }, { 30, ' ' }, { 0, '*' } } },
    { 4, { { 10, ' ' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 10, '*' }, { 50, ' ' }, { 50, '*' }, { 30, '*' }, { 50, ' ' }, { 30, '*' } } },
    { 5, { { 30, ' ' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, '*' }, { 50, ' ' }, { 10, ' ' }, { 10, '*' } } },
    { 6, { { 30, '*' }, { 50, '*' }, { 30, '*' }, { 50, '*' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 50, '*' }, { 10, '*' }, { 10, '*' }, { 30, '*' } } },
    { 10, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' } } },
    { 11, { { 10, ' ' }, { 30, '*' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, '*' }, { 50, ' ' }, { 50, ' ' }, { 50, '*' } } },
    { 12, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' } } },
    { 13, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 15, { { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' } } },
    { 16, { { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 17, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' } } },
    { 20, { { 50, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 21, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 22, { { 50, ' ' }, { 10, '*' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, '*' }, { 10, ' ' }, { 10, '*' }, { 50, ' ' }, { 30, ' ' }, { 10, '*' } } },
    { 23, { { 30, ' ' }, { 50, '*' }, { 10, '*' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, '*' }, { 30, ' ' }, { 30, ' ' }, { 10, '*' } } },
    { 24, { { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 25, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 30, { { 30, '*' }, { 50, ' ' }, { 30, '*' }, { 30, ' ' }, { 30, '*' }, { 50, '*' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' } } },
    { 31, { { 10, '*' }, { 50, ' ' }, { 30, '*' }, { 30, ' ' }, { 10, '*' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 0, ' ' }, { 10, ' ' }, { 50, ' ' } } },
    { -1, { { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' } } },
};
static int capsuleMaxTurnTbl[9] = { 10, 15, 20, 25, 30, 35, 40, 45, 50 };
static int capsuleTurnTbl[9][2] = {
    { 3, 6 },
    { 5, 10 },
    { 5, 15 },
    { 8, 16 },
    { 10, 20 },
    { 10, 20 },
    { 13, 26 },
    { 15, 30 },
    { 15, 35 },
};
static int capsuleHiliteFileTbl[3] = {
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_0),
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_1),
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_2),
};
static HuVecF capsuleCrackScaleTbl[2][3] = {
    {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
    },
    {
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f },
    },
};
static GXColor capsuleTrailColorTbl[4][2] = {
    {
        { 128, 255, 128, 255 },
        { 192, 255, 192, 255 },
    },
    {
        { 255, 255, 128, 255 },
        { 255, 255, 192, 255 },
    },
    {
        { 255, 128, 128, 255 },
        { 255, 192, 192, 255 },
    },
    {
        { 128, 255, 255, 255 },
        { 192, 255, 255, 255 },
    },
};
static HuVecF capsulePlayerThrowRot[3] = {
    { -90.0f, 0.0f, 20.0f },
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 60.0f },
};
static GXColor capsulePlayerThrowColorTbl[4] = {
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
};
static int capsulePlayerThrowDelayTbl[11] = {
    100000, 100000, 100000, 100000, 100000, 100000,
    100000, 100000, 100000, 100000, 100000,
};
static CAP_PLAYER_THROW_FRAME capsulePlayerThrowFrameTbl[48] = {
    { 0.5f, 0, 0, 0 }, { 0.2f, 0, 0, 0 }, { 0.8f, 0, 0, 0 },
    { 0.5f, 500, 0, 0 }, { 0.2f, 500, 0, 0 }, { 0.8f, 500, 0, 0 }, { 1.0f, 500, 0, 0 },
    { 1.2f, 500, 0, 0 }, { 0.2f, 0, 300, 0 }, { 0.2f, 0, -300, 0 }, { 0, 0, 300, 0 },
    { 0, 0, -300, 0 }, { 0.8f, 0, 300, 0 }, { 0.8f, 0, -300, 0 }, { 1.0f, 0, 300, 0 },
    { 1.0f, 0, -300, 0 }, { 0.2f, 0, 800, 0 }, { 0.2f, 0, -800, 0 }, { 0, 0, 800, 0 },
    { 0, 0, -800, 0 }, { 0.8f, 0, 800, 0 }, { 0.8f, 0, -800, 0 }, { 1.0f, 0, 800, 0 },
    { 1.0f, 0, -800, 0 }, { 0.2f, 500, 300, 0 }, { 0.2f, 500, -300, 0 }, { 0, 500, 300, 0 },
    { 0, 500, -300, 0 }, { 0.8f, 500, 300, 0 }, { 0.8f, 500, -300, 0 }, { 1.0f, 500, 300, 0 },
    { 1.0f, 500, -300, 0 }, { 0.2f, 500, 800, 0 }, { 0.2f, 500, -800, 0 }, { 0, 500, 800, 0 },
    { 0, 500, -800, 0 }, { 0.8f, 500, 800, 0 }, { 0.8f, 500, -800, 0 }, { 1.0f, 500, 800, 0 },
    { 1.0f, 500, -800, 0 }, { 0.5f, 500, 0, 1 }, { 0.5f, 500, 0, -1 }, { 0.7f, 500, 0, 1 },
    { 0.7f, 500, 0, -1 }, { 0.3f, 500, 0, 1 }, { 0.3f, 500, 0, -1 }, { 0.7f, 500, 0, 1 },
    { 0.7f, 500, 0, -1 },
};
static HuVecF capsuleAutoThrowRot[2] = {
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 60.0f },
};
static GXColor capsuleAutoThrowColorTbl[4] = {
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
};
static OMOBJ *capEffMasuOkOMObj;
static OMOBJ *capEffRemoveOMObj;
static OMOBJ *capEffRemoveAddOMObj;
static OMOBJ *capEffHiliteOMObj;
static OMOBJ *capEffCrackOMObj;
static OMOBJ *capEffTrailOMObj;
static OMOBJ *capsuleThrowMasuCoinOMObj;
static OMOBJ *capsuleThrowMasuHitOMObj;
static OMOBJ *capsuleThrowRayOMObj;
static OMOBJ *capsuleThrowRingOMObj;
static OMOBJ *capsuleThrowGlowOMObj;
static BOOL capsuleMasuSelectEndF;
static int capsuleComChoice;
static BOOL capsuleUseRemoveOnF;

extern int mbev_CapEffCoinNumGet(OMOBJ *obj);
extern int mbev_CapEffGlowDispGet(OMOBJ *obj);
extern void mbev_CapEffGlowKill(OMOBJ *obj);
extern void mbev_CapEffRingKill(OMOBJ *obj);
extern void mbev_CapEffRayKill(OMOBJ *obj);
extern void mbev_CapEffMasuHitKill(OMOBJ *obj);
extern void mbev_CapEffCoinKill(OMOBJ *obj);
extern OMOBJ *mbev_CapEffRayCreate(float scale, float speed);
extern OMOBJ *mbev_CapEffMasuHitCreate(void);
extern OMOBJ *mbev_CapEffGlowCreate(void);
extern OMOBJ *mbev_CapEffRingCreate(void);
extern OMOBJ *mbev_CapEffCoinCreate(void);
extern void mbev_CapEffRayTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);
extern void mbev_CapEffMasuHitTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);
extern int mbev_CapEffRayAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB, int time, float scale);
extern void mbev_CapEffRayAlphaSet(OMOBJ *obj, float alpha);
extern int mbev_CapEffMasuHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB, float scale,
    float scaleY, int time);
extern int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot, HuVecF *vel, int kind, int time,
    int mode, GXColor *color);
extern void mbev_CapEffCoinMultiAdd(OMOBJ *obj, HuVecF *pos, int num);
extern int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, int time, float scale,
    float gravity, float unk, GXColor *color);
extern void mbev_CapEffColorSet(GXColor *color, int colorNo);
extern void mbWipeDissolveFadeInTime(int time);
extern void mbev_CapPlayerMotShiftWait(int playerNo, int motNo, u32 attr, BOOL waitF);
extern void mbev_CapBonusCoinCall(int playerNo, int capsuleNo, int coinNum, BOOL waitF);
extern BOOL mbev_CapMasuMoveCheck(int masuId);
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbWipeSpecialFadeInCreate(int type, int time);

s16 mbCapValueTypeGet(s16 value);
s16 mbCapMasuDispTypeGet(s16 masuId);
int mbCapObjCreate(int capsuleNo, BOOL flag);
int mbCapFileGet(int capsuleNo);
int mbCapColorGet(int capsuleNo);
int mbCapUseMesGet(int capsuleNo);
int mbCapBonusCoinNumGet(int playerNo, int capsuleNo);
int mbCapComChanceGet(int capsuleNo, int playerNo, int mode);
char *mbCapDebugNameGet(int capsuleNo);
int mbCapObjColorCreate(int capsuleNo, BOOL createF);
void mbCapObjColorPosSet(int id, float x, float y, float z);
void mbCapObjColorScaleSet(int id, float x, float y, float z);
void mbCapObjColorDispSet(int id, BOOL dispF);
void mbCapObjColorMtxGet(int id, Mtx *mtx);
void mbCapObjColorMtxSet(int id, Mtx *mtx);
void mbCapSelectResultGet(int playerNo, int *objId, int *result);
void mbCapSelectResultReset(int playerNo);
void mbCapSelectResultSet(int playerNo, int objId, int result);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
extern BOOL mbSaveNewF;
static inline void CapObjUnitScaleSet(CAPSULE_OBJ_COLOR *obj)
{
    obj->scale.x = obj->scale.y = obj->scale.z = 1.0f;
}

extern const float lbl_802C4514;
extern const float lbl_802C4524;
extern const float lbl_802C4530;
extern const float lbl_802C4540;
extern const float lbl_802C4544;
extern const float lbl_802C455C;
extern const float lbl_802C45C0;
extern const float lbl_802C45D0;
extern const float lbl_802C4574;
extern const float lbl_802C4570;
extern const float lbl_802C4578;
extern const float lbl_802C4598;
extern const float lbl_802C4618;
extern const float lbl_802C465C;
extern const float lbl_802C4660;
extern const float lbl_802C4664;
extern const float lbl_802C4668;
extern u32 mbCapEffNum;
extern s16 *mbCapEffData;

#define CAP_EFF_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

static void CapComKeyHook(void);
static void CapComChoiceSet(int choice);
static void CapEffUse(void);
static void CapEffUseKill(void);
static void CapPlayerThrow(void);
static void CapAutoThrow(CAP_AUTO_THROW_WORK *work);
static void CapEffThrowHook(HU3D_MODEL *modelP, Mtx *mtx);
static void CapEffThrowCreate(int playerNo, float *x, float *y, float *z, float yOfs, int masuId);
static void CapEffThrowMasuCreate(int masuId, int capsuleNo);
static int CapEffThrowMasu(int masuId, int capsuleNo, int playerNo, BOOL bonusF);
static void CapThrowCameraSet(float *x, float *y, float *z, int num);
static void CapThrowCameraCalc(float t, float *x, float *y, float *z, HuVecF *out, int num);
static void CapThrowEndWin(int unused, int value);
static void CapColMdlIdGet(void);
static void CapColKill(void);
static BOOL CapColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out);
static BOOL CapColExec(int playerNo, HuVecF *posA, HuVecF *posB, HuVecF *out);
static void CapGuideKill(void);
static void CapGuideOMExec(OMOBJ *obj);
static BOOL CapEffThrowCheck(HuVecF *pos, int *maxTime);
static void CapEffThrowKill(void);
static BOOL CapEffThrowMasuWait(BOOL waitGlowF);
static void CapEffMasuOkKill(void);
static void CapEffRemoveAddDestroy(void);
static void CapEffHiliteKill(void);
static void CapEffCrackCreate(void);
static void CapEffCrackAdd(HuVecF *pos, HuVecF *rot);
static void CapEffCrackOMExec(OMOBJ *obj);
static void CapEffCrackDraw(HU3D_MODEL *modelP, Mtx *mtx);
static void CapEffCrackKill(void);
static void CapEffTrailCreate(int capsuleNo);
static void CapEffTrailOMExec(OMOBJ *obj);
static void CapEffTrailKill(void);
static void CapEffTrailPosSet(HuVecF *pos);
static void CapEffTrailAdd(HuVecF *pos, int capsuleNo);
static HU3D_MODELID CapEffCreate(ANIMDATA *anim, s16 num);
static void CapEffDraw(HU3D_MODEL *modelP, Mtx *mtx);
static int CapUseSelect(CAP_USE_WORK *work);
static int CapUse(int playerNo, int capsuleNo);
static BOOL CapSelectMasuCheck(int masuId);
static BOOL CapCheckComPath(int playerNo, int max, int mode);
static void CapSelectMasuLinkCheck(s16 *masuFlag, s16 masuId);
static void CapSelectMasuListGet(
    s16 *masuFlag, s16 masuId, s16 frontMax, s16 backMax);
static void CapSelectMasuAddFront(s16 *masuFlag, s16 masuId, s16 max);
static void CapSelectMasuAddBack(s16 *masuFlag, s16 masuId, s16 max);
void mbCapAutoThrowEnd(CAP_AUTO_THROW_WORK *work);
static HUPROCESS *capsuleUseEffProc[4];
static int capsuleUseEffMode[4];
static HuVecF capsuleUseEffPos[4];
static s16 capsuleNum[33][2];
static CAPSULE_LIST capsuleList[33];
static BOOL capsuleSelectComBack;
static CAPSULE_OBJ_COLOR capsuleObjColorData[CAPSULE_OBJ_COLOR_MAX];
static s16 capsuleObjBorderId[6];
static CAPSULE_OBJ_DATA capsuleObjData[8];
static float capsuleTime[8];
static float capsuleBezierX[8];
static float capsuleBezierY[8];
static float capsuleBezierZ[8];
static s16 *capsuleBorderObjId;

void mbCapObjKill(int objId);
void mbCapObjColorKill(int id);
void mbCapObjColorLayerSet(int id, u8 layer);
void mbCapObjColorPosSetV(int id, HuVecF *pos);
void mbCapMasuCapsuleSet(int masuId, int capsuleNo, int playerNo);
int mbCapCostGet(s16 capsuleNo);
BOOL mbCapListExcludeCheck(s16 capsuleNo);

static void CapPlayerThrowKill(void)
{
    CAP_PLAYER_THROW_WORK *work = HuPrcCurrentGet()->property;

    CapEffThrowKill();
    mbCapObjKill(work->capObjId0);
    mbCapObjKill(work->capObjId1);
    mbCapObjColorKill(work->objColorId);
    HuMemDirectFree(work);
    capsulePlayerThrowProc = NULL;
}

static void CapSelectMasuKill(CAP_SELECT_MASU_WORK *work)
{
    CapGuideKill();
    CapEffMasuOkKill();
    if (work->objId != MB_MODEL_NONE) {
        mbObjKill(work->objId);
    }
    work->objId = MB_MODEL_NONE;
    if (work->winId1 != MB_MODEL_NONE) {
        mbWinKill(work->winId1);
    }
    work->winId1 = MB_MODEL_NONE;
    if (work->winId2 != MB_MODEL_NONE) {
        mbWinKill(work->winId2);
    }
    work->winId2 = MB_MODEL_NONE;
    CapEffHiliteKill();
}

static void CapUseDeleteKill(CAP_USE_DELETE_WORK *work)
{
    if (work->capObjId != MB_MODEL_NONE) {
        mbCapObjKill(work->capObjId);
    }
    work->capObjId = MB_MODEL_NONE;
    if (work->objId != MB_MODEL_NONE) {
        mbObjKill(work->objId);
    }
    work->objId = MB_MODEL_NONE;
    if (work->winId0 != MB_MODEL_NONE) {
        mbWinKill(work->winId0);
    }
    work->winId0 = MB_MODEL_NONE;
    if (work->winId1 != MB_MODEL_NONE) {
        mbWinKill(work->winId1);
    }
    work->winId1 = MB_MODEL_NONE;
}

static int CapUse(int playerNo, int capsuleNo)
{
    CAP_USE_WORK *work;
    int result;
    CAP_USE_WORK *workData;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAP_USE_WORK), HU_MEMNUM_OVL);
    work = workData;
    memset(work, 0, sizeof(CAP_USE_WORK));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    result = CapUseSelect(work);
    HuMemDirectFree(work);
    return result;
}

static int CapUseSelect(CAP_USE_WORK *work)
{
    HuVecF playerPos;
    HuVecF capPos;
    int capObjId;
    int oldObjId;
    int result;
    int time;
    int capsuleSlot;
    float t;
    float scale;

    mbCapSelectResultGet(work->playerNo, &oldObjId, &result);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        capObjId = oldObjId;
    } else {
        capObjId = mbCapObjCreate(work->capsuleNo, FALSE);
    }
    mbPlayerPosGet(work->playerNo, &playerPos);
    capPos = playerPos;
    capPos.y += 250.0f;
    if (oldObjId != MB_MODEL_NONE) {
        mbObjPosGet(capObjId, &capPos);
        capPos.x = playerPos.x;
    }
    mbObjPosSet(capObjId, capPos.x, capPos.y, capPos.z);
    mbObjLayerSet(capObjId, 4);
    mbObjAttrSet(capObjId, HU3D_MOTATTR_LOOP);
    if (oldObjId == MB_MODEL_NONE) {
        mbObjDispSet(capObjId, TRUE);
        time = 0;
        do {
            time++;
            t = (float)time / 18.0f;
            if (t > 1.0f) {
                t = 1.0f;
            }
            mbPlayerPosGet(work->playerNo, &capPos);
            capPos.y += 100.0 + (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
            mbObjPosSetV(capObjId, &capPos);
            scale = (double)1.2f *
                sin((M_PI * (90.0f * t)) / 180.0);
            mbObjScaleSet(capObjId, scale, scale, scale);
            HuPrcVSleep();
        } while (!mbCameraMoveCheck() || t < 1.0f);
    }
    if (!GwPlayer[work->playerNo].comF) {
        mbWinCreateChoice(
            MBWIN_TYPE_EVENT,
            MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_USE_CHOICE), -1,
            0);
        mbWinTopInsertMesSet(mbCapUseMesGet(work->capsuleNo), 0);
        if (GwPlayer[work->playerNo].comF) {
            CapComChoiceSet(0);
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() != 0 || mbWinTopChoiceGet() == -1) {
            if (oldObjId == MB_MODEL_NONE) {
                time = 0;
                for (;;) {
                    t = (float)time / 18.0f;
                    if (t >= 1.0f) {
                        break;
                    }
                    t = 1.0f - t;
                    if (t < 0.0f) {
                        t = 0.0f;
                    }
                    mbPlayerPosGet(work->playerNo, &capPos);
                    capPos.y += 100.0 +
                        (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
                    mbObjPosSetV(capObjId, &capPos);
                    scale = (double)1.2f *
                        sin((M_PI * (90.0f * t)) / 180.0);
                    mbObjScaleSet(capObjId, scale, scale, scale);
                    HuPrcVSleep();
                    time++;
                }
            } else {
                mbCapSelectResultSet(work->playerNo, oldObjId, result);
                capObjId = MB_MODEL_NONE;
            }
            if (capObjId != MB_MODEL_NONE) {
                mbCapObjKill(capObjId);
            }
            return FALSE;
        }
    }
    if (!capsuleUseRemoveOnF) {
        mbPlayerCapsuleUseSet(work->capsuleNo);
        capsuleSlot = mbPlayerCapsuleFind(work->playerNo, work->capsuleNo);
        if (capsuleSlot != -1) {
            mbPlayerCapsuleRemove(work->playerNo, capsuleSlot);
        }
    }
    if (result == -1) {
        result = 0;
    }
    mbCapSelectResultSet(work->playerNo, capObjId, result);
    return TRUE;
}

static BOOL CapSelectMasuDispCheck(int masuId)
{
    return mbMasuDispCheck(masuId);
}

static void CapEffMasuOkKill(void)
{
    capEffMasuOkOMObj = NULL;
}

static void CapEffMasuOkNext(void)
{
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;

    work += 32;
    work->state = 10;
}

static void CapEffMasuOkDispSet(BOOL dispF)
{
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;
    int i;

    for (i = 0; i < 32; i++, work++) {
        if (work->state == 1 && dispF) {
            mbObjDispSet(work->modelId, TRUE);
        } else {
            mbObjDispSet(work->modelId, FALSE);
        }
    }
}

static void CapEffRemoveKill(void)
{
    capEffRemoveOMObj = NULL;
}

static BOOL CapEffRemoveCheck(void)
{
    OMOBJ *obj = capEffRemoveOMObj;
    CAP_EFF_REMOVE_WORK *work;

    if (capEffRemoveOMObj == NULL) {
        return FALSE;
    }
    work = obj->data;
    return work->activeCount;
}

static void CapEffUseKill(void)
{
    CAP_EFF_USE_WORK *work = HuPrcCurrentGet()->property;

    CapEffRemoveAddDestroy();
    CapEffHiliteKill();
    capsuleUseEffProc[work->playerNo] = NULL;
    capsuleUseEffMode[work->playerNo] = -1;
    HuMemDirectFree(work);
}

static void CapEffRemoveAddDestroy(void)
{
    capEffRemoveAddOMObj = NULL;
}

static void CapEffHiliteKill(void)
{
    capEffHiliteOMObj = NULL;
}

char lbl_802BFE51[] = "center";
static GXColor capsuleCrackEffColor = { 255, 255, 128, 64 };
static GXColor capsuleCrackEffAmbColor = { 255, 255, 255, 255 };
static GXColor capsuleCrackEffMatColor = { 255, 255, 255, 255 };

static void CapComChoiceSet(int choice)
{
    capsuleComChoice = choice;
    mbWinTopComKeyHookSet(CapComKeyHook);
}

static void CapComKeyHook(void)
{
    s32 key[4];
    s32 playerNo;
    s32 i;
    s32 padNo;
    s16 time;
    s32 keyValue;

    key[0] = key[1] = key[2] = key[3] = 0;
    playerNo = GwSystem.turnPlayerNo;
    padNo = GwPlayer[playerNo].padNo;
    time = GWComKeyDelayGet();
    if (capsuleComChoice >= 0) {
        keyValue = 2;
    } else {
        keyValue = 1;
    }
    key[padNo] = keyValue;
    for (i = 0; i < abs(capsuleComChoice); i++) {
        key[padNo] = keyValue;
        HuWinComKeyWait(key[0], key[1], key[2], key[3], time);
    }
    key[padNo] = 256;
    HuWinComKeyWait(key[0], key[1], key[2], key[3], time);
}

int mbCapThrowColCreate(int dataNum)
{
    int objId;

    if (dataNum != MB_MODEL_NONE) {
        objId = (s16)mbObjCreate(dataNum, NULL, FALSE);
        ((void (*)(int, BOOL))mbObjDispSet)(objId, FALSE);
        capsuleColObjId = objId;
        CapColMdlIdGet();
    } else {
        capsuleColObjId = MB_MODEL_NONE;
        CapColKill();
    }
    return objId;
}

BOOL mbCapThrowColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    BOOL result = CapColCheck(posA, posB, out);

    return result;
}

static void CapColMdlIdGet(void)
{
    int boardNo = MBBoardNoGet();

    if (capsuleColObjId != MB_MODEL_NONE) {
        capsuleColMdlId = mbObjModelIDGet(capsuleColObjId);
    }
}

static BOOL CapColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    HSF_FACE *faceP;
    HSF_DATA *hsfP;
    HSF_BUFFER *faceBufP;
    HSF_BUFFER *vtxBufP;
    BOOL quadF;
    int i;
    int j;
    HU3D_MODEL *modelP;
    float dot;
    float dotEdge;
    float dotA;
    float dotB;
    float invDotA;
    float scale;
    HuVecF faceVtx[4];
    HuVecF faceNorm;
    HuVecF faceCA;
    HuVecF faceBA;
    HuVecF faceBAQuad;
    HuVecF faceCAQuad;
    HuVecF faceNormQuad;
    HuVecF dir;
    HuVecF outPos;
    HuVecF dotVec;
    float mag;
    int temp;

    quadF = FALSE;
    temp = 0;
    if (capsuleColMdlId == HU3D_MODELID_NONE) {
        return FALSE;
    }
    modelP = &Hu3DData[capsuleColMdlId];
    hsfP = modelP->hsf;
    faceBufP = hsfP->face;
    vtxBufP = hsfP->vertex;
    PSVECSubtract(posA, posB, &dir);
    if (PSVECMag(&dir) <= 0.0f) {
        return FALSE;
    }
    PSVECNormalize(&dir, &dir);
    mag = PSVECMag(&dir);
    for (i = 0; i < hsfP->faceNum; i++, faceBufP++) {
        faceP = faceBufP->data;
        for (j = 0; j < faceBufP->count; j++, faceP++) {
            switch (faceP->type & HSF_FACE_MASK) {
            case 0:
                quadF = TRUE;
                break;

            case 1:
                quadF = TRUE;
                break;

            case HSF_FACE_TRI:
                quadF = FALSE;
                faceVtx[0] =
                    *((HuVecF *)vtxBufP->data + faceP->index[0].vertex);
                faceVtx[1] =
                    *((HuVecF *)vtxBufP->data + faceP->index[1].vertex);
                faceVtx[2] =
                    *((HuVecF *)vtxBufP->data + faceP->index[2].vertex);
                break;

            case HSF_FACE_QUAD:
                quadF = TRUE;
                break;

            default:
                break;
            }
            if (!quadF) {
                PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBAQuad);
                PSVECSubtract(&faceVtx[2], &faceVtx[0], &faceCAQuad);
                PSVECCrossProduct(&faceBAQuad, &faceCAQuad, &faceNormQuad);
                if (PSVECMag(&faceNormQuad) > 0.0f) {
                    PSVECNormalize(&faceNormQuad, &faceNormQuad);
                }
                dot = -PSVECDotProduct(&faceNormQuad, &faceVtx[0]);
                dotA = (faceNormQuad.x * posA->x)
                    + (faceNormQuad.y * posA->y)
                    + (faceNormQuad.z * posA->z) + dot;
                dotB = (faceNormQuad.x * posB->x)
                    + (faceNormQuad.y * posB->y)
                    + (faceNormQuad.z * posB->z) + dot;
                if (dotA * dotB > 0) {
                    continue;
                }
                invDotA = -((faceNormQuad.x * posA->x)
                    + (faceNormQuad.y * posA->y)
                    + (faceNormQuad.z * posA->z) + dot);
                dotEdge = (faceNormQuad.x * dir.x)
                    + (faceNormQuad.y * dir.y)
                    + (faceNormQuad.z * dir.z);
                if (dotEdge != 0.0f) {
                    scale = invDotA / dotEdge;
                    PSVECScale(&dir, &outPos, scale);
                    PSVECAdd(&outPos, posA, &outPos);
                    *out = outPos;
                    PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[0], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.x = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    PSVECSubtract(&faceVtx[2], &faceVtx[1], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[1], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.y = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    PSVECSubtract(&faceVtx[0], &faceVtx[2], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[2], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.z = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    if (dotVec.x > 0 && dotVec.y > 0 && dotVec.z > 0) {
                        return TRUE;
                    }
                    if (dotVec.x < 0 && dotVec.y < 0 && dotVec.z < 0) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

static void CapColKill(void)
{
    capsuleColMdlId = MB_MODEL_NONE;
}

static HuVecF colScaleTbl[18] = {
    { -0.5f, 0.0f, 0.5f },
    { -0.5f, 1.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { -0.5f, 0.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { 0.0f, 1.0f, -0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.0f, 1.0f, -0.5f },
    { 0.0f, 0.0f, -0.5f },
    { 0.0f, 0.0f, -0.5f },
    { 0.0f, 1.0f, -0.5f },
    { -0.5f, 1.0f, 0.5f },
    { 0.0f, 0.0f, -0.5f },
    { -0.5f, 1.0f, 0.5f },
    { -0.5f, 0.0f, 0.5f },
};

static HuVec2f charSizeTbl[14] = {
    { 100.0f, 175.0f },
    { 100.0f, 200.0f },
    { 110.0f, 220.0f },
    { 100.0f, 175.0f },
    { 175.0f, 175.0f },
    { 110.0f, 220.0f },
    { 200.0f, 260.0f },
    { 100.0f, 150.0f },
    { 175.0f, 175.0f },
    { 150.0f, 125.0f },
    { 100.0f, 150.0f },
    { 150.0f, 125.0f },
    { 150.0f, 125.0f },
    { 150.0f, 125.0f },
};

static BOOL CapColExec(int playerNo, HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    HuVecF playerPos;
    HuVecF faceVtx[3];
    HuVecF faceNorm;
    HuVecF faceBA;
    HuVecF faceCA;
    HuVecF edgeBA;
    HuVecF edgeCA;
    HuVecF edgeNorm;
    HuVecF dir;
    HuVecF outPos;
    float dot;
    float dotEdge;
    float dotA;
    float dotB;
    float invDotA;
    float scale;
    float mag;
    float dotVec[3];
    BOOL partyF;
    int i;
    int j;
    int temp;
    int temp2;

    temp = 0;
    temp2 = 0;
    PSVECSubtract(posA, posB, &dir);
    if (PSVECMag(&dir) <= 0.0f) {
        return FALSE;
    }
    PSVECNormalize(&dir, &dir);
    mag = PSVECMag(&dir);
    for (i = 0; i < 4; i++) {
        partyF = GwSystem.partyF;
        if ((!partyF && i != 0) || i == playerNo ||
            !mbPlayerDispGet(i)) {
            continue;
        }
        mbPlayerPosGet(i, &playerPos);
        for (j = 0; j < 6; j++) {
            faceVtx[0].x = colScaleTbl[j * 3].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[0].y = colScaleTbl[j * 3].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[0].z = colScaleTbl[j * 3].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[1].x = colScaleTbl[j * 3 + 1].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[1].y = colScaleTbl[j * 3 + 1].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[1].z = colScaleTbl[j * 3 + 1].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[2].x = colScaleTbl[j * 3 + 2].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[2].y = colScaleTbl[j * 3 + 2].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[2].z = colScaleTbl[j * 3 + 2].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            PSVECAdd(&faceVtx[0], &playerPos, &faceVtx[0]);
            PSVECAdd(&faceVtx[1], &playerPos, &faceVtx[1]);
            PSVECAdd(&faceVtx[2], &playerPos, &faceVtx[2]);
            PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBA);
            PSVECSubtract(&faceVtx[2], &faceVtx[0], &faceCA);
            PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
            if (PSVECMag(&faceNorm) > 0.0f) {
                PSVECNormalize(&faceNorm, &faceNorm);
            }
            dot = -PSVECDotProduct(&faceNorm, &faceVtx[0]);
            dotA = (faceNorm.x * posA->x) +
                (faceNorm.y * posA->y) + (faceNorm.z * posA->z) + dot;
            dotB = (faceNorm.x * posB->x) +
                (faceNorm.y * posB->y) + (faceNorm.z * posB->z) + dot;
            if (dotA * dotB > 0.0f) {
                continue;
            }
            invDotA = -((faceNorm.x * posA->x) +
                (faceNorm.y * posA->y) + (faceNorm.z * posA->z) + dot);
            dotEdge = (faceNorm.x * dir.x) +
                (faceNorm.y * dir.y) + (faceNorm.z * dir.z);
            if (dotEdge == 0.0f) {
                continue;
            }
            scale = invDotA / dotEdge;
            PSVECScale(&dir, &outPos, scale);
            PSVECAdd(&outPos, posA, &outPos);
            *out = outPos;
            PSVECSubtract(&faceVtx[1], &faceVtx[0], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[0], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[0] = PSVECDotProduct(&faceNorm, &edgeNorm);
            PSVECSubtract(&faceVtx[2], &faceVtx[1], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[1], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[1] = PSVECDotProduct(&faceNorm, &edgeNorm);
            PSVECSubtract(&faceVtx[0], &faceVtx[2], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[2], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[2] = PSVECDotProduct(&faceNorm, &edgeNorm);
            if (dotVec[0] > 0.0f && dotVec[1] > 0.0f && dotVec[2] > 0.0f) {
                return TRUE;
            }
            if (dotVec[0] < 0.0f && dotVec[1] < 0.0f && dotVec[2] < 0.0f) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void CapGuideGrowSet(void)
{
    void *work = capsuleGuideOMObj->data;

    if (capsuleGuideOMObj != NULL) {
        capsuleGuideOMObj->work[0] = TRUE;
    }
}

static void CapGuideKill(void)
{
    capsuleGuideOMObj = NULL;
}

void MBCapsuleStub1(void)
{
}

BOOL MBCapsuleStub2(void)
{
    return FALSE;
}

int mbCapUseCostGet(void)
{
    return 0;
}

BOOL mbCapEffUseCreate(int playerNo, int capsuleNo)
{
    CAP_EFF_USE_WORK *work;
    CAP_EFF_USE_WORK *workData;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    capsuleUseEffProc[playerNo] =
        HuPrcChildCreate(CapEffUse, 8196, 24576, 0, mbMainProc);
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAP_EFF_USE_WORK), HU_MEMNUM_OVL);
    capsuleUseEffProc[playerNo]->property = work = workData;
    memset(work, 0, sizeof(CAP_EFF_USE_WORK));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    capsuleUseEffMode[playerNo] = 0;
    capsuleUseEffPos[playerNo].x = capsuleUseEffPos[playerNo].y =
        capsuleUseEffPos[playerNo].z = 0.0f;
    HuPrcDestructorSet2(capsuleUseEffProc[playerNo], CapEffUseKill);
    return TRUE;
}

int mbCapEffUseModeGet(int playerNo)
{
    return capsuleUseEffMode[playerNo];
}

BOOL mbCapEffUsePosGet(int playerNo, HuVecF *pos)
{
    int mode = capsuleUseEffMode[playerNo];

    if (mode < 1) {
        return FALSE;
    }
    *pos = capsuleUseEffPos[playerNo];
    return TRUE;
}

static void CapEffUse(void)
{
    CAP_EFF_USE_WORK *work = HuPrcCurrentGet()->property;
    OMOBJ *rayObj;
    OMOBJ *masuHitObj;
    OMOBJ *glowObj;
    HuVecF playerPos;
    HuVecF capPos;
    HuVecF pos;
    HuVecF rot;
    HuVecF rot2;
    HuVecF vel;
    GXColor color;
    GXColor glowColor;
    Mtx mtx;
    int capObjId;
    int oldObjId;
    int result;
    int time;
    int i;
    int j;
    float t;
    float scale;
    float alpha;
    float angle;
    float radius;
    float randF;

    rayObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    masuHitObj = mbev_CapEffMasuHitCreate();
    glowObj = mbev_CapEffGlowCreate();
    mbCapSelectResultGet(work->playerNo, &oldObjId, &result);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        capObjId = oldObjId;
    } else {
        capObjId = mbCapObjCreate(work->capsuleNo, FALSE);
    }
    mbPlayerPosGet(work->playerNo, &playerPos);
    capPos = playerPos;
    capPos.y += 250.0f;
    if (oldObjId != MB_MODEL_NONE) {
        mbObjPosGet(capObjId, &capPos);
        capPos.x = playerPos.x;
    }
    mbObjPosSet(capObjId, capPos.x, capPos.y, capPos.z);
    mbObjLayerSet(capObjId, 4);
    mbObjAttrSet(capObjId, HU3D_MOTATTR_LOOP);
    mbev_CapEffRayTransformSet(rayObj, &capPos, NULL, NULL);
    mbev_CapEffMasuHitTransformSet(masuHitObj, &capPos, NULL, NULL);
    if (oldObjId == MB_MODEL_NONE) {
        mbObjDispSet(capObjId, TRUE);
        time = 0;
        do {
            time++;
            t = (float)time / 18.0f;
            if (t > 1.0f) {
                t = 1.0f;
            }
            mbPlayerPosGet(work->playerNo, &pos);
            pos.y += 100.0 + (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
            mbObjPosSetV(capObjId, &pos);
            scale = (double)1.2f *
                sin((M_PI * (90.0f * t)) / 180.0);
            mbObjScaleSet(capObjId, scale, scale, scale);
            HuPrcVSleep();
        } while (!mbCameraMoveCheck() || t < 1.0f);
    }
    capsuleUseEffMode[work->playerNo] = 1;
    capsuleUseEffPos[work->playerNo] = capPos;
    mbAudFXPlay(MSM_SE_BOARD_30);
    time = 0;
    while ((float)time < 30.0f) {
        alpha = cos((M_PI * (90.0f * ((float)time / 30.0f))) / 180.0);
        for (j = 0; j < 3; j++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * MBCapsuleEffRandF();
            rot.y = 360.0f * MBCapsuleEffRandF();
            rot.z = 360.0f * MBCapsuleEffRandF();
            rot2.x = rot.x + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.y = rot.y + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.z = rot.z + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            scale =
                200.0f * (1.0f + (0.25f * MBCapsuleEffRandF()));
            i = (int)(15.0f + (5.0f * MBCapsuleEffRandF()));
            mbev_CapEffRayAdd(rayObj, &pos, &rot, &rot2, i, scale);
        }
        mbev_CapEffRayAlphaSet(rayObj, alpha);
        for (j = 0; j < 5; j++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * MBCapsuleEffRandF();
            rot.y = 360.0f * MBCapsuleEffRandF();
            rot.z = 360.0f * MBCapsuleEffRandF();
            rot2.x = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.y = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.z = 360.0f * (-0.5f + MBCapsuleEffRandF());
            scale = 100.0f * alpha * (1.5f + MBCapsuleEffRandF());
            randF =
                25.0f * alpha * (1.0f + (0.5f * MBCapsuleEffRandF()));
            i = (int)(20.0f + (10.0f * MBCapsuleEffRandF()));
            mbev_CapEffMasuHitAdd(
                masuHitObj, &pos, &rot, &rot2, scale, randF, i);
        }
        HuPrcVSleep();
        time++;
    }
    mtxRot(mtx, 0.0f, 0.0f, 20.0f);
    for (i = 0; i < 256; i++) {
        pos.x =
            capPos.x + (75.0f * (-0.5f + MBCapsuleEffRandF()));
        pos.y =
            capPos.y + (75.0f * (-0.5f + MBCapsuleEffRandF()));
        pos.z = capPos.z + CAP_EFF_GLOW_Z_OFFSET;
        angle = 360.0f * MBCapsuleEffRandF();
        radius = 5.0f * (0.5f + MBCapsuleEffRandF());
        rot.x = radius * sin((M_PI * angle) / 180.0);
        rot.y = radius * (0.3f * (-0.5f + MBCapsuleEffRandF()));
        rot.z = radius * cos((M_PI * angle) / 180.0);
        PSMTXMultVec(mtx, &rot, &rot);
        mbev_CapEffColorSet(&color, mbRandMod(CAPSULE_EFF_COLOR_RANGE));
        glowColor = color;
        vel = rot;
        scale = 5.0f * (-0.5f + MBCapsuleEffRandF());
        randF = 0.3f;
        time =
            (int)(60.0f * (0.4f + (0.5f * MBCapsuleEffRandF())));
        randF = 100.0f * (randF + (0.2f * MBCapsuleEffRandF()));
        mbev_CapEffGlowAdd(
            glowObj, &pos, &vel, time, randF, scale, 0.0f, &glowColor);
        if (i == 128) {
            HuPrcVSleep();
        }
    }
    capsuleUseEffMode[work->playerNo] = 2;
    capsuleUseEffPos[work->playerNo] = capPos;
    mbObjDispSet(capObjId, FALSE);
    for (time = 0; time < 20; time++) {
        scale = cos((M_PI * (90.0f * ((float)time / 20.0f))) / 180.0);
        mbObjScaleSet(capObjId, scale, scale, scale);
        HuPrcVSleep();
    }
    capsuleObjId = MB_MODEL_NONE;
    while (mbev_CapEffGlowDispGet(glowObj)) {
        HuPrcVSleep();
    }
    if (capObjId != MB_MODEL_NONE) {
        mbCapObjKill(capObjId);
    }
    mbev_CapEffRayKill(rayObj);
    mbev_CapEffMasuHitKill(masuHitObj);
    mbev_CapEffGlowKill(glowObj);
    HuPrcEnd();
}

BOOL mbCapPlayerThrowCheck(void);

void mbCapPlayerThrow(int playerNo, int masuId, int capsuleNo)
{
    CAP_PLAYER_THROW_WORK *work;
    CAP_PLAYER_THROW_WORK *workData;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    capsulePlayerThrowProc = HuPrcChildCreate(CapPlayerThrow, 8196, 24576, 0, mbMainProc);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAP_PLAYER_THROW_WORK), HU_MEMNUM_OVL);
    work = workData;
    capsulePlayerThrowProc->property = work;
    memset(work, 0, sizeof(CAP_PLAYER_THROW_WORK));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->capObjId0 = MB_MODEL_NONE;
    work->capObjId1 = MB_MODEL_NONE;
    work->objColorId = MB_MODEL_NONE;
    work->time = 0;
    HuPrcDestructorSet2(capsulePlayerThrowProc, CapPlayerThrowKill);
    while (mbCapPlayerThrowCheck() == FALSE) {
        HuPrcVSleep();
    }
}

BOOL mbCapPlayerThrowCheck(void)
{
    if (capsulePlayerThrowProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void CapPlayerThrow(void)
{
    CAP_PLAYER_THROW_WORK *work;
    int seNo;
    int playerObjId;
    int capsuleSlot;
    int handTime;
    int playerModelId;
    int coinNum;
    int motionTime;
    HuVecF effPos;
    HuVecF playerRot;
    HuVecF mdl2Pos;
    HuVecF dir;
    HuVecF dirOrig;
    HuVecF pos;
    HuVecF vel;
    HuVecF playerPos;
    HuVecF playerPosOrig;
    HuVecF capPos;
    HuVecF handPos;
    float x[3];
    float y[3];
    float z[3];
    HuVecF throwOut;
    HuVecF glowPos;
    HuVecF glowVel;
    Mtx rotMtx;
    Mtx objMtx;
    Mtx handMtx;
    float time;
    float t;
    float arc;
    float scale;
    float mag;
    GXColor color;
    float magTemp;
    GXColor glowColor;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;

    work = HuPrcCurrentGet()->property;
    if (capsuleThrowHook) {
        capsuleThrowHook(TRUE);
    }
    CapEffThrowMasuCreate(work->masuId, work->capsuleNo);
    work->jumpMotId =
        mbPlayerMotionCreate(work->playerNo, CHARMOT_HSF_c000m1_431);
    HuPrcVSleep();
    if (capsuleObjId != MB_MODEL_NONE) {
        work->capObjId0 = capsuleObjId;
    } else {
        work->capObjId0 =
            mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, FALSE);
    }
    mbPlayerPosGet(work->playerNo, &pos);
    mbObjPosSet(work->capObjId0, pos.x, pos.y + 250.0f, pos.z);
    mbObjScaleSet(work->capObjId0, 1.2f, 1.2f, 1.2f);
    mbObjLayerSet(work->capObjId0, 4);
    work->capObjId1 = mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, FALSE);
    mbObjDispSet(work->capObjId1, FALSE);
    while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo))
        != MB_MODEL_NONE) {
        HuPrcVSleep();
    }
    playerObjId = mbPlayerObjIDGet(work->playerNo);
    playerModelId = mbObjModelIDGet(playerObjId);
    motionTime = (int)mbObjMotionTimeGet(playerObjId);
    mbPlayerRotGet(work->playerNo, &playerRot);
    mbPlayerPosGet(work->playerNo, &work->pos);
    mbMasuPosGet(work->masuId, &work->masuPos);
    PSVECSubtract(&work->masuPos, &work->pos, &dir);
    mbPlayerRotSet(work->playerNo, 0.0f,
        (float)((atan2(dir.x, dir.z) / M_PI) * 180.0), 0.0f);
    mbPlayerMotionSet(work->playerNo, work->jumpMotId, HU3D_MOTATTR_NONE);
    mbPlayerMotionTimeSet(
        work->playerNo, 0.35f * mbPlayerMotionMaxTimeGet(work->playerNo));
    Hu3DMotionCalc(playerModelId);
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    work->pos.x = handMtx[0][3];
    work->pos.y = handMtx[1][3];
    work->pos.z = handMtx[2][3];
    mbPlayerMotionSet(work->playerNo, 1, HU3D_MOTATTR_LOOP);
    mbObjMotionTimeSet(playerObjId, (float)motionTime);
    mbPlayerRotSetV(work->playerNo, &playerRot);
    Hu3DMotionCalc(playerModelId);
    HuPrcVSleep();
    mbMasuPosGet(work->masuId, &work->masuPos);
    work->masuPos.y += 20.0f;
    PSVECSubtract(&work->masuPos, &work->pos, &dir);
    mag = PSVECMag(&dir);
    work->yOfs = 50.0f + (0.1f * mag);
    if (work->yOfs > 700.0f) {
        work->yOfs = 700.0f;
    }
    playerPosOrig = work->pos;
    playerPos = playerPosOrig;
    x[0] = work->pos.x;
    y[0] = work->pos.y;
    z[0] = work->pos.z;
    x[1] = work->pos.x;
    y[1] = work->pos.y;
    z[1] = work->pos.z;
    x[2] = work->masuPos.x;
    y[2] = work->masuPos.y;
    z[2] = work->masuPos.z;
    CapEffThrowCreate(
        work->playerNo, x, y, z, work->yOfs, work->masuId);
    if (!GwPlayer[work->playerNo].comF) {
        mbWipeDissolveFadeInTime(15);
    }
    handTime = 45;
    mbObjPosGet(work->capObjId0, &capPos);
    Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    handPos.x = handMtx[0][3];
    handPos.y = handMtx[1][3];
    handPos.z = handMtx[2][3];
    mbAudFXPlay(MSM_SE_BOARD_36);
    for (time = 0.0f; time < handTime; time++) {
        Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
        Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
            CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0),
            handMtx);
        handPos.x = handMtx[0][3];
        handPos.y = handMtx[1][3];
        handPos.z = handMtx[2][3];
        t = sin((M_PI * (90.0f * (time / handTime))) / 180.0);
        arc = 100.0f * sin((M_PI * (180.0f * t)) / 180.0);
        pos.x = capPos.x + (t * (handPos.x - capPos.x))
            + (arc * sin((M_PI * (180.0f - (270.0f * t))) / 180.0));
        pos.y = capPos.y + (t * (handPos.y - capPos.y));
        pos.z = capPos.z + (t * (handPos.z - capPos.z))
            + (arc * cos((M_PI * (180.0f - (270.0f * t))) / 180.0));
        scale = 0.35f + (0.85f * cos((M_PI * (90.0f * t)) / 180.0));
        mbObjPosSetV(work->capObjId0, &pos);
        mbObjScaleSet(work->capObjId0, scale, scale, scale);
        HuPrcVSleep();
    }
    mbObjHookSet(mbPlayerObjIDGet(work->playerNo),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0),
        work->capObjId0);
    mbPlayerRotateStart(work->playerNo,
        (s16)((atan2(dir.x, dir.z) / M_PI) * 180.0), 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    do {
        HuPrcVSleep();
    } while (!CapEffThrowCheck(&pos, &work->maxTime));
    CapEffThrowKill();
    x[1] = pos.x;
    y[1] = pos.y;
    z[1] = pos.z;
    work->time = 0;
    mbPlayerMotionShiftSet(work->playerNo, work->jumpMotId, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbAudFXPlay(MSM_SE_BOARD_25);
    while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo))
        != MB_MODEL_NONE) {
        HuPrcVSleep();
    }
    time = mbPlayerMotionMaxTimeGet(work->playerNo);
    while (mbPlayerMotionTimeGet(work->playerNo) <= (0.35f * time)) {
        HuPrcVSleep();
    }
    Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    x[0] = work->pos.x = handMtx[0][3];
    y[0] = work->pos.y = handMtx[1][3];
    z[0] = work->pos.z = handMtx[2][3];
    if (work->capObjId0 != MB_MODEL_NONE) {
        mbObjHookObjReset(mbPlayerObjIDGet(work->playerNo),
            CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0));
        mbObjDispSet(work->capObjId0, FALSE);
    }
    capsuleObjId = MB_MODEL_NONE;
    work->objColorId = mbCapObjColorCreate(work->capsuleNo, FALSE);
    mbPlayerPosGet(work->playerNo, &pos);
    mbCapObjColorPosSet(
        work->objColorId, pos.x, lbl_802C4530 + pos.y, pos.z);
    mbCapObjColorScaleSet(work->objColorId, 1.0f, 1.0f, 1.0f);
    mbCapObjColorLayerSet(work->objColorId, 4);
    CapEffTrailAdd(&work->pos, work->capsuleNo);
    seNo = mbAudFXPlay(MSM_SE_BOARD_26);
    do {
        work->time++;
        t = (float)work->time / (float)work->maxTime;
        PSVECSubtract(&work->masuPos, &work->pos, &dir);
        PSVECScale(&dir, &dir, t);
        PSVECAdd(&work->pos, &dir, &effPos);
        mdl2Pos = effPos;
        CapThrowCameraCalc(t, x, y, z, &throwOut, 3);
        effPos.x = throwOut.x;
        effPos.y = throwOut.y;
        effPos.z = throwOut.z;
        PSVECSubtract(&effPos, &mdl2Pos, &pos);
        PSVECScale(&pos, &pos, 0.75f);
        PSVECAdd(&mdl2Pos, &pos, &mdl2Pos);
        mbCapObjColorPosSet(
            work->objColorId, effPos.x, effPos.y, effPos.z);
        mbCapObjColorScaleSet(work->objColorId, 0.5f, 0.5f, 0.5f);
        PSVECSubtract(&work->masuPos, &work->pos, &dir);
        dirOrig = dir;
        dir.x = dirOrig.z;
        dir.y = 0.0f;
        dir.z = -dirOrig.x;
        if ((magTemp = PSVECMag(&dir)) <= 0.0f) {
            dir.z = 1.0f;
        }
        PSMTXRotAxisRad(rotMtx, &dir, 0.1f);
        mbCapObjColorMtxGet(work->objColorId, &objMtx);
        PSMTXConcat(rotMtx, objMtx, objMtx);
        mbCapObjColorMtxSet(work->objColorId, &objMtx);
        CapEffTrailPosSet(&effPos);
        mbObjPosSet(work->capObjId1, mdl2Pos.x, mdl2Pos.y, mdl2Pos.z);
        mbObjDispSet(work->capObjId1, FALSE);
        mbCameraFocusObjSet(work->capObjId1);
        if (capsuleThrowGlowOMObj != NULL) {
            pos.x = effPos.x +
                ((0.5f - MBCapsuleEffRandF()) * lbl_802C4530 * 0.75f);
            pos.y = effPos.y +
                ((0.5f - MBCapsuleEffRandF()) * lbl_802C4530 * 0.75f);
            pos.z = effPos.z +
                ((0.5f - MBCapsuleEffRandF()) * lbl_802C4530 * 0.75f);
            vel.x = vel.y = vel.z = 0.0f;
            color = capsulePlayerThrowColorTbl[
                mbCapColorGet(work->capsuleNo)];
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            glowColor = color;
            glowColorP = &glowColor;
            glowVel = vel;
            glowVelP = &glowVel;
            glowPos = pos;
            glowPosP = &glowPos;
            mbev_CapEffGlowAdd(capsuleThrowGlowOMObj, glowPosP, glowVelP,
                (int)(60.0f * (1.0f + MBCapsuleEffRandF())),
                lbl_802C4530 *
                    (0.2f + (0.025f * MBCapsuleEffRandF())),
                0.0f, 0.025f, glowColorP);
        }
        HuPrcVSleep();
    } while (work->time < work->maxTime);
    if (seNo != MSM_SENO_NONE) {
        mbAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    mbAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->masuPos;
    effPos.y -= 10000.0f;
    CapEffTrailPosSet(&effPos);
    mbObjDispSet(work->capObjId0, FALSE);
    mbObjDispSet(work->capObjId1, FALSE);
    mbCapObjColorDispSet(work->objColorId, FALSE);
    omVibrate(work->playerNo, 20, 4, 4);
    coinNum = CapEffThrowMasu(
        work->masuId, work->capsuleNo, work->playerNo, TRUE);
    CapEffThrowMasuWait(coinNum);
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        mbTutorialCall(18);
    }
    CapThrowEndWin(work->masuId, work->capsuleNo);
    mbev_CapPlayerMotShiftWait(
        work->playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
    mbPlayerRotateStart(work->playerNo, 0, 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    mbCameraFocusPlayerSet(work->playerNo);
    mbCameraMoveOnSet(FALSE);
    if (capsuleThrowHook) {
        capsuleThrowHook(FALSE);
    }
    mbCameraMoveWait();
    mbCameraMoveOnSet(TRUE);
    mbev_CapBonusCoinCall(
        work->playerNo, work->capsuleNo, coinNum, TRUE);
    if (!capsuleUseRemoveOnF) {
        mbPlayerCapsuleUseSet(work->capsuleNo);
        capsuleSlot =
            mbPlayerCapsuleFind(work->playerNo, work->capsuleNo);
        if (capsuleSlot != -1) {
            mbPlayerCapsuleRemove(work->playerNo, capsuleSlot);
        }
    }
    if (work->jumpMotId != MB_MODEL_NONE && !mbExitCheck()) {
        mbPlayerMotionKill(work->playerNo, work->jumpMotId);
    }
    work->jumpMotId = MB_MODEL_NONE;
    HuPrcEnd();
}


static void CapEffThrowHook(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFF_THROW_WORK *work = modelP->hookData;
    CAP_PLAYER_THROW_FRAME *frame;
    HuVecF pos;
    HuVecF dir;
    HuVecF offset;
    HuVecF hitPos;
    float mag;
    float mag2;
    float angle;

    if (work->endF) {
        return;
    }
    if (!work->initF) {
        frame = &capsulePlayerThrowFrameTbl[work->no];
        PSVECSubtract(&work->endPos, &work->startPos, &pos);
        PSVECScale(&pos, &pos, frame->speed);
        PSVECAdd(&pos, &work->startPos, &pos);
        if (pos.y < work->startPos.y) {
            pos.y = work->startPos.y;
        } else if (pos.y < work->endPos.y) {
            pos.y = work->endPos.y;
        }
        pos.y += work->yOfs + frame->yOfs;
        if (frame->radius != 0.0f) {
            PSVECSubtract(&work->endPos, &work->startPos, &dir);
            if (PSVECMag(&dir) > 0.0f) {
                PSVECNormalize(&dir, &dir);
            }
            angle = (float)(90.0 +
                (180.0 * (atan2(dir.x, dir.z) / M_PI)));
            offset.x = frame->radius * sin((M_PI * angle) / 180.0);
            offset.y = 0.0f;
            offset.z = frame->radius * cos((M_PI * angle) / 180.0);
            pos.x += offset.x;
            pos.z += offset.z;
        } else if (frame->dir != 0) {
            PSVECSubtract(&work->endPos, &work->startPos, &dir);
            mag = PSVECMag(&dir);
            if (PSVECMag(&dir) > 0.0f) {
                PSVECNormalize(&dir, &dir);
            }
            offset.x = dir.z * mag * frame->dir;
            offset.y = 0.0f;
            offset.z = dir.x * mag * frame->dir;
            pos.x += offset.z;
            pos.z += offset.x;
        }
        work->x[0] = pos.x;
        work->y[0] = pos.y;
        work->z[0] = pos.z;
        CapThrowCameraSet(work->x, work->y, work->z, 3);
        PSVECSubtract(&pos, &work->startPos, &dir);
        mag = PSVECMag(&dir);
        PSVECSubtract(&work->endPos, &pos, &dir);
        mag2 = PSVECMag(&dir);
        work->maxTime = (int)((mag + mag2) / 25.0f);
        if ((float)work->maxTime < 24.0f) {
            work->maxTime = 24;
        }
        work->pos = work->startPos;
        work->initF = 1;
    }
    work->tick = OSGetTick();
    while ((float)work->initF < (0.5f * (float)work->maxTime)) {
        CapThrowCameraCalc((float)work->initF /
            (0.5f * (float)work->maxTime),
            work->x, work->y, work->z, &pos, 3);
        if (!CapColCheck(&work->pos, &pos, &hitPos) &&
            (work->playerNo == -2 ||
             !CapColExec(work->playerNo, &work->pos, &pos, &hitPos))) {
            work->pos = pos;
            work->initF++;
            if (OSGetTick() - work->tick <= work->delay) {
                continue;
            }
        }
        break;
    }
    if ((float)work->initF >= (0.5f * (float)work->maxTime)) {
        work->endF = TRUE;
        return;
    }
    work->initF = 0;
    work->no++;
    if (work->no >= 48) {
        if (work->playerNo != -2) {
            work->no = 0;
            work->playerNo = -2;
            return;
        }
        work->no = 0;
        work->endF = TRUE;
    }
}

static int CapEffThrowMasu(int masuId, int capsuleNo, int playerNo, BOOL bonusF)
{
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    HuVecF rot2;
    GXColor color;
    GXColor ringColor;
    int coinNum;
    int coinLeft;
    int coinDelay;
    int time;
    int i;
    int life;
    float alpha;
    float scale;
    float scaleY;
    float randF;

    if (!bonusF || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        coinNum = -1;
    } else {
        coinNum = mbCapBonusCoinNumGet(playerNo, capsuleNo);
    }
    coinLeft = coinNum;
    coinDelay = 30;
    vel.x = vel.y = vel.z = 0.0f;
    mbMasuPosGet(masuId, &pos);
    mbMasuRotGet(masuId, &rot);
    CapEffCrackAdd(&pos, &rot);
    mbMasuPosGet(masuId, &pos);
    rot.x = -90.0f;
    rot.y = rot.z = 0.0f;
    vel.x = 2.0f;
    vel.y = 4.0f;
    vel.z = 100.0f * (1.0f + (0.25f * MBCapsuleEffRandF()));
    mbev_CapEffColorSet(&color, mbRandMod(CAPSULE_EFF_COLOR_RANGE));
    ringColor = color;
    mbev_CapEffRingAdd(capsuleThrowRingOMObj, &pos, &rot, &vel,
        3, 15, 1, &ringColor);
    for (time = 0; coinLeft > 0 || time <= 60; time++) {
        if (time > 60) {
            time = 60;
        }
        if (time == 20) {
            mbCapMasuCapsuleSet(masuId, capsuleNo, playerNo);
            mbAudFXPlay(MSM_SE_BOARD_28);
        }
        if (time == 10 || time == 20) {
            mbMasuPosGet(masuId, &pos);
            rot.x = -90.0f;
            rot.y = rot.z = 0.0f;
            vel.x = 2.0f;
            vel.y = 6.0f;
            randF = (1.0f / CAPSULE_EFF_COLOR_RANGE) *
                (s16)mbCapEffData[mbCapEffNum++];
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            vel.z = 100.0f * (1.0f + (0.25f * randF));
            mbev_CapEffColorSet(&color,
                (u16)mbCapEffData[mbCapEffNum++]);
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            ringColor = color;
            mbev_CapEffRingAdd(capsuleThrowRingOMObj, &pos, &rot, &vel,
                3, 10, 2, &ringColor);
        }
        coinDelay--;
        if (coinDelay <= 0 && coinLeft > 0) {
            mbMasuPosGet(masuId, &pos);
            mbev_CapEffCoinMultiAdd(capsuleThrowMasuCoinOMObj, &pos, 1);
            coinLeft--;
            randF = (1.0f / CAPSULE_EFF_COLOR_RANGE) *
                (s16)mbCapEffData[mbCapEffNum++];
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            coinDelay = (int)(3.0f * randF);
        }
        if (masuId > 0) {
            mbMasuPosGet(masuId, &pos);
            mbMasuRotGet(masuId, &rot);
            mbev_CapEffRayTransformSet(
                capsuleThrowRayOMObj, &pos, &rot, NULL);
            mbev_CapEffMasuHitTransformSet(
                capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
        }
        alpha = cos((M_PI * (90.0f * ((float)time / 60.0f))) / 180.0);
        for (i = 0; (float)i < (2.0f * alpha); i++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot.y = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot.z = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.x = rot.x + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.y = rot.y + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.z = rot.z + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            life = (int)(20.0f + (5.0f * MBCapsuleEffRandF()));
            scale = 200.0f * (1.0f +
                ((1.0f / CAPSULE_EFF_COLOR_RANGE) * (s16)mbCapEffData[mbCapEffNum++]));
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            mbev_CapEffRayAdd(capsuleThrowRayOMObj,
                &pos, &rot, &rot2, life, scale);
        }
        mbev_CapEffRayAlphaSet(capsuleThrowRayOMObj, alpha);
        for (i = 0; (float)i < (5.0f * alpha); i++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot.y = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot.z = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.x = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.y = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.z = 360.0f * (-0.5f + MBCapsuleEffRandF());
            scale = 100.0f * alpha * (1.5f +
                ((1.0f / CAPSULE_EFF_COLOR_RANGE) * (s16)mbCapEffData[mbCapEffNum++]));
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            scaleY = 25.0f * alpha * (1.0f + (0.5f *
                ((1.0f / CAPSULE_EFF_COLOR_RANGE) *
                 (s16)mbCapEffData[mbCapEffNum++])));
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            life = (int)(20.0f + (10.0f *
                ((1.0f / CAPSULE_EFF_COLOR_RANGE) *
                 (s16)mbCapEffData[mbCapEffNum++])));
            if (mbCapEffNum >= 1024) {
                mbCapEffNum = 0;
            }
            mbev_CapEffMasuHitAdd(capsuleThrowMasuHitOMObj,
                &pos, &rot, &rot2, scale, scaleY, life);
        }
        HuPrcVSleep();
    }
    return coinNum;
}

static BOOL CapEffThrowCheck(HuVecF *pos, int *maxTime)
{
    HU3D_MODEL *model;
    CAP_EFF_THROW_WORK *work;

    if (capEffThrowMdlId == MB_MODEL_NONE) {
        return FALSE;
    }
    model = &Hu3DData[capEffThrowMdlId];
    work = model->hookData;
    if (work->endF == FALSE) {
        return FALSE;
    }
    pos->x = work->x[1];
    pos->y = work->y[1];
    pos->z = work->z[1];
    *maxTime = work->maxTime;
    return TRUE;
}

static void CapEffThrowKill(void)
{
    if (capEffThrowMdlId != MB_MODEL_NONE) {
        Hu3DModelKill(capEffThrowMdlId);
    }
    capEffThrowMdlId = MB_MODEL_NONE;
}

static void CapEffThrowCreate(int playerNo, float *x, float *y, float *z,
    float yOfs, int masuId)
{
    HU3D_MODEL *model;
    CAP_EFF_THROW_WORK *work;
    int boardNo;

    capEffThrowMdlId = Hu3DHookFuncCreate(CapEffThrowHook);
    Hu3DModelCameraSet(capEffThrowMdlId, 1);
    Hu3DModelLayerSet(capEffThrowMdlId, 7);
    model = &Hu3DData[capEffThrowMdlId];
    work = HuMemDirectMallocNum(HEAP_MODEL, sizeof(*work), model->mallocNo);
    model->hookData = work;
    memset(work, 0, sizeof(*work));
    work->modelId = capEffThrowMdlId;
    work->playerNo = -2;
    work->startPos.x = x[0];
    work->startPos.y = y[0];
    work->startPos.z = z[0];
    work->endPos.x = x[2];
    work->endPos.y = y[2];
    work->endPos.z = z[2];
    work->yOfs = yOfs;
    boardNo = GwSystem.boardNo;
    if (boardNo > 10) {
        boardNo = 10;
    }
    work->delay = capsulePlayerThrowDelayTbl[boardNo];
    work->x[0] = x[0];
    work->x[1] = x[1];
    work->x[2] = x[2];
    work->y[0] = y[0];
    work->y[1] = y[1];
    work->y[2] = y[2];
    work->z[0] = z[0];
    work->z[1] = z[1];
    work->z[2] = z[2];
}

static void CapEffThrowMasuCreate(int masuId, int capsuleNo)
{
    HuVecF pos;
    HuVecF rot;

    if (masuId > 0) {
        mbMasuPosGet((s16)masuId, &pos);
        mbMasuRotGet((s16)masuId, &rot);
    }
    capsuleThrowGlowOMObj = mbev_CapEffGlowCreate();
    capsuleThrowRingOMObj = mbev_CapEffRingCreate();
    capsuleThrowRayOMObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    if (masuId > 0) {
        mbev_CapEffRayTransformSet(capsuleThrowRayOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuHitOMObj = mbev_CapEffMasuHitCreate();
    if (masuId > 0) {
        mbev_CapEffMasuHitTransformSet(
            capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuCoinOMObj = mbev_CapEffCoinCreate();
    CapEffTrailCreate(capsuleNo);
    CapEffCrackCreate();
}

static BOOL CapEffThrowMasuWait(BOOL waitGlowF)
{
    HuPrcSleep(18);
    while (mbev_CapEffCoinNumGet(capsuleThrowMasuCoinOMObj) > 0) {
        HuPrcVSleep();
    }
    if (waitGlowF <= 0) {
        while (mbev_CapEffGlowDispGet(capsuleThrowGlowOMObj) != 0) {
            HuPrcVSleep();
        }
    }
    if (capsuleThrowGlowOMObj != NULL) {
        mbev_CapEffGlowKill(capsuleThrowGlowOMObj);
    }
    if (capsuleThrowRingOMObj != NULL) {
        mbev_CapEffRingKill(capsuleThrowRingOMObj);
    }
    if (capsuleThrowRayOMObj != NULL) {
        mbev_CapEffRayKill(capsuleThrowRayOMObj);
    }
    if (capsuleThrowMasuHitOMObj != NULL) {
        mbev_CapEffMasuHitKill(capsuleThrowMasuHitOMObj);
    }
    if (capsuleThrowMasuCoinOMObj != NULL) {
        mbev_CapEffCoinKill(capsuleThrowMasuCoinOMObj);
    }
    CapEffCrackKill();
    CapEffTrailKill();
    return TRUE;
}

void mbCapThrowHookSet(CAPSULE_THROW_HOOK hook)
{
    capsuleThrowHook = hook;
}

void mbCapEffThrowCreate(HuVecF *startPos, HuVecF *endPos)
{
    HuVecF delta;
    float x[3];
    float y[3];
    float z[3];
    float yOfs;

    x[0] = startPos->x;
    x[1] = startPos->x;
    x[2] = endPos->x;
    y[0] = startPos->y;
    y[1] = startPos->y;
    y[2] = endPos->y;
    z[0] = startPos->z;
    z[1] = startPos->z;
    z[2] = endPos->z;
    PSVECSubtract(endPos, startPos, &delta);
    yOfs = lbl_802C4570 + (lbl_802C4574 * PSVECMag(&delta));
    if (yOfs > lbl_802C4578) {
        yOfs = lbl_802C4578;
    }
    CapEffThrowCreate(-1, x, y, z, yOfs, -1);
}

BOOL mbCapEffThrowCheck(HuVecF *pos, int *maxTime)
{
    CAP_EFF_THROW_WORK *work;
    BOOL result;

    result = FALSE;
    if (capEffThrowMdlId != MB_MODEL_NONE) {
        work = Hu3DData[capEffThrowMdlId].hookData;
        if (work->endF) {
            pos->x = work->x[1];
            pos->y = work->y[1];
            pos->z = work->z[1];
            *maxTime = work->maxTime;
            result = TRUE;
        }
    }
    if (result) {
        if (capEffThrowMdlId != MB_MODEL_NONE) {
            Hu3DModelKill(capEffThrowMdlId);
        }
        capEffThrowMdlId = MB_MODEL_NONE;
    }
    return result;
}

void mbCapAutoThrow(HuVecF *startPos, HuVecF *endPos, HuVecF *masuPos, int playerNo,
    int masuId, int capsuleNo, BOOL maxTime, float startT)
{
    CAP_AUTO_THROW_WORK *workData;
    CAP_AUTO_THROW_WORK *work;

    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*workData), HU_MEMNUM_OVL);
    work = workData;
    memset(work, 0, sizeof(*work));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->maxTime = maxTime;
    work->startT = startT;
    work->startPos = *startPos;
    work->endPos = *endPos;
    work->masuPos = *masuPos;
    CapAutoThrow(work);
    mbCapAutoThrowEnd(work);
    HuMemDirectFree(work);
}

static void CapAutoThrow(CAP_AUTO_THROW_WORK *work)
{
    int capsuleNo;
    int masuId;
    int time;
    int seNo;
    int objColorId;
    float t;
    float glowScale;
    GXColor color;
    GXColor glowColor;
    HuVecF effPos;
    HuVecF particlePos;
    HuVecF particleVel;
    float x[3];
    float y[3];
    float z[3];
    HuVecF throwOut;
    HuVecF glowPos;
    HuVecF glowVel;
    HuVecF rot;
    HuVecF pos;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;

    if (capsuleThrowHook) {
        capsuleThrowHook(TRUE);
    }
    capsuleNo = work->capsuleNo;
    masuId = work->masuId;
    if (masuId > 0) {
        mbMasuPosGet((s16)masuId, &pos);
        mbMasuRotGet((s16)masuId, &rot);
    }
    capsuleThrowGlowOMObj = mbev_CapEffGlowCreate();
    capsuleThrowRingOMObj = mbev_CapEffRingCreate();
    capsuleThrowRayOMObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    if (masuId > 0) {
        mbev_CapEffRayTransformSet(capsuleThrowRayOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuHitOMObj = mbev_CapEffMasuHitCreate();
    if (masuId > 0) {
        mbev_CapEffMasuHitTransformSet(
            capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuCoinOMObj = mbev_CapEffCoinCreate();
    CapEffTrailCreate(capsuleNo);
    CapEffCrackCreate();
    x[0] = work->startPos.x;
    x[1] = work->endPos.x;
    x[2] = work->masuPos.x;
    y[0] = work->startPos.y;
    y[1] = work->endPos.y;
    y[2] = work->masuPos.y;
    z[0] = work->startPos.z;
    z[1] = work->endPos.z;
    z[2] = work->masuPos.z;
    objColorId = mbCapObjColorCreate(work->capsuleNo, FALSE);
    mbCapObjColorPosSetV(objColorId, &work->startPos);
    mbCapObjColorScaleSet(objColorId, 1.0f, 1.0f, 1.0f);
    mbCapObjColorLayerSet(objColorId, 4);
    if (work->startT < 1.0f) {
        mbCameraMoveMasu((s16)work->masuId, NULL, NULL, 1600.0f, -1.0f, -1);
        mbCameraMoveWait();
        mbWipeSpecialFadeOutCreate(1, 60);
    }
    if (work->startT < 1.0f) {
        CapEffTrailAdd(&work->startPos, work->capsuleNo);
    }
    if (work->startT < 1.0f) {
        seNo = mbAudFXPlay(MSM_SE_BOARD_26);
    } else {
        seNo = -1;
    }
    time = work->startT * work->maxTime;
    do {
        if (work->startT >= 1.0f) {
            break;
        }
        time++;
        t = (float)time / (float)work->maxTime;
        CapThrowCameraCalc(t, x, y, z, &throwOut, 3);
        effPos.x = throwOut.x;
        effPos.y = throwOut.y;
        effPos.z = throwOut.z;
        mbCapObjColorPosSet(objColorId, effPos.x, effPos.y, effPos.z);
        mbCapObjColorScaleSet(objColorId, 0.5f, 0.5f, 0.5f);
        if (work->startT < 1.0f) {
            CapEffTrailPosSet(&effPos);
        }
        if (capsuleThrowGlowOMObj != NULL) {
            particlePos.x = effPos.x
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particlePos.y = effPos.y
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particlePos.z = effPos.z
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particleVel.x = particleVel.y = particleVel.z = 0.0f;
            color = capsuleAutoThrowColorTbl[mbCapColorGet(work->capsuleNo)];
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            glowColor = color;
            glowColorP = &glowColor;
            glowVel = particleVel;
            glowVelP = &glowVel;
            glowPos = particlePos;
            glowPosP = &glowPos;
            glowScale = 100.0f
                * (0.2f + (0.025f * MBCapsuleEffRandF()));
            mbev_CapEffGlowAdd(capsuleThrowGlowOMObj, glowPosP, glowVelP,
                (int)(60.0f * (1.0f + MBCapsuleEffRandF())), glowScale,
                0.0f, 0.025f, glowColorP);
        }
        HuPrcVSleep();
    } while (time < work->maxTime);
    if (seNo != -1) {
        mbAudFXStop(seNo);
    }
    seNo = -1;
    mbAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->endPos;
    effPos.y -= 10000.0f;
    CapEffTrailPosSet(&effPos);
    mbCapObjColorKill(objColorId);
    if (work->startT < 1.0f) {
        omVibrate(work->playerNo, 20, 7, 3);
    }
    CapEffThrowMasu(work->masuId, work->capsuleNo, work->playerNo, FALSE);
    HuPrcSleep(18);
    while (mbev_CapEffCoinNumGet(capsuleThrowMasuCoinOMObj) > 0) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(capsuleThrowGlowOMObj) != 0) {
        HuPrcVSleep();
    }
    if (capsuleThrowGlowOMObj != NULL) {
        mbev_CapEffGlowKill(capsuleThrowGlowOMObj);
    }
    if (capsuleThrowRingOMObj != NULL) {
        mbev_CapEffRingKill(capsuleThrowRingOMObj);
    }
    if (capsuleThrowRayOMObj != NULL) {
        mbev_CapEffRayKill(capsuleThrowRayOMObj);
    }
    if (capsuleThrowMasuHitOMObj != NULL) {
        mbev_CapEffMasuHitKill(capsuleThrowMasuHitOMObj);
    }
    if (capsuleThrowMasuCoinOMObj != NULL) {
        mbev_CapEffCoinKill(capsuleThrowMasuCoinOMObj);
    }
    CapEffCrackKill();
    CapEffTrailKill();
    if (work->masuId > 0) {
        CapThrowEndWin(work->masuId, work->capsuleNo);
    }
    if (work->startT < 1.0f) {
        mbWipeSpecialFadeInCreate(1, 1);
    }
    if (capsuleThrowHook) {
        capsuleThrowHook(FALSE);
    }
}

void mbCapAutoThrowEnd(CAP_AUTO_THROW_WORK *work)
{
}

static void CapGuideOMExec(OMOBJ *obj)
{
    CAP_GUIDE_WORK *work = obj->data;
    int i;

    if (mbExitCheck() || capsuleGuideOMObj == NULL) {
        for (i = 0; i < 10; i++, work++) {
            mbObjKill(work->objId);
        }
        omDelObjEx(mbObjMan, obj);
        capsuleGuideOMObj = NULL;
        return;
    }
    if (capsuleGuideOMObj->work[1] != 0) {
        capsuleGuideOMObj->work[1]--;
        return;
    }
    for (i = 0; i < 10; i++, work++) {
        if (!mbObjDispGet(work->objId)) {
            continue;
        }
        if (capsuleGuideOMObj->work[0] == 0) {
            if (work->state == 0) {
                if (work->scale >= lbl_802C45C0) {
                    work->scale = lbl_802C45C0;
                    work->state = 1;
                } else {
                    work->scale += 0.2;
                }
                mbObjScaleSet(work->objId, work->scale, work->scale, work->scale);
            } else if (work->state != 1) {
                continue;
            }
            work->rotY += 20;
            if (work->rotY > 360) {
                work->rotY -= 360;
            }
            work->scale = (float)(1.0 + (0.2 * sin((M_PI * work->rotY) / 180.0)));
        } else {
            work->scale -= lbl_802C455C;
            if (work->scale < 0.0f) {
                work->state = 0;
                mbObjDispSet(work->objId, FALSE);
            }
            mbObjScaleSet(work->objId, work->scale, work->scale, work->scale);
        }
    }
}

static void CapEffMasuOkOMExec(OMOBJ *obj)
{
    CAP_EFF_MASU_OK_WORK *work = obj->data;
    HuVecF pos;
    HuVecF rot;
    int i;

    if (mbExitCheck() || capEffMasuOkOMObj == NULL) {
        for (i = 0; i < 32; i++, work++) {
            mbObjKill(work->modelId);
        }
        mbObjKill(work->modelId);
        omDelObjEx(mbObjMan, obj);
        capEffMasuOkOMObj = NULL;
        return;
    }
    for (i = 0; i < 32; i++, work++) {
        if (work->masuId > 0) {
            mbMasuPosGet(work->masuId, &pos);
            mbMasuRotGet(work->masuId, &rot);
            pos.y += lbl_802C4544;
            mbObjPosSetV(work->modelId, &pos);
            mbObjRotSetV(work->modelId, &rot);
        }
    }
    if (work->masuId > 0) {
        mbMasuPosGet(work->masuId, &pos);
        mbMasuRotGet(work->masuId, &rot);
        pos.y += lbl_802C4544;
        mbObjPosSetV(work->modelId, &pos);
        mbObjRotSetV(work->modelId, &rot);
    }
    switch (work->state) {
        case 1:
            work->state++;
            break;

        case 2:
            work->scale += lbl_802C455C;
            if (work->scale >= 1.0f) {
                work->scale = 1.0f;
                work->state++;
            }
            mbObjScaleSet(work->modelId, work->scale, work->scale, work->scale);
            break;

        case 10:
            work->scale -= lbl_802C455C;
            if (work->scale <= lbl_802C4618) {
                mbObjDispSet(work->modelId, FALSE);
                work->scale = lbl_802C4618;
                work->state++;
            }
            mbObjScaleSet(work->modelId, work->scale, work->scale, work->scale);
            break;
    }
}

static void CapEffRemoveOMExec(OMOBJ *obj)
{
    CAP_EFF_REMOVE_WORK *work = obj->data;
    CAP_EFFECT *effect;
    CAP_EFF_DATA *data;
    int i;

    if (mbExitCheck() || capEffRemoveOMObj == NULL) {
        Hu3DModelKill(work->modelId);
        work->modelId = MB_MODEL_NONE;
        HuSprAnimKill(work->anim);
        work->anim = NULL;
        omDelObjEx(mbObjMan, obj);
        capEffRemoveOMObj = NULL;
        return;
    }
    if (work->activeCount < 1) {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    effect = Hu3DData[work->modelId].hookData;
    effect->unk23 = 0;
    data = effect->data;
    for (i = 0; i < effect->num; i++, data++) {
        if (data->scale > 0.0f) {
            data->pos.x += data->vel.x;
            data->pos.y += data->vel.y;
            data->pos.z += data->vel.z;
            data->rot.y += data->speed;
            if (data->rot.y >= 360.0f) {
                data->rot.y -= 360.0f;
            }
            data->no = (int)data->animTime;
            data->animTime += data->animSpeed;
            if (data->no > 15) {
                data->no = 0;
                data->time = 0;
                data->scale = 0.0f;
                work->activeCount--;
            }
        }
    }
}

static void CapEffCrackCreate(void)
{
    CAP_EFF_CRACK_WORK *work;
    CAP_EFF_CRACK_DATA *data;
    HU3D_MODEL *model;
    HuVecF pos;
    void *dl;
    int i;
    int j;
    int k;
    int l;
    float x;
    float z;
    float length;
    float crackScale;

    capEffCrackOMObj = omAddObjEx(mbObjMan, -32768, 0, 0, OM_GRP_NONE,
        CapEffCrackOMExec);
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    capEffCrackOMObj->data = work;
    memset(work, 0, sizeof(*work));
    work->modelId = Hu3DHookFuncCreate(CapEffCrackDraw);
    Hu3DModelCameraSet(work->modelId, 1);
    Hu3DModelLayerSet(work->modelId, 3);
    model = &Hu3DData[work->modelId];
    model->hookData = work;
    work->num = 24 * 24 * 2;
    work->vtxNum = work->num * 3;
    work->data = HuMemDirectMallocNum(HEAP_MODEL,
        work->num * sizeof(*work->data), model->mallocNo);
    memset(work->data, 0, work->num * sizeof(*work->data));
    work->vtx = HuMemDirectMallocNum(HEAP_MODEL,
        work->vtxNum * sizeof(*work->vtx), model->mallocNo);
    memset(work->vtx, 0, work->vtxNum * sizeof(*work->vtx));
    work->st = HuMemDirectMallocNum(HEAP_MODEL,
        work->vtxNum * sizeof(*work->st), model->mallocNo);
    memset(work->st, 0, work->vtxNum * sizeof(*work->st));
    work->animP = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_capsule, 43),
        HU_MEMNUM_OVL));
    work->color.r = work->color.g = work->color.b = work->color.a = 0;

    crackScale = 200.0f / 24.0f;
    data = work->data;
    z = -100.0f;
    for (i = 0; i < 24; i++, z += crackScale) {
        x = -100.0f;
        for (j = 0; j < 24; j++, x += crackScale) {
            for (k = 0; k < 2; k++, data++) {
                data->flag = TRUE;
                data->scale = 1.0f;
                data->angle = 0.0f;
                data->angleSpeed = 5.0f + (200.0f * MBCapsuleEffRandF());
                if (mbRandMod(CAPSULE_EFF_COLOR_RANGE) & 1) {
                    data->angleSpeed = -data->angleSpeed;
                }
                data->scaleSpeed = 0.035f + (0.035f * MBCapsuleEffRandF());
                data->delay = mbRandMod(CAPSULE_EFF_COLOR_RANGE) & 15;
                for (l = 0; l < 3; l++) {
                    data->pos[l].x = x + (crackScale * capsuleCrackScaleTbl[k][l].x);
                    data->pos[l].y = 0.0f;
                    data->pos[l].z = z + (crackScale * capsuleCrackScaleTbl[k][l].z);
                    length = PSVECMag(&data->pos[l]);
                    if (length > 100.0f) {
                        PSVECNormalize(&data->pos[l], &data->pos[l]);
                        PSVECScale(&data->pos[l], &data->pos[l], 100.0f);
                        length = 100.0f;
                    }
                    data->pos[l].y = sqrtf((100.0f * 100.0f) - (length * length));
                    data->uv[l].x = (data->pos[l].x + 100.0f) / 200.0f;
                    data->uv[l].y = (data->pos[l].z + 100.0f) / 200.0f;
                }
                pos = data->pos[0];
                PSVECAdd(&pos, &data->pos[1], &pos);
                PSVECAdd(&pos, &data->pos[2], &pos);
                PSVECScale(&pos, &data->vel, 1.0f / 3.0f);
                if (PSVECMag(&data->vel) > 0.0f) {
                    PSVECNormalize(&data->vel, &data->accel);
                }
                PSVECScale(&data->accel, &data->accel,
                    3.0f * (0.5f + (0.5f * MBCapsuleEffRandF())));
                data->prevVel = data->vel;
                for (l = 0; l < 3; l++) {
                    PSVECAdd(&data->pos[l], &data->vel, &pos);
                    data->prevPos[l] = pos;
                }
                data->color = capsuleCrackEffColor;
            }
        }
    }

    dl = HuMemDirectMallocNum(HEAP_MODEL, 65536, model->mallocNo);
    GXBeginDisplayList(dl, 65536);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, work->vtxNum);
    for (i = 0; i < work->num; i++) {
        GXPosition1x16(3 * i);
        GXColor1x16(i);
        GXTexCoord1x16(3 * i);
        GXPosition1x16((3 * i) + 1);
        GXColor1x16(i);
        GXTexCoord1x16((3 * i) + 1);
        GXPosition1x16((3 * i) + 2);
        GXColor1x16(i);
        GXTexCoord1x16((3 * i) + 2);
    }
    work->dlSize = GXEndDisplayList();
    work->dl = HuMemDirectMallocNum(HEAP_MODEL, work->dlSize, model->mallocNo);
    memcpy(work->dl, dl, work->dlSize);
    DCFlushRange(work->dl, work->dlSize);
    HuMemDirectFree(dl);
}

static void CapEffCrackOMExec(OMOBJ *obj)
{
    CAP_EFF_CRACK_WORK *work = obj->data;
    CAP_EFF_CRACK_DATA *data;
    float scale;
    int i;

    if (mbExitCheck() || capEffCrackOMObj == NULL) {
        HuSprAnimKill(work->animP);
        Hu3DModelKill(work->modelId);
        omDelObjEx(mbObjMan, obj);
        capEffCrackOMObj = NULL;
        return;
    }
    switch (work->state) {
        case 1:
            data = work->data;
            for (i = 0; i < work->num; i++, data++) {
                data->color.a = (u8)lbl_802C4660;
                data->vel.y = 0.0f;
                data->pos[0].y = 0.0f;
                data->pos[1].y = 0.0f;
                data->pos[2].y = 0.0f;
            }
            Hu3DModelScaleSet(work->modelId, 0.0f, 1.0f, 0.0f);
            work->time = 0;
            work->state++;
            break;

        case 2:
            work->time++;
            scale = (float)work->time / 6.0f;
            if (scale >= 1.0f) {
                scale = 1.0f;
            }
            Hu3DModelScaleSet(work->modelId, scale, 1.0f, scale);
            if (work->time >= 6) {
                work->time = 0;
                work->state++;
            }
            break;

        case 3:
            work->time++;
            scale = lbl_802C465C * (float)work->time;
            if (scale >= 1.0f) {
                scale = 1.0f;
            }
            data = work->data;
            for (i = 0; i < work->num; i++, data++) {
                data->color.a = (u8)(lbl_802C4660 + (lbl_802C4664 * scale));
                data->vel.y = data->prevVel.y * scale;
                data->pos[0].y = data->prevPos[0].y * scale;
                data->pos[1].y = data->prevPos[1].y * scale;
                data->pos[2].y = data->prevPos[2].y * scale;
            }
            work->color.r = (u8)(lbl_802C4660 * scale);
            work->color.g = (u8)(lbl_802C4660 * scale);
            work->color.b = (u8)(lbl_802C4660 * scale);
            work->color.a = (u8)(lbl_802C4668 * scale);
            if (work->time >= 18) {
                work->time = 0;
                work->state++;
            }
            break;

        case 4:
            if (work->color.a < 21) {
                work->color.a = 0;
            } else {
                work->color.a -= 20;
            }
            data = work->data;
            for (i = 0; i < work->num; i++, data++) {
                if (!data->flag) {
                    continue;
                }
                if (data->delay != 0) {
                    data->delay--;
                    continue;
                }
                PSVECAdd(&data->vel, &data->accel, &data->vel);
                data->angle += data->angleSpeed;
                data->scale -= data->scaleSpeed;
                if (data->scale > 0.0f) {
                    if (data->scale <= 1.0f) {
                        data->color.a = (u8)(lbl_802C4668 * data->scale);
                    } else {
                        data->color.a = (u8)lbl_802C4668;
                    }
                } else {
                    data->flag = FALSE;
                }
            }
            break;
    }
}

int mbCapMasuNextGet(int playerNo)
{
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    CAPSULE_LIST temp;
    u8 *weight;
    int weightNo;
    int capsuleNo;
    int total;
    int count;
    int i;
    int j;
    int listNo;
    int otherListNo;

    if ((GwSystem.turnMax >> 1) < GwSystem.turnNo) {
        weightNo = GwPlayer[playerNo].rank + 4;
    } else {
        weightNo = GwPlayer[playerNo].rank;
    }
    listBase = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    list = listBase;
    count = 0;
    total = 0;
    for (i = 0; i < 33 && capsuleList[i].id != -1; i++) {
        weight = &capsuleList[i].unk04;
        if (weight[weightNo] > 0) {
            *list = capsuleList[i];
            weight = &list->unk04;
            for (j = 0; j < mbPlayerCapsuleNumGet(playerNo); j++) {
                if (list->id == mbPlayerCapsuleGet(playerNo, j)) {
                    weight[weightNo] /= 2;
                }
            }
            total += weight[weightNo];
            list++;
            count++;
        }
    }
    if (count < 2) {
        if (count == 1) {
            capsuleNo = listBase[0].id;
        } else {
            capsuleNo = -1;
        }
    } else {
        for (i = 0; i < 256; i++) {
            listNo = mbRandMod(count);
            otherListNo = mbRandMod(count);
            if (listNo != otherListNo) {
                temp = listBase[listNo];
                listBase[listNo] = listBase[otherListNo];
                listBase[otherListNo] = temp;
            }
        }
        for (i = 0; i < count - 1; i++) {
            for (j = i + 1; j < count; j++) {
                u8 *otherWeight;

                weight = &listBase[i].unk04;
                otherWeight = &listBase[j].unk04;
                if (otherWeight[weightNo] < weight[weightNo]) {
                    temp = listBase[i];
                    listBase[i] = listBase[j];
                    listBase[j] = temp;
                }
            }
        }
        total = (int)((float)total * MBCapsuleEffRandF());
        for (i = 0; i < count; i++) {
            total -= (&listBase[i].unk04)[weightNo];
            if (total <= 0) {
                break;
            }
        }
        if (i < count) {
            capsuleNo = listBase[i].id;
        } else {
            capsuleNo = listBase[0].id;
        }
    }
    HuMemDirectFree(listBase);
    return capsuleNo;
}

void mbCapListInit(CAPSULE_LIST *list)
{
    int i;
    int num;

    for (i = 0, num = 0; i < 33; i++) {
        if (list[i].id == -1) {
            break;
        }
        if (mbCapListExcludeCheck(list[i].id)) {
            capsuleList[num] = list[i];
            num++;
        }
    }
    capsuleList[num].id = -1;
    if (!mbSaveNewF) {
        memset(capsuleNum, 0, sizeof(capsuleNum));
    }
}

void mbCapListRead(void)
{
    CAPSULE_LIST_FILE *file;
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    int i;
    int num;
    int boardNo;

    file = capsuleListFileTbl;
    i = 0;
    for (; file->boardNo >= 0; i++, file++) {
        boardNo = GwSystem.boardNo;
        if (file->boardNo == boardNo) {
            break;
        }
    }
    listBase = list = HuDataSelHeapReadNum(
        file->dataNo, HU_MEMNUM_OVL, HEAP_MODEL);
    i = 0;
    num = 0;
    for (; i < 33; i++, list++) {
        if (list->id == -1) {
            break;
        }
        if (mbCapListExcludeCheck(list->id)) {
            capsuleList[num] = *list;
            num++;
        }
    }
    capsuleList[num].id = -1;
    HuMemDirectFree(listBase);
    if (!mbSaveNewF) {
        memset(capsuleNum, 0, sizeof(capsuleNum));
    }
}

int mbCapListCopy(CAPSULE_LIST *list)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        list[i] = capsuleList[i];
    }
    capsuleList[i].id = -1;
    return i;
}

int mbCapRandomListGet(int *capsuleListOut, int maxNum)
{
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    int count;
    CAPSULE_LIST *listWork;
    CAPSULE_LIST temp;
    int i;
    int listNo;
    int otherListNo;
    int num;

    list = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    listBase = list;
    listWork = listBase;
    memset(listBase, 0, sizeof(*listBase) * 33);
    i = 0;
    count = 0;
    for (; i < 33; i++, listWork++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        *listWork = capsuleList[i];
        count++;
    }
    if (count >= 2) {
        for (i = 0; i < 256; i++) {
            listNo = mbRandMod(count);
            otherListNo = mbRandMod(count);
            if (listNo != otherListNo) {
                temp = listBase[listNo];
                listBase[listNo] = listBase[otherListNo];
                listBase[otherListNo] = temp;
            }
        }
    }
    i = 0;
    num = 0;
    for (; i < maxNum; i++) {
        if (listBase[i].id == -1) {
            break;
        }
        capsuleListOut[i] = listBase[i].id;
        num++;
    }
    HuMemDirectFree(listBase);
    return num;
}

void mbCapNumInc(int capsuleNo, int mode)
{
    capsuleNum[capsuleNo][mode]++;
}

static int capsuleListDefineTbl[24][2] = {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 3, 0 },
    { 4, 0 },
    { 5, 0 },
    { 6, 0 },
    { 10, 0 },
    { 11, 0 },
    { 12, 0 },
    { 13, 0 },
    { 25, 0 },
    { 15, 0 },
    { 16, 0 },
    { 17, 0 },
    { 20, 0 },
    { 21, 0 },
    { 22, 0 },
    { 23, 0 },
    { 24, 0 },
    { 30, 0 },
    { 31, 0 },
    { 32, 0 },
    { -1, 0 },
};

static int capsuleListColW[16] = {
    108, 32, 32, 48,
    32, 32, 32, 32,
    32, 32, 32, 48,
    32, 32, 32, 48,
};

static int capsuleNumColW[16] = {
    108, 32, 32, 48,
    32, 32, 32, 32,
    32, 32, 32, 48,
    32, 32, 32, 48,
};

void mbCapNumDebug(void)
{
    extern char lbl_802BFD90[5];
    extern char lbl_802BFE41[3];
    extern char lbl_802BFE44[5];
    extern const float lbl_802C45F0;
    extern const float lbl_802C45F4;
    static GXColor winColor = { 0, 0, 144, 192 };
    CAPSULE_LIST *list;
    float x;
    float y;
    int color;
    int i;
    int prevColor;
    int j;

    do {
        printWin(4, 96, 630, 270, &winColor);
        x = 8.0f;
        y = 100.0f;
        prevColor = -1;
        color = prevColor;
        i = 0;
        list = capsuleList;
        for (; i < 15; i++, list++) {
            x = lbl_802C45F0;
            for (j = 0; j < 4; j++) {
                if (color != prevColor) {
                    fontcolor = color;
                }
                prevColor = color;
                if (j == 0) {
                    if (list->id != -1) {
                        print8(x, y, 1.5f, lbl_802BFE41, mbCapDebugNameGet(list->id));
                    } else {
                        print8(x, y, 1.5f, lbl_802BFD90);
                    }
                } else if (j == 3) {
                    print8(x, y, 1.5f, lbl_802BFE44,
                        capsuleNum[list->id][0] + capsuleNum[list->id][1]);
                } else {
                    print8(x, y, 1.5f, lbl_802BFE44, capsuleNum[list->id][j - 1]);
                }
                x += capsuleNumColW[j];
            }
            y += lbl_802C45F4;
        }
        HuPrcVSleep();
    } while (!(HuPadBtn[0] & PAD_TRIGGER_R) ||
        !(HuPadBtn[0] & PAD_TRIGGER_L));
}

static void CapEffCrackKill(void)
{
    capEffCrackOMObj = NULL;
}

static void CapEffCrackAdd(HuVecF *pos, HuVecF *rot)
{
    CAP_EFF_CRACK_WORK *work;

    if (capEffCrackOMObj == NULL) {
        return;
    }
    work = capEffCrackOMObj->data;
    work->state = 1;
    if (pos == NULL) {
        capEffCrackOMObj->trans.x = 0.0f;
        capEffCrackOMObj->trans.y = 0.0f;
        capEffCrackOMObj->trans.z = 0.0f;
    } else {
        capEffCrackOMObj->trans = *pos;
    }
    if (rot == NULL) {
        capEffCrackOMObj->rot.x = 0.0f;
        capEffCrackOMObj->rot.y = 0.0f;
        capEffCrackOMObj->rot.z = 0.0f;
    } else {
        capEffCrackOMObj->rot = *rot;
    }
    Hu3DModelPosSet(work->modelId, capEffCrackOMObj->trans.x,
        capEffCrackOMObj->trans.y, capEffCrackOMObj->trans.z);
    Hu3DModelRotSet(work->modelId, capEffCrackOMObj->rot.x,
        capEffCrackOMObj->rot.y, capEffCrackOMObj->rot.z);
}

static void CapEffCrackDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFF_CRACK_WORK *work = modelP->hookData;
    CAP_EFF_CRACK_DATA *data = work->data;
    HuVecF *vtx = work->vtx;
    HuVec2f *st = work->st;
    HuVecF pos;
    float sin;
    float cos;
    int i;
    int j;

    if (work->state == 0) {
        return;
    }
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    if (shadowModelDrawF) {
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    } else {
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    }
    GXSetTevColor(GX_TEVREG2, work->color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_C2);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_A2);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetChanAmbColor(GX_COLOR0A0, capsuleCrackEffAmbColor);
    GXSetChanMatColor(GX_COLOR0A0, capsuleCrackEffMatColor);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL,
        GX_DF_CLAMP, GX_AF_NONE);
    HuSprTexLoad(work->animP, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_NONE);

    for (i = 0; i < work->num; i++, data++) {
        if (!data->flag) {
            for (j = 0; j < 3; j++, vtx++, st++) {
                vtx->x = vtx->y = vtx->z = 0.0f;
                st->x = st->y = 0.0f;
            }
            continue;
        }
        if (data->angle == 0.0f) {
            for (j = 0; j < 3; j++, vtx++, st++) {
                PSVECScale(&data->pos[j], &pos, data->scale);
                PSVECAdd(&pos, &data->vel, vtx);
                *st = data->uv[j];
            }
        } else {
            sin = mbSinDeg(data->angle);
            cos = mbCosDeg(data->angle);
            for (j = 0; j < 3; j++, vtx++, st++) {
                PSVECScale(&data->pos[j], &pos, data->scale);
                vtx->x = data->vel.x + ((pos.x * cos) - (pos.y * sin));
                vtx->y = data->vel.y + ((pos.x * sin) + (pos.y * cos));
                vtx->z = data->vel.z + pos.z;
                *st = data->uv[j];
            }
        }
    }
    DCFlushRangeNoSync(work->vtx, work->vtxNum * sizeof(*work->vtx));
    DCFlushRangeNoSync(work->st, work->vtxNum * sizeof(*work->st));
    DCFlushRangeNoSync(work->data, work->num * sizeof(*work->data));
    PPCSync();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, work->vtx, sizeof(*work->vtx));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, &work->data->color, sizeof(*work->data));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, work->st, sizeof(*work->st));
    GXCallDisplayList(work->dl, work->dlSize);
}

static void CapEffTrailKill(void)
{
    capEffTrailOMObj = NULL;
}

static void CapEffTrailCreate(int capsuleNo)
{
    CAP_EFF_TRAIL_WORK *work;
    MBPARTICLE *particle;
    MBPARTICLEDATA *data;
    int i;

    capEffTrailOMObj = omAddObjEx(mbObjMan, -32768, 13, 0, OM_GRP_NONE,
        CapEffTrailOMExec);
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    capEffTrailOMObj->data = work;
    memset(work, 0, sizeof(*work));
    for (i = 0; i < 12; i++) {
        capEffTrailOMObj->mdlId[i] = MB_MODEL_NONE;
        work->prevPos[i].x = 0.0f;
        work->prevPos[i].y = 0.0f;
        work->prevPos[i].z = 0.0f;
    }
    capEffTrailOMObj->mdlId[12] = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 42), HU_MEMNUM_OVL)), 24);
    Hu3DModelCameraSet(capEffTrailOMObj->mdlId[12], 1);
    Hu3DModelLayerSet(capEffTrailOMObj->mdlId[12], 5);
    particle = Hu3DData[capEffTrailOMObj->mdlId[12]].hookData;
    particle->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    data = particle->data;
    for (i = 0; i < particle->num; i++, data++) {
        data->speedDecay = 0.0f;
        data->pos.x = 0.0f;
        data->pos.y = 0.0f;
        data->pos.z = 0.0f;
    }
    capEffTrailOMObj->work[0] = 0;
    capEffTrailOMObj->work[1] = 0;
}

static void CapEffTrailPosSet(HuVecF *pos)
{
    OMOBJ *obj = capEffTrailOMObj;

    if (capEffTrailOMObj != NULL) {
        obj->trans.x = pos->x;
        obj->trans.y = pos->y;
        obj->trans.z = pos->z;
    }
}

static void CapEffTrailAdd(HuVecF *pos, int capsuleNo)
{
    CAP_EFF_TRAIL_WORK *work;
    MBPARTICLE *particle;
    MBPARTICLEDATA *data;
    GXColor color;
    float time;
    int i;

    if (capEffTrailOMObj == NULL) {
        return;
    }
    work = capEffTrailOMObj->data;
    for (i = 0; i < 12; i++) {
        work->prevPos[i] = *pos;
    }
    particle = Hu3DData[capEffTrailOMObj->mdlId[12]].hookData;
    color = capsuleTrailColorTbl[capsuleData[(u8)capsuleNo].color][0];
    data = particle->data;
    for (i = 0; i < particle->num / 2; i++, data++) {
        time = (float)i / 12.0f;
        data->speedDecay = 100.0f * (0.9f + (-0.49999997f * time));
        data->pos = *pos;
        data->color.r = color.r;
        data->color.g = color.g;
        data->color.b = color.b;
        data->color.a = (u8)(64.0f * (1.0f - time));
    }
    color = capsuleTrailColorTbl[capsuleData[(u8)capsuleNo].color][1];
    for (i = 0; i < particle->num / 2; i++, data++) {
        time = (float)i / 12.0f;
        data->speedDecay = 0.75f * 100.0f *
            (0.9f + (-0.49999997f * time));
        data->pos = *pos;
        data->color.r = color.r;
        data->color.g = color.g;
        data->color.b = color.b;
        data->color.a = (u8)(32.0f * (1.0f - time));
    }
    capEffTrailOMObj->work[0] = 1;
}

static void CapEffTrailOMExec(OMOBJ *obj)
{
    CAP_EFF_TRAIL_WORK *work = obj->data;
    CAP_EFF_TRAIL_POINT point[11];
    MBPARTICLE *particle;
    MBPARTICLEDATA *data;
    HuVecF pos[12];
    HuVecF delta;
    float length;
    float totalLength;
    float distance;
    int pointNo;
    int nextPointNo;
    int i;

    if (mbExitCheck() || capEffTrailOMObj == NULL) {
        for (i = 0; i < 12; i++) {
            if (obj->mdlId[i] != MB_MODEL_NONE) {
                mbObjKill(obj->mdlId[i]);
                obj->mdlId[i] = MB_MODEL_NONE;
            }
        }
        mbParticleKill(obj->mdlId[12]);
        obj->mdlId[12] = MB_MODEL_NONE;
        omDelObjEx(mbObjMan, obj);
        capEffTrailOMObj = NULL;
        return;
    }
    if (obj->work[0] == 0) {
        return;
    }
    work->prevPos[obj->work[1]] = obj->trans;
    totalLength = 0.0f;
    for (i = 0; i < 11; i++) {
        pointNo = i + obj->work[1];
        if (pointNo > 11) {
            pointNo -= 12;
        }
        nextPointNo = pointNo + 1;
        if (nextPointNo > 11) {
            nextPointNo -= 12;
        }
        point[i].start = work->prevPos[pointNo];
        point[i].end = work->prevPos[nextPointNo];
        PSVECSubtract(&point[i].end, &point[i].start, &delta);
        point[i].mag = PSVECMag(&delta);
        point[i].totalMag = totalLength;
        totalLength += point[i].mag;
    }
    for (i = 0; i < 12; i++) {
        if (totalLength <= 0.0f) {
            pos[i] = obj->trans;
            continue;
        }
        if (totalLength > 12.0f) {
            distance = 10.0f * (float)i;
        } else {
            distance = (totalLength / 12.0f) * (float)i;
        }
        for (pointNo = 0; pointNo < 11; pointNo++) {
            if ((point[pointNo].totalMag + point[pointNo].mag) > distance) {
                break;
            }
        }
        if (pointNo >= 11) {
            pos[i] = point[10].end;
        } else if (point[pointNo].mag <= 0.0f) {
            pos[i] = point[pointNo].start;
        } else {
            PSVECSubtract(&point[pointNo].end, &point[pointNo].start, &delta);
            PSVECScale(&delta, &delta,
                (distance - point[pointNo].totalMag) / point[pointNo].mag);
            PSVECAdd(&point[pointNo].start, &delta, &pos[i]);
        }
    }
    for (i = 0; i < 12; i++) {
        if (obj->mdlId[i] != MB_MODEL_NONE) {
            mbObjPosSetV(obj->mdlId[i], &pos[i]);
        }
    }
    particle = Hu3DData[obj->mdlId[12]].hookData;
    data = particle->data;
    for (i = 0; i < particle->num / 2; i++, data++) {
        data->pos = pos[i];
    }
    for (i = 0; i < particle->num / 2; i++, data++) {
        data->pos = pos[i];
    }
    obj->work[1]--;
    if ((s32)obj->work[1] < 0) {
        obj->work[1] = 11;
    }
}

s16 mbCapValueTypeGet(s16 value)
{
    return value & CAPSULE_VALUE_TYPE_MASK;
}

static void CapThrowEndWin(int unused, int value)
{
    value = mbCapValueTypeGet(value);

    switch (value) {
        case CAPSULE_HONE:
            mbWinCreate(2,
                MESSNUM(
                    MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_END_SPECIAL),
                -1);
            mbWinTopWait();
            break;

        case CAPSULE_KOOPA:
            mbWinCreate(2,
                MESSNUM(
                    MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_END_SPECIAL),
                -1);
            mbWinTopWait();
            break;
    }
}

s16 mbCapMasuTypeGet(s16 masuId)
{
    return mbCapValueTypeGet(mbMasuCapsuleGet(masuId));
}

s16 mbCapValuePlayerGet(s16 value)
{
    return (value >> CAPSULE_VALUE_PLAYER_SHIFT) & CAPSULE_VALUE_TYPE_MASK;
}

s16 mbCapMasuPlayerGet2(s16 masuId)
{
    return mbMasuCapsuleGet(masuId) >> 8;
}

void mbCapMasuPlayerSet(s16 masuId, s16 playerNo)
{
    s16 capsuleNo = (s16)mbMasuCapsuleGet(masuId);

    capsuleNo |=
        (playerNo & CAPSULE_VALUE_TYPE_MASK) << CAPSULE_VALUE_PLAYER_SHIFT;
    mbMasuCapsuleSet(masuId, capsuleNo);
}

void mbCapMasuPlayerTypeSet(s16 masuId, s16 capsuleNo, s16 playerNo)
{
    mbMasuCapsuleSet(masuId, capsuleNo);
    mbCapMasuPlayerSet(masuId, playerNo);
}

s16 mbCapMasuPlayerGet(s16 masuId)
{
    return mbMasuCapsuleGet(masuId) >> 8;
}

int mbCapComChanceGet(int capsuleNo, int playerNo, int mode)
{
    CAPSULE_COM_CHANCE *data;
    int chance;
    int rank;
    int step;
    int step2;
    int step3;
    int step4;
    int i;
    s8 code;

    data = capsuleChanceTbl;
    capsuleSelectComBack = FALSE;
    i = 0;
    while (data->capsuleNo != -1) {
        if (data->capsuleNo == capsuleNo) {
            break;
        }
        i++;
        data++;
    }
    if (data->capsuleNo < 0) {
        return -1;
    }
    rank = GwPlayer[playerNo].charNo;
    if (rank > 10) {
        rank = 10;
    }
    chance = data->rank[rank].chance;
    code = data->rank[rank].code;
    step = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 7);
    switch (capsuleNo) {
        case 1:
            if (code == '*' && CapCheckComPath(playerNo, 15, 0)) {
                chance = 0;
            }
            break;
        case 2:
            step2 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 3);
            step3 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 5);
            step4 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 4);
            if ((step2 >= 1 && step2 <= 10) || (step3 >= 1 && step3 <= 10) ||
                (step4 >= 1 && step4 <= 10)) {
                if (code == 'a') {
                    chance = 80;
                } else if (code == 'b') {
                    chance = 50;
                }
            }
            break;
        case 3:
            if (code == '*' && CapCheckComPath(playerNo, 10, 1)) {
                chance = 80;
            }
            break;
        case 4:
            if (code == '*') {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    if (i != playerNo &&
                        GwPlayer[i].masuId == GwPlayer[playerNo].masuId) {
                        break;
                    }
                }
                if (i >= GW_PLAYER_MAX && !CapCheckComPath(playerNo, 10, 2)) {
                    chance = 0;
                }
            }
            if (step >= 1 && step <= 10) {
                chance = 0;
            }
            break;
        case 5:
            if (code == '*') {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    if (i != playerNo) {
                        step2 = mbMasuFind_TypeStepGet(GwPlayer[i].masuId, 7);
                        if (step2 >= 1 && step2 <= 10) {
                            break;
                        }
                    }
                }
                if (i < GW_PLAYER_MAX) {
                    chance = 80;
                }
            }
            break;
        case 6:
            if (code == '*' && mbPlayerCoinGet(playerNo) <= 20) {
                chance = 0;
            }
            break;
        case 11:
            if (code == '*' && mbPlayerCoinGet(playerNo) < 10) {
                chance = 0;
            }
            break;
        case 22:
            if (code == '*' && step >= 0 && step <= 5 &&
                mbCapSelectMasuBackNum(GwPlayer[playerNo].masuId) > 0) {
                chance = 80;
                capsuleSelectComBack = TRUE;
            }
            break;
        case 23:
            if (code == '*' && step >= 0 && step <= 5 &&
                mbCapSelectMasuBackNum(GwPlayer[playerNo].masuId) > 0) {
                chance = 80;
                capsuleSelectComBack = TRUE;
            }
            break;
        case 30:
            if (code == '*' &&
                mbPlayerCapsuleNumGet(playerNo) >= mbPlayerCapsuleMaxGet()) {
                chance = 30;
            }
            if (mode == 0 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 0;
            }
            if (mode == 2 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 100;
            }
            break;
        case 31:
            if (code == '*' &&
                mbPlayerCapsuleNumGet(playerNo) >= mbPlayerCapsuleMaxGet()) {
                chance = 30;
            }
            if (mode == 0 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 0;
            }
            if (mode == 2 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 100;
            }
            break;
    }
    switch (mode) {
        case 0:
            return chance;
        case 1:
            return data->rank[rank].chance;
        case 2:
            return 100 - chance;
        default:
            return 0;
    }
}

static int capsuleThrowTbl[8] = { 10, 11, 12, 13, 25, 15, 16, 17 };
static int capsuleTrapTbl[5] = { 20, 21, 22, 23, 24 };
static int capsuleChanceWeightTbl[3][4][5] = {
    {
        { 70, 15, 15 },
        { 70, 20, 10 },
        { 70, 20, 0, 10 },
        { 50, 30, 0, 20 },
    },
    {
        { 40, 30, 30 },
        { 30, 30, 20, 20 },
        { 10, 40, 10, 30, 10 },
        { 10, 40, 0, 30, 20 },
    },
    {
        { 30, 30, 30, 10 },
        { 20, 20, 20, 30, 10 },
        { 10, 30, 0, 40, 20 },
        { 10, 20, 0, 40, 30 },
    },
};
static int capsuleBorderFileTbl[6] = {
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_0),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_1),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_2),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_3),
    0,
    0,
};

static BOOL CapCheckComPath(int playerNo, int max, int mode)
{
    s16 masuStack[32];
    s16 maxStack[32];
    u8 visited[256];
    s16 startMasu;
    s16 masuId;
    s16 nextMasu;
    int stackNum;
    int linkNum;
    int i;

    startMasu = GwPlayer[playerNo].masuId;
    masuId = startMasu;
    stackNum = 0;
    memset(visited, 0, sizeof(visited));
    for (;;) {
        visited[masuId] = TRUE;
        if (masuId != startMasu &&
            ((mbBranchAttrGet() & mbMasuAttrGet(masuId)) ||
             (mbBranchMAttrGet() & mbMasuMAttrGet(masuId)))) {
            max = 0;
        } else {
            if (mbMasuDispCheck(masuId)) {
                max--;
            }
            if (masuId != startMasu) {
                if (mode == 0) {
                    if (mbCapMasuDispTypeGet(masuId) == 2 &&
                        mbCapMasuTypeGet(masuId) == 23 &&
                        mbCapMasuPlayerGet2(masuId) != playerNo) {
                        return TRUE;
                    }
                } else if (mode == 1) {
                    if (mbCapMasuDispTypeGet(masuId) == 2 &&
                        mbCapMasuPlayerGet2(masuId) != playerNo) {
                        return TRUE;
                    }
                } else if (mode == 2) {
                    for (i = 0; i < GW_PLAYER_MAX; i++) {
                        if (i != playerNo && GwPlayer[i].masuId == masuId) {
                            return TRUE;
                        }
                    }
                }
            }
            linkNum = mbMasuLinkNumGet(masuId);
            if (linkNum > 0 && max > 0) {
                for (i = 0; i < linkNum; i++) {
                    nextMasu = mbMasuLinkGet(masuId, i);
                    if (!visited[nextMasu]) {
                        if (i < linkNum - 1) {
                            masuStack[stackNum] = masuId;
                            maxStack[stackNum] = max;
                            stackNum++;
                        }
                        break;
                    }
                }
                if (i < linkNum) {
                    masuId = nextMasu;
                    if (mbMasuLinkNumGet(masuId) < 1) {
                        visited[masuId] = TRUE;
                    }
                    continue;
                }
            }
        }
        if (stackNum < 1) {
            return FALSE;
        }
        stackNum--;
        masuId = masuStack[stackNum];
        max = maxStack[stackNum];
    }
}

int mbCapSelectComGet(int playerNo, int *capsuleTbl, int capsuleNum)
{
    CAPSULE_COM_CHOICE temp;
    CAPSULE_COM_CHOICE choice[10];
    BOOL choiceBack[10];
    BOOL tempBack;
    float random;
    int chance;
    int num;
    int i;
    int j;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return mbTutorialCall(15);
    }
    if (capsuleNum < 1) {
        return -1;
    }
    if (GwSystem.turnNo >= GwSystem.turnMax && mbPlayerCoinGet(playerNo) > 19) {
        for (i = 0; i < capsuleNum; i++) {
            if (capsuleTbl[i] == 6) {
                return i;
            }
        }
    }
    if (mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet() &&
        (lbl_802C4530 * MBCapsuleEffRandF()) < 30.0f) {
        return -1;
    }
    for (i = 0; i < capsuleNum; i++) {
        choice[i].index = i;
        choice[i].capsuleNo = capsuleTbl[i];
        choice[i].chance = mbCapComChanceGet(choice[i].capsuleNo, playerNo, 0);
        choiceBack[i] = capsuleSelectComBack;
        if ((mbCapUseModeGet(choice[i].capsuleNo) == 1 ||
                mbCapUseModeGet(choice[i].capsuleNo) == 2) &&
            mbCapSelectMasuNum(GwPlayer[playerNo].masuId) < 1) {
            choice[i].chance = 0;
        }
    }
    for (i = 0; i < capsuleNum; i++) {
        for (j = i + 1; j < capsuleNum; j++) {
            if (choice[i].chance < choice[j].chance) {
                temp = choice[i];
                choice[i] = choice[j];
                choice[j] = temp;
                tempBack = choiceBack[i];
                choiceBack[i] = choiceBack[j];
                choiceBack[j] = tempBack;
            }
        }
    }
    num = 0;
    chance = 0;
    for (i = 0; i < capsuleNum; i++) {
        if (choice[i].chance > 0) {
            chance += choice[i].chance;
            num++;
        }
    }
    if (num < 1) {
        return -1;
    }
    random = (float)chance * MBCapsuleEffRandF();
    for (i = 0; i < num; i++) {
        random -= choice[i].chance;
        if (random <= 0.0f) {
            capsuleSelectComBack = choiceBack[i];
            return choice[i].index;
        }
    }
    return -1;
}

int mbCapSelectDeleteComGet(int playerNo, int *capsuleTbl, int capsuleNum)
{
    CAPSULE_COM_CHOICE temp;
    CAPSULE_COM_CHOICE choice[10];
    CAPSULE_COM_CHOICE *choiceP;
    int i;
    int j;

    if (capsuleNum <= 0) {
        return -1;
    }
    i = 0;
    choiceP = choice;
    for (; i < capsuleNum; i++, choiceP++) {
        choiceP->index = i;
        choiceP->capsuleNo = capsuleTbl[i];
        choiceP->chance = mbCapComChanceGet(choiceP->capsuleNo, playerNo, 2);
    }
    for (i = 0; i < capsuleNum; i++) {
        for (j = i + 1; j < capsuleNum; j++) {
            if (choice[i].chance < choice[j].chance) {
                temp = choice[i];
                choice[i] = choice[j];
                choice[j] = temp;
            }
        }
    }
    return choice[0].index;
}

void mbCapRandomThrowAdd(int capsuleNo, int playerNo, int checkF)
{
    int masuId;
    int masuType;
    int value;
    int owner;

    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        masuType = mbMasuTypeGet(masuId);
        if (checkF == 0 && mbMasuCapsuleGet(masuId) != -1) {
            continue;
        }
        if (masuType != 1 && masuType != 2) {
            continue;
        }
        if (capsuleNo >= 0) {
            value = capsuleThrowTbl[(capsuleNo - 10) % 8];
        } else {
            value = capsuleThrowTbl[mbRandMod(8)];
        }
        if (playerNo >= 0) {
            owner = playerNo;
        } else {
            owner = mbRandMod(4);
        }
        mbCapMasuCapsuleSet(masuId, value, owner);
    }
}

void mbCapRandomTrapAdd(int capsuleNo, int playerNo, int checkF)
{
    int masuId;
    int masuType;
    int value;
    int owner;

    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        masuType = mbMasuTypeGet(masuId);
        if (checkF == 0 && mbMasuCapsuleGet(masuId) != -1) {
            continue;
        }
        if (masuType != 1 && masuType != 2) {
            continue;
        }
        if (capsuleNo >= 0) {
            value = capsuleTrapTbl[(capsuleNo - 20) % 5];
        } else {
            value = capsuleTrapTbl[mbRandMod(5)];
        }
        if (playerNo >= 0) {
            owner = playerNo;
        } else {
            owner = mbRandMod(4);
        }
        mbCapMasuCapsuleSet(masuId, value, owner);
    }
}

void mbCapMasuCapsuleSet(int masuId, int capsuleNo, int playerNo)
{
    s16 value;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo <= CAPSULE_KETTOU) {
        switch (capsuleNo) {
            case CAPSULE_HONE:
                return;
            case CAPSULE_KOOPA:
                return;
            default:
                mbMasuCapsuleSet(masuId, (s16)capsuleNo);
                value = mbMasuCapsuleGet(masuId);
                value |= ((s16)playerNo & CAPSULE_VALUE_TYPE_MASK)
                    << CAPSULE_VALUE_PLAYER_SHIFT;
                mbMasuCapsuleSet(masuId, value);
                switch (capsuleNo) {
                    case CAPSULE_BOMHEI:
                        break;
                }
                break;
        }
    }
}

s16 mbCapUseModeGet(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].useMode;
}

BOOL mbCapUseTrapCheck(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (mbCapUseModeGet(capsuleNo) == 2) {
        return TRUE;
    }
    return FALSE;
}

int mbCapBuyCostGet(s16 capsuleNo, s16 playerNo)
{
    int cost;
    int costNo;
    int i;

    cost = -1;
    if (!GwSystem.nextTime) {
        costNo = 0;
    } else if (GwPlayer[playerNo].rank == 1) {
        costNo = 0;
    } else if (GwPlayer[playerNo].rank == 0) {
        costNo = 1;
    } else {
        costNo = 2;
    }
    for (i = 0; i < 32 && capsuleList[i].id != -1; i++) {
        if (capsuleNo == capsuleList[i].id) {
            if (capsuleList[i].cost[costNo] > 0) {
                cost = capsuleList[i].cost[costNo];
            }
            break;
        }
    }
    if (cost < 1) {
        cost = mbCapCostGet(capsuleNo);
    }
    return cost;
}

int mbCapSellCostGet(s16 capsuleNo, s16 playerNo)
{
    int cost;
    int costNo;
    int i;

    cost = -1;
    if (GwPlayer[playerNo].rank == 1) {
        costNo = 0;
    } else if (GwPlayer[playerNo].rank == 0) {
        costNo = 2;
    } else {
        costNo = 1;
    }
    for (i = 0; i < 32 && capsuleList[i].id != -1; i++) {
        if (capsuleNo == capsuleList[i].id) {
            if (capsuleList[i].cost[costNo] > 0) {
                cost = capsuleList[i].cost[costNo];
            }
            break;
        }
    }
    if (cost < 1) {
        cost = mbCapCostGet(capsuleNo);
    }
    return cost;
}

int mbCapCostGet(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].cost;
}

BOOL mbCapListExcludeCheck(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].listFlag;
}

BOOL mbCapThrowMasuCheck(int masuId)
{
    BOOL result;
    int masuType;
    u32 masuMAttr;
    int i;

    masuType = mbMasuTypeGet(masuId);
    masuMAttr = mbMasuMAttrGet(masuId);
    if (masuType != 1 && masuType != 2) {
        goto result_false;
    }
    if (mbCapMasuDispTypeGet(masuId) != 0 &&
        mbCapMasuDispTypeGet(masuId) != 1) {
        goto result_false;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (masuId == GwPlayer[i].masuId) {
            result = FALSE;
            goto done;
        }
    }
    result = TRUE;
    goto done;

result_false:
    result = FALSE;
done:
    return result;
}

static BOOL CapSelectMasuCheck(int masuId)
{
    int masuType;
    u32 masuMAttr;
    int i;

    masuType = mbMasuTypeGet(masuId);
    masuMAttr = mbMasuMAttrGet(masuId);
    if (masuType == 1 || masuType == 2) {
        if (mbCapMasuDispTypeGet(masuId) == 0 ||
            mbCapMasuDispTypeGet(masuId) == 1) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (masuId == GwPlayer[i].masuId) {
                    return FALSE;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

void mbCapSelectMasuInit(void)
{
    int maxNum = 0;
    s16 *masuFlagData;
    s16 *masuFlag;
    int masuId;
    int i;
    int num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    for (masuId = 1; masuId <= mbMasuRawNumGet(); masuId++) {
        memset(masuFlag, 0, sizeof(s16) * 256);
        capsuleMasuSelectEndF = TRUE;
        CapSelectMasuListGet(masuFlag, masuId, 5, 5);
        for (i = 0, num = 1; i < 256; i++) {
            if (masuFlag[i] & 1) {
                num++;
            }
        }
        if (num > maxNum) {
            maxNum = num;
        }
    }
    HuMemDirectFree(masuFlag);
}

static void CapSelectMasuListGet(
    s16 *masuFlag, s16 masuId, s16 frontMax, s16 backMax)
{
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddFront(masuFlag, masuId, frontMax);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddBack(masuFlag, masuId, backMax);
    if (masuId != mbMasuFind_AttrIdGet(-1, MASU_FLAG_START)) {
        CapSelectMasuLinkCheck(masuFlag,
            mbMasuTypeFindLink(
                mbMasuFind_AttrIdGet(-1, MASU_FLAG_START), 0));
    }
}

static void CapSelectMasuAddFront(s16 *masuFlag, s16 masuId, s16 max)
{
    s16 link[10];
    s16 linkNum;
    int i;

    linkNum = mbMasuLinkTblGet(masuId, link);
    if (CapSelectMasuCheck(masuId)) {
        masuFlag[masuId] = 1;
    } else {
        masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;
    }
    if (CapSelectMasuDispCheck(masuId) && !capsuleMasuSelectEndF) {
        max--;
    }
    capsuleMasuSelectEndF = FALSE;
    if (max > 0) {
        for (i = 0; i < linkNum; i++) {
            if (mbev_CapMasuMoveCheck(link[i])) {
                continue;
            }
            if (i == 0 && (mbMasuAttrGet(masuId) & MASU_FLAG_BATTAN)) {
                continue;
            }
            CapSelectMasuAddFront(masuFlag, link[i], max);
        }
    }
}

static void CapSelectMasuAddBack(s16 *masuFlag, s16 masuId, s16 max)
{
    s16 parent[10];
    s16 parentNum;
    s16 endF;
    int i;
    int num;

    if (CapSelectMasuCheck(masuId)) {
        masuFlag[masuId] = 1;
    } else {
        masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;
    }
    if (mbMasuDispCheck(masuId) && mbev_CapMasuMoveCheck(masuId)) {
        max = 0;
    }
    if (CapSelectMasuDispCheck(masuId) && !capsuleMasuSelectEndF) {
        max--;
    }
    endF = capsuleMasuSelectEndF;
    capsuleMasuSelectEndF = FALSE;
    num = 0;
    if (max > 0) {
        parentNum = mbMasuLinkParentGet(masuId, parent);
        for (i = 0; i < parentNum; i++) {
            if (!mbev_CapMasuMoveCheck(parent[i]) || mbMasuDispCheck(parent[i])) {
                CapSelectMasuAddBack(masuFlag, parent[i], max);
                num++;
            }
        }
    }
    if (!endF && num <= 0 && mbMasuTypeGet(masuId) == 0) {
        CapSelectMasuLinkCheck(masuFlag, masuId);
    }
}

static void CapSelectMasuLinkCheck(s16 *masuFlag, s16 masuId)
{
    s16 link[10];
    s16 linkNum;
    int i;

    if (masuId >= 1) {
        linkNum = mbMasuLinkTblGet(masuId, link);
        if (masuFlag[masuId] != 0) {
            if (mbMasuTypeGet(masuId) == 0) {
                masuFlag[masuId] = 0;
            } else {
                return;
            }
            for (i = 0; i < linkNum; i++) {
                if (!mbev_CapMasuMoveCheck(link[i])) {
                    CapSelectMasuLinkCheck(masuFlag, link[i]);
                }
            }
        }
    }
}

int mbCapFileGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return mbBoardDataNumGet(capsuleData[capsuleNo].file);
}

int mbCapDescMesGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].descMes;
}

int mbCapUseMesGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].useMes;
}

char *mbCapDebugNameGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].debugName;
}

int mbCapMasuPatGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].masuPat;
}

int mbCapColorGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].color;
}

BOOL mbCapUseCheck(int capsuleNo)
{
    if (mbCapUseModeGet(capsuleNo) == 0) {
        return TRUE;
    }
    return FALSE;
}

BOOL mbCapValidCheck(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo < 0 || capsuleNo >= CAPSULE_MAX) {
        return FALSE;
    }
    if (capsuleData[capsuleNo].file == 0) {
        return FALSE;
    }
    return TRUE;
}

s16 mbCapMasuDispTypeGet(s16 masuId)
{
    s16 capsuleNo;

    capsuleNo = mbMasuCapsuleGet(masuId);
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo == CAPSULE_VALUE_NONE) {
        return 0;
    }
    if (mbMasuTypeGet(masuId) != 1 && mbMasuTypeGet(masuId) != 2) {
        return 0;
    }
    if (mbCapUseModeGet(capsuleNo) == 1) {
        return 1;
    }
    return 2;
}

int mbCapBonusCoinNumGet(int playerNo, int capsuleNo)
{
    int bonus;
    int rank;

    bonus = 0;
    switch (GwPlayer[playerNo].rank) {
        case 0:
            rank = 0;
            break;

        case 1:
            rank = 1;
            break;

        case 2:
            rank = 2;
            break;

        default:
            rank = 3;
            break;
    }
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    switch (capsuleData[capsuleNo].useMode) {
        case 0:
            if ((lbl_802C4530 * MBCapsuleEffRandF()) <
                (lbl_802C4540 + (lbl_802C4544 * rank))) {
                bonus = mbRandMod(rank + 1) + 1;
            }
            break;

        case 1:
            if ((lbl_802C4530 * MBCapsuleEffRandF()) <
                (40.0f + (lbl_802C4544 * rank))) {
                bonus = mbRandMod(rank + 3) + 5;
            }
            break;

        case 2:
            if ((lbl_802C4530 * MBCapsuleEffRandF()) <
                (40.0f + (lbl_802C4544 * rank))) {
                bonus = mbRandMod(rank + 2) + 3;
            }
            break;
    }
    return bonus;
}

s16 mbCapDescWinCreate(int capsuleNo)
{
    HuVec2f pos;
    s16 winId;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    winId = mbWinCreate(6, capsuleData[capsuleNo].descMes, -1);
    mbWinMesSpeedSet(winId, 0);
    mbWinPause(winId);
    mbWinPosGet(winId, &pos);
    pos.y -= lbl_802C4598;
    mbWinPosSet(winId, (int)pos.x, (int)pos.y);
    return winId;
}

int mbCapValidListGet(int *list)
{
    int i;
    int num;

    i = 0;
    num = 0;
    for (; i < CAPSULE_VALID_LIST_MAX; i++) {
        if (mbCapValidCheck(i)) {
            list[num] = i;
            num++;
        }
    }
    return num;
}

static HU3D_MODELID CapEffCreate(ANIMDATA *anim, s16 num)
{
    CAP_EFFECT *effP;
    CAP_EFF_DATA *effDataP;
    s16 i;
    HuVec2f *st;
    HU3D_MODEL *modelP;
    HuVecF *vtx;
    HU3D_MODELID modelId;
    void *dlBuf;
    void *dlBegin;
    u32 workHeap;
    u32 particleHeap;
    u32 vertexHeap;
    u32 stHeap;
    u32 dlBufHeap;
    int dlSizeData;
    u32 dlDataHeap;
    void *workData;
    void *workBase;
    void *particleData;
    void *particleBase;
    void *vertexData;
    void *vertexBase;
    void *stData;
    void *stBase;
    void *dlBufData;
    void *dlBufBase;
    void *dlData;
    void *dlBase;

    modelId = Hu3DHookFuncCreate(CapEffDraw);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    modelP = &Hu3DData[modelId];
    workHeap = modelP->mallocNo;
    workData = HuMemDirectMallocNum(HEAP_MODEL,
        sizeof(CAP_EFFECT), workHeap);
    workBase = workData;
    modelP->hookData = effP = workBase;
    effP->anim = anim;
    effP->num = num;
    effP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    effP->dispAttr = CAP_EFF_DISPATTR_NONE;
    effP->hook = NULL;
    effP->hookMdlP = NULL;
    effP->count = 0;
    effP->attr = CAP_EFF_ATTR_NONE;
    effP->unk23 = 0;
    effP->prevCount = 0;
    effP->mode = effP->time = 0;
    particleHeap = modelP->mallocNo;
    particleData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(CAP_EFF_DATA), particleHeap);
    particleBase = particleData;
    effP->data = effDataP = particleBase;
    memset(effDataP, 0, num * sizeof(CAP_EFF_DATA));
    for (i = 0; i < num; i++, effDataP++) {
        effDataP->scale = 0;
        effDataP->rot.x = effDataP->rot.y = effDataP->rot.z = 0;
        effDataP->animTime = 0;
        effDataP->animSpeed = 1;
        effDataP->pos.x = 0.0f;
        effDataP->pos.y = 0.0f;
        effDataP->pos.z = 0.0f;
        effDataP->color.r = effDataP->color.g = effDataP->color.b =
            effDataP->color.a = 255;
        effDataP->no = 0;
    }
    vertexHeap = modelP->mallocNo;
    vertexData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(HuVecF) * 4, vertexHeap);
    vertexBase = vertexData;
    effP->vertex = vtx = vertexBase;
    for (i = 0; i < num * 4; i++, vtx++) {
        vtx->x = vtx->y = vtx->z = 0;
    }
    stHeap = modelP->mallocNo;
    stData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(HuVec2f) * 4, stHeap);
    stBase = stData;
    effP->st = st = stBase;
    for (i = 0; i < num; i++) {
        st->x = 0;
        st->y = 0;
        st++;

        st->x = 1;
        st->y = 0;
        st++;

        st->x = 1;
        st->y = 1;
        st++;

        st->x = 0;
        st->y = 1;
        st++;
    }
    dlBufHeap = modelP->mallocNo;
    dlBufData = HuMemDirectMallocNum(HEAP_MODEL, 65536, dlBufHeap);
    dlBufBase = dlBufData;
    dlBegin = dlBuf = dlBufBase;
    DCFlushRange(dlBuf, 65536);
    GXBeginDisplayList(dlBegin, 65536);
    GXBegin(GX_QUADS, GX_VTXFMT0, num * 4);
    for (i = 0; i < num; i++) {
        GXPosition1x16(i * 4);
        GXColor1x16(i);
        GXTexCoord1x16(i * 4);

        GXPosition1x16((i * 4) + 1);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 1);

        GXPosition1x16((i * 4) + 2);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 2);

        GXPosition1x16((i * 4) + 3);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 3);
    }
    GXEnd();
    effP->dlSize = GXEndDisplayList();
    dlDataHeap = modelP->mallocNo;
    dlSizeData = effP->dlSize;
    dlData = HuMemDirectMallocNum(HEAP_MODEL, dlSizeData, dlDataHeap);
    dlBase = dlData;
    effP->dl = dlBase;
    memcpy(effP->dl, dlBuf, effP->dlSize);
    DCFlushRange(effP->dl, effP->dlSize);
    HuMemDirectFree(dlBuf);
    return modelId;
}

static void CapEffDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFFECT *effP;
    CAP_EFF_DATA *effDataP;
    HuVecF *vtx;
    HuVec2f *st;
    HuVecF *scaleVtxP;
    s16 i;
    s16 j;
    HuVecF *initVtxP;
    s16 bmpFmt;
    s16 row;
    s16 col;
    CAP_EFF_HOOK hook;

    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRotZ;
    HuVecF scaleVtx[4];
    HuVecF finalVtx[4];
    HuVecF initVtx[4];
    ROMtx basePosMtx;
    static HuVecF posTbl[4] = {
        { -0.5f,  0.5f, 0.0f },
        {  0.5f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
    };
    static HuVec2f uvTbl[4] = {
        { 0.0f, 0.0f },
        { 0.25f, 0.0f },
        { 0.25f, 0.25f },
        { 0.0f, 0.25f },
    };

    effP = modelP->hookData;
    if (effP->prevCounter != GlobalCounter || shadowModelDrawF) {
        if (effP->hookMdlP && effP->hookMdlP != modelP) {
            CapEffDraw(effP->hookMdlP, mtx);
        }
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
            GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
            GX_COLOR0A0);
        if (shadowModelDrawF) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO,
                GX_CC_ZERO);
            GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        } else {
            bmpFmt = effP->anim->bmp->dataFmt & ANIM_BMP_FMTMASK;
            if (bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE,
                    GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_RASC, GX_CC_ZERO);
            }
            if (effP->dispAttr & CAP_EFF_DISPATTR_ZBUF_OFF) {
                GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
            } else if (modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
            GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX,
            GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        HuSprTexLoad(effP->anim, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT,
            GX_LINEAR);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        switch (effP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                    GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE,
                    GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR,
                    GX_LO_NOOP);
                break;
        }
        if (HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
            PSMTXIdentity(mtxInv);
        }
        PSMTXReorder(mtxInv, basePosMtx);
        if (effP->hook) {
            hook = effP->hook;
            hook(modelP, effP, mtx);
        }
        effDataP = effP->data;
        vtx = effP->vertex;
        st = effP->st;
        if (effP->dispAttr & CAP_EFF_DISPATTR_CAMERA_ROT) {
            MTXIdentity(mtxInv);
            MTXIdentity(*(Mtx *)(&basePosMtx));
            initVtx[0] = posTbl[0];
            initVtx[1] = posTbl[1];
            initVtx[2] = posTbl[2];
            initVtx[3] = posTbl[3];
        } else {
            PSMTXROMultVecArray(basePosMtx, &posTbl[0], initVtx, 4);
        }
        for (i = 0; i < effP->num; i++, effDataP++) {
            if (!effDataP->scale) {
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
            } else if (effP->dispAttr & CAP_EFF_DISPATTR_ROT3D) {
                VECScale(&posTbl[0], &scaleVtx[0], effDataP->scale);
                VECScale(&posTbl[1], &scaleVtx[1], effDataP->scale);
                VECScale(&posTbl[2], &scaleVtx[2], effDataP->scale);
                VECScale(&posTbl[3], &scaleVtx[3], effDataP->scale);
                mtxRot(mtxPos, effDataP->rot.x, effDataP->rot.y,
                    effDataP->rot.z);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            } else if (!effDataP->rot.z) {
                scaleVtxP = scaleVtx;
                initVtxP = initVtx;
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
            } else {
                VECScale(&posTbl[0], &scaleVtx[0], effDataP->scale);
                VECScale(&posTbl[1], &scaleVtx[1], effDataP->scale);
                VECScale(&posTbl[2], &scaleVtx[2], effDataP->scale);
                VECScale(&posTbl[3], &scaleVtx[3], effDataP->scale);
                MTXRotRad(mtxRotZ, 'Z', effDataP->rot.z);
                PSMTXConcat(mtxInv, mtxRotZ, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            }
        }
        effDataP = effP->data;
        st = effP->st;
        if (!(effP->dispAttr & CAP_EFF_DISPATTR_NOANIM)) {
            for (i = 0; i < effP->num; i++, effDataP++) {
                row = effDataP->no & 3;
                col = (effDataP->no >> 2) & 3;
                for (j = 0; j < 4; j++, st++) {
                    st->x = (0.25f * row) + uvTbl[j].x;
                    st->y = (0.25f * col) + uvTbl[j].y;
                }
            }
        } else {
            for (i = 0; i < effP->num; i++, effDataP++) {
                for (j = 0; j < 4; j++, st++) {
                    st->x = 4 * uvTbl[j].x;
                    st->y = 4 * uvTbl[j].y;
                }
            }
        }
        DCFlushRangeNoSync(effP->vertex, effP->num * sizeof(HuVecF) * 4);
        DCFlushRangeNoSync(effP->st, effP->num * sizeof(HuVec2f) * 4);
        DCFlushRangeNoSync(effP->data, effP->num * sizeof(CAP_EFF_DATA));
        PPCSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, effP->vertex, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(CAP_EFF_DATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2f));
        GXCallDisplayList(effP->dl, effP->dlSize);
        if (shadowModelDrawF == FALSE) {
            if (!(effP->attr & CAP_EFF_ATTR_COUNTER_UPDATE)) {
                effP->count++;
            }
            if (effP->prevCount != 0 && effP->prevCount <= effP->count) {
                if (effP->attr & CAP_EFF_ATTR_COUNTER_RESET) {
                    effP->count = 0;
                }
                effP->count = effP->prevCount;
            }
            effP->prevCounter = GlobalCounter;
        }
    }
}

int mbCapSelectMasuNum(int masuId)
{
    return mbCapSelectMasuFrontNum(masuId)
        + mbCapSelectMasuBackNum(masuId);
}

int mbCapSelectMasuFrontNum(int masuId)
{
    s16 *masuFlagData;
    s16 *masuFlag;
    s16 i;
    s16 num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    memset(masuFlag, 0, sizeof(s16) * 256);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddFront(masuFlag, masuId, 5);
    for (i = 0, num = 0; i < 256; i++) {
        if (masuFlag[i] & 1) {
            num++;
        }
    }
    HuMemDirectFree(masuFlag);
    return num;
}

int mbCapSelectMasuBackNum(int masuId)
{
    s16 *masuFlagData;
    s16 *masuFlag;
    s16 i;
    s16 num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    memset(masuFlag, 0, sizeof(s16) * 256);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddBack(masuFlag, masuId, 5);
    for (i = 0, num = 0; i < 256; i++) {
        if (masuFlag[i] & 1) {
            num++;
        }
    }
    HuMemDirectFree(masuFlag);
    return num;
}

void mbCapInit(void)
{
    int i;
    int j;
    int objId;
    CAPSULE_OBJ_COLOR *objColorData;
    s16 *borderObjData;
    s16 *borderId;

    objColorData = capsuleObjColorData;
    memset(objColorData, 0, sizeof(capsuleObjColorData));
    for (i = 0; i < 6; i++) {
        capsuleObjBorderId[i] = MB_MODEL_NONE;
    }
    capsuleBorderObjId = borderObjData = HuMemDirectMallocNum(
        HEAP_HEAP, 768, HU_MEMNUM_OVL);
    borderId = borderObjData;
    memset(capsuleBorderObjId, 0, 768);
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 64; j++, borderId++) {
            *borderId = MB_MODEL_NONE;
        }
    }
    for (i = 0; i < 8; i++) {
        capsuleObjData[i].objId = MB_MODEL_NONE;
    }
    objId = mbObjCreate(
        DATANUM(DATA_capsule, CAPSULE_DATA_INIT_MODEL_0), NULL, TRUE);
    mbObjDispSet(objId, FALSE);
    objId = mbObjCreate(
        DATANUM(DATA_capsule, CAPSULE_DATA_INIT_MODEL_1), NULL, TRUE);
    mbObjDispSet(objId, FALSE);
}


static int CapObjColorSearch(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int i;

    for (obj = capsuleObjColorData, i = 0; i < CAPSULE_OBJ_COLOR_MAX; i++, obj++) {
        if (obj->flag && obj->mdlId == id) {
            break;
        }
    }
    if (i < CAPSULE_OBJ_COLOR_MAX) {
        return i;
    } else {
        return -1;
    }
}

int mbCapObjColorCreate(int capsuleNo, BOOL createF)
{
    CAPSULE_OBJ_COLOR *obj;
    int i;
    int capValue;
    u32 file;
    s16 initialValue;
    s16 capValueShort;
    int objFile;
    int fileNo;

    obj = capsuleObjColorData;
    initialValue = (s16)capsuleNo & CAPSULE_VALUE_TYPE_MASK;
    capsuleNo = initialValue;
    for (i = 0; i < CAPSULE_OBJ_COLOR_MAX; i++, obj++) {
        if (!obj->flag) {
            break;
        }
    }
    if (i >= CAPSULE_OBJ_COLOR_MAX) {
        return -1;
    }
    capValue = capsuleNo;
    capValueShort = (s16)capValue & CAPSULE_VALUE_TYPE_MASK;
    capValue = capValueShort;
    objFile = capsuleData[capValue].objFile;
    fileNo = objFile;
    file = fileNo;
    obj->flag = TRUE;
    obj->mdlId = mbObjCreate(file, NULL, createF);
    obj->layer = 4;
    obj->pos.x = obj->pos.y = obj->pos.z = 0.0f;
    obj->rot.x = obj->rot.y = obj->rot.z = 0.0f;
    CapObjUnitScaleSet(obj);
    mbCapObjColorLayerSet(obj->mdlId, obj->layer);
    return obj->mdlId;
}

void mbCapObjColorKill(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        if (obj->flag) {
            mbObjKill(obj->mdlId);
            obj->mdlId = MB_MODEL_NONE;
            obj->mdlId2 = MB_MODEL_NONE;
            obj->flag = FALSE;
        }
    }
}

void mbCapObjColorPosSet(int id, float posX, float posY, float posZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->pos.x = posX;
        obj->pos.y = posY;
        obj->pos.z = posZ;
        mbObjPosSetV(obj->mdlId, &obj->pos);
    }
}

void mbCapObjColorRotSet(int id, float rotX, float rotY, float rotZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->rot.x = rotX;
        obj->rot.y = rotY;
        obj->rot.z = rotZ;
        mbObjRotSetV(obj->mdlId, &obj->rot);
    }
}

void mbCapObjColorScaleSet(int id, float scaleX, float scaleY, float scaleZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->scale.x = scaleX;
        obj->scale.y = scaleY;
        obj->scale.z = scaleZ;
        mbObjScaleSetV(obj->mdlId, &obj->scale);
    }
}

void mbCapObjColorPosSetV(int id, HuVecF *pos)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorPosSet(id, pos->x, pos->y, pos->z);
    }
}

void mbCapObjColorRotSetV(int id, HuVecF *rot)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorRotSet(id, rot->x, rot->y, rot->z);
    }
}

void mbCapObjColorScaleSetV(int id, HuVecF *scale)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorScaleSet(id, scale->x, scale->y, scale->z);
    }
}

void mbCapObjColorPosGet(int id, HuVecF *pos)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *pos = obj->pos;
    }
}

void mbCapObjColorRotGet(int id, HuVecF *rot)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *rot = obj->rot;
    }
}

void mbCapObjColorScaleGet(int id, HuVecF *scale)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *scale = obj->scale;
    }
}

void mbCapObjColorLayerSet(int id, u8 layer)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->layer = layer;
        mbObjLayerSet(obj->mdlId, obj->layer);
    }
}

u8 mbCapObjColorLayerGet(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) == -1) {
        return 0;
    }
    obj = &capsuleObjColorData[idx];
    return obj->layer;
}

void mbCapObjColorAttrSet(int id, u32 attr)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;
    MBMODELID mdlId;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mdlId = obj->mdlId;
        mbObjAttrSet(mdlId, attr);
    }
}

void mbCapObjColorAttrReset(int id, u32 attr)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;
    MBMODELID mdlId;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mdlId = obj->mdlId;
        mbObjAttrReset(mdlId, attr);
    }
}

void mbCapObjColorDispSet(int id, BOOL dispF)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjDispSet(obj->mdlId, dispF);
    }
}

void mbCapObjColorAlphaSet(int id, u8 alpha)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjAlphaSet(obj->mdlId, alpha);
    }
}

void mbCapObjColorMtxSet(int id, Mtx *mtx)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjMtxSet(obj->mdlId, mtx);
    }
}

void mbCapObjColorMtxGet(int id, Mtx *mtx)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjMtxGet(obj->mdlId, mtx);
    }
}

void mbCapObjBorderKill(int objId);

int mbCapObjBorderCreate(int objId, int capsuleNo)
{
    HSF_DATA *hsf;
    HSF_OBJECT *object;
    HSF_OBJECT *objectP;
    char name[HSF_OBJNAME_MAX_LEN];
    int i;
    int j;
    int groupNo;
    int modelId;
    s16 *borderId;

    modelId = mbObjModelIDGet(objId);
    hsf = Hu3DData[modelId].hsf;
    object = hsf->object;
    strcpy(name, MakeObjectName((s8 *)lbl_802BFE51));
    for (i = 0; i < hsf->objectNum; i++, object++) {
        objectP = object;
        if (objectP->constData && strcmp(name, objectP->name) == 0) {
            break;
        }
    }
    if (i >= hsf->objectNum) {
        return -1;
    }
    groupNo = capsuleNo / 10;
    if (capsuleObjBorderId[groupNo] < 0 && capsuleBorderFileTbl[groupNo] != 0) {
        capsuleObjBorderId[groupNo] = mbObjCreate(capsuleBorderFileTbl[groupNo], NULL, TRUE);
    }
    if (capsuleObjBorderId[groupNo] >= 0) {
        mbObjHookSet(objId, lbl_802BFE51, capsuleObjBorderId[groupNo]);
        borderId = &capsuleBorderObjId[groupNo * 64];
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId == -1) {
                *borderId = objId;
                break;
            }
        }
    }
    return capsuleObjBorderId[groupNo];
}

void mbCapObjKill(int objId)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (objId == capsuleObjData[i].objId) {
            break;
        }
    }
    if (i < 8) {
        Hu3DAnimKill(capsuleObjData[i].animId0);
        Hu3DAnimKill(capsuleObjData[i].animId1);
        HuSprAnimKill(capsuleObjData[i].anim);
        mbObjKill(capsuleObjData[i].objId);
        capsuleObjData[i].objId = MB_MODEL_NONE;
    } else {
        mbCapObjBorderKill(objId);
        mbObjKill(objId);
    }
}

void mbCapObjBorderKill(int objId)
{
    s16 *borderId = capsuleBorderObjId;
    int i;
    int j;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId == objId) {
                *borderId = -1;
            }
        }
    }
    for (i = 0; i < 6; i++) {
        borderId = &capsuleBorderObjId[i * 64];
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId >= 0) {
                break;
            }
        }
        if (j >= 64 && capsuleObjBorderId[i] >= 0) {
            mbObjKill(capsuleObjBorderId[i]);
            capsuleObjBorderId[i] = -1;
        }
    }
}


static void CapThrowCameraCalc(float t, float *x, float *y, float *z,
    HuVecF *out, int num)
{
    int lowX;
    int midX;
    int highX;
    int lowY;
    int midY;
    int highY;
    int lowZ;
    int midZ;
    int highZ;
    float spanX;
    float relX;
    float spanY;
    float relY;
    float spanZ;
    float relZ;
    float outX;
    float outY;
    float outZ;

    lowX = 0;
    highX = num - 1;
    while (lowX < highX) {
        midX = (lowX + highX) / 2;
        if (capsuleTime[midX] < t) {
            lowX = midX + 1;
        } else {
            highX = midX;
        }
    }
    if (lowX > 0) {
        lowX--;
    }
    spanX = capsuleTime[lowX + 1] - capsuleTime[lowX];
    relX = t - capsuleTime[lowX];
    outX = x[lowX] + (relX *
        ((relX * ((lbl_802C45D0 * capsuleBezierX[lowX]) +
            ((relX * (capsuleBezierX[lowX + 1] - capsuleBezierX[lowX])) /
                spanX))) +
            (((x[lowX + 1] - x[lowX]) / spanX) -
                (spanX * ((lbl_802C45C0 * capsuleBezierX[lowX]) +
                    capsuleBezierX[lowX + 1])))));
    out->x = outX;

    lowY = 0;
    highY = num - 1;
    while (lowY < highY) {
        midY = (lowY + highY) / 2;
        if (capsuleTime[midY] < t) {
            lowY = midY + 1;
        } else {
            highY = midY;
        }
    }
    if (lowY > 0) {
        lowY--;
    }
    spanY = capsuleTime[lowY + 1] - capsuleTime[lowY];
    relY = t - capsuleTime[lowY];
    outY = y[lowY] + (relY *
        ((relY * ((lbl_802C45D0 * capsuleBezierY[lowY]) +
            ((relY * (capsuleBezierY[lowY + 1] - capsuleBezierY[lowY])) /
                spanY))) +
            (((y[lowY + 1] - y[lowY]) / spanY) -
                (spanY * ((lbl_802C45C0 * capsuleBezierY[lowY]) +
                    capsuleBezierY[lowY + 1])))));
    out->y = outY;

    lowZ = 0;
    highZ = num - 1;
    while (lowZ < highZ) {
        midZ = (lowZ + highZ) / 2;
        if (capsuleTime[midZ] < t) {
            lowZ = midZ + 1;
        } else {
            highZ = midZ;
        }
    }
    if (lowZ > 0) {
        lowZ--;
    }
    spanZ = capsuleTime[lowZ + 1] - capsuleTime[lowZ];
    relZ = t - capsuleTime[lowZ];
    outZ = z[lowZ] + (relZ *
        ((relZ * ((lbl_802C45D0 * capsuleBezierZ[lowZ]) +
            ((relZ * (capsuleBezierZ[lowZ + 1] - capsuleBezierZ[lowZ])) /
                spanZ))) +
            (((z[lowZ + 1] - z[lowZ]) / spanZ) -
                (spanZ * ((lbl_802C45C0 * capsuleBezierZ[lowZ]) +
                    capsuleBezierZ[lowZ + 1])))));
    out->z = outZ;
}
