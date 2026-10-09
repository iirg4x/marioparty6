// Coordinates overlay transitions and the per-frame game object manager.
#define _MATH_H
#include "game/object.h"
#include "game/objdll.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/audio.h"
#include "game/window.h"
#include "game/gamework.h"
#include "game/main.h"

#include "game/esprite.h"
#include "game/flag.h"

#define OMOVLHIS_MAX 16

//TODO: Use from mgdata.h
extern float MgModeWorkFloat[16];
extern int MgModeWorkInt[16];
//TODO: Use from saveload.h
extern void SLWinInit(void);
//TODO: Use from actman.h
void MgActorClose(void);
//TODO: Use from gamemes.h
void GameMesClose(void);
void MgScoreBoxInit(void);

OMOBJ *omDBGSysKeyObj;
static HUPROCESS *omwatchproc;
static OMOVL omnextovl;
OMOVL omcurovl;
s32 omcurdll;
s32 omovlhisidx;
s32 omovlevtno;
static s32 omnextovlevtno;
s32 omovlstat;
static HUPROCESS *omObjManProc;

static s32 omnextovlstat;
u8 omUPauseFlag;
s16 omSysExitReq;
s16 omdispinfo;

static OMOVLHIS omovlhis[OMOVLHIS_MAX];

u8 omSysPauseEnableFlag = TRUE;
OMOVL omprevovl = DLL_NONE;

static void omWatchOverlayProc(void);

