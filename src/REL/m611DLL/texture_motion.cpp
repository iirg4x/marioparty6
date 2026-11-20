#include "REL/m611DLL/texture_owner.h"
#include "datadir_enum.h"

extern "C" {
#include "game/gamework.h"
#include "game/sprput.h"
#include "game/hu3d.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
}

// Packed RGBA4 vertex colors selected for the day variant.
const u16 lbl_1_rodata_A20[2] ATTRIBUTE_ALIGN(0x20) = {
    0x17F9, 0x5741
};

// Constant color sent to GX_KCOLOR0 for the day variant.
const GXColor lbl_1_rodata_A24 = { 16, 16, 16, 5 };

// Packed RGBA4 vertex colors selected for the night variant.
const u16 lbl_1_rodata_A40[2] ATTRIBUTE_ALIGN(0x20) = {
    0x000B, 0x0361
};

// Constant color sent to GX_KCOLOR0 for the night variant.
const GXColor lbl_1_rodata_A44 = { 16, 16, 16, 9 };

// Loads the textures and initializes the four texture transforms.
TextureOwner::TextureOwner()
    : OwnedModel(fn_1_12CB4),
      textureStorage(GXGetTexBufferSize(320, 240, GX_TF_RGB565, GX_FALSE, 0))
{
    // Loads raw texels for GXInitTexObj.
    imageData = fn_80008DD0(DATANUM(0x460000, 37), 0x10000000, 2);
    spriteAnimation = HuSprAnimDataRead(DATANUM(0x460000, 38));
    scrollX = 0.0f;
    scrollY = 0.0f;
    setHookData(this);
    setLayer(2);
    PSMTXIdentity(textureMatrices[0]);
    PSMTXIdentity(textureMatrices[1]);
    PSMTXIdentity(textureMatrices[2]);
    PSMTXIdentity(textureMatrices[3]);
    GXInitTexObj(&baseTexture, textureStorage.begin(), 320, 240, GX_TF_RGB565, GX_CLAMP,
        GX_CLAMP, GX_FALSE);
    GXInitTexObj(&imageTexture, imageData, 128, 128, GX_TF_IA8, GX_REPEAT, GX_REPEAT, GX_TRUE);
    GXInitTexObjLOD(&imageTexture, GX_LIN_MIP_LIN, GX_LINEAR,
                    0.0f, 7.0f, 0.0f,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
}

// Releases the texture model and its texel storage.
TextureOwner::~TextureOwner()
{
}

// Acknowledges a texture effect request.
void TextureOwner::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
}

// Reads the camera position and scales its far plane.
void fn_1_129EC(Vec *position, f32 *farPlane)
{
    Vec cameraUpOrTarget;
    f32 fieldOfViewOrNearPlane;

    fn_80027CD0(1, position, &cameraUpOrTarget, &cameraUpOrTarget);
    fn_80027D8C(1, &fieldOfViewOrNearPlane, &fieldOfViewOrNearPlane, farPlane);
    // Scales the disc to a radius of 0.9 times the far clip distance.
    *farPlane *= 1.8;
}

// Updates the camera-facing model and scrolling texture transforms.
void TextureOwner::update()
{
    Vec position;
    f32 scale;
    f32 layerScale;

    fn_1_129EC(&position, &scale);
    PSMTXScale(textureMatrices[0], scale, 1.0f, scale);
    PSMTXTransApply(textureMatrices[0], textureMatrices[0], position.x, 0.0f, position.z);
    scrollX = scrollX - (10.0f / 7.0f);
    scrollY = scrollY - (10.0f / 17.0f);
    // Each layer repeats over 600 world units.
    layerScale = scale / 600.0f;
    PSMTXScale(textureMatrices[1], layerScale, layerScale, 1.0f);
    PSMTXTransApply(textureMatrices[1], textureMatrices[1],
        static_cast<f32>(
            (static_cast<f64>(position.x) - (0.5 * static_cast<f64>(scrollX))) / 600.0),
        static_cast<f32>(
            (static_cast<f64>(position.z) + (0.8 * static_cast<f64>(scrollY))) / 600.0),
        0.0f);
    PSMTXScale(textureMatrices[2], layerScale, layerScale, 1.0f);
    PSMTXTransApply(textureMatrices[2], textureMatrices[2], (position.x + scrollX) / 600.0f,
        (position.z + scrollY) / 600.0f, 0.0f);
    PSMTXScale(textureMatrices[3], layerScale, layerScale, 1.0f);
    PSMTXTransApply(textureMatrices[3], textureMatrices[3], position.x / 600.0f,
        position.z / 600.0f, 0.0f);
}

