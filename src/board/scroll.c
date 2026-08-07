#define _MATH_H
#include "dolphin/math.h"
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
    u32 flags;
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
static u8 mapPathBit[0x100];
static SCROLLWORK scrollWork;

static SCROLLWORK *scrollWorkP = &scrollWork;
static GXColor starCol = { 255, 255, 255, 255 };

static GXColor charColorTbl[16] = {
    { 227, 67, 67, 255 },
    { 68, 67, 227, 255 },
    { 241, 158, 220, 255 },
    { 67, 228, 68, 255 },
    { 138, 60, 180, 255 },
    { 227, 228, 68, 255 },
    { 192, 192, 192, 255 },
    { 227, 227, 227, 255 },
    { 40, 227, 227, 255 },
    { 227, 139, 40, 255 },
    { 180, 40, 40, 255 },
    { 180, 40, 40, 255 },
    { 40, 180, 40, 255 },
    { 40, 40, 180, 255 },
    { 40, 180, 40, 255 },
    { 40, 40, 180, 255 },
};

static const int mapCharFileTbl[16] = {
    0x00050036, 0x00050037, 0x00050038, 0x00050039,
    0x0005003A, 0x0005003B, 0x0005003C, 0x0005003D,
    0x0005003E, 0x0005003F, 0x00050040, 0x00050041,
    0x00050042, 0x00050043, 0x000500A9, 0x000500A8,
};

static float mapViewZoom;
static int lbl_802C0DD8;
static HU3D_MODELID scrollColModel;
static MBSCROLLSTARFINDFUNC scrollStarFindFunc;
static int scrollColTriNum;
static HSF_FACE *scrollColTriData;
static MBSCROLLHOOK scrollHook;
static ANIMDATA *masuMapAnim;
static ANIMDATA *pathAnim;
static MBSCROLLHOOK mapHook;

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
static void MapDraw(HU3D_MODEL *modelP, Mtx *mtx);
static void MapSprCreate(int type, s16 masuId, int layer);
static void MapBaseSprCreate(void);
static void MapSprPosCalc(MAPSPRWORK *work);
static void MapSprPlayerPosCalc(int unused);
static BOOL MapSprPlayerCol(void);
static void MapSprPlayerColAll(void);
static void MapSprKill(void);
static void MapPathDraw(s16 masuId, Mtx *mtx);

extern void mbWipeDissolveFadeOut(void);
extern void mbWipeDissolveFadeIn(void);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeSpecialFadeInCreate(int type, BOOL pauseF);
extern void *mbMalloc(s32 size);
extern float mbSinDeg(float angle);
extern s8 mbPadStkXGet(int padNo);
extern s8 mbPadStkYGet(int padNo);
extern const float lbl_802C3550;
extern const float lbl_802C3554;
extern const float lbl_802C3558;

