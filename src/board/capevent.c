#define _MATH_H
#include "dolphin/math.h"

#include "game/gamework.h"
#include "game/charman.h"
#include "game/flag.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/coin.h"
#include "game/board/object.h"
#include "game/board/masu.h"
#include "game/object.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/esprite.h"
#include "game/process.h"
#include "game/board/window.h"

#include "humath.h"
#include "messdir_enum.h"

float mbSinDeg(float deg);
float mbCosDeg(float deg);
s8 mbPadStkXGet(int playerNo);
s8 mbPadStkYGet(int playerNo);

#define CAPSULE_KOOPA 43

#define CAPSULE_INVALID -99

#define CAP_EFF_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

typedef void (*CAPSULE_HOOK)(int, int, int, BOOL, BOOL, BOOL);
static int capsuleChoice;
int lbl_802C0FD8;
static int bonusCoinNum;
static int bonusCoinWinId;
static CAPSULE_HOOK capsuleHook;
s16 *mbCapEffData;
u32 mbCapEffNum;
static ANIMDATA *electricEffAnim;
static ANIMDATA *ringHitEffAnim2;
static ANIMDATA *ringHitEffAnim1;
static ANIMDATA *boostEffAnim;
static int biriQMasuNum;


static HUPROCESS *ev_CapBonusCoinProc[GW_PLAYER_MAX];
static char ev_CapBonusCoinMes[16];
static HUPROCESS *ev_CapMainProc[8];
static OMOBJ *ev_CapEffExplodeOMObj[8];
static OMOBJ *ev_CapEffBoostOMObj[8];
static OMOBJ *ev_CapEffSnowOMObj[8];
static OMOBJ *ev_CapEffGlowOMObj[8];
static OMOBJ *ev_CapEffRingOMObj[8];
static OMOBJ *ev_CapEffElectricOMObj[8];
static OMOBJ *ev_CapEffCoinOMObj[8];
static OMOBJ *ev_CapEffCoinManOMObj[8];
static OMOBJ *ev_CapEffStarManOMObj[8];
static OMOBJ *ev_CapEffCapLoseOMObj[8];
static OMOBJ *ev_CapEffRayOMObj[8];
static OMOBJ *ev_CapEffMasuHitOMObj[8];
static OMOBJ *ev_CapEffMoveOMObj[8];
typedef struct CapBonusCoinWork {
    int playerNo;
    int coinNum;
    BOOL highF;
} CAPBONUSCOINWORK;

typedef struct CapEffBoostWork {
    int modelId;
    int time;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFBOOSTWORK;


typedef struct CapEffBoostParticleData {
    s16 time;
    s16 timeTotal;
    u8 _unk04[4];
    HuVecF vel;
    float alpha;
    u8 _unk18[4];
    float angleStep;
    u8 _unk20[0x20];
    float active;
    u8 _unk44[0x10];
    float angle;
    HuVecF pos;
    GXColor color;
    int pat;
} CAPEFFBOOSTPARTICLEWORK;


typedef struct CapEffExplodeWork {
    int modelId;
    int num;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFEXPLODEWORK;

typedef struct CapEffSnowWork {
    int modelId;
    int num;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFSNOWWORK;


typedef struct CapEffGlowWork {
    int modelId;
    int num;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFGLOWWORK;


typedef struct CapEffGlowParticleData {
    s16 mode;
    s16 phase;
    s16 cycle;
    u8 _unk06[2];
    HuVecF vel;
    float scale;
    float time;
    float timeStep;
    u8 _unk20[4];
    float gravity;
    float rotStep;
    u8 _unk2C[0xC];
    float alpha;
    float alphaMax;
    float active;
    float sizeX;
    float sizeY;
    float rotX;
    float rotY;
    float angle;
    HuVecF pos;
    GXColor color;
    int pat;
} CAPEFFGLOWPARTICLEWORK;


typedef struct CapEffSnowParticleWork {
    s16 angle;
    u8 _unk02[6];
    float xAmplitude;
    float yVelocity;
    float _unk10;
    float time;
    float timeStep;
    u8 _unk1C[0x24];
    float active;
    u8 _unk44[0x14];
    HuVecF pos;
    GXColor color;
    u8 _unk68[4];
} CAPEFFSNOWPARTWORK;


typedef struct CapEffExplodeParticleWork {
    u8 _unk00[0x20];
    u8 blendMode;
    u8 _unk21;
    u8 dispAttr;
    u8 _unk23;
    u8 _unk24[0x14];
    ANIMDATA *animP;
    void *data;
} CAPEFFEXPLODEPARTWORK;


typedef struct CapEffExplodeParticleData {
    s16 mode;
    s16 _unk02;
    u8 _unk04[4];
    HuVecF vel;
    u8 _unk14[8];
    float angleStep;
    u8 _unk20[0x18];
    float fadeTime;
    float fadeStep;
    float active;
    u8 _unk44[0x10];
    float angle;
    HuVecF pos;
    GXColor color;
    int pat;
} CAPEFFEXPLODEPARTICLEWORK;


typedef struct CapEffBoostParticleWork {
    u8 _unk00[0x20];
    u8 blendMode;
} CAPEFFBOOSTPARTWORK;

typedef struct CapEffGlowParticleWork {
    u8 _unk00[0x20];
    u8 pat;
    u8 blendMode;
    u8 _unk22[0x16];
    ANIMDATA *animP;
} CAPEFFGLOWPARTWORK;

typedef struct CapEffDispWork {
    u8 _unk00[4];
    int dispF;
} CAPEFFDISPWORK;

typedef struct CapEffGlowKinokoParticleSystemWork {
    u8 _unk00[0x20];
    u8 _unk20;
    u8 _unk21[5];
    s16 num;
    u8 _unk28[0x14];
    void *data;
    u8 _unk40[0x10];
    HuVec2f *grid;
    u8 _unk54[4];
    int gridNum;
    u8 _unk5C[4];
} CAPEFFGLOWKINOKOPARTICLESYSTEMWORK;

typedef struct CapEffParticleSystemWork {
    s16 mode;
    s16 phase;
    u8 _unk04[0x1C];
    u8 dispAttr;
    u8 _unk21;
    u8 blendMode;
    u8 _unk23[3];
    s16 num;
    int _unk28;
    u8 _unk2C[4];
    int _unk30;
    u32 displayListSize;
    ANIMDATA *animP;
    CAPEFFGLOWPARTICLEWORK *data;
    HuVecF *vertices;
    HuVec2f *texCoords;
    void *displayList;
    int _unk4C;
    HuVec2f *grid;
    int _unk54;
    int gridNum;
    int _unk5C;
} CAPEFFPARTICLESYSTEMWORK;

typedef struct CapEffGlowKinokoParticleWork {
    s16 _unk00;
    s16 _unk02;
    s16 _unk04;
    u8 _unk06[0x66];
} CAPEFFGLOWKINOKOPARTICLEWORK;


typedef struct CapEffRingWork {
    int modelId[3];
    int dispF;
    int objIdx;
    ANIMDATA *animP[3];
} CAPEFFRINGWORK;


typedef struct CapEffRingParticleWork {
    s16 _unk00;
    s16 _unk02;
    u8 _unk04[4];
    HuVecF _unk08;
    float _unk14;
    float _unk18;
    float _unk1C;
    u8 _unk20[0x20];
    float _unk40;
    u8 _unk44[8];
    HuVecF _unk4C;
    HuVecF _unk58;
    GXColor color;
    int _unk68;
} CAPEFFRINGPARTICLEWORK;


typedef struct CapEffMasuHitParticleWork {
    s16 _unk00;
    s16 _unk02;
    s16 _unk04;
    u8 _unk06[2];
    HuVecF _unk08;
    HuVecF _unk14;
    HuVecF _unk20;
    float _unk2C;
    float _unk30;
    u8 _unk34[0xC];
    float _unk40;
    u8 _unk44[0x10];
    float _unk54;
    HuVecF _unk58;
    GXColor color;
    int _unk68;
} CAPEFFMASUHITPARTICLEWORK;


typedef struct CapEffRingHitParticleWork {
    u8 _unk00[0x20];
    u8 blendMode;
    u8 _unk21;
    u8 dispAttr;
} CAPEFFRINGHITPARTWORK;


typedef struct CapEffRayParticleWork {
    int index;
    int state;
    int _unk08;
    int _unk0C;
    float _unk10;
    float _unk14;
    HuVecF _unk18;
    HuVecF _unk24;
    HuVecF _unk30;
    HuVecF _unk3C;
    HuVecF vtx[16];
    GXColor color[16];
    HuVecF prevVtx[16];
    GXColor colorLerp[8];
} CAPEFFRAYPARTICLEWORK;


typedef struct CapEffCoinWork {
    int modelId;
    int objIdx;
    int activeF;
    int _unk0C;
    int _unk10;
    int _unk14;
    float _unk18;
    float _unk1C;
    float maxY;
    float _unk24;
    float _unk28;
    HuVecF _unk2C;
    HuVecF _unk38;
    HuVecF _unk44;
    HuVecF _unk50;
    OMOBJ *glowObj;
} CAPEFFCOINWORK;


typedef struct CapEffMoveWork {
    int playerNo;
    int state;
    int motNo;
    int nextMotNo;
    BOOL useMotF;
    BOOL useShiftF;
    int minYF;
    float minY;
    float vel;
    HuVecF pos;
    HuVecF velocity;
    HuVecF posStart;
    HuVecF posEnd;
    HuVecF moveDir;
    HuVecF rot;
    int moveTime;
    int time;
    float rotSpeed;
} CAPEFFMOVEWORK;


typedef struct CapObjMotionWork {
    int _unk00;
    int modelId;
    int time;
    int motNo;
    int nextMotNo;
    u32 attr;
    u32 _unk18;
    BOOL shiftF;
    u32 nextAttr;
} CAPOBJMOTIONWORK;


typedef struct CapEffElectricPartWork {
    int activeNo;
    int phase;
    int phaseMax;
    int time;
    int timeMax;
    HuVecF pos0;
    HuVecF pos1;
    HuVecF pos2;
    int _unk38;
    int _unk3C;
    float length;
    HuVecF posHist[6];
    int modelId;
    HuVecF modelPos;
} CAPEFFELECTRICPARTWORK;


typedef struct CapEffElectricWork {
    int modelId;
    int num;
    int objIdx;
    ANIMDATA *animP;
    CAPEFFELECTRICPARTWORK part[32];
} CAPEFFELECTRICWORK;


typedef struct CapEffRayWork {
    int modelId;
    int objIdx;
    float alpha;
    void *displayList;
    int displayListSize;
    CAPEFFRAYPARTICLEWORK *particleP;
} CAPEFFRAYWORK;


typedef struct CapEffMasuHitWork {
    int modelId;
    int _unk04;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFMASUHITWORK;

typedef struct CapEffOpenWork {
    int masuId;
    int timeMax;
    int mode;
    HuVecF pos;
} CAPEFFOPENWORK;


typedef struct CapCoinManWork {
    u8 _unk00[4];
    int activeF;
    u8 _unk08[0x34];
} CAPCOINMANWORK;

typedef struct CapStarManWork {
    u8 _unk00[4];
    int activeF;
    u8 _unk08[0x34];
} CAPSTARMANWORK;

typedef struct CapEffCapLoseWork {
    int objIdx;          /* 0x00; owner slot on entry zero */
    int activeF;         /* 0x04 */
    int colorObjId;      /* 0x08 */
    int capsuleNo;       /* 0x0C */
    int _unk10;          /* 0x10 */
    int _unk14;          /* 0x14 */
    int time;            /* 0x18 */
    HuVecF pos;          /* 0x1C */
    HuVecF vel;          /* 0x28 */
} CAPEFFCAPLOSEWORK;


#define CAP_WORK_MAX 64

typedef struct EvCapWork {
    int motId[CAP_WORK_MAX][GW_PLAYER_MAX];
    int objId[CAP_WORK_MAX];
    int sprId[CAP_WORK_MAX];
    void *mem[CAP_WORK_MAX];
    int masuId[CAP_WORK_MAX];
    HuVecF objPos[CAP_WORK_MAX];
    int playerMasuId[GW_PLAYER_MAX];
    HuVecF playerPos[GW_PLAYER_MAX];
    int bgId;
    OMOBJ *obj;
} EVCAPWORK;

typedef struct CapWorkFlag {
    u8 _flag00 : 1;
    u8 _flag01 : 1;
    u8 _flag02 : 1;
    u8 _flag03 : 1;
    u8 _flag04 : 1;
    u8 _flag05 : 1;
    u8 _flag06 : 1;
    u8 _flag07 : 1;
    u8 _flag08 : 1;
    u8 _flag09 : 1;
    u8 _flag10 : 1;
    u8 _flag11 : 1;
    u8 _flag12 : 1;
    u8 _flag13 : 1;
    u8 _flag14 : 1;
    u8 _flag15 : 1;
    u8 _flag16 : 1;
    u8 _flag17 : 1;
    u8 _flag18 : 1;
    u8 _flag19 : 1;
    u8 _flag20 : 1;
    u8 _flag21 : 1;
    u8 _flag22 : 1;
    u8 _flag23 : 1;
    u8 _flag24 : 1;
    u8 _flag25 : 1;
    u8 _flag26 : 1;
    u8 _flag27 : 1;
    u8 _flag28 : 1;
    u8 _flag29 : 1;
    u8 _flag30 : 1;
    u8 _flag31 : 1;
} CAPWORKFLAG;

typedef struct CapWork {
    int playerNo;
    int targetPlayerNo;
    int capsuleNo;
    int masuId;
    int masuIdNext;
    int _unk14;
    int _unk18;
    int _unk1C;
    EVCAPWORK objWork;
    CAPWORKFLAG flags;
    int _unkB6C;
    u8 _unkB70[0x5C];
    int processNo;
    OMOBJ *explodeObj;
    OMOBJ *boostObj;
    OMOBJ *snowObj;
    OMOBJ *glowObj;
    OMOBJ *ringObj;
    OMOBJ *coinObj;
    OMOBJ *coinManObj;
    OMOBJ *starManObj;
    OMOBJ *capLoseObj;
} CAPWORK;


typedef struct EvCapsuleData {
    void (*main)(void);
    void (*unk04)(void);
    void (*unk08)(CAPWORK *);
    int unk0C;
    int unk10;
    int bgDataNum;
    int unk18;
} EVCAPSULEDATA;

static int kettouCoinLose = 10;
static int kettouOppCoinLose = 5;
static int capsuleMasuType = -1;
static int capsuleMasuId = -1;
static int capsulePlayer = -1;
static int capsuleMasuPlayer = -1;
static int capsuleEventMasu = -1;
static int capsuleEventPlayer = -1;
static int capsuleEventPrevMasu = -1;
static int capsuleEventPrevPlayer = -1;
char lbl_802BFE90[] = "%d";
static GXColor capsuleRingColor = { 255, 255, 127, 255 };
static GXColor ev_CapsuleRandomColorTbl[7] = {
    { 255, 127, 127, 255 },
    { 255, 127, 64, 255 },
    { 255, 255, 127, 255 },
    { 127, 255, 127, 255 },
    { 127, 127, 255, 255 },
    { 64, 64, 255, 255 },
    { 255, 127, 255, 255 }
};
static HuVecF ev_CapsuleViewOfs = { 0.0f, 100.0f, 0.0f };
static int ev_CapEffRingFile[] = {
    DATANUM(DATA_capsule, 0x31),
    DATANUM(DATA_capsule, 0x32),
    DATANUM(DATA_capsule, 0x33),
    DATANUM(DATA_capsule, 0x31),
    DATANUM(DATA_capsule, 0x38),
    DATANUM(DATA_capsule, 0x38),
};
static GXColor ev_CapEffElectricColor[] = {
    { 192, 192, 255, 255 },
    { 164, 192, 255, 255 },
    { 192, 255, 255, 255 },
    { 192, 164, 255, 255 },
};
static HuVecF basePos[] = {
    { -0.5f, 0.5f, 0.0f },
    { 0.5f, 0.5f, 0.0f },
    { 0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f },
};
static float baseST1[] = {
    0.0f, 0.0f,
    0.25f, 0.0f,
    0.25f, 0.25f,
    0.0f, 0.25f,
};
static float baseST2[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};
static HuVecF viewRot = { -33.0f, 0.0f, 0.0f };
static int chanceTbl[] = { 100, 60, 30, 10 };
static void ev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum, BOOL highF,
    void (*hook)(void));
static float ev_CapRotCamera(float angle);
static void ev_CapComChoiceHook(void);
static void ev_CapWorkOMExec(OMOBJ *obj);
static void ev_CapWorkInit(EVCAPWORK *work, int bgId);
static void ev_CapWorkClose(EVCAPWORK *work);
static void ev_CapCall(CAPWORK *work, BOOL waitF);
void mbev_CapWait(CAPWORK *work);
static void ev_CapBiriQShockOMExec(OMOBJ *obj);
static void ev_CapKill(void);
static void ev_CapBiriQMetalShock(void);
static void ev_CapBiriQMetalShockDestroy(void);
static void ev_CapBonusCoin(void);
static void ev_CapBonusCoinKill(void);
static void ev_CapBonusCoinWin(void);
void mbev_CapBiriQMetalShock(void *workP);
void mbev_CapEffExplodeOMExec(OMOBJ *obj);
void mbev_CapEffBoostOMExec(OMOBJ *obj);
void mbev_CapEffSnowOMExec(OMOBJ *obj);
void mbev_CapEffGlowOMExec(OMOBJ *obj);
void mbev_CapEffRingOMExec(OMOBJ *obj);
void mbev_CapEffRayOMExec(OMOBJ *obj);
void mbev_CapEffRayDraw(HU3D_MODEL *modelP, Mtx *mtx);
void mbev_CapEffMasuHitOMExec(OMOBJ *obj);
void mbev_CapEffCoinOMExec(OMOBJ *obj);
void mbev_CapEffCapLoseOMExec(OMOBJ *obj);
void mbev_CapEffElectricOMExec(OMOBJ *obj);
void mbev_CapPlayerMoveObjExec(OMOBJ *obj);
float mbev_CapAngleWrap(float a, float b);
void mbev_CapBezierGetV(float t, float *a, float *b, float *c, float *out);
void mbev_CapEffExplodeKill(OMOBJ *obj);
void mbev_CapEffBoostKill(OMOBJ *obj);
void mbev_CapEffSnowKill(OMOBJ *obj);
void mbev_CapEffGlowKill(OMOBJ *obj);
void mbev_CapEffRingKill(OMOBJ *obj);
void mbev_CapEffRayKill(OMOBJ *obj);
void mbev_CapEffMasuHitKill(OMOBJ *obj);
void mbev_CapCoinManKill(OMOBJ *obj);
void mbev_CapStarManKill(OMOBJ *obj);
void mbev_CapEffCapLoseKill(OMOBJ *obj);
void mbev_CapObjMotionOMExec(OMOBJ *obj);
int mbCapObjColorCreate(int capsuleNo, BOOL createF);
void mbCapObjColorPosSet(int id, float x, float y, float z);
void mbCapObjColorScaleSet(int id, float x, float y, float z);
void mbCapObjColorLayerSet(int id, u8 layer);
void mbev_CapEffColorSet(GXColor *color, int colorNo);
static void ev_CapEffDraw(HU3D_MODEL *modelP, Mtx *mtx);
static void ev_CapEffGridSet(s16 modelId, int xNum, int yNum, int zNum);
int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot, HuVecF *scale,
    int unk10, int unk14, int unk18, GXColor *color);
OMOBJ *mbev_CapEffRingCreate(void);
OMOBJ *mbev_CapEffRayCreate(float scale, float speed);
OMOBJ *mbev_CapEffMasuHitCreate(void);
int mbev_CapEffRayAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB,
    int time, float scale);
void mbev_CapEffRayAlphaSet(OMOBJ *obj, float alpha);
int mbev_CapEffMasuHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA,
    HuVecF *rotB, float scale, float scaleY, int time);
static s16 ev_CapEffCreate(ANIMDATA *animP, s16 max);
OMOBJ *mbev_CapEffCoinCreate(void);
void mbev_CapEffCoinKill(OMOBJ *obj);

extern s16 mbCapValueTypeGet(s16 value);
extern s16 mbCapValuePlayerGet(s16 value);
extern s16 mbCapUseModeGet(s16 capsuleNo);
extern EVCAPSULEDATA ev_CapsuleData[];
void mbev_CapEffOpenCreate(int playerNo, int masuId, BOOL unk08, BOOL unk0C,
    BOOL unk10);
static void ev_CapEffOpen(void);
static void ev_CapEffOpenKill(void);
BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);
extern const float lbl_802C4688;
extern const float lbl_802C47C8;
void mbev_CapMoveMasuSet(int playerNo, int masuId);
void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF);
BOOL mbev_CapStatusDispCheck(int playerNo);
void mbev_CapCameraViewSet(int playerNo, int viewNo, BOOL stopF);

