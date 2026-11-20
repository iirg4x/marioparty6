#include "datadir_enum.h"
#include "REL/m611DLL/sprite_manager.h"
#include "REL/framework/random.h"
#include <cmath>
#include <new>
#include "REL/m611DLL/burst_models.h"
#include "REL/m611DLL/effect_node.h"
#include "REL/m611DLL/effect_model.h"
#include "REL/m611DLL/rotated_sprite_effect.h"
#include "REL/m611DLL/ground_sprite_effect.h"
#include "REL/m611DLL/motion_particle_owner.h"
#include "REL/m611DLL/particle_strip.h"
#include "REL/m611DLL/timed_flash.h"
#include "REL/m611DLL/particle_roots.h"
#include "REL/m611DLL/list_rotation.h"
#include "REL/m611DLL/motion_light_owner.h"

extern "C" {
#include "game/sprput.h"
#include "dolphin/os.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "humath.h"
#include "game/animdata.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
}

// Particle list cleanup and update routines.
void fn_1_4A74();
void fn_1_44CC();
void fn_1_4FE4();
void fn_1_60A4();
void fn_1_6908();
void fn_1_7238();
void fn_1_7D28();
void fn_1_5618();
void fn_1_459C();
void fn_1_4B44();
void fn_1_50B4();
void fn_1_5704();
void fn_1_6190();
void fn_1_69F4();
void fn_1_7324();
void fn_1_7E14();
void fn_1_9318();

using std::cos;
using std::sin;

// Animation resources shared by the particle effects.
const s32 lbl_1_rodata_228[4] = {
    DATANUM(0x460000, 25), DATANUM(0x460000, 26),
    DATANUM(0x460000, 27), DATANUM(0x460000, 28)
};

// Scene positions of the pulsing sprites.
const Vec lbl_1_rodata_238[] = {
    { 800.0f, 0.0f, 1600.0f },
    { -400.0f, 0.0f, 1600.0f },
    { -800.0f, 0.0f, 1600.0f },
    { -1200.0f, 0.0f, 1600.0f },
    { -1600.0f, 0.0f, 1600.0f },
    { -1600.0f, 0.0f, 800.0f },
    { -1600.0f, 0.0f, 400.0f },
    { -1600.0f, 0.0f, 0.0f },
    { -1600.0f, 0.0f, -400.0f },
    { -1600.0f, 0.0f, -800.0f },
    { -1600.0f, 0.0f, -1200.0f },
    { -1600.0f, 0.0f, -1600.0f },
    { -800.0f, 0.0f, -1600.0f },
    { -400.0f, 0.0f, -1600.0f },
    { 0.0f, 0.0f, -1600.0f },
    { 800.0f, 0.0f, -1600.0f },
    { 1200.0f, 0.0f, -1600.0f },
    { 1600.0f, 0.0f, -1600.0f },
    { 1600.0f, 0.0f, -1200.0f },
    { 1600.0f, 0.0f, -800.0f },
    { 1600.0f, 0.0f, -400.0f },
    { 1600.0f, 0.0f, 800.0f },
    { 1600.0f, 0.0f, -1200.0f },
};

// Loads the sprite animations and creates the scene particle owners.
SpriteManager::SpriteManager()
{
    s32 animationIndex;
    s32 effectIndex;

    for (animationIndex = 0;
         animationIndex < static_cast<s32>(sizeof(animations) / sizeof(animations[0]));
         ++animationIndex) {
        this->animations[animationIndex] = HuSprAnimDataRead(lbl_1_rodata_228[animationIndex]);
        HuSprAnimLock(this->animations[animationIndex]);
    }

    const s32 effectCount = sizeof(lbl_1_rodata_238) / sizeof(lbl_1_rodata_238[0]);
    for (effectIndex = 0; effectIndex < effectCount; ++effectIndex) {
        // Maximum scale of each pulsing sprite.
        new EffectModel(&lbl_1_rodata_238[effectIndex], 568.0f);
    }

    if (GWMgNightFGet() != 0) {
        new MotionParticleOwner();
    }
}

// Deletes the particle owners and releases their shared animations.
SpriteManager::~SpriteManager()
{
    fn_1_4A74();
    fn_1_44CC();
    fn_1_4FE4();
    fn_1_60A4();
    fn_1_6908();
    fn_1_7238();
    fn_1_7D28();
    fn_1_5618();
    for (s32 spriteIndex = 0;
         spriteIndex < static_cast<s32>(sizeof(animations) / sizeof(animations[0]));
         ++spriteIndex) {
        HuSprAnimKill(animations[spriteIndex]);
    }
}

// Returns the selected shared particle animation.
ANIMDATA *fn_1_4224(u32 entryIndex)
{
    return MinigameSingleton<SpriteManager>::getChecked().animations[entryIndex];
}

// Acknowledges a sprite manager request.
void SpriteManager::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
}

// Advances the effects managed by this scene object.
void SpriteManager::update()
{
    fn_1_4B44();
    fn_1_459C();
    fn_1_50B4();
    fn_1_6190();
    fn_1_69F4();
    fn_1_7324();
    fn_1_7E14();
    fn_1_5704();
    fn_1_9318();
}

// Active radial bursts.
ListRoot<RadialBurstModel> lbl_1_bss_AC;

