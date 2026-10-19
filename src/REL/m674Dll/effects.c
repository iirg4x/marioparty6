/* Particle emissions, ground quads, and projected water textures. */
#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/frand.h"
#include "REL/m674Dll/effects.h"
#include "string.h"
#define M674_WATER_TEXTURE_HEADER_BIT_PATTERN 20971760
#define M674_WATER_TEXTURE_SCALE_BIT_PATTERN 1142292480
#define M674_WATER_TEXTURE_FORMAT_BIT_PATTERN 721761
#define M674_WATER_TEXTURE_WRAP_BIT_PATTERN 269488137
HUPROCESS *lbl_1_bss_6C4;
OMOBJ *lbl_1_bss_6C0;
GXColor lbl_1_data_678[2] = {{192, 192, 192, 96}, {144, 144, 208, 96}};
extern s32 fn_1_E18(s32 index);
void fn_1_E180(OMOBJ *object);
void fn_1_E190(OMOBJ *object);
void fn_1_E1C8(OMOBJ *object);
void fn_1_E300(HU3D_MODEL *model, HU3D_PARTICLE *effect, Mtx matrix);
void fn_1_E460(OMOBJ *object);
void fn_1_E4DC(OMOBJ *object);
void fn_1_E56C(HU3D_MODEL *model, Mtx *matrix);
void fn_1_E8B8(OMOBJ *object);
void fn_1_E92C(OMOBJ *object);
void fn_1_E980(HU3D_MODEL *model, Mtx *matrix);
void fn_1_E9D8(OMOBJ *object, M674TextureWork *work);
void fn_1_EBDC(M674TextureWork *work);
void fn_1_F028(M674TextureWork *work, Mtx matrix);
void fn_1_F5D8(M674TextureWork *work);

/* Minigame setup calls this to create the shared effect object and install its effect work. */
void fn_1_E0C4(HUPROCESS *process) {
        OMOBJ * object;
    void * work;
    lbl_1_bss_6C4 = process;
    object = omAddObjEx(lbl_1_bss_6C4, 16, 3U, 0U, - 1, fn_1_E180);
    lbl_1_bss_6C0 = object;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674EffectWork), HU_MEMNUM_OVL);
    object->data = work;
    memset(work, 0, sizeof(M674EffectWork));
    fn_1_E1C8(object);
    fn_1_E460(object);
    fn_1_E8B8(object);
}

void fn_1_E17C(void)
{
}

/* Switch the newly created effect object to its regular update callback. */
void fn_1_E180(OMOBJ *obj)
{
    obj->objFunc = fn_1_E190;
}

/* Update fading ground quads and the projected water texture each object pass. */
void fn_1_E190(OMOBJ *object) {
    fn_1_E4DC(object);
    fn_1_E92C(object);
}

/* The effect initializer calls this to create the emitter and mark every particle slot hidden. */
void fn_1_E1C8(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;
    GXColor *configuration;

    configuration = (GXColor *)object->data;
    *configuration = lbl_1_data_678[0];
    animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_E18(15), HU_MEMNUM_OVL, HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 128);
    object->mdlId[0] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_E300);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* The particle draw hook calls this once per frame to move active particles and fade them out. */
void fn_1_E300(HU3D_MODEL *model, HU3D_PARTICLE *effect, Mtx matrix)
{
    extern OMOBJ *lbl_1_bss_6C0;
    OMOBJ *globalManager = lbl_1_bss_6C0;
    HU3D_PARTICLE_DATA *record;
    s32 index;
    void *managerData;

    managerData = globalManager->data;
    record = effect->data;
    index = 0;
    while (index < effect->maxCnt) {
        if (record->time == 0) {
            record->scale += 4.0f;
            record->color.a -= 2;
            if (record->color.a < 5) {
                record->time = -1;
                record->scale = 0.0f;
            } else {
                record->pos.x += record->accel.x;
                record->pos.y += record->accel.y;
                record->pos.z += record->accel.z;
                record->accel.x *= 0.9;
                record->accel.y = (f32)((f64)record->accel.y - 0.2);
                record->accel.z *= 0.9;
            }
        }
        index++;
        record++;
    }
    DCStoreRangeNoSync(effect->data,
                       effect->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* The effect initializer calls this to create the quad renderer and load its sprite animation. */
void fn_1_E460(OMOBJ *object)
{
    M674EffectWork *work = object->data;
    HU3D_MODELID hook = Hu3DHookFuncCreate(fn_1_E56C);

    object->mdlId[1] = hook;
    Hu3DModelLayerSet(hook, 2);
    work->animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_E18(14), HU_MEMNUM_OVL, HEAP_MODEL));
}

