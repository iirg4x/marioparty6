// Updates the scene models, billboards, and screen overlays.

#include "REL/framework/event_callback_registration.h"
#include "REL/m673DLL/scene_billboard.h"
#include "REL/m673DLL/scene_model.h"
#include "REL/m673DLL/fade_overlay.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/random.h"
#include "REL/m673DLL/scene_resources.h"
#include <memory>
#include <functional>
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/game_object.h"
#include "REL/m673DLL/callback_receiver.h"
#include "REL/m673DLL/screen_overlay.h"
#include "REL/framework/event_callback_adapter_inline.h"

// Binds the scene model to its position and animation events.
SceneModel::SceneModel(const OwnedModel *model, Vec *position, u8 active)
    : GameObject(5), unk10(*position), model(new OwnedModel(*model)), active(active),
      channel1Events(1)
{
    if (active != 0) {
        this->scaleLimit = 1500.0f;
        this->scale = 1500.0f;
    } else {
        this->scaleLimit = 1000.0f;
        this->scale = 1000.0f;
    }

    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneModel::fn_1_CE04), this), 5);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneModel::fn_1_CE48), this), 6);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneModel::fn_1_CE8C), this), 8);

    this->model->setLayer(3);
    this->model->resetAttribute(1);
    this->model->setPosition(position);
    this->model->setHookData(this);
    this->motionTime = 0.0f;
    this->setState(&SceneModel::fn_1_CEDC);
}

// Releases the scene model instance and its event handlers.
SceneModel::~SceneModel()
{
}

// Advances the scene model's state and sets its animation time.
void SceneModel::update()
{
    if (this->state) {
        (this->*this->state)();
    }
    this->model->setMotionTime(this->motionTime);
}

// Starts the scene model's shrinking animation.
void SceneModel::fn_1_CE04(const EventPacket &)
{
    setState(&SceneModel::fn_1_CF9C);
}

// Starts the scene model's growth animation.
void SceneModel::fn_1_CE48(const EventPacket &)
{
    setState(&SceneModel::fn_1_CEE0);
}

// Installs the ending animation callback when active is nonzero.
void SceneModel::fn_1_CE8C(const EventPacket &)
{
    if (active) {
        setState(&SceneModel::fn_1_D05C);
    }
}

// Leaves the scene model unchanged in its idle state.
void SceneModel::fn_1_CEDC()
{
}

// Advances ordinary growth and animation time only when active differs from 1.
void SceneModel::fn_1_CEE0()
{
    if (active != 1) {
        scale = scale + 1.0 / 60.0 * scaleLimit;
        if (scale > scaleLimit) {
            scale = scaleLimit;
            setState(&SceneModel::fn_1_CEDC);
        }
        motionTime = 59.0f * (1.0f - (scale / scaleLimit));
    }
}

// Shrinks the scene model to zero and updates its animation time.
void SceneModel::fn_1_CF9C()
{
    scale = scale - 2.0 / 60.0 * scaleLimit;
    if (scale < 0.0f) {
        scale = 0.0f;
        setState(&SceneModel::fn_1_CEDC);
    }
    motionTime = 59.0f * (1.0f - (scale / scaleLimit));
}

// Grows the scene model to its ending scale and updates its animation time.
void SceneModel::fn_1_D05C()
{
    scale += 1600.0 / 60.0;
    if (scale > 800.0f) {
        scale = 800.0f;
        setState(&SceneModel::fn_1_CEDC);
    }
    motionTime = 59.0f * (1.0f - (scale / 800.0f));
}

// Creates a billboard at the supplied position and registers its events.
SceneBillboard::SceneBillboard(const Vec *position, u8 active)
    : GameObject(3), position(*position), followEffect(0), active(active), channel1Events(1)
{
    SceneResources *resources;
    int delay;

    if (active) {
        this->scaleLimit = 1500.0f;
        this->scale = 1500.0f;
    } else {
        this->scaleLimit = 1000.0f;
        this->scale = 1000.0f;
    }
    delay = (s32)RandomUnsigned() % 120;

    resources = &SceneResources::getChecked();
    this->followEffect.reset(new FollowEffect(&this->position, this->scale,
                                              resources->animationAt(4)));
    this->followEffect->setAlpha(1.0f);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneBillboard::fn_1_D5EC), this), 5);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneBillboard::fn_1_D630), this), 6);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&SceneBillboard::fn_1_D69C), this), 8);
    this->setState(&SceneBillboard::fn_1_D714);
}

