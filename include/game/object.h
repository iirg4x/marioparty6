#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include "dolphin.h"
#include "game/process.h"
#include "game/hu3d.h"
#include "game/omovl.h"

typedef HUPROCESS OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

typedef struct omObjGrp_s {
    u16 memberNo;
    u16 objMax;
    u16 objNum;
    u16 *memberNext;
    OMOBJ **memberList;
} OMOBJGRP;

typedef struct omObjWork_s {
    s16 objMax;
    s16 objIdx;
    s16 objNext;
    s16 objLast;
    s16 objFirst;
    OMOBJ *objData;
    OMOBJGRP *grpData;
} OMOBJWORK;

struct omObj_s {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    Vec trans;
    Vec rot;
    Vec scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

typedef struct omOvlHis_s {
    OMOVL ovl;
    s32 evtno;
    s32 stat;
} OMOVLHIS;

typedef struct ovlTbl_s {
    char *name;
    s32 entryNum;
} OVLTBL;

typedef struct omCameraView_s {
    HuVecF center;
    HuVecF rot;
    float zoom;
} OM_CAMERA_VIEW;

#define omOvlCall(ovl, evtno, stat) omOvlCallEx(ovl, TRUE, evtno, stat)

void omOvlCallEx(OMOVL ovl, s16 unlinkF, s32 evtno, s32 stat);
void omOvlGotoEx(OMOVL ovl, s16 unlinkF, s32 evtno, s32 stat);
void fn_8003A5E4(s16 hisOfs, s16 unlinkF);
void fn_8003A6E4(s16 unlinkF);
void omOvlHisChg(s32 hisOfs, OMOVL ovl, s32 evtno, s32 stat);
OMOVLHIS *omOvlHisGet(s32 hisOfs);
OMOBJMAN *fn_8003A8BC(s16 objMax, s32 objManPrio);
OMOBJ *fn_8003AB38(OMOBJMAN *objMan, s16 prio, u16 mdlcnt, u16 mtncnt, s16 grpNo,
    OMOBJ_FUNC objFunc);
void omAddMember(OMOBJMAN *objMan, u16 grpNo, OMOBJ *obj);
void omDelObjEx(OMOBJMAN *objMan, OMOBJ *obj);
void omDelMember(OMOBJMAN *objMan, OMOBJ *obj);
void omMakeGroupEx(OMOBJMAN *objMan, u16 grpNo, u16 objMax);
OMOBJ **omGetGroupMemberListEx(OMOBJMAN *objMan, s16 grpNo);
void omSetStatBit(OMOBJ *obj, u16 bit);
void omResetStatBit(OMOBJ *obj, u16 bit);

void omSetTra(OMOBJ *obj, float x, float y, float z);
void omSetRot(OMOBJ *obj, float x, float y, float z);
void omSetSca(OMOBJ *obj, float x, float y, float z);

void omAllPause(BOOL pauseF);
char fn_8003B748(void);
OMOVL fn_8003B774(void);

void fn_8003D4FC(OMOBJ *obj);
void fn_8003DAA4(OMOBJ *obj);
void fn_8003E12C(OMOBJMAN *objman);
void omSystemKeyCheckSetup(OMOBJMAN *objman);
void fn_8003E1A4(OMOBJ *obj);
void omSysPauseEnable(u8 flag);
void fn_8003E3F0(s16 flag);
void fn_8003E440(void);
void fn_8003E488(s16 cameraBit, OM_CAMERA_VIEW *cameraView);
void fn_8003E5A4(OM_CAMERA_VIEW *cameraView);
s16 fn_8003E62C(u32 camera, OM_CAMERA_VIEW *cameraView, s32 time, s16 moveType);
s16 fn_8003E7A0(OM_CAMERA_VIEW *cameraView, s32 time, s16 moveType);
s16 fn_8003E8E4(u32 camera, OM_CAMERA_VIEW *cameraView, s32 time);
s16 fn_8003EA54(OM_CAMERA_VIEW *cameraView, s32 time);
BOOL fn_8003EB94(u32 cameraBit);

void fn_8003B674(BOOL pauseF);

void fn_80069F1C(OMOBJMAN *objman);
void fn_8006A030(s16 playerNo, s16 duration, s16 off, s16 on);

extern s16 omSysExitReq;
extern OMOBJ *lbl_802C0260;
extern u8 omSysPauseEnableFlag;
extern OMOVL omprevovl;
extern OMOVL omcurovl;
extern s32 lbl_802C0250;
extern s32 lbl_802C024C;
extern s32 lbl_802C0248;
extern s32 lbl_802C0240;
extern u8 lbl_802C0234;
extern s16 lbl_802C0230;
extern float lbl_80266BD8[16];
extern HuVecF lbl_80266B18[16];
extern HuVecF lbl_80266A58[16];
extern float lbl_802C028C;
extern HuVecF lbl_80266A4C;
extern HuVecF lbl_80266A40;

void fn_8003A2A4(s32 watchPrio, OVLTBL *ovlTbl, OMOVL ovlMax, OMOVL ovlInit);

#endif