static inline BOOL MBTimeDayGet(void)
{
    return GwSystem.curTime == 0;
}

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
    BOOL partyF;
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
        if (btn & PAD_BUTTON_Y) {
            partyF = GwSystem.partyF;
            if (partyF) {
                result = TRUE;
                break;
            }
        }
        switch (mode) {
        case 0:
            maxSpeed = 30.000002f;
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

static BOOL MapViewExec(int playerNo)
{
    float near;
    float far;
    BOOL playerDisp[GW_PLAYER_MAX];
    HU3D_MODELID mapMdlId;
    BOOL result;
    s16 winNo;
    s16 padNo;
    BOOL partyF;
    int i;

    if (mbWipeSpecialStatGet() == FALSE) {
        mbWipeSpecialFadeInCreate(4, TRUE);
        mbStatusDispForceSetAll(FALSE);
    }
    mbCameraNearFarGet(&near, &far);
    mbCameraNearFarSet(100.0f, 30000.0f);
    mbCameraMovePos(&mapViewPos, &mapViewRot, NULL, mapViewZoom, -1.0f, -1);
    mbCameraMoveWait();
    memset(scrollWorkP, 0, sizeof(SCROLLWORK));
    scrollWorkP->playerPosNo = 0;
    MapBaseSprCreate();
    if (mapHook) {
        mapHook(TRUE);
    }
    MapSprPlayerPosCalc(scrollWorkP->playerPosNo);
    mbMasuModelDispSet(FALSE);
    mapMdlId = Hu3DHookFuncCreate(MapDraw);
    Hu3DModelCameraSet(mapMdlId, 4);
    Hu3DModelLayerSet(mapMdlId, 2);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        playerDisp[i] = mbPlayerDispGet(i);
        mbPlayerDispSet(i, FALSE);
    }
    mbEffFadeCreate(1, 128);
    scrollWorkP->mapFrame = 0;
    winNo = mbWinCreateHelp(0x00260006);
    mbWinPosSet(winNo, 120, 408);
    mbWipeSpecialFadeOutCreate(4, 30);
    padNo = GwPlayer[playerNo].padNo;
    while (TRUE) {
        u16 btn = HuPadBtnDown[padNo];

        if (btn & PAD_BUTTON_A) {
            scrollWorkP->playerPosNo = (scrollWorkP->playerPosNo + 1) % 3;
            partyF = GwSystem.partyF;
            if (!partyF && scrollWorkP->playerPosNo == 1) {
                scrollWorkP->playerPosNo = (scrollWorkP->playerPosNo + 1) % 3;
            }
            MapSprPlayerPosCalc(scrollWorkP->playerPosNo);
        }
        if (btn & PAD_BUTTON_B) {
            result = FALSE;
            break;
        }
        if (btn & PAD_BUTTON_X) {
            result = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbWinKill(winNo);
    mbMasuModelDispSet(TRUE);
    if (mapMdlId >= 0) {
        Hu3DModelKill(mapMdlId);
    }
    MapSprKill();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, playerDisp[i]);
    }
    mbEffFadeOutSet(1);
    mbCameraNearFarSet(near, far);
    if (mapHook) {
        mapHook(FALSE);
    }
    return result;
}

static inline s16 MapSprEntry(u32 dataNum, s16 prio)
{
    s16 sprId;

    sprId = espEntry(mbBoardDataNumGet(dataNum), prio, 0);
    espDrawNoSet(sprId, 64);
    return sprId;
}