// Creates and registers a radial particle burst.
RadialBurstModel::RadialBurstModel(Vec *position) : ListNode<RadialBurstModel>()
{
    ANIMDATA *particleAnimation;

    this->insert4(&lbl_1_bss_AC);
    particleAnimation = fn_1_4224(0);
    this->modelId = fn_8003216C(particleAnimation, 32);
    fn_80026790(this->modelId, position);
    fn_80026B8C(this->modelId, 0);
    fn_800324E0(this->modelId, 0.0f);
    fn_800326D0(this->modelId, fn_1_4650);
    fn_80032620(this->modelId, 0.0f);
    fn_8003269C(this->modelId, 0);
    this->unkC = 0;
}

// Releases the radial burst model.
RadialBurstModel::~RadialBurstModel()
{
    fn_800262D0(modelId);
}

// Deletes all radial particle bursts.
void fn_1_44CC()
{
    deleteEffects(lbl_1_bss_AC);
}

// Updates each radial particle burst.
void fn_1_459C()
{
    lbl_1_bss_AC.visit(fn_1_45CC);
}

// Deletes the model owner once its particle effect signals completion.
template <class Burst>
static inline void deleteFinishedBurst(Burst *burst)
{
    HU3D_PARTICLE *particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[burst->modelId].hookData);
    if (particle->dataCnt < 0) {
        delete burst;
    }
}

// Deletes a burst owner when its particle effect reports completion.
void fn_1_45CC(RadialBurstModel *burst)
{
    deleteFinishedBurst(burst);
}

// Initializes and advances the radial particle burst.
void fn_1_4650(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx)
{
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    HU3D_PARTICLE_DATA *initialParticle;
    HU3D_PARTICLE_DATA *entry;
    s32 alpha;
    s32 updatedCount;

    if (particle->count == 0) {
        for (initialParticle = particle->data; initialParticle != end; ++initialParticle) {
            f32 angle = static_cast<f32>(
                (M_PI * static_cast<f64>((360.0f * RandomFloat()) - 180.0f)) / 180.0);

            initialParticle->vel.x = cos(angle);
            initialParticle->vel.y = 0.0f;
            initialParticle->vel.z = sin(angle);
            initialParticle->pos.x = 30.0f * initialParticle->vel.x;
            initialParticle->pos.y = 50.0f;
            initialParticle->pos.z = 30.0f * initialParticle->vel.z;
            initialParticle->vel.x *= 5.0f;
            initialParticle->vel.z *= 5.0f;
            initialParticle->color.r = 192;
            initialParticle->color.g = 192;
            initialParticle->color.b = 255;
            initialParticle->color.a = 128;
            initialParticle->scale = 200.0f;
        }
    }

    updatedCount = 0;
    for (entry = particle->data; entry != end; ++entry) {
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        entry->vel.x *= 0.95;
        entry->vel.y += 0.05;
        entry->vel.z *= 0.95;

        alpha = entry->color.a;
        alpha -= 2;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            particle->dataCnt = -1;
        }
        entry->color.a = static_cast<u8>(alpha);
        ++updatedCount;
    }

    particle->emitCnt = updatedCount;
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Active single fading particles.
ListRoot<FadingParticleModel> lbl_1_bss_C0;

// Creates and registers a single fading particle.
// Nothing in this minigame calls this.
FadingParticleModel::FadingParticleModel(Vec *position) : ListNode<FadingParticleModel>()
{
    ANIMDATA *particleAnimation;

    this->insert4(&lbl_1_bss_C0);
    particleAnimation = fn_1_4224(0);
    this->modelId = fn_8003216C(particleAnimation, 1);
    fn_80026790(this->modelId, position);
    fn_80026B8C(this->modelId, 6);
    fn_800324E0(this->modelId, 100.0f);
    fn_800326D0(this->modelId, fn_1_4BF8);
    fn_80032620(this->modelId, 0.0f);
    fn_8003269C(this->modelId, 0);
    this->unkC = 0;
}

// Releases the model for a single fading particle.
FadingParticleModel::~FadingParticleModel()
{
    fn_800262D0(modelId);
}

// Deletes all single fading particles.
void fn_1_4A74()
{
    deleteEffects(lbl_1_bss_C0);
}

// Updates each single fading particle.
void fn_1_4B44()
{
    lbl_1_bss_C0.visit(fn_1_4B74);
}

// Deletes a particle owner when its effect reports completion.
void fn_1_4B74(FadingParticleModel *burst)
{
    deleteFinishedBurst(burst);
}

// Initializes and advances the falling cyan particle.
void fn_1_4BF8(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx)
{
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 updatedCount;

    if (particle->count == 0) {
        for (initializingEntry = particle->data; initializingEntry != end;
             ++initializingEntry) {
            initializingEntry->vel.x = 2.5f *
                                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.y = 2.5f *
                                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.z = 2.5f *
                                RandomFloat(-1.0f, 1.0f);
            initializingEntry->pos.x = initializingEntry->pos.y =
                initializingEntry->pos.z =
                0.0f;
            initializingEntry->color.r = 128;
            initializingEntry->color.g = 255;
            initializingEntry->color.b = 255;
            initializingEntry->color.a = 128;
        }
    }

    updatedCount = 0;
    for (entry = particle->data; entry != end; ++entry) {
        entry->vel.y = static_cast<f32>(entry->vel.y - (980.665 / 60.0 / 60.0));
        PSVECScale(&entry->vel, &entry->vel, 0.8f);
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        entry->color.r = 255;
        green = entry->color.g;
        green += 32;
        if (green > 255) {
            green = 255;
        }
        entry->color.g = static_cast<u8>(green);
        blue = entry->color.b;
        blue += 32;
        if (blue > 255) {
            blue = 255;
        }
        entry->color.b = static_cast<u8>(blue);
        alpha = entry->color.a;
        alpha -= 8;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            particle->dataCnt = -1;
        }
        entry->color.a = static_cast<u8>(alpha);
        ++updatedCount;
    }

    particle->emitCnt = updatedCount;
}

