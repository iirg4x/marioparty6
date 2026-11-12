#include "REL/m655DLL/game_constants.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "math.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object2.h"
#include "datadir_enum.h"
extern "C" {
#include "game/data.h"
#include "game/hu3d.h"
#include "game/sprite.h"
}
#include "REL/framework/world_units.h"
#define numberof(a) (int)(sizeof(a) / sizeof((a)[0]))
extern "C" {
extern const GXColor iconTevColor;
extern const GXVtxDescList lbl_1_rodata_28C[3];
extern const GXVtxAttrFmtList spriteVertexFormats[];
#include "game/sprput.h"
}
#include "REL/m655DLL/icon_renderer.h"
extern "C" {
IconPosition icon_pos[3] = {
    {{cmToWorld(-250.0f), cmToWorld(550.0f), cmToWorld(-400.0f)}},
    {{cmToWorld(0.0f), cmToWorld(550.0f), cmToWorld(-400.0f)}},
    {{cmToWorld(250.0f), cmToWorld(550.0f), cmToWorld(-400.0f)}},
};
}
extern "C" {
extern const s32 lbl_1_rodata_238[14] = {
    DATANUM(0x720000, 0x0F), DATANUM(0x720000, 0x10), DATANUM(0x720000, 0x11),
    DATANUM(0x720000, 0x12), DATANUM(0x720000, 0x13), DATANUM(0x720000, 0x14),
    DATANUM(0x720000, 0x15), DATANUM(0x720000, 0x16), DATANUM(0x720000, 0x17),
    DATANUM(0x720000, 0x18), DATANUM(0x720000, 0x19), DATANUM(0x720000, 0x1A),
    DATANUM(0x720000, 0x1B), DATANUM(0x720000, 0x1C),
};
extern const s32 lbl_1_rodata_270[2] = {DATANUM(0x720000, 0x0D), DATANUM(0x720000, 0x0E)};
}
IconRenderer::IconRenderer(s32 primaryAnimation, s32 secondaryAnimation)
    : RenderObject(1), primaryAnimation(HuSprAnimRead(
                           fn_80008DD0(lbl_1_rodata_270[primaryAnimation], 0x10000000, 2))),
      secondaryAnimation(
          HuSprAnimRead(fn_80008DD0(lbl_1_rodata_238[secondaryAnimation], 0x10000000, 2))),
      idleAnimation(HuSprAnimRead(fn_80008DD0(DATANUM(0x720000, 0x1D), 0x10000000, 2))),
      initialVector(icon_pos[0]), transition(0.0f) {
    setLayer(5);
}

IconRenderer::~IconRenderer() {}
// Select one of the initial screen positions.
void IconRenderer::fn_1_59C4(int pos) {

/* Retain assertion source location.
 *
 */
    assert((pos >= 0) && (pos < numberof(icon_pos)));
    initialVector.components[0] = icon_pos[pos].components[0];
    initialVector.components[1] = icon_pos[pos].components[1];
    initialVector.components[2] = icon_pos[pos].components[2];
}

void IconRenderer::fn_1_5A88() {
    transition.set(1.0f, (int)secondsToFrames(0.2));
}

void IconRenderer::fn_1_5ADC() {
    transition.set(0.0f, (int)secondsToFrames(0.2));
}

void IconRenderer::update() {
    this->transition.update();
}

void fn_1_5B5C(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
               GXTevColorArg d) {
    GXSetTevColorIn(stage, a, b, c, d);
    GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
}

void fn_1_5BC8(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
               GXTevAlphaArg d) {
    GXSetTevAlphaIn(stage, a, b, c, d);
    GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
}

extern "C" {
extern const GXColor iconTevColor = {0, 0, 0, 0};
extern const GXVtxDescList lbl_1_rodata_28C[3] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};
extern const GXVtxAttrFmtList spriteVertexFormats[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, (GXCompCnt)0, (GXCompType)0, 0},
};
}

void IconRenderer::draw(Mtx *projection) {
    Mtx world;
    Mtx textureMatrix;
    f32 textureScaleX;
    f32 textureScaleY;
    f32 textureOffsetX;
    f32 textureOffsetY;
    f32 left;
    f32 lower;
    f32 lastLeft;
    f32 lastLower;
    f32 initialOpacity;
    f32 opacity;

    initialOpacity = this->transition.value;
    if ((f64)initialOpacity <= 0.0) {
        return;
    }

    PSMTXTranspose(*projection, world);
    PSMTXTransApply(world, world, this->initialVector.components[0],
                    this->initialVector.components[1], this->initialVector.components[2]);
    PSMTXScale(textureMatrix, 1.7f, 1.7f, 1.7f);
    PSMTXConcat(world, textureMatrix, world);
    PSMTXConcat(*projection, world, world);
    GXLoadPosMtxImm(world, GX_PNMTX0);
    GXSetNumChans(0);
    HuSprTexLoad((ANIMDATA *)this->primaryAnimation, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad((ANIMDATA *)this->secondaryAnimation, 0, 1, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    HuSprTexLoad((ANIMDATA *)this->idleAnimation, 0, 2, GX_CLAMP, GX_CLAMP, GX_LINEAR);

    const f32 &firstWidth = (f32)this->primaryAnimation->bmp->sizeX;
    const f32 &firstHeight = (f32)this->primaryAnimation->bmp->sizeY;
    textureScaleX = firstWidth / (f32)this->secondaryAnimation->bmp->sizeX;
    textureScaleY = firstHeight / (f32)this->secondaryAnimation->bmp->sizeY;
    textureOffsetX = (f32)((1.0 - (f64)textureScaleX) / 2.0);
    textureOffsetY = (f32)((1.0 - (f64)textureScaleY) / 2.0);
    PSMTXScale(world, textureScaleX, textureScaleY, 1.0f);
    PSMTXTransApply(world, world, textureOffsetX, textureOffsetY, 0.0f);
    GXLoadTexMtxImm(world, 0x1E, GX_MTX2x4);
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    GXColor tevColor = iconTevColor;
    opacity = this->transition.value;
    tevColor.a = (u8)(255.0 * (f64)opacity);
    GXSetTevColor(GX_TEVREG0, tevColor);
    GXSetNumTevStages(3);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_TEXC, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_28C);
    GXSetVtxAttrFmtv(GX_VTXFMT0, spriteVertexFormats);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    left = -firstWidth;
    GXPosition3f32(left, 0.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    lower = -firstHeight;
    GXPosition3f32(0.0f, lower, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    lastLower = -firstHeight;
    lastLeft = -firstWidth;
    GXPosition3f32(lastLeft, lastLower, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
}
