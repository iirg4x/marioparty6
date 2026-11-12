#include "REL/m671DLL/game_constants.h"
#include "REL/m671DLL/light_layers.h"

#include <vector>
#include "REL/framework/sound.h"
#include "dolphin/mtx.h"

#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "REL/framework/game_object.h"

#include "REL/framework/game_object2.h"
#include "REL/framework/particle.h"
#include "REL/framework/random.h"
#include "REL/m671DLL/particle_game_object.h"
#include "REL/m671DLL/resources.h"
#include "REL/m671DLL/particle_owner.h"

#include <math.h>

extern "C" {
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "game/sprput.h"
}

void fn_1_2AE8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

ParticleGameObject::ParticleGameObject(ANIMDATA *animation, Vec *position)
    : GameObject(2), model(fn_1_2AE8, animation, 0x20)
{
    model.setPosition(position);
    model.setLayer(2);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
}

ParticleGameObject::~ParticleGameObject() {}

void ParticleGameObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt < 0) delete this;
}

void fn_1_3C10(ParticleOwner *, Mtx);

const GXColor lbl_1_rodata_CC = {0xFA, 0xFA, 0xE6, 0x80};

void fn_1_2AE8(HU3D_MODEL *, HU3D_PARTICLE *work, Mtx) {
    HU3D_PARTICLE_DATA *end = work->data + work->maxCnt;
    HU3D_PARTICLE_DATA *first_entry;
    HU3D_PARTICLE_DATA *second_entry;
    int intensity;
    int active;

    if (work->count == 0) {
        for (first_entry = work->data; first_entry != end; ++first_entry) {
            float angle = RandomFloat(-3.1415927f, 3.1415927f);
            first_entry->vel.x = (float)cos(angle);
            first_entry->vel.y = 0.0f;
            first_entry->vel.z = (float)sin(angle);
            first_entry->pos.x = 100.0f * (0.1f * first_entry->vel.x);
            first_entry->pos.y = 50.0f + RandomFloat(0.0f, 50.0f);
            first_entry->pos.z = 100.0f * (0.1f * first_entry->vel.z);
            first_entry->vel.x *= 10.0f;
            first_entry->vel.z *= 10.0f;
            first_entry->color = lbl_1_rodata_CC;
            first_entry->scale = 150.0f;
        }
    }

    active = 0;
    for (second_entry = work->data; second_entry != end; ++second_entry) {
        if (second_entry->scale <= 0.0f) {
            continue;
        }
        PSVECAdd(&second_entry->vel, &second_entry->pos, &second_entry->pos);
        second_entry->vel.x *= 0.9f;
        second_entry->vel.z *= 0.9f;
        second_entry->scale += 0.08333333333333333;
        intensity = second_entry->color.a;
        intensity -= 4;
        if (intensity < 0) {
            intensity = 0;
            second_entry->scale = 0.0f;
            work->dataCnt = -1;
        }
        second_entry->color.a = (u8)intensity;
        ++active;
    }
    work->emitCnt = (s16)active;
}

void fn_1_2D78(HU3D_MODEL *, HU3D_PARTICLE *owner, Mtx value)
{
    void *callback = owner->work;
    static_cast<TimedParticleEffect *>(callback)->draw(value);
}

const GXColor lbl_1_rodata_F8[4] = { {0xD2, 0xDC, 0xF0, 0}, {0xB4, 0xBE, 0xDC, 0}, {0xA0, 0xB4, 0xC8, 0}, {0x78, 0x82, 0xA0, 0} };

void fn_1_2DBC(void *source, HU3D_PARTICLE_DATA *parameters)
{

    parameters->time = 0;
    parameters->parManId = (s16)((RandomUnsigned() & 3) + 2);
    parameters->pos.x = RandomFloat(-30.000002f, 30.000002f);
    parameters->pos.y = 0.0f;
    parameters->pos.z = RandomFloat(-30.000002f, 30.000002f);
    parameters->vel.x = RandomFloat(-0.5f, 0.5f);
    parameters->vel.y = (f32)((f64)(100.0f *
        RandomFloat(5.0f, 6.0f)) * 0.016666666666666666);
    parameters->vel.z = RandomFloat(-0.5f, 0.5f);
    parameters->scale = 100.0f * (RandomFloat(0.0f, 0.2f) + 0.4f);
    parameters->zRot = MTXDegToRad(RandomFloat(0.0f, 360.0f));
    parameters->color = lbl_1_rodata_F8[RandomUnsigned() & 3];
}

