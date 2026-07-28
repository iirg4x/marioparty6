#include "game/board/main.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/window.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/sprite.h"

typedef void (*MBSCROLLHOOK)(BOOL enterF);
typedef s16 (*MBSCROLLSTARFINDFUNC)(int playerNo);

typedef struct MapSprWork_s {
    int used;
    int dispF;
    s16 sprId[2];
    int flags;
    int type;
    int masuId;
    GXColor color;
    s16 arrowSprId[1];
    HuVecF pos2D;
    HuVecF pos;
    HuVecF colPos;
} MAPSPRWORK;

typedef struct ScrollWork_s {
    int mapSprNum;
    int playerPosNo;
    int mapFrame;
    int pathFrame;
    float mapPathScale;
    MAPSPRWORK mapSpr[32];
} SCROLLWORK;

static HuVecF scrollPos;
static HuVecF mapViewPos;
static HuVecF mapViewRot;

static SCROLLWORK scrollWork;

static SCROLLWORK *scrollWorkP = &scrollWork;

static MBSCROLLHOOK mapHook;
static ANIMDATA *pathAnim;
static ANIMDATA *masuMapAnim;
static MBSCROLLHOOK scrollHook;
static HSF_FACE *scrollColTriData;
static int scrollColTriNum;
static MBSCROLLSTARFINDFUNC scrollStarFindFunc;
static HU3D_MODELID scrollColModel;
static int lbl_802C0DD8;
static float mapViewZoom;

static void ScrollCreate(u32 dataNum);
static void ScrollKill(void);
static BOOL ScrollMain(int playerNo);
static BOOL ScrollExec(int playerNo, s16 starMasuId);
static void RotateScrollView(HuVecF *rot, HuVecF *pos, HuVecF *posOut);
static BOOL CheckScrollCol(HuVecF *target, HuVecF *dir, HuVecF *endPos);
static s16 StarMasuGet(int playerNo);
static void InitScrollCol(void);
static void ResolveScrollCol(HuVecF *dir, HuVecF *pos1, HuVecF *pos2, HuVecF *endPos);
static void MapViewCreate(void);
static void MapViewKill(void);
static BOOL MapViewExec(int playerNo);
static void MapSprCreate(int type, int id, int layer);
static void MapBaseSprCreate(void);
static BOOL MapSprPlayerCol(void);
static void MapSprPlayerColAll(void);
static void MapSprKill(void);

extern void mbWipeDissolveFadeOut(void);
extern void mbWipeDissolveFadeIn(void);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeSpecialFadeInCreate(int type, BOOL pauseF);
extern void *mbMalloc(s32 size);
extern s8 mbPadStkXGet(int padNo);
extern s8 mbPadStkYGet(int padNo);

void mbScrollInit(int dataNum)
{
    ScrollCreate(mbObjDataNumGet(dataNum));
    MapViewCreate();
}

void mbScrollClose(void)
{
    ScrollKill();
    MapViewKill();
}

void mbev_Scroll(int playerNo, BOOL mapF)
{
    int cameraStackNo;
    BOOL result;
    BOOL pauseDisableF;

    pauseDisableF = mbPauseDisableGet();
    mbPauseDisableSet(TRUE);
    mbStatusDispBackup();
    mbStatusMasuDispSet(FALSE);
    mbMusParamSet(MB_MUS_CHAN_BG, 96, 500);
    cameraStackNo = mbCameraStackPush();
    lbl_802C0DD8 = 0;
    while (TRUE) {
        if (mapF == FALSE) {
            result = ScrollMain(playerNo);
            if (result == FALSE) {
                break;
            }
            mapF = TRUE;
        } else {
            result = MapViewExec(playerNo);
            if (result == FALSE) {
                break;
            }
            mapF = FALSE;
        }
        mbAudFXPlay(1);
    }
    mbAudFXPlay(3);
    mbMusParamSet(MB_MUS_CHAN_BG, 127, 100);
    mbCameraStackIdxSet(cameraStackNo, -1);
    mbStatusDispRestoreForce();
    mbStatusMasuDispSet(TRUE);
    mbWipeDissolveFadeIn();
    mbPauseDisableSet(pauseDisableF);
}