int mbev_CapCall(int playerNo, int capsuleValue, BOOL moveF, BOOL stopF)
{
    CAPWORK work;
    int masuId;
    int targetPlayerNo;
    int cameraView;
    BOOL moveNumDispF = FALSE;

    masuId = GwPlayer[playerNo].masuId;
    if (moveF) {
        targetPlayerNo = mbCapValuePlayerGet((s16)capsuleValue);
        capsuleValue = mbCapValueTypeGet((s16)capsuleValue);
        if (!mbCapValidCheck(capsuleValue)) {
            return FALSE;
        }
    } else if (stopF) {
        if (playerNo == capsuleEventPlayer
            && capsuleEventMasu == GwPlayer[playerNo].masuId) {
            capsuleEventMasu = -1;
            capsuleEventPlayer = -1;
            return FALSE;
        }
        capsuleEventMasu = -1;
        capsuleEventPlayer = -1;
        if (playerNo == capsulePlayer
            && capsuleMasuId == GwPlayer[playerNo].masuId) {
            capsuleValue = (capsuleMasuPlayer << 8) | capsuleMasuType;
        }
        capsuleMasuType = -1;
        capsuleMasuId = -1;
        capsulePlayer = -1;
        capsuleMasuPlayer = -1;
        if (mbMasuTypeGet((s16)masuId) != 1
            && mbMasuTypeGet((s16)masuId) != 2) {
            return FALSE;
        }
        targetPlayerNo = mbCapValuePlayerGet((s16)capsuleValue);
        capsuleValue = mbCapValueTypeGet((s16)capsuleValue);
        if (playerNo < 0 || playerNo >= GW_PLAYER_MAX
            || !mbCapValidCheck(capsuleValue)) {
            return FALSE;
        }
        if (mbCapUseModeGet(capsuleValue) != 2) {
            return FALSE;
        }
        if (mbev_CapPlayerCheck(targetPlayerNo, playerNo)) {
            return TRUE;
        }
    } else {
        if (playerNo == capsuleEventPrevPlayer
            && capsuleEventPrevMasu == GwPlayer[playerNo].masuId) {
            capsuleEventPrevMasu = -1;
            capsuleEventPrevPlayer = -1;
            return FALSE;
        }
        capsuleEventPrevMasu = -1;
        capsuleEventPrevPlayer = -1;
        if (mbMasuTypeGet((s16)masuId) != 1
            && mbMasuTypeGet((s16)masuId) != 2) {
            return FALSE;
        }
        targetPlayerNo = mbCapValuePlayerGet((s16)capsuleValue);
        capsuleValue = mbCapValueTypeGet((s16)capsuleValue);
        if (playerNo < 0 || playerNo >= GW_PLAYER_MAX
            || !mbCapValidCheck(capsuleValue)) {
            return FALSE;
        }
        if (mbCapUseModeGet(capsuleValue) == 2) {
            return FALSE;
        }
        if (!moveF && !stopF && mbCapValidCheck(capsuleValue)) {
            GwPlayer[playerNo].capsuleMasuNum++;
            if (GwPlayer[playerNo].capsuleMasuNum > 99) {
                GwPlayer[playerNo].capsuleMasuNum = 99;
            }
        }
        if (mbev_CapPlayerCheck(targetPlayerNo, playerNo)) {
            return TRUE;
        }
    }

    omVibrate(playerNo, 20, 4, 4);
    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = capsuleValue;
    work._unk14 = moveF;
    work._unk1C = 0;
    work._unk18 = stopF;
    work.masuId = GwPlayer[work.playerNo].masuId;
    work.masuIdNext = -1;
    if (!moveF) {
        work.targetPlayerNo = targetPlayerNo;
    }
    memset(&work.flags, 0, sizeof(CAPWORKFLAG));
    masuId = GwPlayer[work.playerNo].masuId;
    cameraView = mbCameraPlayerViewNoGet();
    mbAudFXPlay(1035);
    if (!moveF && !stopF) {
        mbev_CapEffOpenCreate(work.playerNo, work.masuId, TRUE, TRUE, TRUE);
    }
    if (ev_CapsuleData[capsuleValue].unk08 != NULL) {
        ev_CapWorkInit(&work.objWork, -1);
        ev_CapsuleData[capsuleValue].unk08(&work);
        ev_CapWorkClose(&work.objWork);
    }
    if (stopF && ev_CapsuleData[work.capsuleNo].unk18 != 0
        && mbCapUseModeGet(capsuleValue) == 2) {
        if ((capsuleValue == 22 || capsuleValue == 25)
            && GwPlayer[playerNo].moveNum <= 1) {
            mbev_CapMoveMasuSet(playerNo, GwPlayer[playerNo].masuId);
        }
        return TRUE;
    }
    switch (ev_CapsuleData[work.capsuleNo].unk10) {
        case 1:
            mbCameraPlayerViewSet(work.playerNo, 0);
            break;
        case 2:
            mbCameraPlayerViewSet(work.playerNo, 1);
            break;
    }
    switch (ev_CapsuleData[work.capsuleNo].unk0C) {
        case 0:
            if (!mbev_CapStatusDispCheck(work.playerNo)) {
                mbev_CapStatusDispSetAll(FALSE, TRUE);
                mbStatusDispFocusSet(work.playerNo, TRUE);
                while (!mbStatusMoveCheck(work.playerNo)) {
                    HuPrcVSleep();
                }
            }
            break;
        case 1:
            mbev_CapStatusDispSetAll(TRUE, FALSE);
            break;
        case 2:
            mbev_CapStatusDispSetAll(FALSE, FALSE);
            break;
    }
    mbPlayerRotateStart(work.playerNo, 0, 15);
    ev_CapCall(&work, TRUE);
    if (moveNumDispF) {
        mbMoveNumDispSet(work.playerNo, TRUE);
    }
    mbev_CapStatusDispSetAll(TRUE, TRUE);
    if (!stopF) {
        mbev_CapCameraViewSet(work.playerNo, cameraView, stopF);
    } else if (GwPlayer[work.playerNo].moveNum > 1) {
        mbCameraPlayerViewSet(work.playerNo, 2);
    } else {
        mbCameraPlayerViewSet(work.playerNo, 0);
    }
    if (masuId != GwPlayer[work.playerNo].masuId || work.flags._flag00) {
        return FALSE;
    }
    return FALSE;
}

void mbev_CapCallKettou(int playerNo, int masuId, BOOL stopF)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 41;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = masuId;
    work.masuIdNext = -1;
    work.flags._flag01 = stopF;
    HuPrcSleep(10);
    mbAudFXPlay(1104);
    ev_CapCall(&work, TRUE);
}

void mbev_CapCallDonkey(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 44;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    omVibrate(playerNo, 20, 7, 3);
    ev_CapCall(&work, TRUE);
}

void mbev_CapCallKoopa(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 43;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    ev_CapCall(&work, TRUE);
}

void mbev_CapCallTeresa(int playerNo, int masuId)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 46;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = masuId;
    work.masuIdNext = -1;
    ev_CapCall(&work, TRUE);
}

void mbev_CapCallMiracle(int playerNo, int masuId)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 42;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = masuId;
    work.masuIdNext = -1;
    ev_CapCall(&work, TRUE);
}





void mbev_CapBiriQShockCreate(int playerNo)
{
    OMOBJ *obj;
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 21;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    biriQMasuNum = 60;
    obj = omAddObjEx(mbObjMan, 32768, 0, 0, -1, ev_CapBiriQShockOMExec);
    mbev_CapBiriQMetalShock(&work);
    biriQMasuNum = 0;
}

void mbev_CapBiriQMetalShockCreate(int playerNo)
{
    HUPROCESS *process;
    CAPWORK *workP;
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 21;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    work.flags._flag07 = TRUE;
    biriQMasuNum = 0;
    process = HuPrcChildCreate(ev_CapBiriQMetalShock, 8196, 24576, 0, mbMainProc);
    workP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPWORK), HU_MEMNUM_OVL);
    process->property = workP;
    memcpy(process->property, &work, sizeof(CAPWORK));
    HuPrcDestructorSet2(process, ev_CapBiriQMetalShockDestroy);
}

extern EVCAPSULEDATA ev_CapsuleData[];
extern int mbBGRead(int dataNum);
int mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
void mbev_CapBonusCoinCall(int playerNo, int capsuleNo, int coinNum,
    BOOL waitF);
void mbev_CapBonusCoin(int playerNo, int coinNum, BOOL waitF, BOOL flag);
int mbCapBonusCoinNumGet(int playerNo, int capsuleNo);
int mbev_CapPlayerSquishVoiceSet(int *playerNo, int masuId, BOOL voiceF);
BOOL mbev_CapCullCheck(int playerNo, int masuId);
BOOL mbev_CapPointCullCheck(HuVecF *pos);
int mbev_CapPlayerComSelSameGet(int playerNo, int selection, BOOL sameF);
int mbev_CapPlayerComSelRandomGet(int playerNo, int selection, int *playerList,
    int playerNum);
void mbPos3DtoNorm(HuVecF *src, s16 cameraMask, HuVecF *dst);

extern s16 mbCapMasuTypeGet(s16 masuId);
extern s16 mbCapValuePlayerGet(s16 value);
extern s16 mbCapMasuDispTypeGet(s16 masuId);
void mbev_CapEffOpenCreate(int playerNo, int masuId, BOOL unk08, BOOL unk0C, BOOL unk10);
BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);

BOOL mbev_CapCallTrap(int playerNo, int masuId, int masuIdNext)
{
    int targetPlayerNo;
    CAPWORK work;

    targetPlayerNo = mbCapValuePlayerGet(mbMasuCapsuleGet(masuIdNext));
    if (mbCapMasuDispTypeGet(masuIdNext) != 2) {
        return FALSE;
    }
    if (mbev_CapPlayerCheck(targetPlayerNo, playerNo)) {
        return TRUE;
    }
    mbAudFXPlay(1035);
    if (GwPlayer[playerNo].biriQF) {
        biriQMasuNum = 60;
    }
    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = mbCapMasuTypeGet(masuIdNext);
    work._unk14 = 1;
    work._unk1C = 1;
    work._unk18 = 0;
    work.masuId = masuId;
    work.masuIdNext = masuIdNext;
    mbev_CapEffOpenCreate(work.playerNo, work.masuIdNext, TRUE, FALSE, FALSE);
    if (ev_CapsuleData[work.capsuleNo].unk18 != 0) {
        ev_CapCall(&work, FALSE);
    }
    return TRUE;
}

int mbev_CapBiriQShockDelayGet(int playerNo)
{
    if (GwPlayer[playerNo].biriQF) {
        return biriQMasuNum;
    }
    return 0;
}

static void ev_CapBiriQShockOMExec(OMOBJ *obj)
{
    if (mbExitCheck() || --biriQMasuNum <= 0) {
        biriQMasuNum = 0;
        omDelObjEx(mbObjMan, obj);
        return;
    }
}

void MBCapsuleStub5(void)
{
}

void MBCapsuleStub6(void)
{
}

void MBCapsuleStub7(void)
{
}

void mbev_CapMoveMasuSet(int playerNo, int masuId)
{
    capsuleEventPlayer = playerNo;
    capsuleEventMasu = masuId;
}

void mbev_CapStopMasuSet(int playerNo, int masuId)
{
    capsuleEventPrevPlayer = playerNo;
    capsuleEventPrevMasu = masuId;
}

void mbev_CapVsEndCall(void)
{
}

void mbev_CapKillerCall(void)
{
}

void mbev_CapKillerMultiCall(void)
{
}

void MBCapsuleStub11(void)
{
}

void MBCapsuleStub12(void)
{
}

BOOL mbev_CapKillerMoveCheck(int playerNo)
{
    return TRUE;
}

BOOL mbev_CapKillerMoveCheckAll(void)
{
    return TRUE;
}

int mbev_CapCapGet(void)
{
    return -1;
}

int mbev_CapBankCoinGet(void)
{
    return GwSystem.bankCoin;
}

int mbev_CapKettouCoinLoseGet(void)
{
    return kettouCoinLose;
}

int mbev_CapKettouOppCoinLoseGet(void)
{
    return kettouOppCoinLose;
}

int mbev_CapKettouCoinLoseGet2(void)
{
    return kettouCoinLose;
}

int mbev_CapKettouOppCoinLoseGet2(void)
{
    return kettouOppCoinLose;
}

void mbev_CapOpeningAdd(int capsuleNum)
{
}

void mbev_CapKoopaAdd(void)
{
}

void mbev_CapBubbleHookSet(CAPSULE_HOOK hook)
{
    capsuleHook = hook;
}

void mbev_CapBubbleHookCall(int type, int modelId, BOOL flag1, BOOL flag2, BOOL flag3)
{
    if (capsuleHook != NULL) {
        capsuleHook(-1, type, modelId, flag1, flag2, flag3);
    }
}

void mbev_CapBubbleHookCallStory(int eventType, int type, int modelId)
{
    if (capsuleHook != NULL) {
        switch (eventType) {
            case 0:
                capsuleHook(CAPSULE_KOOPA, type, modelId, FALSE, FALSE, FALSE);
                break;
            case 1:
                capsuleHook(CAPSULE_INVALID, type, modelId, FALSE, FALSE, FALSE);
                break;
        }
    }
}

void mbev_CapBankCoinInit(void)
{
    GwSystem.bankCoin = 0;
}


static void ev_CapWorkOMExec(OMOBJ *obj)
{
    EVCAPWORK *work;
    HuVecF pos;
    int i;

    work = obj->data;
    if (mbExitCheck() || obj->work[0] != 0) {
        obj->data = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->objId[i] != -1 && work->masuId[i] != -1) {
            mbMasuPosGet(work->masuId[i], &pos);
            PSVECAdd(&pos, &work->objPos[i], &pos);
            mbObjPosSetV(work->objId[i], &pos);
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (work->playerMasuId[i] != -1) {
            mbMasuPosGet(work->playerMasuId[i], &pos);
            PSVECAdd(&pos, &work->playerPos[i], &pos);
            mbPlayerPosSetV(i, &pos);
        }
    }
}

s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo, int dataNum)
{
    int i;

    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->motId[i][playerNo] == -1) {
            break;
        }
    }
    if (i >= CAP_WORK_MAX) {
        return -1;
    }
    work->motId[i][playerNo] = mbPlayerMotionCreate(playerNo, dataNum);
    HuPrcVSleep();
    return work->motId[i][playerNo];
}

int mbev_CapObjCreate(
    EVCAPWORK *work,
    int dataNum,
    int *motFile,
    BOOL linkF,
    int delay,
    BOOL closeDir)
{
    int i;
    int objIdx;

    for (objIdx = 0; objIdx < CAP_WORK_MAX; objIdx++) {
        if (work->objId[objIdx] == -1) {
            break;
        }
    }
    if (objIdx >= CAP_WORK_MAX) {
        return -1;
    }
    work->objId[objIdx] = mbObjCreate(dataNum, NULL, linkF);
    if (closeDir) {
        HuDataDirClose(dataNum);
    }
    mbObjDispSet(work->objId[objIdx], FALSE);
    if (delay > 0) {
        HuPrcSleep(delay);
    }
    if (work->objId[objIdx] != -1 && motFile != NULL) {
        for (i = 0; motFile[i] != -1; i++) {
            mbObjMotionCreate(work->objId[objIdx], motFile[i]);
            if (delay > 0) {
                HuPrcVSleep();
            }
        }
    }
    mbObjDispSet(work->objId[objIdx], TRUE);
    mbObjLayerSet(work->objId[objIdx], 3);
    return work->objId[objIdx];
}



void mbev_CapObjClose(EVCAPWORK *work, int objId)
{
    int i;

    if (objId == -1) {
        return;
    }
    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->objId[i] == objId) {
            mbObjKill(objId);
            work->objId[i] = -1;
        }
    }
}

s16 mbev_CapSprCreate(EVCAPWORK *work, unsigned int dataNum, s16 prio, s16 bank)
{
    int i;

    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->sprId[i] == -1) {
            break;
        }
    }
    if (i >= CAP_WORK_MAX) {
        return -1;
    }
    work->sprId[i] = espEntry(dataNum, prio, bank);
    espDrawNoSet(work->sprId[i], 32);
    return work->sprId[i];
}

void mbev_CapSprClose(EVCAPWORK *work, s16 sprId)
{
    int i;

    if (sprId == -1) {
        return;
    }
    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->sprId[i] == sprId) {
            espKill(sprId);
            work->sprId[i] = -1;
        }
    }
}

void *mbev_CapMalloc(EVCAPWORK *work, int size)
{
    int i;
    void *ptr;

    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->mem[i] == NULL) {
            break;
        }
    }
    if (i >= CAP_WORK_MAX) {
        return NULL;
    }
    ptr = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    work->mem[i] = ptr;
    return work->mem[i];
}

void mbev_CapMallocClose(EVCAPWORK *work, void *ptr)
{
    int i;

    if (ptr == NULL) {
        return;
    }
    for (i = 0; i < CAP_WORK_MAX; i++) {
        if (work->mem[i] == ptr) {
            HuMemDirectFree(ptr);
            work->mem[i] = NULL;
        }
    }
}

void mbev_CapEffOpenCreate(int playerNo, int masuId, BOOL createF, BOOL mode,
    BOOL keepCapsuleF)
{
    HUPROCESS *process;
    void *workData;
    CAPEFFOPENWORK *workP;
    int capsuleValue;

    if (createF) {
        process = HuPrcChildCreate(
            ev_CapEffOpen, 0x2004, 0x6000, 0, mbMainProc);
        HuPrcDestructorSet2(process, ev_CapEffOpenKill);
        workData = HuMemDirectMallocNum(
            HEAP_HEAP, sizeof(CAPEFFOPENWORK), HU_MEMNUM_OVL);
        workP = workData;
        process->property = workP;
        memset(workP, 0, sizeof(CAPEFFOPENWORK));
        workP->masuId = masuId;
        workP->timeMax = 25;
        workP->pos.x = workP->pos.y = workP->pos.z = 0.0f;
        if (!mode) {
            workP->pos.y += 100.0f;
            workP->mode = 0;
        } else {
            workP->mode = 1;
        }
    }
    if (mode) {
        if (!keepCapsuleF) {
            mbMasuCapsuleSet((s16)masuId, -1);
        }
    } else {
        capsuleValue = mbMasuCapsuleGet((s16)masuId);
        capsuleMasuType = mbCapValueTypeGet((s16)capsuleValue);
        capsuleMasuId = masuId;
        capsulePlayer = playerNo;
        capsuleMasuPlayer = mbCapValuePlayerGet((s16)capsuleValue);
        mbMasuCapsuleSet((s16)masuId, -1);
    }
}