/* The effect object's update calls this each frame to grow and fade active ground quads. */
void fn_1_E4DC(OMOBJ *object)
{
    M674EffectQuad *slot;
    GXColor *attributes;
    s32 remaining;
    M674EffectWork *work = object->data;
    s32 index;

    for (index = 0; index < 128; index++) {
        slot = &work->quads[index];
        if (slot->enabled != 0) {
            slot->sizeScale += slot->sizeDelta;
            attributes = &slot->color.channels;
            remaining = attributes->a - 2;
            attributes->a = (u8)remaining;
            if (remaining <= 0) {
                slot->enabled = 0;
            }
        }
    }
}

/* The custom model draw hook calls this to render each active ground quad. */
void fn_1_E56C(HU3D_MODEL *unusedObject, Mtx *matrix)
{
    OMOBJ *object = lbl_1_bss_6C0;
    M674EffectWork *batch = object->data;
    s32 spriteIndex;

    GXLoadPosMtxImm(*matrix, 0U);
    HuSprTexLoad(batch->animation, 0, 0, 0, 0, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(1U, GX_ALWAYS, 0U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0U, GX_AOP_AND, GX_LEQUAL, 255U);
    GXSetNumChans(1U);
    GXSetNumTexGens(1U);
    GXSetNumTevStages(1U);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_VTX, GX_SRC_VTX, 0U, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);

    for (spriteIndex = 0; spriteIndex < 128; spriteIndex++) {
        M674EffectQuad *sprite = &batch->quads[spriteIndex];

        if (sprite->enabled != 0) {
            u32 color = sprite->color.packed;
            f32 halfWidth = sprite->size * sprite->sizeScale;
            f32 centerX = sprite->position.x;
            f32 centerY = sprite->position.y;
            f32 centerZ = sprite->position.z;

            GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
            GXWGFifo.f32 = centerX - halfWidth;
            GXWGFifo.f32 = centerY;
            GXWGFifo.f32 = centerZ - halfWidth;
            GXWGFifo.u32 = color;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = centerX + halfWidth;
            GXWGFifo.f32 = centerY;
            GXWGFifo.f32 = centerZ - halfWidth;
            GXWGFifo.u32 = color;
            GXWGFifo.f32 = 1.0f;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = centerX + halfWidth;
            GXWGFifo.f32 = centerY;
            GXWGFifo.f32 = centerZ + halfWidth;
            GXWGFifo.u32 = color;
            GXWGFifo.f32 = 1.0f;
            GXWGFifo.f32 = 1.0f;
            GXWGFifo.f32 = centerX - halfWidth;
            GXWGFifo.f32 = centerY;
            GXWGFifo.f32 = centerZ + halfWidth;
            GXWGFifo.u32 = color;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.f32 = 1.0f;
        }
    }
}

/* Static graphics data used to draw the animated water surface. */
static const u32 lbl_1_rodata_57C[9] = {
    0, 0, 0, 4, M674_WATER_TEXTURE_HEADER_BIT_PATTERN,
    M674_WATER_TEXTURE_SCALE_BIT_PATTERN, 0, 0,
    0
};
static const u16 lbl_1_rodata_5A0[2] = {761, 3360};
static const GXColor lbl_1_rodata_5A4 = {16, 16, 16, 22};
static const u32 lbl_1_rodata_5A8[8] = {
    0, 0, 0, 0, 0, 0, M674_WATER_TEXTURE_FORMAT_BIT_PATTERN,
    M674_WATER_TEXTURE_WRAP_BIT_PATTERN
};

