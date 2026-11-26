// Captures the framebuffer in an RGB565 texture and draws the transition fan using a separate 4x4 Z24X8 depth texture.

#include "msm_se.h"
#include <math.h>
#include "datadir_enum.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include <vector>
#include <memory>
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/screen_capture_projection.h"

extern "C" {
#include "dolphin/gx.h"
#include "game/data.h"
#include "game/hu3d.h"
}

// Owns the storage for either the framebuffer texture or the depth texture.
struct CaptureTextureTarget : OwnedModel {
    // Creates a model that invokes the capture hook.
    explicit CaptureTextureTarget(HU3D_MODEL_HOOK hook) : OwnedModel(hook)
    {
    }
    // Fills every byte of the texture storage and configures the texture object.
    void configure(u16 width, u16 height, GXTexFmt format, u8 fillByte)
    {
        storage.assign(GXGetTexBufferSize(width, height, format, GX_FALSE, 0), fillByte);
        GXInitTexObj(&texture, &storage[0], width, height, format, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f,
            GX_FALSE, GX_FALSE, GX_ANISO_1);
    }
    std::vector<unsigned char> storage;
    GXTexObj texture;
};

static void captureFramebufferAndDrawDepth(HU3D_MODEL *owner, Mtx *effect);
static void drawFramebufferProjection(HU3D_MODEL *owner, Mtx *effect);

// Creates the screen effect's capture targets and projection transition.
ScreenCaptureProjection::ScreenCaptureProjection(Vec *position, f32 targetProjection, f32 duration)
    : GameObject(10), effectPosition(*position), projection(0.0f),
      targetSmall(new CaptureTextureTarget(captureFramebufferAndDrawDepth)),
      targetLarge(new CaptureTextureTarget(drawFramebufferProjection)),
      smallModel(DATANUM(_m677 << 16, 0x2E)),
      largeModel(DATANUM(_m677 << 16, 0x2F))
{
    f32 perspectiveScratch;
    projection.set(0.0f, targetProjection, (s32) secondsToFrames(duration));
    soundPlay(MSM_SE_MG677_PROJECTION_TRANSITION);
    targetSmall->setHookData(this);
    targetSmall->setLayer(1);
    targetSmall->configure(4, 4, GX_TF_Z24X8, 0xFF);
    targetLarge->setHookData(this);
    targetLarge->setLayer(2);
    targetLarge->configure(640, 480, GX_TF_RGB565, 0);
    fn_80027D8C(1, &perspective, &perspectiveScratch, &perspectiveScratch);
    smallModel.setLayer(1);
    smallModel.setAttribute(loopMotion);
    smallModel.setPosition(position);
    largeModel.setLayer(5);
    largeModel.setAttribute(loopMotion);
    largeModel.setPosition(position);
}

// Releases the models and capture textures.
ScreenCaptureProjection::~ScreenCaptureProjection()
{
}

// Updates the scale of both capture models from the projection transition.
void ScreenCaptureProjection::update()
{
    projection.update();
    float scaleInput = projection.value;
    float scale = (float) (scaleInput / 100.0);
    smallModel.setUniformScale(scale);
    largeModel.setUniformScale(scale);
}

// Sets the next projection scale, bounded below by zero.
void ScreenCaptureProjection::setProjectionScaleTarget(f32 value, f32 seconds)
{
    f32 target = value > 0.0f ? value : 0.0f;
    projection.set(target, (s32) secondsToFrames(seconds));
    if (value <= 0.0f) {
        soundPlay(MSM_SE_MG677_PROJECTION_TRANSITION);
    }
}

// Captures and draws the first screen pass while the projection is visible.
static void captureFramebufferAndDrawDepth(HU3D_MODEL *owner, Mtx *effect)
{
    Mtx output;
    ScreenCaptureProjection *screen = static_cast<ScreenCaptureProjection *>(owner->hookData);
    const float scale = screen->projection.value;
    if (!(scale <= 0.0)) {
        screen->captureFramebuffer();
        screen->loadProjectionModelView(*effect, output);
        screen->drawDepthTextureFan();
    }
}