// Active rotating bursts.
ListRoot<RotatingBurstModel> lbl_1_bss_D4;

// Creates and registers a rotating particle burst.
RotatingBurstModel::RotatingBurstModel(Vec *position) : ListNode<RotatingBurstModel>()
{
    ANIMDATA *particleAnimation;

    this->insert4(&lbl_1_bss_D4);
    particleAnimation = fn_1_4224(2);
    this->modelId = fn_8003216C(particleAnimation, 1);
    fn_80026790(this->modelId, position);
    fn_80026B8C(this->modelId, 6);
    fn_800324E0(this->modelId, 30.0f + (50.0f * RandomFloat()));
    fn_800326D0(this->modelId, fn_1_5168);
    fn_80032620(this->modelId, 0.0f);
    fn_8003269C(this->modelId, 0);
    this->unkC = 0;
}

// Releases the rotating burst model.
RotatingBurstModel::~RotatingBurstModel()
{
    fn_800262D0(modelId);
}

// Deletes all rotating particle bursts.
void fn_1_4FE4()
{
    deleteEffects(lbl_1_bss_D4);
}

// Updates each rotating particle burst.
void fn_1_50B4()
{
    lbl_1_bss_D4.visit(fn_1_50E4);
}

// Deletes a burst owner when its effect reports completion.
void fn_1_50E4(RotatingBurstModel *burst)
{
    deleteFinishedBurst(burst);
}

// Offset applied to each rotating burst particle.
const Vec lbl_1_rodata_3AC = { 0.0f, 0.0f, 75.0f };

// Color palette for the rotating burst.
const GXColor lbl_1_rodata_3B8[8] = {
    { 255, 255, 255, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 255, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
    { 192, 255, 192, 255 },
    { 192, 192, 255, 255 },
    { 255, 255, 255, 255 },
};

// Initializes and advances the rotating particle burst.
void fn_1_5168(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx)
{
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    Mtx firstRotation;
    Mtx secondRotation;
    Vec transformedPosition;
    s32 alpha;
    f32 scale;

    if (particle->count == 0) {
        for (initializingEntry = particle->data; initializingEntry != end;
             ++initializingEntry) {
            scale = 1.0f / 60.0f;
            PSMTXRotRad(firstRotation, 'X',
                        MTXDegToRad(RandomFloat() * 180.0f -
                         90.0f));
            PSMTXRotRad(secondRotation, 'Y',
                        MTXDegToRad(RandomFloat() * 360.0f -
                         180.0f));
            PSMTXConcat(secondRotation, firstRotation, firstRotation);
            PSMTXMultVec(firstRotation, &lbl_1_rodata_3AC, &transformedPosition);
            initializingEntry->vel.x = transformedPosition.x * scale;
            initializingEntry->vel.y = transformedPosition.y * scale;
            initializingEntry->vel.z = transformedPosition.z * scale;
            initializingEntry->pos.x = transformedPosition.x;
            initializingEntry->pos.y = transformedPosition.y;
            initializingEntry->pos.z = transformedPosition.z;
            initializingEntry->color = lbl_1_rodata_3B8[RandomUnsigned() & 0x7];
            initializingEntry->zRot = static_cast<f32>(M_PI *
                static_cast<f64>(360.0f * RandomFloat()) / 180.0);
        }
    }

    for (entry = particle->data; entry != end; ++entry) {
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        alpha = entry->color.a;
        alpha -= particle->emitCnt;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            particle->dataCnt = -1;
        }
        entry->color.a = static_cast<u8>(alpha);
        entry->zRot = static_cast<f32>(M_PI *
            (5.0 + 180.0 *
             (static_cast<f64>(entry->zRot) / M_PI)) / 180.0);
    }

    ++particle->emitCnt;
}

// Active expanding ground effects.
ListRoot<EffectNode> lbl_1_bss_E8;

// Creates and registers an effect below the ground plane.
EffectNode::EffectNode(const Vec *initialPosition, f32 finalSizeIncrease)
    : model(fn_1_55D4), position(*initialPosition)
{
    this->maximumSizeIncrease = finalSizeIncrease;
    this->alpha = 0.0f;
    if (this->position.y > 0.0f) {
        // An effect above the ground deletes itself before entering the list.
        delete this;
        return;
    }
    this->insert4(&lbl_1_bss_E8);
    model.setHookData(this);
    model.setLayer(0);
    this->sizeIncrease = 0.0f;
    this->elapsedFrames = 0;
}

// Releases the ground quad model and unlinks the effect from its list.
EffectNode::~EffectNode()
{
}

// Draws the effect attached to the model hook.
void fn_1_55D4(HU3D_MODEL *model, Mtx *matrix)
{
    EffectNode *effect = static_cast<EffectNode *>(model->hookData);
    fn_1_58D4(effect, *matrix);
}

// Deletes all ground effects.
void fn_1_5618()
{
    deleteEffects(lbl_1_bss_E8);
}

// Updates each ground effect.
void fn_1_5704()
{
    lbl_1_bss_E8.visitMoved(fn_1_5734);
}

// Advances one ground effect.
void fn_1_5734(EffectNode *effect)
{
    fn_1_575C(effect);
}