static void ScrollCreate(u32 dataNum)
{
    if (dataNum == 0) {
        scrollColModel = HU3D_MODELID_NONE;
        scrollColTriNum = 0;
        scrollColTriData = NULL;
    } else {
        scrollColModel = Hu3DModelCreate(HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HEAP_MODEL));
        InitScrollCol();
        Hu3DModelDispOff(scrollColModel);
    }
    scrollStarFindFunc = StarMasuGet;
    scrollHook = NULL;
}

static void ScrollKill(void)
{
    HSF_FACE *triData;

    if (scrollColModel >= 0) {
        Hu3DModelKill(scrollColModel);
        scrollColModel = HU3D_MODELID_NONE;
    }
    if (scrollColTriData) {
        triData = scrollColTriData;
        HuMemDirectFree(triData);
        scrollColTriData = NULL;
    }
}

static BOOL ScrollMain(int playerNo)
{
    BOOL result;
    s16 winNo;
    s16 starMasuId;

    if (mbWipeSpecialStatGet() == FALSE) {
        mbWipeDissolveFadeOut();
        mbStatusDispForceSetAll(FALSE);
    }
    if (scrollHook) {
        scrollHook(TRUE);
    }
    if (scrollStarFindFunc) {
        starMasuId = scrollStarFindFunc(playerNo);
    } else {
        starMasuId = 0;
    }
    if (GWPartyGet() != FALSE) {
        if (starMasuId > 0) {
            winNo = mbWinCreateHelp(0x0026000B);
        } else {
            winNo = mbWinCreateHelp(0x00260005);
        }
    } else {
        winNo = mbWinCreateHelp(0x0026000F);
    }
    result = ScrollExec(playerNo, starMasuId);
    if (result == FALSE) {
        mbWipeDissolveFadeOut();
    } else {
        mbWipeSpecialFadeInCreate(4, TRUE);
    }
    mbWinKill(winNo);
    if (scrollHook) {
        scrollHook(FALSE);
    }
    return result;
}

