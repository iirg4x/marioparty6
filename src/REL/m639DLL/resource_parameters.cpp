#include "REL/m639DLL/lighting_types.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// The shadow fit, the sprite quads and the lighthouse lights that follow the camera.
#include "REL/m639DLL/transition.h"
#include <vector>
#include <cmath>
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/scene_shadow.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m639DLL/resource_parameters.h"
#include "REL/m639DLL/sphere.h"

extern "C" {
#include "dolphin/gx.h"
#include "REL/m639DLL.h"
}
#include "humath.h"

void collectGameObjects(int category, std::vector<GameObject *> *list);

#include "REL/m639DLL/lighthouse_light.h"

extern "C" {
extern const GXVtxDescList lbl_1_rodata_430[];
extern const GXVtxAttrFmtList lbl_1_rodata_448[];
extern float lbl_1_bss_C8;
extern char *lbl_1_data_508[4];
extern AnimationResourcePair lbl_1_data_5B8[5];
void __msl_assertion_failed(const char *, const char *, int);
int HuSprTexLoad(void *, short, int, int, int, int);
void fn_1_DECC(std::vector<Sphere> *positions);
void fn_1_C28C(Vec *position);
void fn_1_C858();
void fn_1_CD58();
void fn_1_D69C(LighthouseLight *owner);


void fn_1_111DC(void *object);
}

extern "C" const double lbl_1_rodata_420[] = {0.5};
/* Initializes resource parameters from the resource's two signed dimensions. */
ResourceParameters::ResourceParameters(ParameterResource *resource, unsigned int mode)
{
    halfWidth = (float)((double)resource->bmp->sizeX * lbl_1_rodata_420[0]);
    halfHeight = (float)((double)resource->bmp->sizeY * lbl_1_rodata_420[0]);
    textureIndex = 0;
    firstTexture = resource;
    secondTexture = 0;
    blendMode = mode;
}

ResourceParameters::ResourceParameters(ParameterResource *resource, ParameterResource *value, unsigned int mode)
{
    halfWidth = (float)((double)resource->bmp->sizeX * lbl_1_rodata_420[0]);
    halfHeight = (float)((double)resource->bmp->sizeY * lbl_1_rodata_420[0]);
    textureIndex = 0;
    firstTexture = resource;
    secondTexture = value;
    blendMode = mode;
}

extern "C" const GXVtxDescList lbl_1_rodata_430[] = {
    {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}
};
extern "C" const GXVtxAttrFmtList lbl_1_rodata_448[] = {
    {GX_VA_POS, GX_POS_XY, GX_F32, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, GX_POS_XY, GX_U8, 0}
};

// Configures the vertex and texture pipeline for the material animation draw.
void fn_1_C858()
{
    Mtx scale;
    PSMTXScale(scale, 0.9f, 1.0f, 1.0f);
    GXLoadPosMtxImm(scale, GX_PNMTX0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(1);
    GXSetAlphaUpdate(0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_430);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_448);
}

/* Draws the textured sprite quad with the owner's current opacity and blend state. */
void ResourceParameters::draw()
{
    bool hasSecondTexture;
    GXBlendFactor destinationBlend;
    float width;
    float height;
    float top;
    float bottom;

    if (opacity <= 0.0f || textureIndex < 0 ||
        textureIndex >= firstTexture->patNum) {
        return;
    }

    hasSecondTexture = secondTexture != 0;
    HuSprTexLoad(firstTexture, (short)textureIndex, 0, 0, 0, 1);
    if (hasSecondTexture) {
        HuSprTexLoad(secondTexture, 0, 1, 0, 0, 1);
    }

    GXColor color = { 0, 0, 0, 0 };
    color.a = (u8)(255.0f * opacity * lbl_1_bss_C8);
    GXSetTevColor(GX_TEVREG0, color);

    destinationBlend = blendMode ? GX_BL_ONE : GX_BL_INVSRCALPHA;
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, destinationBlend,
                   GX_LO_NOOP);
    GXSetNumTevStages((u8)(hasSecondTexture ? 2 : 1));
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0,
                    GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    if (hasSecondTexture) {
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA,
                        GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_FALSE, GX_TEVPREV);
    }

    width = halfWidth;
    height = halfHeight;
    top = centerX - width;
    bottom = centerX + width;
    float upper = centerY - height;
    float lower = centerY + height;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(top, upper);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition2f32(bottom, upper);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition2f32(bottom, lower);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition2f32(top, lower);
    GXTexCoord2f32(0.0f, 1.0f);
}


extern "C" { float lbl_1_bss_C8; u8 lbl_1_bss_CC[4]; }
