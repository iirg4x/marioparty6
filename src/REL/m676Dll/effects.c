/* Scene particles, contact shadows and animated indirect texture effects. */
#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "REL/m676Dll/effect_types.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/frand.h"
#include <string.h>
#include "REL/m676Dll/scene_types.h"
#include "REL/m676Dll/particle_types.h"

OMOBJ **lbl_1_data_7A0[4] = {&lbl_1_bss_E98,&lbl_1_bss_E94,&lbl_1_bss_E98,&lbl_1_bss_E98};
HUPROCESS *lbl_1_bss_10AC;
OMOBJ *lbl_1_bss_10A8;
s32 fn_1_1480(s32 index);

void fn_1_10288(s32 type, s32 index, Vec *position, s32 requestValue);
void fn_1_1033C(HUPROCESS *process);
void fn_1_103F4(void);
void fn_1_103F8(OMOBJ *obj);
void fn_1_10408(OMOBJ *node);
void fn_1_10430(OMOBJ *object);
void fn_1_10550(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_10764(OMOBJ *owner);
void fn_1_108A0(HU3D_MODEL *model, Mtx *view);
void fn_1_10B7C(OMOBJ *object);
void fn_1_10C08(OMOBJ *object);
void fn_1_10C5C(HU3D_MODEL *model, Mtx *matrix);
void fn_1_10CB4(HU3D_MODEL *model, Mtx *matrix);
void fn_1_10D0C(OMOBJ *object, M676TextureWork *work);
void fn_1_10E54(Vec *position, f32 *farPlane);
void fn_1_10EC4(M676TextureWork *work);
void fn_1_11124(M676TextureWork *work);
void fn_1_11184(GXTexObj *texture, GXTexMapID map, u32 matrixId, Mtx inputMatrix);
void fn_1_11238(M676TextureWork *work, s32 textureMap, u32 textureMatrix);
void fn_1_11294(M676TextureWork *work, s32 textureMap, u32 textureMatrix);
void fn_1_112F0(s32 unusedValue, GXIndTexStageID stage, GXTexMapID map, GXTexCoordID coord);
void fn_1_11340(M676TextureWork *work, Mtx *matrix);
void fn_1_113A0(M676TextureWork *texture, Mtx *matrix);
void fn_1_11928(M676TextureWork *work);
void fn_1_119B0(Vec *position, Vec *velocity);
void fn_1_11A74(Vec *position, f32 radius, s32 count);
void fn_1_11EA4(M676ShadowRecord *effect);

/* Queues a scene effect request for the manager to consume during its update. */
void fn_1_10288(s32 type, s32 index, Vec *position, s32 requestValue)
{
    OMOBJ *manager;
    M676SceneEffectWork *storage;
    M676ParticleRecord *request;
    s32 requestLimit;
    s32 i;

    if (type >= 0 && (u32)type < 4U) {
        manager = *lbl_1_data_7A0[type];
        requestLimit = manager->work[0];
        storage = manager->data;
        request = storage->records;
        for (i = 0; i < requestLimit; i++, request++) {
            if (request->state == 0) {
                request->state = 1;
                request->type = type;
                request->playerIndex = index;
                request->position = *position;
                request->areaIndex = requestValue;
                return;
            }
        }
    }
}

/* Creates the scene-effect object and its hooks when fn_1_A0 initializes the board. */
void fn_1_1033C(HUPROCESS *process) {
    HUPROCESS *unusedProcess;
    OMOBJ *effectObject;
    void *effectWork;
    lbl_1_bss_10AC = process;
    effectObject = omAddObjEx(lbl_1_bss_10AC, 16, 4U, 0U, - 1, fn_1_103F8);
    lbl_1_bss_10A8 = effectObject;
    effectWork = HuMemDirectMallocNum(HEAP_HEAP, 248, HU_MEMNUM_OVL);
    effectObject->data = effectWork;
    memset(effectWork, 0, 248);
    fn_1_10430(effectObject);
    fn_1_10764(effectObject);
    fn_1_10B7C(effectObject);
}

void fn_1_103F4(void)
{
}

void fn_1_103F8(OMOBJ *obj)
{
    obj->objFunc = fn_1_10408;
}

void fn_1_10408(OMOBJ *node)
{
    fn_1_10C08(node);
}

/* Called by scene setup to create and initialize the particle model and its slots. */
void fn_1_10430(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum((s32)fn_1_1480(13), HU_MEMNUM_OVL,
                                                    HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 128);
    object->mdlId[0] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleColSet(modelId, 255, 255, 255);
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_10550);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* Particle-model hook: advance live particles and flush their records before GX draws them. */
void fn_1_10550(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry = particle->data;
    s32 index;

    for (index = 0; index < particle->maxCnt; index++, entry++) {
        switch (entry->time) {
        case 0:
            entry->time++;
            entry->scale = (f32)(40.0 + 20.0 *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));
            entry->zRot = 0.0f;
            entry->color.a = 240;
            /* fall through */
        case 1:
            entry->scale += 4.0;
            entry->zRot += 0.008726646192371845;
            entry->color.a -= 4;
            if (entry->pos.y < -100.0 ||
                entry->scale <= 0.0f || entry->color.a > 240) {
                entry->time = -1;
                entry->scale = 0.0f;
            } else {
                PSVECAdd((Vec *)&entry->pos, (Vec *)&entry->accel,
                    (Vec *)&entry->pos);
                entry->accel.x *= 0.98;
                entry->accel.y -= 0.05;
                entry->accel.z *= 0.98;
            }
            break;
        }
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
    PPCSync();
}

/* Called during scene-effect setup to install the contact-shadow hook and its ring vertices. */
void fn_1_10764(OMOBJ *owner)
{
    M676ShadowWork *hookData;
    HU3D_MODEL *modelData;
    s16 modelId;
    f32 angle;
    s32 i;

    modelId = Hu3DHookFuncCreate(fn_1_108A0);
    owner->mdlId[1] = modelId;
    Hu3DModelLayerSet(modelId, 4);
    modelData = &Hu3DData[modelId];
    hookData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*hookData), HU_MEMNUM_OVL);
    modelData->hookData = hookData;
    memset(hookData, 0, sizeof(*hookData));
    i = 0;
    angle = 0.0f;
    while (i < 10) {
        hookData->xOffsets[i] = (f32)sin((3.141592653589793 * (f64)angle) / 180.0);
        hookData->zOffsets[i] = (f32)cos((3.141592653589793 * (f64)angle) / 180.0);
        i++;
        angle += 36.0f;
    }
}