static BOOL ScrollExec(int playerNo, s16 starMasuId)
{
    int mode;
    s8 padNo;
    BOOL result;
    float stkX;
    float stkY;
    float speed;
    float maxSpeed;
    HuVecF cameraPos;
    HuVecF starPos;
    HuVecF savedPos;
    HuVecF scrollDir;
    HuVecF rot;
    HuVecF dir;

    result = FALSE;
    padNo = GwPlayer[playerNo].padNo;
    mode = 0;
    rot.x = -45.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    mbPlayerPosGet(playerNo, &scrollPos);
    RotateScrollView(&rot, &scrollPos, &scrollPos);
    dir.x = HuSin(rot.y) * HuCos(rot.x);
    dir.y = -HuSin(rot.x);
    dir.z = HuCos(rot.y) * HuCos(rot.x);
    PSVECScale(&dir, &dir, 100.0f);
    if (!CheckScrollCol(&scrollPos, &dir, &cameraPos)) {
        ResolveScrollCol(&dir, &scrollPos, &scrollPos, &cameraPos);
    }
    if (starMasuId > 0) {
        mbMasuPosGet(starMasuId, &starPos);
        RotateScrollView(&rot, &starPos, &starPos);
    }
    mbCameraMovePos(&cameraPos, &rot, NULL, 1500.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbWipeDissolveFadeIn();
    while (TRUE) {
        u16 btn = HuPadBtnDown[padNo];

        if (btn & PAD_BUTTON_B) {
            break;
        }
        if ((btn & PAD_BUTTON_Y) && GWPartyGet()) {
            result = TRUE;
            break;
        }
        switch (mode) {
        case 0:
            maxSpeed = 30.0f;
            if (HuPadBtn[padNo] & PAD_BUTTON_A) {
                maxSpeed *= 2.0f;
            }
            stkX = mbPadStkXGet(padNo);
            stkY = -(float)mbPadStkYGet(padNo);
            speed = HuMagPoint2D(stkX, stkY);
            if (speed > 0.0f) {
                stkX /= speed;
                stkY /= speed;
                scrollPos.x += stkX * maxSpeed;
                scrollPos.z += stkY * maxSpeed;
            }
            if ((HuPadBtn[padNo] & PAD_TRIGGER_R) && starMasuId > 0) {
                mode = 1;
                savedPos = scrollPos;
            }
            break;
        case 1:
            if (HuPadBtn[padNo] & PAD_TRIGGER_R) {
                PSVECSubtract(&starPos, &scrollPos, &scrollDir);
                if (PSVECMag(&scrollDir) < 50.0f) {
                    cameraPos = starPos;
                } else {
                    PSVECNormalize(&scrollDir, &scrollDir);
                    scrollPos.x += 50.0f * scrollDir.x;
                    scrollPos.z += 50.0f * scrollDir.z;
                }
            } else {
                PSVECSubtract(&savedPos, &scrollPos, &scrollDir);
                if (PSVECMag(&scrollDir) < 4.0f * 50.0f) {
                    scrollPos = savedPos;
                    mode = 0;
                } else {
                    PSVECNormalize(&scrollDir, &scrollDir);
                    scrollPos.x += 4.0f * (50.0f * scrollDir.x);
                    scrollPos.z += 4.0f * (50.0f * scrollDir.z);
                }
            }
            break;
        }
        if (!CheckScrollCol(&scrollPos, &dir, &cameraPos)) {
            ResolveScrollCol(&dir, &scrollPos, &scrollPos, &cameraPos);
        }
        mbCameraFocusPosSet(&cameraPos);
        HuPrcVSleep();
    }
    return result;
}

static void RotateScrollView(HuVecF *rot, HuVecF *pos, HuVecF *posOut)
{
    posOut->x = pos->x + (HuSin(rot->y) * (pos->y / (HuSin(rot->x) / HuCos(rot->x))));
    posOut->z = pos->z + (HuCos(rot->y) * (pos->y / (HuSin(rot->x) / HuCos(rot->x))));
    posOut->y = 0.0f;
}

static BOOL CheckScrollCol(HuVecF *target, HuVecF *dir, HuVecF *endPos)
{
    float maxArea;
    float area;
    float triArea;
    HuVecF cross;
    HuVecF *vtxP[4];
    HuVecF edge;
    HuVecF up;
    HuVecF out;
    HSF_FACE *faceP;
    HSF_BUFFER *normBufP;
    HSF_BUFFER *vtxBufP;
    HSF_OBJECT *objP;
    HSF_BUFFER *faceBufP;
    int i;
    HU3D_MODEL *modelP;

    maxArea = -1.0f;
    if (scrollColModel < 0) {
        return FALSE;
    }
    modelP = &Hu3DData[scrollColModel];
    objP = modelP->hsf->root;
    if (objP->type != HSF_OBJ_MESH) {
        return FALSE;
    }
    faceBufP = objP->mesh.face;
    vtxBufP = objP->mesh.vertex;
    normBufP = objP->mesh.normal;
    for (faceP = faceBufP->data, i = 0; i < faceBufP->count; i++, faceP++) {
        if (faceP->type == HSF_FACE_TRI) {
            vtxP[0] = ((HuVecF *)vtxBufP->data) + faceP->index[0].vertex;
            triArea = (faceP->nbt[0] * vtxP[0]->x) + (faceP->nbt[1] * vtxP[0]->y)
                + (faceP->nbt[2] * vtxP[0]->z);
            area = ((triArea - (faceP->nbt[0] * target->x)) - (faceP->nbt[1] * target->y)
                - (faceP->nbt[2] * target->z))
                / ((faceP->nbt[0] * dir->x) + (faceP->nbt[1] * dir->y)
                    + (faceP->nbt[2] * dir->z));
            if (area < 0.0f) {
                continue;
            }
            if (maxArea >= 0.0f && area >= maxArea) {
                continue;
            }
            out.x = target->x + (area * dir->x);
            out.y = target->y + (area * dir->y);
            out.z = target->z + (area * dir->z);
            if (faceP->type == HSF_FACE_TRI) {
                vtxP[1] = ((HuVecF *)vtxBufP->data) + faceP->index[1].vertex;
                vtxP[2] = ((HuVecF *)vtxBufP->data) + faceP->index[2].vertex;
                PSVECSubtract(vtxP[1], vtxP[0], &edge);
                PSVECSubtract(&out, vtxP[1], &up);
                PSVECCrossProduct(&edge, &up, &cross);
                if (PSVECDotProduct(&cross, (HuVecF *)faceP->nbt) < 0.0f) {
                    continue;
                }
                PSVECSubtract(vtxP[2], vtxP[1], &edge);
                PSVECSubtract(&out, vtxP[2], &up);
                PSVECCrossProduct(&edge, &up, &cross);
                if (PSVECDotProduct(&cross, (HuVecF *)faceP->nbt) < 0.0f) {
                    continue;
                }
                PSVECSubtract(vtxP[0], vtxP[2], &edge);
                PSVECSubtract(&out, vtxP[0], &up);
                PSVECCrossProduct(&edge, &up, &cross);
                if (PSVECDotProduct(&cross, (HuVecF *)faceP->nbt) < 0.0f) {
                    continue;
                }
            }
            maxArea = area;
        }
    }
    if (maxArea >= 0.0f) {
        endPos->x = target->x + (maxArea * dir->x);
        endPos->y = target->y + (maxArea * dir->y);
        endPos->z = target->z + (maxArea * dir->z);
        return TRUE;
    }
    return FALSE;
}

static void InitScrollCol(void)
{
    HSF_FACE *faceP;
    HSF_FACE *faceP2;
    HSF_BUFFER *faceBufP;
    HSF_BUFFER *vtxBufP;
    HSF_OBJECT *objP;
    int i;
    int j;
    int k;
    int l;
    int nextVtx;
    int prev;
    BOOL linkF;
    HU3D_MODEL *modelP;

    if (scrollColModel < 0) {
        return;
    }
    modelP = &Hu3DData[scrollColModel];
    objP = modelP->hsf->root;
    if (objP->type != HSF_OBJ_MESH) {
        return;
    }
    faceBufP = objP->mesh.face;
    vtxBufP = objP->mesh.vertex;
    scrollColTriNum = 0;
    scrollColTriData = mbMalloc(faceBufP->count * sizeof(HSF_FACE));
    for (faceP = faceBufP->data, i = 0; i < faceBufP->count; i++, faceP++) {
        for (j = 0; j < 3; j++) {
            linkF = FALSE;
            nextVtx = (j + 1) % 3;
            for (faceP2 = faceBufP->data, k = 0; k < faceBufP->count; k++, faceP2++) {
                if (faceP != faceP2) {
                    for (l = 0; l < 3; l++) {
                        prev = l - 1;
                        if (prev < 0) {
                            prev = 2;
                        }
                        if (faceP->index[j].vertex == faceP2->index[l].vertex
                            && faceP->index[nextVtx].vertex == faceP2->index[prev].vertex) {
                            linkF++;
                            goto linked;
                        }
                    }
                }
            }
        linked:
            if (!linkF) {
                scrollColTriData[scrollColTriNum++] = *faceP;
                break;
            }
        }
    }
}

static void ResolveScrollCol(HuVecF *dir, HuVecF *pos1, HuVecF *pos2, HuVecF *endPos)
{
    HSF_FACE *faceP;
    int i;
    HSF_BUFFER *vtxBufP;
    HSF_FACE *outFaceP;
    HSF_OBJECT *objP;
    int no;
    HU3D_MODEL *modelP;
    float scale;
    float mag;
    float xzMag;
    float minMag;
    float scaleY;
    HuVecF inVtx[3];
    HuVecF edge;
    HuVecF edge2;
    HuVecF outPos2;

    outFaceP = 0;
    if (scrollColModel < 0) {
        return;
    }
    modelP = &Hu3DData[scrollColModel];
    objP = modelP->hsf->root;
    if (objP->type != HSF_OBJ_MESH) {
        return;
    }
    vtxBufP = objP->mesh.vertex;
    for (faceP = scrollColTriData, no = 0; no < scrollColTriNum; no++, faceP++) {
        if (PSVECDotProduct(dir, (HuVecF *)faceP->nbt) < 0.0f) {
            continue;
        }
        for (i = 0; i < 3; i++) {
            xzMag = HuMagXZVecF(dir);
            scaleY = (((HuVecF *)vtxBufP->data) + faceP->index[i].vertex)->y / (dir->y / xzMag);
            inVtx[i].x = (((HuVecF *)vtxBufP->data) + faceP->index[i].vertex)->x
                - (scaleY * (dir->x / xzMag));
            inVtx[i].z = (((HuVecF *)vtxBufP->data) + faceP->index[i].vertex)->z
                - (scaleY * (dir->z / xzMag));
            inVtx[i].y = 0.0f;
        }
        for (i = 0; i < 3; i++) {
            int nextVtx = (i + 1) % 3;

            PSVECSubtract(&inVtx[nextVtx], &inVtx[i], &edge);
            scale = ((pos1->x * edge.x) - (edge.x * inVtx[i].x) + (pos1->y * edge.y)
                - (edge.y * inVtx[i].y) + (pos1->z * edge.z) - (edge.z * inVtx[i].z))
                / PSVECSquareMag(&edge);
            if (scale >= 0.0f && scale < 1.0f) {
                edge2.x = inVtx[i].x + (scale * edge.x);
                edge2.y = inVtx[i].y + (scale * edge.y);
                edge2.z = inVtx[i].z + (scale * edge.z);
                PSVECSubtract(&edge2, pos1, &edge);
                mag = PSVECMag(&edge);
                if (outFaceP == 0 || mag < minMag) {
                    outFaceP = faceP;
                    minMag = mag;
                    outPos2 = edge2;
                }
            }
        }
        for (i = 0; i < 3; i++) {
            PSVECSubtract(&inVtx[i], pos1, &edge);
            mag = PSVECMag(&edge);
            if (outFaceP == 0 || mag < minMag) {
                outFaceP = faceP;
                minMag = mag;
                outPos2 = inVtx[i];
            }
        }
    }
    if (outFaceP != 0) {
        HuVecF *vtxP;
        float dot;

        faceP = outFaceP;
        vtxP = ((HuVecF *)vtxBufP->data) + faceP->index[0].vertex;
        dot = (faceP->nbt[0] * vtxP->x) + (faceP->nbt[1] * vtxP->y) + (faceP->nbt[2] * vtxP->z);
        scale = ((dot - (faceP->nbt[0] * outPos2.x)) - (faceP->nbt[1] * outPos2.y)
            - (faceP->nbt[2] * outPos2.z))
            / ((faceP->nbt[0] * dir->x) + (faceP->nbt[1] * dir->y) + (faceP->nbt[2] * dir->z));
        if (pos2 != 0) {
            *pos2 = outPos2;
        }
        if (endPos != 0) {
            endPos->x = outPos2.x + (scale * dir->x);
            endPos->y = outPos2.y + (scale * dir->y);
            endPos->z = outPos2.z + (scale * dir->z);
        }
    }
}

static s16 StarMasuGet(int playerNo)
{
    return mbMasuFind_TypeIdGet(GwPlayer[playerNo].masuId, 7, TRUE, TRUE);
}

void mbev_StarScroll(HuVecF *startPos, HuVecF *endPos, s16 time)
{
    HuVecF rot;
    HuVecF dir;
    HuVecF startCameraPos;
    HuVecF endCameraPos;
    HuVecF pos;
    HuVecF cameraPos;
    float weight;
    int i;

    mbCameraRotGet(&rot);
    startCameraPos.x = startPos->x + (HuSin(rot.y) * (startPos->y / (HuSin(rot.x) / HuCos(rot.x))));
    startCameraPos.z = startPos->z + (HuCos(rot.y) * (startPos->y / (HuSin(rot.x) / HuCos(rot.x))));
    startCameraPos.y = 0.0f;
    endCameraPos.x = endPos->x + (HuSin(rot.y) * (endPos->y / (HuSin(rot.x) / HuCos(rot.x))));
    endCameraPos.z = endPos->z + (HuCos(rot.y) * (endPos->y / (HuSin(rot.x) / HuCos(rot.x))));
    endCameraPos.y = 0.0f;
    dir.x = HuSin(rot.y) * HuCos(rot.x);
    dir.y = -HuSin(rot.x);
    dir.z = HuCos(rot.y) * HuCos(rot.x);
    PSVECScale(&dir, &dir, 100.0f);
    if (!CheckScrollCol(&startCameraPos, &dir, &cameraPos)) {
        ResolveScrollCol(&dir, &startCameraPos, NULL, &cameraPos);
    }
    mbCameraMovePos(&cameraPos, &rot, NULL, 1500.0f, -1.0f, 24);
    mbCameraMoveWait();
    if (time < 0) {
        time = 120;
    }
    for (i = 0; i <= time; i++) {
        weight = (float)i / (float)time;
        pos.x = startCameraPos.x + (weight * (endCameraPos.x - startCameraPos.x));
        pos.y = startCameraPos.y;
        pos.z = startCameraPos.z + (weight * (endCameraPos.z - startCameraPos.z));
        if (!CheckScrollCol(&pos, &dir, &cameraPos)) {
            ResolveScrollCol(&dir, &pos, NULL, &cameraPos);
        }
        mbCameraFocusPosSet(&cameraPos);
        HuPrcVSleep();
    }
}

static void MapViewCreate(void)
{
    mapViewZoom = 20000.0f;
    mapViewPos.x = mapViewPos.y = mapViewPos.z = 0.0f;
    mapViewRot.x = -78.0f;
    mapViewRot.y = 0.0f;
    mapViewRot.z = 0.0f;
    mapHook = NULL;

    masuMapAnim = HuSprAnimDataRead(DATANUM(DATA_bmasu, 2));
    HuSprAnimLock(masuMapAnim);
    pathAnim = HuSprAnimDataRead(DATANUM(DATA_bmasu, 6));
    HuSprAnimLock(pathAnim);
    HuDataDirClose(DATA_bmasu);
}

static void MapViewKill(void)
{
    if (masuMapAnim != NULL) {
        HuSprAnimKill(masuMapAnim);
        masuMapAnim = NULL;
    }
    if (pathAnim != NULL) {
        HuSprAnimKill(pathAnim);
        pathAnim = NULL;
    }
}

static void MapBaseSprCreate(void)
{
    s16 masuIdTbl[12];
    int masuNum;
    int i;
    int playerNo;

    if (GWPartyGet() != FALSE) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i == GwSystem.turnPlayerNo) {
                MapSprCreate(GwPlayer[i].charNo, GwPlayer[i].masuId, 5);
            } else {
                MapSprCreate(GwPlayer[i].charNo, GwPlayer[i].masuId, 4);
            }
        }
    } else {
        playerNo = GwSystem.turnPlayerNo;
        MapSprCreate(GwPlayer[playerNo].charNo, GwPlayer[playerNo].masuId, 1);
    }
    masuNum = mbMasuTypeListGet(7, masuIdTbl);
    for (i = 0; i < masuNum; i++) {
        MapSprCreate(-1, masuIdTbl[i], 0);
    }
    masuNum = mbMasuTypeListGet(10, masuIdTbl);
    for (i = 0; i < masuNum; i++) {
        MapSprCreate(-2, masuIdTbl[i], 0);
    }
}

