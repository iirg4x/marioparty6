// Creates and releases the player's ground-sprite model.
extern "C" {
#include <math.h>
}
#include "REL/m672DLL/effect_bf64.h"
extern "C" {
#include "math.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
extern "C" {
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/data.h"
}
#include "REL/m672DLL/sprite_transform.h"
#include "REL/m672DLL/owner_9314.h"

GroundSprite::GroundSprite(ANIMDATA *image, const Vec *position)
    : GameObject(16), OwnedModel(fn_1_8EAC), animation(image), origin(*position)
{
    radius = 1.0f;
    alpha = 1.0f;
    setHookData(this);
    fn_1_33CB0(2);
}
GroundSprite::~GroundSprite() {}
void fn_1_8E0C(s16 *modelIds, u8 reset)
{
    if (reset == 1) {
        s16 *modelIdPointer = modelIds;
        s16 *modelIdAddress;
        s16 modelId;
        s16 modelIdArgument;
        if (modelIdPointer) {
            modelIdPointer += 8;
        }
        modelIdAddress = modelIdPointer;
        modelId = (s16)*modelIdAddress;
        modelIdArgument = modelId;
        fn_80026A44(modelIdArgument, 1);
        return;
    }

    {
        s16 *modelIdPointer = modelIds;
        s16 *modelIdAddress;
        s16 modelId;
        s16 modelIdArgument;
        if (modelIdPointer) {
            modelIdPointer += 8;
        }
        modelIdAddress = modelIdPointer;
        modelId = (s16)*modelIdAddress;
        modelIdArgument = modelId;
        fn_800269E8(modelIdArgument, 1);
    }
}

void GroundSprite::update() {}

void fn_1_8EAC(HU3D_MODEL *model, Mtx *matrix)
{
    static_cast<SpriteTransform *>(model->hookData)->fn_1_8EE4(*matrix);
}

extern "C" void HuSprTexLoad(ANIMDATA *, s16, s16, GXTexWrapMode, GXTexWrapMode, GXTexFilter);

/* Draws a textured sprite with the object's transform and opacity. */
void SpriteTransform::fn_1_8EE4(Mtx parentMatrix)
{
    Mtx transform;

    PSMTXTrans(transform, x, y, z);
    mtxScaleCat(transform, scale, scale, scale);
    PSMTXConcat(parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    HuSprTexLoad(texture, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    {
        GXColor color = { 0, 0, 64, 128 };
        color.a = (u8)(color.a * alpha);
        GXSetTevColor(GX_TEVREG0, color);
    }
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-0.5f, 0.0f, -0.5f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.5f, 0.0f, -0.5f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(0.5f, 0.0f, 0.5f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-0.5f, 0.0f, 0.5f);
    GXTexCoord2f32(1.0f, 0.0f);
}