/* Custom model hook: draw a translucent ground disc for each registered active shadow. */
void fn_1_108A0(HU3D_MODEL *model, Mtx *view)
{
    M676ShadowWork *effects = model->hookData;
    Mtx matrix;
    M676ShadowRecord *record;
    f32 *xOffsets;
    f32 *zOffsets;
    s32 i;
    s32 vertex;
    f32 radius;

    xOffsets = effects->xOffsets;
    zOffsets = effects->zOffsets;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
        GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    for (i = 0; i < 10; i++) {
        record = effects->records[i];
        if (record != NULL && record->active != 0) {
            radius = record->radius;
            PSMTXTrans(matrix, record->position->x,
                3.0 + record->position->y,
                record->position->z);
            PSMTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 12);
            GXPosition3f32(0.0f, 0.0f, 0.0f);
            GXColor1u32(192);
            for (vertex = 0; vertex < 10; vertex++) {
                GXPosition3f32(xOffsets[vertex] * radius,
                    0.0f, zOffsets[vertex] * radius);
                GXColor1u32(40);
            }
            GXPosition3f32(xOffsets[0] * radius,
                0.0f, zOffsets[0] * radius);
            GXColor1u32(40);
        }
    }
}

/* Called by scene setup to create the two texture-effect hooks and load their source images. */
void fn_1_10B7C(OMOBJ *object)
{
    M676EffectWork *work;
    M676TextureWork *childWork;
    HU3D_MODELID hook;

    work = (M676EffectWork *)object->data;
    childWork = &work->texture;
    hook = Hu3DHookFuncCreate((HU3D_MODEL_HOOK)fn_1_10C5C);
    object->mdlId[2] = hook;
    hook = Hu3DHookFuncCreate((HU3D_MODEL_HOOK)fn_1_10CB4);
    object->mdlId[3] = hook;
    fn_1_10D0C(object, childWork);
    work->enabled = 1;
}

/* Per-frame object callback: update camera-relative texture matrices while the effect is
 * enabled. */
void fn_1_10C08(OMOBJ *object)
{
    M676EffectWork *work;
    M676TextureWork *childWork;

    work = object->data;
    childWork = &work->texture;
    if (work->enabled != 0) {
        fn_1_10EC4(childWork);
    }
}