// Releases the billboard effect and its event handlers.
SceneBillboard::~SceneBillboard()
{
}

// Runs the billboard's current state callback.
void SceneBillboard::update()
{
    if (this->state) {
        (this->*this->state)();
    }
}

// Starts the billboard's fade-out state.
void SceneBillboard::fn_1_D5EC(const EventPacket &)
{
    setState(&SceneBillboard::fn_1_D7C8);
}

// Shows the billboard and starts its growth state.
void SceneBillboard::fn_1_D630(const EventPacket &)
{
    this->followEffect->setAlpha(1.0f);
    setState(&SceneBillboard::fn_1_D718);
}

// Shows an active billboard and starts its ending growth state.
void SceneBillboard::fn_1_D69C(const EventPacket &)
{
    if (this->active) {
        this->followEffect->setAlpha(1.0f);
        setState(&SceneBillboard::fn_1_D8B0);
    }
}

// Leaves the billboard unchanged in its idle state.
void SceneBillboard::fn_1_D714()
{
}

// Grows the billboard to its scale limit.
void SceneBillboard::fn_1_D718()
{
    FollowEffect *effect;
    f32 currentScale;

    scale = scale + 1.0 / 60.0 * scaleLimit;
    if (scale > scaleLimit) {
        scale = scaleLimit;
        setState(&SceneBillboard::fn_1_D714);
    }
    currentScale = scale;
    effect = followEffect.get();
    effect->scale = currentScale;
}

// Reduces the billboard's scale and hides it at the end of the fade.
void SceneBillboard::fn_1_D7C8()
{
    f32 currentScale;

    this->scale = this->scale - 2.0 / 60.0 * this->scaleLimit;
    if (this->scale < 0.0f) {
        FollowEffect *effect;
        this->scale = 0.0f;
        effect = this->followEffect.get();
        effect->setAlpha(0.0f);
        setState(&SceneBillboard::fn_1_D714);
    }
    currentScale = this->scale;
    FollowEffect *effect;
    effect = this->followEffect.get();
    effect->scale = currentScale;
}

// Grows the billboard toward its ending size.
void SceneBillboard::fn_1_D8B0()
{
    FollowEffect *effect;
    f32 currentScale;

    scale += 1600.0 / 60.0;
    if (scale > 800.0f) {
        scale = 800.0f;
        setState(&SceneBillboard::fn_1_D714);
    }
    currentScale = scale;
    effect = followEffect.get();
    effect->scale = currentScale;
}

// Creates the scene fade effect and registers its two event callbacks.
FadeOverlay::FadeOverlay()
    : GameObject(3), followEffect(0), channel1Events(1)
{
    Vec origin;
    SceneResources *resources;
    origin.x = 0.0f;
    origin.y = 0.0f;
    origin.z = 0.0f;
    resources = &SceneResources::getChecked();
    // The fade effect keeps the address of this local after the constructor returns.
    // The overlay has a separate position member for the scene position table.
    this->followEffect.reset(new FollowEffect(&origin, 1200.0f, resources->animationAt(3)));
    this->followEffect->setAlpha(1.0f);
    this->colorScale = 1.0f;
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&FadeOverlay::fn_1_DD3C), this), 5);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&FadeOverlay::fn_1_DDA8), this), 6);
    this->setState(&FadeOverlay::fn_1_DE14);
}

// Releases the fade effect and its event handler.
FadeOverlay::~FadeOverlay()
{
}

// Runs the overlay's current state callback.
void FadeOverlay::update()
{
    if (this->state) {
        (this->*this->state)();
    }
}

// Shows the overlay and starts its fade-out state.
void FadeOverlay::fn_1_DD3C(const EventPacket &)
{
    followEffect->setAlpha(1.0f);
    setState(&FadeOverlay::fn_1_DF58);
}