// Called by main.c during system startup to start the overlay watcher and request the boot overlay.
void omMasterInit(s32 overlayWatchPriority, OVLTBL *overlayTable, OMOVL overlayCount,
                  OMOVL initialOverlay)
{
    s16 workIndex;
    omDLLInit(overlayTable);
    omwatchproc = HuPrcCreate(omWatchOverlayProc, overlayWatchPriority, 12288, 0);
    HuPrcSetStat(omwatchproc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    omcurovl = DLL_NONE;
    omovlhisidx = -1;
    omOvlCall(initialOverlay, 0, 0);
    omDBGSysKeyObj = NULL;
    for(workIndex=0; workIndex<16; workIndex++) {
        MgModeWorkInt[workIndex] = MgModeWorkFloat[workIndex] = 0;
    }
    omSysPauseEnable(TRUE);
}

// The process created by omMasterInit waits for requests and initializes overlays after fades.
static void omWatchOverlayProc(void)
{
    while(1) {
        if(omcurovl == DLL_NONE) {
            if(omnextovl >= 0 && fadeStat == FALSE) {
                HuPrcSleep(0);
                OSReport(
                    "++++++++++++++++++++ Start New OVL %d (EVT:%d STAT:0x%08x) ++++++++++++++++++\n",
                    omnextovl, omnextovlevtno, omnextovlstat);
                OSReport("objman>Init esp\n");
                espInit();
                OSReport("objman>Call objectsetup\n");
                HuAudDllSndGrpSet(omnextovl);
                omcurovl = omnextovl;
                omovlevtno = omnextovlevtno;
                omovlstat = omnextovlstat;
                omnextovl = DLL_NONE;
                if(_CheckFlag(FLAG_MG_PRACTICE)) {
                    GameMesPracticeCreate();
                }
                omSysPauseEnable(TRUE);
                MgActorInit();
                omcurdll = omDLLStart(omcurovl, FALSE);
                OSReport("objman>ObjectSetup end\n");
                if(omcurovl != DLL_NONE) {
                    goto watch;
                } else {
                    continue;
                }
            } else {
                HuPrcVSleep();
            }
        } else {
            watch:
            HuPrcChildWatch();
        }
    }
}

// Called by overlay transitions to append the requested overlay to history and schedule it; if the
// history index is already at least OMOVLHIS_MAX, logs an error and skips both.
void omOvlCallEx(OMOVL overlay, s16 unlinkObjects, s32 eventNo, s32 overlayStatus)
{
    OSReport("objman>Call New Ovl %d(%d)\n", overlay, unlinkObjects);
    if(omovlhisidx >= OMOVLHIS_MAX) {
        OSReport("objman>OVL Call over error\n");
    } else {
        omovlhis[++omovlhisidx].ovl = overlay;
        omovlhis[omovlhisidx].evtno = eventNo;
        omovlhis[omovlhisidx].stat = overlayStatus;
        omOvlGotoEx(overlay, unlinkObjects, eventNo, overlayStatus);
    }
}

// Called by overlay transitions to schedule an overlay, closing the current one if active.
void omOvlGotoEx(OMOVL overlay, s16 unlinkObjects, s32 eventNo, s32 overlayStatus)
{
    omprevovl = omcurovl;
    if(omcurovl >= 0) {
        omOvlKill(unlinkObjects);
    }
    omnextovl = overlay;
    omnextovlevtno = eventNo;
    omnextovlstat = overlayStatus;
}

// Called by overlay code to return to history unless another overlay request is already queued.
void omOvlReturnEx(s16 historyOffset, s16 unlinkObjects)
{
    if(omnextovl >= 0) {
        return;
    }
    omovlhisidx -= historyOffset;
    OSReport("objman>Ovl Return %d=%d(%d)\n", historyOffset, omovlhisidx, unlinkObjects);
    if(omovlhisidx < 0) {
        OSReport("objman>OVL under error\n");
        omovlhisidx = 0;
    }
    omOvlGotoEx(omovlhis[omovlhisidx].ovl, unlinkObjects, omovlhis[omovlhisidx].evtno,
                omovlhis[omovlhisidx].stat);
}

// Called during an overlay transition to release overlay-owned state and end its DLL.
void omOvlKill(s16 unlinkObjects)
{
    MgActorClose();
    CharModelKill(GW_CHARA_NULL);
    GameMesClose();
    Hu3DAllKill();
    HuWinAllKill();
    HuSprClose();
    HuPrcChildKill(omwatchproc);
    HuMemDirectFreeNum(HEAP_HEAP, HU_MEMNUM_OVL);
    HuDataDirCloseNum(HU_MEMNUM_OVL);
    HuMemDirectFreeNum(HEAP_DVD, HU_MEMNUM_OVL);
    HuMemDirectFreeNum(HEAP_MODEL, HU_MEMNUM_OVL);
    SLWinInit();
    HuPadRumbleAllStop();
    HuAudFXListnerKill();
    OSReport("OvlKill %d\n", unlinkObjects);
    omSysExitReq = FALSE;
    omDLLNumEnd(omcurovl, unlinkObjects);
    omcurovl = DLL_NONE;
    omDBGSysKeyObj = NULL;
}

// Called by overlay code to replace a return-history entry when its offset is in range.
void omOvlHisChg(s32 historyOffset, OMOVL overlay, s32 eventNo, s32 overlayStatus)
{
    OMOVLHIS *historyEntry;
    if(omovlhisidx-historyOffset < 0 || omovlhisidx-historyOffset >= OMOVLHIS_MAX) {
        OSReport("objman> omOvlHisChg: overlay 実行履歴の範囲外を変更しようとしました\n");
        return;
    }
    historyEntry = &omovlhis[omovlhisidx-historyOffset];
    historyEntry->ovl = overlay;
    historyEntry->evtno = eventNo;
    historyEntry->stat = overlayStatus;
}

// Called by overlay code to read a return-history entry, or NULL when its offset is out of range.
OMOVLHIS *omOvlHisGet(s32 historyOffset)
{
    if(omovlhisidx-historyOffset < 0 || omovlhisidx-historyOffset >= OMOVLHIS_MAX) {
        OSReport("objman> omOvlHisGet: overlay 実行履歴の範囲外を参照しようとしました\n");
        return NULL;
    }
    return &omovlhis[omovlhisidx-historyOffset];
}

static void omMain(void);

static void omDestroyObjMan(void);

// Called by scene managers to allocate an object manager and start its frame process.
OMOBJMAN *omInitObjMan(s16 maxObjectCount, s32 objectManagerPriority)
{
    OMOBJGRP *groupData;
    OMOBJ *objectData;
    OMOBJWORK *objectWork;
    OMOBJMAN *objectManager;
    s32 objectIndex;
    OSReport("objman>InitObjMan start\n");
    // Reserve five extra object slots beyond the count requested by the caller.
    maxObjectCount += 5;
    omSysExitReq = FALSE;
    omObjManProc = objectManager =
        HuPrcChildCreate(omMain, objectManagerPriority, 24576, 0, omwatchproc);
    HuPrcSetStat(objectManager, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    objectWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(OMOBJWORK), HU_MEMNUM_OVL);
    objectWork->objMax = maxObjectCount;
    objectManager->property = objectWork;
    objectManager->destructor = omDestroyObjMan;
    objectWork->objIdx = 0;
    objectWork->objNext = 0;
    objectWork->objLast = OM_OBJ_NONE;
    objectWork->objFirst = OM_OBJ_NONE;
    objectWork->objData = objectData =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(OMOBJ) * maxObjectCount, HU_MEMNUM_OVL);
    objectWork->grpData = groupData =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(OMOBJGRP) * OM_GRP_MAX, HU_MEMNUM_OVL);
    for(objectIndex=0; objectIndex<maxObjectCount; objectIndex++) {
        OMOBJ *object = &objectData[objectIndex];
        object->stat = OM_STAT_DELETED;
        object->prio = object->prev = object->next = OM_OBJ_NONE;
        object->mode = 0;
        object->trans.x = object->trans.y = object->trans.z = object->rot.x = object->rot.y =
            object->rot.z = 0;
        object->scale.x = object->scale.y = object->scale.z = 1;
        object->mdlId = object->mtnId = NULL;
        object->objFunc = object->data = NULL;
        object->nextNo = objectIndex+1;
        object->mtncnt = 0;
        // Reset the motion ID pointer after initializing this object slot.
        object->mtnId = NULL;
    }
    for(objectIndex=0; objectIndex<OM_GRP_MAX; objectIndex++) {
        groupData[objectIndex].objMax = 0;
        groupData[objectIndex].objNum = 0;
        groupData[objectIndex].memberNo = 0;
        groupData[objectIndex].memberList = NULL;
        groupData[objectIndex].memberNext = NULL;
    }
    OSReport("objman>InitObjMan end\n");
    omUPauseFlag = FALSE;
    HuPrcAllUPause(0);
    omCameraViewInit();
    MgScoreBoxInit();
    return objectManager;
}

