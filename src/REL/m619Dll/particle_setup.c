/* Creates and animates the scene's burst, ring and looping particle effects. */
#include "REL/m619Dll/module_types.h"
#include "REL/m619Dll/particle.h"

void fn_1_C69C(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
s16 fn_1_C7E4(void *owner, ANIMDATA *animation, s32 particleCount);
void fn_1_C890(M619ParticleEffect *effect);
void fn_1_C8CC(M619ParticleEffect *effect);
void fn_1_C8D8(M619ParticleEffect *flash, HuVecF *position);
void fn_1_C910(M619ParticleEffect *flash, HuVecF *rotation);
void fn_1_C948(HU3D_MODEL *model, HU3D_PARTICLE *task, Mtx matrix);
/* Creates the scene's 16-particle radial burst when that effect is started. */
void fn_1_CE20(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
void fn_1_D004(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
/* When a transition or player effect starts, creates its 32-particle expanding ring. */
void fn_1_D420(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
void fn_1_D604(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
/* Scene animation setup calls this to create its six-particle repeating ring effects. */
void fn_1_D9EC(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation);
void fn_1_DBD0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

/* Player-object setup calls this for both flash points, each with a five-particle burst. */
void fn_1_C69C(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation) {
    M619Scene *mainState;
    HU3D_MODELID model;

    mainState = (M619Scene *)lbl_1_bss_0->data;
    model = fn_1_C7E4(work, mainState->animations[0], 5);
    work->model = model;
    Hu3DModelLayerSet(model, 6);
    Hu3DParticleHookSet(model, fn_1_C948);
    Hu3DParticleBlendModeSet(model, 1U);
    Hu3DParticleColSet(model, 255U, 255U, 255U);
    Hu3DModelAttrSet(model, 1U);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    work->position = *position;
    work->positionSource = position;
    work->rotation = *rotation;
    work->rotationSource = rotation;
    work->state = 0;
    work->active = 0;
}

/* Called by effect setup to create the particle model, attach its owner, and clear its pool. */
inline s16 fn_1_C7E4(void *owner, ANIMDATA *animation, s32 particleCount) {
    s16 particleSystemId;
    HU3D_PARTICLE *particleSystem;
    HU3D_PARTICLE_DATA *particle;
    s32 particleIndex;

    particleSystemId = Hu3DParticleCreate(animation, (s16)particleCount);
    particleSystem = (HU3D_PARTICLE *)Hu3DData[particleSystemId].hookData;
    particleSystem->work = owner;
    particle = particleSystem->data;
    particleIndex = 0;
    while (particleIndex < particleCount) {
        particle->scale = 0.0f;
        particle->time = 0;
        particle->parManId = 0;
        particleIndex++;
        particle++;
    }
    return particleSystemId;
}

void fn_1_C890(M619ParticleEffect *effect) {
    Hu3DModelAttrReset(effect->model, 1U);
    effect->state = 16;
}

void fn_1_C8CC(M619ParticleEffect *effect) {
    effect->state = 256;
}

void fn_1_C8D8(M619ParticleEffect *flash, HuVecF *position) {
    Hu3DModelPosSetV(flash->model, position);
}

void fn_1_C910(M619ParticleEffect *flash, HuVecF *rotation) {
    Hu3DModelRotSetV(flash->model, rotation);
}

/* Updates the five-particle burst from its source position on each model hook. */
void fn_1_C948(HU3D_MODEL *model, HU3D_PARTICLE *task, Mtx matrix) {
    M619ParticleEffect *state;
    HU3D_PARTICLE_DATA *point;
    s32 completedCount;
    s32 index;
    s32 pointCount;
    f32 scale;
    f32 angle;
    f32 sineAngle;
    f32 cosineAngle;
    f32 sineRandomAngle;
    f32 cosineRandomAngle;
    f32 alpha;

    state = task->work;
    switch (state->state) {
    case 16:
        point = task->data;
        pointCount = task->maxCnt;
        scale = 36.0f;
        for (index = 0; index < pointCount; index++, point++) {
            point->time = 0;
            point->parManId = 0;
            point->pos = *state->positionSource;
            point->color.r = 255;
            point->color.g = 255;
            point->color.b = 255;
            point->color.a = 255;

            angle = (f32)((3.141592653589793 *
                    (f64)(90.0f +
                          (state->rotationSource->y +
                           (scale *
                            ((f32)index +
                              ((f32)frandmod(10001) / 10000.0f) -
                             0.5f))))) /
                   180.0);
            sineAngle = (f32)sin((f64)angle);
            cosineAngle = (f32)cos((f64)angle);

            angle = (f32)((3.141592653589793 *
                         (f64)(45.0f +
                               (45.0f *
                                ((f32)frandmod(10001) /
                                 10000.0f)))) /
                        180.0);
            sineRandomAngle = (f32)sin((f64)angle);
            cosineRandomAngle = (f32)sin((f64)angle);
            point->vel.x = 2.5f * sineAngle * cosineRandomAngle;
            point->vel.y = 2.0f +
                               (2.5f * sineRandomAngle);
            point->vel.z = 2.5f * cosineAngle * cosineRandomAngle;
            point->scale = 45.0f;
            point->zRot = (f32)(3.141592653589793 *
                                  ((f32)frandmod(10001) /
                                  10000.0f));
        }
        state->state = 256;
        /* fall through */
    case 256:
        completedCount = 0;
        point = task->data;
        pointCount = task->maxCnt;
        for (index = 0; index < pointCount; index++, point++) {
            if (point->parManId == 0) {
                point->time++;
                scale = (f32)point->time / 30.0f;
                point->pos.x += point->vel.x;
                point->pos.y += point->vel.y;
                point->pos.z += point->vel.z;
                point->vel.y -= 0.272f;
                alpha = 255.0f * (1.0f - scale);
                point->color.a = (u8)alpha;
                point->zRot += 0.02617993877991494;
                if (point->time >= 30) {
                    point->parManId = 1;
                }
            } else if (point->parManId == 1) {
                point->scale = 0.0f;
                completedCount++;
            }
        }
        DCStoreRangeNoSync(task->data,
                           task->maxCnt * sizeof(HU3D_PARTICLE_DATA));
        if (completedCount == pointCount) {
            Hu3DModelAttrReset(state->model, 1U);
            state->state = 0;
        }
        /* fall through */
    case 0:
        break;
    }
}

/* Model-transition setup calls this to create its 16-particle radial burst. */
void fn_1_CE20(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation) {
    M619Scene *mainState;
    s16 particleModel;

    mainState = (M619Scene *)lbl_1_bss_0->data;
    particleModel = fn_1_C7E4(work, mainState->animations[0], 16);
    work->model = particleModel;
    Hu3DModelLayerSet(particleModel, 6);
    Hu3DParticleHookSet(particleModel, fn_1_D004);
    Hu3DParticleBlendModeSet(particleModel, 1U);
    Hu3DParticleColSet(particleModel, 255U, 255U, 255U);
    Hu3DModelAttrSet(particleModel, 1U);
    Hu3DModelPosSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    work->positionSource = position;
    work->rotation.x = 0.0f;
    work->rotation.y = 0.0f;
    work->rotation.z = 0.0f;
    work->rotationSource = rotation;
    work->state = 0;
    work->active = 0;
}

/* Initializes and advances an outward particle burst on each model hook. */
void fn_1_D004(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix) {
    M619ParticleEffect *system;
    HU3D_PARTICLE_DATA *entry;
    s32 particleCount;
    s32 index;
    f32 age;
    f32 angle;
    f32 sine;
    f32 cosine;
    f32 angleStep;
    f32 alpha;

    system = (M619ParticleEffect *)particle->work;
    switch (system->state) {
    case 16:
        particle->dataCnt = 0;
        entry = (HU3D_PARTICLE_DATA *)particle->data;
        particleCount = particle->maxCnt;
        index = 0;
        while (index < particleCount) {
            entry->time = 0;
            angleStep = 360.0f / (f32)particleCount;
            angle = (f32)((3.141592653589793 *
                           ((angleStep * (f32)index) +
                            (angleStep * (((f32)frandmod(10001) /
                                          10000.0f) - 0.5f)))) /
                          180.0);
            sine = (f32)sin((f64)angle);
            cosine = (f32)cos((f64)angle);
            entry->pos.x = 65.0f * sine;
            entry->pos.y = 0.0f;
            entry->pos.z = 65.0f * cosine;
            entry->color.r = 255;
            entry->color.g = 255;
            entry->color.b = 255;
            entry->color.a = 255;
            entry->vel.x = 2.0f * sine;
            entry->vel.y = 5.0f;
            entry->vel.z = 2.0f * cosine;
            entry->scale = 45.0f;
            entry->zRot = (f32)(3.141592653589793 *
                                      (f64)((f32)frandmod(10001) /
                                           10000.0f));
            index++;
            entry++;
        }
        system->state = 256;
        /* fall through */
    case 256:
        entry = (HU3D_PARTICLE_DATA *)particle->data;
        particleCount = particle->maxCnt;
        index = 0;
        while (index < particleCount) {
            entry->time++;
            age = (f32)entry->time / 30.0f;
            if (age > 1.0f) {
                age = 1.0f;
            }
            entry->pos.x += entry->vel.x;
            entry->pos.y += entry->vel.y;
            entry->pos.z += entry->vel.z;
            entry->vel.y -= 0.272f;
            alpha = 255.0f * (1.0f - age);
            entry->color.a = (u8)alpha;
            entry->zRot += 0.02617993877991494;
            index++;
            entry++;
        }
        DCStoreRangeNoSync(particle->data, particle->maxCnt * 72);
        particle->dataCnt++;
        if (particle->dataCnt >= 30) {
            system->state = 0;
        }
        /* fall through */
    case 0:
        break;
    }
}

/* Model-transition setup calls this to create its 32-particle expanding ring. */
void fn_1_D420(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation) {
    M619Scene *mainState;
    s16 particleModel;

    mainState = (M619Scene *)lbl_1_bss_0->data;
    particleModel = fn_1_C7E4(work, mainState->animations[0], 32);
    work->model = particleModel;
    Hu3DModelLayerSet(particleModel, 6);
    Hu3DParticleHookSet(particleModel, fn_1_D604);
    Hu3DParticleBlendModeSet(particleModel, 1U);
    Hu3DParticleColSet(particleModel, 255U, 255U, 255U);
    Hu3DModelAttrSet(particleModel, 1U);
    Hu3DModelPosSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    work->positionSource = position;
    work->rotation.x = 0.0f;
    work->rotation.y = 0.0f;
    work->rotation.z = 0.0f;
    work->rotationSource = rotation;
    work->state = 0;
    work->active = 0;
}

/* The particle hook expands the ring and fades it over 45 eligible updates. */
void fn_1_D604(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix) {
    M619ParticleEffect *work;
    HU3D_PARTICLE_DATA *data;
    s32 particleCount;
    s32 particleIndex;
    f32 angleStep;
    f32 angle;
    f32 sine;
    f32 cosine;
    f32 progress;
    f32 alpha;

    work = (M619ParticleEffect *)particle->work;
    switch (work->state) {
    case 16:
        particle->dataCnt = 0;
        data = particle->data;
        particleCount = particle->maxCnt;
        for (particleIndex = 0; particleIndex < particleCount;
             particleIndex++, data++) {
            data->time = 0;
            angleStep = 360.0f / (f32)particleCount;
            angle = (f32)((3.141592653589793 *
                           (f64)(angleStep * (f32)particleIndex)) /
                          180.0);
            sine = (f32)sin((f64)angle);
            cosine = (f32)cos((f64)angle);
            data->pos.x = 130.0f * sine;
            data->pos.y = 0.0f;
            data->pos.z = 130.0f * cosine;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
            data->color.a = 255;
            data->vel.x = 5.0f * sine;
            data->vel.y = 9.0f;
            data->vel.z = 5.0f * cosine;
            data->scale = 45.0f;
            data->zRot = (f32)(3.141592653589793 *
                                (f64)((f32)frandmod(10001) / 10000.0f));
        }
        work->state = 256;
        /* fall through */
    case 256:
        data = particle->data;
        particleCount = particle->maxCnt;
        for (particleIndex = 0; particleIndex < particleCount;
             particleIndex++, data++) {
            data->time++;
            progress = (f32)data->time / 45.0f;
            if (progress > 1.0f) {
                progress = 1.0f;
            }
            data->pos.x += data->vel.x;
            data->pos.y += data->vel.y;
            data->pos.z += data->vel.z;
            data->vel.y -= 0.272f;
            alpha = 255.0f * (1.0f - progress);
            if (alpha < 0.0f) {
                alpha = 0.0f;
            }
            data->color.a = (u8)alpha;
            data->zRot += 0.02617993877991494;
        }
        DCStoreRangeNoSync(particle->data, particle->maxCnt * 72);
        particle->dataCnt++;
        if (particle->dataCnt >= 45) {
            work->state = 0;
        }
        /* fall through */
    case 0:
        break;
    }
}

/* Scene animation setup calls this three times to create its six-particle ring effects. */
void fn_1_D9EC(M619ParticleEffect *work, const HuVecF *position,
               const HuVecF *rotation) {
    M619Scene *mainState;
    s16 particleModel;

    mainState = (M619Scene *)lbl_1_bss_0->data;
    particleModel = fn_1_C7E4(work, mainState->animations[0], 6);
    work->model = particleModel;
    Hu3DModelLayerSet(particleModel, 6);
    Hu3DParticleHookSet(particleModel, fn_1_DBD0);
    Hu3DParticleBlendModeSet(particleModel, 1U);
    Hu3DParticleColSet(particleModel, 255U, 255U, 255U);
    Hu3DModelAttrSet(particleModel, 1U);
    Hu3DModelPosSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(particleModel, 0.0f, 0.0f,
                    0.0f);
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    work->positionSource = position;
    work->rotation.x = 0.0f;
    work->rotation.y = 0.0f;
    work->rotation.z = 0.0f;
    work->rotationSource = rotation;
    work->state = 0;
    work->active = 0;
}

/* The particle hook restarts the ring every 60 updates until the effect's final cycle ends. */
void fn_1_DBD0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix) {
    M619ParticleEffect *work;
    HU3D_PARTICLE_DATA *data;
    f32 angleOffset;
    f32 angle;
    f32 progress;
    f32 alpha;
    s32 particleCount;
    s32 particleIndex;

    work = (M619ParticleEffect *)particle->work;
    switch (work->state) {
    case 16:
        particle->dataCnt = 0;
        data = particle->data;
        particleCount = particle->maxCnt;
        angleOffset = 360.0f * ((f32)frandmod(10001) / 10000.0f);
        for (particleIndex = 0; particleIndex < particleCount;
             particleIndex++, data++) {
            data->time = 0;
            angle = (f32)((3.141592653589793 *
                           (f64)((15.0f * (f32)particleIndex) +
                                 (angleOffset +
                                   (10.0f * (((f32)frandmod(10001) /
                                             10000.0f) - 0.5f))))) /
                          180.0);
            data->pos.x = (f32)(130.0 * sin((f64)angle));
            data->pos.y = 0.0f;
            data->pos.z = (f32)(130.0 * cos((f64)angle));
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
            data->color.a = 255;
            data->vel.x = (f32)(2.0 * sin((f64)angle));
            data->vel.y = 6.0f;
            data->vel.z = (f32)(2.0 * cos((f64)angle));
            data->scale = 45.0f;
            data->zRot = (f32)(3.141592653589793 *
                                (f64)((f32)frandmod(10001) / 10000.0f));
        }
        work->state = 17;
        /* fall through */
    case 17:
    case 256:
        data = particle->data;
        particleCount = particle->maxCnt;
        for (particleIndex = 0; particleIndex < particleCount;
             particleIndex++, data++) {
            data->time++;
            progress = (f32)data->time / 60.0f;
            if (progress > 1.0f) {
                progress = 1.0f;
            }
            data->pos.x += data->vel.x;
            data->pos.y += data->vel.y;
            data->pos.z += data->vel.z;
            data->vel.y -= 0.272f;
            alpha = 255.0f * (1.0f - progress);
            if (alpha < 0.0f) {
                alpha = 0.0f;
            }
            data->color.a = (u8)alpha;
            data->zRot += 0.02617993877991494;
        }
        DCStoreRangeNoSync(particle->data, particle->maxCnt * 72);
        particle->dataCnt++;
        if (particle->dataCnt >= 60) {
            if (work->state == 256) {
                work->state = 0;
            } else {
                work->state = 16;
            }
        }
        break;
    case 0:
        break;
    }
}