static void ev_CapEffOpen(void)
{
    HUPROCESS *process;
    CAPEFFOPENWORK *workP;
    OMOBJ *rayObj;
    OMOBJ *ringObj;
    OMOBJ *masuHitObj;
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    HuVecF scale;
    HuVecF particlePos;
    GXColor color;
    int i;
    int j;
    int time;
    float weight;
    float particleScale;
    float particleScaleY;

    process = HuPrcCurrentGet();
    workP = process->property;
    rayObj = mbev_CapEffRayCreate(1.0f, 0.0f);
    ringObj = mbev_CapEffRingCreate();
    masuHitObj = mbev_CapEffMasuHitCreate();
    mbMasuPosGet((s16)workP->masuId, &pos);
    PSVECAdd(&pos, &workP->pos, &pos);
    scale.x = scale.z = 0.0f;
    scale.y = 5.0f;
    PSVECAdd(&pos, &scale, &pos);

    if (workP->mode != 0) {
        rot.x = -90.0f;
        rot.y = rot.z = 0.0f;
        scale.x = 1.0f;
        scale.y = 2.0f;
        scale.z = 100.0f;
        mbev_CapEffColorSet(&color, mbRandMod(0x8000));
        mbev_CapEffRingAdd(
            ringObj, &pos, &rot, &scale, 1, 10, 0, &color);
        scale.x = 2.0f;
        scale.y = 2.5f;
        scale.z = 200.0f;
        mbev_CapEffColorSet(&color, mbRandMod(0x8000));
        mbev_CapEffRingAdd(
            ringObj, &pos, &rot, &scale, 3, 10, 1, &color);

        for (i = 0; i <= workP->timeMax; i++) {
            weight = cos((M_PI * (90.0f
                * ((float)i / (float)workP->timeMax))) / 180.0);
            mbMasuPosGet((s16)workP->masuId, &pos);
            PSVECAdd(&pos, &workP->pos, &pos);
            for (j = 0; j < 3; j++) {
                particlePos = pos;
                rot.x = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                rot.y = 360.0f * MBCapsuleEffRandF();
                rot.z = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.x = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.y = rot.y + (60.0f * MBCapsuleEffRandF());
                vel.z = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                time = 10.0f + (5.0f * MBCapsuleEffRandF());
                particleScale = 200.0f * (1.0f
                    + (0.25f * MBCapsuleEffRandF()));
                mbev_CapEffRayAdd(rayObj, &particlePos, &rot, &vel, time,
                    particleScale);
            }
            mbev_CapEffRayAlphaSet(rayObj, weight);
            for (j = 0; (float)j < 5.0f * weight; j++) {
                particlePos = pos;
                rot.x = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                rot.y = 360.0f * MBCapsuleEffRandF();
                rot.z = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.x = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.y = 360.0f * MBCapsuleEffRandF();
                vel.z = 180.0f * (-0.5f
                    + MBCapsuleEffRandF());
                time = 10.0f + (5.0f * MBCapsuleEffRandF());
                particleScaleY = weight * (50.0f * (1.0f
                    + (0.5f * MBCapsuleEffRandF())));
                particleScale = 100.0f * (2.0f
                    + MBCapsuleEffRandF());
                mbev_CapEffMasuHitAdd(masuHitObj, &particlePos, &rot, &vel,
                    particleScale, particleScaleY, time);
            }
            HuPrcVSleep();
        }
    } else {
        mbCameraRotGet(&rot);
        scale.x = 1.0f;
        scale.y = 2.0f;
        scale.z = 100.0f;
        mbev_CapEffColorSet(&color, mbRandMod(0x8000));
        mbev_CapEffRingAdd(
            ringObj, &pos, &rot, &scale, 1, 10, 0, &color);
        scale.x = 2.0f;
        scale.y = 2.5f;
        scale.z = 200.0f;
        mbev_CapEffColorSet(&color, mbRandMod(0x8000));
        mbev_CapEffRingAdd(
            ringObj, &pos, &rot, &scale, 3, 10, 1, &color);

        for (i = 0; i <= workP->timeMax; i++) {
            weight = cos((M_PI * (90.0f
                * ((float)i / (float)workP->timeMax))) / 180.0);
            mbMasuPosGet((s16)workP->masuId, &pos);
            PSVECAdd(&pos, &workP->pos, &pos);
            for (j = 0; j < 3; j++) {
                particlePos = pos;
                rot.x = 360.0f * MBCapsuleEffRandF();
                rot.y = 360.0f * MBCapsuleEffRandF();
                rot.z = 360.0f * MBCapsuleEffRandF();
                vel.x = rot.x + (60.0f * (-0.5f
                    + MBCapsuleEffRandF()));
                vel.y = rot.y + (60.0f * (-0.5f
                    + MBCapsuleEffRandF()));
                vel.z = rot.z + (60.0f * (-0.5f
                    + MBCapsuleEffRandF()));
                time = 10.0f + (5.0f * MBCapsuleEffRandF());
                particleScale = 200.0f * (1.0f
                    + (0.25f * MBCapsuleEffRandF()));
                mbev_CapEffRayAdd(rayObj, &particlePos, &rot, &vel, time,
                    particleScale);
            }
            mbev_CapEffRayAlphaSet(rayObj, weight);
            for (j = 0; (float)j < 5.0f * weight; j++) {
                particlePos = pos;
                rot.x = 360.0f * MBCapsuleEffRandF();
                rot.y = 360.0f * MBCapsuleEffRandF();
                rot.z = 360.0f * MBCapsuleEffRandF();
                vel.x = 360.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.y = 360.0f * (-0.5f
                    + MBCapsuleEffRandF());
                vel.z = 360.0f * (-0.5f
                    + MBCapsuleEffRandF());
                time = 10.0f + (5.0f * MBCapsuleEffRandF());
                particleScaleY = weight * (50.0f * (1.0f
                    + (0.5f * MBCapsuleEffRandF())));
                particleScale = 100.0f * (2.0f
                    + MBCapsuleEffRandF());
                mbev_CapEffMasuHitAdd(masuHitObj, &particlePos, &rot, &vel,
                    particleScale, particleScaleY, time);
            }
            HuPrcVSleep();
        }
    }
    mbev_CapEffRayKill(rayObj);
    mbev_CapEffRingKill(ringObj);
    mbev_CapEffMasuHitKill(masuHitObj);
    HuPrcEnd();
}

void mbev_CapNullKill(void)
{
}

void mbev_CapDebugCam(void)
{
    CAPWORK *work;
    MBCAMERA *camera;
    HuVecF center;
    HuVecF centerOrig;
    HuVecF rot;
    HuVecF rotOrig;
    float zoom;
    float zoomOrig;
    float speed;
    float dispX;
    float dispY;
    int mode;
    BOOL objDispF;
    int objId;

    work = HuPrcCurrentGet()->property;
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    mbCameraMoveWait();
    camera = mbCameraGet();
    center.x = camera->center.x + camera->offset.x;
    center.y = camera->center.y + camera->offset.y;
    center.z = camera->center.z + camera->offset.z;
    rot = camera->rot;
    zoom = camera->zoom;
    centerOrig = center;
    rotOrig = rot;
    zoomOrig = zoom;
    mode = 0;
    objDispF = FALSE;
    dispX = 32.0f;
    dispY = 50.0f;
    objId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, 0x44), NULL, FALSE, 5, FALSE);
    mbObjPosSet(objId, center.x, center.y, center.z);
    mbObjDispSet(objId, FALSE);
    mbCameraFocusObjSet(objId);
    do {
        if (HuPadBtnDown[0] & PAD_BUTTON_X) {
            mode++;
            if (mode > 2) {
                mode = 0;
            }
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_Y) {
            objDispF ^= TRUE;
        }
        mbObjDispSet(objId, objDispF);
        if (mode == 0) {
            if (HuPadBtn[0] & PAD_TRIGGER_L) {
                speed = 2.0f;
            } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
                speed = 0.5f;
            } else {
                speed = 1.0f;
            }
            if (fabs((double)mbPadStkXGet(0)) > 8.0) {
                center.x += speed * (0.05f * (float)mbPadStkXGet(0));
            }
            if (fabs((double)mbPadStkYGet(0)) > 8.0) {
                center.z -= speed * (0.05f * (float)mbPadStkYGet(0));
            }
            if (HuPadBtn[0] & PAD_BUTTON_UP) {
                center.y += 25.0f * speed;
            }
            if (HuPadBtn[0] & PAD_BUTTON_DOWN) {
                center.y -= 25.0f * speed;
            }
            if (HuPadBtn[0] & PAD_BUTTON_A) {
                zoom += 25.0f * speed;
            }
            if (HuPadBtn[0] & PAD_BUTTON_B) {
                zoom -= 25.0f * speed;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_START) {
                center = centerOrig;
                rot = rotOrig;
                zoom = zoomOrig;
            }
            fontcolor = FONT_COLOR_WHITE;
            print8(36.0 + dispX, 84.0 + dispY, 1.5f,
                "\xFD\x01" "POSITION MODE");
        } else if (mode == 1) {
            if (HuPadBtn[0] & PAD_TRIGGER_L) {
                speed = 2.0f;
            } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
                speed = 0.5f;
            } else {
                speed = 1.0f;
            }
            if (fabs((double)mbPadStkXGet(0)) > 8.0) {
                rot.y += speed * (0.05f * (float)mbPadStkXGet(0));
            }
            if (fabs((double)mbPadStkYGet(0)) > 8.0) {
                rot.x += speed * (0.05f * (float)mbPadStkYGet(0));
            }
            if (rot.x >= 360.0f) {
                rot.x -= 360.0f;
            } else if (rot.x < 0.0f) {
                rot.x += 360.0f;
            }
            if (rot.y >= 360.0f) {
                rot.y -= 360.0f;
            } else if (rot.y < 0.0f) {
                rot.y += 360.0f;
            }
            if (HuPadBtn[0] & PAD_BUTTON_A) {
                zoom += 25.0f * speed;
            }
            if (HuPadBtn[0] & PAD_BUTTON_B) {
                zoom -= 25.0f * speed;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_START) {
                center = centerOrig;
                rot = rotOrig;
                zoom = zoomOrig;
            }
            fontcolor = FONT_COLOR_WHITE;
            print8(36.0 + dispX, 84.0 + dispY, 1.5f,
                "\xFD\x01" "ROTATE MODE");
        } else {
            if (fabs((double)mbPadStkXGet(0)) > 8.0) {
                dispX += 0.05f * (float)mbPadStkXGet(0);
            }
            if (fabs((double)mbPadStkYGet(0)) > 8.0) {
                dispY += 0.05f * (float)mbPadStkYGet(0);
            }
            fontcolor = FONT_COLOR_YELLOW;
            print8(36.0 + dispX, 84.0 + dispY, 1.5f,
                "\xFD\x01" "DISP MODE");
        }
        print8(36.0 + dispX, 96.0 + dispY, 1.5f,
            "\xFD\x01" "RX:%.4f RY:%.4f RZ:%.4f",
            rot.x, rot.y, rot.z);
        print8(36.0 + dispX, 108.0 + dispY, 1.5f,
            "\xFD\x01" "CX:%.4f CY:%.4f CZ:%.4f",
            center.x, center.y, center.z);
        print8(36.0 + dispX, 120.0 + dispY, 1.5f,
            "\xFD\x01" "ZM:%.4f", zoom);
        mbObjPosSet(objId, center.x, center.y, center.z);
        mbCameraEyeSet(center.x, center.y, center.z);
        mbCameraRotSet(rot.x, rot.y, rot.z);
        mbCameraZoomSet(zoom);
        HuPrcVSleep();
    } while (!(HuPadBtn[0] & PAD_TRIGGER_L)
        || !(HuPadBtn[0] & PAD_TRIGGER_R));
    mbCameraMoveOnSet(TRUE);
    mbCameraMovePos(&centerOrig, &rotOrig, NULL, zoomOrig, -1.0f, 1);
    mbCameraFocusPlayerSet(work->playerNo);
    HuPrcEnd();
}

void mbev_CapDebugCamKlll(void)
{
}

void mbev_CapDebugWarp(void)
{
    CAPWORK *work;
    HuVecF pos;
    HuVecF prevPos;
    HuVecF linkPos;
    HuVecF delta;
    HuVecF markerPos;
    HuVecF screenPos;
    s16 linkTbl[MASU_LINK_MAX * 2];
    float angleTbl[MASU_LINK_MAX * 2];
    float stickX;
    float stickY;
    float stickAngle;
    float angle;
    float weight;
    int playerNo;
    int masuId;
    int linkNum;
    int i;
    int j;
    int objId;
    s16 sprId;

    work = HuPrcCurrentGet()->property;
    playerNo = work->playerNo;
    mbCapEffUseCreate(playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(playerNo) >= 0) {
        HuPrcVSleep();
    }
    masuId = GwPlayer[playerNo].masuId;
    mbPlayerPosGet(playerNo, &pos);
    objId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, 0x44), NULL, FALSE, 5, FALSE);
    mbObjPosSet(objId, pos.x, pos.y, pos.z);
    mbObjDispSet(objId, FALSE);
    mbCameraMoveObj(objId, NULL, &ev_CapsuleViewOfs, 3000.0f, -1.0f, 1);
    sprId = mbev_CapSprCreate(&work->objWork,
        mbBoardDataNumGet(DATANUM(DATA_board, 1)), 2000, 0);
    espDispOn(sprId);
    espDrawNoSet(sprId, 0);
    espAttrSet(sprId, 1);
    espPosSet(sprId, 0.0f, 0.0f);
    espBankSet(sprId, 0);
    do {
        for (i = 0, linkNum = 0; i < mbMasuLinkNumGet((s16)masuId); i++) {
            linkTbl[linkNum++] = mbMasuLinkGet((s16)masuId, i);
        }
        linkNum += mbMasuLinkParentGet((s16)masuId, &linkTbl[linkNum]);
        for (i = 0; i < linkNum; i++) {
            mbMasuPosGet(linkTbl[i], &linkPos);
            PSVECSubtract(&linkPos, &pos, &delta);
            angle = 180.0f * (atan2(delta.x, delta.z) / M_PI);
            angleTbl[i] = ev_CapRotCamera(angle);
        }
        stickX = (float)mbPadStkXGet(0);
        stickY = (float)mbPadStkYGet(0);
        if (fabs((double)stickX) > 8.0 || fabs((double)stickY) > 8.0) {
            for (i = 0; i < linkNum; i++) {
                stickAngle = 180.0f * (atan2(stickX, -stickY) / M_PI);
                angle = mbev_CapAngleWrap(angleTbl[i], stickAngle);
                if (fabs((double)angle) < 30.0) {
                    masuId = linkTbl[i];
                    prevPos = pos;
                    mbMasuPosGet((s16)masuId, &pos);
                    for (j = 0; (float)j < 18.0f; j++) {
                        weight = (float)j / 18.0f;
                        PSVECSubtract(&pos, &prevPos, &delta);
                        PSVECScale(&delta, &delta, weight);
                        PSVECAdd(&prevPos, &delta, &markerPos);
                        mbObjPosSetV(objId, &markerPos);
                        Hu3D3Dto2D(&markerPos, 1, &screenPos);
                        screenPos.x -= 32.0f;
                        screenPos.y -= 16.0f;
                        espPosSet(sprId, screenPos.x, screenPos.y);
                        HuPrcVSleep();
                    }
                    mbObjPosSetV(objId, &pos);
                    break;
                }
            }
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (HuPadBtnDown[0] & (PAD_BUTTON_A << i)) {
                GwPlayer[i].masuId = masuId;
                mbPlayerPosSetV(i, &pos);
                mbev_PlayerColMasuSet(i, masuId, TRUE);
            }
        }
        Hu3D3Dto2D(&pos, 1, &screenPos);
        screenPos.x -= 32.0f;
        screenPos.y -= 16.0f;
        espPosSet(sprId, screenPos.x, screenPos.y);
        HuPrcVSleep();
    } while (!(HuPadBtn[0] & PAD_TRIGGER_L)
        || !(HuPadBtn[0] & PAD_TRIGGER_R));
    mbCameraMoveOnSet(TRUE);
    mbCameraFocusPlayerSet(playerNo);
    HuPrcEnd();
}

void mbev_CapDebugWarpKill(void)
{
}

void mbev_CapDebugPosSelect(void)
{
    CAPWORK *work;

    work = HuPrcCurrentGet()->property;
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    mbCapSelectMasuInit();
    HuPrcEnd();
}

void mbev_CapDebugPosSelectKill(void)
{
}

void mbev_CapCircuitCallKettou(void)
{
}

void mbev_CapRandomBonusCoin(int playerNo, int capsuleNo, BOOL waitF)
{
    mbev_CapBonusCoinCall(playerNo, capsuleNo, -1, waitF);
}

void mbev_CapBonusCoinCall(int playerNo, int capsuleNo, int coinNum,
    BOOL waitF)
{
    int unk = 0;
    int coinNumWork = 0;
    BOOL partyF = GwSystem.partyF;

    if (!partyF || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    if (coinNum > 0) {
        mbev_CapBonusCoin(playerNo, coinNum, waitF, TRUE);
    } else if (coinNum != 0) {
        coinNumWork = mbCapBonusCoinNumGet(playerNo, capsuleNo);
        if (coinNumWork > 0) {
            mbev_CapBonusCoin(playerNo, coinNumWork, waitF, TRUE);
        }
    }
}

void mbev_CapBonusCoin(int playerNo, int coinNum, BOOL waitF, BOOL highF)
{
    HUPROCESS *process;
    void *workData;
    CAPBONUSCOINWORK *workP;

    process = ev_CapBonusCoinProc[playerNo] = HuPrcChildCreate(ev_CapBonusCoin, 8196, 24576, 0, mbMainProc);
    HuPrcDestructorSet2(process, ev_CapBonusCoinKill);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPBONUSCOINWORK), HU_MEMNUM_OVL);
    process->property = workData;
    workP = workData;
    memset(workP, 0, sizeof(CAPBONUSCOINWORK));
    workP->playerNo = playerNo;
    workP->coinNum = coinNum;
    workP->highF = highF;
    if (waitF) {
        while (!mbev_CapBonusCoinCheck(playerNo)) {
            HuPrcVSleep();
        }
    }
}

static void ev_CapBonusCoin(void)
{
    HUPROCESS *process;
    CAPBONUSCOINWORK *workP;
    OMOBJ *obj;

    process = HuPrcCurrentGet();
    workP = process->property;
    bonusCoinNum = workP->coinNum;
    bonusCoinWinId = -1;
    obj = mbev_CapEffCoinCreate();
    ev_CapCoinAdd(obj, workP->playerNo, workP->coinNum, workP->highF, ev_CapBonusCoinWin);
    mbev_CapEffCoinKill(obj);
    if (bonusCoinWinId >= 0) {
        mbWinWait(bonusCoinWinId);
    }
    bonusCoinWinId = -1;
    HuPrcEnd();
}




static void ev_CapBonusCoinKill(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    CAPBONUSCOINWORK *workP = process->property;

    ev_CapBonusCoinProc[workP->playerNo] = NULL;
    HuMemDirectFree(workP);
}

static void ev_CapBonusCoinWin(void)
{
    bonusCoinWinId = mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX99, 0x39), -1);
    sprintf(ev_CapBonusCoinMes, lbl_802BFE90, bonusCoinNum);
    mbWinTopInsertMesSet((u32)ev_CapBonusCoinMes, 0);
}

BOOL mbev_CapBonusCoinCheck(int playerNo)
{
    if (ev_CapBonusCoinProc[playerNo] != NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void mbev_CapKettouEndCall(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 41;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    work.flags._flag02 = TRUE;
    mbPlayerColSnapSet(TRUE);
    ev_CapCall(&work, TRUE);
}

void mbev_CapDonkeyEndCall(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 44;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    work.flags._flag03 = TRUE;
    mbPlayerColSnapSet(TRUE);
    ev_CapCall(&work, TRUE);
}

void mbev_CapKoopaEndCall(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 43;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 0;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    work.flags._flag04 = TRUE;
    mbPlayerColSnapSet(TRUE);
    ev_CapCall(&work, TRUE);
}

void mbev_CapKillerMoveCall(int playerNo)
{
    CAPWORK work;

    memset(&work, 0, sizeof(CAPWORK));
    work.playerNo = playerNo;
    work.targetPlayerNo = -1;
    work.capsuleNo = 40;
    work._unk14 = 0;
    work._unk1C = 0;
    work._unk18 = 1;
    work.masuId = GwPlayer[playerNo].masuId;
    work.masuIdNext = -1;
    ev_CapCall(&work, TRUE);
}






void mbev_CapInit(void)
{
    int i;
    s16 *dataP;

    for (i = 0; i < 8; i++) {
        ev_CapMainProc[i] = NULL;
    }
    for (i = 0; i < 8; i++) {
        ev_CapEffExplodeOMObj[i] = NULL;
        ev_CapEffBoostOMObj[i] = NULL;
        ev_CapEffSnowOMObj[i] = NULL;
        ev_CapEffGlowOMObj[i] = NULL;
        ev_CapEffRingOMObj[i] = NULL;
        ev_CapEffElectricOMObj[i] = NULL;
        ev_CapEffCoinOMObj[i] = NULL;
        ev_CapEffCoinManOMObj[i] = NULL;
        ev_CapEffStarManOMObj[i] = NULL;
        ev_CapEffCapLoseOMObj[i] = NULL;
        ev_CapEffRayOMObj[i] = NULL;
        ev_CapEffMasuHitOMObj[i] = NULL;
    }
    mbCapListRead();
    capsuleHook = NULL;
    mbev_ShopEnableSet(FALSE);
    mbCapThrowColCreate(-1);
    mbCapThrowHookSet(NULL);
    mbev_CapTeresaStealSet(-1, 0, NULL, NULL);
    boostEffAnim = HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 0x35), HU_MEMNUM_OVL));
    HuSprAnimLock(boostEffAnim);
    ringHitEffAnim1 = HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 0x38), HU_MEMNUM_OVL));
    HuSprAnimLock(ringHitEffAnim1);
    ringHitEffAnim2 = HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 0x31), HU_MEMNUM_OVL));
    HuSprAnimLock(ringHitEffAnim2);
    electricEffAnim = HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 0x37), HU_MEMNUM_OVL));
    HuSprAnimLock(electricEffAnim);
    mbCapEffNum = 0;
    dataP = HuMemDirectMallocNum(HEAP_HEAP, 0x800, HU_MEMNUM_OVL);
    mbCapEffData = dataP;
    for (i = 0; i < 1024; i++) {
        mbCapEffData[i] = frand() & 0x7FFF;
    }
    capsuleMasuType = -1;
    capsuleMasuId = -1;
    capsulePlayer = -1;
    capsuleMasuPlayer = -1;
    capsuleEventMasu = -1;
    capsuleEventPlayer = -1;
    capsuleEventPrevMasu = -1;
    capsuleEventPrevPlayer = -1;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        ev_CapBonusCoinProc[i] = NULL;
    }
}

