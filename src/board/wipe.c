// Board scene transitions and their full-screen wipe effects.
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
double cos(double x);
double atan2(double y, double x);

#include "datadir_enum.h"
#include "game/data.h"
#include "game/disp.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/board/main.h"

#include <string.h>

extern BOOL mbTutorialExitReqGet(void);
extern BOOL mbPauseEnableCheck(void);
extern void *mbMalloc(s32 size);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
void mbWipeSpecialKill(void);
void mbWipeSpecialWait(void);

typedef struct WipeSpecialData_s {
    BOOL active; // Whether the special wipe update callback is running.
    int mode; // IN prepares captured effects or shrinks an image; OUT animates the prepared effect.
    int incomingComplete; // TRUE after capture setup or the incoming image animation finishes.
    s16 updateType; // Special wipe update variant from 1 to 8; zero stops its callback.
    s16 elapsedFrames; // Elapsed update frames for the current wipe.
    s16 durationFrames; // Requested wipe duration in update frames.
    s16 masuModelId[3]; // Model IDs for the three board-space wipe shapes.
    s16 hookModelId; // Model ID for the screen-space update and draw callback.
    u32 textureBytes; // Bytes allocated for the saved 320-by-240 RGB565 screen image.
    u16 *screenTexture; // Saved screen image used by the special wipe renderers.
    void *effectWork; // Per-effect animation data for grid, paper, or image wipes.
    int drawType; // Variant from 1 to 8; the hook draws grid, paper, and images for types 4 to 8.
} WIPE_SPECIAL_DATA;

typedef struct WipeGridEntry_s {
    Vec pos; // Grid-entry position in screen coordinates; final row and column are off-screen and
             // not drawn.
    Mtx transform; // Per-cell scale and rotation during the grid wipe.
} WIPE_GRID_ENTRY;

typedef struct WipeGridWork_s {
    int xCount; // Number of grid columns, including edge cells.
    int yCount; // Number of grid rows, including edge cells.
    int count; // Number of grid entries initialized in entry.
    WIPE_GRID_ENTRY entry[88]; // Transforms for 11-by-8 grid entries; drawing uses the first
                               // 10-by-7 as tile centers.
} WIPE_GRID_WORK;

typedef struct WipeImageWork_s {
    int imageNo; // Selected wipe image animation index.
    float centerX; // Image center X in the 576-pixel-wide image viewport.
    float centerY; // Image center Y in the 480-pixel-high image viewport.
    float unusedSlot; // Not read or written by the image effect.
    float size; // Image square side length in screen pixels.
    float alpha; // Image opacity from 0.0 to 1.0.
} WIPE_IMAGE_WORK;

typedef struct WipePaperVtx_s {
    float distance; // Distance along the fold's sweep across the screen, in pixels.
    float angle; // Rotation in degrees for the strip following this sample.
    float bend; // Fold angle in degrees at this sample.
    Vec pos[2]; // Endpoints of the sampled paper strip in screen space.
    HuVec2f texCoord[2]; // Saved-screen texture coordinates for the endpoints.
} WIPE_PAPER_VTX;

typedef struct WipePaperWork_s {
    Vec start; // First point of the fold's sweep across the screen.
    Vec end; // Last point of the fold's sweep across the screen.
    Vec sweepDirection; // Vector from the first sweep point to the last, in screen pixels.
    Vec rotationAxis; // Axis perpendicular to the sweep direction, along each paper strip.
    float length; // Sweep length in screen pixels.
    WIPE_PAPER_VTX vtx[64]; // Sixty-four samples used to animate the paper fold.
} WIPE_PAPER_WORK;

static void WipeMasuMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void WipeSpecialDraw(HU3D_MODEL *model, Mtx *mtx);
static void WipeShapeUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipeGridUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipePaperVtxUpdate(WIPE_PAPER_WORK *paperWork, float progress);
static void WipePaperUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipeImageUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipeGridDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipePaperDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);
static void WipeImageDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData);

static int wipeMasuFileTbl[3] = {
    DATANUM(DATA_bmasu, 7),
    DATANUM(DATA_bmasu, 8),
    DATANUM(DATA_bmasu, 9),
};

static int wipeImageFileTbl[3] = {
    DATANUM(DATA_bmasu, 10),
    DATANUM(DATA_bmasu, 12),
    DATANUM(DATA_bmasu, 11),
};

static void (*fadeFunc[8])(HU3D_MODEL *, Mtx *, WIPE_SPECIAL_DATA *) = {
    WipeShapeUpdate, WipeShapeUpdate, WipeShapeUpdate, WipeGridUpdate,
    WipePaperUpdate, WipeImageUpdate, WipeImageUpdate, WipeImageUpdate,
};

static HuVec2f paperST[4] = {
    { 0.6f, -1.0f },
    { -0.6f, 1.0f },
    { 0.6f, 1.0f },
    { -0.6f, -1.0f },
};

static Vec paperPos[4] = {
    { 0.0f, 0.0f, 0.0f },
    { 640.0f, 0.0f, 0.0f },
    { 640.0f, 480.0f, 0.0f },
    { 0.0f, 480.0f, 0.0f },
};

