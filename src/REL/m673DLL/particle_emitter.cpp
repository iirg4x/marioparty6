// Updates and draws the rising and colliding particles.

#include "REL/minigame_particle_quad.h"
#include "game/sprput.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
extern "C" {
#include "math.h"
#include "dolphin/mtx/GeoTypes.h"
}
#include "dolphin/gx/GXStruct.h"
#include "REL/framework/random.h"
#include "REL/m673DLL/collision_body.h"
#include "REL/m673DLL/particle_group.h"
#include "REL/m673DLL/rising_particles.h"
#include "REL/m673DLL/colliding_particles.h"
#include "REL/m673DLL/vector_check.h"
#include "REL/m673DLL/colliding_particle.h"
#include "REL/m673DLL/scene_resources.h"
#include "REL/m673DLL/follow_effect.h"

// Initializes the particle's randomized offset and motion parameters.
static inline void initializeParticle(CollidingParticle *particle, f32 angle)
{
    f32 finalValue;
    f64 randomY;
    f64 randomX;

    randomY = 100.0f * RandomFloat(-0.2f, 0.2f);
    randomX = 100.0f * RandomFloat(-0.2f, 0.2f);
    particle->offset.x = randomX;
    particle->offset.y = randomY;
    particle->offset.z = 0.0f;
    particle->direction.x = 0.0f;
    particle->direction.y = 0.0f;
    particle->direction.z = 0.0f;
    particle->direction.x = (f32)sin((M_PI * angle) / 180.0);
    particle->direction.z = (f32)cos((M_PI * angle) / 180.0);
    particle->direction.y = 0.01f;
    particle->rotation = RandomFloat(0.0f, 359.9f);
    particle->width = 150.0f;
    particle->height = 150.0f;
    particle->scale = 0.3f;
    particle->scaleStep = 0.02f;
    particle->speed = 10.0f;
    particle->alpha = 0.0f;
    particle->active = 0;
    particle->countdown = 0;
    particle->followEffect.scale = 0.0f;
    finalValue = particle->width * particle->scale / 2.0f;
    particle->collisionBody.radius = finalValue;
}