static void ev_CapKill(void)
{
    HUPROCESS *process;
    CAPWORK *workP;
    void (*hook)(void);

    process = HuPrcCurrentGet();
    workP = process->property;
    if (workP->capsuleNo != -1) {
        if (ev_CapsuleData[workP->capsuleNo].unk04 != NULL) {
            hook = ev_CapsuleData[workP->capsuleNo].unk04;
            hook();
        }
    }
    HuDataDirClose(DATA_capsuleshop);
    HuDataDirClose(DATA_capsulechar0);
    HuDataDirClose(DATA_capsulechar1);
    HuDataDirClose(DATA_capsulechar2);
    HuDataDirClose(DATA_capsulechar3);
    HuDataDirClose(DATA_capsulechar4);
    lbl_802C0FD8 = 0;
    ev_CapWorkClose(&workP->objWork);
    if (mbExitCheck() == FALSE) {
        if (workP->explodeObj != NULL) {
            mbev_CapEffExplodeKill(workP->explodeObj);
        }
        if (workP->boostObj != NULL) {
            mbev_CapEffBoostKill(workP->boostObj);
        }
        if (workP->snowObj != NULL) {
            mbev_CapEffSnowKill(workP->snowObj);
        }
        if (workP->glowObj != NULL) {
            mbev_CapEffGlowKill(workP->glowObj);
        }
        if (workP->ringObj != NULL) {
            mbev_CapEffRingKill(workP->ringObj);
        }
        if (workP->coinObj != NULL) {
            mbev_CapEffCoinKill(workP->coinObj);
        }
        if (workP->coinManObj != NULL) {
            mbev_CapCoinManKill(workP->coinManObj);
        }
        if (workP->starManObj != NULL) {
            mbev_CapStarManKill(workP->starManObj);
        }
        if (workP->capLoseObj != NULL) {
            mbev_CapEffCapLoseKill(workP->capLoseObj);
        }
    }
    ev_CapMainProc[workP->processNo] = NULL;
    _ClearFlag(FLAG_BOARD_STAR_RESET);
    HuMemDirectFree(workP);
}


void mbev_CapWait(CAPWORK *work)
{
    EVCAPWORK *objWork;

    objWork = &work->objWork;
    if (objWork != NULL) {
        if (objWork->bgId >= 0) {
            mbBGReadWait(objWork->bgId);
            objWork->bgId = -1;
        }
        mbCameraMoveWait();
    }
}

static void ev_CapWorkClose(EVCAPWORK *work)
{
    int i;
    int j;
    void *memP;

    if (work != NULL) {
        if (work->obj != NULL) {
            work->obj->work[0] = 1;
        }
        for (i = 0; i < CAP_WORK_MAX; i++) {
            if (!mbExitCheck()) {
                for (j = 0; j < GW_PLAYER_MAX; j++) {
                    if (work->motId[i][j] != -1) {
                        mbPlayerMotionKill(j, work->motId[i][j]);
                    }
                }
            }
            if (work->objId[i] != -1) {
                mbObjKill(work->objId[i]);
            }
            if (work->sprId[i] != -1) {
                espKill(work->sprId[i]);
            }
            if (work->mem[i] != NULL) {
                memP = work->mem[i];
                HuMemDirectFree(memP);
            }
        }
    }
}

void mbev_CapNull(void)
{
    void *workP = HuPrcCurrentGet()->property;

    HuPrcEnd();
}

void mbev_CapStatusDispSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF posBegin;
    HuVecF posEnd;

    if (mbStatusDispGet(leftPlayer) == FALSE) {
        posBegin.x = -130.0f;
        posBegin.y = 240.0f;
        posBegin.z = 0.0f;
    } else {
        mbStatusPosGet(leftPlayer, &posBegin);
    }
    posEnd.x = 130.0f;
    posEnd.y = 240.0f;
    posEnd.z = 0.0f;
    mbStatusMoveSet(leftPlayer, &posBegin, &posEnd, STATUS_MOVE_SIN, 30);

    if (mbStatusDispGet(rightPlayer) == FALSE) {
        posBegin.x = 706.0f;
        posBegin.y = 240.0f;
        posBegin.z = 0.0f;
    } else {
        mbStatusPosGet(rightPlayer, &posBegin);
    }
    posEnd.x = 446.0f;
    posEnd.y = 240.0f;
    posEnd.z = 0.0f;
    mbStatusMoveSet(rightPlayer, &posBegin, &posEnd, STATUS_MOVE_SIN, 30);

    if (waitF) {
        do {
            HuPrcVSleep();
        } while (!mbStatusMoveCheck(leftPlayer)
            || !mbStatusMoveCheck(rightPlayer));
    }
}


void mbev_CapDuelStatusOnSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF posOff;
    HuVecF posOn;

    mbStatusPosOffGet(0, &posOff);
    mbStatusPosOnGet(0, &posOn);
    mbStatusLayoutSet(leftPlayer, STATUS_LAYOUT_TOP);
    mbStatusMoveSet(leftPlayer, &posOff, &posOn, STATUS_MOVE_SIN, 30);

    mbStatusPosOffGet(1, &posOff);
    mbStatusPosOnGet(1, &posOn);
    mbStatusLayoutSet(rightPlayer, STATUS_LAYOUT_TOP);
    mbStatusMoveSet(rightPlayer, &posOff, &posOn, STATUS_MOVE_SIN, 30);

    if (waitF) {
        do {
            HuPrcVSleep();
        } while (!mbStatusMoveCheck(leftPlayer) || !mbStatusMoveCheck(rightPlayer));
    }
}

void mbev_CapDuelStatusDispSet(int leftPlayer, int rightPlayer, BOOL waitF)
{
    HuVecF posOff;
    HuVecF posOn;

    mbStatusPosOffGet(0, &posOff);
    mbStatusPosOnGet(0, &posOn);
    mbStatusMoveSet(leftPlayer, &posOn, &posOff, STATUS_MOVE_REVCOS, 30);

    mbStatusPosOffGet(1, &posOff);
    mbStatusPosOnGet(1, &posOn);
    mbStatusMoveSet(rightPlayer, &posOn, &posOff, STATUS_MOVE_REVCOS, 30);

    if (waitF) {
        do {
            HuPrcVSleep();
        } while (!mbStatusMoveCheck(leftPlayer) || !mbStatusMoveCheck(rightPlayer));
        mbStatusDispForceSet(leftPlayer, FALSE);
        mbStatusDispForceSet(rightPlayer, FALSE);
    }
}

void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF)
{
    int i;
    int playerNum;
    int dispNum;
    BOOL allDispF;
    BOOL allNoDispF;

    for (i = 0, playerNum = 0; i < GW_PLAYER_MAX; i++) {
        playerNum++;
    }

    for (i = 0, dispNum = 0; i < GW_PLAYER_MAX; i++) {
        if (mbStatusDispGet(i) != FALSE) {
            dispNum++;
        }
    }
    if (dispNum == playerNum) {
        allDispF = TRUE;
    } else {
        allDispF = FALSE;
    }

    for (i = 0, dispNum = 0; i < GW_PLAYER_MAX; i++) {
        if (mbStatusDispGet(i) == FALSE) {
            dispNum++;
        }
    }
    if (dispNum == playerNum) {
        allNoDispF = TRUE;
    } else {
        allNoDispF = FALSE;
    }

    if (dispF) {
        if (allDispF) {
            return;
        }
        if (allDispF == FALSE && allNoDispF == FALSE) {
            mbev_CapStatusDispSetAll(FALSE, TRUE);
        }
    } else if (allNoDispF) {
        return;
    }
    mbStatusDispSetAll(dispF);

    if (waitF) {
        do {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (mbStatusMoveCheck(i) == FALSE) {
                    break;
                }
            }
            HuPrcVSleep();
        } while (i < GW_PLAYER_MAX);
    }
}

BOOL mbev_CapStatusDispCheck(int playerNo)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i == playerNo) {
            if (mbStatusDispGet(i) == FALSE) {
                break;
            }
        } else if (mbStatusDispGet(i) != FALSE) {
            break;
        }
    }
    if (i >= GW_PLAYER_MAX) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void mbev_CapPlayerMoveObjInit(void)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        ev_CapEffMoveOMObj[i] = NULL;
    }
}

void mbev_CapPlayerMoveObjClose(int playerNo)
{
    ev_CapEffMoveOMObj[playerNo] = NULL;
}

void mbev_CapPlayerMoveObjKill(void)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        ev_CapEffMoveOMObj[i] = NULL;
    }
}

void mbev_CapPlayerMoveHitCreate(int playerNo, BOOL useMotF, BOOL useShiftF)
{
    OMOBJ *obj;
    CAPEFFMOVEWORK *workP;

    obj = ev_CapEffMoveOMObj[playerNo] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapPlayerMoveObjExec);
    workP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFMOVEWORK), HU_MEMNUM_OVL);
    obj->data = workP;
    memset(workP, 0, sizeof(CAPEFFMOVEWORK));
    if (useMotF) {
        workP->motNo = 9;
    } else {
        workP->motNo = -1;
    }
    if (useShiftF) {
        workP->nextMotNo = 6;
    } else {
        workP->nextMotNo = -1;
    }
    workP->playerNo = playerNo;
    workP->state = 0;
    workP->useMotF = useMotF;
    workP->useShiftF = useShiftF;
    workP->minYF = TRUE;
    mbPlayerPosGet(playerNo, &workP->pos);
    workP->minY = workP->pos.y;
    workP->vel = 6.5333333f;
    workP->velocity.x = workP->velocity.z = 0.0f;
    workP->velocity.y = 60.000004f;
    if (workP->useMotF && workP->motNo != -1) {
        workP->state = 0;
        mbPlayerMotionSet(playerNo, workP->motNo, 0);
    } else {
        workP->state = 1;
        if (workP->useShiftF && workP->nextMotNo != -1) {
            mbPlayerMotionShiftSet(playerNo, workP->nextMotNo, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
    }
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
}

void mbev_CapPlayerMoveEjectCreate(int playerNo, BOOL useShiftF)
{
    OMOBJ *obj;
    CAPEFFMOVEWORK *workP;

    obj = ev_CapEffMoveOMObj[playerNo] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapPlayerMoveObjExec);
    workP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFMOVEWORK), HU_MEMNUM_OVL);
    obj->data = workP;
    memset(workP, 0, sizeof(CAPEFFMOVEWORK));
    workP->motNo = -1;
    if (useShiftF) {
        workP->nextMotNo = 6;
    } else {
        workP->nextMotNo = -1;
    }
    workP->playerNo = playerNo;
    workP->state = 0;
    workP->useMotF = TRUE;
    workP->useShiftF = useShiftF;
    workP->minYF = TRUE;
    mbPlayerPosGet(playerNo, &workP->pos);
    workP->minY = workP->pos.y;
    workP->vel = 6.5333333f;
    workP->velocity.x = workP->velocity.z = 0.0f;
    workP->velocity.y = 80.0f;
    workP->state = 0;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
}

void mbev_CapPlayerMoveIdleCreate(int playerNo, int moveTime)
{
    OMOBJ *obj;
    CAPEFFMOVEWORK *workP;
    HuVecF masuPos;
    HuVecF delta;

    obj = ev_CapEffMoveOMObj[playerNo] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapPlayerMoveObjExec);
    workP = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFMOVEWORK), HU_MEMNUM_OVL);
    obj->data = workP;
    memset(workP, 0, sizeof(CAPEFFMOVEWORK));
    workP->motNo = 9;
    workP->nextMotNo = 6;
    workP->playerNo = playerNo;
    workP->state = 1;
    workP->useMotF = TRUE;
    workP->useShiftF = TRUE;
    workP->minYF = TRUE;
    mbPlayerPosGet(playerNo, &workP->pos);
    workP->minY = workP->pos.y;
    workP->vel = 6.5333333f;
    workP->velocity.x = workP->velocity.z = 0.0f;
    workP->velocity.y = 80.0f;
    workP->state = 1;
    mbMasuPosGet(GwPlayer[playerNo].masuId, &masuPos);
    mbPlayerPosGet(playerNo, &workP->posStart);
    workP->posEnd = workP->posStart;
    workP->posEnd.y += 1000.0f;
    PSVECSubtract(&workP->posStart, &masuPos, &delta);
    PSVECScale(&delta, &delta, 4.0f);
    PSVECAdd(&workP->posEnd, &delta, &workP->moveDir);
    workP->moveTime = moveTime;
    workP->time = 0;
    workP->rotSpeed = 2.0f * (-0.5f
        + MBCapsuleEffRandF());
    mbPlayerRotGet(playerNo, &workP->rot);
    mbPlayerMotionSet(playerNo, workP->motNo, 0);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
}

void mbev_CapPlayerMoveObjExec(OMOBJ *obj)
{
    CAPEFFMOVEWORK *workP = obj->data;
    HuVecF pos;
    float minY;
    float weight;

    switch (workP->state) {
        case 0:
            PSVECAdd(&workP->pos, &workP->velocity, &workP->pos);
            workP->velocity.y -= workP->vel;
            if (workP->minYF) {
                mbMasuPosGet(GwPlayer[workP->playerNo].masuId, &pos);
                minY = pos.y;
            } else {
                minY = workP->minY;
            }
            if (workP->pos.y <= minY) {
                workP->pos.y = minY;
                workP->state = 4;
                if (workP->useShiftF && workP->nextMotNo != -1) {
                    mbPlayerMotionShiftSet(workP->playerNo, workP->nextMotNo,
                        0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                } else {
                    mbPlayerMotionShiftSet(workP->playerNo, 1, 0.0f, 8.0f,
                        HU3D_MOTATTR_LOOP);
                }
                mbPlayerColSnapPlayerSet(workP->playerNo, TRUE);
            }
            mbPlayerPosSetV(workP->playerNo, &workP->pos);
            break;

        case 1:
            workP->time++;
            weight = (float)workP->time / 60.0f;
            mbev_CapBezierGetV(weight, (float *)&workP->posStart,
                (float *)&workP->posEnd, (float *)&workP->moveDir,
                (float *)&pos);
            mbPlayerPosSetV(workP->playerNo, &pos);
            mbPlayerRotSet(workP->playerNo, workP->rot.x,
                workP->rot.y + ((720.0f * weight) * workP->rotSpeed),
                workP->rot.z);
            if (weight >= 1.0f) {
                mbPlayerDispSet(workP->playerNo, FALSE);
                workP->state = 2;
                workP->time = 0;
            }
            break;

        case 2:
            workP->time++;
            if (workP->time >= workP->moveTime) {
                workP->pos = workP->posEnd;
                workP->pos.y += 1000.0f;
                mbPlayerPosSetV(workP->playerNo, &workP->pos);
                mbPlayerRotSetV(workP->playerNo, &workP->rot);
                mbPlayerDispSet(workP->playerNo, TRUE);
                workP->minY = workP->posStart.y;
                workP->state = 0;
                workP->time = 0;
            }
            break;
    }

    if (mbExitCheck() || workP->state == 4
        || ev_CapEffMoveOMObj[workP->playerNo] == NULL) {
        int playerNo = workP->playerNo;

        HuMemDirectFree(workP);
        obj->data = NULL;
        omDelObjEx(mbObjMan, obj);
        ev_CapEffMoveOMObj[playerNo] = NULL;
    }
}

void mbev_CapCoinAdd(
    OMOBJ *obj, int playerNo, int coinNum, BOOL highF)
{
    ev_CapCoinAdd(obj, playerNo, coinNum, highF, NULL);
}

void mbev_CapPlayerSquishSet(int *playerNo, int masuId)
{
    mbev_CapPlayerSquishVoiceSet(playerNo, masuId, FALSE);
}

int mbev_CapPlayerSquishVoiceSet(int *out, int masuId, BOOL voiceF)
{
    int playerNoWork[GW_PLAYER_MAX];
    int i;
    int playerNum = 0;

    if (out == NULL) {
        out = playerNoWork;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (masuId == GwPlayer[i].masuId) {
            out[playerNum++] = i;
        }
    }
    for (i = 0; i < playerNum; i++) {
        if (voiceF) {
            CharMotionVoiceOnSet(GwPlayer[out[i]].charNo, 46, FALSE);
        }
        mbPlayerMotionSet(out[i], 10, 0);
        mbPlayerMotionTimeSet(out[i], lbl_802C47C8);
        mbPlayerMotionSpeedSet(out[i], lbl_802C4688);
    }
    return playerNum;
}



BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2)
{
    BOOL team1;
    BOOL team2;

    if ((int)GwSystem.tagF == FALSE) {
        if (playerNo1 == playerNo2) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    team1 = GwPlayer[playerNo2].team;
    team2 = GwPlayer[playerNo1].team;
    if (team2 == team1) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL mbev_CapCullPlayerCheck(int playerNo)
{
    return mbev_CapCullCheck(playerNo, 0);
}

BOOL mbev_CapCullCheck(int playerNo, int masuId)
{
    static float charSize[][2] = {
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
    static HuVecF pointEdge[] = {
        { 0.0f, 0.0f, 0.0f },
        { -1.0f, 2.0f, 0.0f },
        { 1.0f, 2.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f },
        { 1.0f, -1.0f, 0.0f },
    };
    HuVecF pos;
    HuVecF edge;
    int charNo;
    int i;

    if (masuId <= 0) {
        mbPlayerPosGet(playerNo, &pos);
    } else {
        mbMasuPosGet((s16)masuId, &pos);
    }
    charNo = GwPlayer[playerNo].charNo;
    for (i = 0; i < 5; i++) {
        edge.x = pointEdge[i].x * charSize[charNo][0];
        edge.y = pointEdge[i].y * charSize[charNo][1];
        edge.z = pointEdge[i].z * charSize[charNo][0];
        PSVECAdd(&edge, &pos, &edge);
        if (mbev_CapPointCullCheck(&edge)) {
            return TRUE;
        }
    }
    return FALSE;
}







s16 mbev_CapMasuPrevGet(s16 masuId, HuVecF *pos)
{
    s16 linkTbl[MASU_LINK_MAX * 2];
    s16 prevMasu;
    s16 i;
    s16 linkNum;

    if (masuId <= 0) {
        return -1;
    }
    linkNum = mbMasuLinkParentGet(masuId, linkTbl);
    for (i = 0; i < linkNum; i++) {
        prevMasu = linkTbl[i];
        break;
    }
    if (i >= linkNum) {
        return -1;
    }
    if (pos != NULL) {
        mbMasuPosGet(prevMasu, pos);
    }
    return prevMasu;
}





BOOL mbev_CapPlayerMotShiftCheck(int playerNo)
{
    int objId;
    int modelId;

    objId = mbPlayerObjIDGet(playerNo);
    modelId = mbObjModelIDGet(objId);
    if (Hu3DMotionShiftIDGet(modelId) == -1) {
        return TRUE;
    }
    return FALSE;
}




void mbev_CapPlayerIdleWait(void)
{
    int i;
    int modelId;
    int objId;
    BOOL shiftF;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotIdleSet(i);
    }
    while (TRUE) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            objId = mbPlayerObjIDGet(i);
            modelId = mbObjModelIDGet(objId);
            if (Hu3DMotionShiftIDGet(modelId) == -1) {
                shiftF = TRUE;
            } else {
                shiftF = FALSE;
            }
            if (shiftF == FALSE) {
                break;
            }
        }
        HuPrcVSleep();
        if (i >= GW_PLAYER_MAX) {
            break;
        }
    }
}


void mbev_CapVibrate(int type)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        switch (type) {
            case 0:
                omVibrate(i, 20, 4, 4);
                break;

            case 1:
                omVibrate(i, 20, 7, 3);
                break;

            case 2:
                omVibrate(i, 20, 20, 0);
                break;
        }
    }
}

int mbev_CapPlayerMasuNumGet(int masuId)
{
    int i;
    int count;

    i = 0;
    count = 0;
    for (; i < GW_PLAYER_MAX; i++) {
        if (masuId == GwPlayer[i].masuId) {
            count++;
        }
    }
    return count;
}