static Vec paperLightPos = { 0.0f, 0.0f, 1000.0f };
static Vec paperLightDir = { 0.0f, 0.0f, -1.0f };

static GXColor paperLightColor = { 255, 255, 255, 255 };
static GXColor paperMatColor = { 255, 255, 255, 255 };
static GXColor paperAmbColor = { 192, 192, 192, 255 };

static WIPE_SPECIAL_DATA wipeSpecialData;
static ANIMDATA *wipeImageAnim[3];

// Board transition code starts a standard wipe unless a tutorial exit is pending.
void mbWipeCreate(s16 mode, s16 type, s16 durationFrames)
{
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL) || !mbTutorialExitReqGet()) {
        WipeCreate(mode, type, durationFrames);
    }
}

// Board callers yield one scheduler pass at a time until the standard wipe has finished.
void mbWipeWait(void)
{
    while (WipeCheck()) {
        HuPrcVSleep();
    }
}

// Board transitions start a normal outgoing wipe and wait unless tutorial exit is pending.
void mbWipeFadeOut(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        // WipeColorSet ignores this request after the outgoing wipe has started.
        WipeColorSet(0, 0, 0);
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions reveal the scene and wait; tutorial exit suppresses the wipe.
void mbWipeFadeIn(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 21);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions request a white outgoing wipe and wait unless tutorial exit is pending.
void mbWipeWhiteFadeOut(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITE, 21);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions request a white incoming wipe and wait unless tutorial exit is pending.
void mbWipeWhiteFadeIn(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_WHITE, 21);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions capture the outgoing frame for the next cross fade, then wait for capture.
void mbWipeDissolveFadeOut(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSS_COPY, 1);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions dissolve the saved outgoing frame to reveal the new scene, then wait.
void mbWipeDissolveFadeIn(void)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSS_COPY, 30);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions start a normal outgoing wipe for the requested duration and wait.
void mbWipeFadeOutTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        // WipeColorSet ignores this request after the outgoing wipe has started.
        WipeColorSet(0, 0, 0);
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions reveal the scene over the requested number of frames.
void mbWipeFadeInTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions request a white outgoing wipe for the requested duration and wait.
void mbWipeWhiteFadeOutTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITE, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions request a white incoming wipe for the requested duration and wait.
void mbWipeWhiteFadeInTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_WHITE, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions capture the outgoing frame; cross-copy capture ignores the given duration.
void mbWipeDissolveFadeOutTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSS_COPY, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board transitions fade the saved outgoing frame away over the requested duration and wait.
void mbWipeDissolveFadeInTime(int durationFrames)
{
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSS_COPY, durationFrames);
        shouldRunWipe = TRUE;
    }
    if (shouldRunWipe) {
        while (WipeCheck()) {
            HuPrcVSleep();
        }
    }
}