void fn_1_2D78(HU3D_MODEL *, HU3D_PARTICLE *owner, Mtx value);

/* Attaches the particle pool to its owner and sets up its drawing model. */
static inline void initializeParticleModel(int modelId, TimedParticleEffect *owner,
                                          Vec *position, Vec *rotation)
{
    owner->owner = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[(s16)modelId].hookData);
    owner->owner->work = owner;
    fn_800326D0(modelId, fn_1_2D78);
    fn_80026B64(modelId, 1);
    fn_8003269C(modelId, 0);
    fn_80026B8C(modelId, 6);
    fn_80026790(modelId, position);
    fn_80026838(modelId, rotation);
    owner->modelId = modelId;
}

TimedParticleEffect::TimedParticleEffect(ANIMDATA *animation, Vec *initialPosition, Vec *rotation)
    : GameObject(2), position(*initialPosition)
{
    HU3D_MODELID id;
    HU3D_PARTICLE_DATA *particleDataEnd;
    HU3D_PARTICLE_DATA *particleData;
    id = fn_8003216C(animation, 0x40);
    initializeParticleModel(id, this, initialPosition, rotation);
    particleDataEnd = owner->data + owner->maxCnt;
    particleData = owner->data;
    while (particleData != particleDataEnd) {
        particleData->scale = 0.0f;
        ++particleData;
    }
    state = 0;
    elapsedTime = 0.0f;
    duration = 2.0f;
    active = 0;
}

TimedParticleEffect::~TimedParticleEffect()
{
    owner->work = 0;
    fn_800262D0(modelId);
}

void TimedParticleEffect::update()
{
    switch (state) {
    case 0:
        elapsedTime += 0.016666666666666666;
        if (elapsedTime <
            duration) {
            return;
        }
        elapsedTime = 0.0f;
        soundPlayAt(1, 0x8A2, &position);
        ++state;
        /* fall through */
    case 1: {
        HU3D_PARTICLE_DATA *end = owner->data +
            owner->maxCnt;
        HU3D_PARTICLE_DATA *entry = owner->data;

        while (entry != end) {
            fn_1_2DBC(entry, entry);
            ++entry;
        }
        ++state;
        /* fall through */
    }
    case 2:
        elapsedTime += 0.016666666666666666;
        if (active != 0) {
            duration = RandomFloat(4.0f, 6.0f);
            elapsedTime = 0.0f;
            state = 0;
        }
    }
}

void TimedParticleEffect::draw(Mtx)
{
    HU3D_PARTICLE *particle = owner;
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    HU3D_PARTICLE_DATA *entry;

    active = 1;
    entry = particle->data;
    while (entry != end) {
        if (entry->scale != 0.0f) {
            f32 oldValue;

            entry->zRot += 0.008726646f;
            entry->pos.x += entry->vel.x;
            entry->pos.z += entry->vel.z;
            if (entry->time == 0) {
                entry->scale += 0.3333333383003871;
                oldValue = entry->vel.y;
                entry->vel.y += -gravity;
                oldValue = 0.5f * (oldValue + entry->vel.y);
                entry->pos.y += oldValue;
                entry->color.a += 0x10;
                if (entry->color.a > 0xBE) {
                    entry->color.a = 0xBE;
                }
                if (entry->pos.y > 80.0f) {
                    entry->time += 1;
                }
            } else {
                entry->scale += 0.6666666766007742;
                entry->pos.y = (f32)((f64)entry->pos.y - 0.3333333333333333);
                entry->color.a -= entry->parManId;
                if (entry->color.a < 6) {
                    if (elapsedTime > 3.0f) {
                        entry->color.a = 0;
                        entry->scale = 0.0f;
                    } else {
                        fn_1_2DBC(entry, entry);
                    }
                }
            }
            active = 1;
        }
        ++entry;
    }
}

