#include "game/board/masu.h"
#include "game/board/audio.h"
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
#include "game/sprite.h"

#include "humath.h"
#include "string.h"

#define M_PI 3.141592653589793

#define CAPSULE_OBJ_COLOR_MAX 128

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

typedef struct CapUseWork_s {
    int playerNo;
    int capsuleNo;
} CAP_USE_WORK;

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

static HUPROCESS *capsulePlayerThrowProc;
static CAPSULE_THROW_HOOK capsuleThrowHook;
static OMOBJ *capsuleGuideOMObj;
static int capsuleObjId = MB_MODEL_NONE;
static int capsuleColObjId = MB_MODEL_NONE;
static int capsuleColMdlId = MB_MODEL_NONE;
static int capEffThrowMdlId = MB_MODEL_NONE;
static BOOL capsuleMasuSelectComF[4] = { FALSE, FALSE, FALSE, FALSE };
static CAPSULE_DATA capsuleData[] = {
    { 0x000C0000, 0x000C001C, 0x00370000, 0x00380000, 0, 0, 5, 'A', 0, "KINOKO", 1 },
    { 0x000C0001, 0x000C001C, 0x00370001, 0x00380001, 0, 0, 10, 'B', 0, "S KINOKO", 1 },
    { 0x000C0002, 0x000C001C, 0x00370002, 0x00380002, 0, 0, 10, 'A', 0, "P KINOKO", 1 },
    { 0x000C0003, 0x000C001C, 0x00370003, 0x00380003, 0, 0, 20, 'A', 0, "M KINOKO", 1 },
    { 0x000C0004, 0x000C001C, 0x00370004, 0x00380004, 0, 0, 20, 'B', 0, "KILLER", 1 },
    { 0x000C0005, 0x000C001C, 0x00370005, 0x00380005, 0, 0, 15, 'C', 0, "DOKAN", 1 },
    { 0x000C0006, 0x000C001C, 0x00370006, 0x00380006, 0, 0, 30, 'E', 0, "HANACHAN", 1 },
    { 0x000C0007, 0x000C001C, 0x00370007, 0x00380007, 0, 0, 30, 'E', 0, "N KINOKO", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "NULL", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "NASI", 0 },
    { 0x000C0008, 0x000C001D, 0x00370008, 0x00380008, 1, 1, 10, 'A', 1, "TOGEZO", 1 },
    { 0x000C0009, 0x000C001D, 0x00370009, 0x00380009, 1, 1, 15, 'A', 1, "KURIBO", 1 },
    { 0x000C000A, 0x000C001D, 0x0037000A, 0x0038000A, 1, 1, 20, 'B', 1, "PAKKUN", 1 },
    { 0x000C000B, 0x000C001D, 0x0037000B, 0x0038000B, 1, 1, 5, 'D', 1, "JANGO", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 0, "HANUKE", 0 },
    { 0x000C000C, 0x000C001D, 0x0037000D, 0x0038000D, 1, 1, 10, 'C', 1, "KOKAMEKKU", 1 },
    { 0x000C000D, 0x000C001D, 0x0037000E, 0x0038000E, 1, 1, 5, 'B', 1, "KAMEKKU", 1 },
    { 0x000C000E, 0x000C001D, 0x0037000F, 0x0038000F, 1, 1, 10, 'A', 1, "THROWMAN", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 0, "SUKA", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 0, "KARA", 0 },
    { 0x000C000F, 0x000C001E, 0x00370010, 0x00380010, 2, 2, 10, 'B', 2, "BOBLE", 1 },
    { 0x000C0010, 0x000C001E, 0x00370011, 0x00380011, 2, 2, 15, 'C', 2, "BIRIQ", 1 },
    { 0x000C0011, 0x000C001E, 0x00370012, 0x00380012, 2, 2, 15, 'B', 2, "TUMUJIKUN", 1 },
    { 0x000C0012, 0x000C001E, 0x00370013, 0x00380013, 2, 2, 15, 'C', 2, "DOSSUN", 1 },
    { 0x000C0013, 0x000C001E, 0x00370014, 0x00380014, 2, 2, 10, 'C', 2, "BOMUHEI", 1 },
    { 0x000C0014, 0x000C001E, 0x0037000C, 0x0038000C, 2, 2, 10, 'E', 2, "PATAPATA", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "NETA", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "GA  ", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "TUKI", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "TA  ", 0 },
    { 0x000C0015, 0x000C001F, 0x00370015, 0x00380015, 3, 3, 20, 'D', 3, "HONE", 1 },
    { 0x000C0016, 0x000C001F, 0x00370016, 0x00380016, 3, 3, 10, 'D', 3, "LIGHT", 1 },
    { 0x000C0021, 0x000C001F, 0x00370017, 0x00380017, 3, 3, 15, 'D', 3, "TARU", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0x000C0021, 0x000C001F, 0x00370018, 0x00380018, 10, 3, 1, 'C', 3, "KILLER MOVE", 0 },
    { 0x000C0021, 0x000C001F, 0x00370019, 0x00380019, 10, 3, 1, 'C', 3, "KETTOU", 0 },
    { 0x000C0021, 0x000C0020, 0x0037001A, 0x0038001A, 10, 3, 1, 'C', 3, "MIRACLE", 0 },
    { 0x000C0021, 0x000C001F, 0x0037001B, 0x0038001B, 9, 3, 1, 'E', 3, "KOOPA", 0 },
    { 0x000C0021, 0x000C001F, 0x0037001C, 0x0038001C, 10, 3, 1, 'E', 3, "DONKEY", 0 },
    { 0x000C0021, 0x000C001F, 0x0037001D, 0x0038001D, 10, 3, 1, 'Z', 3, "VS", 0 },
    { 0x000C0021, 0x000C001F, 0x0037001E, 0x0038001E, 10, 3, 1, 'Z', 3, "R_TERESA", 0 },
    { 0x00050017, 0x000C001F, 0x0037001F, 0x00380022, 10, 0, 0, 'Z', 0, "DICE", 1 },
    { 0x000C0022, 0x000C001F, 0x00370020, 0x00380022, 10, 0, 0, 'Z', 0, "YAMERU", 1 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0x000C0021, 0x000C001F, 0x00370021, 0x0038001F, 10, 0, 1, 'Z', 0, "DEBUG CAM TEST", 0 },
    { 0x000C0021, 0x000C001F, 0x00370022, 0x00380020, 10, 0, 1, 'Z', 0, "DEBUG WARP TEST", 0 },
    { 0x000C0021, 0x000C001F, 0x00370023, 0x00380021, 10, 0, 1, 'Z', 0, "DEBUG SETPOS TEST", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, 0x00380022, 10, 0, 0, 'Z', 1, "0000", 0 },
};