// Board initialization creates wipe models, the screen buffer, and retains image animations.
void mbWipeSpecialInit(void)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;
    u32 textureBytes;
    void *allocatedTexture;
    u16 *screenTexture;
    int assetIndex;

    memset(wipeData, 0, sizeof(*wipeData));
    for (assetIndex = 0; assetIndex < 3; assetIndex++) {
        wipeData->masuModelId[assetIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(wipeMasuFileTbl[assetIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(wipeData->masuModelId[assetIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelMatHookSet(wipeData->masuModelId[assetIndex], WipeMasuMatHook);
        Hu3DModelPosSet(wipeData->masuModelId[assetIndex], 0.0f, 0.0f, -100.0f);
        Hu3DModelCameraSet(wipeData->masuModelId[assetIndex], HU3D_CAM2);
        Hu3DModelLayerSet(wipeData->masuModelId[assetIndex], 7);
    }
    wipeData->textureBytes = GXGetTexBufferSize(320, 240, GX_TF_RGB565,
        GX_FALSE, 0);
    textureBytes = wipeData->textureBytes;
    allocatedTexture = HuMemDirectMallocNum(HEAP_HEAP, textureBytes, HU_MEMNUM_OVL);
    screenTexture = allocatedTexture;
    wipeData->screenTexture = screenTexture;
    DCFlushRange(wipeData->screenTexture, wipeData->textureBytes);
    wipeData->hookModelId = Hu3DHookFuncCreate(WipeSpecialDraw);
    Hu3DModelCameraSet(wipeData->hookModelId, HU3D_CAM2);
    Hu3DModelLayerSet(wipeData->hookModelId, 6);
    for (assetIndex = 0; assetIndex < 3; assetIndex++) {
        wipeImageAnim[assetIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(wipeImageFileTbl[assetIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        HuSprAnimLock(wipeImageAnim[assetIndex]);
    }
}

// Board shutdown stops an active special wipe and releases its buffer and image animations.
void mbWipeSpecialClose(void)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;
    void *screenTexture;
    int imageIndex;

    mbWipeSpecialKill();
    if (wipeData->screenTexture) {
        screenTexture = wipeData->screenTexture;
        HuMemDirectFree(screenTexture);
        wipeData->screenTexture = NULL;
    }
    for (imageIndex = 0; imageIndex < 3; imageIndex++) {
        HuSprAnimKill(wipeImageAnim[imageIndex]);
        wipeImageAnim[imageIndex] = NULL;
    }
}

// Callers cancel a special wipe and release the effect-specific animation data.
void mbWipeSpecialKill(void)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;
    void *effectData;

    if (wipeData->effectWork) {
        effectData = wipeData->effectWork;
        HuMemDirectFree(effectData);
        wipeData->effectWork = NULL;
    }
    wipeData->updateType = 0;
    wipeData->active = FALSE;
    wipeData->drawType = 0;
    wipeData->incomingComplete = 0;
}

// The board-space masu model material hook maps the saved screen image onto its surface.
static void WipeMasuMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    WIPE_SPECIAL_DATA *wipeData;
    HU3D_CAMERA *camera;
    Mtx texMtx;
    GXColor color;
    float tanFov;

    camera = &Hu3DCamera[Hu3DCameraNo];
    wipeData = &wipeSpecialData;
    GXSetNumTexGens(1);
    tanFov = sin(M_PI * (0.5f * camera->fov) / 180.0)
        / cos(M_PI * (0.5f * camera->fov) / 180.0);
    MTXIdentity(texMtx);
    texMtx[0][0] = (-0.5f * drawObj->matrix[0][0])
        / (camera->aspect * (tanFov * drawObj->matrix[2][3]));
    texMtx[1][1] = (0.5f * drawObj->matrix[1][1])
        / (tanFov * drawObj->matrix[2][3]);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0,
        GX_FALSE, GX_PTIDENTITY);
    Hu3DTexLoad(wipeData->screenTexture, 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    color.a = 255.0f * (1.0f - material->invAlpha);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    Hu3DMatLightSet(drawObj->model, 2, material->hiliteScale);
}

// The Hu3D hook advances the active special wipe, then draws its selected effect each frame.
static void WipeSpecialDraw(HU3D_MODEL *model, Mtx *mtx)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;

    if (mbExitCheck()) {
        return;
    }
    if (wipeData->active && fadeFunc[wipeData->updateType - 1]) {
        fadeFunc[wipeData->updateType - 1](model, mtx, wipeData);
    }
    switch (wipeData->drawType) {
        case 4:
            WipeGridDraw(model, mtx, wipeData);
            break;
        case 5:
            WipePaperDraw(model, mtx, wipeData);
            break;
        case 6:
        case 7:
        case 8:
            WipeImageDraw(model, mtx, wipeData);
            break;
    }
}

// The special-wipe callback shrinks a saved-screen masu model or captures the current scene.
static void WipeShapeUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    float progress;
    float shapeScale;

    if (wipeData->mode == WIPE_MODE_OUT) {
        progress = (float)wipeData->elapsedFrames / (float)wipeData->durationFrames;
        if (!mbPauseEnableCheck()) {
            wipeData->elapsedFrames++;
        }
        // The outgoing shape starts at half scale and contracts toward zero.
        shapeScale = 0.5 * (1.0 - sin(M_PI * (90.0f * progress) / 180.0));
        Hu3DModelScaleSet(wipeData->masuModelId[wipeData->updateType - 1],
            shapeScale, shapeScale, shapeScale);
        if (wipeData->elapsedFrames > wipeData->durationFrames) {
            Hu3DModelAttrSet(wipeData->masuModelId[wipeData->updateType - 1],
                HU3D_ATTR_DISPOFF);
            wipeData->updateType = 0;
            wipeData->active = FALSE;
            wipeData->drawType = 0;
            wipeData->incomingComplete = 0;
        }
    } else {
        Hu3DModelAttrReset(wipeData->masuModelId[wipeData->updateType - 1],
            HU3D_ATTR_DISPOFF);
        Hu3DModelPosSet(wipeData->masuModelId[wipeData->updateType - 1],
            0.0f, 0.0f, -100.0f);
        Hu3DModelScaleSet(wipeData->masuModelId[wipeData->updateType - 1],
            1.0f, 1.0f, 1.0f);
        GXSetCopyFilter(GX_TRUE, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565,
            GX_TRUE, wipeData->screenTexture);
        GXPixModeSync();
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        wipeData->updateType = 0;
        wipeData->active = FALSE;
        wipeData->incomingComplete = 1;
    }
}

// The special-wipe callback spins and shrinks captured grid cells, or prepares a new capture.
static void WipeGridUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_GRID_WORK *gridWork;
    WIPE_GRID_WORK *finishedGridWork;
    WIPE_GRID_ENTRY *gridCell;
    int gridIndex;
    int columnIndex;
    float progress;
    float cellScale;

    if (mbPauseEnableCheck()) {
        return;
    }
    if (wipeData->mode == WIPE_MODE_OUT) {
        gridWork = wipeData->effectWork;
        progress = (float)wipeData->elapsedFrames / (float)wipeData->durationFrames;
        if (!mbPauseEnableCheck()) {
            wipeData->elapsedFrames++;
        }
        cellScale = 1.0 - sin(M_PI * (90.0f * progress) / 180.0);
        gridCell = gridWork->entry;
        for (gridIndex = 0; gridIndex < gridWork->count; gridCell++, gridIndex++) {
            MTXScale(gridCell->transform, cellScale, cellScale, 1.0f);
            mtxRotCat(gridCell->transform, 0.0f, 0.0f, 180.0f * progress);
        }
        if (wipeData->elapsedFrames > wipeData->durationFrames) {
            finishedGridWork = wipeData->effectWork;
            HuMemDirectFree(finishedGridWork);
            wipeData->effectWork = NULL;
            wipeData->updateType = 0;
            wipeData->active = FALSE;
            wipeData->drawType = 0;
            wipeData->incomingComplete = 0;
        }
    } else {
        gridWork = mbMalloc(sizeof(WIPE_GRID_WORK));
        wipeData->effectWork = gridWork;
        gridWork->xCount = 11;
        gridWork->yCount = 8;
        gridWork->count = gridWork->xCount * gridWork->yCount;
        gridCell = gridWork->entry;
        for (gridIndex = 0; gridIndex < 8; gridIndex++) {
            for (columnIndex = 0; columnIndex < 11; gridCell++, columnIndex++) {
                gridCell->pos.x = 320.0f / (gridWork->xCount - 1)
                    + (640.0f * columnIndex) / (gridWork->xCount - 1);
                gridCell->pos.y = 240.0f / (gridWork->yCount - 1)
                    + (480.0f * gridIndex) / (gridWork->yCount - 1);
                gridCell->pos.z = -100.0f;
                MTXIdentity(gridCell->transform);
            }
        }
        GXSetCopyFilter(GX_TRUE, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565,
            GX_TRUE, wipeData->screenTexture);
        GXPixModeSync();
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        wipeData->updateType = 0;
        wipeData->active = FALSE;
        wipeData->incomingComplete = 1;
    }
}

// The outgoing paper callback computes the 64 samples' fold angles for the current frame.
static void WipePaperVtxUpdate(WIPE_PAPER_WORK *paperWork, float progress)
{
    WIPE_PAPER_VTX *paperVertex;
    HuVec2f foldProfile[64];
    int sampleIndex;
    float foldRadius;
    float foldTravel;
    float profileDistance;
    float foldAngle;

    paperVertex = paperWork->vtx;
    foldRadius = 200.0f * (1.0f - progress);
    foldTravel = progress * (paperWork->length + ((float)M_PI * foldRadius));
    for (sampleIndex = 0; sampleIndex < 64; sampleIndex++, paperVertex++) {
        profileDistance = paperVertex->distance + foldTravel;
        if (profileDistance < paperWork->length) {
            foldProfile[sampleIndex].x = profileDistance;
            foldProfile[sampleIndex].y = 0.0f;
            paperVertex->bend = 0.0f;
        } else if (profileDistance < paperWork->length + ((float)M_PI * foldRadius)) {
            profileDistance -= paperWork->length;
            foldAngle = 360.0f * (profileDistance / (2.0f * (float)M_PI * foldRadius));
            foldProfile[sampleIndex].x = paperWork->length + (foldRadius * mbSinDeg(foldAngle));
            foldProfile[sampleIndex].y = foldRadius * (1.0f - mbCosDeg(foldAngle));
            if (foldAngle > 90.0f) {
                foldAngle = 180.0f - foldAngle;
            }
            paperVertex->bend = foldAngle;
        } else {
            profileDistance -= paperWork->length + ((float)M_PI * foldRadius);
            foldProfile[sampleIndex].x = paperWork->length - profileDistance;
            foldProfile[sampleIndex].y = 2.0f * foldRadius;
            paperVertex->bend = 0.0f;
        }
    }
    paperVertex = paperWork->vtx;
    for (sampleIndex = 0; sampleIndex < 63; sampleIndex++, paperVertex++) {
        HuVec2f segmentDirection;

        segmentDirection.x = foldProfile[sampleIndex + 1].x - foldProfile[sampleIndex].x;
        segmentDirection.y = foldProfile[sampleIndex + 1].y - foldProfile[sampleIndex].y;
        paperVertex->angle = 180.0 * (atan2(segmentDirection.y, segmentDirection.x) / M_PI);
    }
}

// The special-wipe callback captures or folds the screen along the current player's fixed diagonal.
static void WipePaperUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_PAPER_WORK *paperWork;
    WIPE_PAPER_WORK *finishedPaperWork;
    WIPE_PAPER_VTX *paperVertex;
    HuVec2f startPoint;
    HuVec2f endPoint;
    HuVec2f direction;
    HuVec2f point;
    float intersections[4];
    float fractionOrIntersection; // Reused for animation progress, edge projection, and sort
                                  // scratch.
    float firstEdgeProjection;
    float lastEdgeProjection;
    int pointIndex;
    int sortIndex;
    int compareIndex;
    int playerNo;

    if (mbPauseEnableCheck()) {
        return;
    }
    if (wipeData->mode == WIPE_MODE_OUT) {
        paperWork = wipeData->effectWork;
        fractionOrIntersection = (float)wipeData->elapsedFrames / (float)wipeData->durationFrames;
        if (!mbPauseEnableCheck()) {
            wipeData->elapsedFrames++;
        }
        WipePaperVtxUpdate(paperWork, fractionOrIntersection);
        if (wipeData->elapsedFrames > wipeData->durationFrames) {
            finishedPaperWork = wipeData->effectWork;
            HuMemDirectFree(finishedPaperWork);
            wipeData->effectWork = NULL;
            wipeData->updateType = 0;
            wipeData->active = FALSE;
            wipeData->drawType = 0;
            wipeData->incomingComplete = 0;
        }
    } else {
        paperWork = mbMalloc(sizeof(WIPE_PAPER_WORK));
        wipeData->effectWork = paperWork;
        playerNo = GwSystem.turnPlayerNo;
        // Invalid player indices still use one of the four fixed paper directions.
        if (playerNo < 0) {
            playerNo = 0;
        } else if (playerNo > 3) {
            playerNo = 3;
        }
        direction = paperST[playerNo];
        startPoint.x = 320.0f - (400.0f * direction.x);
        startPoint.y = 240.0f - (400.0f * direction.y);
        endPoint.x = 320.0f + (400.0f * direction.x);
        endPoint.y = 240.0f + (400.0f * direction.y);
        PSVECSubtract((Vec *)&endPoint, (Vec *)&startPoint, (Vec *)&direction);

        firstEdgeProjection = 9999.0f;
        lastEdgeProjection = -9999.0f;
        // Project each screen corner onto the diagonal to find the full screen span.
        for (pointIndex = 0; pointIndex < 4; pointIndex++) {
            fractionOrIntersection = ((direction.y * paperPos[pointIndex].y)
                    + ((direction.x * paperPos[pointIndex].x)
                        - (startPoint.x * direction.x))
                    - (startPoint.y * direction.y))
                / ((direction.x * direction.x)
                    + (direction.y * direction.y));
            if (fractionOrIntersection < firstEdgeProjection) {
                firstEdgeProjection = fractionOrIntersection;
            }
            if (fractionOrIntersection > lastEdgeProjection) {
                lastEdgeProjection = fractionOrIntersection;
            }
        }
        paperWork->start.x = startPoint.x + (firstEdgeProjection * direction.x);
        paperWork->start.y = startPoint.y + (firstEdgeProjection * direction.y);
        paperWork->end.x = startPoint.x + (lastEdgeProjection * direction.x);
        paperWork->end.y = startPoint.y + (lastEdgeProjection * direction.y);
        PSVECSubtract(&paperWork->end, &paperWork->start, &paperWork->sweepDirection);
        paperWork->length = PSVECMag(&paperWork->sweepDirection);
        paperWork->rotationAxis.x = paperWork->sweepDirection.y;
        paperWork->rotationAxis.y = -paperWork->sweepDirection.x;
        paperWork->rotationAxis.z = 0.0f;
        direction.x = paperWork->sweepDirection.y;
        direction.y = -paperWork->sweepDirection.x;

        paperVertex = paperWork->vtx;
        for (pointIndex = 0; pointIndex < 64; pointIndex++, paperVertex++) {
            fractionOrIntersection = (float)pointIndex / 63.0f;
            paperVertex->distance = paperWork->length * fractionOrIntersection;
            point.x = paperWork->start.x + (fractionOrIntersection * paperWork->sweepDirection.x);
            point.y = paperWork->start.y + (fractionOrIntersection * paperWork->sweepDirection.y);
            intersections[0] = -point.x / direction.x;
            intersections[1] = (640.0f - point.x) / direction.x;
            intersections[2] = -point.y / direction.y;
            intersections[3] = (480.0f - point.y) / direction.y;
            // Sort the four edge intersections so entries 1 and 2 bound the strip.
            for (sortIndex = 0; sortIndex < 3; sortIndex++) {
                for (compareIndex = sortIndex; compareIndex < 4; compareIndex++) {
                    if (intersections[sortIndex] > intersections[compareIndex]) {
                        fractionOrIntersection = intersections[sortIndex];
                        intersections[sortIndex] = intersections[compareIndex];
                        intersections[compareIndex] = fractionOrIntersection;
                    }
                }
            }
            paperVertex->pos[0].x = point.x + (intersections[1] * direction.x);
            paperVertex->pos[0].y = point.y + (intersections[1] * direction.y);
            paperVertex->pos[0].z = -500.0f;
            paperVertex->texCoord[0].x = paperVertex->pos[0].x / 640.0f;
            paperVertex->texCoord[0].y = paperVertex->pos[0].y / 480.0f;
            paperVertex->pos[1].x = point.x + (intersections[2] * direction.x);
            paperVertex->pos[1].y = point.y + (intersections[2] * direction.y);
            paperVertex->pos[1].z = -500.0f;
            paperVertex->texCoord[1].x = paperVertex->pos[1].x / 640.0f;
            paperVertex->texCoord[1].y = paperVertex->pos[1].y / 480.0f;
        }
        GXSetCopyFilter(GX_TRUE, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565,
            GX_TRUE, wipeData->screenTexture);
        GXPixModeSync();
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE,
            RenderMode->vfilter);
        wipeData->updateType = 0;
        wipeData->active = FALSE;
        wipeData->incomingComplete = 1;
    }
}

