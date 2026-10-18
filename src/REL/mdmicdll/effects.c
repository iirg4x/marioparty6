/* Scene distortion, particles and effect groups used by microphone mode. */
#include "dolphin/math.h"
#include <string.h>
#include "dolphin/os.h"
#include "datadir_enum.h"
#include "game/main.h"
#include "game/data.h"
#include "game/memory.h"
#include "REL/mdmicdll/stage.h"

#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)
#define MDMIC_MODEL_VISIBLE_MASK 0xFFFFFFFE
enum {
    MDMIC_PARTICLE_TARGET_COUNT = 5,
    MDPARTY_COLOR_COMPONENT_MASK = 255,
};

extern s32 frandmod(s32 modulus);

/* Particle callbacks use this sine curve to interpolate values over an effect's frames. */
float fn_1_35A10(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Particle callbacks use this linear interpolation for timed effect values. */
float fn_1_35AF8(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((time / duration) * (endValue - startValue));
}

/* Particle callbacks use this smoothing step to approach a target value. */
float fn_1_35B3C(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

/* The overlay effect registers this layer hook to refresh the half-size screen texture. */
void fn_1_35B6C(s16 layerNo)
{
    if (lbl_1_bss_A70) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_A70, GX_FALSE);
    }
}

/* The overlay model's material hook binds the captured screen and scrolling textures. */
void fn_1_35BD8(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    HU3D_CAMERA *camera;
    MDMIC_TEXTURE_WORK *work;
    Mtx tmpMtx;
    Mtx scaleMtx;
    Mtx texMtx;
    Mtx lightMtx;
    Mtx invMtx;
    GXTexObj texObj;

    work = drawObj->model->hookData;
    camera = Hu3DCamera;

    GXInitTexObj(
        &texObj, lbl_1_bss_A70, 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(
        &texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE,
        GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP1);
    HuSprTexLoad(lbl_1_bss_A68, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    HuSprTexLoad(lbl_1_bss_A6C, 0, GX_TEXMAP3, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumTexGens(3);
    GXSetNumTevStages(3);

    C_MTXLightPerspective(
        lightMtx, camera->fov, 1.2f, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXInverse(Hu3DCameraMtx, invMtx);
    PSMTXConcat(invMtx, drawObj->matrix, tmpMtx);
    PSMTXConcat(lightMtx, Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(
        GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE,
        GX_PTIDENTITY);

    PSMTXTrans(
        tmpMtx, 0.0f, work->textureScrollY += work->textureScrollStep, 0.0f);
    PSMTXScale(scaleMtx, 0.8f, 0.8f, 1.0f);
    PSMTXConcat(scaleMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(
        GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE,
        GX_PTIDENTITY);
    GXSetTexCoordGen2(
        GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
        GX_PTIDENTITY);

    GXSetTevColor(GX_TEVREG0, work->color);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(
        GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);
    GXSetTevAlphaIn(
        GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(
        GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);
    GXSetTevAlphaIn(
        GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(
        GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP3, GX_COLOR0);
    GXSetTevColorIn(
        GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV);
    GXSetTevColorOp(
        GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);
    GXSetTevAlphaIn(
        GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(
        GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
        GX_TEVPREV);

    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(
        GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, work->indirectMatrix, -1);
}

/* The overlay object's update callback reduces distortion and scroll speed, then hides the
 * overlay when its lifetime expires. */
void fn_1_36058(OMOBJ *obj)
{
    MDMIC_TEXTURE_WORK *work = lbl_1_bss_AD0;

    work->indirectMatrix[0][0] = work->indirectMatrix[1][0] = 0.0f;
    work->indirectMatrix[0][1] = work->indirectMatrix[1][1] = 0.0f;
    work->indirectMatrix[0][2] = work->indirectMatrix[1][2] =
        (0.09f * work->remainingFrames) / 90.0f;
    work->textureScrollStep = (-0.02f * work->remainingFrames) / 90.0f;
    if ((work->remainingFrames -= 1.0f) < 0.0f) {
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DLayerHookReset(1);
        obj->objFunc = NULL;
    }
}

/* Effect callers show an overlay at the supplied world x/y position and a fixed depth. */
void fn_1_36158(s16 textureSlot, HuVecF *position)
{
    OMOBJ *obj = lbl_1_bss_A64;

    if (obj) {
        lbl_1_bss_AD0[textureSlot].textureScrollY = 0.0f;
        lbl_1_bss_AD0[textureSlot].remainingFrames = 90.0f;
        /* The overlay model stays at this fixed depth regardless of input z. */
        Hu3DModelPosSet(
            obj->mdlId[textureSlot], position->x, position->y, -840.0f);
        Hu3DModelAttrReset(obj->mdlId[textureSlot], HU3D_ATTR_DISPOFF);
        Hu3DLayerHookSet(1, fn_1_35B6C);
        obj->objFunc = fn_1_36058;
    }
}

/* The object manager calls this callback to create and configure the capture overlay model. */
void fn_1_3624C(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    lbl_1_bss_A68 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 103), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A6C = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 103), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A70 = HuMemDirectMallocNum(
        HEAP_MODEL,
        GXGetTexBufferSize(320, 240, GX_TF_RGB565, GX_FALSE, 0),
        HU_MEMNUM_OVL);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdmic, 102), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 90.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 6.0f, 6.0f, 6.0f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelMatHookSet(obj->mdlId[0], fn_1_35BD8);
    memset(lbl_1_bss_AD0, 0, sizeof(lbl_1_bss_AD0[0]));
    model = &Hu3DData[obj->mdlId[0]];
    model->hookData = lbl_1_bss_AD0;
    obj->objFunc = NULL;
}

/* Overlay cleanup calls this to release the model and its framebuffer texture. */
void fn_1_36430(OMOBJ *obj)
{
    HU3D_MODEL *model;

    model = NULL;
    Hu3DLayerHookReset(1);
    if (obj) {
        model = &Hu3DData[obj->mdlId[0]];
        model->hookData = lbl_1_bss_AD0;
        Hu3DModelKill(obj->mdlId[0]);
        if (lbl_1_bss_A70) {
            HuMemDirectFree(lbl_1_bss_A70);
        }
        lbl_1_bss_A70 = NULL;
    }
    obj = NULL;
}

/* Particle-effect setup allocates the object that owns the capture overlay. */
void fn_1_364E0(void)
{
    lbl_1_bss_A64 = omAddObjEx(lbl_1_bss_A60, 4096, 1, 0, -1, fn_1_3624C);
}

/* Effect cleanup releases the capture overlay resources and clears its owner reference. */
void fn_1_36530(void)
{
    if (lbl_1_bss_A64) {
        fn_1_36430(lbl_1_bss_A64);
    }
    lbl_1_bss_A64 = NULL;
}

/* Point-based effect callers show the overlay at this supplied position. */
void fn_1_3660C(Point3d *position)
{
    fn_1_36158(0, position);
}

/* Render-layer hook registered by fn_1_37088 draws the fading screen texture. */
void fn_1_366CC(s16 layerNo)
{
    GXTexObj texObj;
    Mtx44 projection;
    Mtx model;
    float fade;
    s16 i;

    MTXOrtho(projection, 0.0f, 480.0f, 0.0f, 640.0f, 0.0f, 10.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXInitTexObj(
        &texObj, lbl_1_bss_A70, 320, 240, GX_TF_RGB565, GX_CLAMP,
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
    GXSetBlendMode(
        GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
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

    PSMTXIdentity(model);
    GXLoadPosMtxImm(model, GX_PNMTX0);
    fade = fn_1_35AF8(0.0f, 1.0f, lbl_1_bss_A74, 90.0f);
    lbl_1_bss_A74++;
    if (lbl_1_bss_A74 > 90) {
        lbl_1_bss_A74 = 90;
    }

    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(i * -1, i * -1, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(0.0f, 0.0f);

        GXPosition3f32(640 + i, i * -1, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(1.0f, 0.0f);

        GXPosition3f32(640 + i, 480 + i, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(1.0f, 1.0f);

        GXPosition3f32(i * -1, 480 + i, 0.0f);
        GXColor4u8(
            255.0f - (8.0f * fade), 255.0f - (8.0f * fade),
            255.0f - (8.0f * fade), 128.0f * fade);
        GXTexCoord2f32(0.0f, 1.0f);
    }

    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(lbl_1_bss_A70, GX_FALSE);
    Hu3DZClear();
}

/* Effect setup registers the screen-texture fade on render layer 14. */
void fn_1_37088(void)
{
    Hu3DLayerHookSet(14, fn_1_366CC);
}

/* Effect cleanup removes the screen-texture fade hook from render layer 14. */
void fn_1_370B4(void)
{
    Hu3DLayerHookReset(14);
}

/* Effect callers initialize one timed particle at a world-space position. */
void fn_1_370D8(
    s16 particleNo, float lifetime, float targetScale, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_ACC];
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
    data->vel.x = 0.0f;
    data->vel.y = lifetime;
    data->accel.x = targetScale;
    data->accel.y = color.a;
    particle->dataCnt++;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* The timed-particle model's draw callback advances particle size, opacity and lifetime. */
void fn_1_371D8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            data->scale = fn_1_35A10(
                0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = fn_1_35A10(
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

/* Particle-effect setup creates the timed particle model and installs its draw callback. */
void fn_1_37498(void)
{
    lbl_1_bss_ACC = Hu3DParticleCreate(lbl_1_bss_B20[0], 8);
    Hu3DModelPosSet(lbl_1_bss_ACC, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_ACC, 1.0f, 1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_ACC, 7);
    Hu3DModelAttrSet(lbl_1_bss_ACC, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_ACC, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_ACC, fn_1_371D8);
    Hu3DParticleBlendModeSet(lbl_1_bss_ACC, 1U);
}

/* Particle-effect cleanup destroys the timed particle model. */
void fn_1_375AC(void)
{
    Hu3DModelKill(lbl_1_bss_ACC);
}

/* Effect callers activate one particle model with a position, lifetime and color. */
void fn_1_375D8(s16 effectNo, float duration, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_AC4[effectNo]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_AC4[effectNo], position);
    data = particle->data;
    data->time = 1;
    data->color.r = color.r;
    data->color.g = color.g;
    data->color.b = color.b;
    data->color.a = color.a;
    data->time = 0;
    data->parManId = duration;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* The four radial-burst models use this particle hook to rotate their streaks and fade them. */
void fn_1_376C0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *first;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->dataCnt <= 0) {
        model->attr |= HU3D_ATTR_DISPOFF;
        return;
    }
    if (particle->dataCnt == 1) {
        first = particle->data;
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->attr |= HU3D_PARTICLE_ATTR_SCALEY;
            data->zRot = MTXDegToRad(frandmod(360));
            data->vel.z = frandmod(25) + 50;
            data->vel.x =
                -sind(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->vel.y =
                cosd(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->scale = 0.1f * data->vel.z;
            data->scaleY = 3.0f * data->vel.z;
            data->color.r = first->color.r;
            data->color.g = first->color.g;
            data->color.b = first->color.b;
            data->color.a = first->color.a;
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        first = particle->data;
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            if ((i % 2) == 0) {
                data->zRot += i / 500.0f;
            } else {
                data->zRot -= i / 500.0f;
            }
            data->vel.x =
                -sind(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->vel.y =
                cosd(MTXRadToDeg(data->zRot)) * data->vel.z;
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
            /* The burst fade starts at full opacity regardless of the activation color's alpha. */
            data->color.a = fn_1_35A10(
                255.0f, 0.0f, first->time, first->parManId);
        }
        if ((first->time += 1.0f) > first->parManId) {
            particle->dataCnt = 0;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Particle-effect setup creates four burst models and installs their update hook. */
void fn_1_37C08(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_AC4[i] = Hu3DParticleCreate(lbl_1_bss_B20[1], 16);
        Hu3DModelPosSet(
            lbl_1_bss_AC4[i], 200.0f * i, 200.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_AC4[i], 1.0f, 1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_AC4[i], 7);
        Hu3DModelAttrSet(lbl_1_bss_AC4[i], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_AC4[i], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_AC4[i], fn_1_376C0);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_AC4[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

/* Particle-effect cleanup destroys the four burst particle models. */
void fn_1_37DC8(void)
{
    s16 i;

    i = 0;
    while (i < 4) {
        Hu3DModelKill(lbl_1_bss_AC4[i]);
        i += 1;
    }
}

/* Effect setup clears every particle slot and marks the emitter ready to run. */
void fn_1_37E20(void)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 particleNo;
    HU3D_MODEL *model;

    model = &Hu3DData[lbl_1_bss_AC2];
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
    model->attr &= MDMIC_MODEL_VISIBLE_MASK;
}

/* The particle update hook moves the rising particles and spawns new ones in free slots. */
void fn_1_37EC0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 color;
    s16 i;
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
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            if (data->time == 0) {
                if (spawnCount < 7) {
                    color = rand8() + 128;
                    color &= MDPARTY_COLOR_COMPONENT_MASK;
                    /* The sampled red value is discarded; red is forced to full intensity. */
                    data->color.r = 255;
                    color = rand8() + 128;
                    color &= MDPARTY_COLOR_COMPONENT_MASK;
                    /* The sampled green value is also discarded. */
                    data->color.g = 255;
                    color = rand8() % 204;
                    color &= MDPARTY_COLOR_COMPONENT_MASK;
                    data->color.b = color;
                    if ((i % 4) == 0) {
                        data->color.b = 255;
                    }
                    data->color.a = 255;
                    data->scale = 80.0f;
                    data->vel.x = (45.0f * (i % 8)) + particle->emitCnt;
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
                data->pos.z = fn_1_35AF8(
                    800.0f, -2000.0f, data->accel.x, data->accel.y);
                data->zRot += 0.1f;
                data->scale = fn_1_35AF8(
                    150.0f, 50.0f, data->accel.x, data->accel.y);
                data->vel.x += 2.0f;
                data->vel.z = fn_1_35AF8(
                    500.0f, radius, data->accel.x, data->accel.y);
                if (data->accel.x > data->accel.y - 10.0f) {
                    data->color.a = fn_1_35AF8(
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

/* Particle-effect setup creates the rising model and installs its draw callback. */
void fn_1_3864C(void)
{
    lbl_1_bss_AC2 = Hu3DParticleCreate(lbl_1_bss_B20[2], 1000);
    Hu3DModelPosSet(lbl_1_bss_AC2,
                    0.0f,
                    275.0f,
                    0.0f);
    Hu3DModelScaleSet(lbl_1_bss_AC2,
                      1.0f,
                      1.0f,
                      1.0f);
    Hu3DModelLayerSet(lbl_1_bss_AC2, 7);
    Hu3DModelAttrSet(lbl_1_bss_AC2, 1U);
    Hu3DParticleScaleSet(lbl_1_bss_AC2, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_AC2, fn_1_37EC0);
}

/* Particle-effect cleanup removes the rising particle model. */
void fn_1_3874C(void)
{
    Hu3DModelKill(lbl_1_bss_AC2);
}

/* Effect callers start the burst model at a position for the requested duration. */
void fn_1_38778(s16 effectNo, float duration, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_ABA[effectNo]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_ABA[effectNo], position);
    data = particle->data;
    data->time = 0;
    data->parManId = duration;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

/* The four burst models call this hook to expand, scatter and fade their particles. */
void fn_1_38844(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *first;
    HU3D_PARTICLE_DATA *data;
    HuVecF randomVector;
    HuVecF direction;
    float speed;
    s16 i;
    s16 color;

    if (particle->dataCnt == 1) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            randomVector.x = frandmod(100) - 50;
            randomVector.y = frandmod(100) - 50;
            randomVector.z = frandmod(100) - 50;
            PSVECNormalize(&randomVector, &direction);
            speed = 200.0f;
            data->vel.x = direction.x * speed;
            data->vel.y = direction.y * speed;
            data->vel.z = direction.z * speed;
            color = rand8() + 128;
            color &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.r = color;
            color = rand8() + 128;
            color &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.g = color;
            color = rand8() % 128;
            color &= MDPARTY_COLOR_COMPONENT_MASK;
            data->color.b = color;
            data->color.a = 204;
            data->pos.x = data->accel.x = frandmod(4) - 2;
            data->pos.y = data->accel.y = frandmod(4) - 2;
            data->pos.z = data->accel.z = frandmod(4) - 2;
            data->scale = 0.0f;
        }
        particle->dataCnt = 2;
    } else if (particle->dataCnt == 2) {
        first = particle->data;
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->scale = fn_1_35A10(
                0.0f, 125.0f, first->time, 5.0f);
        }
        if ((first->time += 1.0f) > 15) {
            first->time = 0;
            particle->dataCnt = 3;
        }
    } else if (particle->dataCnt == 3) {
        first = particle->data;
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->pos.x = fn_1_35A10(
                data->accel.x, data->vel.x, first->time,
                first->parManId);
            data->pos.y = fn_1_35A10(
                data->accel.y, data->vel.y, first->time,
                first->parManId);
            data->pos.z = fn_1_35A10(
                data->accel.z, data->vel.z, first->time,
                first->parManId);
            data->scale = fn_1_35AF8(
                50.0f, 0.0f, first->time, first->parManId);
        }
        if ((first->time += 1.0f) > first->parManId) {
            particle->dataCnt = 0;
            model->attr |= HU3D_ATTR_DISPOFF;
        }
    }
}

/* Particle-effect setup creates four burst models and installs their particle callback. */
void fn_1_3912C(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 4) {
        lbl_1_bss_ABA[effectNo] = Hu3DParticleCreate(lbl_1_bss_B20[2], 64);
        Hu3DModelPosSet(lbl_1_bss_ABA[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_ABA[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_ABA[effectNo], 7);
        Hu3DModelAttrSet(lbl_1_bss_ABA[effectNo], 1U);
        Hu3DParticleScaleSet(lbl_1_bss_ABA[effectNo], 1.0f);
        Hu3DParticleHookSet(lbl_1_bss_ABA[effectNo], fn_1_38844);
        Hu3DParticleBlendModeSet(lbl_1_bss_ABA[effectNo], 1U);
        effectNo += 1;
    }
}

/* Particle-effect cleanup destroys those four burst models. */
void fn_1_392C0(void)
{
    s16 i;

    i = 0;
    while (i < 4) {
        Hu3DModelKill(lbl_1_bss_ABA[i]);
        i += 1;
    }
}

/* Effect callers show or hide one of the two color-transition particle models. */
void fn_1_39318(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_AB6[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_AB6[index], 1U);
    }
}

/* Effect callers activate every particle in one color-transition model. */
void fn_1_39394(s16 effectNo, HuVecF *position, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_AB6[effectNo]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_AB6[effectNo], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_AB6[effectNo], HU3D_ATTR_DISPOFF);
}

/* Effect callers move one color-transition model's particles into their fade state. */
void fn_1_39498(s16 effectNo)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_AB6[effectNo]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 2;
    }
}

/* The color-transition models call this hook to blend particle alpha and scale. */
void fn_1_39520(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    MDMIC_PARTICLE_TARGET particleTarget[
        MDMIC_PARTICLE_TARGET_COUNT] = {
        { 16, 255 },
        { 16, 255 },
        { 48, 128 },
        { 128, 64 },
        { 255, 32 },
    };
    HU3D_PARTICLE_DATA *data;
    float velX1;
    float velY1;
    float velX2;
    float velY2;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x =
                particleTarget[i % MDMIC_PARTICLE_TARGET_COUNT].alpha;
            data->vel.y =
                particleTarget[i % MDMIC_PARTICLE_TARGET_COUNT].scale;
            velX1 = data->vel.x;
            data->color.a =
                fn_1_35B3C(data->color.a, velX1, 10.0f);
            velY1 = data->vel.y;
            data->scale = fn_1_35B3C(data->scale, velY1, 10.0f);
        } else if (data->time == 2) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            velX2 = data->vel.x;
            data->color.a = fn_1_35B3C(data->color.a, velX2, 10.0f);
            velY2 = data->vel.y;
            data->scale = fn_1_35B3C(data->scale, velY2, 10.0f);
        } else {
            data->color.a = 0;
            data->scale = 0.0f;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Particle-effect setup creates two color-transition models and installs their draw callback. */
void fn_1_39998(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        lbl_1_bss_AB6[effectNo] = Hu3DParticleCreate(*lbl_1_bss_B20, 10);
        Hu3DModelPosSet(lbl_1_bss_AB6[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_AB6[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_AB6[effectNo], 1U);
        Hu3DModelLayerSet(lbl_1_bss_AB6[effectNo], 2);
        Hu3DParticleHookSet(lbl_1_bss_AB6[effectNo], fn_1_39520);
        Hu3DParticleBlendModeSet(lbl_1_bss_AB6[effectNo], 1U);
        effectNo += 1;
    }
}

/* Particle-effect cleanup removes the two color-transition models. */
void fn_1_39B04(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_AB6[i]);
        i += 1;
    }
}

/* Effect callers show or hide one of the two color-spark models. */
void fn_1_39B5C(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_AB2[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_AB2[index], 1U);
    }
}

/* Effect callers set a color-spark model's tint and spawn center, then enable it. */
void fn_1_39BD8(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_AB2[effectNo]];
    particle = model->hookData;
    particle->dataCnt = 1;
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
    Hu3DModelAttrReset(lbl_1_bss_AB2[effectNo], HU3D_ATTR_DISPOFF);
}

/* Effect callers stop spawning particles from the selected color-spark model. */
void fn_1_39D10(s16 effectNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_AB2[effectNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* The color-spark models call this hook to spawn, move and fade their particles. */
void fn_1_39D64(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;
    float color;
    float random;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
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
            random = frandmod(128);
            color = particle->pos.x + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.r = color;
            color = particle->pos.y + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.g = color;
            color = particle->pos.z + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.b = color;
            data->color.a = 0;
        } else if (data->time == 1) {
            data->pos.x += data->accel.x;
            data->pos.y += data->accel.y;
            data->pos.z += data->accel.z;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(127) + 128;
            }
            random = fn_1_35AF8(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = frandmod(90) * random;
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

/* Particle-effect setup creates two color-spark models and installs their draw callback. */
void fn_1_3A364(void)
{
    s16 effectNo;

    effectNo = 0;
    while (effectNo < 2) {
        lbl_1_bss_AB2[effectNo] = Hu3DParticleCreate(lbl_1_bss_B20[3], 256);
        Hu3DModelPosSet(lbl_1_bss_AB2[effectNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_AB2[effectNo], 1.0f, 1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_AB2[effectNo], 1U);
        Hu3DModelLayerSet(lbl_1_bss_AB2[effectNo], 2);
        Hu3DParticleHookSet(lbl_1_bss_AB2[effectNo], fn_1_39D64);
        Hu3DParticleBlendModeSet(lbl_1_bss_AB2[effectNo], 1U);
        effectNo += 1;
    }
}

/* Particle-effect cleanup removes the two color-spark models. */
void fn_1_3A4D0(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_AB2[i]);
        i += 1;
    }
}

/* Effect callers select a color-transition, sparkle or hidden state for one player. */
void fn_1_3A528(s16 modelNo, HuVecF *pos, s16 display)
{
    GXColor colors[2] = {
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };
    if (display == 1) {
        fn_1_39394(modelNo, pos, &colors[modelNo]);

        fn_1_39BD8(modelNo, pos, &colors[modelNo]);
    } else if (display == 0) {
        fn_1_39498(modelNo);

        fn_1_39D10(modelNo);
    } else if (display == 2) {
        Hu3DModelAttrSet(lbl_1_bss_AB6[modelNo], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_AB2[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Effect callers show or hide all five particle models in one group. */
void fn_1_3A8EC(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_A76[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_A76[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Effect callers set a group's particle tint and spawn center, then enable its emitters. */
void fn_1_3A9A0(s16 groupNo, HuVecF *spawnPosition, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *emitter;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_A76[groupNo][i]];
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
        Hu3DModelAttrReset(lbl_1_bss_A76[groupNo][i], HU3D_ATTR_DISPOFF);
    }
}

/* Effect callers stop new emission from all five group models; existing particles keep updating. */
void fn_1_3AB08(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_A76[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}

/* The group emitters call this hook to spawn, move and fade their particles. */
void fn_1_3AB88(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleData;
    s16 i;
    s16 spawnCount = 0;
    float colorComponent;
    float randomValue;

    if (emitter->count == 0) {
        for (i = 0, particleData = emitter->data; i < emitter->maxCnt; i++, particleData++) {
            particleData->time = 0;
        }
        emitter->dataCnt = 1;
        emitter->pos.x = 255.0f;
        emitter->pos.y = 255.0f;
        emitter->pos.z = 255.0f;
    }

    for (i = 0, particleData = emitter->data; i < emitter->maxCnt; i++, particleData++) {
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
            /* New particles use the spawn center with x and z offsets from -50 through 49 model
             * units. */
            particleData->pos.x = (emitter->spawnCenter.x + (float)frandmod(100)) - 50.0f;
            particleData->pos.y = emitter->spawnCenter.y;
            particleData->pos.z = (emitter->spawnCenter.z + (float)frandmod(100)) - 50.0f;
            randomValue = frandmod(32);
            colorComponent = emitter->pos.x + randomValue;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.r = colorComponent;
            colorComponent = emitter->pos.y + randomValue;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.g = colorComponent;
            colorComponent = emitter->pos.z + randomValue;
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
            randomValue = fn_1_35AF8(1.0f, 0.0f, particleData->vel.x, particleData->vel.y);
            particleData->scale = 40.0f * randomValue;
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

/* Particle-effect setup creates thirty group emitters and installs their draw callbacks. */
void fn_1_3B134(void)
{
    MDMIC_PARTICLE_COUNTS particleCounts = {{10, 10, 10, 10, 256}};
    s16 emitterNo;
    s16 groupNo;

    for (groupNo = 0; groupNo < 6; groupNo += 1) {
        for (emitterNo = 0; emitterNo < 5; emitterNo += 1) {
            lbl_1_bss_A76[groupNo][emitterNo] = Hu3DParticleCreate(
                lbl_1_bss_B20[emitterNo + 4],
                particleCounts.values[emitterNo]);
            Hu3DModelPosSet(lbl_1_bss_A76[groupNo][emitterNo],
                0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_A76[groupNo][emitterNo],
                1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(lbl_1_bss_A76[groupNo][emitterNo], 1U);
            Hu3DModelLayerSet(lbl_1_bss_A76[groupNo][emitterNo], 2);
            Hu3DParticleHookSet(lbl_1_bss_A76[groupNo][emitterNo], fn_1_3AB88);
            Hu3DParticleBlendModeSet(lbl_1_bss_A76[groupNo][emitterNo], 1U);
        }
    }
}

/* Particle-effect cleanup destroys all thirty group emitters. */
void fn_1_3B360(void)
{
    s16 row;
    s16 column;

    for (row = 0; row < 6; row++) {
        column = 0;
        while (column < 5) {
            Hu3DModelKill(lbl_1_bss_A76[row][column]);
            column += 1;
        }
    }
}

/* Mic Mode startup loads the particle archives and creates every effect system. */
void fn_1_3B3E4(HUPROCESS *process)
{
    s16 animationNo;

    lbl_1_bss_A60 = process;
    fn_1_364E0();
    animationNo = 0;
    while (animationNo < 9) {
        lbl_1_bss_B20[animationNo] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_FA0[animationNo], HU_MEMNUM_OVL, HEAP_MODEL));
        animationNo += 1;
    }
    fn_1_37498();
    fn_1_37C08();
    fn_1_3864C();
    fn_1_3912C();
    fn_1_39998();
    fn_1_3A364();
    fn_1_3B134();
}

/* Mic Mode shutdown calls this to remove each particle and overlay effect system. */
void fn_1_3BE34(void)
{
    fn_1_375AC();
    fn_1_37DC8();
    fn_1_3874C();
    fn_1_392C0();
    fn_1_39B04();
    fn_1_3A4D0();
    fn_1_3B360();
    fn_1_370B4();
    fn_1_36530();
}

/* Effect callers trigger a timed sparkle and show the screen overlay at this position. */
void fn_1_3C060(HuVecF *position)
{
    static const GXColor lbl_1_rodata_452 = {128, 128, 255, 128};
    GXColor savedColor;
    GXColor sourceColor;
    GXColor *colorBuffer;
    HU3D_PARTICLE_DATA *particleData;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;

    fn_1_3660C(position);
    sourceColor = lbl_1_rodata_452;
    colorBuffer = &sourceColor;
    savedColor = *colorBuffer;
    model = &Hu3DData[lbl_1_bss_ACC];
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
    model->attr &= MDMIC_MODEL_VISIBLE_MASK;
}

/* Effect callers launch a timed particle and a radial burst using the selected palette color. */
void fn_1_3C258(s16 particleNo, Point3d *position, s16 colorNo)
{
    GXColor palette[5] = {
        {254, 77, 75, 255},
        {50, 127, 200, 255},
        {199, 175, 0, 255},
        {52, 192, 63, 255},
        {159, 93, 200, 255},
    };
    Point3d positionOffset;

    positionOffset.x = position->x;
    positionOffset.y = position->y;
    positionOffset.z = 10.0f + position->z;
    fn_1_370D8(particleNo, 10.0f, 500.0f, &positionOffset, palette[colorNo]);
    fn_1_375D8(particleNo, 10.0f, &positionOffset, palette[colorNo]);
}

/* Effect callers launch a timed particle and a mode-selected burst with separate lifetimes. */
void fn_1_3C554(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode)
{
    GXColor palette[5] = {
        {254, 77, 75, 255},
        {50, 127, 200, 255},
        {199, 175, 0, 255},
        {52, 192, 63, 255},
        {159, 93, 200, 255},
    };
    HuVecF particlePosition;

    particlePosition.x = position->x;
    particlePosition.y = position->y;
    particlePosition.z = position->z;
    Hu3DZClearLayerSet(7);
    if (mode == 0) {
        fn_1_370D8(particleNo, 20.0f, 500.0f, &particlePosition, palette[colorNo]);
        fn_1_38778(particleNo, 30.0f, &particlePosition);
        return;
    }
    fn_1_370D8(particleNo, 60.0f, 500.0f, &particlePosition, palette[colorNo]);
    fn_1_375D8(particleNo, 30.0f, &particlePosition, palette[colorNo]);
}

/* Effect reset installs the screen-trail hook on render layer 14, clears particle slots and
* reveals their model. */
void fn_1_3C9F8(void)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 particleNo;
    HU3D_MODEL *model;

    Hu3DLayerHookSet(14, fn_1_366CC);
    model = &Hu3DData[lbl_1_bss_AC2];
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
    model->attr &= MDMIC_MODEL_VISIBLE_MASK;
}

/* Effect callers start, stop or hide a selected color group at the supplied position. */
void fn_1_3CAB8(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo)
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
        fn_1_3A9A0(groupNo, pos, &colors[colorNo]);
    } else if (mode == 0) {
        fn_1_3AB08(groupNo);
    } else if (mode == 2) {
        fn_1_3A8EC(groupNo, FALSE);
    }
}

ANIMDATA *lbl_1_bss_B20[9];
MDMIC_TEXTURE_WORK lbl_1_bss_AD0[2];
HU3D_MODELID lbl_1_bss_ACC;
HU3D_MODELID lbl_1_bss_AC4[4];
HU3D_MODELID lbl_1_bss_AC2;
HU3D_MODELID lbl_1_bss_ABA[4];
HU3D_MODELID lbl_1_bss_AB6[2];
HU3D_MODELID lbl_1_bss_AB2[2];
HU3D_MODELID lbl_1_bss_A76[6][5];
s16 lbl_1_bss_A74;
void *lbl_1_bss_A70;
ANIMDATA *lbl_1_bss_A6C;
ANIMDATA *lbl_1_bss_A68;
OMOBJ *lbl_1_bss_A64;
HUPROCESS *lbl_1_bss_A60;

s32 lbl_1_data_FA0[9] = {
    DATANUM(DATA_mdmic, 105), DATANUM(DATA_mdmic, 106), DATANUM(DATA_mdmic, 104),
    DATANUM(DATA_mdmic, 107), DATANUM(DATA_mdmic, 112), DATANUM(DATA_mdmic, 112),
    DATANUM(DATA_mdmic, 112), DATANUM(DATA_mdmic, 112), DATANUM(DATA_mdmic, 112),
};
