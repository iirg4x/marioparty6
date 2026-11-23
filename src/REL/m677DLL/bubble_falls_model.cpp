// Animates a Bubble falls model and draws its light and shadow.

#include "msm_se.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/game_object_man.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/interpolation.h"
#include "REL/framework/game_object2.h"
#include "REL/framework/p_info.h"
#include "REL/framework/sound.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/trail_burst.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/bubble_falls_model.h"

static bool createBubbleLight(BubbleFallsModel *owner);
static void updateBubbleLight(BubbleFallsModel *owner);

// Highest position at which the ground shadow is drawn.
static float shadowFadeHeight = cmToWorld(1000.0f);
// Point-light attenuation distance per unit of bubble scale.
static float bubbleLightRangePerScale = cmToWorld(20.0f);

// Counts the playback objects.
int countBubbleFallsModels()
{
    std::vector<GameObject *> objects;
    collectGameObjects(11, &objects);
    return objects.size();
}

// Initializes the model at the starting position and angle.
BubbleFallsModel::BubbleFallsModel(const OwnedModel &source, const Vec &startPosition,
                                     float angle, bool enabled)
    : RenderObject(11), state(&BubbleFallsModel::startFallLight), enabled(enabled), lightId(-1),
      model(source), position(startPosition), modelScale(1.0f), phase(0.0f), timer(0)
{
    setLayer(4);
    model.setLayer(4);
    model.setAttribute(loopMotion);
    model.resetAttribute(1);
    model.setRotationY(angle);
    vectorSet(&velocity, 0.0f, 0.0f, 0.0f);
}

// Releases the point light if it was created.
BubbleFallsModel::~BubbleFallsModel()
{
    if (lightId >= 0) {
        Hu3DGLightKill(lightId);
    }
}

// Advances the animation and removes the object after its last state.
void BubbleFallsModel::update()
{
    if (lightId >= 0) {
        moveFallingBubble(this);
        updateBubbleLight(this);
    }
    phase = phase + (2.0 * M_PI) / secondsToFrames(0.8);
    if (state) {
        (this->*state)();
    } else {
        delete this;
    }
}

// Updates the falling model's position and scale, with a floor at zero height.
void moveFallingBubble(BubbleFallsModel *owner)
{
    PSVECAdd(&owner->velocity, &owner->position, &owner->position);
    owner->velocity.y -= gravity;
    if (owner->position.y <= 0.0f) {
        owner->position.y = 0.0f;
        owner->velocity.y = 0.0f;
    }
    float factor = 40.0f;
    Vec position = owner->position;
    float scale = owner->modelScale.value;
    position.y = position.y + ((double) factor * scale);
    owner->model.setPosition(&position);
    float updatedScale = owner->modelScale.value;
    owner->model.setUniformScale(updatedScale);
}

// Creates the point light and returns whether allocation succeeded.
static bool createBubbleLight(BubbleFallsModel *owner)
{
    owner->lightId = Hu3DGLightCreate(owner->position.x, owner->position.y, owner->position.z, 0.0f,
                                      0.0f, 1.0f, 0xFFU, 0xA0U, 0U);
    if (owner->lightId < 0) {
        return false;
    }
    Hu3DGLightStaticSet(owner->lightId, 1);
    return true;
}

// Updates the light to face the camera.
static void updateBubbleLight(BubbleFallsModel *owner)
{
    if (owner->lightId >= 0) {
        Vec cameraTarget;
        Vec cameraPosition;
        Vec cameraUp;
        float intensity;
        fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
        PSVECSubtract(&cameraPosition, &cameraTarget, &cameraTarget);
        Vec &lightDirection = cameraTarget;
        Vec *direction = &lightDirection;
        PSVECNormalize(direction, direction);
        intensity = owner->modelScale.value;
        Hu3DGLightPointSet(owner->lightId,
            (bubbleLightRangePerScale * (double) intensity), 0.99f, GX_DA_STEEP);
        Hu3DGLightPosSetV(owner->lightId, &owner->position, &cameraTarget);
    }
}

// Waits for a point light before starting the fall.
void BubbleFallsModel::startFallLight()
{
    if (createBubbleLight(this)) {
        setState(&BubbleFallsModel::waitForLanding);
    }
}

// Starts the landing animation at ground level.
void BubbleFallsModel::waitForLanding()
{
    int duration;
    if (!(position.y > 0.0f)) {
        PlayerWork::getChecked().rumble(0, (int) secondsToFrames(0.15));
        modelScale.set(0.6f, 1.0f, (int) secondsToFrames(0.3));
        duration = secondsToFrames(1L);
        timer = duration;
        burstBubbleParticles(this, 64);
        soundPlay(MSM_SE_MG677_BUBBLE_EXPANSION_BURST);
        setState(&BubbleFallsModel::waitBeforeShrinking);
    }
}

// Advances the landing animation and waits for its next phase.
void BubbleFallsModel::waitBeforeShrinking()
{
    modelScale.update();
    if (modelScale.isFinished()) {
        unsigned char expired = 0;
        if (timer > 0 && --timer == 0)
            expired = 1;
        if (expired != 0) {
            if (enabled != 0) {
                PlayerWork::getChecked().rumble(2, (int) secondsToFrames(0.8));
            }
            modelScale.set(0.0f, (int) secondsToFrames(0.8));
            setState(&BubbleFallsModel::shrinkBubble);
        }
    }
}

