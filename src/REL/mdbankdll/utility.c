/* Interpolation, sprite-group attributes and particle effects for the Star Bank. */
#include <math.h>
#include "dolphin.h"
#include "humath.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/frand.h"
#include "game/object.h"
#include "game/window.h"
#include "REL/mdbankdll.h"

extern s16 lbl_1_bss_19E8;
extern s16 lbl_1_bss_19EA;
extern s16 lbl_1_bss_19EC;
extern ANIMDATA *lbl_1_bss_19F0[2];
extern s32 lbl_1_data_A20[2];

/* Interpolates between clamped endpoints using a quarter cosine curve. */
float fn_1_11064(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return (float)(start + ((end - start) *
        (1.0 - cos((M_PI *
         ((90.0f / duration) * time)) / 180.0))));
}

/* Writes the three supplied coordinates to a vector. */
void fn_1_1115C(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

/* Interpolates linearly between clamped endpoints. */
float fn_1_1116C(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

/* Blends a scalar toward its target using the supplied weight. */
float fn_1_111B0(float current, float target, float weight)
{
    if (current == target || weight <= 1.0f) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

/* Blends all three vector coordinates toward a target. */
void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_111B0(current->x, target->x, weight);
    current->y = fn_1_111B0(current->y, target->y, weight);
    current->z = fn_1_111B0(current->z, target->z, weight);
}

/* Adds one full sine oscillation to the starting value over the specified duration. */
float fn_1_11384(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sin((M_PI *
            ((360.0f / durationFrames) * elapsedFrames)) / 180.0));
}

/* Interpolates between clamped endpoints using a quarter sine curve. */
float fn_1_11458(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sin((M_PI *
        ((90.0f / duration) * time)) / 180.0));
}

/* Returns a half sine excursion that starts and ends at the starting value. */
float fn_1_11540(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sin((M_PI *
        ((180.0f / duration) * time)) / 180.0));
}

/* Evaluates one coordinate of a quadratic Bezier curve. */
float fn_1_1161C(float start, float control, float end, float weight)
{
    float inverse = 1.0f - weight;

    return (weight * weight * end)
        + ((inverse * inverse * start)
            + (2.0f * (control * (inverse * weight))));
}

/* Evaluates a three-dimensional quadratic Bezier curve. */
void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight)
{
    out->x = fn_1_1161C(start->x, control->x, end->x, weight);
    out->y = fn_1_1161C(start->y, control->y, end->y, weight);
    out->z = fn_1_1161C(start->z, control->z, end->z, weight);
}