// Expands and fades the ground effect and deletes it after fading.
void fn_1_575C(EffectNode *effect)
{
    ++effect->elapsedFrames;
    effect->sizeIncrease =
        effect->maximumSizeIncrease * (static_cast<f32>(effect->elapsedFrames) / 60.0f);
    effect->alpha = static_cast<f32>(
        (255.0 * (1.0 - static_cast<f64>(effect->sizeIncrease / effect->maximumSizeIncrease))) /
        static_cast<f64>(1.0f + (-effect->position.y / 100.0f)));
    effect->position.y += 1.0f / 30.0f;
    if (effect->position.y > -1.0f) {
        effect->position.y = -1.0f;
    }
    if (effect->alpha < 0.0f) {
        delete effect;
    }
}

// Resets the supplied texture matrix to identity.
static inline MtxPtr identityDrawTextureMatrix(Mtx matrix)
{
    PSMTXIdentity(matrix);
    return matrix;
}

// Loads the ground effect texture.
static inline void loadDrawSpriteTexture(ANIMDATA *texture)
{
    HuSprTexLoad(texture, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
}

// Sets how TEV combines sprite color and texture alpha.
static inline void configureSpriteTev(GXTevColorArg colorInput, GXTevAlphaArg alphaInput)
{
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, colorInput);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, alphaInput, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
}

// Sets culling, color and alpha writes, depth and alpha tests, and blending for sprite quads.
static inline void configureSpriteBlend()
{
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 255);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
}

