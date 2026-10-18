/* Solo Mode stage scenery, particle effects and screen overlays. */
#include "dolphin/math.h"
#include <string.h>
#include "dolphin/os.h"
#include "datadir_enum.h"
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/hsfex.h"
#include "game/frand.h"
#include "REL/mdsingdll/stage_effects.h"
#define MDSING_MODEL_VISIBLE_MASK 0xFFFFFFFEU
#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)
enum { MDPARTY_STAGE_PARTICLE_TARGET_COUNT = 5, MDPARTY_COLOR_COMPONENT_MASK = 255 };
typedef struct MdpartyStageParticleTarget {
    /* Target particle size and opacity (0-255) for each highlight layer. */
    u8 scale;
    u8 alpha;
} MDPARTY_STAGE_PARTICLE_TARGET;
typedef struct MDMinidllHalfwordTemplate {
    /* Maximum particle count for each of an effect group's five emitters. */
    s16 particleCount[5];
} MDMinidllHalfwordTemplate;

float fn_1_2CF38(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D020(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D0FC(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D140(float currentValue, float targetValue, float weight);
void fn_1_2D170(s16 layerNo);
void fn_1_2D1DC(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_2D65C(OMOBJ *obj);
void fn_1_2D75C(s16 textureSlot, HuVecF *position);
void fn_1_2D850(OMOBJ *obj);
void fn_1_2DA34(OMOBJ *obj);
void fn_1_2DAE4(void);
void fn_1_2DB34(void);
void fn_1_2DC10(Point3d *position);
void fn_1_2DCD0(s16 layerNo);
void fn_1_2E68C(void);
void fn_1_2E6B8(void);
void fn_1_2E6DC(
    s16 particleNo, float lifetimeFrames, float targetScale, HuVecF *position, GXColor color);
void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2EA9C(void);
void fn_1_2EBB0(void);
void fn_1_2EBDC(s16 effectNo, float lifetimeFrames, HuVecF *position, GXColor color);
void fn_1_2ECC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2F20C(void);
void fn_1_2F3CC(void);
void fn_1_2F424(void);
void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2FC50(void);
void fn_1_2FD50(void);
void fn_1_2FD7C(s16 effectNo, float durationFrames, HuVecF *position);
void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_30730(void);
void fn_1_308C4(void);
void fn_1_3091C(HuVecF *position, GXColor *color, float lifetimeFrames, float targetScale);
void fn_1_30A40(float lifetimeFrames, float travelDistance, HuVecF *position,
    GXColor *color);
void fn_1_30BE0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31088(void);
void fn_1_3116C(void);
void fn_1_31198(s16 effectNo, s16 visible);
void fn_1_31214(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31318(s16 effectNo);
void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31818(void);
void fn_1_31984(void);
void fn_1_319DC(s16 effectNo, s16 visible);
void fn_1_31A58(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31B90(s16 effectNo);
void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_321E4(void);
void fn_1_32350(void);
void fn_1_323A8(s16 modelNo, HuVecF *position, s16 displayMode);
void fn_1_3276C(s16 groupNo, s16 show);
void fn_1_32820(s16 groupNo, HuVecF *spawnPosition, GXColor *color);
void fn_1_32988(s16 groupNo);
void fn_1_32A08(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_32FB4(void);
void fn_1_331E0(void);
void fn_1_33264(HUPROCESS *process);
void fn_1_33D84(void);
void fn_1_33FC0(HuVecF *position);
void fn_1_341B8(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_344B4(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_34958(void);
void fn_1_34A18(HuVecF *position);
void fn_1_34FA4(s16 groupNo, HuVecF *position, s16 mode, s16 colorNo);

/* Ease an effect value toward its endpoint over the supplied frame interval. */
float fn_1_2CF38(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / durationFrames) * elapsedFrames));
}

/* Pulse an effect value toward its peak and back over the supplied frame interval. */
float fn_1_2D020(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sind((180.0f / durationFrames) * elapsedFrames));
}

/* Interpolate effect values linearly, clamping at both ends of the frame interval. */
float fn_1_2D0FC(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((elapsedFrames / durationFrames) * (endValue - startValue));
}

/* Smooth an effect value toward its target using the supplied averaging weight. */
float fn_1_2D140(float currentValue, float targetValue, float weight)
{
    if (currentValue == targetValue) {
        return targetValue;
    }
    return (targetValue + (currentValue * (weight - 1.0f))) / weight;
}

/* Capture a half-resolution copy of the screen from the scenery's render-layer callback. */
void fn_1_2D170(s16 layerNo)
{
    if (lbl_1_bss_13C8) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_13C8, GX_FALSE);
    }
}

/* Combine the captured screen and scrolling textures in the distortion model's material
 * callback. */
void fn_1_2D1DC(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    Mtx transform;
    Mtx textureScale;
    Mtx textureMatrix;
    Mtx textureProjection;
    Mtx inverseView;
    GXTexObj screenTexture;
    MDPARTY_STAGE_TEXTURE_WORK *textureWork;
    HU3D_CAMERA *camera;

    textureWork = drawObj->model->hookData;
    camera = Hu3DCamera;
    GXInitTexObj(&screenTexture, lbl_1_bss_13C8, 320U, 240U, GX_TF_RGB565,
        GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&screenTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f,
        0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&screenTexture, GX_TEXMAP1);
    HuSprTexLoad(lbl_1_bss_13C0, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_13C4, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3U);
    GXSetNumTevStages(3U);
    C_MTXLightPerspective(textureProjection, camera->fov, 1.2f, 0.5f, -0.5f,
        0.5f, 0.5f);
    PSMTXInverse(Hu3DCameraMtx, inverseView);
    PSMTXConcat(inverseView, drawObj->matrix, transform);
    PSMTXConcat(textureProjection, Hu3DCameraMtx, textureMatrix);
    PSMTXConcat(textureMatrix, transform, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, 33U, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 33U, 0U, 125U);
    PSMTXTrans(transform, 0.0f,
        (textureWork->textureScrollY += textureWork->textureScrollStep), 0.0f);
    PSMTXScale(textureScale, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(textureScale, transform, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, 30U, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, 30U, 0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXSetTevColor(GX_TEVREG0, textureWork->color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0,
        GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U,
        GX_TEVPREV);
    GXSetNumIndStages(1U);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1U, 0U, GX_ITM_0);
    GXSetIndTexMtx(
        GX_ITM_0, (const f32 (*)[3]) &textureWork->matrixRow0, -1);
}

/* Reduce the screen distortion and scrolling over ninety updates, then hide its model. */
void fn_1_2D65C(OMOBJ *obj)
{
    MDPARTY_STAGE_TEXTURE_WORK *work = lbl_1_bss_1428;

    work->matrixRow0.x = work->matrixRow1.x = 0.0f;
    work->matrixRow0.y = work->matrixRow1.y = 0.0f;
    work->matrixRow0.z = work->matrixRow1.z =
        (0.09f * work->remainingFrames) / 90.0f;
    work->textureScrollStep =
        (-0.02f * work->remainingFrames) / 90.0f;
    if ((work->remainingFrames -= 1.0f) < 0.0f) {
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DLayerHookReset(1);
        obj->objFunc = NULL;
    }
}

/* Start a ninety-frame distortion effect at the requested X/Y and fixed Z of -840. */
void fn_1_2D75C(s16 textureSlot, HuVecF *position)
{
    OMOBJ *obj = lbl_1_bss_13BC;

    if (obj) {
        lbl_1_bss_1428[textureSlot].textureScrollY = 0.0f;
        lbl_1_bss_1428[textureSlot].remainingFrames = 90.0f;
        Hu3DModelPosSet(
            obj->mdlId[textureSlot], position->x, position->y, -840.0f);
        Hu3DModelAttrReset(obj->mdlId[textureSlot], HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_2D170);
        obj->objFunc = fn_1_2D65C;
    }
}

/* Load the distortion model, its textures, and its screen-capture buffer during scene setup. */
void fn_1_2D850(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    lbl_1_bss_13C0 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 134), HU_MEMNUM_OVL,
            HEAP_MODEL));
    lbl_1_bss_13C4 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 134), HU_MEMNUM_OVL,
            HEAP_MODEL));
    lbl_1_bss_13C8 = HuMemDirectMallocNum(
        HEAP_MODEL,
        GXGetTexBufferSize(320, 240, GX_TF_RGB565, GX_FALSE, 0),
        HU_MEMNUM_OVL);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 133), HU_MEMNUM_OVL,
            HEAP_MODEL));
    Hu3DModelPosSet(obj->mdlId[0], 0.0f, 0.0f,
        0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 90.0f, 0.0f,
        0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 6.0f, 6.0f, 6.0f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelMatHookSet(obj->mdlId[0], fn_1_2D1DC);
    memset(lbl_1_bss_1428, 0, sizeof(MDPARTY_STAGE_TEXTURE_WORK));
    model = &Hu3DData[obj->mdlId[0]];
    model->hookData = lbl_1_bss_1428;
    obj->objFunc = NULL;
}