extern void fn_1_37FC(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

void fn_1_37FC(HU3D_MODEL *, HU3D_PARTICLE *owner, Mtx value)
{
    void *callback = owner->work;
    fn_1_3C10(static_cast<ParticleOwner *>(callback), value);
}

static inline void initializeParticleModel(int modelId,
                                               ParticleOwner *owner)
{
    owner->particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[(s16)modelId].hookData);
    owner->particle->work = owner;
    fn_800326D0(modelId, fn_1_37FC);
    fn_800325B0(modelId, 0xFF, 0xFF, 0xFF);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    fn_800326D0(modelId, fn_1_37FC);
    fn_80026B8C(modelId, 6);
    fn_80026B64(modelId, 1);
    fn_80026760(modelId, 0.0f, 0.0f, 0.0f);
    fn_80026808(modelId, 0.0f, 0.0f, 0.0f);
    fn_800269E8(modelId, 1);
    owner->positionX = 0.0f;
    owner->positionY = 0.0f;
    owner->positionZ = 0.0f;
    owner->vector.x = 0.0f;
    owner->vector.y = 0.0f;
    owner->vector.z = 0.0f;
    owner->modelId = modelId;
}

ParticleOwner::ParticleOwner()
{
    Resources *lightManagerData;
    HU3D_MODELID modelId;

    completed = 0;
    callbackReady = 0;
    lightManagerData = &Resources::getChecked();
    this->animation = lightManagerData->animation(0);
    modelId = fn_8003216C(this->animation, 0x20);
    initializeParticleModel(modelId, this);
}

ParticleOwner::~ParticleOwner()
{
    particle->work = 0;
    fn_800262D0(modelId);
}

extern "C" {

const GXColor lbl_1_rodata_16C = {0xFA, 0xFA, 0xE6, 0};

}

void fn_1_3AB0(ParticleOwner *source, HU3D_PARTICLE_DATA *particle)
{
    s32 randomValue;

    particle->time = 0;
    particle->parManId = (s16)((RandomUnsigned() & 0xF) + 4);
    particle->pos.x = source->positionX + RandomFloat(-50.0f, 50.0f);
    particle->pos.y = source->positionY;
    particle->pos.z = source->positionZ + RandomFloat(-50.0f, 50.0f);
    particle->scale = 100.0f *
                      (0.6f +
                       RandomFloat(0.0f, 0.3f));
    particle->zRot = MTXDegToRad(RandomFloat(0.0f, 360.0f));
    randomValue = RandomUnsigned();
    particle->color = lbl_1_rodata_16C;
}

void fn_1_3C10(ParticleOwner *emitter, Mtx)
{
    HU3D_MODEL *model;
    s32 index;
    u8 noParticles;
    HU3D_PARTICLE_DATA *particle;
    HU3D_PARTICLE *list;

    if (!emitter->completed) {
        return;
    }
    model = &lbl_802C01C8[emitter->modelId];
    list = emitter->particle;
    noParticles = 1;
    index = 0;
    particle = list->data;
    for (; index < list->maxCnt; ++index, ++particle) {
        if (particle->time == 0xFF) {
            if (emitter->callbackReady) {
                continue;
            }
            fn_1_3AB0(emitter, particle);
        }
        if (particle->time == 0) {
            particle->color.a += 0x10;
            if (particle->color.a > 0xEE) {
                particle->color.a = 0xFF;
                particle->time += 1;
            }
        } else {
            particle->color.a -= particle->parManId;
            if (particle->color.a < 0x14) {
                particle->color.a = 0;
                particle->time = 0xFF;
            }
        }
        particle->zRot += MTXDegToRad(1.0f);
        noParticles = 0;
    }
    if (noParticles == 1) {
        emitter->callbackReady = 1;
        fn_800269E8(emitter->modelId, 1);
    }
}