// Configures the common particle culling, alpha, texture coordinates, and vertex format.
static inline void SetParticleVertexState()
{
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZCompLoc(GX_TRUE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

// Draws a textured quad centered on the current position matrix.
// Initializes the render layer, animation resource, position, and item count.
ParticleGroup::ParticleGroup(ANIMDATA *value, s32 count, const Position3 &initial)
    : RenderObject(4), texture(value), position(initial.value), particleCount(count)
{
}

// Creates the rising-particle group and assigns each particle its first state.
ParticleEmitter::ParticleEmitter(ANIMDATA *texture, s32 particleCount, s32 layer,
                                 const Position3 &position)
    : ParticleGroup(texture, particleCount, position), stopWhenExpired(0), alphaScale(1.0f)
{
    EmitterParticle particle;
    s32 index;

    this->enabled = 1;
    setLayer(layer);
    callback = 0;

    for (index = 0; index < this->particleCount; ++index) {
        fn_1_2B34(this, &particle);
        particle.countdown = secondsToFrames((2.0 * index) / particleCount);
        if (particle.countdown == 0) {
            particle.active = 1;
        } else {
            particle.active = 0;
        }
        particles.push_back(particle);
    }
}

// Releases the base particle render object.
ParticleGroup::~ParticleGroup()
{
}

// Destroys the rising particles and releases their vector.
ParticleEmitter::~ParticleEmitter()
{
}

// Resets a particle for reuse by the rising particle group.
inline void ParticleEmitter::initParticle(EmitterParticle *particle)
{
    f64 randomZ;
    f64 randomY;
    f64 randomX;
    f32 randomPosition[3];

    randomZ = RandomFloat(5.0f, 5.0f);
    randomY = RandomFloat(-5.0f, 5.0f);
    randomX = RandomFloat(-5.0f, 5.0f);
    randomPosition[0] = randomX;
    randomPosition[1] = (f32)randomY;
    randomPosition[2] = (f32)randomZ;
    particle->position.x = randomPosition[0];
    particle->position.y = randomPosition[1];
    particle->position.z = randomPosition[2];
    particle->direction.x = 0.0f;
    particle->direction.y = 1.0f;
    particle->direction.z = 0.0f;
    particle->rotation = RandomFloat(0.0f, 359.9f);
    particle->width = 150.0f;
    particle->height = 150.0f;
    particle->scale = 0.5f;
    particle->alpha = 0.0f;
    particle->speed = 50.0f / 60.0f;
    particle->state = 0;
}

// Randomizes the particle position and rotation and sets its initial motion and size.
void fn_1_2B34(ParticleEmitter *, EmitterParticle *parameters)
{
    ParticleEmitter::initParticle(parameters);
}

// Advances particle delays, motion, and recycling.
void ParticleEmitter::update()
{
    s32 index;
    EmitterParticle *particle;

    for (index = 0; index < this->particleCount; index++) {
        particle = &this->particles.at(index);

        switch (particle->state) {
        case 0:
            if (particle->countdown != 0) {
                particle->countdown--;
            } else {
                particle->state = 1;
                particle->active = 1;
            }
            break;
        case 1:
            particle->alpha += 10.0f;
            if (!(particle->alpha < 255.0f)) {
                particle->alpha = 255.0f;
                particle->state = 2;
            }
            break;
        case 2:
            if (fn_1_B58(&particle->direction) == 0) {
                PSVECNormalize(&particle->direction, &particle->direction);
            }
            Vec movement = particle->direction;
            f32 speed = particle->speed;
            PSVECScale(&movement, &movement, speed);
            PSVECAdd(&particle->position, &movement, &particle->position);
            particle->scale -= 0.005f;
            if (particle->scale <= 0.0f) {
                if (this->stopWhenExpired != 0) {
                    particle->state = 3;
                    particle->active = 0;
                } else {
                    initParticle(particle);
                }
            }
            break;
        }
    }
}

// Draws the enabled particles as textured billboards.
void ParticleEmitter::draw(Mtx *matrix)
{
    Mtx spriteScale;
    Mtx spriteTranslation;
    Mtx parentTransform;
    Mtx spriteTransform;
    Mtx inverseTransform;
    Mtx rotation;
    s32 index;
    f32 halfWidth;
    f32 halfHeight;

    if (this->enabled != 0 && this->alphaScale != 0.0f) {
        PSMTXTranspose(*matrix, inverseTransform);
        GXLoadPosMtxImm(*matrix, GX_PNMTX0);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                      GX_DF_NONE, GX_AF_NONE);
        HuSprTexLoad(this->texture, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE,
                        GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2,
                        GX_FALSE, GX_TEVPREV);
        SetParticleVertexState();
        GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);

        PSMTXTrans(parentTransform, this->position.x, this->position.y,
                   this->position.z);
        for (index = 0; index < this->particleCount; ++index) {
            EmitterParticle sprite =
                static_cast<const std::vector<EmitterParticle> &>(this->particles).at(index);
            if (sprite.active != 0) {
                f32 scaledOpacity = sprite.alpha;
                scaledOpacity *= this->alphaScale;
                GXColor color = { 0xFF, 0x4E, 0x11, 0x00 };
                color.a = (u8)scaledOpacity;
                GXSetTevColor(GX_TEVREG0, color);
                PSMTXScale(spriteScale, sprite.scale, sprite.scale, 1.0f);
                PSMTXTrans(spriteTranslation, sprite.position.x,
                           sprite.position.y, sprite.position.z);
                MTXRotDeg(rotation, 'z', sprite.rotation);
                PSMTXConcat(*matrix, parentTransform, spriteTransform);
                PSMTXConcat(spriteTransform, spriteTranslation, spriteTransform);
                PSMTXConcat(spriteTransform, inverseTransform, spriteTransform);
                PSMTXConcat(spriteTransform, spriteScale, spriteTransform);
                PSMTXConcat(spriteTransform, rotation, spriteTransform);
                GXLoadPosMtxImm(spriteTransform, GX_PNMTX0);

                halfWidth = sprite.width / 2.0f;
                halfHeight = sprite.height / 2.0f;
                DrawParticleQuad(halfWidth, halfHeight);
            }
        }
    }
}

// Sets the rising particle group's enabled flag.
void fn_1_3588(ParticleEmitter *emitter, u8 enabled)
{
    emitter->enabled = enabled;
}

// Creates the colliding-particle group at the supplied position.
BurstEmitter::BurstEmitter(ANIMDATA *animation, int count, int layer,
                           Position3 initialPosition)
    : ParticleGroup(animation, count, initialPosition), baseAngle(0.0f), oscillationAngle(0.0f),
      elapsedFrames(0), active(0)
{
    s32 index;
    CollidingParticle *entry;

    setLayer(layer);

    for (index = 0; index < this->particleCount; index++) {
        entry = new CollidingParticle(index, count);
        entry->index = index;
        particles.push_back(entry);
    }
}