// Shows the overlay and starts its fade-in state.
void FadeOverlay::fn_1_DDA8(const EventPacket &)
{
    followEffect->setAlpha(1.0f);
    setState(&FadeOverlay::fn_1_DE18);
}

// Leaves the fade effect unchanged in its idle state.
void FadeOverlay::fn_1_DE14()
{
}

// Increases the overlay's color intensity toward full intensity.
void FadeOverlay::fn_1_DE18()
{
    GXColor color;

    this->colorScale += 1.0 / 60.0;
    if (this->colorScale > 1.0f) {
        this->colorScale = 1.0f;
        this->followEffect->setAlpha(1.0f);
        setState(&FadeOverlay::fn_1_DE14);
    }
    color.r = (u8)(255.0f * this->colorScale);
    color.g = color.b = color.r;
    color.a = 0xFF;
    this->followEffect->setColor(color);
}

// Reduces the overlay's color intensity and hides it at zero.
void FadeOverlay::fn_1_DF58()
{
    GXColor color;

    this->colorScale += -0.05;
    if (this->colorScale < 0.0f) {
        this->colorScale = 0.0f;
        this->followEffect->setAlpha(0.0f);
        setState(&FadeOverlay::fn_1_DE14);
    }
    color.r = (u8)(255.0f * this->colorScale);
    color.g = color.b = color.r;
    color.a = 0xFF;
    this->followEffect->setColor(color);
}

// Registers the receiver for scene events.
CallbackReceiver::CallbackReceiver()
    : MinigameSingleton<CallbackReceiver>(), RenderObject(4), unk96040(0), channel1Events(1)
{
    setLayer(5);
    GXInitTexObj(&texture, pixels, 640, 480,
                 GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    registerEventCallback(&channel1Events,
                          std::bind1st(std::mem_fun(&CallbackReceiver::fn_1_E4BC), this), 5);
    registerEventCallback(&channel1Events,
                          std::bind1st(std::mem_fun(&CallbackReceiver::fn_1_E4CC), this), 6);
}

// Releases the scene event handlers and render object.
CallbackReceiver::~CallbackReceiver()
{
}

// Leaves capture to the draw callback.
void CallbackReceiver::update()
{
}

// Configures projected texture coordinates and captures the scene texture.
void CallbackReceiver::draw(Mtx *modelView)
{
    Mtx textureMatrix;
    Mtx lightPerspective;
    HU3D_CAMERA *camera;

    GXLoadPosMtxImm(*modelView, GX_PNMTX0);
    camera = Hu3DCamera;
    GXSetNumTexGens(1);
    C_MTXLightPerspective(lightPerspective, camera->fov, camera->aspect, 0.5f, -0.5f, 0.5f,
                          0.5f);
    PSMTXConcat(lightPerspective, *modelView, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
    GXCopyTex(pixels, GX_TRUE);
}

// Sets the receiver flag in response to a scene event.
void CallbackReceiver::fn_1_E4BC(const EventPacket &)
{
    unk96040 = 1;
}

// Clears the receiver flag in response to a scene event.
void CallbackReceiver::fn_1_E4CC(const EventPacket &)
{
    unk96040 = 0;
}

// Creates a screen overlay using the supplied color.
ScreenOverlay::ScreenOverlay(GXColor value)
    : RenderObject(4)
{
    color = value;
    setLayer(5);
}

// Releases the screen-overlay render object.
ScreenOverlay::~ScreenOverlay()
{
}

// Keeps the overlay color between draws.
void ScreenOverlay::update()
{
}

// Draws a screen-sized rectangle with stored RGB and a separate TEV constant for alpha.
void ScreenOverlay::draw(Mtx *)
{
    Mtx identity;

    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    GXSetNumTexGens(0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    GXColor color = this->color;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-320.0f, -240.0f, -100.0f);
    GXPosition3f32(-320.0f, 240.0f, -100.0f);
    GXPosition3f32(320.0f, 240.0f, -100.0f);
    GXPosition3f32(320.0f, -240.0f, -100.0f);
}
