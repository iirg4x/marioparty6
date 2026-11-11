#include "REL/m672DLL/random.h"
/* The particle effects drawn around the board: one rising burst, one ring that carries collision
 * spheres, and one spinning ring of two textures. */
extern "C" {
#include "dolphin.h"
#include "game/animdata.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/m672DLL/collision_object.h"
#include "REL/m672DLL/particle_groups.h"
#include "humath.h"
#include "REL/framework/frame_time.h"

extern "C" {
double sin(double);
double cos(double);
int HuSprTexLoad(ANIMDATA *, int, int, int, int, int);
void fn_1_3350(CollidingParticles *work);
void fn_1_3568(CollidingParticles *work);
void fn_1_3810(RingParticles *state);
void fn_1_3928(RingParticles *state, RingParticle *vector, int index);
void fn_1_48BC(RingParticles *ring);
void fn_1_48C8(RingParticles *ring);
}

inline unsigned char isZero(EffectVector direction);

ParticleGroup::ParticleGroup(ANIMDATA *value, int count, const EffectVector &point)
    : RenderObject(3), texture(value), position(point), particleCount(count)
{
}

RisingParticles::RisingParticles(ANIMDATA *value, int count, int layer, WorldPosition point)
    : ParticleGroup(value, count, point.value), stopWhenExpired(0)
{
    RisingParticle item;
    setLayer((short)layer);
    callback = 0;

    for (int i = 0; i < particleCount; ++i) {
        initParticle(&item);
        item.countdown = (unsigned int)secondsToFrames((2.0 * i) / count);
        if (item.countdown == 0) {
            item.active = 1;
        } else {
            item.active = 0;
        }
        particles.push_back(item);
    }
}

RisingParticles::~RisingParticles() {}

void RisingParticles::initParticle(RisingParticle *particle)
{
    particle->position.set(EffectPoint(RandomFloat(-5.0f, 5.0f), RandomFloat(-5.0f, 5.0f), RandomFloat(5.0f, 5.0f)));
    particle->direction.value.x = 0.0f;
    particle->direction.value.y = 0.5f;
    particle->direction.value.z = 0.0f;
    particle->rotation = RandomFloat(0.0f, 359.9f);
    particle->width = 75.0f;
    particle->height = 75.0f;
    particle->scale = 0.9f;
    particle->alpha = 0.0f;
    particle->speed = 0.8333333f;
    particle->state = 0;
}

void RisingParticles::update()
{
    int index;

    for (index = 0; index < particleCount; ++index) {
        RisingParticle *particle = &particles.at(index);

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
        case 2: {
            EffectVector movement;
            if (isZero(particle->direction) == 0) {
                PSVECNormalize(&particle->direction.value, &particle->direction.value);
            }
            movement = particle->direction;
            float speed = particle->speed;
            PSVECScale(&movement.value, &movement.value, speed);
            PSVECAdd(&particle->position.value, &movement.value, &particle->position.value);
            particle->scale -= 0.01f;
            if (particle->scale <= 0.0f) {
                if (stopWhenExpired != 0) {
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
}

void RisingParticles::draw(Mtx *camera)
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

    PSMTXTranspose(*camera, inverseTransform);
    GXLoadPosMtxImm(*camera, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    HuSprTexLoad(texture, 0, 0, 1, 1, 1);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_FALSE,
                    GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZCompLoc(GX_TRUE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                  GX_PTIDENTITY);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);

    PSMTXTrans(parentTransform, position.value.x, position.value.y, position.value.z);
    for (index = 0; index < particleCount; index++) {
        RisingParticle sprite = static_cast<const std::vector<RisingParticle> &>(particles).at(index);
        if (sprite.active != 0) {
            GXColor color = {255, 78, 17, 0};
            color.a = (u8)sprite.alpha;
            GXSetTevColor(GX_TEVREG0, color);
            PSMTXScale(spriteScale, sprite.scale, sprite.scale, 1.0f);
            PSMTXTrans(spriteTranslation, sprite.position.value.x, sprite.position.value.y,
                       sprite.position.value.z);
            PSMTXRotRad(rotation, 'z', MTXDegToRad(sprite.rotation));
            PSMTXConcat(*camera, parentTransform, spriteTransform);
            PSMTXConcat(spriteTransform, spriteTranslation, spriteTransform);
            PSMTXConcat(spriteTransform, inverseTransform, spriteTransform);
            PSMTXConcat(spriteTransform, spriteScale, spriteTransform);
            PSMTXConcat(spriteTransform, rotation, spriteTransform);
            GXLoadPosMtxImm(spriteTransform, GX_PNMTX0);

            halfWidth = sprite.width / 2.0f;
            halfHeight = sprite.height / 2.0f;
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-halfWidth, halfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 0.0f);
            GXPosition3f32(halfWidth, halfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32(halfWidth, -halfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 1.0f);
            GXPosition3f32(-halfWidth, -halfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 1.0f);
        }
    }
}

CollidingParticles::CollidingParticles(ANIMDATA *value, int count, int layer, WorldPosition point)
    : ParticleGroup(value, count, point.value), emitAngle(0.0f), angle(0.0f), started(0), stopped(0)
{
    CollidingParticle item;
    int i;

    setLayer((short)layer);
    callback = 0;

    for (i = 0; i < particleCount; ++i) {
        initParticle(&item);
        item.timer = (unsigned int)secondsToFrames((2.0f * i) / count);
        item.active = 0;
        particles.push_back(item);
        targets.push_back(new CollisionObject(100.0f));
        CollisionObject *target = targets.at(i);
        target->setPosition(point.value);
        target->radius = 0.0f;
        target->state = 2;
    }
}

CollidingParticles::~CollidingParticles() {}

void CollidingParticles::initParticle(CollidingParticle *particle)
{
    particle->position.set(100.0f * RandomFloat(-0.3f, 0.3f), 100.0f * RandomFloat(-0.3f, 0.3f), 0.0f);
    particle->velocity.set(0.0f, 0.0f, 0.0f);
    particle->velocity.value.z = 1.0f;
    particle->velocity.value.y = 0.01f;
    particle->randomAngle = RandomFloat(0.0f, 359.9f);
    particle->width = 150.0f;
    particle->height = 150.0f;
    particle->rotation = 0.2f;
    particle->rotationStep = 0.02f;
    particle->speed = 20.0f;
    particle->alpha = 0.0f;
    particle->state = 0;
}

void CollidingParticles::update()
{
    int index;
    if (started == 0)
    {
        return;
    }
    for (index = 0; index < particleCount; ++index)
    {
        CollidingParticle * particle = &particles.at(index);
        switch (particle->state)
        {
        case 0 :
            if (particle->timer != 0)
            {
                --particle->timer;
            }
            else
            {
                particle->angle = emitAngle;
                particle->active = 1;
                particle->state = 1;
            }
            break;
        case 1 :
            particle->alpha += 10.0f;
            if (particle->alpha < 255.0f)
            {
            }
            else
            {
                particle->alpha = 255.0f;
                particle->state = 2;
            }
            break;
        case 2 :
            {
                CollisionObject * target = targets.at(index);
                EffectVector velocity;
                EffectVector transformed;
                Mtx rotation;
                float speed;
                float targetScale;
                float resetScale;
                if (isZero(particle->velocity) == 0)
                {
                    PSVECNormalize(&particle->velocity.value, &particle->velocity.value);
                }
                velocity = particle->velocity;
                speed = particle->speed;
                PSVECScale(&velocity.value, &velocity.value, speed);
                PSVECAdd(&particle->position.value, &velocity.value, &particle->position.value);
                PSMTXRotRad(rotation, 0x79, MTXDegToRad(particle->angle));
                PSMTXMultVec(rotation, &particle->position.value, &transformed.value);
                PSVECAdd(&transformed.value, &position.value, &transformed.value);
                target->setPosition(transformed);
                targetScale = particle->width * particle->rotation / 2.0f / 2.0f;
                target->radius = targetScale;
                particle->rotation += particle->rotationStep;
                particle->speed *= 0.985f;
                if (particle->rotation > 1.0f)
                {
                    particle->velocity.value.y *= 1.1f;
                }
                if (particle->rotation > 1.5f)
                {
                    particle->alpha -= 25.0f;
                    if (particle->alpha < 0.0f)
                    {
                        if (stopped != 0)
                        {
                            particle->state = 3;
                            particle->active = 0;
                        }
                        else
                        {
                            initParticle(particle);
                            resetScale = particle->width * particle->rotation / 2.0f;
                            target->radius = resetScale;
                        }
                    }
                }
                break;
            }
        }
    }
}

void CollidingParticles::draw(Mtx *camera)
{
    Mtx scale;
    Mtx translation;
    Mtx inverse;
    Mtx particleRotation;
    Mtx ownerRotation;
    Mtx spin;
    Mtx transform;
    Vec particlePosition;
    Vec particleOffset;
    int index;
    float halfWidth;
    float halfHeight;

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX,
                 GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    HuSprTexLoad(texture, 0, 0, 1, 1, 1);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetZCompLoc(GX_TRUE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                     GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);

    for (index = 0; index < particleCount; ++index) {
        CollidingParticle particle = static_cast<const std::vector<CollidingParticle> &>(particles).at(index);
        if (particle.active != 0) {
            GXColor color = {255, 78, 17, 0};
            color.a = (unsigned char)particle.alpha;
            GXSetTevColor(GX_TEVREG0, color);
            PSMTXRotRad(ownerRotation, 'y', MTXDegToRad(angle));
            PSMTXRotRad(particleRotation, 'y', MTXDegToRad(particle.angle));
            PSMTXMultVec(ownerRotation, &offset, &particleOffset);
            PSMTXMultVec(particleRotation, &particle.position.value, &particlePosition);
            PSVECAdd(&particlePosition, &particleOffset, &particlePosition);
            PSVECAdd(&particlePosition, &position.value, &particlePosition);
            PSMTXRotRad(spin, 'z', MTXDegToRad(particle.randomAngle));
            PSMTXScale(scale, particle.rotation, particle.rotation, 1.0f);
            PSMTXTrans(translation, particlePosition.x, particlePosition.y, particlePosition.z);
            PSMTXConcat(*camera, translation, transform);
            PSMTXConcat(transform, particleRotation, transform);
            PSMTXTranspose(transform, inverse);
            PSMTXConcat(transform, inverse, transform);
            PSMTXConcat(transform, scale, transform);
            PSMTXConcat(transform, spin, transform);
            GXLoadPosMtxImm(transform, GX_PNMTX0);

            halfWidth = particle.width / 2.0f;
            halfHeight = particle.height / 2.0f;
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-halfWidth, halfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 0.0f);
            GXPosition3f32(halfWidth, halfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32(halfWidth, -halfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 1.0f);
            GXPosition3f32(-halfWidth, -halfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 1.0f);
        }
    }
}

void fn_1_3350(CollidingParticles *work)
{
    int index;

    work->started = 1;
    for (index = 0; index < work->particleCount; ++index) {
        CollisionObject *item;
        CollidingParticle parameters = work->particles.at(index);

        work->initParticle(&parameters);

        item = work->targets.at(index);
        item->active = 1;
    }
}

void fn_1_3568(CollidingParticles *work)
{
    int index;
    CollisionObject *node;

    work->stopped = 1;
    for (index = 0; index < work->particleCount; index++) {
        node = work->targets.at(index);
        node->active = 0;
    }
}

RingParticles::RingParticles(ANIMDATA *first, ANIMDATA *second, int count, int layer, WorldPosition point)
    : ParticleGroup(first, count, point.value), secondTexture(second)
{
    setLayer((short)layer);
    fn_1_3810(this);
}

RingParticles::~RingParticles() {}

void fn_1_3810(RingParticles *state)
{
    RingParticle vector;
    int index;

    state->ringWidth = 350.0f;
    state->ringDepth = 350.0f;
    state->ringAngle = 0.0f;
    state->ringScale = 1.0f;
    state->scaleOffset = 0.0f;
    state->pulseAngle = 0.0f;
    state->alpha = 0.0f;
    state->active = 0;
    state->fadingOut = 0;
    state->fadeState = 0;
    index = 0;
    for (; index < state->particleCount; index += 1) {
        fn_1_3928(state, &vector, index);
        state->items.push_back(vector);
    }
}

void fn_1_3928(RingParticles *state, RingParticle *vector, int index)
{
    vector->position.x = 0.0f;
    vector->position.y = 0.0f;
    vector->position.z = 0.0f;
    vector->width = 80.0f;
    vector->height = 80.0f;
    vector->scale = 1.0f;
    vector->orbitAngle = (360.0f / state->particleCount) * index;
    vector->bobAngle = RandomFloat(0.0f, 359.9f);
    vector->orbitStep = 18.0f;
    vector->bobStep = (float)(0.016666666666666666 *
                        RandomFloat(360.0f, 720.0f) *
                        2.0);
    vector->orbitRadius = 100.0f *
                 RandomFloat(0.8f, 1.0f);
    vector->spin = RandomFloat(0.0f, 359.9f);
}

void RingParticles::update()
{
    float targetAlpha;
    int particleIndex;
    RingParticle *particle;

    if (this->active != 0) {
        switch (this->fadeState) {
        case 0:
            targetAlpha = 128.0 * (1.0 - 2.0f * this->scaleOffset);
            if (this->alpha < targetAlpha) {
                this->alpha += 2.0f;
            } else {
                this->alpha = targetAlpha;
                this->fadeState = 1;
            }
            break;
        case 1:
            if (this->fadingOut == 0) {
                this->alpha = 128.0 * (1.0 - 2.0f * this->scaleOffset);
            } else {
                this->alpha -= 10.0f;
                if (this->alpha < 0.0f) {
                    fn_1_3810(this);
                }
            }
            break;
        }
        this->ringAngle += 18.0;
        if (this->ringAngle > 360.0f) {
            this->ringAngle -= 360.0f;
        }
        this->pulseAngle += 12.0;
        this->scaleOffset = 0.05f *
                          sin((M_PI * this->pulseAngle) / 180.0);
        this->ringScale = 1.0f + this->scaleOffset;
        for (particleIndex = 0; particleIndex < this->particleCount; ++particleIndex) {
            particle = &this->items.at(particleIndex);
            particle->position.x = particle->orbitRadius *
                                 cos((M_PI * particle->orbitAngle) / 180.0);
            particle->position.z = particle->orbitRadius *
                                 sin((M_PI * particle->orbitAngle) / 180.0);
            particle->position.y = 10.0 + 100.0 * (0.1f *
                                 sin((M_PI * particle->bobAngle) / 180.0));
            particle->bobAngle -= particle->bobStep;
            particle->orbitAngle -= particle->orbitStep;
            if (particle->bobAngle < 0.0f) particle->bobAngle += 360.0f;
            if (particle->orbitAngle < 0.0f) particle->orbitAngle += 360.0f;
        }
    }
}

void RingParticles::draw(Mtx *camera)
{
    Mtx scale;
    Mtx translation;
    Mtx rotation;
    Mtx transform;
    Mtx inverseRotation;
    Mtx particleRotation;
    Vec position;
    float ringHalfWidth;
    float ringHalfDepth;
    float particleHalfWidth;
    float particleHalfHeight;
    int particleIndex;

    if (this->active != 0) {
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        HuSprTexLoad(this->texture, 0, 0, 1, 1, 1);
        HuSprTexLoad(this->secondTexture, 0, 1, 1, 1, 1);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2,
                       GX_FALSE, GX_TEVPREV);
        GXSetCullMode(GX_CULL_BACK);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GXSetZCompLoc(GX_TRUE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                         GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXColor ringColor = {255, 255, 255, 0};
        ringColor.a = this->alpha;
        GXSetTevColor(GX_TEVREG0, ringColor);
        PSMTXTrans(translation, this->position.value.x, this->position.value.y, this->position.value.z);
        PSMTXScale(scale, this->ringScale, 1.0f, this->ringScale);
        PSMTXRotRad(rotation, 'y', MTXDegToRad(this->ringAngle));
        PSMTXConcat(*camera, translation, transform);
        PSMTXConcat(transform, scale, transform);
        PSMTXConcat(transform, rotation, transform);
        GXLoadPosMtxImm(transform, GX_PNMTX0);
        ringHalfWidth = this->ringWidth / 2.0f;
        ringHalfDepth = this->ringDepth / 2.0f;
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-ringHalfWidth, 0.0f, -ringHalfDepth);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(ringHalfWidth, 0.0f, -ringHalfDepth);
        GXTexCoord2f32(0.0f, 1.0f);
        GXPosition3f32(ringHalfWidth, 0.0f, ringHalfDepth);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(-ringHalfWidth, 0.0f, ringHalfDepth);
        GXTexCoord2f32(1.0f, 0.0f);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
        GXColor particleColor = {255, 255, 255, 0};
        particleColor.a = this->alpha;
        GXSetTevColor(GX_TEVREG0, particleColor);
        for (particleIndex = 0; particleIndex < this->particleCount; ++particleIndex) {
            RingParticle particle = static_cast<const std::vector<RingParticle> &>(items).at(particleIndex);
            PSVECAdd(&particle.position, &this->position.value, &position);
            PSMTXTrans(translation, position.x, position.y, position.z);
            PSMTXScale(scale, particle.scale, particle.scale, 1.0f);
            PSMTXRotRad(particleRotation, 'z', MTXDegToRad(particle.spin));
            PSMTXConcat(*camera, translation, transform);
            PSMTXTranspose(transform, inverseRotation);
            PSMTXConcat(transform, inverseRotation, transform);
            PSMTXConcat(transform, scale, transform);
            PSMTXConcat(transform, particleRotation, transform);
            GXLoadPosMtxImm(transform, GX_PNMTX0);
            particleHalfWidth = particle.width / 2.0f;
            particleHalfHeight = particle.height / 2.0f;
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(-particleHalfWidth, particleHalfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 0.0f);
            GXPosition3f32(particleHalfWidth, particleHalfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32(particleHalfWidth, -particleHalfHeight, 0.0f);
            GXTexCoord2f32(0.0f, 1.0f);
            GXPosition3f32(-particleHalfWidth, -particleHalfHeight, 0.0f);
            GXTexCoord2f32(1.0f, 1.0f);
        }
    }
}

void fn_1_48BC(RingParticles *ring)
{
    ring->active = 1;
}

void fn_1_48C8(RingParticles *ring)
{
    ring->fadingOut = 1;
}

inline unsigned char isZero(EffectVector direction)
{
    unsigned char isZero;
    unsigned char xyZero;

    isZero = 0;
    xyZero = 0;
    if ((0.0f == direction.value.x) && (0.0f == direction.value.y)) {
        xyZero = 1;
    }
    if ((xyZero != 0) && (0.0f == direction.value.z)) {
        isZero = 1;
    }
    return isZero;
}