/* The effect initializer calls this to create the projected-texture hook and water state. */
void fn_1_E8B8(OMOBJ *object)
{
    M674EffectWork *work;
    void *childWork;
    HU3D_MODELID hook;

    work = object->data;
    childWork = &work->texture;
    hook = Hu3DHookFuncCreate((HU3D_MODEL_HOOK)fn_1_E980);
    object->mdlId[2] = hook;
    fn_1_E9D8(object, childWork);
    work->enabled = 1;
}

/* The effect object's update calls this each frame while projected water is enabled. */
void fn_1_E92C(OMOBJ *object)
{
    M674EffectWork *work;
    M674TextureWork *childWork;

    work = object->data;
    childWork = &work->texture;
    if (work->enabled != 0) {
        fn_1_EBDC(childWork);
    }
}

/* Draw the projected water texture using the current model view matrix. */
void fn_1_E980(HU3D_MODEL *object, Mtx *matrix)
{
    OMOBJ *base = lbl_1_bss_6C0;
    M674EffectWork *work = base->data;
    M674TextureWork *callTarget = &work->texture;
    fn_1_F028(callTarget, *matrix);
}

/* The water-hook initializer calls this to allocate the render target and load overlay textures. */
void fn_1_E9D8(OMOBJ *object, M674TextureWork *work)
{
    u32 textureSize;
    s16 modelId = object->mdlId[2];

    Hu3DModelLayerSet(modelId, 4);
    if (work->textureBuffer != NULL) {
        return;
    }
    textureSize = GXGetTexBufferSize(320, 240, 4, 0, 0);
    work->textureBuffer = HuMemDirectMallocNum(HEAP_HEAP, textureSize, HU_MEMNUM_OVL);
    work->textureData = HuDataSelHeapReadNum(fn_1_E18(16), HU_MEMNUM_OVL, HEAP_MODEL);
    work->animationData = HuSprAnimRead(
        HuDataSelHeapReadNum(fn_1_E18(17), HU_MEMNUM_OVL, HEAP_MODEL));
    work->scrollX = 0.0f;
    work->scrollY = 0.0f;
    PSMTXIdentity(work->textureMatrices[0]);
    PSMTXIdentity(work->textureMatrices[1]);
    PSMTXIdentity(work->textureMatrices[2]);
    PSMTXIdentity(work->textureMatrices[3]);
    GXInitTexObj(&work->outputTexture, work->textureBuffer, 320, 240, 4, 0, 0, 0);
    GXInitTexObj(&work->animationTexture, work->textureData, 128, 128, 3, 1, 1, 1);
    GXInitTexObjLOD(&work->animationTexture, 5, 1, 0.0f,
                    7.0f, 0.0f, 0, 0, 0);
}

/* Read the selected camera position and scale its far clipping distance. */
void fn_1_EB6C(Vec *position, f32 *farPlane)
{
    Vec unusedCameraVector;
    f32 unusedPerspective;

    Hu3DCameraPosGet(1, position, &unusedCameraVector, &unusedCameraVector);
    Hu3DCameraPerspectiveGet(1, &unusedPerspective, &unusedPerspective, farPlane);
    *farPlane *= 1.8;
}

