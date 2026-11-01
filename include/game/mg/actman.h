#ifndef _MG_ACTMAN_H
#define _MG_ACTMAN_H

#include "game/object.h"
#include "game/hu3d.h"
#include "game/pad.h"
#include "game/mg/colman.h"

typedef struct MgPlayer_s MGPLAYER;
typedef struct MgActor_s MGACTOR;

typedef void (*MGPLAYER_MODE_FUNC)(MGPLAYER *playerP);
typedef void (*MGACTOR_CORRECT_HOOK)(MGACTOR *actorP, int param);

struct MgActor_s {
    s8 no;
    int mdlId;
    u8 attr;
    int param;
    int type;
    HuVecF oldPos;
    HuVecF forceA;
    HuVecF forceB;
    float terminalVelY;
    int correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook;
    HuVecF push;
    HuVecF pos;
    HuVecF vel;
    float rotY;
    float gravity;
    float velY;
    u16 colMesh;
    HuVecF colOfs;
    HuVecF colNorm;
    u32 colGroundAttr;
    HSF_OBJECT *colObj;
    s16 colFace;
};

struct MgPlayer_s {
    s16 playerNo;
    s16 charNo;
    s8 padNo;
    u16 camBit;
    OMOBJ *omObj;
    u32 actionFlag;
    u16 motNo;
    s8 stunType;
    float stunAngle;
    float stunTime;
    float squishTime;
    u16 attr;
    u16 modeAttr;
    u16 vibAttr;
    s16 subMode;
    int mode;
    float timer;
    float squishOldHeight;
    s16 work[4];
    MGACTOR *actor;
    MGPLAYER_MODE_FUNC modeFunc[18];
};

typedef struct MgActorParam_s {
    float height;
    float radius;
    int param;
    int type;
    COL_NARROW_HOOK narrowHook;
    u32 attr;
    int correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook;
} MGACTOR_PARAM;

typedef struct MgActorColMapPoly_s {
    HuVecF pos;
    u32 code;
    int mdlNo;
    HSF_OBJECT *obj;
    int triNo;
} MGACTOR_COLMAP_POLY;

BOOL fn_8007E818(MGPLAYER *playerP);
void fn_8007E888(MGPLAYER *playerP);
BOOL fn_8007E8B4(MGPLAYER *playerP, HuVecF *pos, float velZ, float radius);
BOOL fn_8007EB24(MGACTOR *actorP, HuVecF *pos, float velZ, float radius);
void fn_8007ED60(MGPLAYER *playerP);
void fn_8007EDBC(MGPLAYER *playerP);
BOOL fn_8007EDE8(MGPLAYER *playerP, int mode);
BOOL fn_8007EEBC(MGPLAYER *playerP, int mode);

void fn_800802A8(void);
void fn_8008033C(void);
OMOBJMAN *fn_80080418(void);
void fn_80080520(MGPLAYER *playerP, int stkX, int stkY, int btnDown, int btn);
void fn_800804E8(HU3D_MODELID *mdlId, s16 mdlNum, int bodyMax);
void fn_800805CC(MGACTOR *actorP, u32 mask);
void fn_8008062C(MGACTOR *actorP, u32 mask);
u32 fn_8008068C(MGACTOR *actorP, u32 mask);
void fn_800806DC(MGPLAYER *playerP, MGPLAYER_MODE_FUNC *funcTbl);
MGPLAYER *fn_80080728(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
    u32 actionFlag, unsigned int *motDataNum);
MGPLAYER *fn_800808C0(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
    u32 actionFlag, unsigned int *motDataNum, unsigned int flag);
MGACTOR *fn_80080B04(MGACTOR_PARAM *param, int mdlId);
void fn_80080D7C(int mdlNo, u32 mask);
u32 fn_80080DAC(int mdlNo);
void fn_80080DD4(MGACTOR *actorP, u32 mask);
u32 fn_80080E38(MGACTOR *actorP);
void fn_80080E8C(void);
void fn_80080EBC(void);
void fn_80080EEC(MGACTOR *actorP, float bounce);

void fn_80080F38(MGACTOR *actorP);

void fn_800856A8(void);

BOOL fn_80086910(HuVecF *pos1, HuVecF *pos2, u32 mask, MGACTOR_COLMAP_POLY *outPoly);
void fn_8008697C(MGPLAYER *playerP, HuVecF *pos);
void fn_80086AA8(MGACTOR *actorP, HuVecF *pos);
void fn_80086B04(MGACTOR *actorP, HuVecF *pos);
void fn_80086BC8(MGACTOR *actorP, HuVecF *pos);
void fn_80086C8C(MGACTOR *actorP, HuVecF *push);
void fn_80086CA8(MGACTOR *actorP, float rotY);
void fn_80086CB0(MGACTOR *actorP, float *rotY);
void fn_80086CBC(MGACTOR *actorP, float gravity);
void fn_80086CC4(MGACTOR *actorP, float velY);
void fn_80086CCC(MGACTOR *actorP, HuVecF *vel);
BOOL fn_80086CE8(MGACTOR *actorP, int *mesh);
BOOL fn_80086D54(MGACTOR *actorP, HuVecF *normal);
BOOL fn_80086DD0(MGACTOR *actorP, u32 *code);
void fn_80086E3C(MGPLAYER *playerP);
void fn_80086F00(MGPLAYER *playerP, HuVecF *pos);
void fn_80086F44(MGACTOR *actorP);
void fn_80086FA0(MGACTOR *actorP, HuVecF *pos);

void fn_800870A4(COL_ATTRPARAM *param, u32 polyAttr);
void fn_800870D4(COL_ATTRPARAM *param, u32 polyAttr);

u16 fn_8008717C(MGPLAYER *playerP, u16 attr);
void fn_80087168(MGPLAYER *playerP, u16 attr);
void fn_80087158(MGPLAYER *playerP, u16 attr);

u16 fn_80087114(MGPLAYER *playerP, u16 attr);
void fn_80087104(MGPLAYER *playerP, u16 attr);

u16 fn_80087148(MGPLAYER *playerP, u16 attr);
void fn_80087134(MGPLAYER *playerP, u16 attr);
void fn_80087124(MGPLAYER *playerP, u16 attr);

BOOL fn_8008718C(MGPLAYER *playerP, int stunType, float angle, int maxTime);
BOOL fn_80087218(MGPLAYER *playerP);

#endif