// Process destructor installed by omInitObjMan; clears the active-list tail on shutdown.
static void omDestroyObjMan(void)
{
    OMOBJMAN *objectManager = HuPrcCurrentGet();
    OMOBJWORK *objectWork = objectManager->property;
    objectWork->objLast = OM_OBJ_NONE;
    OSReport("objman>Destory ObjMan\n");
}

static void omInsertObj(OMOBJMAN *objectManager, OMOBJ *object);

// Called by gameplay systems to allocate an object slot and register its frame callback.
OMOBJ *omAddObjEx(OMOBJMAN *objectManager, s16 priority, u16 modelCount, u16 motionCount,
                  s16 groupNumber, OMOBJ_FUNC objectCallback)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJ *objectData = objectWork->objData;
    OMOBJ *object;
    s16 nextObjectIndex;
    s32 arrayIndex;
    if(objectWork->objIdx == objectWork->objMax) {
        OSReport("Error: ObjMax Over!\n");
        return NULL;
    }
    nextObjectIndex = objectWork->objNext;
    object = &objectData[nextObjectIndex];
    object->objNext = nextObjectIndex;
    object->prio = priority;
    omInsertObj(objectManager, object);
    if(modelCount) {
        object->mdlId =
            HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3D_MODELID) * modelCount, HU_MEMNUM_OVL);
        object->mdlcnt = modelCount;
        for(arrayIndex=0; arrayIndex<modelCount; arrayIndex++) {
            object->mdlId[arrayIndex] = HU3D_MODELID_NONE;
        }
    } else {
        object->mdlId = NULL;
        object->mdlcnt = 0;
    }
    if(motionCount) {
        object->mtnId =
            HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3D_MODELID) * motionCount, HU_MEMNUM_OVL);
        object->mtncnt = motionCount;
    } else {
        object->mtnId = NULL;
        object->mtncnt = 0;
    }
    if(groupNumber >= 0) {
        omAddMember(objectManager, groupNumber, object);
    } else {
        object->grpNo = groupNumber;
        object->memberNo = 0;
    }
    object->stat = OM_STAT_ACTIVE;
    object->mode = 0;
    object->objFunc = objectCallback;
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    objectWork->objNext = object->nextNo;
    objectWork->objIdx++;
    omSetTra(object, 0.0f, 0.0f, 0.0f);
    omSetRot(object, 0.0f, 0.0f, 0.0f);
    omSetSca(object, 1.0f, 1.0f, 1.0f);
    return object;
}