// Draws the second screen pass while the projection is visible.
static void drawFramebufferProjection(HU3D_MODEL *owner, Mtx *effect)
{
    Mtx output;
    ScreenCaptureProjection *screen = static_cast<ScreenCaptureProjection *>(owner->hookData);
    const float scale = screen->projection.value;
    if (!(scale <= 0.0)) {
        screen->loadProjectionModelView(*effect, output);
        screen->drawFramebufferTextureRing(output);
    }
}

// Copies the framebuffer into the large effect texture.
void ScreenCaptureProjection::captureFramebuffer()
{
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
    GXCopyTex(&targetLarge->storage[0], GX_FALSE);
    GXPixModeSync();
}

// Builds the modelview matrix for the current projection scale.
void ScreenCaptureProjection::loadProjectionModelView(Mtx source, Mtx output)
{
    float x;
    float y;
    float z;
    const float scale = projection.value;
    const float uniformScale = scale;
    const float *scaleAddress = &uniformScale;
    const float *scaleValue = scaleAddress;
    PSMTXScale(output, *scaleValue, *scaleValue, *scaleValue);
    z = effectPosition.z;
    y = effectPosition.y;
    x = effectPosition.x;
    PSMTXTransApply(output, output, x, y, z);
    PSMTXConcat(source, output, output);
    GXLoadPosMtxImm(output, GX_PNMTX0);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
}

// Vertex attributes used to draw the captured view.
static const GXVtxDescList texturedVertexDescriptors[] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_TEX0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE },
};

// Vertex formats used to draw the captured view.
static const GXVtxAttrFmtList texturedVertexFormats[] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
    { GX_VA_NULL, (GXCompCnt) 0, (GXCompType) 0, 0 },
};

// Draws the fan with depth supplied by the 4x4 Z24X8 texture filled with 0xFF.
void ScreenCaptureProjection::drawDepthTextureFan()
{
    f32 angle;
    f32 sine;
    f32 cosine;
    GXSetNumChans(0);
    GXLoadTexObj(&targetSmall->texture, GX_TEXMAP0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0U);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFFU, GX_AOP_OR, GX_ALWAYS, 0xFFU);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(texturedVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, texturedVertexFormats);
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 66);
    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXTexCoord2f32(0.5f, 0.5f);
    for (s32 vertex = 0; vertex < 64; ++vertex) {
        angle = (f32) ((M_PI * 360.0 / 180.0) * vertex / 64.0);
        sine = (f32)::sin(angle);
        cosine = (f32)::cos(angle);
        GXPosition3f32(cosine, 0.0f, sine);
        GXTexCoord2f32((f32) (0.5 * (1.0f + cosine)), (f32) (0.5 * (1.0f + sine)));
    }
    GXPosition3f32(1.0f, 0.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.5f);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0U);
}

static const GXVtxDescList coloredVertexDescriptors[] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_CLR0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE },
};

static const GXVtxAttrFmtList coloredVertexFormats[] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0 },
    { GX_VA_NULL, GX_POS_XY, GX_U8, 0 },
};

// Draws the large capture texture with vertex colors.
void ScreenCaptureProjection::drawFramebufferTextureRing(Mtx view)
{
    f32 angle;
    s32 vertex;
    f64 sine;
    f64 cosine;
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXLoadTexObj(&targetLarge->texture, GX_TEXMAP0);
    {
        Mtx textureProjection;
        C_MTXLightPerspective(textureProjection, perspective, 1.2f, 0.5f, -0.5f, 0.5f, 0.5f);
        PSMTXConcat(textureProjection, view, textureProjection);
        GXLoadTexMtxImm(textureProjection, GX_TEXMTX0, GX_MTX3x4);
    }
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(coloredVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, coloredVertexFormats);
    const u16 outerColor = 0x000F;
    const u16 innerColor = 0x0000;
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 130);
    for (vertex = 0; vertex <= 64; ++vertex) {
        angle = (f32) (((M_PI * 360.0 / 180.0) * vertex) / 64.0);
        sine = (f32) sin((double) angle);
        cosine = (f32) cos((double) angle);
        GXPosition3f32(sine, 0.0f, cosine);
        GXColor1x16(outerColor);
        GXPosition3f32((f32) (0.8 * (double) sine), 0.0f, (f32) (0.8 * (double) cosine));
        GXColor1x16(innerColor);
    }
}