/* The effect update calls this while water is enabled to follow the camera and scroll textures. */
void fn_1_EBDC(M674TextureWork *work) {
    Point3d cameraPosition;
    Point3d cameraScratch;
    f32 farPlane;
    f32 perspectiveScratch;
    f32 textureScale;

    Hu3DCameraPosGet(1, &cameraPosition, &cameraScratch, &cameraScratch);
    Hu3DCameraPerspectiveGet(1, &perspectiveScratch, &perspectiveScratch, &farPlane);
    farPlane *= 1.8;
    PSMTXScale(work->textureMatrices[0], farPlane, 1.0f, farPlane);
    PSMTXTransApply(work->textureMatrices[0], work->textureMatrices[0], cameraPosition.x, 0.0f,
                    cameraPosition.z);
    work->scrollX = (f32) ((f64) work->scrollX - 1.4285714285714286);
    work->scrollY = (f32) ((f64) work->scrollY - 0.5882352941176471);
    textureScale = farPlane / 600.0f;
    PSMTXScale(work->textureMatrices[1], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[1], work->textureMatrices[1],
                    (f32) (((f64) cameraPosition.x - (0.5 * (f64) work->scrollX)) / 600.0),
                    (f32) (((f64) cameraPosition.z + (0.8 * (f64) work->scrollY)) / 600.0), 0.0f);
    PSMTXScale(work->textureMatrices[2], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[2], work->textureMatrices[2],
                    (cameraPosition.x + work->scrollX) / 600.0f,
                    (cameraPosition.z + work->scrollY) / 600.0f, 0.0f);
    PSMTXScale(work->textureMatrices[3], textureScale, textureScale, 1.0f);
    PSMTXTransApply(work->textureMatrices[3], work->textureMatrices[3], cameraPosition.x / 600.0f,
                    cameraPosition.z / 600.0f, 0.0f);
}

/* The water draw hook calls this to capture the scene and apply the camera projection. */
void fn_1_EE3C(M674TextureWork *work, GXTexMapID map, u32 matrixId, Mtx inputMatrix) {
    Mtx perspectiveMatrix;
    f32 fov;
    f32 nearFar;

    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(work->textureBuffer, GX_FALSE);
    GXPixModeSync();
    GXLoadTexObj(&work->outputTexture, map);
    Hu3DCameraPerspectiveGet(1, &fov, &nearFar, &nearFar);
    C_MTXLightPerspective(perspectiveMatrix, fov, 1.2f, 0.5f,
                          -0.5f, 0.5f, 0.5f);
    PSMTXConcat(perspectiveMatrix, inputMatrix, perspectiveMatrix);
    GXLoadTexMtxImm(perspectiveMatrix, matrixId, GX_MTX3x4);
}

/* Bind the animated water image and its texture matrix for one TEV stage. */
void fn_1_EF30(M674TextureWork *work, GXTexMapID map, u32 matrixId) {
    GXLoadTexObj(&work->animationTexture, map);
    GXLoadTexMtxImm(work->textureMatrices[2], matrixId, GX_MTX2x4);
}

