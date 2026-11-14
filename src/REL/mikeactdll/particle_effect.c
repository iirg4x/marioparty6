/* Particle effects and display controls for the microphone activity. */
#include "game/audio.h"
#include "REL/mikeactdll/particle_effect.h"
/* Particle effects and display controls. */
#include "REL/mikeactdll/module_types.h"
#include "dolphin/os.h"
#include <string.h>

extern s16 lbl_1_bss_DE[3];
extern s16 lbl_1_bss_D8[3];

typedef struct MikeActParticleTarget {
    u8 scale;
    u8 alpha;
} MikeActParticleTarget;

void fn_1_CA6C(void);
void fn_1_D444(void);

void fn_1_C2C0(void)
{
    fn_1_CA6C();
    fn_1_D444();
}

void fn_1_C2E4(s16 playerNo, HuVecF *effectData)
{
    fn_1_D5F4(playerNo, effectData, 1);
}

void fn_1_C318(short state) {
    fn_1_D5F4(state, 0, 0);
}

void fn_1_C348(short state) {
    fn_1_D5F4(state, 0, 2);
}

float fn_1_C378(float start, float end, float elapsed, float duration)
{
    if (elapsed <= 0.0f) {
        return start;
    }
    if (elapsed >= duration) {
        return end;
    }
    return start + ((elapsed / duration) * (end - start));
}

inline float fn_1_C378(float start, float end, float elapsed, float duration);

float fn_1_C3BC(float current, float target, float divisor)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (divisor - 1.0f))) / divisor;
}

inline float fn_1_C3BC(float current, float target, float divisor);

void fn_1_C3EC(s16 index, s16 visible)
{
    if (visible != 0) {
        fn_80026A44(lbl_1_bss_DE[index], 1U);
    } else {
        fn_800269E8(lbl_1_bss_DE[index], 1U);
    }
}

void fn_1_C468(s16 modelNo, HuVecF *position, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &lbl_802C01C8[lbl_1_bss_DE[modelNo]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        fn_80026790(lbl_1_bss_DE[modelNo], position);
    }
    fn_80026A44(lbl_1_bss_DE[modelNo], (1 << 0));
}

void fn_1_C56C(s16 modelNo)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &lbl_802C01C8[lbl_1_bss_DE[modelNo]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 2;
    }
}