// The special-wipe callback grows or shrinks the selected board image around screen center.
static void WipeImageUpdate(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_IMAGE_WORK *imageWork;
    WIPE_IMAGE_WORK *finishedImageWork;
    float progress;

    if (mbPauseEnableCheck()) {
        return;
    }
    if (wipeData->mode == WIPE_MODE_OUT) {
        imageWork = wipeData->effectWork;
        progress = (float)wipeData->elapsedFrames / (float)wipeData->durationFrames;
        if (!mbPauseEnableCheck()) {
            wipeData->elapsedFrames++;
        }
        imageWork->size = 1000.0f * progress;
        if (wipeData->elapsedFrames > wipeData->durationFrames) {
            if (wipeData->effectWork) {
                finishedImageWork = wipeData->effectWork;
                HuMemDirectFree(finishedImageWork);
                wipeData->effectWork = NULL;
            }
            wipeData->updateType = 0;
            wipeData->active = FALSE;
            wipeData->drawType = 0;
            wipeData->incomingComplete = 0;
        }
    } else {
        if (!wipeData->effectWork) {
            imageWork = mbMalloc(sizeof(WIPE_IMAGE_WORK));
            wipeData->effectWork = imageWork;
            imageWork->centerX = 288.0f;
            imageWork->centerY = 240.0f;
            imageWork->alpha = 1.0f;
            imageWork->imageNo = wipeData->updateType - 6;
        } else {
            imageWork = wipeData->effectWork;
        }
        progress = (float)wipeData->elapsedFrames / (float)wipeData->durationFrames;
        if (!mbPauseEnableCheck()) {
            wipeData->elapsedFrames++;
        }
        imageWork->size = 1000.0f * (1.0f - progress);
        if (wipeData->elapsedFrames > wipeData->durationFrames) {
            wipeData->updateType = 0;
            wipeData->active = FALSE;
            wipeData->incomingComplete = 1;
            return;
        }
    }
}