/* Bind the second animated water layer and its texture matrix. */
void fn_1_EF7C(M674TextureWork *work, s32 textureMap, u32 textureMatrix)
{
    HuSprTexLoad(work->animationData, 0, textureMap,
        GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXLoadTexMtxImm(work->textureMatrices[3], textureMatrix, GX_MTX2x4);
}

/* Set the texture map and coordinate source used by one indirect-texture stage. */
void fn_1_EFD8(s32 unused, GXIndTexStageID stage, GXTexMapID map, GXTexCoordID coord)
{
    GXSetIndTexOrder(stage, coord, map);
    GXSetIndTexCoordScale(stage, GX_ITS_1, GX_ITS_1);
}

static const f32 lbl_1_rodata_614[3][2][3] = {
    {{0.01f, 0.0f, 0.0f}, {0.0f, 0.01f, 0.0f}},
    {{0.02f, 0.0f, 0.0f}, {0.0f, 0.02f, 0.0f}},
    {{0.03f, 0.0f, 0.0f}, {0.0f, 0.03f, 0.0f}}
};
static const GXVtxDescList lbl_1_rodata_65C[4] = {
    {GX_VA_POS, GX_INDEX8}, {GX_VA_CLR0, GX_INDEX8},
    {GX_VA_TEX0, GX_INDEX8}, {GX_VA_NULL, GX_NONE}
};
static const GXVtxAttrFmtList lbl_1_rodata_67C[4] = {
    {GX_VA_POS, GX_POS_XYZ, GX_S16, 14},
    {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_S16, 14},
    {GX_VA_NULL, 0, 0, 0}
};
static const s16 lbl_1_rodata_6C0[32][3] ATTRIBUTE_ALIGN(32) = {
    {0, 0, 0},
    {0, 0, 8192},
    {-1761, 0, 8000},
    {-3439, 0, 7434},
    {-4957, 0, 6521},
    {-6243, 0, 5303},
    {-7237, 0, 3837},
    {-7893, 0, 2191},
    {-8179, 0, 443},
    {-8084, 0, -1325},
    {-7610, 0, -3032},
    {-6780, 0, -4597},
    {-5633, 0, -5947},
    {-4223, 0, -7019},
    {-2615, 0, -7763},
    {-885, 0, -8143},
    {885, 0, -8143},
    {2615, 0, -7763},
    {4223, 0, -7019},
    {5633, 0, -5947},
    {6780, 0, -4597},
    {7610, 0, -3032},
    {8084, 0, -1325},
    {8179, 0, 443},
    {7893, 0, 2191},
    {7237, 0, 3837},
    {6243, 0, 5303},
    {4957, 0, 6521},
    {3439, 0, 7434},
    {1761, 0, 8000}
};
static const s16 lbl_1_rodata_780[32][2] ATTRIBUTE_ALIGN(32) = {
    {8192, 8192},
    {8192, 16384},
    {6430, 16192},
    {4752, 15626},
    {3234, 14713},
    {1948, 13495},
    {954, 12029},
    {298, 10383},
    {12, 8635},
    {107, 6866},
    {581, 5159},
    {1411, 3594},
    {2558, 2244},
    {3968, 1172},
    {5576, 428},
    {7306, 48},
    {9077, 48},
    {10807, 428},
    {12415, 1172},
    {13825, 2244},
    {14972, 3594},
    {15802, 5159},
    {16276, 6866},
    {16371, 8635},
    {16085, 10383},
    {15429, 12029},
    {14435, 13495},
    {13149, 14713},
    {11631, 15626},
    {9953, 16192}
};
static const u8 lbl_1_rodata_800[96] ATTRIBUTE_ALIGN(32) = {
    160, 0, 31, 0, 1, 0, 1, 0, 1, 2, 0, 2,
    3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6,
    7, 0, 7, 8, 0, 8, 9, 0, 9, 10, 0, 10,
    11, 0, 11, 12, 0, 12, 13, 0, 13, 14, 0, 14,
    15, 0, 15, 16, 0, 16, 17, 0, 17, 18, 0, 18,
    19, 0, 19, 20, 0, 20, 21, 0, 21, 22, 0, 22,
    23, 0, 23, 24, 0, 24, 25, 0, 25, 26, 0, 26,
    27, 0, 27, 28, 0, 28, 29, 0, 29, 1, 0, 1
};

/* The water model hook calls this to configure TEV stages and draw the animated surface. */
void fn_1_F028(M674TextureWork *work, f32 (*viewMatrix)[4])
{
    Mtx combinedMatrix;
    PSMTXConcat(viewMatrix, work->textureMatrices[0],
        combinedMatrix);
    GXLoadPosMtxImm(combinedMatrix, 0U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_VTX, GX_SRC_VTX, 0U,
        GX_DF_NONE, GX_AF_NONE);
    fn_1_EE3C(work, GX_TEXMAP0, 30U, combinedMatrix);
    GXLoadTexObj(&work->animationTexture, GX_TEXMAP1);
    GXLoadTexMtxImm(work->textureMatrices[2], 33U,
        GX_MTX2x4);
    HuSprTexLoad(work->animationData, 0, 2, 1, 1, 1);
    GXLoadTexMtxImm(work->textureMatrices[3], 36U,
        GX_MTX2x4);
    GXLoadTexMtxImm(work->textureMatrices[1], 39U,
        GX_MTX2x4);
    GXSetNumTexGens(4U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, 30U,
        0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, 39U,
        0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, 33U,
        0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, 36U,
        0U, 125U);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_rodata_614[0], 0);
    GXSetIndTexMtx(GX_ITM_1, lbl_1_rodata_614[1], 0);
    GXSetIndTexMtx(GX_ITM_2, lbl_1_rodata_614[2], 0);
    GXSetNumIndStages(2U);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP2);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
    GXSetTevKColor(GX_KCOLOR0, lbl_1_rodata_5A4);
    GXSetNumTevStages(4U);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1U, 0U, GX_ITM_0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, 1U, 0U, GX_ITM_1);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA,
        GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
        GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_CPREV, GX_CC_RASC, GX_CC_RASA,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE3, GX_INDTEXSTAGE1, 1U, 0U, GX_ITM_2);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC,
        GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1U, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(1U);
    GXSetAlphaUpdate(0U);
    GXSetZCompLoc(1U);
    GXSetZMode(1U, GX_LESS, 1U);
    GXSetAlphaCompare(GX_ALWAYS, 255U, GX_AOP_OR, GX_ALWAYS, 255U);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    fn_1_F5D8(work);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetNumIndStages(0U);
}