/* Layer-2 model hook: copy the current framebuffer into the effect texture. */
void fn_1_10C5C(HU3D_MODEL *model, Mtx *matrix) {
    OMOBJ *base = lbl_1_bss_10A8;
    M676EffectWork *work = base->data;
    M676TextureWork *callTarget = &work->texture;
    fn_1_11340(callTarget, matrix);
}

/* Layer-5 model hook: render the animated indirect-texture surface with the supplied matrix. */
void fn_1_10CB4(HU3D_MODEL *model, Mtx *matrix)
{
    OMOBJ *base = lbl_1_bss_10A8;
    M676EffectWork *work = base->data;
    M676TextureWork *callTarget = &work->texture;

    fn_1_113A0(callTarget, matrix);
}

const u8 lbl_1_rodata_6B0[48] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 64, 0, 240,
    68, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const u16 lbl_1_rodata_6E0[2] = {1497,597};
const GXColor lbl_1_rodata_6E4 = {6,6,6,18};
const u8 lbl_1_rodata_6E8[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 3, 97, 16, 16, 16, 9
};
/* Called once from scene setup to assign render layers and allocate the effect's texture assets. */
void fn_1_10D0C(OMOBJ *object, M676TextureWork *work)
{
    s16 modelId;
    u32 textureSize;

    modelId = object->mdlId[2];
    Hu3DModelLayerSet(modelId, 2);
    modelId = object->mdlId[3];
    Hu3DModelLayerSet(modelId, 5);
    if (work->textureBuffer != NULL) {
        return;
    }
    textureSize = GXGetTexBufferSize(320, 240, 4, 0, 0);
    work->textureBuffer = HuMemDirectMallocNum(HEAP_HEAP, textureSize, HU_MEMNUM_OVL);
    work->textureData = HuSprAnimRead(
        HuDataSelHeapReadNum(fn_1_1480(14), HU_MEMNUM_OVL, HEAP_MODEL));
    work->animationData = HuSprAnimRead(
        HuDataSelHeapReadNum(fn_1_1480(15), HU_MEMNUM_OVL, HEAP_MODEL));
    work->scrollOffsetX = 0.0f;
    work->scrollOffsetZ = 0.0f;
    PSMTXIdentity(work->textureMatrices[0]);
    PSMTXIdentity(work->textureMatrices[1]);
    PSMTXIdentity(work->textureMatrices[2]);
    PSMTXIdentity(work->textureMatrices[3]);
    GXInitTexObj(&work->outputTexture, work->textureBuffer, 320, 240,
        GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0);
}

/* Read the active camera position and extend its far plane for the texture projection. */
void fn_1_10E54(Vec *position, f32 *farPlane)
{
    Vec scratchPosition;
    f32 scratchPerspective;

    Hu3DCameraPosGet(1, position, &scratchPosition, &scratchPosition);
    Hu3DCameraPerspectiveGet(1, &scratchPerspective, &scratchPerspective, farPlane);
    *farPlane *= 1.8;
}

/* Updates the camera-relative texture matrices each frame while the effect object is active. */
void fn_1_10EC4(M676TextureWork *work) {
    Point3d cameraPosition;
    Point3d ignoredCameraVector;
    f32 farPlane;
    f32 ignoredPerspective;
    f32 textureScale;

    /* Up and target share scratch storage because neither direction is used here. */
    Hu3DCameraPosGet(1, &cameraPosition, &ignoredCameraVector, &ignoredCameraVector);
    Hu3DCameraPerspectiveGet(1, &ignoredPerspective, &ignoredPerspective, &farPlane);
    farPlane *= 1.8;
    PSMTXScale(work->textureMatrices[0], farPlane, 1.0f, farPlane);
    PSMTXTransApply(work->textureMatrices[0], work->textureMatrices[0], cameraPosition.x, 0.0f,
                    cameraPosition.z);
    work->scrollOffsetX = (f32) ((f64) work->scrollOffsetX - 2.857142857142857);
    work->scrollOffsetZ = (f32) ((f64) work->scrollOffsetZ - 1.1764705882352942);
    textureScale = farPlane / 1200.0f;
    PSMTXScale(work->textureMatrices[1], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[1], work->textureMatrices[1],
                    (f32) (((f64) cameraPosition.x - (0.5 * (f64) work->scrollOffsetX)) / 1200.0),
                    (f32) (((f64) cameraPosition.z + (0.8 * (f64) work->scrollOffsetZ)) / 1200.0),
                    0.0f);
    PSMTXScale(work->textureMatrices[2], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[2], work->textureMatrices[2],
                    (cameraPosition.x + work->scrollOffsetX) / 1200.0f,
                    (cameraPosition.z + work->scrollOffsetZ) / 1200.0f, 0.0f);
    PSMTXScale(work->textureMatrices[3], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[3], work->textureMatrices[3], cameraPosition.x / 1200.0f,
                    cameraPosition.z / 1200.0f, 0.0f);
}