// The Hu3D draw hook renders saved-screen texture across the animated grid cells.
static void WipeGridDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_GRID_WORK *gridWork = wipeData->effectWork;
    WIPE_GRID_ENTRY *gridCell;
    Mtx drawMtx;
    Mtx44 projection;
    GXColor color = { 255, 255, 255, 255 };
    float tileWidth;
    float tileHeight;
    int rowIndex;
    int columnIndex;

    C_MTXOrtho(projection, 0.0f, 480.0f,
        0.0f, 640.0f, 0.0f, 1000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    GXSetScissor(0, 0, 640, 480);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_SPOT);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);
    Hu3DTexLoad(wipeData->screenTexture, 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);

    tileWidth = 640.0f / (gridWork->xCount - 1);
    tileHeight = 480.0f / (gridWork->yCount - 1);
    for (rowIndex = 0; rowIndex < gridWork->yCount - 1; rowIndex++) {
        for (columnIndex = 0; columnIndex < gridWork->xCount - 1; columnIndex++) {
            gridCell = &gridWork->entry[columnIndex + (rowIndex * gridWork->xCount)];
            PSMTXTrans(drawMtx, gridCell->pos.x, gridCell->pos.y, gridCell->pos.z);
            PSMTXConcat(drawMtx, gridCell->transform, drawMtx);
            GXLoadPosMtxImm(drawMtx, GX_PNMTX0);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(0.5f * -tileWidth, 0.5f * -tileHeight, 0.0f);
            GXTexCoord2f32((float)columnIndex / (gridWork->xCount - 1),
                (float)rowIndex / (gridWork->yCount - 1));
            GXPosition3f32(0.5f * tileWidth, 0.5f * -tileHeight, 0.0f);
            GXTexCoord2f32((float)(columnIndex + 1) / (gridWork->xCount - 1),
                (float)rowIndex / (gridWork->yCount - 1));
            GXPosition3f32(0.5f * tileWidth, 0.5f * tileHeight, 0.0f);
            GXTexCoord2f32((float)(columnIndex + 1) / (gridWork->xCount - 1),
                (float)(rowIndex + 1) / (gridWork->yCount - 1));
            GXPosition3f32(0.5f * -tileWidth, 0.5f * tileHeight, 0.0f);
            GXTexCoord2f32((float)columnIndex / (gridWork->xCount - 1),
                (float)(rowIndex + 1) / (gridWork->yCount - 1));
        }
    }
}