// Draws the expanding ground effect quad.
void fn_1_58D4(EffectNode *effect, const Mtx sourceMatrix)
{
    Mtx modelMatrix;
    f32 intensity;
    f32 computedIntensity;
    s32 intensityInteger;

    PSMTXScale(modelMatrix, 150.0f + effect->sizeIncrease, 1.0f, 150.0f + effect->sizeIncrease);
    PSMTXTransApply(modelMatrix, modelMatrix, effect->position.x,
                    effect->position.y, effect->position.z);
    PSMTXConcat(sourceMatrix, modelMatrix, modelMatrix);
    GXLoadPosMtxImm(modelMatrix, GX_PNMTX0);
    GXSetNumChans(0U);
    GXSetChanCtrl(GX_COLOR_NULL, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    loadDrawSpriteTexture(fn_1_4224(0));
    GXLoadTexMtxImm(identityDrawTextureMatrix(modelMatrix), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    {
        f32 positionLimit = 1500.0f;
        computedIntensity = 255.0f +
            (255.0f * (effect->position.y / positionLimit));
    }
    if (computedIntensity < 0.0f) {
        intensity = 0.0f;
    } else if (computedIntensity > 255.0f) {
        intensity = 255.0f;
    } else {
        intensity = computedIntensity;
    }

    intensityInteger = static_cast<s32>(intensity);
    GXColor color = { 0, 0, 255, 0 };
    color.r = static_cast<u8>(intensityInteger);
    color.g = static_cast<u8>(intensityInteger);
    color.a = static_cast<u8>(effect->alpha);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    configureSpriteTev(GX_CC_C0, GX_CA_A0);
    configureSpriteBlend();
    static const GXVtxDescList drawVertexDescriptors[] = {
        { GX_VA_POS, GX_DIRECT },
        { GX_VA_TEX0, GX_DIRECT },
        { GX_VA_NULL, GX_NONE }, };
    static const GXVtxAttrFmtList drawVertexFormats[] = {
        { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
        { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
        { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }, };
    GXClearVtxDesc();
    GXSetVtxDescv(drawVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, drawVertexFormats);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(-0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 1.0f);
}

// Active pulsing sprites.
ListRoot<EffectModel> lbl_1_bss_FC;

// Creates and registers a pulsing sprite effect.
EffectModel::EffectModel(const Vec *initialPosition, f32 finalScale)
    : ListNode<EffectModel>(),
    model(fn_1_5FCC), position(*initialPosition)
{
    this->maximumScale = finalScale;
    this->insert4(&lbl_1_bss_FC);
    model.setHookData(this);
    model.setLayer(0);
    this->phase = RandomUnsigned() & 0x1FF;
    this->scale = 0.0f;
    this->offsetScale = 0.0f;
}

// Releases the pulsing sprite model and unlinks the effect from its list.
EffectModel::~EffectModel()
{
}

// Draws the two layers of a pulsing sprite effect.
void fn_1_5FCC(HU3D_MODEL *model, Mtx *matrix)
{
    EffectModel *effect = static_cast<EffectModel *>(model->hookData);
    f32 alpha;

    alpha = static_cast<f32>(
        255.0 * (1.0 - static_cast<f64>(effect->scale / effect->maximumScale)));
    fn_1_6294(effect, *matrix, effect->scale, alpha);
    alpha = static_cast<f32>(
        255.0 * (1.0 - static_cast<f64>(effect->offsetScale / effect->maximumScale)));
    fn_1_6294(effect, *matrix, effect->offsetScale, alpha);
}

// Deletes all pulsing sprite effects.
void fn_1_60A4()
{
    deleteEffects(lbl_1_bss_FC);
}

// Updates each pulsing sprite effect.
void fn_1_6190()
{
    lbl_1_bss_FC.visitMoved(fn_1_61C0);
}

// Advances one pulsing sprite effect.
void fn_1_61C0(EffectModel *effect)
{
    fn_1_61E8(effect);
}

// Advances the sprite phase and updates both layer scales.
void fn_1_61E8(EffectModel *effect)
{
    // The second layer runs half a cycle ahead.
    effect->phase = (effect->phase + 1) & 0x1FF;
    effect->scale =
        (static_cast<f32>(effect->phase) / 511.0f) * effect->maximumScale;
    effect->offsetScale =
        (static_cast<f32>((effect->phase + 256) & 0x1FF) / 511.0f) * effect->maximumScale;
}

// Loads the pulsing sprite texture.
static inline void loadEffectSpriteTexture(ANIMDATA *texture)
{
    HuSprTexLoad(texture, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
}

// Resets the supplied texture matrix to identity.
static inline MtxPtr identityEffectTextureMatrix(Mtx matrix)
{
    PSMTXIdentity(matrix);
    return matrix;
}

// Draws one scaled layer of the pulsing sprite effect.
void fn_1_6294(EffectModel *effect, Mtx parentMatrix, f32 scale, f32 opacity)
{
    Mtx transform;

    PSMTXScale(transform, scale, 1.0f, scale);
    PSMTXTransApply(transform, transform, effect->position.x, 0.0f, effect->position.z);
    PSMTXConcat(parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumChans(0U);
    GXSetChanCtrl(GX_COLOR_NULL, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    loadEffectSpriteTexture(fn_1_4224(1));
    GXLoadTexMtxImm(identityEffectTextureMatrix(transform), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXColor color = { 0, 0, 0, 0 };
    color.a = static_cast<u8>(opacity);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    configureSpriteTev(GX_CC_ONE, GX_CA_A0);
    configureSpriteBlend();
    static const GXVtxDescList effectVertexDescriptors[] = {
        { GX_VA_POS, GX_DIRECT },
        { GX_VA_TEX0, GX_DIRECT },
        { GX_VA_NULL, GX_NONE }
    };
    static const GXVtxAttrFmtList effectVertexFormats[] = {
        { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
        { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
        { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }
    };
    GXClearVtxDesc();
    GXSetVtxDescv(effectVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, effectVertexFormats);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(-0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 1.0f);
}

// Active rotated sprites.
ListRoot<RotatedSpriteEffect> lbl_1_bss_110;

// Creates and registers a rotated sprite effect.
RotatedSpriteEffect::RotatedSpriteEffect(const Vec *initialPosition, f32 initialRotation)
    : ListNode<RotatedSpriteEffect>(), model(fn_1_68C4), position(*initialPosition)
{
    this->rotationY = initialRotation;
    this->insert4(&lbl_1_bss_110);
    model.setHookData(this);
    model.setLayer(0);
    this->alpha = 0;
    this->elapsedFrames = 0;
    this->scale = 0.0f;
    this->maximumScale = 600.0f;
}

// Releases the rotated sprite model and unlinks the effect from its list.
RotatedSpriteEffect::~RotatedSpriteEffect()
{
}

// Draws the rotated sprite attached to the model hook.
void fn_1_68C4(HU3D_MODEL *model, Mtx *matrix)
{
    RotatedSpriteEffect *effect = static_cast<RotatedSpriteEffect *>(model->hookData);
    fn_1_6B54(effect, *matrix);
}

// Deletes all rotated sprite effects.
void fn_1_6908()
{
    deleteEffects(lbl_1_bss_110);
}

// Updates each rotated sprite effect.
void fn_1_69F4()
{
    lbl_1_bss_110.visitMoved(fn_1_6A24);
}

// Advances one rotated sprite effect.
void fn_1_6A24(RotatedSpriteEffect *effect)
{
    fn_1_6A4C(effect);
}

// Advances the sprite scale and fades its alpha over the effect duration.
template <class Effect>
static inline void updateSpriteEffect(Effect *effect, f32 duration)
{
    effect->scale = effect->maximumScale * (static_cast<f32>(effect->elapsedFrames) / duration);
    effect->alpha = static_cast<s32>(255.0 *
                          (1.0 - static_cast<f64>(effect->scale / effect->maximumScale)));
    if (effect->alpha < 0) {
        delete effect;
    }
    // The frame count is advanced after the effect is deleted.
    ++effect->elapsedFrames;
}

// Expands and fades the rotated sprite and deletes it after fading.
void fn_1_6A4C(RotatedSpriteEffect *effect)
{
    updateSpriteEffect(effect, 90.0f);
}

// Resets the supplied texture matrix to identity.
static inline MtxPtr identityRotatedTextureMatrix(Mtx matrix)
{
    PSMTXIdentity(matrix);
    return matrix;
}

// Inputs for the rotated sprite vertices.
static const GXVtxDescList rotatedVertexDescriptors[] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_CLR0, GX_DIRECT },
    { GX_VA_TEX0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE }
};

// Attribute formats for the rotated sprite vertices.
static const GXVtxAttrFmtList rotatedVertexFormats[] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
    { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }
};

// Draws the rotated sprite quad.
void fn_1_6B54(RotatedSpriteEffect *effect, Mtx parentMatrix)
{
    Mtx transform;
    Mtx rotation;
    ANIMDATA *texture;

    PSMTXScale(transform, effect->scale, 1.0f, effect->scale);
    PSMTXRotRad(rotation, 'y', MTXDegToRad(effect->rotationY));
    PSMTXConcat(transform, rotation, transform);
    PSMTXTransApply(transform, transform, effect->position.x, 0.0f, effect->position.z);
    PSMTXConcat(parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    texture = fn_1_4224(1);
    HuSprTexLoad(texture, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXLoadTexMtxImm(identityRotatedTextureMatrix(transform), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    configureSpriteTev(GX_CC_ONE, GX_CA_RASA);
    configureSpriteBlend();
    GXClearVtxDesc();
    GXSetVtxDescv(rotatedVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, rotatedVertexFormats);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-0.5f, 1.0f, -0.5f);
    GXColor4u8(255, 255, 255, static_cast<u8>(effect->alpha));
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(0.5f, 1.0f, -0.5f);
    GXColor4u8(255, 255, 255, static_cast<u8>(effect->alpha));
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.5f, 1.0f, 0.5f);
    GXColor4u8(255, 255, 255, 0);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(-0.5f, 1.0f, 0.5f);
    GXColor4u8(255, 255, 255, 0);
    GXTexCoord2f32(1.0f, 1.0f);
}

// Active ground sprites.
ListRoot<GroundSpriteEffect> lbl_1_bss_124;

// Creates and registers a ground sprite effect.
GroundSpriteEffect::GroundSpriteEffect(const Vec *initialPosition)
    : ListNode<GroundSpriteEffect>(),
    model(fn_1_71F4), position(*initialPosition)
{
    this->insert4(&lbl_1_bss_124);
    model.setHookData(this);
    model.setLayer(0);
    this->alpha = 0;
    this->elapsedFrames = 0;
    this->scale = 0.0f;
    this->maximumScale = 500.0f;
}

// Releases the ground sprite model and unlinks the effect from its list.
GroundSpriteEffect::~GroundSpriteEffect()
{
}

// Draws the ground sprite attached to the model hook.
void fn_1_71F4(HU3D_MODEL *model, Mtx *matrix)
{
    GroundSpriteEffect *effect = static_cast<GroundSpriteEffect *>(model->hookData);
    fn_1_7484(effect, *matrix);
}

// Deletes all ground sprite effects.
void fn_1_7238()
{
    deleteEffects(lbl_1_bss_124);
}

// Updates each ground sprite effect.
void fn_1_7324()
{
    lbl_1_bss_124.visitMoved(fn_1_7354);
}

// Advances one ground sprite effect.
void fn_1_7354(GroundSpriteEffect *effect)
{
    fn_1_737C(effect);
}

// Expands and fades the ground sprite and deletes it after fading.
void fn_1_737C(GroundSpriteEffect *effect)
{
    // The ground sprite grows for one second.
    updateSpriteEffect(effect, 60.0f);
}

// Base color of the ground sprite.
static const GXColor quadColorBase = { 0, 0, 0, 0 };

// Inputs for the ground sprite vertices.
static const GXVtxDescList quadVertexDescriptors[] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_TEX0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE }
};

// Attribute formats for the ground sprite vertices.
static const GXVtxAttrFmtList quadVertexFormats[] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
    { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }
};

// Resets the supplied texture matrix to identity.
static inline MtxPtr identityQuadTextureMatrix(Mtx matrix)
{
    PSMTXIdentity(matrix);
    return matrix;
}

// Loads the ground sprite texture.
static inline void loadQuadSpriteTexture(ANIMDATA *texture)
{
    HuSprTexLoad(texture, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
}

// Draws the ground sprite quad.
void fn_1_7484(GroundSpriteEffect *effect, Mtx parentMatrix)
{
    Mtx transform;

    PSMTXScale(transform, effect->scale, 1.0f, effect->scale);
    PSMTXTransApply(transform, transform, effect->position.x, 0.0f, effect->position.z);
    PSMTXConcat(parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumChans(0U);
    GXSetChanCtrl(GX_COLOR_NULL, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    loadQuadSpriteTexture(fn_1_4224(1));
    GXLoadTexMtxImm(identityQuadTextureMatrix(transform), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXColor color = quadColorBase;
    color.a = static_cast<u8>(effect->alpha);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    configureSpriteTev(GX_CC_ONE, GX_CA_A0);
    configureSpriteBlend();
    GXClearVtxDesc();
    GXSetVtxDescv(quadVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, quadVertexFormats);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(0.5f, 1.0f, -0.5f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(-0.5f, 1.0f, 0.5f);
    GXTexCoord2f32(1.0f, 1.0f);
}

// Active falling particle pools.
ListRoot<MotionParticleOwner> lbl_1_bss_138;

// Creates the falling particles and their ground intersection ring points.
MotionParticleOwner::MotionParticleOwner() : model(fn_1_7CF0)
{
    s32 index;
    s32 ringIndex;
    FallingParticle *particle;
    f32 angle;
    f32 radians;

    this->insert4(&lbl_1_bss_138);
    model.setHookData(this);
    model.setLayer(4);

    for (index = 0; index < static_cast<s32>(sizeof(particles) / sizeof(particles[0])); ++index) {
        particle = &this->particles[index];
        particle->position.x = static_cast<f32>(100.0 *
            (35.0 *
             (static_cast<f64>(RandomFloat()) - 0.5)));
        particle->position.y = 100.0f * (15.0f * RandomFloat());
        particle->position.z = static_cast<f32>(100.0 *
            (35.0 *
             (static_cast<f64>(RandomFloat()) - 0.5)));
        static f32 initialSpeed = 5.0f;
        particle->velocity.x = static_cast<f32>((1.0 + RandomFloat()) *
                              initialSpeed);
        particle->velocity.y = -(3.0f + RandomFloat()) *
                         initialSpeed;
        particle->velocity.z = static_cast<f32>(initialSpeed *
            (static_cast<f64>(RandomFloat()) - 0.5));
    }

    for (ringIndex = 0; ringIndex < static_cast<s32>(sizeof(ring) / sizeof(ring[0])); ++ringIndex) {
        angle = 360.0f *
                    (static_cast<f32>(ringIndex & 0x7) / 8.0f) -
                180.0f;
        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = static_cast<f32>(M_PI * static_cast<f64>(angle) / 180.0);
        this->ring[ringIndex].x = cos(radians);
        this->ring[ringIndex].y = 0.0f;
        this->ring[ringIndex].z = sin(radians);
    }
}

// Releases the falling-particle model and unlinks the pool from its list.
MotionParticleOwner::~MotionParticleOwner()
{
}

// Draws the particle pool attached to the model hook.
void fn_1_7CF0(HU3D_MODEL *model, Mtx *matrix)
{
    fn_1_806C(static_cast<MotionParticleOwner *>(model->hookData), *matrix);
}

// Deletes all falling particle pools.
void fn_1_7D28()
{
    deleteEffects(lbl_1_bss_138);
}

// Updates each falling particle pool.
void fn_1_7E14()
{
    lbl_1_bss_138.visitMoved(fn_1_7E44);
}

// Advances one falling particle pool.
void fn_1_7E44(MotionParticleOwner *pool)
{
    fn_1_7E6C(pool);
}

// Advances the falling particles and respawns particles below the ground.
void fn_1_7E6C(MotionParticleOwner *pool)
{
    s32 index;

    const s32 particleCount = sizeof(pool->particles) / sizeof(pool->particles[0]);
    for (index = 0; index < particleCount; ++index) {
        FallingParticle *particle = &pool->particles[index];

        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->position.z += particle->velocity.z;
        if (particle->position.y < 0.0f) {
            particle->position.x = static_cast<f32>(100.0 *
                (50.0 * (RandomFloat() - 0.5)));
            particle->position.y = 100.0f * (10.0f + RandomFloat());
            particle->position.z = static_cast<f32>(100.0 *
                (45.0 * (RandomFloat() - 0.5)));
            static f32 respawnSpeed = 5.0f;
            particle->velocity.x = static_cast<f32>(respawnSpeed *
                (1.0 + RandomFloat()));
            particle->velocity.y = respawnSpeed *
                -(3.0f + RandomFloat());
            particle->velocity.z = static_cast<f32>(respawnSpeed *
                (RandomFloat() - 0.5));
        }
    }
}

// Draws the falling particles and their ground intersections.
void fn_1_806C(MotionParticleOwner *pool, Mtx sourceMatrix)
{
    s32 particleIndex;

    GXLoadPosMtxImm(sourceMatrix, GX_PNMTX0);
    configurePositionColorDrawing(true);

    const s32 particleCount = sizeof(pool->particles) / sizeof(pool->particles[0]);
    for (particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
        FallingParticle *particle = &pool->particles[particleIndex];
        Vec projectedPosition;
        projectedPosition.x =
            particle->position.x + (5.0f * particle->velocity.x);
        projectedPosition.y =
            particle->position.y + (5.0f * particle->velocity.y);
        projectedPosition.z =
            particle->position.z + (5.0f * particle->velocity.z);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(static_cast<f32>(particle->position.x - 0.1),
                       particle->position.y, particle->position.z);
        GXColor4u8(0, 0, 255, 0);
        GXPosition3f32(static_cast<f32>(particle->position.x + 0.1),
                       particle->position.y, particle->position.z);
        GXColor4u8(0, 0, 255, 0);
        GXPosition3f32(1.0f + projectedPosition.x,
                       projectedPosition.y, projectedPosition.z);
        GXColor4u8(255, 255, 255, 128);
        GXPosition3f32(projectedPosition.x - 1.0f,
                       projectedPosition.y, projectedPosition.z);
        GXColor4u8(255, 255, 255, 128);

        if ((projectedPosition.y < 0.0f) &&
            (particle->position.y > 0.0f)) {
            f32 fraction = -projectedPosition.y /
                (particle->position.y - projectedPosition.y);
            Vec center;
            center.x = projectedPosition.x +
                (fraction * (particle->position.x - projectedPosition.x));
            center.y = 1.0f;
            center.z = projectedPosition.z +
                (fraction * (particle->position.z - projectedPosition.z));
            f32 radius = -projectedPosition.y / 3.5f;
            s32 alpha = static_cast<s32>(192.0f + (2.0f * projectedPosition.y));
            s32 fanIndex;

            if (alpha < 0) {
                alpha = 0;
            }
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0,
                    1 + sizeof(pool->ring) / sizeof(pool->ring[0]));
            GXPosition3f32(center.x, center.y, center.z);
            GXColor4u8(0, 0, 0, 0);
            for (fanIndex = 0;
         fanIndex < static_cast<s32>(sizeof(pool->ring) / sizeof(pool->ring[0]));
         ++fanIndex) {
                GXPosition3f32(
                    center.x + (pool->ring[fanIndex].x * radius),
                    center.y, center.z + (pool->ring[fanIndex].z * radius));
                GXColor4u8(255, 255, 255, static_cast<u8>(alpha));
            }
        }
    }
}

// Active position strips.
ListRoot<ParticleStrip> lbl_1_bss_15C;

// Creates and registers a strip between two position points.
ParticleStrip::ParticleStrip() : model(fn_1_8800)
{
    this->insert4(&lbl_1_bss_15C);
    model.setHookData(this);
    model.setLayer(1);
    this->start.x = this->start.y = this->start.z = 0.0f;
    this->end.x = this->end.y = this->end.z = 0.0f;
}

// Releases the strip model and unlinks the strip from its list.
ParticleStrip::~ParticleStrip()
{
}

// Draws the strip attached to the model hook.
void fn_1_8800(HU3D_MODEL *model, Mtx *matrix)
{
    fn_1_8980(static_cast<ParticleStrip *>(model->hookData), *matrix);
}

// Deletes all position strips.
// Nothing in this minigame calls this.
void fn_1_8838()
{
    deleteEffects(lbl_1_bss_15C);
}

// Updates each position strip.
// Nothing in this minigame calls this.
void fn_1_8924()
{
    lbl_1_bss_15C.visitMoved(fn_1_8954);
}

// Runs the position strip update hook.
void fn_1_8954(ParticleStrip *strip)
{
    fn_1_897C(strip);
}

// Provides the empty position strip update hook.
void fn_1_897C(ParticleStrip *)
{
}

// Draws a shaded strip around the two position points.
void fn_1_8980(ParticleStrip *strip, Mtx sourceMatrix)
{
    GXLoadPosMtxImm(sourceMatrix, GX_PNMTX0);
    configurePositionColorDrawing(true);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 10);
    GXPosition3f32(strip->start.x - 3.0f, strip->start.y, strip->start.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->end.x - 3.0f, strip->end.y, strip->end.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->start.x + 3.0f, strip->start.y, strip->start.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->end.x + 3.0f, strip->end.y, strip->end.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->start.x + 3.0f, strip->start.y, strip->start.z + 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->end.x + 3.0f, strip->end.y, strip->end.z + 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->start.x - 3.0f, strip->start.y, strip->start.z + 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->end.x - 3.0f, strip->end.y, strip->end.z + 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->start.x - 3.0f, strip->start.y, strip->start.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
    GXPosition3f32(strip->end.x - 3.0f, strip->end.y, strip->end.z - 3.0f);
    GXColor4u8(128, 128, 128, 255);
}

// Active timed flashes.
ListRoot<TimedFlash> lbl_1_bss_170;

// Creates and registers a timed flash effect.
TimedFlash::TimedFlash() : model(fn_1_91DC)
{
    this->countdown = 0;
    this->insert4(&lbl_1_bss_170);
    model.setHookData(this);
    model.setLayer(4);
}

// Releases the flash model and unlinks the flash from its list.
TimedFlash::~TimedFlash()
{
}

// Draws the flash when its countdown is active.
void fn_1_91DC(HU3D_MODEL *model, Mtx *matrix)
{
    TimedFlash *effect = static_cast<TimedFlash *>(model->hookData);
    if (effect->countdown > 0) {
        fn_1_93E0(effect, *matrix);
    }
}

// Deletes all timed flash effects.
// Nothing in this minigame calls this.
void fn_1_922C()
{
    deleteEffects(lbl_1_bss_170);
}

// Updates each timed flash effect.
void fn_1_9318()
{
    lbl_1_bss_170.visitMoved(fn_1_9348);
}

// Advances one timed flash effect.
void fn_1_9348(TimedFlash *effect)
{
    fn_1_9390(effect);
}

// Starts or refreshes the flash countdown.
void fn_1_9370(TimedFlash *effect)
{
    if (effect->countdown == 0) {
        effect->frame = 0;
    }
    effect->countdown = 10;
}

// Advances the flash countdown and clamps its opacity.
void fn_1_9390(TimedFlash *effect)
{
    effect->alpha = 0;
    if (effect->countdown > 0) {
        effect->countdown--;
        effect->alpha = effect->countdown * 32;
        if (effect->alpha > 255) {
            effect->alpha = 255;
        }
    }
    effect->frame--;
}

// Resets the supplied texture matrix to identity.
static inline MtxPtr identityRenderTextureMatrix(Mtx matrix)
{
    PSMTXIdentity(matrix);
    return matrix;
}

// Loads the flash effect texture.
static inline void loadRenderSpriteTexture(ANIMDATA *texture)
{
    HuSprTexLoad(texture, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
}

// Draws the flash quad facing the camera.
void fn_1_93E0(TimedFlash *effect, Mtx parentMatrix)
{
    Mtx inverseMatrix;
    Mtx transform;
    f32 halfSize;

    PSMTXInverse(parentMatrix, inverseMatrix);
    inverseMatrix[0][3] = inverseMatrix[1][3] = inverseMatrix[2][3] = 0.0f;
    PSMTXTrans(transform, effect->position.x, effect->position.y, effect->position.z);
    PSMTXConcat(parentMatrix, transform, transform);
    PSMTXConcat(transform, inverseMatrix, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    loadRenderSpriteTexture(fn_1_4224(3));
    GXLoadTexMtxImm(identityRenderTextureMatrix(transform), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    configureSpriteTev(GX_CC_TEXC, GX_CA_A0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 255);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXColor color = { 0, 0, 0, 0 };
    color.a = static_cast<u8>(effect->alpha);
    GXSetTevColor(GX_TEVREG0, color);
    static const GXVtxDescList renderVertexDescriptors[] = {
        { GX_VA_POS, GX_DIRECT },
        { GX_VA_TEX0, GX_DIRECT },
        { GX_VA_NULL, GX_NONE }
    };
    static const GXVtxAttrFmtList renderVertexFormats[] = {
        { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
        { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
        { GX_VA_NULL, GX_POS_XY, GX_U8, 0 }
    };
    GXClearVtxDesc();
    GXSetVtxDescv(renderVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, renderVertexFormats);
    halfSize = static_cast<f32>(((effect->frame & 0x7) * 3) + 70);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-halfSize, -halfSize, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(halfSize, -halfSize, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(halfSize, halfSize, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(-halfSize, halfSize, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
}
