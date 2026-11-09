#include "REL/m654DLL/sprite_draw.h"
#include "REL/m654DLL/parameter_interpolation.h"
#include "dolphin/os.h"

#include "REL/framework/model.h"


SpriteParameters::SpriteParameters(ANIMDATA *image) {
 halfWidth = (float)((double)image->bmp->sizeX / 2.0);
 halfHeight = (float)((double)image->bmp->sizeY / 2.0);
 animation = image;
}

SpriteParameters &SpriteParameters::operator=(const SpriteParameters &source) {
 animation = source.animation;
 halfWidth = source.halfWidth;
 halfHeight = source.halfHeight;
 x = source.x;
 y = source.y;
 scaleX = source.scaleX;
 scaleY = source.scaleY;
 return *this;
}

#include <stddef.h>

extern "C" {

const GXVtxDescList lbl_1_rodata_360[3] = {
    {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}
};
const GXVtxAttrFmtList lbl_1_rodata_378[3] = {
    {GX_VA_POS, GX_POS_XY, GX_F32, 0}, {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0}, {GX_VA_NULL, GX_POS_XY, GX_U8, 0}
};
const GXColor lbl_1_rodata_3A8 = {0, 0, 0, 0};
const float lbl_1_rodata_3AC = 0.0f;
const float lbl_1_rodata_3B0 = 255.0f;
const float lbl_1_rodata_3B4 = 1.0f;
}

void fn_1_81B4(void)
{
    float matrix[3][4];
    float (*matrixPointer)[4];

    PSMTXIdentity(matrix);
    matrixPointer = matrix;
    GXLoadPosMtxImm(matrixPointer, GX_PNMTX0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(1);
    GXSetAlphaUpdate(0);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_360);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_378);
}






/* Current alpha interpolation value. */
static inline float AlphaScale8314(void) { return lbl_1_bss_A8.getValue(); }

void fn_1_8314(SpriteParameters *sprite)
{
    if (sprite->scaleY <= lbl_1_rodata_3AC) {
        return;
    }

    HuSprTexLoad(sprite->animation, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);

    GXColor color = lbl_1_rodata_3A8;
    color.a = (unsigned char)((double)AlphaScale8314() * (double)(lbl_1_rodata_3B0 * sprite->scaleY));
    GXSetTevColor(GX_TEVREG0, color);

    float halfWidth = sprite->halfWidth * sprite->scaleX;
    float halfHeight = sprite->halfHeight * sprite->scaleX;
    float left = sprite->x - halfWidth;
    float right = sprite->x + halfWidth;
    float bottom = sprite->y - halfHeight;
    float top = sprite->y + halfHeight;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXWGFifo.f32 = left;
    GXWGFifo.f32 = bottom;
    GXWGFifo.f32 = lbl_1_rodata_3AC;
    GXWGFifo.f32 = lbl_1_rodata_3AC;
    GXWGFifo.f32 = right;
    GXWGFifo.f32 = bottom;
    GXWGFifo.f32 = lbl_1_rodata_3B4;
    GXWGFifo.f32 = lbl_1_rodata_3AC;
    GXWGFifo.f32 = right;
    GXWGFifo.f32 = top;
    GXWGFifo.f32 = lbl_1_rodata_3B4;
    GXWGFifo.f32 = lbl_1_rodata_3B4;
    GXWGFifo.f32 = left;
    GXWGFifo.f32 = top;
    GXWGFifo.f32 = lbl_1_rodata_3AC;
    GXWGFifo.f32 = lbl_1_rodata_3B4;
}

ParameterInterpolation lbl_1_bss_A8(lbl_1_rodata_3AC, 0);