// The Hu3D draw hook renders the saved screen as a lit, folding paper strip.
static void WipePaperDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_PAPER_WORK *paperWork = wipeData->effectWork;
    WIPE_PAPER_VTX *paperVertex = paperWork->vtx;
    Mtx rotationMtx;
    Mtx normalMtx;
    GXLightObj light;
    Mtx44 projection;
    GXColor color = { 255, 255, 255, 255 };
    Vec currentEndpoints[2];
    Vec nextEndpoints[2];
    Vec segmentDeltas[2];
    Vec lightingNormal;
    int stripIndex;

    C_MTXOrtho(projection, 0.0f, 480.0f,
        0.0f, 640.0f, 0.0f, 1000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXSetViewport(0.0f, 0.0f, 640.0f,
        480.0f, 0.0f, 1.0f);
    GXSetScissor(0, 0, 640, 480);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_SPOT);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);

    memset(&light, 0, sizeof(light));
    GXInitLightAttnA(&light, 1.0f, 0.0f,
        0.0f);
    GXInitLightDistAttn(&light, 0.0f, 1.0f, GX_DA_OFF);
    GXInitLightPos(&light, paperLightPos.x, paperLightPos.y,
        paperLightPos.z);
    GXInitLightDir(&light, paperLightDir.x, paperLightDir.y,
        paperLightDir.z);
    GXInitLightColor(&light, paperLightColor);
    GXLoadLightObjImm(&light, GX_LIGHT0);

    Hu3DTexLoad(wipeData->screenTexture, 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE, GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
        GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanMatColor(GX_COLOR0A0, paperMatColor);
    GXSetChanAmbColor(GX_COLOR0A0, paperAmbColor);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    currentEndpoints[0] = paperVertex->pos[0];
    currentEndpoints[1] = paperVertex->pos[1];
    PSMTXIdentity(rotationMtx);
    GXLoadPosMtxImm(rotationMtx, GX_PNMTX0);
    PSMTXInvXpose(rotationMtx, normalMtx);
    GXLoadNrmMtxImm(normalMtx, GX_PNMTX0);
    for (stripIndex = 0; stripIndex < 63; stripIndex++, paperVertex++) {
        PSVECSubtract(&paperVertex[1].pos[0], &paperVertex->pos[0], &segmentDeltas[0]);
        PSVECSubtract(&paperVertex[1].pos[1], &paperVertex->pos[1], &segmentDeltas[1]);
        PSMTXRotAxisRad(rotationMtx, &paperWork->rotationAxis,
            ((float)M_PI / 180.0f) * paperVertex->angle);
        PSMTXMultVec(rotationMtx, &segmentDeltas[0], &segmentDeltas[0]);
        PSMTXMultVec(rotationMtx, &segmentDeltas[1], &segmentDeltas[1]);
        PSVECAdd(&currentEndpoints[0], &segmentDeltas[0], &nextEndpoints[0]);
        PSVECAdd(&currentEndpoints[1], &segmentDeltas[1], &nextEndpoints[1]);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        PSMTXRotAxisRad(rotationMtx, &paperWork->rotationAxis,
            ((float)M_PI / 180.0f) * (90.0f + paperVertex->bend));
        PSMTXMultVec(rotationMtx, &paperWork->sweepDirection, &lightingNormal);
        GXPosition3f32(currentEndpoints[0].x, currentEndpoints[0].y, currentEndpoints[0].z);
        GXNormal3f32(lightingNormal.x, lightingNormal.y, lightingNormal.z);
        GXTexCoord2f32(paperVertex->texCoord[0].x, paperVertex->texCoord[0].y);
        GXPosition3f32(currentEndpoints[1].x, currentEndpoints[1].y, currentEndpoints[1].z);
        GXNormal3f32(lightingNormal.x, lightingNormal.y, lightingNormal.z);
        GXTexCoord2f32(paperVertex->texCoord[1].x, paperVertex->texCoord[1].y);

        PSMTXRotAxisRad(rotationMtx, &paperWork->rotationAxis,
            ((float)M_PI / 180.0f) * (90.0f + paperVertex[1].bend));
        PSMTXMultVec(rotationMtx, &paperWork->sweepDirection, &lightingNormal);
        GXPosition3f32(nextEndpoints[1].x, nextEndpoints[1].y, nextEndpoints[1].z);
        GXNormal3f32(lightingNormal.x, lightingNormal.y, lightingNormal.z);
        GXTexCoord2f32(paperVertex[1].texCoord[1].x, paperVertex[1].texCoord[1].y);
        GXPosition3f32(nextEndpoints[0].x, nextEndpoints[0].y, nextEndpoints[0].z);
        GXNormal3f32(lightingNormal.x, lightingNormal.y, lightingNormal.z);
        GXTexCoord2f32(paperVertex[1].texCoord[0].x, paperVertex[1].texCoord[0].y);
        currentEndpoints[0] = nextEndpoints[0];
        currentEndpoints[1] = nextEndpoints[1];
    }
}

