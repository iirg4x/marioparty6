/* Builds the captured-water textures and their Daft Rafts render passes. */
#include "datadir_enum.h"
#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include <string.h>

typedef struct M609TextureResources {
    s16 modelId;
    u8 beforeFrameBuffers[6];
    void *frameBufferA;
    void *frameBufferB;
    void *frameBufferC;
    ANIMDATA *animationA;
    ANIMDATA *animationB;
    ANIMDATA *animationC;
    u8 remainder[4];
} M609TextureResources;

typedef struct M609TextureInput {
    u8 beforeMatrix[12];
    Mtx matrix;
} M609TextureInput;

M609TextureResources lbl_1_bss_6E68;

f32 lbl_1_data_858[2][3] = {{0.05f,0.0f,0.0f},{0.0f,0.05f,0.0f}};
void fn_1_16C0C(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
extern void HuSprTexLoad(ANIMDATA *anim, s16 bitmapNo, s16 texMapId,
                         GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                         GXTexFilter filter);

extern s16 lbl_802C0278;

/* Loads water assets and capture buffers during model setup in fn_1_D1CC. */
void fn_1_1661C(void) {
    s16 nightMode;

    lbl_1_bss_6E68.modelId = fn_80025A04(fn_80008DD0(
        DATANUM(0x440000, 45), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_6E68.modelId, 4);
    fn_800269E8(lbl_1_bss_6E68.modelId, ((1 << 30) | (1 << 6)));
    fn_80027884(lbl_1_bss_6E68.modelId, fn_1_16C0C);
    lbl_1_bss_6E68.frameBufferA = HuMemDirectMallocNum(
        2, GXGetTexBufferSize(640, 480, GX_TF_RGB565, 1, 1),
        0x10000000);
    lbl_1_bss_6E68.frameBufferB = HuMemDirectMallocNum(
        2, GXGetTexBufferSize(640, 480, GX_TF_RGBA8, 0, 0),
        0x10000000);
    lbl_1_bss_6E68.frameBufferC = HuMemDirectMallocNum(
        2, GXGetTexBufferSize(640, 480, GX_TF_RGB565, 1, 1),
        0x10000000);
    lbl_1_bss_6E68.animationA = HuSprAnimRead(fn_80008DD0(
        DATANUM(0x210000, 10), 0x10000000, 2));
    lbl_1_bss_6E68.animationB = HuSprAnimRead(fn_80008DD0(
        DATANUM(0x440000, 47), 0x10000000, 2));
    nightMode = lbl_802C0278;
    if (nightMode == 0) {
        lbl_1_bss_6E68.animationC = HuSprAnimRead(fn_80008DD0(
            DATANUM(0x440000, 48), 0x10000000, 2));
    } else {
        lbl_1_bss_6E68.animationC = HuSprAnimRead(fn_80008DD0(
            DATANUM(0x440000, 49), 0x10000000, 2));
    }
    fn_8002BDB4(2, 0, 0, 640, 480, GX_TF_RGBA8, 0,
                       lbl_1_bss_6E68.frameBufferB);
}

/* Unused draw helper: samples frameBufferA as 320 by 240 RGB565 with alpha blending;
 * this module allocates that buffer without filling it. */
void fn_1_1681C(M609TextureInput *textureInput) {
    Mtx compositeMatrix;
    Mtx textureMatrix;
    Mtx perspectiveMatrix;
    Mtx inverseCameraMatrix;
    HU3D_CAMERA *camera;

    camera = &Hu3DCamera[lbl_802C01A6];
    fn_800360D8(lbl_1_bss_6E68.frameBufferA, 320, 240,
                GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 1, GX_TEXMAP0);
    C_MTXLightPerspective(perspectiveMatrix, camera->fov, camera->aspect,
                          (0.5f), (-0.5f),
                          (0.5f), (0.5f));
    PSMTXInverse(lbl_802613F4, inverseCameraMatrix);
    PSMTXConcat(inverseCameraMatrix, textureInput->matrix, compositeMatrix);
    PSMTXConcat(perspectiveMatrix, lbl_802613F4, textureMatrix);
    PSMTXConcat(textureMatrix, compositeMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS,
                      GX_TEXMTX0, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_REG, 0,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, 0, GX_SRC_REG, GX_SRC_REG, 0,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(1, GX_GREATER, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
}

/* Unused draw helper: samples the 640 by 480 scene capture with alpha blending. */
void fn_1_16A38(M609TextureInput *textureInput) {
    Mtx compositeMatrix;
    Mtx textureMatrix;
    Mtx perspectiveMatrix;
    Mtx inverseCameraMatrix;
    HU3D_CAMERA *camera;

    camera = &Hu3DCamera[lbl_802C01A6];
    fn_800360D8(lbl_1_bss_6E68.frameBufferB, 640, 480,
                GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 1, GX_TEXMAP0);
    C_MTXLightPerspective(perspectiveMatrix, camera->fov, camera->aspect,
                          (0.5f), (-0.5f),
                          (0.5f), (0.5f));
    PSMTXInverse(lbl_802613F4, inverseCameraMatrix);
    PSMTXConcat(inverseCameraMatrix, textureInput->matrix, compositeMatrix);
    PSMTXConcat(perspectiveMatrix, lbl_802613F4, textureMatrix);
    PSMTXConcat(textureMatrix, compositeMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS,
                      GX_TEXMTX0, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
}

const GXColor lbl_1_rodata_10D0 = {96,96,96,0};
const GXColor lbl_1_rodata_10D4 = {160,160,160,0};

/* Configures the water material in the hook set by fn_1_1661C;
 * taki_yoko gets a fourth TEV stage. */
void fn_1_16C0C(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material) {
    Mtx transformMatrix;
    Mtx textureMatrix;
    Mtx perspectiveMatrix;
    Mtx inverseCameraMatrix;
    HU3D_CAMERA *camera;
    HU3D_ATTR_ANIM *attributeAnimation;
    GXColor waterColor;
    GXColor captureTint;
    s32 specialObject;
    s16 lightMask;

    camera = &Hu3DCamera[lbl_802C01A6];
    waterColor = lbl_1_rodata_10D0;
    captureTint = lbl_1_rodata_10D4;
    specialObject = 0;
    if (strcmp(drawObj->object->name, "taki_yoko") == 0) {
        specialObject = 1;
    }

    fn_800360D8(lbl_1_bss_6E68.frameBufferB, 640, 480,
                GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_TRUE, GX_TEXMAP2);
    HuSprTexLoad(lbl_1_bss_6E68.animationA, 0, GX_TEXMAP3,
                 GX_REPEAT, GX_REPEAT, GX_LINEAR);
    if (specialObject) {
        HuSprTexLoad(lbl_1_bss_6E68.animationB, 0, GX_TEXMAP1,
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    HuSprTexLoad(lbl_1_bss_6E68.animationC, 0, GX_TEXMAP4,
                 GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);

    C_MTXLightPerspective(perspectiveMatrix, camera->fov, camera->aspect,
                          (0.5f), (-0.5f),
                          (0.5f), (0.5f));
    PSMTXInverse(lbl_802613F4, inverseCameraMatrix);
    PSMTXConcat(inverseCameraMatrix, drawObj->matrix, transformMatrix);
    PSMTXConcat(perspectiveMatrix, lbl_802613F4, textureMatrix);
    PSMTXConcat(textureMatrix, transformMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);

    attributeAnimation = (HU3D_ATTR_ANIM *)drawObj->object->mesh.attribute[
        material->attr[0]].animWorkP;
    PSMTXTrans(textureMatrix, attributeAnimation->trans3D.x,
               attributeAnimation->trans3D.y, attributeAnimation->trans3D.z);
    PSMTXInverse(textureMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX1, GX_MTX2x4);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS,
                      GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0);
    GXSetTevColor(GX_TEVREG2, captureTint);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_C2, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_POS,
                      GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP4, GX_COLOR0);
    GXSetTevColor(GX_TEVREG0, waterColor);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC,
                    GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXA,
                    GX_CC_RASA, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP3);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE,
                    GX_FALSE, GX_ITM_0);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD1, GX_TEXMAP3);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE,
                    GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_data_858, -1);
    /* Only indirect stage 0 is enabled, although stage 1 is also configured above. */
    GXSetNumIndStages(1);

    if (specialObject) {
        attributeAnimation = (HU3D_ATTR_ANIM *)drawObj->object->mesh.attribute[
            material->attr[0]].animWorkP;
        PSMTXTrans(textureMatrix, attributeAnimation->trans3D.x,
                   attributeAnimation->trans3D.y, attributeAnimation->trans3D.z);
        PSMTXInverse(textureMatrix, textureMatrix);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX3, GX_MTX2x4);
        GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX3, GX_FALSE, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, GX_TEXMAP1, GX_COLOR0);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_TEXA,
                        GX_CC_RASA, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTexGens(4);
        GXSetNumTevStages(4);
    } else {
        GXSetNumTexGens(3);
        GXSetNumTevStages(3);
    }

    GXSetNumChans(1);
    lightMask = fn_80029E8C(drawObj->model, lbl_802613F4,
                          lbl_80261424, (0.0f));
    GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_ALPHA0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX,
                  (u32)lightMask, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
}