/* Copy the rendered 640-by-480 framebuffer into the effect's half-size RGB565 texture. */
void fn_1_11124(M676TextureWork *work)
{
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, 1);
    GXCopyTex(work->textureBuffer, 0);
    GXPixModeSync();
}

/* Bind a texture and compose its projection with the supplied camera-space matrix. */
void fn_1_11184(GXTexObj *texture, GXTexMapID map, u32 matrixId, Mtx inputMatrix)
{
    Mtx perspectiveMatrix;
    f32 fov;
    f32 nearFar;

    GXLoadTexObj(texture, map);
    Hu3DCameraPerspectiveGet(1, &fov, &nearFar, &nearFar);
    C_MTXLightPerspective(perspectiveMatrix, fov, 1.2f,
        0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(perspectiveMatrix, inputMatrix, perspectiveMatrix);
    GXLoadTexMtxImm(perspectiveMatrix, matrixId, GX_MTX3x4);
}

/* Binds the first source image with its scrolling texture transform. */
void fn_1_11238(M676TextureWork *work, s32 textureMap, u32 textureMatrix)
{
    HuSprTexLoad(work->textureData, 0, textureMap,
        GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXLoadTexMtxImm(work->textureMatrices[2], textureMatrix, GX_MTX2x4);
}

/* Binds bitmap zero of the second source texture with its camera-relative transform. */
void fn_1_11294(M676TextureWork *work, s32 textureMap, u32 textureMatrix)
{
    HuSprTexLoad(work->animationData, 0, textureMap,
        GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXLoadTexMtxImm(work->textureMatrices[3], textureMatrix, GX_MTX2x4);
}

/* Sets an indirect-texture stage's map/coordinate order and unit scale; ignores the first
 * argument. */
void fn_1_112F0(s32 unusedValue, GXIndTexStageID stage, GXTexMapID map, GXTexCoordID coord)
{
    GXSetIndTexOrder(stage, coord, map);
    GXSetIndTexCoordScale(stage, GX_ITS_1, GX_ITS_1);
}

/* Called by the layer-2 model hook to refresh the effect texture from the framebuffer. */
void fn_1_11340(M676TextureWork *work, Mtx *matrix)
{
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, 1);
    GXCopyTex(work->textureBuffer, 0);
    GXPixModeSync();
}

const f32 lbl_1_rodata_754[3][2][3] = { { { 0.01f, 0, 0 }, { 0, 0.01f, 0 } },
                                        { { 0.02f, 0, 0 }, { 0, 0.02f, 0 } },
                                        { { 0.03f, 0, 0 }, { 0, 0.03f, 0 } } };
/* Called by the layer-5 model hook to configure GX and draw the animated indirect-texture mesh. */
void fn_1_113A0(M676TextureWork *texture, Mtx *matrix)
{
    Mtx positionMatrix;

    PSMTXConcat(*matrix, texture->textureMatrices[0],
        positionMatrix);
    GXLoadPosMtxImm(positionMatrix, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_NONE, GX_AF_NONE);
    fn_1_11184(&texture->outputTexture, GX_TEXMAP0, GX_TEXMTX0, positionMatrix);
    HuSprTexLoad(texture->textureData, 0, 1, 1,
        1, 1);
    GXLoadTexMtxImm(texture->textureMatrices[2], GX_TEXMTX1,
        GX_MTX2x4);
    HuSprTexLoad(texture->animationData, 0, 2, 1,
        1, 1);
    GXLoadTexMtxImm(texture->textureMatrices[3], GX_TEXMTX2,
        GX_MTX2x4);
    GXLoadTexMtxImm(texture->textureMatrices[1], GX_TEXMTX3,
        GX_MTX2x4);
    GXSetNumTexGens(4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, 0,
        GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, 0,
        GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, 0,
        GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, 0,
        GX_PTIDENTITY);
    GXSetIndTexMtx(GX_ITM_0, (f32 (*)[3])lbl_1_rodata_754[0], 0);
    GXSetIndTexMtx(GX_ITM_1, (f32 (*)[3])lbl_1_rodata_754[1], 0);
    GXSetIndTexMtx(GX_ITM_2, (f32 (*)[3])lbl_1_rodata_754[2], 0);
    GXSetNumIndStages(2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
    GXSetTevKColor(GX_KCOLOR0, lbl_1_rodata_6E4);
    GXSetNumTevStages(4);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, 1, 0, GX_ITM_1);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA,
        GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
        GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_CPREV, GX_CC_RASC, GX_CC_RASA,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE3, GX_INDTEXSTAGE1, 1, 0, GX_ITM_2);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC,
        GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(1);
    GXSetAlphaUpdate(0);
    GXSetZCompLoc(1);
    GXSetZMode(1, GX_LESS, 1);
    GXSetAlphaCompare(GX_ALWAYS, 255, GX_AOP_OR, GX_ALWAYS, 255);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    fn_1_11928(texture);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetNumIndStages(0);
}

const GXVtxDescList lbl_1_rodata_79C[] = { { GX_VA_POS, GX_INDEX8 },
                                           { GX_VA_CLR0, GX_INDEX8 },
                                           { GX_VA_TEX0, GX_INDEX8 },
                                           { GX_VA_NULL, GX_NONE } };
const GXVtxAttrFmtList lbl_1_rodata_7BC[] = { { GX_VA_POS, GX_POS_XYZ, GX_S16, 14 },
                                              { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0 },
                                              { GX_VA_TEX0, GX_TEX_ST, GX_S16, 14 },
                                              { GX_VA_NULL, 0, 0, 0 } };
/* Disc mesh for the animated indirect texture effect. */
const M676EffectVertex lbl_1_rodata_800[32] ATTRIBUTE_ALIGN(32) = {
    { 0, 0, 0 },         { 0, 0, 8192 },      { -1761, 0, 8000 },  { -3439, 0, 7434 },
    { -4957, 0, 6521 },  { -6243, 0, 5303 },  { -7237, 0, 3837 },  { -7893, 0, 2191 },
    { -8179, 0, 443 },   { -8084, 0, -1325 }, { -7610, 0, -3032 }, { -6780, 0, -4597 },
    { -5633, 0, -5947 }, { -4223, 0, -7019 }, { -2615, 0, -7763 }, { -885, 0, -8143 },
    { 885, 0, -8143 },   { 2615, 0, -7763 },  { 4223, 0, -7019 },  { 5633, 0, -5947 },
    { 6780, 0, -4597 },  { 7610, 0, -3032 },  { 8084, 0, -1325 },  { 8179, 0, 443 },
    { 7893, 0, 2191 },   { 7237, 0, 3837 },   { 6243, 0, 5303 },   { 4957, 0, 6521 },
    { 3439, 0, 7434 },   { 1761, 0, 8000 },   { 0, 0, 0 },         { 0, 0, 0 }
};
const M676EffectUV lbl_1_rodata_8C0[32] = {
    { 8192, 8192 },   { 8192, 16384 },  { 6430, 16192 },  { 4752, 15626 },  { 3234, 14713 },
    { 1948, 13495 },  { 954, 12029 },   { 298, 10383 },   { 12, 8635 },     { 107, 6866 },
    { 581, 5159 },    { 1411, 3594 },   { 2558, 2244 },   { 3968, 1172 },   { 5576, 428 },
    { 7306, 48 },     { 9077, 48 },     { 10807, 428 },   { 12415, 1172 },  { 13825, 2244 },
    { 14972, 3594 },  { 15802, 5159 },  { 16276, 6866 },  { 16371, 8635 },  { 16085, 10383 },
    { 15429, 12029 }, { 14435, 13495 }, { 13149, 14713 }, { 11631, 15626 }, { 9953, 16192 },
    { 0, 0 },         { 0, 0 }
};
const u8 lbl_1_rodata_940[96] = {
    160, 0, 31, 0, 1, 0, 1, 0, 1, 2, 0, 2, 3, 0, 3, 4,
    0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9, 0,
    9, 10, 0, 10, 11, 0, 11, 12, 0, 12, 13, 0, 13, 14, 0, 14,
    15, 0, 15, 16, 0, 16, 17, 0, 17, 18, 0, 18, 19, 0, 19, 20,
    0, 20, 21, 0, 21, 22, 0, 22, 23, 0, 23, 24, 0, 24, 25, 0,
    25, 26, 0, 26, 27, 0, 27, 28, 0, 28, 29, 0, 29, 1, 0, 1
};
/* Submit the indexed disc mesh used by the animated indirect-texture effect. */
void fn_1_11928(M676TextureWork *work) {
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_79C);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_7BC);
    GXSetArray(GX_VA_POS, (void *)lbl_1_rodata_800, 6U);
    GXSetArray(GX_VA_CLR0, (void *)lbl_1_rodata_6E0, 2U);
    GXSetArray(GX_VA_TEX0, (void *)lbl_1_rodata_8C0, 4U);
    GXCallDisplayList((void *)lbl_1_rodata_940, 96U);
}