// The Hu3D draw hook uses the board image's alpha as a black mask and fills its outside region.
static void WipeImageDraw(HU3D_MODEL *model, Mtx *mtx,
    WIPE_SPECIAL_DATA *wipeData)
{
    WIPE_IMAGE_WORK *imageWork;
    Mtx44 imageProjection;
    Mtx posMtx;
    Mtx44 projection;
    GXColor color = { 255, 255, 255, 255 };
    float halfSize;
    float left;
    float top;
    float right;
    float bottom;

    imageWork = wipeData->effectWork;
    C_MTXOrtho(projection, 0.0f, 480.0f,
        0.0f, 640.0f, 0.0f, 1000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXSetViewport(0.0f, 0.0f, 640.0f,
        480.0f, 0.0f, 1.0f);
    GXSetScissor(0, 0, 640, 480);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_SPOT);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);

    C_MTXOrtho(imageProjection, 0.0f, 480.0f,
        0.0f, 576.0f, 0.0f, 1000.0f);
    GXSetProjection(imageProjection, GX_ORTHOGRAPHIC);
    HuSprTexLoad(wipeImageAnim[imageWork->imageNo], 0, GX_TEXMAP0, GX_CLAMP,
        GX_CLAMP, GX_LINEAR);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    PSMTXIdentity(posMtx);
    GXLoadPosMtxImm(posMtx, GX_PNMTX0);

    halfSize = 0.5f * imageWork->size;
    left = imageWork->centerX - halfSize;
    top = imageWork->centerY - halfSize;
    right = imageWork->centerX + halfSize;
    bottom = imageWork->centerY + halfSize;
    color.a = 255.0f * imageWork->alpha;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    if (top > 0.0f) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, 0.0f);
        GXPosition2f32(576.0f, 0.0f);
        GXPosition2f32(576.0f, top);
        GXPosition2f32(0.0f, top);
    }
    if (bottom < 480.0f) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, bottom);
        GXPosition2f32(576.0f, bottom);
        GXPosition2f32(576.0f, 480.0f);
        GXPosition2f32(0.0f, 480.0f);
    }
    if (left > 0.0f) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, top);
        GXPosition2f32(left, top);
        GXPosition2f32(left, bottom);
        GXPosition2f32(0.0f, bottom);
    }
    if (left < 576.0f) {
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(right, top);
        GXPosition2f32(576.0f, top);
        GXPosition2f32(576.0f, bottom);
        GXPosition2f32(right, bottom);
    }

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(left, top);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition2f32(right, top);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition2f32(right, bottom);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition2f32(left, bottom);
    GXTexCoord2f32(0.0f, 1.0f);
}