void fn_1_C5F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    MikeActParticleTarget particleTarget[
        5] = {
        { 16, 255 },
        { 16, 255 },
        { 48, 128 },
        { 128, 64 },
        { 255, 32 },
    };
    HU3D_PARTICLE_DATA *data;
    float velX1;
    float velY1;
    float velX2;
    float velY2;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x =
                particleTarget[i % 5].alpha;
            data->vel.y =
                particleTarget[i % 5].scale;
            velX1 = data->vel.x;
            data->color.a =
                fn_1_C3BC(data->color.a, velX1, 10.0f);
            velY1 = data->vel.y;
            data->scale = fn_1_C3BC(data->scale, velY1, 10.0f);
        } else if (data->time == 2) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            velX2 = data->vel.x;
            data->color.a = fn_1_C3BC(data->color.a, velX2, 10.0f);
            velY2 = data->vel.y;
            data->scale = fn_1_C3BC(data->scale, velY2, 10.0f);
        } else {
            data->color.a = 0;
            data->scale = 0.0f;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the three particle models and attach their update callback. */
void fn_1_CA6C(void) {
    s16 i;

    i = 0;
    while (i < 3) {
        lbl_1_bss_DE[i] = fn_8003216C(HuSprAnimRead(fn_80008DD0(0xA70013, 0x10000000, 2)), 10);
        fn_80026760(lbl_1_bss_DE[i], 0.0f, 0.0f, 0.0f);
        fn_800268B0(lbl_1_bss_DE[i], 1.0f, 1.0f, 1.0f);
        fn_800269E8(lbl_1_bss_DE[i], (1 << 0));
        fn_80026B8C(lbl_1_bss_DE[i], 7);
        fn_800326D0(lbl_1_bss_DE[i], fn_1_C5F4);
        fn_8003269C(lbl_1_bss_DE[i], 1U);
        i += 1;
    }
}

/* Destroy the three particle models. */
void fn_1_CBE4(void) {
    s16 i;

    i = 0;
    while (i < 3) {
        fn_800262D0(lbl_1_bss_DE[i]);
        i += 1;
    }
}

void fn_1_CC3C(s16 index, s16 visible)
{
    if (visible != 0) {
        fn_80026A44(lbl_1_bss_D8[index], 1U);
    } else {
        fn_800269E8(lbl_1_bss_D8[index], 1U);
    }
}

void fn_1_CCB8(s16 modelNo, const HuVecF *position,
                                     GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &lbl_802C01C8[lbl_1_bss_D8[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 1;
    if (color != NULL) {
        particle->pos.x = color->r;
        particle->pos.y = color->g;
        particle->pos.z = color->b;
    }
    if (position != NULL) {
        particle->spawnCenter.x = position->x;
        particle->spawnCenter.y = position->y;
        particle->spawnCenter.z = position->z;
    }
    fn_80026A44(lbl_1_bss_D8[modelNo], (1 << 0));
}

inline void fn_1_CCB8(s16 modelNo, const HuVecF *position,
                                     GXColor *color);

void fn_1_CDF0(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &lbl_802C01C8[lbl_1_bss_D8[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

void fn_1_CE44(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;
    float color;
    float random;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && particle->dataCnt == 1 && spawnCount < 2) {
            spawnCount++;
            data->time = 1;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(30) + 30;
            data->accel.x = frandmod(100) - 50;
            data->accel.y = -frandmod(100) - 50;
            data->accel.z = frandmod(100) - 50;
            PSVECNormalize(&data->accel, &data->accel);
            data->accel.x *= 2.0f;
            data->accel.y *= 5.0f;
            data->accel.z *= 2.0f;
            data->pos.x = particle->spawnCenter.x + frandmod(100) - 50;
            data->pos.y = particle->spawnCenter.y + frandmod(100) - 50;
            data->pos.z = particle->spawnCenter.z + frandmod(100) - 50;
            random = frandmod(128);
            color = particle->pos.x + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.r = color;
            color = particle->pos.y + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.g = color;
            color = particle->pos.z + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.b = color;
            data->color.a = 0;
        } else if (data->time == 1) {
            data->pos.x += data->accel.x;
            data->pos.y += data->accel.y;
            data->pos.z += data->accel.z;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(64) + 128;
            }
            random = fn_1_C378(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = frandmod(60) * random;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the three colored particle models and attach their update callback. */
void fn_1_D444(void) {
    s16 i;

    i = 0;
    while (i < 3) {
        lbl_1_bss_D8[i] = fn_8003216C(HuSprAnimRead(fn_80008DD0(0xA70014, 0x10000000, 2)), 128);
        fn_80026760(lbl_1_bss_D8[i], 0.0f, 0.0f, 0.0f);
        fn_800268B0(lbl_1_bss_D8[i], 1.0f, 1.0f, 1.0f);
        fn_800269E8(lbl_1_bss_D8[i], (1 << 0));
        fn_80026B8C(lbl_1_bss_D8[i], 7);
        fn_800326D0(lbl_1_bss_D8[i], fn_1_CE44);
        i += 1;
    }
}

/* Destroy the three colored particle models. */
void fn_1_D59C(void) {
    s16 i;

    i = 0;
    while (i < 3) {
        fn_800262D0(lbl_1_bss_D8[i]);
        i += 1;
    }
}

void fn_1_D5F4(s16 modelNo, HuVecF *position, s16 display)
{
    GXColor colors[2] = {
        { 255, 178, 110, 0 },
        { 109, 207, 246, 0 },
    };

    if (display == 1 && position) {
        fn_1_C468(modelNo, position, colors);
        fn_1_CCB8(modelNo, position, colors);
    } else if (display == 0) {
        fn_1_C56C(modelNo);
        fn_1_CDF0(modelNo);
    } else if (display == 2) {
        fn_1_C3EC(modelNo, 0);
        fn_800269E8(lbl_1_bss_D8[modelNo], (1 << 0));
    }
}

s16 lbl_1_bss_DE[3];
s16 lbl_1_bss_D8[3];