static void MapSprCreate(int type, s16 masuId, int layer)
{
    MAPSPRWORK *work;
    int i;
    int j;
    u32 dataNum;

    work = scrollWorkP->mapSpr;
    for (i = 0; i < 32; i++, work++) {
        if (work->used == FALSE) {
            break;
        }
    }
    if (i >= 32) {
        return;
    }
    if (type >= 0) {
        work->color = charColorTbl[type];
    } else {
        work->color.r = work->color.g = work->color.b = 0;
        work->color.a = 255;
    }
    work->used = TRUE;
    work->dispF = TRUE;
    work->type = type;
    work->flags = layer;
    work->masuId = masuId;
    work->sprId[1] = -1;
    if (work->type >= 0 && work->type <= 15) {
        dataNum = mapCharFileTbl[work->type];
        work->sprId[0] = MapSprEntry(dataNum, 2000);
        work->arrowSprId[0] = MapSprEntry(0x000500AF, 2300);
        espColorSet(work->arrowSprId[0], work->color.r, work->color.g, work->color.b);
    } else {
        switch (type) {
        case -1:
            work->sprId[0] = MapSprEntry(0x000500A6, 2500);
            work->sprId[1] = MapSprEntry(0x000500A6, 2600);
            espScaleSet(work->sprId[0], 0.6f, 0.6f);
            espScaleSet(work->sprId[1], 0.6f, 0.6f);
            espTPLvlSet(work->sprId[1], 0.5f);
            break;
        case -2:
            work->sprId[0] = MapSprEntry(0x000500A7, 2500);
            work->sprId[1] = MapSprEntry(0x000500A7, 2600);
            espScaleSet(work->sprId[0], 0.6f, 0.6f);
            espScaleSet(work->sprId[1], 0.6f, 0.6f);
            espTPLvlSet(work->sprId[1], 0.5f);
            break;
        case 19:
            work->sprId[0] = MapSprEntry(0x000500AD, 2700);
            espScaleSet(work->sprId[0], 0.5f, 0.5f);
            break;
        case 16:
            work->sprId[0] = MapSprEntry(0x000500AA, 2700);
            espScaleSet(work->sprId[0], 0.5f, 0.5f);
            break;
        case 17:
            work->sprId[0] = MapSprEntry(0x000500AB, 2700);
            espScaleSet(work->sprId[0], 0.5f, 0.5f);
            break;
        case 18:
            work->sprId[0] = MapSprEntry(0x000500AC, 2700);
            espScaleSet(work->sprId[0], 0.5f, 0.5f);
            break;
        case 20:
            work->sprId[0] = MapSprEntry(0x000500AE, 2700);
            espScaleSet(work->sprId[0], 0.5f, 0.5f);
            break;
        }
        for (j = 0; j < 1; j++) {
            work->arrowSprId[j] = -1;
        }
    }
    MapSprPosCalc(work);
    work->pos = work->pos2D;
    scrollWorkP->mapSprNum++;
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

static void MapSprPosCalc(MAPSPRWORK *work)
{
    HuVecF masuPos;
    float posY;

    mbMasuPosGet(work->masuId, &masuPos);
    Hu3D3Dto2D(&masuPos, 1, &work->pos2D);
    if (work->type >= 0 && work->type <= 15) {
        work->pos.x = work->pos2D.x;
        if (work->pos2D.y > 240.0f) {
            posY = work->pos2D.y - 64.0f;
        } else {
            posY = work->pos2D.y + 64.0f;
        }
        work->pos.y = posY;
        work->pos.z = 0.0f;
    } else {
        work->pos = work->pos2D;
    }
}

static void MapSprPlayerPosCalc(int unused)
{
    MAPSPRWORK *work;
    HuVecF dir;
    HuVecF arrowPos;
    HuVecF masuPos;
    float posY;
    float mag;
    float scale;
    float rot;
    int i;

    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        if (work->flags != 0) {
            work->dispF = FALSE;
            if (work->sprId[0] >= 0) {
                espDispOff(work->sprId[0]);
            }
            if (work->sprId[1] >= 0) {
                espDispOff(work->sprId[1]);
            }
            {
                int j;

                for (j = 0; j < 1; j++) {
                    if (work->arrowSprId[j] >= 0) {
                        espDispOff(work->arrowSprId[j]);
                    }
                }
            }
            mbMasuPosGet(work->masuId, &masuPos);
            Hu3D3Dto2D(&masuPos, 1, &work->pos2D);
            if (work->type >= 0 && work->type <= 15) {
                work->pos.x = work->pos2D.x;
                if (work->pos2D.y > 240.0f) {
                    posY = work->pos2D.y - 64.0f;
                } else {
                    posY = work->pos2D.y + 64.0f;
                }
                work->pos.y = posY;
                work->pos.z = 0.0f;
            } else {
                work->pos = work->pos2D;
            }
        }
    }
    switch (scrollWorkP->playerPosNo) {
    case 0:
        work = scrollWorkP->mapSpr;
        for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
            if (work->flags & 1) {
                work->dispF = TRUE;
                if (work->sprId[0] >= 0) {
                    espDispOn(work->sprId[0]);
                }
                if (work->sprId[1] >= 0) {
                    espDispOn(work->sprId[1]);
                }
                {
                    int j;

                    for (j = 0; j < 1; j++) {
                        if (work->arrowSprId[j] >= 0) {
                            espDispOn(work->arrowSprId[j]);
                        }
                    }
                }
            }
        }
        break;
    case 1:
        work = scrollWorkP->mapSpr;
        for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
            if (work->flags & 4) {
                work->dispF = TRUE;
                if (work->sprId[0] >= 0) {
                    espDispOn(work->sprId[0]);
                }
                if (work->sprId[1] >= 0) {
                    espDispOn(work->sprId[1]);
                }
                {
                    int j;

                    for (j = 0; j < 1; j++) {
                        if (work->arrowSprId[j] >= 0) {
                            espDispOn(work->arrowSprId[j]);
                        }
                    }
                }
            }
        }
        break;
    }
    MapSprPlayerColAll();
    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        espPosSet(work->sprId[0], work->pos.x, work->pos.y);
        if (work->sprId[1] >= 0) {
            espPosSet(work->sprId[1], work->pos.x, work->pos.y);
        }
        if (work->type >= 0 && work->type <= 15) {
            dir.x = work->pos.x - work->pos2D.x;
            dir.y = work->pos.y - work->pos2D.y;
            dir.z = 0.0f;
            mag = PSVECMag(&dir);
            arrowPos.x = work->pos2D.x + (0.5f * dir.x);
            arrowPos.y = work->pos2D.y + (0.5f * dir.y);
            espPosSet(work->arrowSprId[0], arrowPos.x, arrowPos.y);
            rot = (atan2(dir.x, -dir.y) / M_PI) * 180.0;
            espZRotSet(work->arrowSprId[0], rot);
            scale = mag / 64.0f;
            espScaleSet(work->arrowSprId[0], 1.0f, scale);
        }
    }
}