/* Bind the water surface's indexed vertex arrays and call its display list. */
void fn_1_F5D8(M674TextureWork *work) {
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_65C);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_67C);
    GXSetArray(GX_VA_POS, (void *)lbl_1_rodata_6C0, 6U);
    GXSetArray(GX_VA_CLR0, (void *)lbl_1_rodata_5A0, 2U);
    GXSetArray(GX_VA_TEX0, (void *)lbl_1_rodata_780, 4U);
    GXCallDisplayList((void *)lbl_1_rodata_800, 96U);
}

/* Emit one rising particle near the supplied point when a particle slot is free. */
void fn_1_F660(Vec *origin)
{
    OMOBJ *manager = lbl_1_bss_6C0;
    GXColor *color = manager->data;
    HU3D_MODELID modelId = manager->mdlId[0];
    HU3D_PARTICLE *work = Hu3DData[modelId].hookData;
    HU3D_PARTICLE_DATA *slot = work->data;
    s32 index = 0;

    while (index < work->maxCnt) {
        if (slot->time < 0) {
            f32 randomX;
            f32 randomZ;

            slot->time = 0;
            randomX = (f32)(50.0 * (3.0517578125e-05 * (f32)((s32)(frand() & 0xFFFF) - 32768)));
            randomZ = (f32)(50.0 * (3.0517578125e-05 * (f32)((s32)(frand() & 0xFFFF) - 32768)));
            slot->pos.x = origin->x + randomX;
            slot->pos.y =
                (f32) (origin->y + (30.0 * (1.52587890625e-05 * (f32) (s32) (frand() & 0xFFFF))));
            slot->pos.z = origin->z + randomZ;
            slot->accel.x = (f32)(0.05 * (f64)randomX);
            slot->accel.y =
                (f32) (5.0 + (0.3 * (1.52587890625e-05 * (f64) (f32) (s32) (frand() & 0xFFFF))));
            slot->accel.z = (f32)(0.05 * (f64)randomZ);
            slot->scale =
                (f32) (40.0 + (20.0 * (1.52587890625e-05 * (f64) (f32) (s32) (frand() & 0xFFFF))));
            slot->zRot = (f32)(360.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));
            slot->color = *color;
            return;
        }
        index++;
        slot++;
    }
}

/* Add a fading white quad to the first free slot in the effect manager. */
void fn_1_F9A4(Vec *origin, f32 size, f32 scale, f32 scaleStep, u8 alpha)
{
    s32 index;
    OMOBJ *manager = lbl_1_bss_6C0;
    M674EffectWork *work = manager->data;
    M674EffectQuad *slot;

    for (index = 0; index < 128; index++) {
        slot = &work->quads[index];
        if (slot->enabled == 0) {
            slot->enabled = 1;
            slot->color.packed = (s32)alpha - 256;
            slot->size = size;
            slot->sizeDelta = scaleStep;
            slot->sizeScale = scale;
            slot->position = *origin;
            return;
        }
    }
}