int mbev_CapPlayerNoSearch(int playerNo)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != playerNo) {
            return i;
        }
    }
    return -1;
}

int mbev_CapPlayerOrderGet(
    int *order, int excludePlayer, int priorityPlayer, BOOL orderF)
{
    int i;
    int count;
    int j;
    int temp;

    i = 0;
    count = 0;
    for (; i < GW_PLAYER_MAX; i++) {
        if (i != excludePlayer) {
            order[count] = i;
            count++;
        }
    }
    if (orderF != FALSE) {
        for (i = 0; i < count - 1; i++) {
            for (j = i + 1; j < count; j++) {
                if (GwPlayer[order[i]].rank > GwPlayer[order[j]].rank) {
                    temp = order[i];
                    order[i] = order[j];
                    order[j] = temp;
                }
            }
        }
        for (i = 0; i < count; i++) {
            if (order[i] == priorityPlayer && i != 0) {
                temp = order[i];
                order[i] = order[0];
                order[0] = temp;
            }
        }
    }
    return count;
}

s16 mbev_CapCoinDisp(int playerNo, int coinNum, BOOL winMotF, BOOL waitF);

int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, float scale,
    float gravity, int time, int arg);

int mbev_CapEffCoinNumGet(OMOBJ *obj)
{
    int i;
    CAPEFFCOINWORK *workP;
    int objIdx;
    int count;

    count = 0;
    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffCoinOMObj[objIdx] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFCOINWORK);
    for (i = 0; i < 128; i++, workP++) {
        if (workP->activeF) {
            count++;
        }
    }
    return count;
}

void mbev_CapEffExplodeOMExec(OMOBJ *obj)
{
    CAPEFFEXPLODEWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFEXPLODEPARTICLEWORK *particleWorkP;

    workP = obj->data;
    if (mbExitCheck() || ev_CapEffExplodeOMObj[workP->objIdx] == (OMOBJ *)-1) {
        Hu3DModelKill(workP->modelId);
        workP->modelId = -1;
        HuSprAnimKill(workP->animP);
        workP->animP = NULL;
        ev_CapEffExplodeOMObj[workP->objIdx] = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if (workP->num <= 0) {
        Hu3DModelAttrSet(workP->modelId, 1);
        return;
    }
    Hu3DModelAttrReset(workP->modelId, 1);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    particleP->_unk21[2] = 0;
    for (i = 0; i < particleP->num; i++, particleWorkP++) {
        if (particleWorkP->active <= 0.0f) {
            continue;
        }
        particleWorkP->pos.x += particleWorkP->vel.x;
        particleWorkP->pos.y += particleWorkP->vel.y;
        particleWorkP->pos.z += particleWorkP->vel.z;
        particleWorkP->angle += particleWorkP->angleStep;
        if (particleWorkP->angle >= 360.0f) {
            particleWorkP->angle -= 360.0f;
        }
        particleWorkP->fadeTime += particleWorkP->fadeStep;
        particleWorkP->color.a = 255.0f
            * (1.0f - (particleWorkP->fadeTime * 0.0625f));
        if (particleWorkP->fadeTime >= 16.0f) {
            particleWorkP->pat = 0;
            particleWorkP->mode = 0;
            particleWorkP->active = 0.0f;
            workP->num--;
        }
    }
}

int mbev_CapEffExplodeAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, float active,
    float angleStep, float fadeStep, GXColor *color)
{
    CAPEFFEXPLODEWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFEXPLODEPARTICLEWORK *particleWorkP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffExplodeOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    i = 0;
    while (i < particleP->num) {
        if (particleWorkP->active <= 0.0f) {
            break;
        }
        i++;
        particleWorkP++;
    }
    if (i >= particleP->num) {
        return -1;
    }
    particleWorkP->mode = particleWorkP->_unk02 = 0;
    particleWorkP->pos.x = pos->x;
    particleWorkP->pos.y = pos->y;
    particleWorkP->pos.z = pos->z;
    particleWorkP->vel.x = vel->x;
    particleWorkP->vel.y = vel->y;
    particleWorkP->vel.z = vel->z;
    particleWorkP->angleStep = angleStep;
    particleWorkP->active = active;
    particleWorkP->color.r = color->r;
    particleWorkP->color.g = color->g;
    particleWorkP->color.b = color->b;
    particleWorkP->color.a = color->a;
    particleWorkP->color.r = particleWorkP->color.g =
        particleWorkP->color.b = 255;
    particleWorkP->angle = 0.0f;
    particleWorkP->pat = 0;
    particleWorkP->mode = 0;
    particleWorkP->fadeTime = 0.0f;
    particleWorkP->fadeStep = fadeStep;
    workP->num++;
    return i;
}

void mbev_CapEffExplodeCircleAdd(OMOBJ *obj, HuVecF *posP, float radius,
    float scale, float angle)
{
    HuVecF posBase;
    HuVecF velBase;
    HuVecF pos;
    HuVecF vel;
    HuVecF dir;
    HuVecF posTemp;
    HuVecF pos1;
    HuVecF pos2;
    HuVecF vel1;
    HuVecF vel2;
    GXColor colorTemp;
    GXColor color;
    GXColor color1;
    GXColor color2;
    float value;
    float fadeStep;
    float distance;
    float active;
    float halfDistance;
    float randF;

    posBase.x = posP->x + (radius * (scale * mbCosDeg(angle)));
    posBase.y = posP->y + 50.0f;
    posBase.z = posP->z + (radius * (scale * mbSinDeg(angle)));
    value = 100.0f * (0.005f + (0.04f *
        MBCapsuleEffRandF()));
    velBase.x = value * mbCosDeg(angle);
    velBase.y = value * mbSinDeg(angle);
    velBase.z = 0.0f;
    randF = MBCapsuleEffRandF();
    colorTemp.r = 64.0f + (32.0f * randF);
    colorTemp.g = 64.0f + (32.0f * randF);
    colorTemp.b = 64.0f + (32.0f * randF);
    colorTemp.a = 64.0f + (63.0f *
        MBCapsuleEffRandF());
    color = colorTemp;
    fadeStep = 0.33f + (0.2f *
        MBCapsuleEffRandF());
    distance = 100.0f * (0.5f + (0.5f *
        MBCapsuleEffRandF()));
    active = 100.0f * (1.0f + (0.5f *
        MBCapsuleEffRandF()));
    pos = posBase;
    vel = velBase;
    dir.x = vel.z;
    dir.y = 0.0f;
    dir.z = vel.x;
    if (PSVECMag(&dir) > 0.0f) {
        PSVECNormalize(&dir, &dir);
    }
    halfDistance = 0.5f * distance;
    posTemp.x = pos.x + (dir.x * halfDistance);
    posTemp.y = pos.y + (dir.y * halfDistance);
    posTemp.z = pos.z + (dir.z * halfDistance);
    color1 = color;
    vel1 = vel;
    pos1 = posTemp;
    mbev_CapEffExplodeAdd(obj, &pos1, &vel1, active, 2.5f, fadeStep,
        &color1);
    posTemp.x = pos.x - (dir.x * halfDistance);
    posTemp.y = pos.y - (dir.y * halfDistance);
    posTemp.z = pos.z - (dir.z * halfDistance);
    color2 = color;
    vel2 = vel;
    pos2 = posTemp;
    mbev_CapEffExplodeAdd(obj, &pos2, &vel2, active, -2.5f, fadeStep,
        &color2);
}

void mbev_CapEffDustCloudAdd(OMOBJ *obj, HuVecF *posP)
{
    HuVecF posBase;
    HuVecF velBase;
    HuVecF pos;
    HuVecF vel;
    HuVecF dir;
    HuVecF posTemp;
    HuVecF pos1;
    HuVecF pos2;
    HuVecF vel1;
    HuVecF vel2;
    GXColor colorTemp;
    GXColor color;
    GXColor color1;
    GXColor color2;
    float angle;
    float radius;
    float value;
    float fadeStep;
    float distance;
    float angleStep;
    float active;
    float halfDistance;
    float randF;
    int i;

    for (i = 0; i < 32; i++) {
        angle = 11.25f * (float)i;
        radius = 0.75f * (100.0f * (0.5f *
            MBCapsuleEffRandF()));
        posBase.x = posP->x + (radius * mbCosDeg(angle));
        posBase.y = 100.0f + (posP->y + (radius * mbSinDeg(angle)));
        posBase.z = 50.0f + posP->z;
        value = 100.0f * (0.005f + (0.04f *
            MBCapsuleEffRandF()));
        velBase.x = value * mbCosDeg(angle);
        velBase.y = value * mbSinDeg(angle);
        velBase.z = 0.0f;
        randF = MBCapsuleEffRandF();
        colorTemp.r = 32.0f + (32.0f * randF);
        colorTemp.g = 32.0f + (32.0f * randF);
        colorTemp.b = 32.0f + (32.0f * randF);
        colorTemp.a = 128.0f + (63.0f *
            MBCapsuleEffRandF());
        color = colorTemp;
        fadeStep = 0.33f + (0.66f *
            MBCapsuleEffRandF());
        distance = 100.0f * (0.5f + (0.5f *
            MBCapsuleEffRandF()));
        angleStep = -0.5f
            + MBCapsuleEffRandF();
        active = 100.0f * (1.5f + (0.5f *
            MBCapsuleEffRandF()));
        pos = posBase;
        vel = velBase;
        dir.x = vel.z;
        dir.y = 0.0f;
        dir.z = vel.x;
        if (PSVECMag(&dir) > 0.0f) {
            PSVECNormalize(&dir, &dir);
        }
        halfDistance = 0.5f * distance;
        posTemp.x = pos.x + (dir.x * halfDistance);
        posTemp.y = pos.y + (dir.y * halfDistance);
        posTemp.z = pos.z + (dir.z * halfDistance);
        color1 = color;
        vel1 = vel;
        pos1 = posTemp;
        mbev_CapEffExplodeAdd(obj, &pos1, &vel1, active, angleStep, fadeStep,
            &color1);
        posTemp.x = pos.x - (dir.x * halfDistance);
        posTemp.y = pos.y - (dir.y * halfDistance);
        posTemp.z = pos.z - (dir.z * halfDistance);
        color2 = color;
        vel2 = vel;
        pos2 = posTemp;
        mbev_CapEffExplodeAdd(obj, &pos2, &vel2, active, -angleStep, fadeStep,
            &color2);
    }

    for (i = 0; i < 32; i++) {
        angle = 360.0f
            * MBCapsuleEffRandF();
        radius = 0.33f * (100.0f * (0.5f *
            MBCapsuleEffRandF()));
        posBase.x = posP->x + (1.25f * (100.0f * (-0.5f
            + MBCapsuleEffRandF())));
        posBase.y = posP->y + (1.25f * (100.0f * (-0.5f
            + MBCapsuleEffRandF())));
        posBase.z = posP->z + (50.0f
            * MBCapsuleEffRandF());
        value = 100.0f * (0.005f + (0.04f *
            MBCapsuleEffRandF()));
        velBase.x = value * mbCosDeg(angle);
        velBase.y = value * mbSinDeg(angle);
        velBase.z = 0.0f;
        randF = MBCapsuleEffRandF();
        colorTemp.r = 192.0f + (32.0f * randF);
        colorTemp.g = 192.0f + (32.0f * randF);
        colorTemp.b = 192.0f + (32.0f * randF);
        colorTemp.a = 128.0f + (63.0f *
            MBCapsuleEffRandF());
        color = colorTemp;
        fadeStep = 0.33f + (0.66f *
            MBCapsuleEffRandF());
        distance = 100.0f * (0.5f + (0.5f *
            MBCapsuleEffRandF()));
        angleStep = -0.5f
            + MBCapsuleEffRandF();
        active = 100.0f * (1.5f + (0.5f *
            MBCapsuleEffRandF()));
        pos = posBase;
        vel = velBase;
        dir.x = vel.z;
        dir.y = 0.0f;
        dir.z = vel.x;
        if (PSVECMag(&dir) > 0.0f) {
            PSVECNormalize(&dir, &dir);
        }
        halfDistance = 0.5f * distance;
        posTemp.x = pos.x + (dir.x * halfDistance);
        posTemp.y = pos.y + (dir.y * halfDistance);
        posTemp.z = pos.z + (dir.z * halfDistance);
        color1 = color;
        vel1 = vel;
        pos1 = posTemp;
        mbev_CapEffExplodeAdd(obj, &pos1, &vel1, active, angleStep, fadeStep,
            &color1);
        posTemp.x = pos.x - (dir.x * halfDistance);
        posTemp.y = pos.y - (dir.y * halfDistance);
        posTemp.z = pos.z - (dir.z * halfDistance);
        color2 = color;
        vel2 = vel;
        pos2 = posTemp;
        mbev_CapEffExplodeAdd(obj, &pos2, &vel2, active, -angleStep, fadeStep,
            &color2);
    }
}

void mbev_CapEffDustExplodeAdd(OMOBJ *obj, HuVecF *posP)
{
    HuVecF posBase;
    HuVecF velBase;
    HuVecF pos;
    HuVecF vel;
    HuVecF vel2;
    HuVecF pos2;
    HuVecF vel1;
    HuVecF pos1;
    HuVecF dir;
    HuVecF posTemp;
    GXColor colorTemp;
    GXColor color;
    GXColor color2;
    GXColor color1;
    int result2;
    int result;
    int i;
    GXColor *color1P;
    HuVecF *vel1P;
    HuVecF *pos1P;
    GXColor *color2P;
    HuVecF *vel2P;
    HuVecF *pos2P;
    float distance;
    float posCos;
    float posSin;
    float velCos;
    float velSin;
    float angle;
    float value;
    float active;
    float angleStep;
    float fadeStep;
    float halfDistance;

    for (i = 0; i < 32; i++) {
        angle = 11.25f * (float)i;
        value = 0.33f * (100.0f *
            MBCapsuleEffRandF());
        posCos = mbCosDeg(angle);
        posBase.x = posP->x + (value * posCos);
        posSin = mbSinDeg(angle);
        posBase.y = 100.0f + (posP->y + (value * posSin));
        posBase.z = 50.0f + posP->z;
        value = 100.0f * (0.005f + (0.04f *
            MBCapsuleEffRandF()));
        velCos = mbCosDeg(angle);
        velBase.x = value * velCos;
        velSin = mbSinDeg(angle);
        velBase.y = value * velSin;
        velBase.z = 0.0f;
        value = MBCapsuleEffRandF();
        colorTemp.r = 192.0f + (32.0f * value);
        colorTemp.g = 192.0f + (32.0f * value);
        colorTemp.b = 192.0f + (32.0f * value);
        colorTemp.a = 192.0f + (63.0f *
            MBCapsuleEffRandF());
        color.r = colorTemp.r;
        color.g = colorTemp.g;
        color.b = colorTemp.b;
        color.a = colorTemp.a;
        fadeStep = 0.33f + (0.66f *
            MBCapsuleEffRandF());
        distance = 100.0f * (0.5f + (0.5f *
            MBCapsuleEffRandF()));
        angleStep = -0.5f +
            MBCapsuleEffRandF();
        active = 100.0f * (1.0f + (0.5f *
            MBCapsuleEffRandF()));
        vel = velBase;
        pos = posBase;
        dir.x = vel.z;
        dir.z = vel.x;
        dir.y = 0.0f;
        if (PSVECMag(&dir) > 0.0f) {
            PSVECNormalize(&dir, &dir);
        }
        halfDistance = 0.5f * distance;
        posTemp.x = pos.x + (dir.x * halfDistance);
        posTemp.y = pos.y + (dir.y * halfDistance);
        posTemp.z = pos.z + (dir.z * halfDistance);
        color1.r = color.r;
        color1.g = color.g;
        color1.b = color.b;
        color1.a = color.a;
        color1P = &color1;
        vel1 = vel;
        vel1P = &vel1;
        pos1 = posTemp;
        pos1P = &pos1;
        result = mbev_CapEffExplodeAdd(obj, pos1P, vel1P, active, angleStep, fadeStep,
            color1P);
        posTemp.x = pos.x - (dir.x * halfDistance);
        posTemp.y = pos.y - (dir.y * halfDistance);
        posTemp.z = pos.z - (dir.z * halfDistance);
        color2.r = color.r;
        color2.g = color.g;
        color2.b = color.b;
        color2.a = color.a;
        color2P = &color2;
        vel2 = vel;
        vel2P = &vel2;
        pos2 = posTemp;
        pos2P = &pos2;
        result2 = mbev_CapEffExplodeAdd(obj, pos2P, vel2P, active, -angleStep, fadeStep,
            color2P);
    }
}

void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF *posP)
{
    HuVecF posBase;
    HuVecF velBase;
    HuVecF pos;
    HuVecF vel;
    HuVecF vel2;
    HuVecF pos2;
    HuVecF vel1;
    HuVecF pos1;
    HuVecF dir;
    HuVecF posTemp;
    GXColor colorTemp;
    GXColor color;
    GXColor color2;
    GXColor color1;
    int result2;
    int result;
    int i;
    GXColor *color1P;
    HuVecF *vel1P;
    HuVecF *pos1P;
    GXColor *color2P;
    HuVecF *vel2P;
    HuVecF *pos2P;
    float distance;
    float posCos;
    float posSin;
    float velCos;
    float velSin;
    float angle;
    float value;
    float active;
    float angleStep;
    float fadeStep;
    float halfDistance;

    for (i = 0; i < 32; i++) {
        angle = 22.5f * (float)i;
        angle += 20.0f * (-0.5f +
            MBCapsuleEffRandF());
        value = 0.33f * (100.0f *
            MBCapsuleEffRandF());
        posCos = mbCosDeg(angle);
        posBase.x = posP->x + (value * posCos);
        posBase.y = 50.0f + posP->y;
        posSin = mbSinDeg(angle);
        posBase.z = posP->z + (value * posSin);
        value = 100.0f * (0.03f + (0.06f *
            MBCapsuleEffRandF()));
        velCos = mbCosDeg(angle);
        velBase.x = value * velCos;
        velBase.y = 0.01f * (100.0f *
            MBCapsuleEffRandF());
        velSin = mbSinDeg(angle);
        velBase.z = value * velSin;
        value = MBCapsuleEffRandF();
        colorTemp.r = 192.0f + (63.0f * value);
        colorTemp.g = 192.0f + (63.0f * value);
        colorTemp.b = 192.0f + (63.0f * value);
        colorTemp.a = 64.0f + (63.0f *
            MBCapsuleEffRandF());
        color.r = colorTemp.r;
        color.g = colorTemp.g;
        color.b = colorTemp.b;
        color.a = colorTemp.a;
        fadeStep = 0.33f + (0.66f *
            MBCapsuleEffRandF());
        distance = 100.0f * (0.5f + (0.5f *
            MBCapsuleEffRandF()));
        angleStep = -0.5f +
            MBCapsuleEffRandF();
        active = 100.0f * (1.0f + (0.5f *
            MBCapsuleEffRandF()));
        vel = velBase;
        pos = posBase;
        dir.x = vel.z;
        dir.z = vel.x;
        dir.y = 0.0f;
        if (PSVECMag(&dir) > 0.0f) {
            PSVECNormalize(&dir, &dir);
        }
        halfDistance = 0.5f * distance;
        posTemp.x = pos.x + (dir.x * halfDistance);
        posTemp.y = pos.y + (dir.y * halfDistance);
        posTemp.z = pos.z + (dir.z * halfDistance);
        color1.r = color.r;
        color1.g = color.g;
        color1.b = color.b;
        color1.a = color.a;
        color1P = &color1;
        vel1 = vel;
        vel1P = &vel1;
        pos1 = posTemp;
        pos1P = &pos1;
        result = mbev_CapEffExplodeAdd(obj, pos1P, vel1P, active, angleStep, fadeStep,
            color1P);
        posTemp.x = pos.x - (dir.x * halfDistance);
        posTemp.y = pos.y - (dir.y * halfDistance);
        posTemp.z = pos.z - (dir.z * halfDistance);
        color2.r = color.r;
        color2.g = color.g;
        color2.b = color.b;
        color2.a = color.a;
        color2P = &color2;
        vel2 = vel;
        vel2P = &vel2;
        pos2 = posTemp;
        pos2P = &pos2;
        result2 = mbev_CapEffExplodeAdd(obj, pos2P, vel2P, active, -angleStep, fadeStep,
            color2P);
    }
    mbCameraShakeSet(30, 50.0f);
}

