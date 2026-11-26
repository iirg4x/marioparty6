#include "REL/m634DLL/TextureController.h"
#include "game/sprput.h"
#include "REL/m634DLL/render_owners.h"
#include "REL/m634DLL/scene.h"
#include "REL/framework/game_object2.h"
#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

extern "C" {
#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/animdata.h"

#include "game/sprite.h"
#include "math.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
}
#include "REL/m634DLL/model_hook.h"
#include "REL/framework/model.h"
#include "REL/m634DLL/model_action.h"

extern "C" {

struct TextureController;

}
#include "REL/m634DLL/mode_render_object.h"

extern "C" {
float lbl_1_data_478 = 980.665f;
}

// Creates the framebuffer texture and configures its sampling.
TextureController::TextureController()
    : ModeRenderObject(0x60)
{
    setCamera(1);
    setLayer(2);
    GXInitTexObj(&texture, frameBuffer, 0x280, 0x1E0, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
}

// Resets the indirect texture and coordinate state.
TextureController::~TextureController()
{
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, 0, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, 0, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, 0, 0, 0);
}

// Does nothing.
void TextureController::update() { }

// Copies the camera image into the framebuffer texture.
void TextureController::draw(Mtx *modelView)
{
    Mtx textureMatrix;
    Mtx lightPerspective;
    HU3D_CAMERA *camera;

    GXLoadPosMtxImm(*modelView, GX_PNMTX0);
    camera = Hu3DCamera;
    GXSetNumTexGens(1);
    C_MTXLightPerspective(lightPerspective, camera->fov, camera->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(lightPerspective, *modelView, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
    GXCopyTex(frameBuffer, GX_TRUE);
}

// Assigns camera 1 and its drawing layer.
Camera1Renderer::Camera1Renderer()
    : ModeRenderObject(0x60)
{
    setCamera(1);
    setLayer(3);
}

// Does nothing.
void Camera1Renderer::update() { }

// Runs the camera drawing operation.
void Camera1Renderer::draw(Mtx *)
{
    fn_8002BF48();
}

// Accepts scene mode events without changing state.
void Camera1Renderer::mode(int mode)
{
    switch (mode) {
    case 0:
    case 3:
        break;
    }
}

// Assigns camera 2 and its drawing layer.
Camera2Renderer::Camera2Renderer()
    : ModeRenderObject(0x60)
{
    setCamera(2);
    setLayer(1);
}

// Does nothing.
void Camera2Renderer::update() { }

const f32 lbl_1_rodata_184[6] = { -212.0f, 150.0f, -526.0f, -249.0f, 6.0f, -498.0f };

/* Begins the quad that displays the captured framebuffer texture. */
static inline void texturedQuadBegin(void)
{
    setTexturedQuadFormat();
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
}

// Draws the captured camera image as a textured quad during the second camera pass.
void Camera2Renderer::draw(Mtx *modelView)
{
    TextureController *controller;
    GXTexObj texture;
    GXTexObj *textureData;

    GXLoadPosMtxImm(*modelView, GX_PNMTX0);
    controller = &TextureController::getChecked();
    textureData = &controller->texture;
    texture = *textureData;
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetCullMode(GX_CULL_FRONT);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    texturedQuadBegin();
    GXPosition3f32(lbl_1_rodata_184[0], lbl_1_rodata_184[1], lbl_1_rodata_184[2]);
    GXTexCoord2f32(0.4f, 0.0f);
    GXPosition3f32(lbl_1_rodata_184[3], lbl_1_rodata_184[1], lbl_1_rodata_184[2]);
    GXTexCoord2f32(0.6f, 0.0f);
    GXPosition3f32(lbl_1_rodata_184[3], lbl_1_rodata_184[4], lbl_1_rodata_184[5]);
    GXTexCoord2f32(0.6f, 0.7f);
    GXPosition3f32(lbl_1_rodata_184[0], lbl_1_rodata_184[4], lbl_1_rodata_184[5]);
    GXTexCoord2f32(0.4f, 0.7f);
}

// Accepts scene mode events without changing state.
void Camera2Renderer::mode(int mode)
{
    switch (mode) {
    case 0:
    case 3:
        break;
    }
}

// Stores the sprite animation and configures its camera and layer.
SpriteRenderer::SpriteRenderer(ANIMDATA *parameter)
    : ModeRenderObject(0x60)
{
    texture = parameter;
    colorIndex = 0;
    setCamera(1);
    setLayer(0);
}

// Switches the sprite color when the scene piece becomes initialized.
void SpriteRenderer::update()
{
    if (colorIndex == 1)
        return;

    Scene *scene = &Scene::getChecked();
    if (scene->fn_1_ED80(1) != 0) {
        colorIndex = 1;
    }
}

const GXColor lbl_1_rodata_1A8[2] = { { 128, 128, 128, 255 }, { 255, 255, 255, 255 } };

const f32 lbl_1_rodata_1B0[6] = { -30.0f, -50.0f, -300.0f, 30.0f, 140.0f, -300.0f };

// Draws the sprite with its selected color.
void SpriteRenderer::draw(Mtx *)
{
    Mtx matrix;

    PSMTXIdentity(matrix);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    HuSprTexLoad(texture, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetTevColor(GX_TEVREG0, lbl_1_rodata_1A8[colorIndex]);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    texturedQuadBegin();
    GXPosition3f32(lbl_1_rodata_1B0[0], lbl_1_rodata_1B0[1], lbl_1_rodata_1B0[2]);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(lbl_1_rodata_1B0[3], lbl_1_rodata_1B0[1], lbl_1_rodata_1B0[2]);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(lbl_1_rodata_1B0[3], lbl_1_rodata_1B0[4], lbl_1_rodata_1B0[5]);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(lbl_1_rodata_1B0[0], lbl_1_rodata_1B0[4], lbl_1_rodata_1B0[5]);
    GXTexCoord2f32(1.0f, 0.0f);
}

// Forwards the material draw hook to the model action owner.
void fn_1_96AC(HU3D_DRAW_OBJ *owner, HSF_MATERIAL *data)
{
    fn_1_98DC(owner->model->hookData, owner, data);
}

// Forwards the model draw hook to the model action owner.
void fn_1_96EC(HU3D_MODEL *self, Mtx *value)
{
    fn_1_9964(self->hookData, value);
}

// Configures both model actions and their draw hooks.
ModelActionOwner::ModelActionOwner(u32 firstId, s32 secondId, s32 thirdId)
    : ParticleObjectBase(0x50)
    , firstAction(new OwnedModel(firstId))
    , secondAction(new OwnedModel(fn_1_96EC))
{
    secondAction->setLayer((s16)secondId);
    secondAction->setCamera(2);
    secondAction->setHookData(this);
    firstAction->setLayer((s16)thirdId);
    firstAction->setCamera(2);
    firstAction->setHookData(this);
    fn_80027884(*firstAction, fn_1_96AC);
}

// Does nothing.
void ModelActionOwner::update() { }

// Configures depth-only rendering for the model action.
void fn_1_98DC(void *, HU3D_DRAW_OBJ *, HSF_MATERIAL *)
{
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetColorUpdate(GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
}

// Runs the model action drawing operation.
void fn_1_9964(void *object, Mtx *matrix)
{
    fn_8002BF48();
}

/* Model owners release their sequence, audio-node array and tree-owned storage. */

// Releases the object and its member-owned resources.
ModelActionOwner::~ModelActionOwner() { }

extern "C" {
float lbl_1_bss_40 = (1.0 / 60.0) * ((1.0 / 60.0) * lbl_1_data_478);
}