// Helper called by omAddObjEx; inserts before existing objects with an equal or lower priority.
static void omInsertObj(OMOBJMAN *objectManager, OMOBJ *object)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJ *objectData = objectWork->objData;
    s16 objectIndex = object->objNext;
    s16 priority = object->prio;
    s16 nextObjectIndex;
    s16 previousObjectIndex;
    OMOBJ *nextObject;
    if(objectWork->objFirst == OM_OBJ_NONE) {
        object->prev = OM_OBJ_NONE;
        object->next = OM_OBJ_NONE;
        objectWork->objFirst = objectIndex;
        objectWork->objLast = objectIndex;
        return;
    }
    for (nextObjectIndex = objectWork->objFirst; nextObjectIndex != OM_OBJ_NONE;
         nextObjectIndex = nextObject->next) {
        nextObject = &objectData[nextObjectIndex];
        if(nextObject->prio <= priority) {
            break;
        }
        previousObjectIndex = nextObjectIndex;
    }
    if(nextObjectIndex != OM_OBJ_NONE) {
        object->prev = nextObject->prev;
        object->next = nextObjectIndex;
        if(nextObject->prev != OM_OBJ_NONE) {
            objectData[nextObject->prev].next = objectIndex;
        } else {
            objectWork->objFirst = objectIndex;
        }
        nextObject->prev = objectIndex;
    } else {
        object->next = OM_OBJ_NONE;
        object->prev = previousObjectIndex;
        nextObject->next = objectIndex;
        objectWork->objLast = objectIndex;
    }
}

// Called by omAddObjEx to add an object when its group has a free member slot; full groups skip it.
void omAddMember(OMOBJMAN *objectManager, u16 groupNumber, OMOBJ *object)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJGRP *group = &objectWork->grpData[groupNumber];
    if(group->objNum != group->objMax) {
        object->grpNo = groupNumber;
        object->memberNo = group->memberNo;
        group->memberList[group->memberNo] = object;
        group->memberNo = group->memberNext[group->memberNo];
        group->objNum++;
    }
}

// Called by object owners to free an object and unlink it from its group and priority list.
void omDelObjEx(OMOBJMAN *objectManager, OMOBJ *object)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJ *objectData = objectWork->objData;
    s16 objectIndex = object->objNext;
    if(objectWork->objIdx == 0 || object->stat == OM_STAT_DELETED) {
        return;
    }
    objectWork->objIdx--;
    if(object->grpNo >= 0) {
        omDelMember(objectManager, object);
    }
    if(object->mtnId != NULL) {
        HuMemDirectFree(object->mtnId);
        object->mtnId = NULL;
    }
    if(object->mdlId != NULL) {
        HuMemDirectFree(object->mdlId);
        object->mdlId = NULL;
    }
    if(object->data != NULL) {
        HuMemDirectFree(object->data);
        object->data = NULL;
    }
    object->stat = OM_STAT_DELETED;
    if(object->next >= 0) {
        objectData[object->next].prev = object->prev;
    }
    if(object->prev >= 0) {
        objectData[object->prev].next = object->next;
    }
    if(objectWork->objIdx) {
        if(object->prev < 0) {
            objectWork->objFirst = objectData[object->next].objNext;
        }
        if(object->next < 0) {
            objectWork->objLast = objectData[object->prev].objNext;
        }
    } else {
        objectWork->objFirst = objectWork->objLast = OM_OBJ_NONE;
    }
    object->nextNo = objectWork->objNext;
    objectWork->objNext = objectIndex;
}

// Removes the object from its group when omDelObjEx deletes it.
void omDelMember(OMOBJMAN *objectManager, OMOBJ *object)
{
    if(object->grpNo != OM_GRP_NONE) {
        OMOBJWORK *objectWork = objectManager->property;
        OMOBJ *objectData = objectWork->objData;
        OMOBJGRP *group = &objectWork->grpData[object->grpNo];
        group->memberList[object->memberNo] = NULL;
        group->memberNext[object->memberNo] = group->memberNo;
        group->memberNo = object->memberNo;
        object->grpNo = OM_GRP_NONE;
        group->objNum--;
    }
}

// Called during scene setup to allocate a group's member slots and initialize the available-slot
// chain.
void omMakeGroupEx(OMOBJMAN *objectManager, u16 groupNumber, u16 maxMemberCount)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJGRP *group = &objectWork->grpData[groupNumber];
    s32 memberIndex;
    if(group->memberList != NULL) {
        HuMemDirectFree(group->memberList);
    }
    if(group->memberNext != NULL) {
        HuMemDirectFree(group->memberNext);
    }
    group->memberNo = 0;
    group->objMax = maxMemberCount;
    group->objNum = 0;
    group->memberList =
        HuMemDirectMallocNum(HEAP_HEAP, maxMemberCount * sizeof(OMOBJ *), HU_MEMNUM_OVL);
    group->memberNext = HuMemDirectMallocNum(HEAP_HEAP, maxMemberCount*sizeof(u16), HU_MEMNUM_OVL);
    for(memberIndex=0; memberIndex<maxMemberCount; memberIndex++) {
        group->memberList[memberIndex] = NULL;
        // The final slot links to maxMemberCount, which terminates the available-slot chain.
        group->memberNext[memberIndex] = memberIndex+1;
    }
}