static BOOL MapSprPlayerCol(void)
{
    MAPSPRWORK *work;
    MAPSPRWORK *work2;
    HuVecF delta;
    float deltaX;
    BOOL result;
    int i;
    int j;

    result = FALSE;
    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        work->colPos.x = work->colPos.y = work->colPos.z = 0.0f;
    }
    for (i = 0; i < scrollWorkP->mapSprNum - 1; i++) {
        work = &scrollWorkP->mapSpr[i];
        if (work->type >= 0 && work->type <= 15 && work->dispF) {
            for (j = i + 1; j < scrollWorkP->mapSprNum; j++) {
                work2 = &scrollWorkP->mapSpr[j];
                if (work2->type >= 0 && work2->type <= 15 && work2->dispF) {
                    PSVECSubtract(&work2->pos, &work->pos, &delta);
                    if (fabs(delta.x) < 64.0f && fabs(delta.y) < 64.0f) {
                        deltaX = 0.5f * (64.0f - fabs(delta.x));
                        if (delta.x < 0.0f) {
                            deltaX = -deltaX;
                        }
                        work->colPos.x += -deltaX;
                        work2->colPos.x += deltaX;
                        result = TRUE;
                    }
                }
            }
        }
    }
    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        if (work->type >= 0 && work->type <= 15) {
            work->pos.x += work->colPos.x;
            if (work->pos.x < 48.0f) {
                work->pos.x = 48.0f;
            } else if (work->pos.x > 528.0f) {
                work->pos.x = 528.0f;
            }
        }
    }
    return result;
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

static s16 masuPatTbl[11] = {
    -1, 0, 1, 2, 6, 7, -2, 5, 8, -1, 9,
};

static inline void MapSprScaleSet(void)
{
    MAPSPRWORK *work;
    float weight;
    float scale;
    int i;

    weight = (float)scrollWorkP->mapFrame++ / lbl_802C3550;
    if (scrollWorkP->mapFrame > 24U) {
        scrollWorkP->mapFrame = 0;
    }
    scale = 0.6f + (lbl_802C3554 * mbSinDeg(lbl_802C3558 * weight));
    work = scrollWorkP->mapSpr;
    for (i = 0; i < scrollWorkP->mapSprNum; i++, work++) {
        if (work->type < 0) {
            espScaleSet(work->sprId[1], scale, scale);
        }
    }
}

