// Object manager state and interfaces for overlay game objects.
#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include "dolphin.h"
#include "game/process.h"
#include "game/hu3d.h"
#include "game/omovl.h"

#define OM_STAT_DELETED (1 << 0)
#define OM_STAT_DISABLED (1 << 1)
#define OM_STAT_ACTIVE (1 << 2)
#define OM_STAT_PAUSED (1 << 4)
#define OM_STAT_NOPAUSE (1 << 5)
#define OM_STAT_40 (1 << 6)

#define OM_STAT_SPRPAUSE (1 << 7)
#define OM_STAT_MODELPAUSE (1 << 8)

#define OM_GRP_NONE -1

typedef HUPROCESS OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

#define OM_GRP_MAX 10
#define OM_OBJ_NONE -1

typedef struct omObjGrp_s {
    u16 memberNo; // Head of the group's free member-slot chain.
    u16 objMax; // Maximum number of members this group can hold.
    u16 objNum; // Number of objects currently assigned to the group.
    u16 *memberNext; // Free-slot links used to choose the next group member slot.
    OMOBJ **memberList; // Objects currently stored in the group's member slots.
} OMOBJGRP;

typedef struct omObjWork_s {
    s16 objMax; // Capacity of the object array, including the manager's reserved slots.
    s16 objIdx; // Number of objects currently allocated from the object array.
    s16 objNext; // Next free object-array slot.
    s16 objLast; // Last object in the priority-ordered update list, or OM_OBJ_NONE.
    s16 objFirst; // First object in the priority-ordered update list, or OM_OBJ_NONE.
    OMOBJ *objData; // Object slots owned by this manager.
    OMOBJGRP *grpData; // Group membership tables owned by this manager.
} OMOBJWORK;

struct omObj_s {
    u16 stat; // Object-manager state and pause flags.
    s16 objNext; // Index of this object in the manager's object array.
    s16 prio; // Priority used to order the object's update callback.
    s16 prev; // Previous object-array index in the priority-ordered update list.
    s16 next; // Next object-array index in the priority-ordered update list.
    s16 nextNo; // Next free object-array index after this slot is released.
    s16 grpNo; // Group index, or OM_GRP_NONE when the object has no group.
    u16 memberNo; // This object's slot in its group's member list.
    u32 mode; // State value interpreted by the object's callback.
    OMOBJ_FUNC objFunc; // Function called to update this object.
    Vec trans; // Object position in 3D space.
    Vec rot; // Object rotation in 3D space.
    Vec scale; // Object scale on each axis.
    u16 mdlcnt; // Number of model IDs allocated for this object.
    HU3D_MODELID *mdlId; // Model IDs owned by this object.
    u16 mtncnt; // Number of motion IDs allocated for this object.
    HU3D_MOTIONID *mtnId; // Motion IDs owned by this object.
    u32 work[4]; // Four words of scratch state available to the object's callback.
    void *data; // Heap allocation owned by this object, when present.
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

#define OM_OUTVIEW_PRIO 32730

#define omOvlCall(ovl, evtno, stat) omOvlCallEx(ovl, TRUE, evtno, stat)
#define omOvlGoto(ovl, evtno, stat) omOvlGotoEx(ovl, TRUE, evtno, stat)
#define omOvlReturn(hisOfs) omOvlReturnEx(hisOfs, TRUE)
#define omAddObj(objman, prio, mdlcnt, mtncnt, objFunc)                                            \
    omAddObjEx(objman, prio, mdlcnt, mtncnt, OM_GRP_NONE, objFunc)
#define omAddOutViewObj(objman) omAddObj(objman, OM_OUTVIEW_PRIO, 0, 0, omOutView)
#define omDelObj omDelObjEx
#define omMakeGroup omMakeGroupEx
#define omGetGroupMemberList omGetGroupMemberListEx

#define omObjGetWork(obj, type) ((type *)(&((obj)->work[0])))
#define omObjGetDataAs(obj, type) ((type *)((obj)->data))

#define OM_CAMERA_SINGLE (1 << 16)

#define OM_CAMERAMOVE_SIMPLE 0
#define OM_CAMERAMOVE_COS 1
#define OM_CAMERAMOVE_LINEAR 2
#define OM_CAMERAMOVE_COSSIN 2

void omOvlCallEx(OMOVL ovl, s16 unlinkF, s32 evtno, s32 stat);
void omOvlGotoEx(OMOVL ovl, s16 unlinkF, s32 evtno, s32 stat);
void omOvlReturnEx(s16 hisOfs, s16 unlinkF);
void omOvlKill(s16 unlinkF);
void omOvlHisChg(s32 hisOfs, OMOVL ovl, s32 evtno, s32 stat);
OMOVLHIS *omOvlHisGet(s32 hisOfs);
OMOBJMAN *omInitObjMan(s16 objMax, s32 objManPrio);
OMOBJ *omAddObjEx(OMOBJMAN *objMan, s16 prio, u16 mdlcnt, u16 mtncnt, s16 grpNo,
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
char omPauseChk(void);
OMOVL omCurrentOvlGet(void);

void omOutView(OMOBJ *obj);
void omOutViewMulti(OMOBJ *obj);
void omDBGSystemKeyCheckSetup(OMOBJMAN *objman);
void omSystemKeyCheckSetup(OMOBJMAN *objman);
void omSystemKeyCheck(OMOBJ *obj);
void omSysPauseEnable(u8 flag);
void omSysPauseCtrl(s16 flag);
void omCameraViewInit(void);
void omCameraViewSetMulti(s16 cameraBit, OM_CAMERA_VIEW *cameraView);
void omCameraViewSet(OM_CAMERA_VIEW *cameraView);
s16 omCameraViewMoveMulti(u32 camera, OM_CAMERA_VIEW *cameraView, s32 time, s16 moveType);
s16 omCameraViewMove(OM_CAMERA_VIEW *cameraView, s32 time, s16 moveType);
s16 omCameraViewMoveSimpleMulti(u32 camera, OM_CAMERA_VIEW *cameraView, s32 time);
s16 omCameraViewMoveSimple(OM_CAMERA_VIEW *cameraView, s32 time);
BOOL omCameraViewCheck(u32 cameraBit);

void omObjManPause(BOOL pauseF);

void omGameSysInit(OMOBJMAN *objman);
void omVibrate(s16 playerNo, s16 duration, s16 off, s16 on);

extern s16 omSysExitReq;
extern OMOBJ *omDBGSysKeyObj;
extern u8 omSysPauseEnableFlag;
extern OMOVL omprevovl;
extern OMOVL omcurovl;
extern s32 omcurdll;
extern s32 omovlhisidx;
extern s32 omovlevtno;
extern s32 omovlstat;
extern u8 omUPauseFlag;
extern s16 omdispinfo;
extern float CZoomM[HU3D_CAM_MAX];
extern HuVecF CenterM[HU3D_CAM_MAX];
extern HuVecF CRotM[HU3D_CAM_MAX];
extern float CZoom;
extern HuVecF Center;
extern HuVecF CRot;

void omMasterInit(s32 watchPrio, OVLTBL *ovlTbl, OMOVL ovlMax, OMOVL ovlInit);

#endif
