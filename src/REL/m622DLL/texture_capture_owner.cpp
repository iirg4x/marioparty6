extern "C" {
#include "dolphin/types.h"
}

#include <math.h>

extern "C" {
#include "dolphin/gx.h"
}

#include "dolphin/mtx.h"

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

extern "C" {
#include "game/mg/score.h"
}

#include "game/pad.h"

#include "game/frand.h"

extern "C" {
#include "game/sprite.h"
}

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"
}

#include "datadir_enum.h"
#include "REL/m622DLL/texture_capture_owner.h"

typedef s16 HUSPR_GROUPID;

extern "C" {

}

extern "C" { void *lbl_1_bss_F8; }

extern "C" { void *lbl_1_bss_FC; }

extern "C" { void __msl_assertion_failed(const char *, const char *, int); }

void captureFirstTexture(s16 layerNo);

void captureSecondTexture(s16 layerNo);

void fn_1_989C(HU3D_MODEL *, Mtx *);

extern "C" { void fn_8002C2AC(s16); }

/* Initializes the capture model, two texture buffers and their drawing hooks. */
TextureCaptureOwner::TextureCaptureOwner()
    : OwnedModel(fn_1_989C),
      firstBuffer(GXGetTexBufferSize(640, 480, GX_TF_I4, 0, 0)),
      secondBuffer(GXGetTexBufferSize(640, 480, GX_TF_I4, 0, 0))
{
    fn_1_33E80(this);
    fn_1_33CB0(4);
    GXInitTexObj(&textures[0], firstBuffer.begin(),
                 640, 480, GX_TF_I4, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObj(&textures[1], secondBuffer.begin(),
                 640, 480, GX_TF_I4, GX_CLAMP, GX_CLAMP, 0);
    lbl_1_bss_F8 = firstBuffer.begin();
    lbl_1_bss_FC = secondBuffer.begin();
    fn_80025918(8, captureFirstTexture);
    fn_80025918(9, captureSecondTexture);
    fn_8002C2AC(1);
}

void captureFirstTexture(s16 layerNo) {
    GXSetTexCopySrc(0U, 0U, 0x280U, 0x1E0U);
    GXSetTexCopyDst(0x280U, 0x1E0U, GX_TF_I4, 0U);
    GXCopyTex(lbl_1_bss_F8, 1U);
}

void captureSecondTexture(s16 layerNo) {
    GXSetTexCopySrc(0U, 0U, 0x280U, 0x1E0U);
    GXSetTexCopyDst(0x280U, 0x1E0U, GX_TF_I4, 0U);
    GXCopyTex(lbl_1_bss_FC, 1U);
}

TextureCaptureOwner::~TextureCaptureOwner()
{
}

void TextureCaptureOwner::clearFlag(int bit)
{
    flags &= ~(1U << bit);
}

extern void fn_1_98D4(TextureCaptureOwner *, Mtx);

extern "C" void fn_80027D8C(int cameraBit, f32 *fov, f32 *nearPlane, f32 *farPlane);

void fn_1_9C30(void *);

void TextureCaptureOwner::update()
{
}

void fn_1_989C(HU3D_MODEL *model, Mtx *viewMatrix)
{
    fn_1_98D4(static_cast<TextureCaptureOwner *>(model->hookData), *viewMatrix);
}

const GXColor lbl_1_rodata_328 = {0, 32, 0, 0};
const GXColor lbl_1_rodata_32C = {192, 255, 192, 0};

void fn_1_98D4(TextureCaptureOwner *owner, Mtx viewMatrix)
{
    Mtx projectionMatrix;
    f32 fieldOfView;
    f32 aspect;

    GXLoadPosMtxImm(viewMatrix, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXLoadTexObj(&owner->textures[1], GX_TEXMAP0);
    GXLoadTexObj(&owner->textures[0], GX_TEXMAP1);
    fn_80027D8C(1, &fieldOfView, &aspect, &aspect);
    C_MTXLightPerspective(projectionMatrix, fieldOfView, 1.2f,
                          0.5f, -0.5f,
                          0.5f, 0.5f);
    PSMTXConcat(projectionMatrix, viewMatrix, projectionMatrix);
    GXLoadTexMtxImm(projectionMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0,
                      GX_FALSE, GX_PTIDENTITY);

    {
        GXColor color0 = lbl_1_rodata_328;
        GXColor color1 = lbl_1_rodata_32C;

        GXSetTevKColor(GX_KCOLOR0, color0);
        GXSetTevKColor(GX_KCOLOR1, color1);
    }
    GXSetNumTevStages(2);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXA, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    fn_1_9C30(owner);
}

const GXVtxDescList lbl_1_rodata_33C[] = {
    {GX_VA_POS, GX_DIRECT}, {GX_VA_NULL, GX_NONE}
};
const GXVtxAttrFmtList lbl_1_rodata_34C[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_NULL, GX_POS_XY, GX_U8, 0}
};

void fn_1_9C30(void *owner)
{
    s32 i;

    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_33C);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_34C);
    for (i = 0; i < 4; i++) {
        f32 leftX = -548.365f;
        f32 rightX = -421.511f;
        f32 topY = 215.392f;
        f32 bottomY = 121.669f;
        f32 topZ = (f32)(-512.365 + (f64)(i * 0x122));
        f32 bottomZ = (f32)(-440.48 + (f64)(i * 0x122));

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(leftX, topY, topZ);
        GXPosition3f32(rightX, topY, topZ);
        GXPosition3f32(rightX, bottomY, bottomZ);
        GXPosition3f32(leftX, bottomY, bottomZ);
    }
}