void mbev_CapEffDustMultiAdd(OMOBJ *obj, HuVecF *posP, HuVecF *rotP, int num)
{
    Mtx mtx;
    HuVecF pos;
    HuVecF vel;
    GXColor colorTemp;
    GXColor color;
    int i;
    int angle;
    int colorBase;
    float sinAngle;
    float cosAngle;

    mtxRot(mtx, rotP->x, rotP->y, rotP->z);
    for (i = 0; i < num; i++) {
        pos = *posP;
        angle = (int)((360.0f / (float)num) * (float)i);
        sinAngle = mbSinDeg((float)angle);
        vel.x = 0.075f * (100.0f * sinAngle)
            * (0.9f + (0.1f *
                MBCapsuleEffRandF()));
        vel.y = 0.0f;
        cosAngle = mbCosDeg((float)angle);
        vel.z = 0.075f * (100.0f * cosAngle)
            * (0.9f + (0.1f *
                MBCapsuleEffRandF()));
        PSMTXMultVec(mtx, &vel, &vel);
        colorBase = 63.0f
            * MBCapsuleEffRandF();
        colorTemp.r = colorTemp.g = colorTemp.b = colorBase + 192;
        colorTemp.a = 192.0f + (63.0f *
            MBCapsuleEffRandF());
        color = colorTemp;
        mbev_CapEffExplodeAdd(obj, &pos, &vel, 200.0f,
            0.5f * (-0.5f +
                MBCapsuleEffRandF()),
            0.33f, &color);
    }
}

int mbev_CapEffBoostAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, float active,
    float angleStep, int time, GXColor *color)
{
    CAPEFFBOOSTWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFBOOSTPARTICLEWORK *particleWorkP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffBoostOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    i = 0;
    while (i < particleP->num) {
        if (particleWorkP->active <= 0.0) {
            break;
        }
        i++;
        particleWorkP++;
    }
    if (i >= particleP->num) {
        return -1;
    }
    particleWorkP->time = particleWorkP->timeTotal = time;
    particleWorkP->vel.x = vel->x;
    particleWorkP->vel.y = vel->y;
    particleWorkP->vel.z = vel->z;
    particleWorkP->alpha = color->a;
    particleWorkP->angleStep = angleStep;
    particleWorkP->active = active;
    particleWorkP->angle = 360.0f *
        MBCapsuleEffRandF();
    particleWorkP->pos.x = pos->x;
    particleWorkP->pos.y = pos->y;
    particleWorkP->pos.z = pos->z;
    particleWorkP->color.r = color->r;
    particleWorkP->color.g = color->g;
    particleWorkP->color.b = color->b;
    particleWorkP->color.a = color->a;
    particleWorkP->pat = 0;
    workP->time++;
    return i;
}

int mbev_CapEffSnowAdd(OMOBJ *obj, HuVecF *pos, int time)
{
    CAPEFFSNOWWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFSNOWPARTWORK *particleWorkP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffSnowOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    i = 0;
    while (i < particleP->num) {
        if (particleWorkP->active <= 0.0) {
            break;
        }
        i++;
        particleWorkP++;
    }
    if (i >= particleP->num) {
        return -1;
    }
    particleWorkP->pos = *pos;
    particleWorkP->xAmplitude = 0.5f * (1.5f + (0.2f *
        MBCapsuleEffRandF()));
    particleWorkP->_unk10 = 1.5f + (0.2f *
        MBCapsuleEffRandF());
    particleWorkP->yVelocity = 2.0f * -(1.5f + (0.2f *
        MBCapsuleEffRandF()));
    particleWorkP->time = 1.0f;
    particleWorkP->timeStep = 1.0f / (float)time;
    particleWorkP->angle = mbRandMod(360);
    particleWorkP->active = 15.000000953674316f * (1.0f + (0.25f *
        MBCapsuleEffRandF()));
    particleWorkP->color.r = particleWorkP->color.g = particleWorkP->color.b =
        particleWorkP->color.a = 255;
    workP->num++;
    return i;
}

int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, int time, float scale,
    float gravity, float rotStep, GXColor *color)
{
    CAPEFFGLOWWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFGLOWPARTICLEWORK *particleWorkP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    i = 0;
    while (i < particleP->num) {
        if (particleWorkP->active <= 0.0f) {
            break;
        }
        i++;
        particleWorkP++;
    }
    if (i >= particleP->num) {
        return -1;
    }
    particleWorkP->mode = particleWorkP->phase = 0;
    particleWorkP->pos.x = pos->x;
    particleWorkP->pos.y = pos->y;
    particleWorkP->pos.z = pos->z;
    particleWorkP->vel.x = vel->x;
    particleWorkP->vel.y = vel->y;
    particleWorkP->vel.z = vel->z;
    particleWorkP->scale = scale;
    particleWorkP->time = 1.0f;
    if (time > 1) {
        particleWorkP->timeStep = 1.0f / (float)time;
    } else {
        particleWorkP->timeStep = 1.0f;
    }
    particleWorkP->gravity = rotStep;
    particleWorkP->rotStep = gravity;
    particleWorkP->active = scale;
    particleWorkP->color.r = color->r;
    particleWorkP->color.g = color->g;
    particleWorkP->color.b = color->b;
    particleWorkP->color.a = color->a;
    particleWorkP->angle = 360.0f *
        MBCapsuleEffRandF();
    particleWorkP->pat = 0;
    particleWorkP->mode = 0;
    workP->num++;
    return i;
}

int mbev_CapEffGlowKinokoAdd(OMOBJ *obj, HuVecF *posP, int time, float scale,
    float xRange, float yRange, float zRange, int type, GXColor *colorP)
{
    HuVecF pos;
    HuVecF vel;
    GXColor colorTemp;
    GXColor color;
    float gravity;
    float rotStep;
    float randF;
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == NULL) {
            break;
        }
    }
    pos.x = posP->x + (xRange * (-0.5f
        + MBCapsuleEffRandF()));
    pos.y = posP->y + (yRange * (-0.5f
        + MBCapsuleEffRandF()));
    pos.z = posP->z + (zRange
        * MBCapsuleEffRandF());
    vel.x = 0.0f;
    vel.z = 0.0f;
    switch (type) {
        case 1:
            vel.y = -(1.0f + (0.2f
                * MBCapsuleEffRandF()));
            gravity = 0.0f;
            rotStep = 0.0f;
            if (mbRandMod(0x8000) & 1) {
                gravity *= -1.0f;
            }
            break;

        case 2:
            vel.y = 1.0f + (0.2f
                * MBCapsuleEffRandF());
            gravity = 0.05f + (0.02f
                * MBCapsuleEffRandF());
            rotStep = 0.0f;
            if (mbRandMod(0x8000) & 1) {
                gravity *= -1.0f;
            }
            break;

        case 3:
            vel.y = 0.0f;
            gravity = 0.05f + (0.02f
                * MBCapsuleEffRandF());
            rotStep = 0.08166666f;
            if (mbRandMod(0x8000) & 1) {
                gravity *= -1.0f;
            }
            break;

        default:
            vel.y = 0.0f;
            gravity = 0.0f;
            rotStep = 0.0f;
            break;
    }
    if (colorP != NULL) {
        color = *colorP;
    } else {
        randF = MBCapsuleEffRandF();
        colorTemp.r = 192.0f + (63.0f * randF);
        colorTemp.g = 192.0f + (63.0f * randF);
        colorTemp.b = 192.0f + (63.0f * randF);
        colorTemp.a = 192.0f + (63.0f *
            MBCapsuleEffRandF());
        color = colorTemp;
    }
    return mbev_CapEffGlowAdd(obj, &pos, &vel, time, scale, gravity, rotStep,
        &color);
}

void mbev_CapEffGlowCoinAdd(OMOBJ *obj, HuVecF *posP, HuVecF *rotP)
{
    Mtx mtx;
    HuVecF velTemp;
    HuVecF pos;
    HuVecF vel;
    GXColor colorTemp;
    GXColor color;
    int i;
    GXColor *colorP;
    HuVecF *velP;
    HuVecF *posLocalP;
    float angle;
    float direction;
    float scale;
    float sinAngle;
    float cosDirection;
    float sinDirection;
    float cosAngle;
    float cosDirection2;
    float timeScale;

    if (rotP != NULL) {
        mtxRot(mtx, rotP->x, rotP->y, rotP->z);
    } else {
        PSMTXIdentity(mtx);
    }
    for (i = 0; i < 16; i++) {
        angle = (45.0f * (float)i) + (10.0f * (-0.5f +
            MBCapsuleEffRandF()));
        if (i & 1) {
            direction = 10.0f *
                MBCapsuleEffRandF();
        } else {
            direction = -10.0f *
                MBCapsuleEffRandF();
        }
        scale = 5.0f * (0.5f +
            MBCapsuleEffRandF());
        sinAngle = mbSinDeg(angle);
        cosDirection = mbCosDeg(direction);
        velTemp.x = scale * (cosDirection * sinAngle);
        sinDirection = mbSinDeg(direction);
        velTemp.y = scale * sinDirection;
        cosAngle = mbCosDeg(angle);
        cosDirection2 = mbCosDeg(direction);
        velTemp.z = scale * (cosDirection2 * cosAngle);
        PSMTXMultVec(mtx, &velTemp, &velTemp);
        colorTemp.r = 255;
        colorTemp.g = 255;
        colorTemp.b = 0;
        colorTemp.a = 255;
        color.r = colorTemp.r;
        color.g = colorTemp.g;
        color.b = colorTemp.b;
        color.a = colorTemp.a;
        colorP = &color;
        vel = velTemp;
        velP = &vel;
        pos = *posP;
        posLocalP = &pos;
        timeScale = 60.0f * (0.5f + (0.25f *
            MBCapsuleEffRandF()));
        mbev_CapEffGlowAdd(obj, posLocalP, velP,
            (int)(100.0f * (0.5f + (0.3f *
                MBCapsuleEffRandF()))),
            timeScale, 0.0f, 0.16333334f, colorP);
    }
}

void mbev_CapEffElectricOMExec(OMOBJ *obj)
{
    CAPEFFELECTRICWORK *work = obj->data;
    CAPEFFPARTICLESYSTEMWORK *particleSystem;
    CAPEFFGLOWPARTICLEWORK *particle;
    CAPEFFELECTRICPARTWORK *part;
    HuVecF end;
    HuVecF delta;
    float horizontal;
    int i;
    int j;

    part = work->part;
    if (mbExitCheck()
        || ev_CapEffElectricOMObj[work->objIdx] == (OMOBJ *)-1) {
        Hu3DModelKill(work->modelId);
        work->modelId = -1;
        HuSprAnimKill(work->animP);
        work->animP = NULL;
        ev_CapEffElectricOMObj[work->objIdx] = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    particleSystem = Hu3DData[work->modelId].hookData;
    particle = particleSystem->data;
    for (i = 0; i < 32; i++, part++, particle += 6) {
        if (part->activeNo < 0 || ++part->time < part->timeMax) {
            continue;
        }
        part->time = 0;
        if (part->modelId < 0) {
            end = part->pos0;
        } else {
            mbObjPosGet(part->modelId, &end);
            PSVECAdd(&end, &part->modelPos, &end);
            PSVECSubtract(&end, &part->pos2, &delta);
            for (j = 0; j < 6; j++) {
                PSVECAdd(&particle[j].pos, &delta, &particle[j].pos);
            }
        }
        end.x += 2.5f * (100.0f * (-0.5f
            + MBCapsuleEffRandF()));
        end.y += 2.5f * (100.0f * (-0.5f
            + MBCapsuleEffRandF()));
        end.z += 2.5f * (100.0f * (-0.5f
            + MBCapsuleEffRandF()));
        part->pos2 = part->pos1;
        part->pos1 = end;
        for (j = 5; j > 0; j--) {
            part->posHist[j] = part->posHist[j - 1];
            particle[j] = particle[j - 1];
        }
        part->posHist[0] = part->pos2;
        PSVECSubtract(&part->pos1, &part->pos2, &delta);
        PSVECScale(&delta, &delta, 0.5f);
        PSVECAdd(&part->pos2, &delta, &particle[0].pos);
        PSVECSubtract(&part->pos1, &part->pos2, &delta);
        horizontal = sqrt(delta.x * delta.x + delta.z * delta.z);
        particle[0].rotX = 180.0 * (atan2(-delta.y, horizontal) / M_PI);
        particle[0].rotY = 90.0
            + (180.0 * (atan2(delta.x, delta.z) / M_PI));
        particle[0].angle = 0.0f;
        particle[0].pat = mbRandMod(4);
        particle[0].active = PSVECMag(&delta);
        if (particle[0].active <= 0.0f) {
            particle[0].active = 1.0f;
        }
        part->length = particle[0].active;
        if (++part->phase >= part->phaseMax) {
            part->activeNo = -1;
            for (j = 0; j < 6; j++) {
                particle[j].active = 0.0f;
            }
            work->num--;
        }
    }
}

OMOBJ *mbev_CapEffRayCreate(float unk00, float unk04)
{
    CAPEFFRAYPARTICLEWORK *particleP;
    CAPEFFRAYWORK *workP;
    int j;
    HU3D_MODEL *modelP;
    int i;
    int objIdx;
    OMOBJ *obj;
    void *dlBuf;
    void *dlBegin;
    u32 dataHeap;
    u32 dlBufHeap;
    int dlSizeData;
    u32 dlDataHeap;
    CAPEFFRAYPARTICLEWORK *dataBuf;
    CAPEFFRAYPARTICLEWORK *dataP;
    void *dlBufData;
    void *dlBeginData;
    void *dlData;
    void *dlP;
    float t;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffRayOMObj[objIdx] == NULL) {
            break;
        }
    }
    obj = omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapEffRayOMExec);
    ev_CapEffRayOMObj[objIdx] = obj;
    workP = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFRAYWORK), HU_MEMNUM_OVL);
    memset(workP, 0, sizeof(CAPEFFRAYWORK));
    workP->objIdx = objIdx;
    workP->alpha = 1.0f;
    workP->modelId = Hu3DHookFuncCreate(mbev_CapEffRayDraw);
    Hu3DModelCameraSet(workP->modelId, 1);
    Hu3DModelLayerSet(workP->modelId, 5);
    modelP = &Hu3DData[workP->modelId];
    modelP->hookData = workP;
    dataHeap = modelP->mallocNo;
    dataBuf = HuMemDirectMallocNum(HEAP_MODEL, 128 * sizeof(CAPEFFRAYPARTICLEWORK),
        dataHeap);
    workP->particleP = dataP = dataBuf;
    particleP = dataP;
    memset(particleP, 0, 128 * sizeof(CAPEFFRAYPARTICLEWORK));
    for (i = 0; i < 128; i++, particleP++) {
        particleP->index = i;
        particleP->state = 0;
        particleP->_unk08 = 0;
        particleP->_unk0C = 0;
        particleP->_unk10 = 1.0f;
        particleP->_unk18.x = particleP->_unk18.y = particleP->_unk18.z = 0.0f;
        particleP->_unk24.x = particleP->_unk24.y = particleP->_unk24.z = 0.0f;
        particleP->_unk30.x = particleP->_unk30.y = particleP->_unk30.z = 0.0f;
        for (j = 0; j < 16; j += 2) {
            t = (float)(j / 2) / 7.0f;
            particleP->vtx[j].x = -0.05f - (unk04 * t);
            particleP->vtx[j].y = unk00 + t;
            particleP->vtx[j].z = 0.0f;
            particleP->vtx[j + 1].x = 0.05f + (unk04 * t);
            particleP->vtx[j + 1].y = unk00 + t;
            particleP->vtx[j + 1].z = 0.0f;
            particleP->prevVtx[j] = particleP->vtx[j];
            particleP->prevVtx[j + 1] = particleP->vtx[j + 1];
        }
        for (j = 0; j < 16; j++) {
            t = (float)j / 7.0f;
            mbev_CapEffColorSet(&particleP->color[j], mbRandMod(0x8000));
            particleP->color[j].a =
                255.0 * sin(3.141592653589793 * (180.0f * t) / 180.0);
        }
    }
    DCFlushRangeNoSync(workP->particleP, 128 * sizeof(CAPEFFRAYPARTICLEWORK));
    dlBufHeap = modelP->mallocNo;
    dlBufData = HuMemDirectMallocNum(HEAP_MODEL, 0x10000, dlBufHeap);
    dlBeginData = dlBufData;
    dlBegin = dlBuf = dlBeginData;
    DCFlushRange(dlBuf, 0x10000);
    GXBeginDisplayList(dlBegin, 0x10000);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 16);
    for (i = 0; i < 8; i++) {
        GXPosition1x16(i * 2);
        GXColor1x16(i);
        GXPosition1x16((i * 2) + 1);
        GXColor1x16(i);
    }
    GXEnd();
    workP->displayListSize = GXEndDisplayList();
    dlDataHeap = modelP->mallocNo;
    dlSizeData = workP->displayListSize;
    dlData = HuMemDirectMallocNum(HEAP_MODEL, dlSizeData, dlDataHeap);
    dlP = dlData;
    workP->displayList = dlP;
    memcpy(workP->displayList, dlBuf, workP->displayListSize);
    DCFlushRange(workP->displayList, workP->displayListSize);
    HuMemDirectFree(dlBuf);
    return obj;
}

OMOBJ *mbev_CapEffCoinCreate(void)
{
    OMOBJ *obj;
    int i;
    CAPEFFCOINWORK *workP;
    int objIdx;
    CAPEFFCOINWORK *workP2;
    void *workData;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffCoinOMObj[objIdx] == NULL) {
            break;
        }
    }
    obj = ev_CapEffCoinOMObj[objIdx] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapEffCoinOMExec);
    workP = HuMemDirectMallocNum(HEAP_HEAP, 128 * sizeof(CAPEFFCOINWORK), HU_MEMNUM_OVL);
    obj->data = workP;
    workData = workP;
    workP2 = workP;
    memset(workP2, 0, 128 * sizeof(CAPEFFCOINWORK));
    workP2->objIdx = objIdx;
    for (i = 0; i < 128; i++, workP2++) {
        workP2->modelId = -1;
        workP2->activeF = 0;
        workP2->_unk0C = 1;
        workP2->_unk10 = 0;
        workP2->_unk14 = 0;
        workP2->_unk1C = 0.0f;
        workP2->maxY = -1000000.0f;
        workP2->_unk2C.x = workP2->_unk2C.y = workP2->_unk2C.z = 0.0f;
        workP2->_unk38.x = workP2->_unk38.y = workP2->_unk38.z = 0.0f;
        workP2->_unk44.x = workP2->_unk44.y = workP2->_unk44.z = 1.0f;
        workP2->_unk50.x = workP2->_unk50.y = workP2->_unk50.z = 0.0f;
    }
    return obj;
}