/* Sets the supplied attributes on every member of a sprite group. */
void fn_1_11880(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Clears the supplied attributes on every member of a sprite group. */
void fn_1_11900(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

/* Positions the thirty-two-particle effect model. */
void fn_1_11980(float posX, float posY, float posZ) {
    fn_80026760(lbl_1_bss_19EC, posX, posY, posZ);
}

/* Spawns and updates the orbiting particles, including their scale and opacity cycles. */
void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && spawnCount == 0) {
            spawnCount = 1;
            data->time = 1;
            data->parManId = 0;
            data->vel.x = (float)(frandmod(3) + 1);
            data->vel.y = (float)(frandmod(3) + 1);
            data->vel.z = (float)(frandmod(3) + 1);
            data->accel.x = 0.0f;
            data->accel.y = (float)(frandmod(720) + 360);
            data->accel.z = (float)(frandmod(80) + 100);
            data->accel.x = (float)frandmod(360);
            data->speedDecay = 0.0f;
            data->colorIdx = (float)(frandmod(180) + 60);
            data->speedDecay = (float)frandmod(60);
            data->scale = 10.0f;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
        } else if (data->time == 1) {
            float amount;

            amount = fn_1_1116C(0.0f, 360.0f, data->accel.x, data->accel.y);
            data->pos.x = data->accel.z
                * sin(M_PI
                    * (amount * data->vel.x) / 180.0);
            data->pos.y = data->accel.z
                * cos(M_PI
                    * (amount * data->vel.y) / 180.0);
            data->pos.z = data->accel.z
                * sin(M_PI
                    * (amount * data->vel.z) / 180.0);
            data->scale = (float)(frandmod(30) + 10);
            if (rand8() % 10 == 0) {
                data->zRot = MTXDegToRad((float)frandmod(360));
            }
            data->color.a = (u8)fn_1_11540(0.0f, 255.0f, data->speedDecay, data->colorIdx);
            if ((data->accel.x += 1.0f) > data->accel.y) {
                data->vel.x = (float)(frandmod(4) + 1);
                data->vel.y = (float)(frandmod(4) + 1);
                data->vel.z = (float)(frandmod(4) + 1);
                data->accel.z = (float)(frandmod(80) + 100);
                data->accel.x = 0.0f;
                data->accel.y = (float)(frandmod(720) + 360);
            }
            if ((data->speedDecay += 1.0f) > data->colorIdx) {
                data->speedDecay = 0.0f;
                data->colorIdx = (float)(frandmod(180) + 60);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates the thirty-two-particle effect model and installs its update hook. */
void fn_1_12120(void)
{
    lbl_1_bss_19EC = fn_8003216C(lbl_1_bss_19F0[0], 0x20);
    fn_80026760(lbl_1_bss_19EC, 0.0f, 375.0f, -100.0f);
    fn_800268B0(lbl_1_bss_19EC, 1.0f, 1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EC, 6);
    fn_800326D0(lbl_1_bss_19EC, fn_1_119C4);
    fn_8003269C(lbl_1_bss_19EC, 1);
}

/* Destroys the thirty-two-particle effect model. */
void fn_1_12204(void)
{
    fn_800262D0(lbl_1_bss_19EC);
}

/* Initializes a free particle for a colored scale and opacity pulse. */
void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot == particle->maxCnt) {
        return;
    }
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = 0;
    data->pos.x = pos->x;
    data->pos.y = pos->y;
    data->pos.z = pos->z;
    data->scale = 0.0f;
    data->vel.x = 0.0f;
    data->vel.y = velocityY;
    data->accel.x = accelY;
    data->accel.y = (float)color->a;
}

/* Initializes a free particle for a colored burst with a randomized direction. */
void fn_1_12354(float velocityY, float velocityZ, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 2;
            break;
        }
    }
    if (slot == particle->maxCnt) {
        return;
    }
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = color->a;
    data->pos.x = pos->x;
    data->pos.y = pos->y;
    data->pos.z = pos->z;
    data->vel.x = 0.0f;
    data->vel.y = velocityY;
    data->vel.z = velocityZ;
    data->accel.x = pos->x;
    data->accel.y = pos->y;
    data->accel.z = pos->z;
    data->effectDirection.x = (float)(frandmod(100) - 50);
    data->effectDirection.y = (float)(frandmod(100) - 50);
    data->effectDirection.z = 0.0f;
    PSVECNormalize(&data->effectDirection, &data->effectDirection);
}

/* Updates the pulse and burst particles and flushes their data for rendering. */
void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    float positionScale;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {

            data->scale = fn_1_11458(0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = (u8)fn_1_11540(0.0f, data->accel.y, data->vel.x, data->vel.y);
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        } else if (data->time == 2) {

            positionScale = fn_1_11458(0.0f, data->vel.z, data->vel.x, data->vel.y);
            data->scale = (float)(frandmod(6) + 1);
            data->pos.x = data->accel.x + data->effectDirection.x * positionScale;
            data->pos.y = data->accel.y + data->effectDirection.y * positionScale;
            data->pos.z = data->accel.z + data->effectDirection.z * positionScale;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates the thousand-particle effect model and installs its update hook. */
void fn_1_12984(void)
{
    lbl_1_bss_19EA = fn_8003216C(lbl_1_bss_19F0[1], 1000);
    fn_80026760(lbl_1_bss_19EA, 0.0f,
        0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_19EA, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EA, 6);
    fn_800326D0(lbl_1_bss_19EA, fn_1_124F4);
    fn_8003269C(lbl_1_bss_19EA,
        1);
}

/* Destroys the thousand-particle effect model. */
void fn_1_12A68(void)
{
    fn_800262D0(lbl_1_bss_19EA);
}

/* Positions the four-particle effect model one hundred units beyond the supplied Z. */
void fn_1_12A94(float x, float y, float z)
{
    fn_80026760(lbl_1_bss_19E8, x, y, 100.0f + z);
}

/* Sets the state of every particle in the four-particle effect. */
void fn_1_12AE8(s16 time)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19E8];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = time;
    }
}