// Called by group users to retrieve the group's member-pointer array.
OMOBJ **omGetGroupMemberListEx(OMOBJMAN *objectManager, s16 groupNumber)
{
    OMOBJWORK *objectWork = objectManager->property;
    return objectWork->grpData[groupNumber].memberList;
}

// Called by object controls to add status bits to an object's flags.
void omSetStatBit(OMOBJ *object, u16 statusBits)
{
    object->stat |= statusBits;
}

// Called by object controls to clear status bits from an object's flags.
void omResetStatBit(OMOBJ *object, u16 statusBits)
{
    object->stat &= ~statusBits;
}

// Called by object controls to set the object's position components in game units.
void omSetTra(OMOBJ *object, float x, float y, float z)
{
    object->trans.x = x;
    object->trans.y = y;
    object->trans.z = z;
}

// Called by object controls to set the object's rotation components.
void omSetRot(OMOBJ *object, float x, float y, float z)
{
    object->rot.x = x;
    object->rot.y = y;
    object->rot.z = z;
}

// Called by object controls to set the object's scale components.
void omSetSca(OMOBJ *object, float x, float y, float z)
{
    object->scale.x = x;
    object->scale.y = y;
    object->scale.z = z;
}

#define BLACK_SHADOW "\xFD\x01"

// Child process created by omInitObjMan; invokes eligible callbacks and updates the first valid
// model's transforms unless model updates are paused.
static void omMain(void)
{
    OMOBJMAN *objMan = HuPrcCurrentGet();
    OMOBJWORK *objWork = objMan->property;
    OMOBJ *objData = objWork->objData;
    s16 objectIndex;
    omDLLDBGOut();
    while(1) {
        if(omdispinfo) {
            float debugScale = 1.5f;
            GXColor debugPanelColor;
            debugPanelColor.a = 96;
            debugPanelColor.r = 0;
            debugPanelColor.g = 0;
            debugPanelColor.b = 255;
            printWin(15, 31, 128*debugScale, 48*debugScale, &debugPanelColor);
            fontcolor = FONT_COLOR_YELLOW;
            print8(16, 32, debugScale, BLACK_SHADOW "H:%08lX(%ld)",
                   HuMemUsedMallocSizeGet(HEAP_HEAP), HuMemUsedMallocBlockGet(HEAP_HEAP));
            print8(16, 32 + (8 * debugScale), debugScale, BLACK_SHADOW "M:%08lX(%ld)",
                   HuMemUsedMallocSizeGet(HEAP_MODEL), HuMemUsedMallocBlockGet(HEAP_MODEL));
            print8(16, 32 + (16 * debugScale), debugScale, BLACK_SHADOW "OBJ:%d/%d",
                   objWork->objIdx, objWork->objMax);
            print8(16, 32 + (24 * debugScale), debugScale, BLACK_SHADOW "OVL:%ld(%ld<%ld)",
                   omovlhisidx, omcurovl, omprevovl);
            print8(16, 32 + (32 * debugScale), debugScale, BLACK_SHADOW "POL:%ld(%d)",
                   totalPolyCnted, totalMatCnted);
            print8(16, 32 + (40 * debugScale), debugScale, BLACK_SHADOW "D:%08lX(%ld)",
                   HuMemUsedMallocSizeGet(HEAP_DVD), HuMemUsedMallocBlockGet(HEAP_DVD));
        }
        if(HuLoadProcModeGet()) {
            HuPrcVSleep();
            continue;
        }
        objectIndex = objWork->objLast;
        while(objectIndex != OM_OBJ_NONE) {
            OMOBJ *object = &objData[objectIndex];
            objectIndex = object->prev;
            if((object->stat & (OM_STAT_DELETED|OM_STAT_DISABLED)) == 0) {
                if (object->objFunc != NULL &&
                    (object->stat & (OM_STAT_40 | 0x8 | OM_STAT_PAUSED)) == 0) {
                    object->objFunc(object);
                }
                if(omcurovl == DLL_NONE || objWork->objLast == OM_OBJ_NONE) {
                    break;
                }
                if((object->stat & (OM_STAT_DELETED|OM_STAT_DISABLED)) == 0) {
                    // A callback may remove the next list entry; resume from this object's updated
                    // link.
                    if((objData[objectIndex].stat & (OM_STAT_DELETED|OM_STAT_DISABLED)) != 0) {
                        objectIndex = object->prev;
                    }
                    if (object->mdlId != NULL && object->mdlId[0] != HU3D_MODELID_NONE &&
                        !(object->stat & OM_STAT_MODELPAUSE)) {
                        Hu3DModelPosSet(object->mdlId[0], object->trans.x, object->trans.y,
                                        object->trans.z);
                        Hu3DModelRotSet(object->mdlId[0], object->rot.x, object->rot.y,
                                        object->rot.z);
                        Hu3DModelScaleSet(object->mdlId[0], object->scale.x, object->scale.y,
                                          object->scale.z);
                    }
                }
            }
        }
        HuPrcVSleep();
    }
}