void mbev_CapEffCoinOMExec(OMOBJ *obj)
{
    CAPEFFCOINWORK *workP;
    HuVecF pos;
    HuVecF vel;
    GXColor color;
    float angle;
    float angleX;
    float speed;
    float randF;
    int i;
    int j;

    workP = obj->data;
    if (mbExitCheck()
        || ev_CapEffCoinOMObj[workP->objIdx] == (OMOBJ *)-1) {
        for (i = 0; i < 128; i++, workP++) {
            if (workP->modelId >= 0) {
                mbCoinObjNumDec(workP->modelId);
            }
            workP->modelId = -1;
        }
        workP = obj->data;
        ev_CapEffCoinOMObj[workP->objIdx] = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    for (i = 0; i < 128; i++, workP++) {
        if (!workP->activeF) {
            continue;
        }
        PSVECAdd(&workP->_unk2C, &workP->_unk50, &workP->_unk2C);
        workP->_unk50.y -= workP->_unk1C;
        workP->_unk50.y *= 0.95f;
        workP->_unk38.y += workP->_unk18;
        mbCoinObjPosSetV(workP->modelId, &workP->_unk2C);
        mbCoinObjRotSetV(workP->modelId, &workP->_unk38);
        mbCoinObjScaleSetV(workP->modelId, &workP->_unk44);
        if (workP->_unk0C == 0) {
            workP->_unk24 -= workP->_unk28;
            mbCoinObjAlphaSet(workP->modelId,
                mbCosDeg(90.0f * (1.0f - workP->_unk24)
                    * (1.0f - workP->_unk24)));
        }
        workP->_unk14++;
        if (workP->_unk14 <= workP->_unk10
            && (workP->_unk50.y >= 0.0f
                || workP->_unk2C.y >= workP->maxY)) {
            continue;
        }
        workP->activeF = FALSE;
        if (workP->modelId >= 0) {
            mbCoinObjNumDec(workP->modelId);
        }
        workP->modelId = -1;
        if (workP->_unk0C == 3 && workP->glowObj != NULL) {
            angle = 0.0f;
            for (j = 0; j < 16; j++) {
                angle += 360.0f * (0.5f + MBCapsuleEffRandF());
                pos = workP->_unk2C;
                randF = MBCapsuleEffRandF();
                speed = 50.0f * (1.0f + (0.2f * randF));
                randF = MBCapsuleEffRandF();
                angleX = -(75.0f + (15.0f * randF));
                vel.x = speed * (mbSinDeg(angle) * mbCosDeg(angleX));
                vel.z = speed * (mbCosDeg(angle) * mbCosDeg(angleX));
                vel.y = speed * mbSinDeg(angleX);
                randF = MBCapsuleEffRandF();
                color.r = 192.0f + (63.0f * randF);
                color.g = 192.0f + (63.0f * randF);
                color.b = 128.0f + (63.0f * randF);
                color.a = 192.0f + (63.0f *
                    MBCapsuleEffRandF());
                mbev_CapEffGlowAdd(workP->glowObj, &pos, &vel, 6,
                    100.0f * (0.5f + (0.3f *
                        MBCapsuleEffRandF())),
                    0.0f, 3.2666667f, &color);
            }
        } else if (workP->_unk0C == 4) {
            pos = workP->_unk2C;
            pos.y -= 15.0f;
            mbCoinEffCreate(&pos);
        }
    }
}

int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, float scale,
    float gravity, int time, int arg)
{
    CAPEFFCOINWORK *workP;
    int i;
    int coinNo;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffCoinOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    coinNo = 0;
    while (coinNo < 128) {
        if (workP->activeF == 0) {
            break;
        }
        coinNo++;
        workP++;
    }
    if (coinNo >= 128) {
        return -1;
    }
    workP->activeF = 1;
    workP->_unk0C = arg;
    workP->_unk10 = time;
    workP->_unk14 = 0;
    workP->_unk1C = gravity;
    workP->maxY = -1000000.0f;
    workP->_unk24 = 1.0f;
    workP->_unk28 = 1.0f / (float)time;
    workP->_unk2C = *pos;
    workP->_unk38.y = 360.0f * MBCapsuleEffRandF();
    workP->_unk38.x = 30.0f * (-0.5f + MBCapsuleEffRandF());
    workP->_unk38.z = 0.0f;
    workP->_unk44.x = workP->_unk44.y = workP->_unk44.z = scale;
    workP->_unk50 = *vel;
    workP->_unk18 = 10.0f * (-0.5f + MBCapsuleEffRandF());
    workP->modelId = mbCoinCreate2();
    mbCoinObjPosSetV(workP->modelId, &workP->_unk2C);
    mbCoinObjRotSetV(workP->modelId, &workP->_unk38);
    mbCoinObjScaleSetV(workP->modelId, &workP->_unk44);
    return coinNo;
}

void mbev_CapEffCoinMultiAdd(OMOBJ *obj, HuVecF *pos, int num)
{
    HuVecF pos2;
    HuVecF vel;
    s16 angleRand;
    float angleRandF;
    s16 angleXRand;
    float angleXRandF;
    s16 speedRand;
    float speedRandF;
    float cosAngleX1;
    float sinAngle;
    float cosAngleX2;
    float cosAngle;
    float sinAngleX;
    float angle;
    float angleX;
    float speed;
    int i;

    CAP_EFF_RAND_NEXT();
    angleRand = mbCapEffData[mbCapEffNum];
    angleRandF = (float)angleRand * (1.0f / 32767.0f);
    angle = 360.0f * angleRandF;
    for (i = 0; i < num; i++) {
        angle += 360.0f * (1.0f / (float)num);
        pos2 = *pos;
        pos2.y += 100.0f;
        CAP_EFF_RAND_NEXT();
        angleXRand = mbCapEffData[mbCapEffNum];
        angleXRandF = (float)angleXRand * (1.0f / 32767.0f);
        angleX = 65.0f + (15.0f * angleXRandF);
        CAP_EFF_RAND_NEXT();
        speedRand = mbCapEffData[mbCapEffNum];
        speedRandF = (float)speedRand * (1.0f / 32767.0f);
        speed = 20.0f * (0.8f + (0.4f * speedRandF));
        cosAngleX1 = mbCosDeg(angleX);
        sinAngle = mbSinDeg(angle);
        vel.x = speed * (sinAngle * cosAngleX1);
        cosAngleX2 = mbCosDeg(angleX);
        cosAngle = mbCosDeg(angle);
        vel.z = speed * (cosAngle * cosAngleX2);
        sinAngleX = mbSinDeg(angleX);
        vel.y = speed * sinAngleX;
        mbev_CapEffCoinAdd(obj, &pos2, &vel, 0.75f, 0.8166666666666667f, 30, 0);
    }
}

static void ev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum, BOOL highF,
    void (*hook)(void))
{
    HuVecF playerPos;
    HuVecF pos;
    HuVecF vel;
    float maxY;
    int delay;
    int i;
    int coinNo;
    void (*hookP)(void);

    if (coinNum <= 0) {
        return;
    }
    if (coinNum <= 5) {
        delay = 5;
    } else if (coinNum <= 10) {
        delay = 3;
    } else if (coinNum <= 20) {
        delay = 2;
    } else {
        delay = 1;
    }
    for (i = 0; i < coinNum; i++) {
        mbPlayerPosGet(playerNo, &playerPos);
        pos = playerPos;
        if (highF) {
            pos.y += 600.0f;
        } else {
            pos.y += 300.0f;
        }
        pos.x += 0.5f * (100.0f * (-0.5f +
            MBCapsuleEffRandF()));
        vel.x = vel.y = vel.z = 0.0f;
        coinNo = mbev_CapEffCoinAdd(obj, &pos, &vel, 0.75f, 4.9f, 30, 4);
        if (coinNo >= 0) {
            int objNo;
            CAPEFFCOINWORK *workP;

            maxY = 150.0f + playerPos.y;
            for (objNo = 0; objNo < 8; objNo++) {
                if (ev_CapEffCoinOMObj[objNo] == obj) {
                    break;
                }
            }
            workP = obj->data;
            workP += coinNo;
            workP->maxY = maxY;
        }
        HuPrcSleep(delay);
    }
    while (mbev_CapEffCoinNumGet(obj) > 0) {
        HuPrcVSleep();
    }
    if (hook != NULL) {
        hookP = hook;
        hookP();
    }
    mbCoinAddDispExec(playerNo, coinNum, FALSE, TRUE);
    mbev_CapCoinDisp(playerNo, coinNum, TRUE, TRUE);
}

float mbev_CapAngleWrap(float a, float b)
{
    float result;

    if (a >= 360) {
        a -= 360;
    } else if (a < 0) {
        a += 360;
    }
    if (b >= 360) {
        b -= 360;
    } else if (b < 0) {
        b += 360;
    }
    result = a - b;
    if (result <= -180.0f) {
        result += 360;
    } else if (result >= 180.0f) {
        result -= 360;
    }
    return result;
}

float mbev_CapAngleLerp(float a, float b, float t)
{
    float result;
    float delta;

    if (a >= 360.0) {
        a -= 360.0;
    } else if (a < 0.0) {
        a += 360.0;
    }
    if (b >= 360.0) {
        b -= 360.0;
    } else if (b < 0.0) {
        b += 360.0;
    }
    delta = (a - b) + 360.0;
    if (fabs(delta) >= 360) {
        delta = fmod(delta, 360);
    }
    if (delta < 180.0) {
        if (delta <= t) {
            result = delta;
        } else {
            result = t;
        }
    } else {
        if ((360.0 - delta) <= t) {
            result = -(360.0 - delta);
        } else {
            result = -t;
        }
    }
    result += b;
    if (result >= 360.0) {
        result -= 360.0;
    } else if (result < 0.0) {
        result += 360.0;
    }
    return result;
}

float mbev_CapAngleSumLerp(float t, float a, float b)
{
    float wrapAngle = mbev_CapAngleWrap(b, a);

    return mbev_CapAngleLerp(b, a, fabs(wrapAngle * t));
}

static float ev_CapRotCamera(float angle)
{
    MBCAMERA *camera;
    Mtx mtx;
    HuVecF dir;

    camera = mbCameraGet();
    mtxRot(mtx, camera->rot.x, camera->rot.y, camera->rot.z);
    dir.x = sin((M_PI * angle) / 180.0);
    dir.y = 0.0f;
    dir.z = cos((M_PI * angle) / 180.0);
    PSMTXMultVec(mtx, &dir, &dir);
    return 180.0 * (atan2(dir.x, dir.z) / M_PI);
}

void mbev_CapHermiteConstGet(float t, float *a, float *b, float *c, float *d)
{
    float square = t * t;
    float cube = t * square;
    float blend = ((3.0 * square) - cube) - cube;

    *a = 1.0 - blend;
    *b = blend;
    *c = t + ((cube - square) - square);
    *d = cube - square;
}

float mbev_CapHermiteConstGet2(float t, float a, float b, float c, float d)
{
    float delta;
    float h00;
    float h01;
    float h10;
    float h11;
    float result;
    float half0;
    float half1;
    float tangent0;
    float tangent1;
    int deltaInt;

    delta = c - b;
    deltaInt = c - b;
    if (b == c) {
        tangent0 = delta;
    } else {
        half0 = 0.5f;
        tangent0 = half0 * (delta + (b - a));
    }
    if (b == d) {
        tangent1 = delta;
    } else {
        half1 = 0.5f;
        tangent1 = half1 * (delta + (d - c));
    }
    mbev_CapHermiteConstGet(t, &h00, &h01, &h10, &h11);
    return result = (h00 * b) + (h01 * c) + (h10 * tangent0)
        + (h11 * tangent1);
}

void mbev_CapHermiteGetV(
    float t,
    HuVecF *a,
    HuVecF *b,
    HuVecF *c,
    HuVecF *d,
    HuVecF *out)
{
    out->x = mbev_CapHermiteConstGet2(t, a->x, b->x, c->x, d->x);
    out->y = mbev_CapHermiteConstGet2(t, a->y, b->y, c->y, d->y);
    out->z = mbev_CapHermiteConstGet2(t, a->z, b->z, c->z, d->z);
}

float mbev_CapBezierGet(float t, float a, float b, float c)
{
    float temp = 1.0 - t;
    float result = (a * (temp * temp)) + (temp * t * b * 2.0) + (t * t * c);

    return result;
}

void mbev_CapBezierGetV(float t, float *a, float *b, float *c, float *out)
{
    int i;

    for (i = 0; i < 3; i++) {
        *out++ = mbev_CapBezierGet(t, *a++, *b++, *c++);
    }
}

float mbev_CapBezierSlopeGet(float t, float a, float b, float c)
{
    float result = 2.0 * (((t - 1.0) * a) + ((1.0 - (2.0 * t)) * b)
        + (t * c));

    return result;
}

void mbev_CapBezierNormGetV(float t, float *a, float *b, float *c, float *out)
{
    int i;
    float temp[3];
    float mag;

    for (i = 0; i < 3; i++) {
        temp[i] = mbev_CapBezierSlopeGet(t, *a++, *b++, *c++);
    }
    mag = HuMagPoint3D(temp[0], temp[1], temp[2]);
    if (mag) {
        mag = 1.0 / mag;
        for (i = 0; i < 3; i++) {
            *out++ = temp[i] * mag;
        }
    } else {
        *out++ = 0;
        *out++ = 0;
        *out++ = 1;
    }
}



int mbev_CapPlayerComSelGet(int playerNo, int selection)
{
    return mbev_CapPlayerComSelSameGet(playerNo, selection, FALSE);
}

int mbev_CapPlayerComSelSameGet(int playerNo, int selection, BOOL sameF)
{
    int playerList[4];
    int i;
    int playerNum;

    for (i = 0, playerNum = 0; i < 4; i++) {
        if ((int)GwSystem.tagF != FALSE) {
            BOOL playerCheckF;

            if ((int)GwSystem.tagF == FALSE) {
                if (playerNo == i) {
                    playerCheckF = TRUE;
                } else {
                    playerCheckF = FALSE;
                }
            } else {
                BOOL team1;
                BOOL team2;

                team1 = GwPlayer[i].team;
                team2 = GwPlayer[playerNo].team;
                if (team2 == team1) {
                    playerCheckF = TRUE;
                } else {
                    playerCheckF = FALSE;
                }
            }
            if (playerCheckF) {
                continue;
            }
        }
        if (sameF) {
            if (GwPlayer[playerNo].masuId != GwPlayer[i].masuId) {
                continue;
            }
        }
        if (i == playerNo) {
            continue;
        }
        playerList[playerNum] = i;
        playerNum++;
    }
    return mbev_CapPlayerComSelRandomGet(playerNo, selection, playerList, playerNum);
}




void mbev_CapChoiceSet(int choice)
{
    capsuleChoice = choice;
    mbWinTopComKeyHookSet(ev_CapComChoiceHook);
}

void mbev_CapVecChase(
    float weight, HuVecF *src, HuVecF *target, HuVecF *out)
{
    HuVecF delta;

    PSVECSubtract(target, src, &delta);
    PSVECScale(&delta, &delta, weight);
    PSVECAdd(src, &delta, out);
}

void mbev_CapEffBoostKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffBoostOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffBoostOMObj[i] = (OMOBJ *)-1;
}

void mbev_CapEffElectricKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffElectricOMObj[i] == obj) {
            break;
        }
    }
    i >= 8;
    ev_CapEffElectricOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapEffElectricDispGet(OMOBJ *obj)
{
    int i;
    CAPEFFDISPWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffElectricOMObj[i] == obj) {
            break;
        }
    }
    i >= 8;
    workP = omObjGetDataAs(obj, CAPEFFDISPWORK);
    return workP->dispF;
}

int mbev_CapEffBoostTimeGet(OMOBJ *obj)
{
    int i;
    CAPEFFBOOSTWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffBoostOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFBOOSTWORK);
    return workP->time;
}

void mbev_CapEffBoostBlendModeSet(OMOBJ *obj, int blendMode)
{
    int i;
    CAPEFFBOOSTWORK *workP;
    CAPEFFBOOSTPARTWORK *particleP;
    HU3D_MODEL *modelP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffBoostOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFBOOSTWORK);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleP->blendMode = blendMode;
}






void mbev_CapEffSnowKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffSnowOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffSnowOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapEffSnowDispGet(OMOBJ *obj)
{
    int i;
    CAPEFFDISPWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffSnowOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFDISPWORK);
    return workP->dispF;
}






void mbev_CapEffGlowKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffGlowOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapEffGlowDispGet(OMOBJ *obj)
{
    int i;
    CAPEFFDISPWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFDISPWORK);
    return workP->dispF;
}

void mbev_CapEffGlowPatSet(OMOBJ *obj, int pat)
{
    int i;
    CAPEFFBOOSTWORK *workP;
    CAPEFFGLOWPARTWORK *particleP;
    HU3D_MODEL *modelP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFBOOSTWORK);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleP->pat = pat;
}

void mbev_CapEffGlowBlendModeSet(OMOBJ *obj, int blendMode)
{
    int i;
    CAPEFFBOOSTWORK *workP;
    CAPEFFGLOWPARTWORK *particleP;
    HU3D_MODEL *modelP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFBOOSTWORK);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleP->blendMode = blendMode;
}

void mbev_CapEffGlowAnimSet(OMOBJ *obj, int dataNum)
{
    int i;
    CAPEFFBOOSTWORK *workP;
    CAPEFFGLOWPARTWORK *particleP;
    HU3D_MODEL *modelP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFBOOSTWORK);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    HuSprAnimKill(workP->animP);
    workP->animP = particleP->animP = HuSprAnimRead(HuDataReadNum(dataNum, HU_MEMNUM_OVL));
}


int mbev_CapEffGlowKinokoTimeSet(OMOBJ *obj, int index, int unk08, int unk0A)
{
    int i;
    CAPEFFBOOSTWORK *workP;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFGLOWKINOKOPARTICLEWORK *particleWorkP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffGlowOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    if (index < 0 || index >= particleP->num) {
        return FALSE;
    }
    particleWorkP = &((CAPEFFGLOWKINOKOPARTICLEWORK *)particleP->data)[index];
    particleWorkP->_unk00 = unk08;
    particleWorkP->_unk02 = unk0A;
    particleWorkP->_unk04 = 0;
    return TRUE;
}



void mbev_CapEffRingKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffRingOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapEffRingDispGet(OMOBJ *obj)
{
    int i;
    CAPEFFRINGWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFRINGWORK);
    return workP->dispF;
}








int mbev_CapEffRayAdd(OMOBJ *obj, HuVecF *unk04, HuVecF *unk08, HuVecF *unk0C,
    int unk10, float unk14)
{
    CAPEFFRAYWORK *workP;
    int i;
    CAPEFFRAYPARTICLEWORK *particleP;
    int particleNo;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRayOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    particleP = workP->particleP;
    particleNo = 0;
    while (particleNo < 128) {
        if (particleP->state == 0) {
            break;
        }
        particleNo++;
        particleP++;
    }
    if (particleNo >= 128) {
        return -1;
    }
    particleP->state = 1;
    particleP->_unk08 = 0;
    particleP->_unk0C = unk10;
    particleP->_unk10 = unk14;
    particleP->_unk18 = *unk04;
    particleP->_unk24 = *unk08;
    particleP->_unk30 = *unk0C;
    return particleNo;
}

OMOBJ *mbev_CapEffRingCreate(void)
{
    CAPEFFRINGWORK *workP;
    OMOBJ *obj;
    CAPEFFRINGHITPARTWORK *particleP;
    HU3D_MODEL *modelP;
    int modelId;
    ANIMDATA *animP;
    void *workData;
    int j;
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == NULL) {
            break;
        }
    }
    obj = ev_CapEffRingOMObj[i] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapEffRingOMExec);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFRINGWORK), HU_MEMNUM_OVL);
    obj->data = workData;
    workP = workData;
    memset(workP, 0, sizeof(CAPEFFRINGWORK));
    workP->dispF = 0;
    workP->objIdx = i;
    for (j = 0; j < 3; j++) {
        workP->animP[j] = animP =
            HuSprAnimRead(HuDataReadNum(ev_CapEffRingFile[j], HU_MEMNUM_OVL));
        workP->modelId[j] = modelId = ev_CapEffCreate(animP, 32);
        Hu3DModelLayerSet(modelId, 5);
        workP->dispF = 0;
        modelP = &Hu3DData[modelId];
        particleP = modelP->hookData;
        particleP->blendMode = 1;
        particleP->dispAttr = 0x4F;
    }
    return obj;
}


OMOBJ *mbev_CapEffRingHitCreate(void)
{
    CAPEFFRINGWORK *workP;
    OMOBJ *obj;
    CAPEFFRINGHITPARTWORK *particleP;
    HU3D_MODEL *modelP;
    ANIMDATA *animP;
    int modelId;
    void *workData;
    int i;
    int j;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == NULL) {
            break;
        }
    }
    obj = ev_CapEffRingOMObj[i] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapEffRingOMExec);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFRINGWORK), HU_MEMNUM_OVL);
    obj->data = workData;
    workP = workData;
    memset(workP, 0, sizeof(CAPEFFRINGWORK));
    workP->dispF = 0;
    workP->objIdx = i;
    for (j = 0; j < 3; j++) {
        if (j == 0) {
            workP->animP[j] = animP = ringHitEffAnim2;
        } else {
            workP->animP[j] = animP = ringHitEffAnim1;
        }
        workP->modelId[j] = modelId = ev_CapEffCreate(animP, 32);
        Hu3DModelLayerSet(modelId, 5);
        workP->dispF = 0;
        modelP = &Hu3DData[modelId];
        particleP = modelP->hookData;
        particleP->blendMode = 1;
        particleP->dispAttr = 0x4F;
    }
    return obj;
}