// Destroys the colliding particles and releases their vector.
BurstEmitter::~BurstEmitter()
{
    s32 loopLimit;
    s32 index;

    loopLimit = particles.size();
    for (index = 0; index < loopLimit; index++) {
        delete particles.at(index);
    }
}

// Updates particles and spawns them in sequence while the group is active.
void BurstEmitter::update()
{
    s32 index;
    CollidingParticle *entry;
    f32 angularOffset;
    Vec *positionPointer;

    if (this->active != 0) {
        for (index = 0; index < this->particleCount; index++) {
            entry = this->particles.at(index);
            entry->fn_1_48C0();
            if (this->elapsedFrames == entry->index * 4) {
                angularOffset = (f32)(45.0 * sin((M_PI * this->oscillationAngle) / 180.0));
                angularOffset += this->baseAngle;
                Vec position(this->position);
                positionPointer = &position;
                fn_1_4570(entry, angularOffset, positionPointer);
                this->oscillationAngle = this->oscillationAngle - 760.0 / 60.0;
            }
            if (this->elapsedFrames >= secondsToFrames(5.0f)) {
                fn_1_406C(this);
            }
        }
        this->elapsedFrames++;
    }
}

// Draws the active colliding particle billboards.
void BurstEmitter::draw(Mtx *matrix)
{
    Mtx spriteScale;
    Mtx parentTransform;
    Mtx spriteTranslation;
    Mtx inverseTransform;
    Mtx rotationY;
    Mtx rotationZ;
    Mtx spriteTransform;
    s32 index;
    f32 halfWidth;
    f32 halfHeight;

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    HuSprTexLoad(this->texture, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE,
                    GX_TEVPREV);
    SetParticleVertexState();
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);

    PSMTXTrans(parentTransform, this->position.x, this->position.y, this->position.z);
    PSMTXConcat(*matrix, parentTransform, parentTransform);
    for (index = 0; index < this->particleCount; index++) {
        CollidingParticle *sprite =
            static_cast<const std::vector<CollidingParticle *> &>(this->particles).at(index);
        if (sprite->active != 0) {
            GXColor color = { 0xFF, 0x4E, 0x11, 0x00 };
            color.a = (u8)sprite->alpha;
            GXSetTevColor(GX_TEVREG0, color);
            MTXRotDeg(rotationY, 'y', sprite->angle);
            MTXRotDeg(rotationZ, 'z', sprite->rotation);
            PSMTXScale(spriteScale, sprite->scale, sprite->scale, 1.0f);
            PSMTXTrans(spriteTranslation, sprite->offset.x, sprite->offset.y,
                       sprite->offset.z);
            PSMTXConcat(parentTransform, spriteTranslation, spriteTransform);
            PSMTXTranspose(spriteTransform, inverseTransform);
            PSMTXConcat(spriteTransform, inverseTransform, spriteTransform);
            PSMTXConcat(spriteTransform, spriteScale, spriteTransform);
            PSMTXConcat(spriteTransform, rotationZ, spriteTransform);
            GXLoadPosMtxImm(spriteTransform, GX_PNMTX0);

            halfWidth = sprite->width / 2.0f;
            halfHeight = sprite->height / 2.0f;
            DrawParticleQuad(halfWidth, halfHeight);
        }
    }
}

// Starts directional emission of colliding particles if it is inactive.
void fn_1_3F74(BurstEmitter *emitter, f32 angle)
{
    if (emitter->active != 0) {
        return;
    }
    emitter->active = 1;
    emitter->baseAngle = angle;
    emitter->oscillationAngle = 0.0f;
    emitter->elapsedFrames = 0;
    emitter->offset.x = (f32)(sin(M_PI * emitter->baseAngle / 180.0));
    emitter->offset.y = 0.0f;
    emitter->offset.z = (f32)(cos(M_PI * emitter->baseAngle / 180.0));
    PSVECScale(&emitter->offset, &emitter->offset, 100.0f);
    emitter->offset.y = 140.0f;
}

// Stops further spawns and freezes existing particles. Individually active particles
// can still be drawn at their last positions.
void fn_1_406C(BurstEmitter *emitter)
{
    emitter->active = 0;
}

// Returns the colliding particle animation from the scene resources.
ANIMDATA *fn_1_4078()
{
    SceneResources &resources = SceneResources::getChecked();
    return resources.animationAt(2);
}