const float lbl_802C3550 = 24.0f;
const float lbl_802C3554 = 0.2f;
const float lbl_802C3558 = 90.0f;

static void MapPathDraw(s16 masuId, Mtx *mtx)
{
    Mtx pathMtx;
    Mtx startMtx;
    HuVecF pos2D;
    HuVecF endPos;
    HuVecF pos;
    HuVecF dir;
    HuVecF posCamera;
    HuVecF endPosCamera;
    HU3D_CAMERA *camera;
    float mag;
    float y;
    float x;
    float pathScale;
    int linkNum;
    s16 linkMasuId;
    u32 attr;
    u32 mAttr;
    int i;

    mapPathBit[masuId] = TRUE;
    mbMasuPosGet(masuId, &pos);
    camera = &Hu3DCamera[0];
    PSMTXMultVec(*mtx, &pos, &posCamera);
    x = posCamera.z * (HuSin(camera->fov / 2.0f) / HuCos(camera->fov / 2.0f)) * 1.2f;
    y = posCamera.z * (HuSin(camera->fov / 2.0f) / HuCos(camera->fov / 2.0f));
    pos2D.x = 288.0f + (posCamera.x * (288.0f / -x));
    pos2D.y = 240.0f + (posCamera.y * (240.0f / y));
    pos2D.z = 0.0f;
    PSMTXTrans(startMtx, pos2D.x, pos2D.y, 0.0f);
    linkNum = mbMasuLinkNumGet(masuId);
    for (i = 0; i < linkNum; i++) {
        linkMasuId = mbMasuLinkGet(masuId, i);
        attr = mbMasuAttrGet(linkMasuId);
        mAttr = mbMasuMAttrGet(linkMasuId);
        if ((attr & (u16)mbBranchAttrGet()) == 0 && (mAttr & mbBranchMAttrGet()) == 0) {
            HU3D_CAMERA *camera2;
            float endY;
            float endX;

            mbMasuPosGet(linkMasuId, &pos);
            camera2 = &Hu3DCamera[0];
            PSMTXMultVec(*mtx, &pos, &endPosCamera);
            endX = endPosCamera.z * (HuSin(camera2->fov / 2.0f)
                / HuCos(camera2->fov / 2.0f)) * 1.2f;
            endY = endPosCamera.z * (HuSin(camera2->fov / 2.0f)
                / HuCos(camera2->fov / 2.0f));
            endPos.x = 288.0f + (endPosCamera.x * (288.0f / -endX));
            endPos.y = 240.0f + (endPosCamera.y * (240.0f / endY));
            endPos.z = 0.0f;
            PSVECSubtract(&endPos, &pos2D, &dir);
            dir.z = 0.0f;
            mag = PSVECMag(&dir);
            pathScale = mag / 16.0f;
            pos.x = pos2D.x + (0.5f * dir.x);
            pos.y = pos2D.y + (0.5f * dir.y);
            mtxRot(pathMtx, 0.0f, 0.0f, (atan2(dir.x, -dir.y) / M_PI) * 180.0f);
            mtxScaleCat(pathMtx, 1.0f, pathScale, 0.0f);
            mtxTransCat(pathMtx, pos.x, pos.y, 0.0f);
            GXLoadPosMtxImm(pathMtx, GX_PNMTX0);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition2f32(-4.0f, -8.0f);
            GXTexCoord2f32(0.0f, scrollWorkP->mapPathScale);
            GXPosition2f32(4.0f, -8.0f);
            GXTexCoord2f32(1.0f, scrollWorkP->mapPathScale);
            GXPosition2f32(4.0f, 8.0f);
            GXTexCoord2f32(1.0f, scrollWorkP->mapPathScale + pathScale);
            GXPosition2f32(-4.0f, 8.0f);
            GXTexCoord2f32(0.0f, scrollWorkP->mapPathScale + pathScale);
            GXEnd();
            if (!mapPathBit[linkMasuId]) {
                MapPathDraw(linkMasuId, mtx);
            }
        }
    }
}