void fn_1_3E7C(ParticleOwner *owner, const Vec *position) {
    if (owner->completed != 0) {
        owner->positionX = position->x;
        owner->positionY = position->y;
        owner->positionZ = position->z;
    }
}

void ParticleOwner::setVector(const Vec *source) {
    vector.x = source->x;
    vector.y = source->y;
    vector.z = source->z;
}

/* Initializes each particle in the emitter's pool with randomized motion. */
void fn_1_3EC0(ParticleOwner *emitter,
               const Vec *source)
{
    HU3D_PARTICLE *system;
    HU3D_PARTICLE_DATA *particle;
    s32 index;

    emitter->positionX = source->x;
    emitter->positionY = source->y;
    emitter->positionZ = source->z;
    system = emitter->particle;
    particle = system->data;
    emitter->completed = 1;
    emitter->callbackReady = 0;
    for (index = 0; index < system->maxCnt; ++index, ++particle) {
        fn_1_3AB0(emitter, particle);
    }
    fn_80026A44(emitter->modelId, 1U);
}

void fn_1_4078(ParticleOwner *record) {
    record->callbackReady = 1;
}

AttributeOwner::AttributeOwner() : RenderObject(2)
{
    Resources *lightManagerData;
    Resources *lightManager;
    Resources *lightManagerOwner;
    ANIMDATA *animation;

    Resources::getChecked();
    lightManager = Resources::self;
    lightManagerOwner = lightManager;
    lightManagerData = lightManagerOwner;
    animation = lightManagerData->animations.at(1);
    this->animation = animation;
    position.value.x = 0.0f;
    position.value.y = 0.0f;
    position.value.z = 0.0f;
    alphaScale = 1.0f;
    scaleXZ = 1.0f;
    fn_800269E8(*static_cast<const Model *>(this), 1U);
    setLayer(1);
}

void fn_1_41B4(AttributeOwner *owner, u8 reset)
{
    if (reset == 1) {
        fn_80026A44(*static_cast<Model *>(owner), 1);
        return;
    }
    fn_800269E8(*static_cast<Model *>(owner), 1);
}

void AttributeOwner::update()
{
}

void AttributeOwner::draw(Mtx *parentMatrix)
{
    Mtx transform;

    PSMTXTrans(transform, this->position.value.x, this->position.value.y, this->position.value.z);
    mtxScaleCat(transform, this->scaleXZ, 1.0f, this->scaleXZ);
    PSMTXConcat(*parentMatrix, transform, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    HuSprTexLoad(this->animation, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    {
        GXColor color = { 0, 0, 64, 128 };
        color.a = (u8)(color.a * this->alphaScale);
        GXSetTevColor(GX_TEVREG0, color);
    }
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-125.0f, 0.0f, -125.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(125.0f, 0.0f, -125.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(125.0f, 0.0f, 125.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-125.0f, 0.0f, 125.0f);
    GXTexCoord2f32(1.0f, 0.0f);
}

inline LightLayer0::~LightLayer0() {}
inline LightLayer4::~LightLayer4() {}

LightLayer0::LightLayer0() : RenderObject(3) { setLayer(0); }

void LightLayer0::update() {}

void LightLayer0::draw(Mtx *)
{
    Resources *setting = &Resources::getChecked();
    short lightId = setting->id();
    static Vec position = {0.0f, 400.0f, 0.0f};
    static Vec direction = {0.0f, 1.0f, 0.0f};
    Hu3DGLightPosSetV(lightId, &position, &direction);
}

LightLayer4::LightLayer4() : RenderObject(3) { setLayer(4); }

void LightLayer4::update() {}

void LightLayer4::draw(Mtx *)
{
    Resources *setting = &Resources::getChecked();
    short lightId = setting->id();
    static Vec position = {0.0f, 400.0f, 0.0f};
    static Vec direction = {0.0f, -1.0f, 0.1f};
    Hu3DGLightPosSetV(lightId, &position, &direction);
}

