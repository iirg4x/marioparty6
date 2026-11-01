#ifndef _MG_COLMAN_H
#define _MG_COLMAN_H

#include <dolphin.h>
#include "game/hu3d.h"

typedef struct ColNarrowParam_s {
    int paramA;
    int paramB;
    int type;
    HuVecF point;
    HuVecF normPos;
    int colResult;
} COL_NARROW_PARAM;

typedef struct ColBody_s COLBODY;

typedef int (*COL_NARROW_HOOK)(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
typedef void (*COL_CORRECT_HOOK)(COLBODY *body, void *user);

typedef struct ColAttrParam_s {
    int type;
    float speed;
    float yDeviate;
    float maxDot;
    u32 attr;
} COL_ATTRPARAM;

typedef struct ColBodyParam_s {
    float height;
    float radius;
    float bounce;
    int paramA;
    int paramB;
    int type;
    u32 mask;
    COL_NARROW_HOOK narrowHook;
    COL_NARROW_HOOK narrowHook2;
    COL_CORRECT_HOOK colCorrectHook;
    void *user;
    u32 attr;
} COLBODY_PARAM;

typedef struct ColBodyPoint_s {
    HuVecF colOfs;
    HuVecF normal;
    u32 polyAttr;
    u16 meshNo;
    HSF_OBJECT *obj;
    s16 faceNo;
} COLBODY_POINT;

struct ColBody_s {
    COLBODY_PARAM param;
    u32 groundAttr;
    u32 hitAttr;
    HuVecF pos[2];
    s16 colPointNum;
    COLBODY_POINT colPoint[9];
    HuVecF oldPos;
    HuVecF moveDir;
    u32 paramAttr;
    float colT;
    float oldColT;
    s32 colBit[(256+31)/32];
};

COLBODY *fn_8007BAB8(int no);
void fn_8007BAD0(void);
void ColMapInit(HU3D_MODELID *mdlId, s16 mdlNum, int actorMax);
void fn_8007D118(int mdlNo, u32 mask);
u32 fn_8007D18C(int mdlNo);
BOOL fn_8007D200(void);
BOOL fn_8007D260(void);
void fn_8007D2C0(void);
BOOL fn_8007D52C(void);
void fn_8007D534(void);
void fn_8007D550(COL_ATTRPARAM *param, u32 polyAttr);
void fn_8007D618(COL_ATTRPARAM *param, u32 polyAttr);
void fn_8007D6E0(HuVecF *pos, int no);
void fn_8007D7B4(HuVecF *pos, int no);
void fn_8007D84C(COLBODY_PARAM *param, int no);
COLBODY *fn_8007D8E0(int no);
BOOL fn_8007D91C(HuVecF *pos1, HuVecF *pos2, u32 mask, HuVecF *outPos, u32 *outCode,
    int *outMdlNo, HSF_OBJECT **outObj, int *outTriNo);
void fn_8007DDB8(void);

#endif