/* Remove the distortion model and release its screen-capture buffer during scene teardown. */
void fn_1_2DA34(OMOBJ *obj)
{
    HU3D_MODEL *model;

    model = NULL;
    Hu3DLayerHookReset(1);
    if (obj) {
        model = &Hu3DData[obj->mdlId[0]];
        model->hookData = lbl_1_bss_1428;
        Hu3DModelKill(obj->mdlId[0]);
        if (lbl_1_bss_13C8) {
            HuMemDirectFree(lbl_1_bss_13C8);
        }
        lbl_1_bss_13C8 = NULL;
    }
    obj = NULL;
}

/* Create the screen-distortion object when the Solo stage effects are initialized. */
void fn_1_2DAE4(void)
{
    lbl_1_bss_13BC = omAddObjEx(lbl_1_bss_13B8, 4096, 1, 0, -1, fn_1_2D850);
}

/* Remove the screen-distortion object and clear its handle during effects teardown. */
void fn_1_2DB34(void)
{
    if (lbl_1_bss_13BC) {
        fn_1_2DA34(lbl_1_bss_13BC);
    }
    lbl_1_bss_13BC = NULL;
}

/* Start the first distortion model at the supplied X/Y and fixed Z of -840, then capture its
 * backdrop. */
void fn_1_2DC10(Point3d *position)
{
    OMOBJ *obj = lbl_1_bss_13BC;

    if (obj) {
        lbl_1_bss_1428[0].textureScrollY = 0.0f;
        lbl_1_bss_1428[0].remainingFrames = 90.0f;
        Hu3DModelPosSet(*obj->mdlId, position->x, position->y, -840.0f);
        Hu3DModelAttrReset(*obj->mdlId, HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_2D170);
        obj->objFunc = fn_1_2D65C;
    }
}