// Draws the texture effect attached to the model hook.
void fn_1_12CB4(HU3D_MODEL *model, Mtx *matrix)
{
    fn_1_12F00(static_cast<TextureOwner *>(model->hookData), *matrix);
}

// Copies the frame buffer into the base texture and loads its projection.
void fn_1_12CEC(TextureOwner *owner, GXTexMapID map, u32 matrixId, Mtx inputMatrix)
{
    Mtx perspectiveMatrix;
    f32 fov;
    f32 nearOrFarPlane;

    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
    GXCopyTex(owner->textureStorage.begin(), GX_FALSE);
    GXPixModeSync();
    GXLoadTexObj(&owner->baseTexture, map);
    fn_80027D8C(1, &fov, &nearOrFarPlane, &nearOrFarPlane);
    C_MTXLightPerspective(perspectiveMatrix, fov, 1.2f, 0.5f,
                          -0.5f, 0.5f, 0.5f);
    PSMTXConcat(perspectiveMatrix, inputMatrix, perspectiveMatrix);
    GXLoadTexMtxImm(perspectiveMatrix, matrixId, GX_MTX3x4);
}

// Indirect-texture matrices passed to GXSetIndTexMtx.
const f32 lbl_1_rodata_A8C[3][2][3] = {
    {
        { 0.01f, 0.0f, 0.0f },
        { 0.0f, 0.01f, 0.0f }
    },
    {
        { 0.02f, 0.0f, 0.0f },
        { 0.0f, 0.02f, 0.0f }
    },
    {
        { 0.03f, 0.0f, 0.0f },
        { 0.0f, 0.03f, 0.0f }
    }
};

// Vertex attribute descriptors passed to GXSetVtxDescv.
const GXVtxDescList lbl_1_rodata_AD4[4] = {
    { GX_VA_POS, GX_INDEX8 },
    { GX_VA_CLR0, GX_INDEX8 },
    { GX_VA_TEX0, GX_INDEX8 },
    { GX_VA_NULL, GX_NONE }
};

// Vertex attribute formats passed to GXSetVtxAttrFmtv.
const GXVtxAttrFmtList lbl_1_rodata_AF4[4] = {
    { GX_VA_POS, GX_POS_XYZ, GX_S16, 14 },
    { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_S16, 14 },
    { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }
};

// Vertex positions supplied for the texture-effect display list.
const s16 lbl_1_rodata_B40[] ATTRIBUTE_ALIGN(0x20) = {
    0, 0, 0,   0, 0, 8192,   -1761, 0, 8000,
    -3439, 0, 7434,   -4957, 0, 6521,   -6243, 0, 5303,
    -7237, 0, 3837,   -7893, 0, 2191,   -8179, 0, 443,
    -8084, 0, -1325,   -7610, 0, -3032,   -6780, 0, -4597,
    -5633, 0, -5947,   -4223, 0, -7019,   -2615, 0, -7763,
    -885, 0, -8143,   885, 0, -8143,   2615, 0, -7763,
    4223, 0, -7019,   5633, 0, -5947,   6780, 0, -4597,
    7610, 0, -3032,   8084, 0, -1325,   8179, 0, 443,
    7893, 0, 2191,   7237, 0, 3837,   6243, 0, 5303,
    4957, 0, 6521,   3439, 0, 7434,   1761, 0, 8000,
    0, 0, 0,   0, 0, 0,
};

// Texture coordinates supplied for the texture-effect display list.
const s16 lbl_1_rodata_C00[] ATTRIBUTE_ALIGN(0x20) = {
    8192, 8192, 8192, 16384, 6430, 16192, 4752, 15626,
    3234, 14713, 1948, 13495, 954, 12029, 298, 10383,
    12, 8635, 107, 6866, 581, 5159, 1411, 3594,
    2558, 2244, 3968, 1172, 5576, 428, 7306, 48,
    9077, 48, 10807, 428, 12415, 1172, 13825, 2244,
    14972, 3594, 15802, 5159, 16276, 6866, 16371, 8635,
    16085, 10383, 15429, 12029, 14435, 13495, 13149, 14713,
    11631, 15626, 9953, 16192, 0, 0, 0, 0
};