/* Randomizes the four-particle effect color and scale according to its current state. */
void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                data->color.r = 0x30;
                data->color.g = 0x30;
                data->color.b = 0x30;
                data->color.a = (u8)(frandmod(24) + 0x30);
                data->scale = (float)(frandmod(100) + 600);
            }
        } else if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                data->color.r = 0x40;
                data->color.g = 0x40;
                data->color.b = 0x40;
                data->color.a = (u8)(frandmod(24) + 0x5C);
                data->scale = (float)(frandmod(100) + 600);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates the four-particle effect model and installs its update hook. */
void fn_1_12D98(void)
{
    lbl_1_bss_19E8 = fn_8003216C(lbl_1_bss_19F0[1], 4);
    fn_80026760(lbl_1_bss_19E8, 0.0f,
        375.0f, 100.0f);
    fn_800268B0(lbl_1_bss_19E8, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19E8, 6);
    fn_800326D0(lbl_1_bss_19E8, fn_1_12B60);
    fn_8003269C(lbl_1_bss_19E8, 1);
}

/* Destroys the four-particle effect model. */
void fn_1_12E7C(void)
{
    fn_800262D0(lbl_1_bss_19E8);
}

/* Loads the two particle animations and creates all three bank particle effects. */
void fn_1_12EA8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_19F0[i] = HuSprAnimRead(fn_80008DD0(
            lbl_1_data_A20[i], 0x10000000, 2));
    }

    fn_1_12120();
    fn_1_12984();
    fn_1_12D98();
}

/* Destroys all three bank particle effect models. */
void fn_1_1317C(void)
{
    fn_1_12204();
    fn_1_12A68();
    fn_1_12E7C();
}

/* Offsets the supplied position and emits a dark pulse with thirty bright burst particles. */
void fn_1_131C8(HuVecF *pos)
{
    GXColor baseColor = {0x28, 0x28, 0x28, 0xFF};
    GXColor brightColor = {0xFF, 0xFF, 0xFF, 0xFF};
    float randomVelocity;
    float randomAccel;
    float randomDirection;
    s16 attempt;

    pos->z += 20.0f;
    randomAccel = (float)(frandmod(300) + 300);
    randomVelocity = (float)(frandmod(6) + 6);

    fn_1_12230(pos, &baseColor, randomVelocity, randomAccel);

    for (attempt = 0; attempt < 30; attempt++) {
        randomDirection = (float)(frandmod(70) + 80);
        fn_1_12354(10.0f, randomDirection, pos, &brightColor);
    }
}

/* Offsets the supplied position and attempts to emit ten gray pulse particles. */
void fn_1_13598(HuVecF *position)
{
    GXColor color = {0x40, 0x40, 0x40, 0x80};
    s16 attempt;

    position->z += 20.0f;
    for (attempt = 0; attempt < 10; ++attempt) {
        float randomAccel = (float)(frandmod(500) + 500);

        fn_1_12230(position, &color, 60.0f, randomAccel);
    }
}

/* Sets the state of the four-particle effect. */
void fn_1_13764(s16 time)
{
    fn_1_12AE8(time);
}