/* Draw four offset copies of the captured screen and refresh the capture on each layer draw. */
void fn_1_2DCD0(s16 layerNo)
{
    GXTexObj texObj;
    Mtx44 projection;
    Mtx model;
    float fade;
    s16 copyNo;

    MTXOrtho(projection, 0.0f, 480.0f, 0.0f, 640.0f, 0.0f, 10.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXInitTexObj(
        &texObj, lbl_1_bss_13C8, 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(
        &texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE,
        GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(
        GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
        GX_FALSE, GX_PTIDENTITY);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetTevOrder(
        GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(
        GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC,
        GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(
        GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
        GX_CA_ZERO);
    GXSetTevAlphaOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(
        GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

    PSMTXIdentity(model);
    GXLoadPosMtxImm(model, GX_PNMTX0);
    fade = fn_1_2D0FC(0.0f, 1.0f, lbl_1_bss_13CC, 90.0f);
    lbl_1_bss_13CC++;
    if (lbl_1_bss_13CC > 90) {
        lbl_1_bss_13CC = 90;
    }

    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    for (copyNo = 0; copyNo < 4; copyNo++) {
        GXPosition3f32(copyNo * -1, copyNo * -1, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(0.0f, 0.0f);

        GXPosition3f32(640 + copyNo, copyNo * -1, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(1.0f, 0.0f);

        GXPosition3f32(640 + copyNo, 480 + copyNo, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(1.0f, 1.0f);

        GXPosition3f32(copyNo * -1, 480 + copyNo, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(0.0f, 1.0f);
    }

    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(lbl_1_bss_13C8, GX_FALSE);
    Hu3DZClear();
}

/* Enable the captured-screen overlay's render callback during the stage transition. */
void fn_1_2E68C(void)
{
    Hu3DLayerHookSet(14, fn_1_2DCD0);
}

/* Remove the captured-screen overlay callback when its transition is finished. */
void fn_1_2E6B8(void)
{
    Hu3DLayerHookReset(14);
}

/* Start one growing, fading particle at the supplied world position and color. */
void fn_1_2E6DC(
    s16 particleNo, float lifetimeFrames, float targetScale, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1426];
    particle = model->hookData;
    data = &particle->data[particleNo];
    data->time = 1;
    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
    data->scale = 0.0f;
    data->color.r = color.r;
    data->color.g = color.g;
    data->color.b = color.b;
    data->color.a = color.a;
    /* Velocity and acceleration fields carry this effect's timing, scale, and alpha. */
    data->vel.x = 0.0f;
    data->vel.y = lifetimeFrames;
    data->accel.x = targetScale;
    data->accel.y = color.a;
    particle->dataCnt++;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* Grow active particles toward their target scales and fade them during particle updates. */
void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        if (data->time == 1) {
            data->scale = fn_1_2CF38(
                0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = fn_1_2CF38(
                data->accel.y, 0.0f, data->vel.x, data->vel.y);
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                particle->dataCnt--;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the eight-particle growing-fade effect during stage effects setup. */
void fn_1_2EA9C(void)
{
    lbl_1_bss_1426 = Hu3DParticleCreate(lbl_1_bss_1478[0], 8);
    Hu3DModelPosSet(lbl_1_bss_1426, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_1426, 1.0f, 1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_1426, 7);
    Hu3DModelAttrSet(lbl_1_bss_1426, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_1426, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_1426, fn_1_2E7DC);
    Hu3DParticleBlendModeSet(lbl_1_bss_1426, HU3D_PARTICLE_BLEND_ADDCOL);
}

/* Release the growing-fade particle model during stage effects teardown. */
void fn_1_2EBB0(void)
{
    Hu3DModelKill(lbl_1_bss_1426);
}

/* Start a colored radial particle effect with the requested position and lifetime. */
void fn_1_2EBDC(s16 effectNo, float lifetimeFrames, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_141E[effectNo]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_141E[effectNo], position);
    /* The first particle carries the color and lifetime shared by the whole burst. */
    data = particle->data;
    data->time = 1;
    data->color.r = color.r;
    data->color.g = color.g;
    data->color.b = color.b;
    data->color.a = color.a;
    data->time = 0;
    data->parManId = lifetimeFrames;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* Initialize radial streaks, rotate them, and fade them over their shared lifetime. */
void fn_1_2ECC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *sharedParticle;
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    if (particle->dataCnt == 1) {
        sharedParticle = particle->data;
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->attr |= HU3D_PARTICLE_ATTR_SCALEY;
            data->zRot = MTXDegToRad(frandmod(360));
            data->vel.z = frandmod(25) + 50;
            data->vel.x =
                -sind(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->vel.y =
                cosd(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->scale = 0.1f * data->vel.z;
            data->scaleY = 3.0f * data->vel.z;
            data->color.r = sharedParticle->color.r;
            data->color.g = sharedParticle->color.g;
            data->color.b = sharedParticle->color.b;
            data->color.a = sharedParticle->color.a;
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        sharedParticle = particle->data;
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            if ((particleNo % 2) == 0) {
                data->zRot += particleNo / 500.0f;
            } else {
                data->zRot -= particleNo / 500.0f;
            }
            data->vel.x =
                -sind(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->vel.y =
                cosd(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
            data->color.a = fn_1_2CF38(
                255.0f, 0.0f, sharedParticle->time, sharedParticle->parManId);
        }
        if ((sharedParticle->time += 1.0f) > sharedParticle->parManId) {
            particle->dataCnt = 0;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create four radial streak effect slots during stage effects setup. */
void fn_1_2F20C(void)
{
    s16 effectNo;

    for (effectNo = 0; effectNo < 4; effectNo++) {
        lbl_1_bss_141E[effectNo] = Hu3DParticleCreate(lbl_1_bss_1478[1], 16);
        Hu3DModelPosSet(
            lbl_1_bss_141E[effectNo], 200.0f * effectNo,
            200.0f, 0.0f);
        Hu3DModelScaleSet(
            lbl_1_bss_141E[effectNo], 1.0f, 1.0f,
            1.0f);
        Hu3DModelLayerSet(lbl_1_bss_141E[effectNo], 7);
        Hu3DModelAttrSet(lbl_1_bss_141E[effectNo], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_141E[effectNo], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_141E[effectNo], fn_1_2ECC4);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_141E[effectNo], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

/* Release all four radial streak particle models during effects teardown. */
void fn_1_2F3CC(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 4) {
        Hu3DModelKill(lbl_1_bss_141E[effectNo]);
        effectNo += 1;
    }
}

/* Clear the spiral particles and start their emission when the stage transition begins. */
void fn_1_2F424(void)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_141C];
    particle = model->hookData;
    particleNo = 0;
    data = particle->data;
    while (particleNo < particle->maxCnt) {
        data->time = 0;
        data->scale = 0.0f;
        particleNo += 1;
        data += 1;
    }
    particle->dataCnt = 1;
    model->attr &= MDSING_MODEL_VISIBLE_MASK;
}

/* Emit spiral particles and move them away from the camera during particle updates. */
void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 colorComponent;
    s16 particleNo;
    s16 spawnCount = 0;
    float radius = 125.0f;

    if (particle->dataCnt == 1) {
        particle->emitCnt -= 2;
        if (particle->emitCnt < 0) {
            particle->emitCnt += 360.0f;
        }
        if (particle->count > 100) {
            radius -= particle->count - 100;
            if (radius < 1.0f) {
                radius = 1.0f;
            }
        }
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            if (data->time == 0) {
                if (spawnCount < 7) {
                    /* Red and green stay fixed, but each still consumes a random draw. */
                    colorComponent = rand8() + 128;
                    colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
                    data->color.r = 255;
                    colorComponent = rand8() + 128;
                    colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
                    data->color.g = 255;
                    colorComponent = rand8() % 204;
                    colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
                    data->color.b = colorComponent;
                    if ((particleNo % 4) == 0) {
                        data->color.b = 255;
                    }
                    data->color.a = 255;
                    data->scale = 80.0f;
                    data->vel.x = (45.0f * (particleNo % 8)) + particle->emitCnt;
                    data->vel.z = 750.0f - (particle->count * 2);
                    if (data->vel.z < 1.0f) {
                        data->vel.z = 1.0f;
                    }
                    data->accel.x = 0.0f;
                    data->accel.y = frandmod(30) + 90;
                    data->pos.x = -sind(data->vel.x) * data->vel.z;
                    data->pos.y = cosd(data->vel.x) * data->vel.z;
                    data->pos.z = 800.0f;
                    data->time = 1;
                    spawnCount++;
                }
            } else if (data->time == 1) {
                data->pos.x = -sind(data->vel.x) * data->vel.z;
                data->pos.y = cosd(data->vel.x) * data->vel.z;
                data->pos.z = fn_1_2D0FC(
                    800.0f, -2000.0f, data->accel.x, data->accel.y);
                data->zRot += 0.1f;
                data->scale = fn_1_2D0FC(
                    150.0f, 50.0f, data->accel.x, data->accel.y);
                data->vel.x += 2.0f;
                data->vel.z = fn_1_2D0FC(
                    500.0f, radius, data->accel.x, data->accel.y);
                if (data->accel.x > data->accel.y - 10.0f) {
                    data->color.a = fn_1_2D0FC(
                        255.0f, 0.0f,
                        data->accel.x - (data->accel.y - 10.0f), 10.0f);
                }
                if ((data->accel.x += 1.0f) > data->accel.y) {
                    data->time = 0;
                    data->scale = 0.0f;
                }
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the spiral particle model and install its update callback during effects setup. */
void fn_1_2FC50(void)
{
    lbl_1_bss_141C = Hu3DParticleCreate(lbl_1_bss_1478[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_141C,
                    0.0f,
                    275.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_141C,
                      1.0f,
                      1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_141C, 7);
    Hu3DModelAttrSet(lbl_1_bss_141C, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_141C, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_141C, fn_1_2F4C4);
}

/* Release the spiral particle model during effects teardown. */
void fn_1_2FD50(void)
{
    Hu3DModelKill(lbl_1_bss_141C);
}

/* Start a particle burst at the supplied position with the requested travel duration. */
void fn_1_2FD7C(s16 effectNo, float durationFrames, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1414[effectNo]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_1414[effectNo], position);
    data = particle->data;
    data->time = 0;
    data->parManId = durationFrames;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* Grow a particle burst, then spread and shrink its particles during successive updates. */
void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *sharedParticle;
    HU3D_PARTICLE_DATA *data;
    HuVecF randomVector;
    HuVecF direction;
    float speed;
    s16 particleNo;
    s16 colorComponent;

    if (particle->dataCnt == 1) {
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            randomVector.x = frandmod(100) - 50;
            randomVector.y = frandmod(100) - 50;
            randomVector.z = frandmod(100) - 50;
            PSVECNormalize(&randomVector, &direction);
            speed = 200.0f;
            data->vel.x = direction.x * speed;
            data->vel.y = direction.y * speed;
            data->vel.z = direction.z * speed;
            colorComponent = rand8() + 128;
            colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.r = colorComponent;
            colorComponent = rand8() + 128;
            colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.g = colorComponent;
            colorComponent = rand8() % 128;
            colorComponent &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.b = colorComponent;
            data->color.a = 204;
            data->pos.x = data->accel.x = frandmod(4) - 2;
            data->pos.y = data->accel.y = frandmod(4) - 2;
            data->pos.z = data->accel.z = frandmod(4) - 2;
            data->scale = 0.0f;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        sharedParticle = particle->data;
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->scale = fn_1_2CF38(
                0.0f, 125.0f, sharedParticle->time, 5.0f);
        }
        if ((sharedParticle->time += 1.0f) > 15) {
            sharedParticle->time = 0;
            particle->dataCnt = 3;
        }
    } else if (particle->dataCnt == 3) {
        sharedParticle = particle->data;
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->pos.x = fn_1_2CF38(
                data->accel.x, data->vel.x, sharedParticle->time,
                sharedParticle->parManId);
            data->pos.y = fn_1_2CF38(
                data->accel.y, data->vel.y, sharedParticle->time,
                sharedParticle->parManId);
            data->pos.z = fn_1_2CF38(
                data->accel.z, data->vel.z, sharedParticle->time,
                sharedParticle->parManId);
            data->scale = fn_1_2D0FC(
                50.0f, 0.0f, sharedParticle->time, sharedParticle->parManId);
        }
        if ((sharedParticle->time += 1.0f) > sharedParticle->parManId) {
            particle->dataCnt = 0;
            model->attr |= HU3D_ATTR_DISPOFF;
        }
    }
}

/* Create four particle-burst effect slots and install their callbacks during effects setup. */
void fn_1_30730(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 4) {
        lbl_1_bss_1414[effectNo] = Hu3DParticleCreate(lbl_1_bss_1478[2], 64);
        Hu3DModelPosSet(lbl_1_bss_1414[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_1414[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_1414[effectNo], 7);
        Hu3DModelAttrSet(lbl_1_bss_1414[effectNo], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_1414[effectNo], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_1414[effectNo], fn_1_2FE48);
        Hu3DParticleBlendModeSet(lbl_1_bss_1414[effectNo], HU3D_PARTICLE_BLEND_ADDCOL);
        effectNo += 1;
    }
}

/* Release all four particle-burst models during stage effects teardown. */
void fn_1_308C4(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 4) {
        Hu3DModelKill(lbl_1_bss_1414[effectNo]);
        effectNo += 1;
    }
}

/* Allocate a growing pulse particle for the reward impact effect, if a slot is free. */
void fn_1_3091C(HuVecF *position, GXColor *color, float lifetimeFrames, float targetScale)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1412];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot == particle->maxCnt) {
        return;
    }
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = 0;
    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
    data->scale = 0.0f;
    data->vel.x = 0.0f;
    data->vel.y = lifetimeFrames;
    data->accel.x = targetScale;
    data->accel.y = (float)color->a;
}

/* Allocate a scattering reward-impact particle with a random planar direction, if a slot is
 * free. */
void fn_1_30A40(float lifetimeFrames, float travelDistance, HuVecF *position,
    GXColor *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1412];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;

    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        if (data->time == 0) {
            data->time = 2;
            break;
        }
    }
    if (particleNo == particle->maxCnt) {
        return;
    }

    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = color->a;
    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
    data->vel.x = 0.0f;
    data->vel.y = lifetimeFrames;
    data->vel.z = travelDistance;
    data->accel.x = position->x;
    data->accel.y = position->y;
    data->accel.z = position->z;
    /* These adjacent scratch fields hold a normalized planar scatter direction. */
    data->speedDecay = frandmod(100) - 50;
    data->colorIdx = frandmod(100) - 50;
    data->scaleBase = 0.0f;
    PSVECNormalize(
        (HuVecF *)&data->speedDecay, (HuVecF *)&data->speedDecay);
}

/* Animate growing pulses and scattering particles during the reward-impact particle callback. */
void fn_1_30BE0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;
    float positionScale;

    if (particle->count == 0) {
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->time = 0;
        }
    }
    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        if (data->time == 1) {
            float unusedScalar;
            float velX;
            float alpha;
            float accelX;
            float velY;
            float accelY;
            float alphaVelX;
            float alphaVelY;

            data->scale = fn_1_2CF38(0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = fn_1_2D020(0.0f, data->accel.y, data->vel.x, data->vel.y);
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        } else if (data->time == 2) {
            float unusedScalar;
            float velX;
            float velZ;
            float velY;

            positionScale = fn_1_2CF38(0.0f, data->vel.z, data->vel.x, data->vel.y);
            data->scale = (float)(frandmod(15) + 5);
            data->pos.x = data->accel.x + data->speedDecay * positionScale;
            data->pos.y = data->accel.y + data->colorIdx * positionScale
                - 3.0f * data->vel.x;
            data->pos.z = data->accel.z + data->scaleBase * positionScale;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the shared reward-impact particle model during stage effects setup. */
void fn_1_31088(void)
{
    lbl_1_bss_1412 = Hu3DParticleCreate(lbl_1_bss_1478[0], 1000);
    Hu3DModelPosSet(lbl_1_bss_1412, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_1412, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_1412, 6);
    Hu3DParticleHookSet(lbl_1_bss_1412, fn_1_30BE0);
    Hu3DParticleBlendModeSet(lbl_1_bss_1412, HU3D_PARTICLE_BLEND_ADDCOL);
}

/* Release the reward-impact particle model during effects teardown. */
void fn_1_3116C(void)
{
    Hu3DModelKill(lbl_1_bss_1412);
}

/* Show or hide one layered highlight model when its menu effect changes. */
void fn_1_31198(s16 effectNo, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_140E[effectNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140E[effectNo], HU3D_ATTR_DISPOFF);
    }
}

/* Begin fading a layered highlight in, optionally changing its position and color. */
void fn_1_31214(s16 effectNo, HuVecF *position, GXColor *color)
{
    s16 particleNo;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[effectNo]];
    particle = model->hookData;
    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_140E[effectNo], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_140E[effectNo], HU3D_ATTR_DISPOFF);
}

/* Begin fading a layered highlight out when its menu effect is dismissed. */
void fn_1_31318(s16 effectNo)
{
    s16 particleNo;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[effectNo]];
    particle = model->hookData;
    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        data->time = 2;
    }
}

/* Ease the layered highlight particles toward their visible or hidden scale and alpha targets. */
void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    MDPARTY_STAGE_PARTICLE_TARGET particleTarget[
        MDPARTY_STAGE_PARTICLE_TARGET_COUNT] = {
        { 16, 255 },
        { 16, 255 },
        { 48, 128 },
        { 128, 64 },
        { 255, 32 },
    };
    HU3D_PARTICLE_DATA *data;
    float visibleAlphaTarget;
    float visibleScaleTarget;
    float hiddenAlphaTarget;
    float hiddenScaleTarget;
    s16 particleNo;

    if (particle->count == 0) {
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->time = 0;
        }
    }
    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x =
                particleTarget[particleNo % MDPARTY_STAGE_PARTICLE_TARGET_COUNT].alpha;
            data->vel.y =
                particleTarget[particleNo % MDPARTY_STAGE_PARTICLE_TARGET_COUNT].scale;
            visibleAlphaTarget = data->vel.x;
            data->color.a =
                fn_1_2D140(data->color.a, visibleAlphaTarget, 10.0f);
            visibleScaleTarget = data->vel.y;
            data->scale = fn_1_2D140(data->scale, visibleScaleTarget, 10.0f);
        } else if (data->time == 2) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            hiddenAlphaTarget = data->vel.x;
            data->color.a = fn_1_2D140(data->color.a, hiddenAlphaTarget, 10.0f);
            hiddenScaleTarget = data->vel.y;
            data->scale = fn_1_2D140(data->scale, hiddenScaleTarget, 10.0f);
        } else {
            data->color.a = 0;
            data->scale = 0.0f;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the two layered highlight models during stage effects setup. */
void fn_1_31818(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        lbl_1_bss_140E[effectNo] = Hu3DParticleCreate(*lbl_1_bss_1478, 10);
        Hu3DModelPosSet(lbl_1_bss_140E[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_140E[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_140E[effectNo], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140E[effectNo], 2);
        Hu3DParticleHookSet(lbl_1_bss_140E[effectNo], fn_1_313A0);
        Hu3DParticleBlendModeSet(lbl_1_bss_140E[effectNo], HU3D_PARTICLE_BLEND_ADDCOL);
        effectNo += 1;
    }
}

/* Release both layered highlight models during effects teardown. */
void fn_1_31984(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        Hu3DModelKill(lbl_1_bss_140E[effectNo]);
        effectNo += 1;
    }
}

/* Show or hide one sparkling emitter model when its menu effect changes. */
void fn_1_319DC(s16 effectNo, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_140A[effectNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140A[effectNo], HU3D_ATTR_DISPOFF);
    }
}

/* Start a sparkling emitter, optionally changing its spawn center and base color. */
void fn_1_31A58(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_140A[effectNo]];
    particle = model->hookData;
    particle->dataCnt = 1;
    /* The emitter position vector carries its base RGB color; spawnCenter carries position. */
    if (color != NULL) {
        particle->pos.x = color->r;
        particle->pos.y = color->g;
        particle->pos.z = color->b;
    }
    if (position != NULL) {
        particle->spawnCenter.x = position->x;
        particle->spawnCenter.y = position->y;
        particle->spawnCenter.z = position->z;
    }
    Hu3DModelAttrReset(lbl_1_bss_140A[effectNo], HU3D_ATTR_DISPOFF);
}

/* Stop a sparkling emitter from creating new particles while its active particles finish. */
void fn_1_31B90(s16 effectNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_140A[effectNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Emit up to four sparkles per update and shrink them over their individual lifetimes. */
void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;
    s16 spawnCount = 0;
    float colorComponent;
    float brightnessOrFade;

    if (particle->count == 0) {
        for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
             particleNo++, data++) {
            data->time = 0;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }

    for (particleNo = 0, data = particle->data; particleNo < particle->maxCnt;
         particleNo++, data++) {
        if (data->time == 0 && particle->dataCnt == 1 && spawnCount <= 3) {
            spawnCount++;
            data->time = 1;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(30) + 60;
            data->accel.x = frandmod(100) - 50;
            data->accel.y = -frandmod(100) - 50;
            data->accel.z = frandmod(100) - 50;
            PSVECNormalize(&data->accel, &data->accel);
            data->accel.x *= 2.0f;
            data->accel.y *= 5.0f;
            data->accel.z *= 2.0f;
            data->pos.x = particle->spawnCenter.x + frandmod(100) - 50;
            data->pos.y = particle->spawnCenter.y + frandmod(100) - 50;
            data->pos.z = particle->spawnCenter.z + frandmod(100) - 50;
            brightnessOrFade = frandmod(128);
            colorComponent = particle->pos.x + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            data->color.r = colorComponent;
            colorComponent = particle->pos.y + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            data->color.g = colorComponent;
            colorComponent = particle->pos.z + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            data->color.b = colorComponent;
            data->color.a = 0;
        } else if (data->time == 1) {
            data->pos.x += data->accel.x;
            data->pos.y += data->accel.y;
            data->pos.z += data->accel.z;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(127) + 128;
            }
            brightnessOrFade = fn_1_2D0FC(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = frandmod(90) * brightnessOrFade;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create two sparkling emitter models during stage effects setup. */
void fn_1_321E4(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        lbl_1_bss_140A[effectNo] = Hu3DParticleCreate(lbl_1_bss_1478[3], 256);
        Hu3DModelPosSet(lbl_1_bss_140A[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_140A[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_140A[effectNo], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140A[effectNo], 2);
        Hu3DParticleHookSet(lbl_1_bss_140A[effectNo], fn_1_31BE4);
        Hu3DParticleBlendModeSet(lbl_1_bss_140A[effectNo], HU3D_PARTICLE_BLEND_ADDCOL);
        effectNo += 1;
    }
}

/* Release both sparkling emitter models during effects teardown. */
void fn_1_32350(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        Hu3DModelKill(lbl_1_bss_140A[effectNo]);
        effectNo += 1;
    }
}

/* Show, fade out, or immediately hide a paired highlight and sparkling menu effect. */
void fn_1_323A8(s16 modelNo, HuVecF *position, s16 displayMode)
{
    GXColor colors[2] = {
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };
    if (displayMode == 1) {
        fn_1_31214(modelNo, position, &colors[modelNo]);

        fn_1_31A58(modelNo, position, &colors[modelNo]);
    } else if (displayMode == 0) {
        fn_1_31318(modelNo);

        fn_1_31B90(modelNo);
    } else if (displayMode == 2) {
        Hu3DModelAttrSet(lbl_1_bss_140E[modelNo], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_140A[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Show or hide all five emitter models belonging to one effect group. */
void fn_1_3276C(s16 groupNo, s16 show)
{
    s16 emitterNo;

    for (emitterNo = 0; emitterNo < 5; emitterNo++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_13CE[groupNo][emitterNo], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_13CE[groupNo][emitterNo], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Start all five emitters in a group, optionally changing their spawn center and color. */
void fn_1_32820(s16 groupNo, HuVecF *spawnPosition, GXColor *color)
{
    s16 emitterNo;
    HU3D_MODEL *model;
    HU3D_PARTICLE *emitter;

    for (emitterNo = 0; emitterNo < 5; emitterNo++) {
        model = &Hu3DData[lbl_1_bss_13CE[groupNo][emitterNo]];
        emitter = model->hookData;
        emitter->dataCnt = 1;
        if (color != NULL) {
            emitter->pos.x = color->r;
            emitter->pos.y = color->g;
            emitter->pos.z = color->b;
        }
        if (spawnPosition != NULL) {
            /* This emitter vector is used as the center for new particles' spawn positions. */
            emitter->spawnCenter.x = spawnPosition->x;
            emitter->spawnCenter.y = spawnPosition->y;
            emitter->spawnCenter.z = spawnPosition->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_13CE[groupNo][emitterNo], HU3D_ATTR_DISPOFF);
    }
}

/* Stop new particles from all five emitters in a group without clearing existing particles. */
void fn_1_32988(s16 groupNo)
{
    s16 emitterNo;
    HU3D_MODEL *model;
    s16 *emissionState;

    for (emitterNo = 0; emitterNo < 5; emitterNo++) {
        model = &Hu3DData[lbl_1_bss_13CE[groupNo][emitterNo]];
        /* The first halfword is the particle emitter's dataCnt emission state. */
        emissionState = model->hookData;
        *emissionState = 0;
    }
}

/* Emit at most one rising sparkle per model update and fade it over its lifetime. */
void fn_1_32A08(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleData;
    s16 particleNo;
    s16 spawnCount = 0;
    float colorComponent;
    float brightnessOrFade;

    if (emitter->count == 0) {
        for (particleNo = 0, particleData = emitter->data; particleNo < emitter->maxCnt;
             particleNo++, particleData++) {
            particleData->time = 0;
        }
        emitter->dataCnt = 1;
        emitter->pos.x = 255.0f;
        emitter->pos.y = 255.0f;
        emitter->pos.z = 255.0f;
    }

    for (particleNo = 0, particleData = emitter->data; particleNo < emitter->maxCnt;
         particleNo++, particleData++) {
        if (particleData->time == 0 && emitter->dataCnt == 1 && spawnCount < 1) {
            spawnCount++;
            particleData->time = 1;
            particleData->vel.x = 0.0f;
            particleData->vel.y = frandmod(30) + 30;
            particleData->accel.x = frandmod(100) - 50;
            particleData->accel.y = -frandmod(100) - 50;
            particleData->accel.z = frandmod(100) - 50;
            PSVECNormalize(&particleData->accel, &particleData->accel);
            particleData->accel.x *= 2.0f;
            particleData->accel.z *= 2.0f;
            particleData->colorIdx = frandmod(10) + 5;
            particleData->pos.x =
                (emitter->spawnCenter.x + (float)frandmod(100)) -
                50.0f;
            particleData->pos.y = emitter->spawnCenter.y;
            particleData->pos.z =
                (emitter->spawnCenter.z + (float)frandmod(100)) -
                50.0f;
            brightnessOrFade = frandmod(32);
            colorComponent = emitter->pos.x + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.r = colorComponent;
            colorComponent = emitter->pos.y + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.g = colorComponent;
            colorComponent = emitter->pos.z + brightnessOrFade;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.b = colorComponent;
            particleData->color.a = 0;
        } else if (particleData->time == 1) {
            particleData->pos.y += particleData->colorIdx;
            particleData->colorIdx += particleData->accel.y;
            if (rand8() % 5 == 0) {
                particleData->zRot = MTXDegToRad(frandmod(360));
                particleData->color.a = frandmod(127) + 128;
            }
            brightnessOrFade = fn_1_2D0FC(1.0f, 0.0f, particleData->vel.x, particleData->vel.y);
            particleData->scale = 40.0f * brightnessOrFade;
            if (++particleData->vel.x > particleData->vel.y) {
                particleData->time = 0;
                particleData->scale = 0.0f;
                particleData->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        emitter->data, emitter->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

static const MDMinidllHalfwordTemplate lbl_1_rodata_4A8 = {{10, 10, 10, 10, 256}};

/* Create six groups of five particle emitters during stage effects setup. */
void fn_1_32FB4(void)
{
    MDMinidllHalfwordTemplate particleCounts;
    s16 emitterNo;
    s16 groupNo;

    particleCounts = lbl_1_rodata_4A8;
    for (groupNo = 0; groupNo < 6; groupNo += 1) {
        for (emitterNo = 0; emitterNo < 5; emitterNo += 1) {
            lbl_1_bss_13CE[groupNo][emitterNo] = Hu3DParticleCreate(
                lbl_1_bss_1478[emitterNo + 4],
                particleCounts.particleCount[emitterNo]);
            Hu3DModelPosSet(lbl_1_bss_13CE[groupNo][emitterNo],
                0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_13CE[groupNo][emitterNo],
                1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(lbl_1_bss_13CE[groupNo][emitterNo], HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_13CE[groupNo][emitterNo], 2);
            Hu3DParticleHookSet(lbl_1_bss_13CE[groupNo][emitterNo], fn_1_32A08);
            Hu3DParticleBlendModeSet(lbl_1_bss_13CE[groupNo][emitterNo],
                                     HU3D_PARTICLE_BLEND_ADDCOL);
        }
    }
}

/* Release every grouped emitter model during stage effects teardown. */
void fn_1_331E0(void)
{
    s16 groupNo;
    s16 emitterNo;

    for (groupNo = 0; groupNo < 6; groupNo++) {
        emitterNo = 0;
        while (emitterNo < 5) {
            Hu3DModelKill(lbl_1_bss_13CE[groupNo][emitterNo]);
            emitterNo += 1;
        }
    }
}

/* Load the stage particle textures and create the Solo menu's visual effect models. */
void fn_1_33264(HUPROCESS *process)
{
    s16 animIndex;

    lbl_1_bss_13B8 = process;
    fn_1_2DAE4();
    animIndex = 0;
    while (animIndex < 9) {
        lbl_1_bss_1478[animIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_CD8[animIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        animIndex += 1;
    }
    fn_1_2EA9C();
    fn_1_2F20C();
    fn_1_2FC50();
    fn_1_30730();
    fn_1_31088();
    fn_1_31818();
    fn_1_321E4();
    fn_1_32FB4();
}

/* Release all stage effect models and remove their screen-capture callbacks. */
void fn_1_33D84(void)
{
    fn_1_2EBB0();
    fn_1_2F3CC();
    fn_1_2FD50();
    fn_1_308C4();
    Hu3DModelKill(lbl_1_bss_1412);
    fn_1_31984();
    fn_1_32350();
    fn_1_331E0();
    fn_1_2E6B8();
    fn_1_2DB34();
}

/* Start the board-card transition's distortion at fixed Z of -840 and its blue pulse
 * at the supplied position. */
void fn_1_33FC0(HuVecF *position)
{
    GXColor savedColor;
    GXColor *colorBuffer;
    HU3D_PARTICLE_DATA *particleData;
    OMOBJ *object;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;

    object = lbl_1_bss_13BC;
    if (object) {
        lbl_1_bss_1428[0].textureScrollY = 0.0f;
        lbl_1_bss_1428[0].remainingFrames = 90.0f;
        Hu3DModelPosSet(*object->mdlId, position->x, position->y, -840.0f);
        Hu3DModelAttrReset(*object->mdlId, HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_2D170);
        object->objFunc = fn_1_2D65C;
    }
    {
    GXColor sourceColor = {128, 128, 255, 128};
    colorBuffer = &sourceColor;
    savedColor = *colorBuffer;
    model = &Hu3DData[lbl_1_bss_1426];
    particle = model->hookData;
    particleData = &particle->data[5];
    particleData->time = 1;
    particleData->pos.x = position->x;
    particleData->pos.y = position->y;
    particleData->pos.z = position->z;
    particleData->scale = 0.0f;
    particleData->color = savedColor;
    particleData->vel.x = 0.0f;
    particleData->vel.y = 120.0f;
    particleData->accel.x = 3000.0f;
    particleData->accel.y = (f32) savedColor.a;
    particle->dataCnt += 1;
    model->attr &= MDSING_MODEL_VISIBLE_MASK;
    }
}

/* Play a player-colored pulse and radial streak burst when a character is confirmed. */
void fn_1_341B8(s16 particleNo, Point3d *position, s16 colorNo)
{
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    Point3d positionOffset;
    GXColor firstColor;
    GXColor secondColor;
    HU3D_PARTICLE *firstParticle;
    HU3D_MODEL *firstModel;
    HU3D_PARTICLE *secondParticle;
    HU3D_MODEL *secondModel;
    HU3D_PARTICLE_DATA *firstData;
    HU3D_PARTICLE_DATA *secondData;

    positionOffset.x = position->x;
    positionOffset.y = position->y;
    positionOffset.z = 10.0f + position->z;
    firstColor = palette[colorNo];
    firstModel = &Hu3DData[lbl_1_bss_1426];
    firstParticle = firstModel->hookData;
    firstData = &firstParticle->data[particleNo];
    firstData->time = 1;
    firstData->pos.x = positionOffset.x;
    firstData->pos.y = positionOffset.y;
    firstData->pos.z = positionOffset.z;
    firstData->scale = 0.0f;
    firstData->color = firstColor;
    firstData->vel.x = 0.0f;
    firstData->vel.y = 10.0f;
    firstData->accel.x = 500.0f;
    firstData->accel.y = (f32) firstColor.a;
    firstParticle->dataCnt += 1;
    firstModel->attr &= MDSING_MODEL_VISIBLE_MASK;
    secondColor = palette[colorNo];
    secondModel = &Hu3DData[lbl_1_bss_141E[particleNo]];
    secondParticle = secondModel->hookData;
    Hu3DModelPosSetV(lbl_1_bss_141E[particleNo], &positionOffset);
    secondData = secondParticle->data;
    secondData->time = 1;
    secondData->color = secondColor;
    secondData->time = 0;
    secondData->parManId = 10;
    secondParticle->dataCnt = 1;
    secondModel->attr &= MDSING_MODEL_VISIBLE_MASK;
}

/* Play a colored pulse with a spreading burst or radial streaks for the requested effect mode. */
void fn_1_344B4(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode)
{
    GXColor palette[5] = {
        { 254, 77, 75, 255 },
        { 50, 127, 200, 255 },
        { 199, 175, 0, 255 },
        { 52, 192, 63, 255 },
        { 159, 93, 200, 255 },
    };
    HuVecF particlePosition;
    GXColor color;
    GXColor secondColor;
    GXColor thirdColor;
    HU3D_PARTICLE_DATA *firstData;
    HU3D_PARTICLE_DATA *alternateFirstData;
    HU3D_PARTICLE_DATA *alternateSecondData;
    HU3D_PARTICLE *firstParticle;
    HU3D_MODEL *firstModel;
    HU3D_PARTICLE_DATA *zeroModeData;
    HU3D_PARTICLE *zeroModeParticle;
    HU3D_MODEL *zeroModeModel;
    HU3D_PARTICLE *alternateFirstParticle;
    HU3D_MODEL *alternateFirstModel;
    HU3D_PARTICLE *alternateSecondParticle;
    HU3D_MODEL *alternateSecondModel;

    particlePosition.x = position->x;
    particlePosition.y = position->y;
    particlePosition.z = position->z;
    Hu3DZClearLayerSet(7);
    if (mode == 0) {
        color = palette[colorNo];
        firstModel = &Hu3DData[lbl_1_bss_1426];
        firstParticle = firstModel->hookData;
        firstData = &firstParticle->data[particleNo];
        firstData->time = 1;
        firstData->pos.x = particlePosition.x;
        firstData->pos.y = particlePosition.y;
        firstData->pos.z = particlePosition.z;
        firstData->scale = 0.0f;
        firstData->color = color;
        firstData->vel.x = 0.0f;
        firstData->vel.y = 20.0f;
        firstData->accel.x = 300.0f;
        firstData->accel.y = (f32) color.a;
        firstParticle->dataCnt += 1;
        firstModel->attr &= MDSING_MODEL_VISIBLE_MASK;
        zeroModeModel = &Hu3DData[lbl_1_bss_1414[particleNo]];
        zeroModeParticle = zeroModeModel->hookData;
        Hu3DModelPosSetV(lbl_1_bss_1414[particleNo], &particlePosition);
        zeroModeData = zeroModeParticle->data;
        zeroModeData->time = 0;
        zeroModeData->parManId = 30;
        zeroModeParticle->dataCnt = 1;
        zeroModeModel->attr &= MDSING_MODEL_VISIBLE_MASK;
        return;
    }
    secondColor = palette[colorNo];
    alternateFirstModel = &Hu3DData[lbl_1_bss_1426];
    alternateFirstParticle = alternateFirstModel->hookData;
    alternateFirstData = &alternateFirstParticle->data[particleNo];
    alternateFirstData->time = 1;
    alternateFirstData->pos.x = particlePosition.x;
    alternateFirstData->pos.y = particlePosition.y;
    alternateFirstData->pos.z = particlePosition.z;
    alternateFirstData->scale = 0.0f;
    alternateFirstData->color = secondColor;
    alternateFirstData->vel.x = 0.0f;
    alternateFirstData->vel.y = 60.0f;
    alternateFirstData->accel.x = 300.0f;
    alternateFirstData->accel.y = (f32) secondColor.a;
    alternateFirstParticle->dataCnt += 1;
    alternateFirstModel->attr &= MDSING_MODEL_VISIBLE_MASK;
    thirdColor = palette[colorNo];
    alternateSecondModel = &Hu3DData[lbl_1_bss_141E[particleNo]];
    alternateSecondParticle = alternateSecondModel->hookData;
    Hu3DModelPosSetV(lbl_1_bss_141E[particleNo], &particlePosition);
    alternateSecondData = alternateSecondParticle->data;
    alternateSecondData->time = 1;
    alternateSecondData->color = thirdColor;
    alternateSecondData->time = 0;
    alternateSecondData->parManId = 30;
    alternateSecondParticle->dataCnt = 1;
    alternateSecondModel->attr &= MDSING_MODEL_VISIBLE_MASK;
}

/* Start the departure sequence's captured-screen overlay and spiral particles together. */
void fn_1_34958(void)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 particleNo;
    HU3D_MODEL *model;

    Hu3DLayerHookSet(14, fn_1_2DCD0);
    model = &Hu3DData[lbl_1_bss_141C];
    particle = model->hookData;
    particleNo = 0;
    data = particle->data;
    while (particleNo < particle->maxCnt) {
        data->time = 0;
        data->scale = 0.0f;
        particleNo += 1;
        data += 1;
    }
    particle->dataCnt = 1;
    model->attr &= MDSING_MODEL_VISIBLE_MASK;
}

/* Add 100 to the supplied position's Z, then play a pulse and six scattering particles
 * for the falling reward impact. */
void fn_1_34A18(HuVecF *position)
{
    GXColor pulseColor = { 40, 40, 40, 128 };
    GXColor goldScatterColor = { 128, 128, 0, 128 };
    GXColor grayScatterColor = { 128, 128, 128, 128 };
    f32 pulseLifetimeFrames;
    s16 particleNo;

    position->z += 100.0f;
    pulseLifetimeFrames = frandmod(6) + 6;
    fn_1_3091C(position, &pulseColor, pulseLifetimeFrames, 200.0f);
    for (particleNo = 0; particleNo < 6; particleNo++) {
        if (particleNo % 2 == 0) {
            fn_1_30A40(
                frandmod(5) + 10, 50.0f, position, &goldScatterColor);
        } else {
            fn_1_30A40(
                frandmod(5) + 10, 50.0f, position, &grayScatterColor);
        }
    }
}

/* Start, stop emission, or hide a grouped sparkle effect using the requested menu color. */
void fn_1_34FA4(s16 groupNo, HuVecF *position, s16 mode, s16 colorNo)
{
    GXColor colors[7] = {
        { 254, 77, 75, 0 },
        { 50, 127, 200, 0 },
        { 199, 175, 0, 0 },
        { 52, 192, 63, 0 },
        { 159, 93, 200, 0 },
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };

    if (mode == 1) {
        fn_1_32820(groupNo, position, &colors[colorNo]);
    } else if (mode == 0) {
        fn_1_32988(groupNo);
    } else if (mode == 2) {
        fn_1_3276C(groupNo, FALSE);
    }
}

/* Particle texture resources: pulse, streak, burst, sparkles, then five grouped emitters. */
s32 lbl_1_data_CD8[9] = {
    DATANUM(DATA_mdsing, 136), DATANUM(DATA_mdsing, 137),
    DATANUM(DATA_mdsing, 135), DATANUM(DATA_mdsing, 138),
    DATANUM(DATA_mdsing, 156), DATANUM(DATA_mdsing, 156),
    DATANUM(DATA_mdsing, 156), DATANUM(DATA_mdsing, 156),
    DATANUM(DATA_mdsing, 156),
};

/* Loaded particle textures shared by all stage effect models. */
ANIMDATA *lbl_1_bss_1478[9];
/* Texture scroll, distortion matrix, color, and remaining frames for the screen effect. */
MDPARTY_STAGE_TEXTURE_WORK lbl_1_bss_1428[2];
/* Shared growing-fade particles; each slot has its own scale, alpha, and lifetime. */
HU3D_MODELID lbl_1_bss_1426;
/* Four radial streak slots, the departure spiral, and four spreading burst slots. */
HU3D_MODELID lbl_1_bss_141E[4];
HU3D_MODELID lbl_1_bss_141C;
HU3D_MODELID lbl_1_bss_1414[4];
/* Shared pulse and scattering-particle pool for falling reward impacts. */
HU3D_MODELID lbl_1_bss_1412;
/* Paired layered highlights and sparkling emitters used by the menu effects. */
HU3D_MODELID lbl_1_bss_140E[2];
HU3D_MODELID lbl_1_bss_140A[2];
/* Six effect groups, each containing five independently textured particle emitters. */
HU3D_MODELID lbl_1_bss_13CE[6][5];
/* Captured-screen overlay fade timer, clamped to ninety frames. */
s16 lbl_1_bss_13CC;
/* Half-resolution captured screen and the two distortion textures. */
void *lbl_1_bss_13C8;
ANIMDATA *lbl_1_bss_13C4;
ANIMDATA *lbl_1_bss_13C0;
/* Distortion model object and the object manager that owns stage effect objects. */
OMOBJ *lbl_1_bss_13BC;
HUPROCESS *lbl_1_bss_13B8;