static void MapDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    Mtx texMtx;
    Mtx posMtx;
    Mtx cameraMtx;
    Mtx44 projection;
    HuVecF pos;
    HuVecF posCamera;
    HU3D_CAMERA *camera;
    float texX;
    float texY;
    float y;
    float x;
    int masuNum;
    s16 startMasuId;
    u32 attr;
    int masuType;
    u32 mAttr;
    int patNo;

    mbCameraLookAtGet(cameraMtx);
    C_MTXOrtho(projection, 0.0f, 480.0f, 0.0f, 576.0f, 0.0f, 100.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    HuSprTexLoad(masuMapAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad(pathAnim, 0, GX_TEXMAP1, GX_CLAMP, GX_REPEAT, GX_LINEAR);
    GXSetTevColor(GX_TEVREG0, starCol);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG,
        GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanAmbColor(GX_COLOR0A0, starCol);
    GXSetChanMatColor(GX_COLOR0A0, starCol);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    scrollWorkP->mapPathScale = (float)scrollWorkP->pathFrame++ / 30.0f;
    if (scrollWorkP->pathFrame > 30U) {
        scrollWorkP->pathFrame = 0;
    }
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    startMasuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    memset(mapPathBit, 0, sizeof(mapPathBit));
    MapPathDraw(startMasuId, &cameraMtx);
    if (mbMasuDispGet()) {
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
            GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        {
            int i;

            masuNum = mbMasuNumGet();
            for (i = 1; i < masuNum; i++) {
                masuType = mbMasuTypeGet(i);
                attr = mbMasuAttrGet(i);
                mAttr = mbMasuMAttrGet(i);
                if (masuType == 0 || (attr & ~0x10 & mbMasuDispAttrGet()) != 0
                    || (mAttr & mbMasuDispMAttrGet()) != 0) {
                    continue;
                }
                patNo = masuPatTbl[masuType];
                if (patNo < 0) {
                    if (patNo == -1) {
                        continue;
                    }
                    patNo = MBTimeDayGet() ? 4 : 3;
                }
                texX = (patNo % 4) / 4.0f;
                texY = (float)(patNo / 4) / 3.0f;
                PSMTXScale(texMtx, 0.25f, 1.0f / 3.0f, 0.0f);
                mtxTransCat(texMtx, texX, texY, 0.0f);
                GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
                mbMasuPosGet(i, &pos);
                camera = &Hu3DCamera[0];
                PSMTXMultVec(cameraMtx, &pos, &posCamera);
                x = posCamera.z * (HuSin(camera->fov / 2.0f)
                    / HuCos(camera->fov / 2.0f)) * 1.2f;
                y = posCamera.z * (HuSin(camera->fov / 2.0f)
                    / HuCos(camera->fov / 2.0f));
                pos.x = 288.0f + (posCamera.x * (288.0f / -x));
                pos.y = 240.0f + (posCamera.y * (240.0f / y));
                pos.z = 0.0f;
                PSMTXTrans(posMtx, pos.x, pos.y, 0.0f);
                GXLoadPosMtxImm(posMtx, GX_PNMTX0);
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition2f32(-8.0f, -8.0f);
                GXTexCoord2f32(0.0f, 0.0f);
                GXPosition2f32(8.0f, -8.0f);
                GXTexCoord2f32(1.0f, 0.0f);
                GXPosition2f32(8.0f, 8.0f);
                GXTexCoord2f32(1.0f, 1.0f);
                GXPosition2f32(-8.0f, 8.0f);
                GXTexCoord2f32(0.0f, 1.0f);
                GXEnd();
            }
        }
        MapSprScaleSet();
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

void mbMapSprAdd(int type, s16 id)
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
