// Draws the ground sprite and handles its model attributes.

#include "REL/m673DLL/ground_sprite.h"
#include "game/sprput.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "dolphin/gx.h"
#include "game/animdata.h"
extern "C" {
#include "game/hu3d.h"
}

// Creates a ground sprite at the supplied world position.
GroundSprite::GroundSprite(ANIMDATA *animation, const Vec *origin)
    : GameObject(16), OwnedModel(fn_1_8718), animation(animation),
      origin(*origin), radius(1.0f), alpha(1.0f)
{
    setHookData(this);
    setLayer(2);
}

// Releases the ground-sprite model.
GroundSprite::~GroundSprite()
{
}

// Shows the sprite when the flag equals 1 and hides it for every other value.
void fn_1_8678(GroundSprite *sprite, u8 reset)
{
    if (reset == 1) {
        fn_80026A44(*sprite, 0x1);
        return;
    }
    fn_800269E8(*sprite, 0x1);
}

// Leaves the ground sprite unchanged between draws.
void GroundSprite::update()
{
}

// Draws the ground sprite through its model callback.
void fn_1_8718(HU3D_MODEL *model, Mtx *matrix)
{
    static_cast<GroundSprite *>(model->hookData)->fn_1_8EE4(*matrix);
}

// Draws the ground sprite in the supplied transform.
void GroundSprite::fn_1_8EE4(Mtx parentMatrix)
{
    Mtx transform;

    PSMTXTrans(transform, this->origin.x, this->origin.y, this->origin.z);
    mtxScaleCat(transform, this->radius, this->radius, this->radius);
    PSMTXConcat(parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    HuSprTexLoad(this->animation, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    GXColor color = { 0x00, 0x00, 0x40, 0x80 };
    color.a = (u8)(color.a * this->alpha);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
        GX_AF_NONE);
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