// GX display-list commands for the texture effect.
const u8 lbl_1_rodata_C80[0x60] ATTRIBUTE_ALIGN(0x20) = {
    0xA0, 0x00, 0x1F, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x01, 0x02, 0x00, 0x02, 0x03, 0x00, 0x03, 0x04,
    0x00, 0x04, 0x05, 0x00, 0x05, 0x06, 0x00, 0x06,
    0x07, 0x00, 0x07, 0x08, 0x00, 0x08, 0x09, 0x00,
    0x09, 0x0A, 0x00, 0x0A, 0x0B, 0x00, 0x0B, 0x0C,
    0x00, 0x0C, 0x0D, 0x00, 0x0D, 0x0E, 0x00, 0x0E,
    0x0F, 0x00, 0x0F, 0x10, 0x00, 0x10, 0x11, 0x00,
    0x11, 0x12, 0x00, 0x12, 0x13, 0x00, 0x13, 0x14,
    0x00, 0x14, 0x15, 0x00, 0x15, 0x16, 0x00, 0x16,
    0x17, 0x00, 0x17, 0x18, 0x00, 0x18, 0x19, 0x00,
    0x19, 0x1A, 0x00, 0x1A, 0x1B, 0x00, 0x1B, 0x1C,
    0x00, 0x1C, 0x1D, 0x00, 0x1D, 0x01, 0x00, 0x01
};

// Loads the image texture and its transform.
void fn_1_12E04(TextureOwner *owner, GXTexMapID map, u32 matrixId)
{
    GXLoadTexObj(&owner->imageTexture, map);
    GXLoadTexMtxImm(owner->textureMatrices[2], matrixId, GX_MTX2x4);
}

// Loads the sprite texture and its transform.
void fn_1_12E50(TextureOwner *owner, s32 textureMap, u32 matrixId)
{
    HuSprTexLoad(owner->spriteAnimation, 0, static_cast<s16>(textureMap), GX_REPEAT, GX_REPEAT,
        GX_LINEAR);
    GXLoadTexMtxImm(owner->textureMatrices[3], matrixId, GX_MTX2x4);
}

// Configures one indirect texture stage.
void fn_1_12EB0(s32, GXIndTexStageID stage, GXTexMapID map, GXTexCoordID coord)
{
    GXSetIndTexOrder(stage, coord, map);
    GXSetIndTexCoordScale(stage, GX_ITS_1, GX_ITS_1);
}

// Configures the texture stages and selected constant color.
void fn_1_12F00(TextureOwner *owner, Mtx viewMatrix)
{
    Mtx modelViewMatrix;
    PSMTXConcat(viewMatrix, owner->textureMatrices[0], modelViewMatrix);
    GXLoadPosMtxImm(modelViewMatrix, GX_PNMTX0);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    fn_1_12CEC(owner, GX_TEXMAP0, GX_TEXMTX0, modelViewMatrix);
    fn_1_12E04(owner, GX_TEXMAP1, GX_TEXMTX1);
    fn_1_12E50(owner, GX_TEXMAP2, GX_TEXMTX2);
    GXLoadTexMtxImm(owner->textureMatrices[1], GX_TEXMTX3, GX_MTX2x4);
    GXSetNumTexGens(4U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_rodata_A8C[0], 0);
    GXSetIndTexMtx(GX_ITM_1, lbl_1_rodata_A8C[1], 0);
    GXSetIndTexMtx(GX_ITM_2, lbl_1_rodata_A8C[2], 0);
    GXSetNumIndStages(2U);
    fn_1_12EB0(0, GX_INDTEXSTAGE0, GX_TEXMAP1, GX_TEXCOORD2);
    fn_1_12EB0(0, GX_INDTEXSTAGE1, GX_TEXMAP2, GX_TEXCOORD3);
    if (GWMgNightFGet() == 0) {
        GXSetTevKColor(GX_KCOLOR0, lbl_1_rodata_A24);
    } else {
        GXSetTevKColor(GX_KCOLOR0, lbl_1_rodata_A44);
    }
    GXSetNumTevStages(4U);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_1);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_CPREV, GX_CC_RASC, GX_CC_RASA, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevIndWarp(GX_TEVSTAGE3, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_2);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD2, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 255, GX_AOP_OR, GX_ALWAYS, 255);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    fn_1_13510(owner);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetNumIndStages(0U);
}

// Sets the vertex arrays and draws the texture-effect display list.
void fn_1_13510(TextureOwner *)
{
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_AD4);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_AF4);
    GXSetArray(GX_VA_POS, const_cast<s16 *>(lbl_1_rodata_B40), 3 * sizeof(s16));
    if (GWMgNightFGet() == 0) {
        GXSetArray(GX_VA_CLR0, const_cast<u16 *>(lbl_1_rodata_A20), sizeof(u16));
    } else {
        GXSetArray(GX_VA_CLR0, const_cast<u16 *>(lbl_1_rodata_A40), sizeof(u16));
    }
    GXSetArray(GX_VA_TEX0, const_cast<s16 *>(lbl_1_rodata_C00), 2 * sizeof(s16));
    GXCallDisplayList(const_cast<u8 *>(lbl_1_rodata_C80), sizeof(lbl_1_rodata_C80));
}