// Darkens the player's materials and spawns effects above the player's position.
void darkenPlayerAndSpawnEffects(double progress)
{
    PlayerWork &character = PlayerWork::getChecked();
    character.fn_1_1CEFC((float) interpolateValue(0.0, 0.7, progress));
    Vec position = character.getPosition();
    position.y = (float) ((double) position.y + (0.5 * progress) * character.height());
    new TrailBurst(&position, 400.0f, 1.0f, 10);
    if (progress < 1.0) {
        ParticleAnimations &effects = ParticleAnimations::getChecked();
        effects.createGrayImpactParticles(&position, 8, 0.3f, 3.0f, 1);
    }
}

// Animates the model's scale and completes the sequence when it finishes.
void BubbleFallsModel::shrinkBubble()
{
    modelScale.update();
    if (enabled != 0) {
        float currentScale = modelScale.value;
        float progress = (float) (1.0 - (double) currentScale);
        darkenPlayerAndSpawnEffects(progress);
    }
    if (modelScale.isFinished()) {
        burstBubbleParticles(this, 32);
        soundPlay(MSM_SE_MG677_BUBBLE_SHRINK_BURST);
        setState(0);
    }
}

// Spawns a particle burst at the playback model.
void burstBubbleParticles(BubbleFallsModel *owner, int particleCount)
{
    new TrailBurst(&owner->position, 200.0f, 2.0f, particleCount);
}

// Draws the ground shadow within its height and scale limits.
void BubbleFallsModel::draw(Mtx *matrix)
{
    float current = modelScale.value;
    if (current <= 0.0 || position.y >= shadowFadeHeight) {
        return;
    }
    configureShadowDraw(*matrix);
    drawShadowDisc();
}

// Initial color before the ground shadow alpha is set.
static const GXColor bubbleShadowColor = { 0, 0, 0, 0 };

// Sets the shadow transform and configures GX to draw it.
void BubbleFallsModel::configureShadowDraw(Mtx view)
{
    Mtx transform;
    float scale = this->modelScale.value;
    float pulse = (float) ((double) scale * (60.0 + 5.0 * std::sin((double) this->phase)));
    PSMTXScale(transform, pulse, 1.0f, pulse);
    PSMTXTransApply(transform, transform, this->position.x, 0.5f, this->position.z);
    PSMTXConcat(view, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXSetNumTexGens(0);
    float clamped;
    float fade = this->position.y / shadowFadeHeight;
    if (fade < 0.0f)
        clamped = 0.0f;
    else if (fade > 1.0f)
        clamped = 1.0f;
    else
        clamped = fade;
    const double amount = clamped;
    const int alpha = (u8) interpolateValue(255, 0, amount);
    GXColor color = bubbleShadowColor;
    color.a = alpha;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
}

// GX vertex descriptors for the ground shadow.
static const GXVtxDescList bubbleShadowVertexDescriptors[] = { { GX_VA_POS, GX_INDEX8 },
                                                  { GX_VA_CLR0, GX_INDEX8 },
                                                  { GX_VA_NULL, GX_NONE } };

// GX vertex formats for the ground shadow.
static const GXVtxAttrFmtList bubbleShadowVertexFormats[] = { { GX_VA_POS, GX_POS_XYZ, GX_S8, 6 },
                                                     { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0 },
                                                     { GX_VA_NULL, GX_POS_XY, GX_U8, 0 } };

// Vertex positions for the ground shadow.
static const s8 bubbleShadowVertices[][3] ATTRIBUTE_ALIGN(32) = {
    { 0, 0, 0 },    { 0, 0, 64 },   { 31, 0, 55 }, { 55, 0, 32 },   { 64, 0, 0 },
    { 55, 0, -31 }, { 31, 0, -55 }, { 0, 0, -64 }, { -31, 0, -55 }, { -55, 0, -32 },
    { -64, 0, 0 },  { -55, 0, 32 }, { -32, 0, 55 }
};
// Vertex colors for the ground shadow.
static const u16 bubbleShadowVertexColors[] ATTRIBUTE_ALIGN(32) = { 0xFC0F, 0xF000 };
// Indexed triangle-fan display list for the ground shadow.
static const u8 bubbleShadowDisplayList[] ATTRIBUTE_ALIGN(32) = {
    0xA0, 0x00, 0x0E, 0x00, 0x00, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01, 0x04, 0x01, 0x05, 0x01, 0x06,
    0x01, 0x07, 0x01, 0x08, 0x01, 0x09, 0x01, 0x0A, 0x01, 0x0B, 0x01, 0x0C, 0x01, 0x01, 0x01, 0x00
};

// Draws the shadow's indexed vertices.
void BubbleFallsModel::drawShadowDisc()
{
    GXClearVtxDesc();
    GXSetVtxDescv(bubbleShadowVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, bubbleShadowVertexFormats);
    GXSetArray(GX_VA_POS, (void *) bubbleShadowVertices, 3);
    GXSetArray(GX_VA_CLR0, (void *) bubbleShadowVertexColors, 2);
    GXCallDisplayList(bubbleShadowDisplayList, 0x20);
}