void mbev_CapEffRingOMExec(OMOBJ *obj)
{
    CAPEFFRINGWORK *workP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleSystemP;
    CAPEFFRINGPARTICLEWORK *particleP;
    HU3D_MODEL *modelP;
    float weight;
    float easedIn;
    float easedOut;
    int i;
    int j;

    workP = obj->data;
    if (mbExitCheck()
        || ev_CapEffRingOMObj[workP->objIdx] == (OMOBJ *)-1) {
        for (i = 0; i < 3; i++) {
            Hu3DModelKill(workP->modelId[i]);
            workP->modelId[i] = -1;
        }
        for (i = 0; i < 3; i++) {
            HuSprAnimKill(workP->animP[i]);
            workP->animP[i] = NULL;
        }
        ev_CapEffRingOMObj[workP->objIdx] = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    for (i = 0; i < 3; i++) {
        if (workP->dispF <= 0) {
            Hu3DModelAttrSet(workP->modelId[i], 1);
        } else {
            Hu3DModelAttrReset(workP->modelId[i], 1);
            modelP = &Hu3DData[workP->modelId[i]];
            particleSystemP = modelP->hookData;
            particleP = particleSystemP->data;
            particleSystemP->_unk21[2] = 0;
            for (j = 0; j < particleSystemP->num; j++, particleP++) {
                if (particleP->_unk40 <= 0.0f) {
                    continue;
                }
                switch (particleP->_unk00) {
                case 0:
                    weight = (float)++particleP->_unk02 / particleP->_unk14;
                    easedIn = mbSinDeg(90.0f * weight);
                    weight = easedIn;
                    particleP->_unk40 = particleP->_unk08.z
                        * (particleP->_unk08.x
                            + weight * (1.0f - particleP->_unk08.x));
                    particleP->color.a = particleP->_unk1C * weight;
                    if (weight >= 1.0f) {
                        particleP->_unk40 = particleP->_unk08.z;
                        particleP->color.a = particleP->_unk1C;
                        particleP->_unk00++;
                        particleP->_unk02 = 0;
                    }
                    break;
                case 1:
                    weight = (float)++particleP->_unk02 / particleP->_unk18;
                    easedOut = mbSinDeg(90.0f * weight);
                    weight = easedOut;
                    particleP->_unk40 = particleP->_unk08.z
                        * (1.0f + weight * (particleP->_unk08.y - 1.0f));
                    particleP->color.a = particleP->_unk1C * (1.0f - weight);
                    if (weight >= 1.0f) {
                        particleP->_unk40 = 0.0f;
                        workP->dispF--;
                    }
                    break;
                }
            }
        }
    }
}

int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot, HuVecF *scale,
    int unk10, int unk14, int index, GXColor *color)
{
    CAPEFFRINGWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *particleP;
    CAPEFFRINGPARTICLEWORK *particleWorkP;
    int particleNo;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId[index % 3]];
    particleP = modelP->hookData;
    particleWorkP = particleP->data;
    particleNo = 0;
    while (particleNo < particleP->num) {
        if (particleWorkP->_unk40 <= 0.0f) {
            break;
        }
        particleNo++;
        particleWorkP++;
    }
    if (particleNo >= particleP->num) {
        return -1;
    }
    particleWorkP->_unk00 = particleWorkP->_unk02 = 0;
    particleWorkP->_unk58.x = pos->x;
    particleWorkP->_unk58.y = pos->y;
    particleWorkP->_unk58.z = pos->z;
    particleWorkP->_unk08.x = scale->x;
    particleWorkP->_unk08.y = scale->y;
    particleWorkP->_unk08.z = scale->z;
    particleWorkP->_unk14 = unk10;
    particleWorkP->_unk18 = unk14;
    particleWorkP->_unk1C = color->a;
    particleWorkP->_unk40 = scale->z;
    particleWorkP->color = *color;
    particleWorkP->_unk4C.x = rot->x;
    particleWorkP->_unk4C.y = rot->y;
    particleWorkP->_unk4C.z = rot->z;
    particleWorkP->_unk68 = 0;
    particleWorkP->_unk00 = 0;
    workP->dispF++;
    return particleNo;
}

void mbev_CapEffRingHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot,
    HuVecF *scale)
{
    GXColor color;

    color = capsuleRingColor;
    mbev_CapEffRingAdd(obj, pos, rot, scale, 1, 12, 2, &color);
}



void mbev_CapEffRingAnimSet(OMOBJ *obj, int index, int dataNum)
{
    int i;
    CAPEFFRINGWORK *workP;
    CAPEFFGLOWPARTWORK *particleP;
    HU3D_MODEL *modelP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRingOMObj[i] == obj) {
            break;
        }
    }
    workP = obj->data;
    modelP = &Hu3DData[workP->modelId[index % 3]];
    particleP = modelP->hookData;
    HuSprAnimKill(workP->animP[index % 3]);
    workP->animP[index % 3] = particleP->animP =
        HuSprAnimRead(HuDataReadNum(dataNum, HU_MEMNUM_OVL));
}

OMOBJ *mbev_CapEffElectricCreate(void)
{
    CAPEFFELECTRICWORK *workP;
    OMOBJ *obj;
    CAPEFFELECTRICPARTWORK *partP;
    HU3D_MODEL *modelP;
    CAPEFFEXPLODEPARTWORK *particleP;
    void *workData;
    int i = 0;
    int objIdx;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffElectricOMObj[objIdx] == NULL) {
            break;
        }
    }
    obj = ev_CapEffElectricOMObj[objIdx] =
        omAddObjEx(mbObjMan, 32768, 0, 0, -1, mbev_CapEffElectricOMExec);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPEFFELECTRICWORK), HU_MEMNUM_OVL);
    obj->data = workData;
    workP = workData;
    memset(workP, 0, sizeof(CAPEFFELECTRICWORK));
    workP->animP = electricEffAnim;
    workP->modelId = ev_CapEffCreate(workP->animP, 192);
    ev_CapEffGridSet(workP->modelId, 4, 1, 0);
    Hu3DModelLayerSet(workP->modelId, 5);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    particleP->dispAttr = 0x5D;
    particleP->blendMode = 1;
    workP->num = 0;
    workP->objIdx = objIdx;
    partP = workP->part;
    for (i = 0; i < 32; i++, partP++) {
        partP->activeNo = -1;
    }
    return obj;
}

static s16 ev_CapEffCreate(ANIMDATA *animP, s16 max)
{
    s16 modelId;
    HU3D_MODEL *model;
    CAPEFFPARTICLESYSTEMWORK *work;
    CAPEFFGLOWPARTICLEWORK *particle;
    HuVecF *vertex;
    HuVec2f *texCoord;
    void *displayListBuffer;
    int i;
    int j;

    modelId = Hu3DHookFuncCreate(ev_CapEffDraw);
    Hu3DModelCameraSet(modelId, 1);
    model = &Hu3DData[modelId];
    work = HuMemDirectMallocNum(HEAP_MODEL, sizeof(CAPEFFPARTICLESYSTEMWORK),
        model->mallocNo);
    model->hookData = work;
    work->animP = animP;
    HuSprAnimLock(animP);
    work->num = max;
    work->dispAttr = 0;
    work->blendMode = 0;
    work->_unk4C = 0;
    work->_unk5C = 0;
    work->_unk28 = 0;
    work->_unk21 = 0;
    work->_unk23[0] = 0;
    work->_unk30 = 0;
    work->phase = 0;
    work->mode = 0;
    work->grid = NULL;
    work->_unk54 = 0;
    work->gridNum = 16;

    work->data = particle = HuMemDirectMallocNum(HEAP_MODEL,
        max * sizeof(CAPEFFGLOWPARTICLEWORK), model->mallocNo);
    memset(particle, 0, max * sizeof(CAPEFFGLOWPARTICLEWORK));
    for (i = 0; i < max; i++, particle++) {
        particle->active = 0.0f;
        particle->sizeX = 1.0f;
        particle->sizeY = 1.0f;
        particle->rotX = 0.0f;
        particle->rotY = 0.0f;
        particle->angle = 0.0f;
        particle->alpha = 0.0f;
        particle->alphaMax = 1.0f;
        particle->pos.x = 0.0f;
        particle->pos.y = 0.0f;
        particle->pos.z = 0.0f;
        particle->color.r = 255;
        particle->color.g = 255;
        particle->color.b = 255;
        particle->color.a = 255;
        particle->pat = 0;
    }
    work->vertices = vertex = HuMemDirectMallocNum(HEAP_MODEL,
        max * 4 * sizeof(HuVecF), model->mallocNo);
    for (i = 0; i < max * 4; i++, vertex++) {
        vertex->x = vertex->y = vertex->z = 0.0f;
    }
    work->texCoords = texCoord = HuMemDirectMallocNum(HEAP_MODEL,
        max * 4 * sizeof(HuVec2f), model->mallocNo);
    for (i = 0; i < max; i++) {
        for (j = 0; j < 4; j++, texCoord++) {
            texCoord->x = baseST2[j * 2];
            texCoord->y = baseST2[j * 2 + 1];
        }
    }

    displayListBuffer = HuMemDirectMallocNum(HEAP_MODEL, 0x10000,
        model->mallocNo);
    DCFlushRange(displayListBuffer, 0x10000);
    GXBeginDisplayList(displayListBuffer, 0x10000);
    GXBegin(GX_QUADS, GX_VTXFMT0, max * 4);
    for (i = 0; i < max; i++) {
        for (j = 0; j < 4; j++) {
            GXPosition1x16(i * 4 + j);
            GXColor1x16(i);
            GXTexCoord1x16(i * 4 + j);
        }
    }
    work->displayListSize = GXEndDisplayList();
    work->displayList = HuMemDirectMallocNum(HEAP_MODEL,
        work->displayListSize, model->mallocNo);
    memcpy(work->displayList, displayListBuffer, work->displayListSize);
    DCFlushRange(work->displayList, work->displayListSize);
    HuMemDirectFree(displayListBuffer);
    return modelId;
}

static void ev_CapEffGridSet(s16 modelId, int xNum, int yNum, int mode)
{
    HU3D_MODEL *model;
    CAPEFFGLOWKINOKOPARTICLESYSTEMWORK *work;
    HuVec2f *grid;
    int mallocNo;
    void *gridData;
    HuVec2f *gridBase;
    float xStep;
    float yStep;
    int gridNum;
    int y;
    int x;

    if (xNum < 1) {
        xNum = 1;
    }
    if (yNum < 1) {
        yNum = 1;
    }
    gridNum = xNum * yNum;
    xStep = 1.0f / (float)xNum;
    yStep = 1.0f / (float)yNum;
    model = &Hu3DData[modelId];
    work = model->hookData;
    work->gridNum = gridNum;
    mallocNo = model->mallocNo;
    gridData = HuMemDirectMallocNum(HEAP_MODEL,
        gridNum * sizeof(HuVec2f) * 4, mallocNo);
    gridBase = gridData;
    work->grid = grid = gridBase;
    memset(grid, 0, gridNum * sizeof(HuVec2f) * 4);
    for (y = 0; y < yNum; y++) {
        for (x = 0; x < xNum; x++) {
            if (mode) {
                grid->x = (float)y * xStep;
                grid->y = (float)x * yStep;
                grid++;
                grid->x = (float)(y + 1) * xStep;
                grid->y = (float)x * yStep;
                grid++;
                grid->x = (float)(y + 1) * xStep;
                grid->y = (float)(x + 1) * yStep;
                grid++;
                grid->x = (float)y * xStep;
                grid->y = (float)(x + 1) * yStep;
                grid++;
            } else {
                grid->x = (float)x * xStep;
                grid->y = (float)y * yStep;
                grid++;
                grid->x = (float)(x + 1) * xStep;
                grid->y = (float)y * yStep;
                grid++;
                grid->x = (float)(x + 1) * xStep;
                grid->y = (float)(y + 1) * yStep;
                grid++;
                grid->x = (float)x * xStep;
                grid->y = (float)(y + 1) * yStep;
                grid++;
            }
        }
    }
}











void mbev_CapEffExplodeKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffExplodeOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffExplodeOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapEffExplodeAnimGet(OMOBJ *obj)
{
    int i;
    CAPEFFEXPLODEWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffExplodeOMObj[i] == obj) {
            break;
        }
    }
    if (ev_CapEffExplodeOMObj[i] == NULL) {
        return 0;
    } else {
        workP = omObjGetDataAs(obj, CAPEFFEXPLODEWORK);
        return workP->num;
    }
}

void mbev_CapEffExplodeAnimSet(OMOBJ *obj, int dataNum)
{
    CAPEFFEXPLODEPARTWORK *particleP;
    CAPEFFEXPLODEWORK *workP;
    int i;
    HU3D_MODEL *modelP;
    void *dataP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffExplodeOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFEXPLODEWORK);
    modelP = &Hu3DData[workP->modelId];
    particleP = modelP->hookData;
    dataP = particleP->data;
    if (workP->animP != NULL) {
        HuSprAnimKill(workP->animP);
    }
    workP->animP = particleP->animP =
        HuSprAnimRead(HuDataReadNum(dataNum, HU_MEMNUM_OVL));
}

void mbev_CapEffCoinKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffCoinOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffCoinOMObj[i] = (OMOBJ *)-1;
}

void mbev_CapCoinManKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffCoinManOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffCoinManOMObj[i] = (OMOBJ *)-1;
}

void mbev_CapStarManKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffStarManOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffStarManOMObj[i] = (OMOBJ *)-1;
}


void mbev_CapEffCapLoseKill(OMOBJ *obj)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffCapLoseOMObj[i] == obj) {
            break;
        }
    }
    ev_CapEffCapLoseOMObj[i] = (OMOBJ *)-1;
}

int mbev_CapCoinManNumGet(OMOBJ *obj)
{
    CAPCOINMANWORK *workP;
    int objIdx;
    int i;
    int count;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffCoinManOMObj[objIdx] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPCOINMANWORK);
    i = 0;
    count = 0;
    for (; i < 64; i++, workP++) {
        if (workP->activeF) {
            count++;
        }
    }
    return count;
}

int mbev_CapStarManNumGet(OMOBJ *obj)
{
    CAPSTARMANWORK *workP;
    int objIdx;
    int i;
    int count;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffStarManOMObj[objIdx] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPSTARMANWORK);
    i = 0;
    count = 0;
    for (; i < 8; i++, workP++) {
        if (workP->activeF) {
            count++;
        }
    }
    return count;
}

int mbev_CapEffCapLoseNumGet(OMOBJ *obj)
{
    CAPEFFCAPLOSEWORK *workP;
    int objIdx;
    int i;
    int count;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffCapLoseOMObj[objIdx] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFCAPLOSEWORK);
    i = 0;
    count = 0;
    for (; i < 6; i++, workP++) {
        if (workP->activeF) {
            count++;
        }
    }
    return count;
}


int mbev_CapEffCapLoseObjAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, float scale, int time,
    int capsuleNo)
{
    int objNo;
    CAPEFFCAPLOSEWORK *workP;
    int workNo;
    MBMODELID colorModelId;

    for (objNo = 0; objNo < 8; objNo++) {
        if (ev_CapEffCapLoseOMObj[objNo] == obj) {
            break;
        }
    }
    workP = obj->data;
    workNo = 0;
    while (workNo < 6) {
        if (workP->activeF == 0) {
            break;
        }
        workNo++;
        workP++;
    }
    if (workNo >= 6) {
        return -1;
    }
    workP->activeF = 1;
    workP->colorObjId = mbCapObjColorCreate(capsuleNo, TRUE);
    colorModelId = (MBMODELID)workP->colorObjId;
    mbObjAttrSet(colorModelId, HU3D_MOTATTR_LOOP);
    workP->capsuleNo = capsuleNo;
    workP->_unk10 = -1;
    workP->_unk14 = 0;
    workP->time = time;
    workP->pos = *pos;
    workP->vel = *vel;
    mbCapObjColorPosSet(workP->colorObjId, pos->x, pos->y, pos->z);
    mbCapObjColorScaleSet(workP->colorObjId, scale, scale, scale);
    mbCapObjColorLayerSet(workP->colorObjId, 3);
    return workNo;
}


static void ev_CapBiriQMetalShockDestroy(void)
{
    void *workP = HuPrcCurrentGet()->property;

    HuMemDirectFree(workP);
}

static void ev_CapBiriQMetalShock(void)
{
    void *workP = HuPrcCurrentGet()->property;

    mbev_CapBiriQMetalShock(workP);
    HuPrcEnd();
}

static void ev_CapEffOpenKill(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    void *workP = process->property;

    HuMemDirectFree(workP);
}

void mbev_CapPlayerMoveMinYSet(int playerNo, float minY)
{
    CAPEFFMOVEWORK *workP;
    OMOBJ *obj = ev_CapEffMoveOMObj[playerNo];

    if (obj != NULL) {
        workP = omObjGetDataAs(obj, CAPEFFMOVEWORK);
        workP->minY = minY;
        workP->minYF = FALSE;
    }
}

void mbev_CapPlayerMoveVelSet(int playerNo, float vel, HuVecF *moveDir)
{
    CAPEFFMOVEWORK *workP;
    OMOBJ *obj = ev_CapEffMoveOMObj[playerNo];

    if (obj != NULL) {
        workP = omObjGetDataAs(obj, CAPEFFMOVEWORK);
        workP->vel = vel;
        workP->velocity = *moveDir;
    }
}

BOOL mbev_CapEffCoinMaxYSet(OMOBJ *obj, int coinNo, float maxY)
{
    CAPEFFCOINWORK *workP;
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffCoinOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFCOINWORK);
    workP = &workP[coinNo];
    workP->maxY = maxY;
    return TRUE;
}

BOOL mbev_CapPlayerMoveObjCheck(int playerNo)
{
    OMOBJ *obj = ev_CapEffMoveOMObj[playerNo];
    CAPEFFMOVEWORK *workP;

    if (obj == NULL) {
        return TRUE;
    }
    workP = omObjGetDataAs(obj, CAPEFFMOVEWORK);
    if (workP->state >= 1) {
        return TRUE;
    }
    return FALSE;
}

void mbev_CapEffRayKill(OMOBJ *obj)
{
    int i;
    CAPEFFRAYWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRayOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFRAYWORK);
    ev_CapEffRayOMObj[workP->objIdx] = (OMOBJ *)-1;
}

void mbev_CapEffRayAlphaSet(OMOBJ *obj, float alpha)
{
    CAPEFFRAYWORK *workP;
    int i;
    CAPEFFRAYPARTICLEWORK *particleP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRayOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFRAYWORK);
    particleP = workP->particleP;
    workP->alpha = alpha;
}

void mbev_CapEffRayTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx)
{
    CAPEFFRAYWORK *workP;
    int i;
    CAPEFFRAYPARTICLEWORK *particleP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffRayOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFRAYWORK);
    particleP = workP->particleP;
    if (pos != NULL) {
        Hu3DModelPosSet(workP->modelId, pos->x, pos->y, pos->z);
    }
    if (rot != NULL) {
        Hu3DModelRotSet(workP->modelId, rot->x, rot->y, rot->z);
    }
    if (mtx != NULL) {
        Hu3DModelMtxSet(workP->modelId, mtx);
    }
}

void mbev_CapEffMasuHitKill(OMOBJ *obj)
{
    int i;
    CAPEFFMASUHITWORK *workP;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffMasuHitOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFMASUHITWORK);
    ev_CapEffMasuHitOMObj[workP->objIdx] = (OMOBJ *)-1;
}

void mbev_CapEffMasuHitTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx)
{
    CAPEFFMASUHITWORK *workP;
    int i;

    for (i = 0; i < 8; i++) {
        if (ev_CapEffMasuHitOMObj[i] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFMASUHITWORK);
    if (pos != NULL) {
        Hu3DModelPosSet(workP->modelId, pos->x, pos->y, pos->z);
    }
    if (rot != NULL) {
        Hu3DModelRotSet(workP->modelId, rot->x, rot->y, rot->z);
    }
    if (mtx != NULL) {
        Hu3DModelMtxSet(workP->modelId, mtx);
    }
}

void mbev_CapEffCoinGlowSet(OMOBJ *obj, OMOBJ *glowObj)
{
    CAPEFFCOINWORK *workP;
    int objIdx;
    int i;

    for (objIdx = 0; objIdx < 8; objIdx++) {
        if (ev_CapEffCoinOMObj[objIdx] == obj) {
            break;
        }
    }
    workP = omObjGetDataAs(obj, CAPEFFCOINWORK);
    for (i = 0; i < 128; i++, workP++) {
        workP->glowObj = glowObj;
    }
}

void mbev_CapPlayerRotate(int playerNo, float angle)
{
    mbPlayerRotateStart(playerNo, angle, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
}

void mbev_CapEffColorSet(GXColor *color, int colorNo)
{
    if (colorNo < 0) {
        colorNo *= -1;
    }
    *color = ev_CapsuleRandomColorTbl[colorNo % 7];
}

static void ev_CapComChoiceHook(void)
{
    int key[4];
    int playerNo;
    s16 delay;
    int keyValue;
    int i;
    int padNo;

    key[0] = key[1] = key[2] = key[3] = 0;
    playerNo = GwSystem.turnPlayerNo;
    padNo = GwPlayer[playerNo].padNo;
    delay = GWComKeyDelayGet();
    keyValue = 4;
    key[padNo] = keyValue;
    for (i = 0; i < capsuleChoice; i++) {
        key[padNo] = keyValue;
        HuWinComKeyWait(key[0], key[1], key[2], key[3], delay);
    }
    key[padNo] = PAD_BUTTON_A;
    HuWinComKeyWait(key[0], key[1], key[2], key[3], delay);
}
