#include "REL/framework/world_units.h"
#include "REL/framework/frame_time.h"
#include <cmath>
#include "dolphin/types.h"
extern "C" {
#include "dolphin/gx.h"
#include "dolphin/mtx.h"

}
#include "REL/m681DLL/texture_node.h"
#include "REL/m681DLL/game_constants.h"

extern "C" signed char lbl_1_data_0 = -1;

// Allocates the small depth texture and initializes the floor-circle renderer.
FloorCircle::FloorCircle(const Vec *initialPosition, f32 initialScale)
    : RenderObject(1), position(*initialPosition), scale(initialScale), vertexCount(0x20),
      textureBuffer(GXGetTexBufferSize(4, 4, GX_TF_Z24X8, GX_FALSE, GX_FALSE), lbl_1_data_0)
{
    this->setLayer(2);
    GXInitTexObj(&textureObject, &textureBuffer[0], 4, 4, GX_TF_Z24X8,
                 GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(&textureObject, GX_NEAR, GX_NEAR,
                    0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
}

extern "C" {

extern const GXVtxDescList lbl_1_rodata_14[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

extern const GXVtxAttrFmtList lbl_1_rodata_2C[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, GX_POS_XY, GX_U8, 0},
};
}

// Configures a texture-combiner stage with the supplied color and alpha inputs.
static inline void configureTevStage(GXTevStageID stage,
                                       GXTexCoordID coordinate,
                                       GXTexMapID map,
                                       GXChannelID channel,
                                       GXTevColorArg colorA,
                                       GXTevColorArg colorB,
                                       GXTevColorArg colorC,
                                       GXTevColorArg colorD,
                                       GXTevAlphaArg alphaD)
{
    GXSetTevOrder(stage, coordinate, map, channel);
    GXSetTevColorIn(stage, colorA, colorB, colorC, colorD);
    GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, alphaD);
    GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
}

// Releases the circle texture buffer and render-object bases.
FloorCircle::~FloorCircle()
{
}

// Performs no update; the circle is drawn by its model hook.
void FloorCircle::update()
{
}

// Builds the circle transform and draws its two components.
void FloorCircle::draw(Mtx *matrix)
{
    Mtx transform;
    f32 scale;
    f32 translationX;
    f32 translationY;
    f32 translationZ;

    if (!(this->scale <= 0.0f)) {
        scale = this->scale;
        PSMTXScale(transform, scale, scale, scale);
        translationZ = this->position.z;
        translationY = this->position.y;
        translationX = this->position.x;
        PSMTXTransApply(transform, transform, translationX, translationY,
                        translationZ);
        PSMTXConcat(*matrix, transform, transform);
        GXLoadPosMtxImm(transform, GX_PNMTX0);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetCullMode(GX_CULL_BACK);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetZCompLoc(GX_FALSE);
        fn_1_4E8();
        fn_1_944();
    }
}

// Draws the textured circular fan.
void FloorCircle::fn_1_4E8()
{
    s32 vertexCount;
    s32 vertex;
    f32 angle;
    f32 sine;
    f32 cosine;

    GXLoadTexObj(&textureObject, GX_TEXMAP0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
    GXSetNumTevStages(2);
    configureTevStage(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                         GX_COLOR0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_C0, GX_CA_KONST);
    configureTevStage(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0,
                         GX_COLOR_NULL, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_CPREV, GX_CA_APREV);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_14);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_2C);
    GXColor color = {0, 0, 0, 0};
    GXSetTevColor(GX_TEVREG0, color);

    vertexCount = this->vertexCount;
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, (u16)(vertexCount + 2));
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.5f;
    GXWGFifo.f32 = 0.5f;

    for (vertex = 0; vertex < vertexCount; ++vertex) {
        angle = (f32)(((M_PI * 360.0 / 180.0) * (f64)vertex) / (f64)vertexCount);
        sine = (f32)std::sin((f64)angle);
        cosine = (f32)std::cos((f64)angle);
        GXWGFifo.f32 = cosine;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = sine;
        GXWGFifo.f32 = (f32)(0.5 * (f64)(1.0f + cosine));
        GXWGFifo.f32 = (f32)(0.5 * (f64)(1.0f + sine));
    }

    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.5f;
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
}

extern const GXVtxDescList lbl_1_rodata_88[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

extern const GXVtxAttrFmtList lbl_1_rodata_A0[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0},
    {GX_VA_NULL, GX_POS_XY, GX_U8, 0},
};

// Draws the untextured outer ring.
void FloorCircle::fn_1_944()
{
    s32 segmentCount;
    s32 segment;
    f32 angle;
    f64 cosine;
    f64 sine;

    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    configureTevStage(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0,
        GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC, GX_CA_RASA);
    GXSetZMode(1, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_88);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_A0);
    segmentCount = vertexCount;
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)((segmentCount + 1) * 2));

    for (segment = 0; segment <= segmentCount; ++segment) {
        angle = (f32)(((M_PI * 360.0 / 180.0) * (f64)segment) / (f64)segmentCount);
        cosine = (f32)std::cos((f64)angle);
        sine = (f32)std::sin((f64)angle);
        GXWGFifo.f32 = cosine;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = sine;
        GXWGFifo.u16 = 0x105A;
        GXWGFifo.f32 = (f32)(1.3 * (f64)cosine);
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = (f32)((1.3 * (f64)sine) - 0.1);
        GXWGFifo.u16 = 0x2040;
    }
}