// Board event callers start an asynchronous special wipe with the requested mode and duration.
void mbWipeSpecialCreate(int mode, int type, int durationFrames)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;
    void *effectData;

    if (!_CheckFlag(FLAG_BOARD_TUTORIAL) || !mbTutorialExitReqGet()) {
        wipeData->mode = mode;
        wipeData->updateType = type;
        wipeData->elapsedFrames = 0;
        wipeData->durationFrames = durationFrames;
        wipeData->active = TRUE;
        wipeData->drawType = type;
        if (mode == WIPE_MODE_IN) {
            if (wipeData->effectWork) {
                effectData = wipeData->effectWork;
                HuMemDirectFree(effectData);
            }
            wipeData->effectWork = NULL;
        }
    }
}

// Special fade callers share this setup, which suppresses tutorial-exit wipes and clears IN work.
static inline BOOL mbWipeSpecialFadeCreate(int mode, int type, int durationFrames)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;
    void *effectData;
    BOOL shouldRunWipe;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL) && mbTutorialExitReqGet()) {
        shouldRunWipe = FALSE;
    } else {
        wipeData->mode = mode;
        wipeData->updateType = type;
        wipeData->elapsedFrames = 0;
        wipeData->durationFrames = durationFrames;
        wipeData->active = TRUE;
        wipeData->drawType = type;
        if (mode == WIPE_MODE_IN) {
            if (wipeData->effectWork) {
                effectData = wipeData->effectWork;
                HuMemDirectFree(effectData);
            }
            wipeData->effectWork = NULL;
        }
        shouldRunWipe = TRUE;
    }
    return shouldRunWipe;
}

// Board callers remove the saved shape, grid, or paper screen, or expand an image mask, and wait.
void mbWipeSpecialFadeOutCreate(int type, int durationFrames)
{
    if (mbWipeSpecialFadeCreate(WIPE_MODE_OUT, type, durationFrames)) {
        mbWipeSpecialWait();
    }
}

// Board callers capture a shape, grid, or paper screen, or shrink an image mask, and wait.
void mbWipeSpecialFadeInCreate(int type, int durationFrames)
{
    if (mbWipeSpecialFadeCreate(WIPE_MODE_IN, type, durationFrames)) {
        mbWipeSpecialWait();
    }
}

// Board event callers test whether special or standard wipe work is still active.
BOOL mbWipeSpecialCheck(void)
{
    WIPE_SPECIAL_DATA *wipeData = &wipeSpecialData;

    return wipeData->active | WipeCheck();
}

// Board callers yield one scheduler pass at a time until both wipe systems are idle.
void mbWipeSpecialWait(void)
{
    while (mbWipeSpecialCheck()) {
        HuPrcVSleep();
    }
}

// Board flows test for a completed incoming special phase or a standard wipe outside dummy mode.
BOOL mbWipeSpecialStatGet(void)
{
    return wipeSpecialData.incomingComplete | WipeCheckIn();
}