static int capsuleBorderFileTbl[6] = {
    0x000C0018,
    0x000C0019,
    0x000C001A,
    0x000C001B,
    0,
    0,
};
static int capsuleThrowTbl[8] = { 10, 11, 12, 13, 25, 15, 16, 17 };
static int capsuleTrapTbl[5] = { 20, 21, 22, 23, 24 };
static int capsuleChanceTbl[3][4][5] = {
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
static int capsuleHiliteFileTbl[3] = { 0x000C0031, 0x000C0032, 0x000C0033 };
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
extern void mbWipeSpecialFadeOutCreate(int type, int time);
extern void mbWipeSpecialFadeInCreate(int type, int time);

s16 mbCapValueTypeGet(s16 value);
s16 mbCapMasuDispTypeGet(s16 masuId);
int mbCapObjCreate(int capsuleNo, BOOL flag);
int mbCapFileGet(int capsuleNo);
int mbCapColorGet(int capsuleNo);
int mbCapBonusCoinNumGet(int playerNo, int capsuleNo);
int mbCapComChanceGet(int capsuleNo, int playerNo, int mode);
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
extern const float lbl_802C44F8;
extern const float lbl_802C44C0;
extern const float lbl_802C4514;
extern const float lbl_802C4524;
extern const float lbl_802C45C0;
extern const float lbl_802C4570;
extern const float lbl_802C4598;
extern u32 mbCapEffNum;
extern s16 *mbCapEffData;

#define CAP_EFF_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

static void CapComKeyHook(void);
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

const float lbl_802C44F8 = 0.0f;
const float lbl_802C44C0 = 1.0f;
const float lbl_802C4514 = 3.725290298461914e-09f;
const float lbl_802C4524 = 0.25f;
const float lbl_802C45C0 = 2.0f;
const float lbl_802C4570 = 50.0f;
const float lbl_802C4544 = 10.0f;
const float lbl_802C466C = 120.0f;
const float lbl_802C4670 = 12.0f;

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

void mbCapAutoThrowEnd(CAP_AUTO_THROW_WORK *work)
{
}

void mbCapNumInc(int capsuleNo, int mode)
{
    capsuleNum[capsuleNo][mode]++;
}

static void CapEffCrackKill(void)
{
    capEffCrackOMObj = NULL;
}

static void CapEffTrailKill(void)
{
    capEffTrailOMObj = NULL;
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

s16 mbCapValueTypeGet(s16 value)
{
    return value & 0xFF;
}

static void CapThrowEndWin(int unused, int value)
{
    value = mbCapValueTypeGet(value);

    switch (value) {
        case 0x1E:
            mbWinCreate(2, 0x370025, -1);
            mbWinTopWait();
            break;

        case 0x2B:
            mbWinCreate(2, 0x370025, -1);
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
    return (value >> 8) & 0xFF;
}

s16 mbCapMasuPlayerGet2(s16 masuId)
{
    return mbMasuCapsuleGet(masuId) >> 8;
}

void mbCapMasuPlayerSet(s16 masuId, s16 playerNo)
{
    s16 capsuleNo = (s16)mbMasuCapsuleGet(masuId);

    capsuleNo |= (playerNo & 0xFF) << 8;
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
    if (capsuleNo <= 0x29) {
        switch (capsuleNo) {
            case 0x1E:
                return;
            case 0x2B:
                return;
            default:
                mbMasuCapsuleSet(masuId, (s16)capsuleNo);
                value = mbMasuCapsuleGet(masuId);
                value |= ((s16)playerNo & 0xFF) << 8;
                mbMasuCapsuleSet(masuId, value);
                switch (capsuleNo) {
                    case 0x18:
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

static void CapSelectMasuListGet(
    s16 *masuFlag, s16 masuId, s16 frontMax, s16 backMax)
{
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddFront(masuFlag, masuId, frontMax);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddBack(masuFlag, masuId, backMax);
    if (masuId != mbMasuFind_AttrIdGet(-1, 0x8000)) {
        CapSelectMasuLinkCheck(masuFlag,
            mbMasuTypeFindLink(mbMasuFind_AttrIdGet(-1, 0x8000), 0));
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
    if (capsuleNo < 0 || capsuleNo >= 0x35) {
        return FALSE;
    }
    if (capsuleData[capsuleNo].file == 0) {
        return FALSE;
    }
    return TRUE;
}

int mbCapValidListGet(int *list)
{
    int i;
    int num;

    i = 0;
    num = 0;
    for (; i < 0x21; i++) {
        if (mbCapValidCheck(i)) {
            list[num] = i;
            num++;
        }
    }
    return num;
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


const float lbl_802C4598 = 8.0f;

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