// Prints every object slot's state and transforms when the manager dump helper is invoked.
static void omDumpObj(OMOBJMAN *objectManager)
{
    OMOBJWORK *objectWork = objectManager->property;
    OMOBJ *objectData = objectWork->objData;
    s32 objectIndex;
    OSReport("=================== 現在登録されている OBJECT ==================\n");
    OSReport(
        "STAT PRI GRPN MEMN PROG (TRA) (ROT) (SCA) mdlcnt mtncnt work[0] work[1] work[2] work[3] *data\n");
    for(objectIndex=0; objectIndex<objectWork->objMax; objectIndex++) {
        OMOBJ *object = &objectData[objectIndex];
        OSReport(
            "%04d:%04X %04X %d %d %08X (%.2f %.2f %.2f) (%.2f %.2f %.2f) (%.2f %.2f %.2f) %d %d %08X %08X %08X %08X %08X\n",
            object->stat, object->stat, object->prio, object->grpNo, object->mode, object->objFunc,
            object->trans.x, object->trans.y, object->trans.z, object->rot.x, object->rot.y,
            object->rot.z, object->scale.x, object->scale.y, object->scale.z, object->mdlcnt,
            object->mtncnt, object->work[0], object->work[1], object->work[2], object->work[3],
            object->data);
    }
    OSReport("================================================================\n");
}

// Called by omSystemKeyCheck to pause or resume eligible object callbacks, including scripted
// requests.
void omAllPause(BOOL pauseFlag)
{
    OMOBJMAN *objectManager = HuPrcCurrentGet();
    OMOBJWORK *objectWork = objectManager->property;
    s32 objectIndex;
    if(pauseFlag) {
        for(objectIndex=0; objectIndex<objectWork->objMax; objectIndex++) {
            if((objectWork->objData[objectIndex].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omSetStatBit(&objectWork->objData[objectIndex], OM_STAT_PAUSED);
            }
        }
    } else {
        for(objectIndex=0; objectIndex<objectWork->objMax; objectIndex++) {
            if((objectWork->objData[objectIndex].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omResetStatBit(&objectWork->objData[objectIndex], OM_STAT_PAUSED);
            }
        }
    }
}

// Called by the memory-card device message flow to suspend and restore gameplay object callbacks.
void omObjManPause(BOOL pauseFlag)
{
    OMOBJMAN *objectManager = omObjManProc;
    OMOBJWORK *objectWork = objectManager->property;
    s32 objectIndex;
    if(pauseFlag) {
        for(objectIndex=0; objectIndex<objectWork->objMax; objectIndex++) {
            if((objectWork->objData[objectIndex].stat & (OM_STAT_DELETED|OM_STAT_SPRPAUSE)) == 0) {
                omSetStatBit(&objectWork->objData[objectIndex], OM_STAT_40);
            }
        }
    } else {
        for(objectIndex=0; objectIndex<objectWork->objMax; objectIndex++) {
            if((objectWork->objData[objectIndex].stat & (OM_STAT_DELETED|OM_STAT_SPRPAUSE)) == 0) {
                omResetStatBit(&objectWork->objData[objectIndex], OM_STAT_40);
            }
        }
    }
}

// Returns the saved system-key pause bit; MCDeviceMesExec reads it before pausing other processes.
char omPauseChk(void)
{
    if(omDBGSysKeyObj == NULL) {
        return 0;
    } else {
        return omDBGSysKeyObj->work[0] & 0x1;
    }
}

// Returns the currently active overlay ID.
OMOVL omCurrentOvlGet(void)
{
    return omcurovl;
}