// Creates a collision body and a quad for a colliding particle.
CollidingParticle::CollidingParticle(s32, s32)
    : collisionBody(100.0f), followEffect(&effectPosition, 100.0f, fn_1_4078())
{
    ParticleCallback callback;

    this->fn_1_4334(0.0f);
    this->effectPosition.x = 0.0f;
    this->effectPosition.y = 0.0f;
    this->effectPosition.z = 0.0f;
    GXColor color = { 0xFF, 0xC8, 0xC8, 0xFF };
    this->followEffect.color = color;
    this->followEffect.setAlpha(0.0f);
    callback = &CollidingParticle::fn_1_49F0;
    this->state = callback;
}

// Releases the collision body and the particle quad.
CollidingParticle::~CollidingParticle()
{
}

// Resets the particle's motion parameters for the supplied angle.
void CollidingParticle::fn_1_4334(f32 angle)
{
    initializeParticle(this, angle);
}

// Starts a particle at the supplied position and angle.
void fn_1_4570(CollidingParticle *particle, f32 angle, const Vec *position)
{
    f32 size;
    f64 x;
    f64 y;
    f64 z;

    particle->fn_1_4334(angle);
    particle->origin = *position;
    particle->angle = angle;
    particle->active = 1;
    particle->followEffect.setAlpha(0.0f);
    size = particle->width;
    particle->followEffect.scale = size;
    particle->effectPosition.x = 0.0f;
    particle->effectPosition.y = 0.0f;
    particle->effectPosition.z = 0.0f;
    z = particle->origin.z;
    y = particle->origin.y;
    x = particle->origin.x;
    particle->collisionBody.position.set(x, y, z);
    ParticleCallback callback = &CollidingParticle::fn_1_4980;
    particle->state = callback;
}

// Advances the particle state and updates the collision body and attached quad.
void CollidingParticle::fn_1_48C0()
{
    f32 scaledValue;
    f32 halfValue;

    if (state != 0) {
        (this->*state)();
    }
    effectPosition = collisionBody.position.value;
    scaledValue = 3.0f * (width * scale);
    followEffect.scale = scaledValue;
    halfValue = (width * scale) / 2.0f;
    collisionBody.radius = halfValue;
}

// Waits for the particle countdown, then activates collision and the next state.
void CollidingParticle::fn_1_4980()
{
    if (countdown != 0) {
        --countdown;
        return;
    }
    collisionBody.relation = 2;
    collisionBody.active = 1;
    ParticleCallback callback = &CollidingParticle::fn_1_49F4;
    state = callback;
}

// Leaves the colliding particle unchanged in its idle state.
void CollidingParticle::fn_1_49F0()
{
}

// Sets half opacity and enters the moving state.
void CollidingParticle::fn_1_49F4()
{
    ParticleCallback callback;
    f32 amount;

    alpha = 255.0f;
    alpha += 10.0f;
    if (alpha > 255.0f) {
        alpha = 255.0f;
    }
    amount = 0.5f * (alpha / 255.0f);
    followEffect.setAlpha(amount);
    if (alpha < 255.0f) {
        return;
    }
    followEffect.setAlpha(0.5f);
    callback = &CollidingParticle::fn_1_4AE8;
    state = callback;
}

// Advances the particle offset along its direction and updates its position.
void CollidingParticle::fn_1_4AE8()
{
    f32 amount;
    f32 movementSpeed;

    if (fn_1_B58(&direction) == 0) {
        PSVECNormalize(&direction, &direction);
    }
    Vec displacement = direction;
    movementSpeed = speed;
    PSVECScale(&displacement, &displacement, movementSpeed);
    PSVECAdd(&offset, &displacement, &offset);
    PSVECAdd(&offset, &origin, &collisionBody.position.value);
    collisionBody.position.value.y = collisionBody.position.value.y - 100.0;
    scale += this->scaleStep;
    speed *= 0.985f;
    if (scale > 1.0f) {
        direction.y *= 1.2f;
    }
    if (scale > 1.5f) {
        alpha -= 25.0f;
        if (alpha < 30.0f) {
            collisionBody.active = 0;
        }
        if (alpha < 0.0f) {
            ParticleCallback callback;
            alpha = 0.0f;
            active = 0;
            collisionBody.active = 0;
            callback = &CollidingParticle::fn_1_49F0;
            state = callback;
        }
        amount = 0.5f * (alpha / 255.0f);
        followEffect.setAlpha(amount);
    }
}

// Returns whether all three vector components are zero.
u8 fn_1_4CF8(const Vec *vector)
{
    u8 allZero;
    u8 xyZero;

    allZero = 0;
    xyZero = 0;
    if (0.0f == vector->x && 0.0f == vector->y) {
        xyZero = 1;
    }
    if (xyZero != 0 && 0.0f == vector->z) {
        allZero = 1;
    }
    return allZero;
}
