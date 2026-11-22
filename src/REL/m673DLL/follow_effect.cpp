// Draws an effect quad in the XZ plane at the supplied position.

#include "game/sprput.h"
#include "REL/m673DLL/follow_effect.h"
#include "REL/m673DLL/callback_receiver.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
}

// Attaches an animated quad to a world position.
FollowEffect::FollowEffect(const Vec *position, f32 scale, ANIMDATA *animation)
    : RenderObject(4), position(position), scale(scale), animation(animation), visible(1)
{
    setLayer(6);
    // Default white tint for the effect quad.
    static const GXColor value = { 0xFF, 0xFF, 0xFF, 0xFF };
    color = value;
    alpha = 1.0f;
}

// Releases the quad render object.
FollowEffect::~FollowEffect()
{
}

// Leaves the particle quad anchored to its position source.
void FollowEffect::update()
{
}

// Draws the effect using its animation and captured scene texture.
void FollowEffect::draw(Mtx *transform)
{
    GXColor drawColor;
    Mtx textureMatrix;
    Mtx perspective;
    GXTexObj texture;
    CallbackReceiver *textureSource;
    GXTexObj *texturePointer;
    HU3D_CAMERA *camera;
    f32 halfWidth;

    if (this->visible != 0 && this->alpha != 0.0f) {
        GXLoadPosMtxImm(*transform, GX_PNMTX0);
        camera = Hu3DCamera;
        GXSetNumTexGens(2);
        C_MTXLightPerspective(perspective, camera->fov, camera->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
        PSMTXConcat(perspective, *transform, textureMatrix);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
        GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        textureSource = &CallbackReceiver::getChecked();
        texturePointer = &textureSource->texture;
        texture = *texturePointer;
        GXLoadTexObj(&texture, GX_TEXMAP0);
        HuSprTexLoad(this->animation, 0, GX_TEXMAP1, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        GXSetZCompLoc(GX_TRUE);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
        drawColor = this->color;
        drawColor.a = (u8)(drawColor.a * this->alpha);
        GXSetTevColor(GX_TEVREG0, drawColor);
        GXSetNumTevStages(2);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP,
                      GX_AF_SPOT);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        halfWidth = 0.5f * this->scale;
        Vec worldPosition = *this->position;
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-halfWidth + worldPosition.x, worldPosition.y, -halfWidth + worldPosition.z);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(halfWidth + worldPosition.x, worldPosition.y, -halfWidth + worldPosition.z);
        GXTexCoord2f32(0.0f, 1.0f);
        GXPosition3f32(halfWidth + worldPosition.x, worldPosition.y, halfWidth + worldPosition.z);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(-halfWidth + worldPosition.x, worldPosition.y, halfWidth + worldPosition.z);
        GXTexCoord2f32(1.0f, 0.0f);
    }
}

// Sets the effect's visibility flag.
void fn_1_BFBC(FollowEffect *effect, u8 visible)
{
    effect->visible = visible;
}