static void MapSprPlayerColAll(void)
{
    int i;

    for (i = 0; i < 50; i++) {
        if (MapSprPlayerCol() == FALSE) {
            break;
        }
    }
}

static void MapSprKill(void)
{
    MAPSPRWORK *work;
    int i;
    int j;

    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        if (work->sprId[0] >= 0) {
            espKill(work->sprId[0]);
        }
        if (work->sprId[1] >= 0) {
            espKill(work->sprId[1]);
        }
        for (j = 0; j < 1; j++) {
            if (work->arrowSprId[j] >= 0) {
                espKill(work->arrowSprId[j]);
            }
        }
    }
}

void mbScrollStarFindFuncSet(MBSCROLLSTARFINDFUNC findFunc)
{
    scrollStarFindFunc = findFunc;
}

void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom)
{
    if (rot) {
        mapViewRot = *rot;
    }
    if (pos) {
        mapViewPos = *pos;
    }
    if (zoom >= 0.0f) {
        mapViewZoom = zoom;
    }
}

void mbMapHookSet(MBSCROLLHOOK hook)
{
    mapHook = hook;
}

void mbScrollHookSet(MBSCROLLHOOK hook)
{
    scrollHook = hook;
}

void mbMapSprAdd(int type, int id)
{
    if (type >= 16) {
        MapSprCreate(type, id, 0);
    } else {
        MapSprCreate(type, id, 5);
    }
}

void mbev_ScrollCapsule(int playerNo)
{
    mbev_Scroll(playerNo, FALSE);
}