/* Adds a particle at position with the supplied initial velocity when a slot is free. */
void fn_1_119B0(Vec *position, Vec *velocity)
{
    OMOBJ *object;
    HU3D_MODELID modelId;
    HU3D_PARTICLE *work;
    HU3D_PARTICLE_DATA *record;
    s32 index;

    object = lbl_1_bss_10A8;
    modelId = object->mdlId[0];
    work = Hu3DData[modelId].hookData;
    record = work->data;
    for (index = 0; index < work->maxCnt; index++, record++) {
        if (record->time < 0) {
            record->time = 0;
            record->pos = *position;
            record->accel = *velocity;
            break;
        }
    }
}

/* Spawn count particles around a point, with randomized height, spread, and launch velocity. */
void fn_1_11A74(Vec *position, f32 radius, s32 count)
{
    s32 particleIndex;

    for (particleIndex = 0; particleIndex < count; particleIndex++) {
        Vec particlePosition;
        Vec particleVelocity;
        s32 slot;
        OMOBJ *manager;
        s16 modelId;
        HU3D_PARTICLE *hookWork;
        HU3D_PARTICLE_DATA *particle;
        f32 sine;
        f32 cosine;
        f32 angle;

        angle = (f32)((270.0 *
            (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF))) -
            135.0);
        sine = (f32)sin((3.141592653589793 * (f64)angle) /
            180.0);
        cosine = (f32)cos((3.141592653589793 * (f64)angle) /
            180.0);

        particlePosition.x = (f32)((f64)(position->x + (sine * radius)) +
            (15.0 * (3.0517578125e-05 *
                (f64)(f32)((s32)(frand() & 0xFFFF) - 32768))));
        particlePosition.y = (f32)(20.0 +
            ((f64)position->y + (40.0 *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)))));
        particlePosition.z = (f32)((f64)(position->z + (cosine * radius)) +
            (15.0 * (3.0517578125e-05 *
                (f64)(f32)((s32)(frand() & 0xFFFF) - 32768))));
        particleVelocity.x = (f32)((f64)sine +
            (2.0 * (3.0517578125e-05 *
                (f64)(f32)((s32)(frand() & 0xFFFF) - 32768))));
        particleVelocity.y = (f32)(4.0 +
            (2.0 * (1.52587890625e-05 *
                (f64)(f32)(s32)(frand() & 0xFFFF))));
        particleVelocity.z = (f32)((f64)cosine +
            (2.0 * (3.0517578125e-05 *
                (f64)(f32)((s32)(frand() & 0xFFFF) - 32768))));

        manager = lbl_1_bss_10A8;
        modelId = manager->mdlId[0];
        hookWork = Hu3DData[modelId].hookData;
        particle = hookWork->data;
        for (slot = 0; slot < hookWork->maxCnt; slot++, particle++) {
            if (particle->time < 0) {
                particle->time = 0;
                particle->pos = particlePosition;
                particle->accel = particleVelocity;
                break;
            }
        }
    }
}

/* Register a shadow record in the first free slot used by the custom shadow draw hook. */
void fn_1_11EA4(M676ShadowRecord *effect)
{
    M676ShadowRecord **entry;
    s32 i;
    OMOBJ *object;
    HU3D_MODEL *model;
    s16 modelId;
    M676ShadowWork *work;

    object = lbl_1_bss_10A8;
    modelId = object->mdlId[1];
    model = &Hu3DData[modelId];
    work = model->hookData;
    entry = work->records;
    for (i = 0; i < 10; i++, entry++) {
        if (*entry == NULL) {
            *entry = effect;
            break;
        }
    }
}
