/* The computer player's steering: it reads its position, picks a state at random and steers. */
#include "dolphin/mtx.h"
#include <math.h>
#include "REL/framework/p_info.h"

#include "REL/m653DLL/car.h"
typedef Car M653SteeringOwner;
extern "C" {
extern Car *lbl_1_bss_5C[2];

}
f32 fn_1_9D30(f32, f32, Vec *);
s32 fn_1_9EF4(f32 x, f32 z, Point3d *result, f32 threshold);
f32 fn_1_37CC(f32);

extern "C" {
f32 fn_1_2300(const Vec *source, Vec *destination);
f32 fn_1_24C4(const Vec *left, const Vec *right);
void fn_1_2558(M653SteeringOwner *owner);
void fn_1_268C(M653SteeringWork *work);
void fn_1_2A30(M653SteeringWork *work);
void fn_1_2E60(M653SteeringWork *work, s32 state);
void fn_1_2EF8(M653SteeringWork *object);
void fn_1_3040(M653SteeringWork *state);
void fn_1_333C(M653SteeringWork *state);
void fn_1_3444(M653SteeringWork *owner);
void fn_1_364C(M653SteeringWork *work);
void fn_1_36CC(M653SteeringWork *source, const Vec *direction);
}

float RandomFloat(float from, float to);

f32 fn_1_2300(const Vec *source, Vec *destination)
{
    float x = source->x;
    float z = source->z;
    float length = sqrtf(x * x + z * z);

    if (length > 0.0f) {
        float scale = 1.0f / length;
        x = x * scale;
        z = z * scale;
    }

    destination->x = x;
    destination->y = 0.0f;
    destination->z = z;
    return length;
}

f32 fn_1_24C4(const Vec *left, const Vec *right)
{
    return left->x * right->x + left->z * right->z;
}

M653SteeringWork::M653SteeringWork(s32 upperValue, s32 lowerValue, Car *owner)
    : value_00(upperValue), scale_04((upperValue + 1) / 4.0f), owner_08(owner), state_5C(0), state_60(lowerValue)
{
}

void fn_1_2558(M653SteeringOwner *owner)
{
    if (owner->work.get() == 0) {
        owner->work.reset(new M653SteeringWork(owner->workSetting()->difficulty(), owner->workSetting()->groupNo(), owner));
    }
    fn_1_2A30(owner->work.get());
}

void fn_1_268C(M653SteeringWork *work)
{
    f32 firstInverse;
    f32 firstLength;
    f32 secondInverse;
    f32 secondLength;
    f32 firstZ;
    f32 firstX;
    f32 secondZ;
    f32 secondX;
    f32 firstLengthSquared;
    f32 secondLengthSquared;

    fn_1_1008(work->owner_08, &work->position_0C);
    fn_1_1024(work->owner_08, &work->direction_24);
    fn_1_1040(work->owner_08, &work->direction_30);
    fn_1_1008(lbl_1_bss_5C[work->state_60 ^ 1], &work->target_18);
    work->position_0C.y = 0.0f;
    work->target_18.y = 0.0f;

    firstX = work->direction_24.x;
    firstZ = work->direction_24.z;
    firstLengthSquared = firstX * firstX + firstZ * firstZ;
    firstLength = sqrtf(firstLengthSquared);
    if (firstLength > 0.0f) {
        firstInverse = 1.0f / firstLength;
        firstX *= firstInverse;
        firstZ *= firstInverse;
    }
    work->direction_24.x = firstX;
    work->direction_24.y = 0.0f;
    work->direction_24.z = firstZ;

    secondX = work->direction_30.x;
    secondZ = work->direction_30.z;
    secondLengthSquared = secondX * secondX + secondZ * secondZ;
    secondLength = sqrtf(secondLengthSquared);
    if (secondLength > 0.0f) {
        secondInverse = 1.0f / secondLength;
        secondX *= secondInverse;
        secondZ *= secondInverse;
    }
    work->direction_30.x = secondX;
    work->direction_30.y = 0.0f;
    work->direction_30.z = secondZ;
    work->distance_48 = fn_1_9D30(work->position_0C.x, work->position_0C.z, &work->normal_3C);
}

static inline void readSteeringFrame(M653SteeringWork *work)
{
    f32 secondLength;
    f32 secondInverse;
    f32 firstLength;
    f32 firstInverse;
    f32 secondX;
    f32 secondZ;
    f32 firstX;
    f32 firstZ;

    fn_1_1008(work->owner_08, &work->position_0C);
    fn_1_1024(work->owner_08, &work->direction_24);
    fn_1_1040(work->owner_08, &work->direction_30);
    fn_1_1008(lbl_1_bss_5C[work->state_60 ^ 1], &work->target_18);
    work->position_0C.y = 0.0f;
    work->target_18.y = 0.0f;

    firstX = work->direction_24.x;
    firstZ = work->direction_24.z;
    firstLength = sqrtf(firstX * firstX + firstZ * firstZ);
    if (firstLength > 0.0f) {
        firstInverse = 1.0f / firstLength;
        firstX *= firstInverse;
        firstZ *= firstInverse;
    }
    work->direction_24.x = firstX;
    work->direction_24.y = 0.0f;
    work->direction_24.z = firstZ;

    secondX = work->direction_30.x;
    secondZ = work->direction_30.z;
    secondLength = sqrtf(secondX * secondX + secondZ * secondZ);
    if (secondLength > 0.0f) {
        secondInverse = 1.0f / secondLength;
        secondX *= secondInverse;
        secondZ *= secondInverse;
    }
    work->direction_30.x = secondX;
    work->direction_30.y = 0.0f;
    work->direction_30.z = secondZ;
    work->distance_48 = fn_1_9D30(work->position_0C.x, work->position_0C.z, &work->normal_3C);
}

void fn_1_2A30(M653SteeringWork *work)
{
    readSteeringFrame(work);
    if (work->distance_48 < 200.0f) {
        fn_1_2E60(work, 1);
    }

    switch (work->state_5C) {
    case 0:
        fn_1_2EF8(work);
        break;
    case 1:
        fn_1_3040(work);
        break;
    case 2:
        fn_1_333C(work);
        break;
    case 3:
        fn_1_3444(work);
        break;
    case 4:
        fn_1_364C(work);
        break;
    }
}

void fn_1_2E60(M653SteeringWork *work, s32 state)
{
    s32 duration;
    work->state_5C = state;
    if (state == 1) {
        work->flag_58 = 0;
        duration = (s32)((1.0f - work->scale_04) * fn_1_37CC(RandomFloat(0.2f, 0.8f)));
        work->duration_54.value = duration;
    }
}

/* Selects a random duration and mode, using the object's scale for the default case. */
void fn_1_2EF8(M653SteeringWork *object)
{
    s32 firstDuration;
    s32 secondDuration;
    s32 scaledDuration;

    if (RandomFloat(0.0f, 1.0f) < 0.5) {
        firstDuration = (s32)fn_1_37CC(RandomFloat(0.5f, 1.5f));
        object->timer_4C.value = firstDuration;
        fn_1_2E60(object, 2);
        return;
    }
    if (RandomFloat(0.0f, 1.0f) < 0.5) {
        secondDuration = (s32)fn_1_37CC(RandomFloat(1.0f, 2.0f));
        object->timer_4C.value = secondDuration;
        fn_1_2E60(object, 4);
        return;
    }
    scaledDuration = (s32)(object->scale_04 * fn_1_37CC(RandomFloat(3.0f, 4.0f)));
    object->timer_4C.value = scaledDuration;
    fn_1_2E60(object, 3);
}

static inline void normalizeHorizontal3040(Vec *vector)
{
    f32 length;
    f32 inverseLength;
    f32 x;
    f32 z;

    x = vector->x;
    z = vector->z;
    length = sqrtf(x * x + z * z);
    if (length > 0.0f) {
        inverseLength = 1.0f / length;
        x *= inverseLength;
        z *= inverseLength;
    }
    vector->x = x;
    vector->y = 0.0f;
    vector->z = z;
}

static inline f32 boundaryFacing3040(const M653SteeringWork *state)
{
    return state->direction_24.x * state->normal_3C.x + state->direction_24.z * state->normal_3C.z;
}

void fn_1_3040(M653SteeringWork *state)
{
    Vec targetPosition;
    Vec difference;
    f32 facing;
    f32 fade;

    if (state->distance_48 > 300.0f) {
        fn_1_2E60(state, 0);
        return;
    }

    fn_1_9EF4(state->position_0C.x, state->position_0C.z, &state->normal_3C, 300.0f);
    targetPosition.x = 0.0f;
    targetPosition.z = 243.14915f;
    targetPosition.y = 0.0f;
    PSVECSubtract(&targetPosition, &state->position_0C, &difference);
    normalizeHorizontal3040(&difference);
    fn_1_36CC(state, &targetPosition);

    facing = boundaryFacing3040(state);
    fade = 1.0f - state->scale_04;
    fade = 1.0f - (fade * fade);
    if (facing > 0.6f) {
        state->owner_08->fn_1_183C(fade, fade);
    }
    if (facing < -0.6f) {
        state->owner_08->fn_1_183C(-fade, -fade);
    }
}

static inline f32 boundaryFacing333C(const M653SteeringWork *state, const Vec *difference)
{
    return state->direction_24.x * difference->x + state->direction_24.z * difference->z;
}

void fn_1_333C(M653SteeringWork *state)
{
    Vec difference;
    f32 facingForThreshold;
    u8 timerExpired = 0;

    if (state->timer_4C.value > 0) {
        if ((state->timer_4C.value -= 1) == 0) timerExpired = 1;
    }
    if (timerExpired != 0) fn_1_2E60(state, 0);

    PSVECSubtract(&state->target_18, &state->position_0C, &difference);
    facingForThreshold = boundaryFacing333C(state, &difference);
    if (facingForThreshold > 0.6f) {
        state->owner_08->fn_1_183C(1.0f, 1.0f);
        return;
    }
    fn_1_36CC(state, &state->target_18);
}

static inline f32 vectorMagnitude3444(const Vec *vector)
{
    return PSVECMag(vector);
}

static inline f32 vectorDot3444(const Vec *left, const Vec *right)
{
    return PSVECDotProduct(left, right);
}

static inline s32 steeringState3444(const M653SteeringWork *owner)
{
    return owner->state_50.value;
}

static inline void updateProjection3444(M653SteeringWork *owner, Vec *position, Vec *direction, s32 flag, s32 motion)
{
    s32 state;

    fn_1_1008(owner->owner_08, position);
    fn_1_1024(owner->owner_08, direction);
    if (flag != 0) {
        fn_1_1040(owner->owner_08, direction);
        fn_1_1008(owner->owner_08, position);
        state = (s32)fn_1_9D30(position->x, position->z, direction);
        if (motion != 0) {
            owner->state_50.value = state;
        }
    }
}

void fn_1_3444(M653SteeringWork *owner)
{
    Vec direction;
    Vec positionTarget;
    Vec position;
    Vec difference;
    f32 unitDistance;
    f32 alignmentThreshold;
    u8 timerExpired = 0;
    s32 cooldownExpired;
    s32 duration;

    if (owner->timer_4C.value > 0) {
        if ((owner->timer_4C.value -= 1) == 0) {
            timerExpired = 1;
        }
    }
    if (timerExpired != 0) {
        owner->state_5C = 0;
    }

    updateProjection3444(owner, &position, &direction, 0, 1);
    PSVECSubtract(&owner->target_18, &position, &difference);
    difference.y = 0.0f;
    unitDistance = vectorMagnitude3444(&difference);
    if (!(unitDistance <= 20.0f)) {
        PSVECScale(&difference, &difference, 1.0f / unitDistance);
        alignmentThreshold = vectorDot3444(&direction, &difference);
        if ((alignmentThreshold > 0.5f + 0.48f * owner->scale_04) || steeringState3444(owner) != 0) {
            if (steeringState3444(owner) == 0) {
                duration = (s32)fn_1_37CC(RandomFloat(0.5f, 1.0f));
                owner->state_50.value = duration;
            }
            owner->owner_08->fn_1_183C(1.0f, 1.0f);
        } else {
            fn_1_36CC(owner, &positionTarget);
        }

        cooldownExpired = 0;
        if (owner->state_50.value > 0) {
            if ((owner->state_50.value -= 1) == 0) {
                cooldownExpired = 1;
            }
        }
    }
}

void fn_1_364C(M653SteeringWork *work)
{
    u8 finished = 0;
    if (work->timer_4C.value > 0) {
        if (--work->timer_4C.value == 0) {
            finished = 1;
        }
    }
    if (finished != 0) {
        fn_1_2E60(work, 0);
    }
    fn_1_36CC(work, &work->target_18);
}

void fn_1_36CC(M653SteeringWork *source, const Vec *direction)
{
    f32 sourceProjection;
    f32 directionProjection;
    f32 projectedSource;
    f32 projectedDirection;

    projectedSource = source->direction_30.x * source->position_0C.x + source->direction_30.z * source->position_0C.z;
    sourceProjection = projectedSource;
    projectedDirection = source->direction_30.x * direction->x + source->direction_30.z * direction->z;
    directionProjection = projectedDirection;

    if (directionProjection > sourceProjection) {
        source->owner_08->fn_1_183C(1.0f, -1.0f);
        return;
    }
    source->owner_08->fn_1_183C(-1.0f, 1.0f);
}

float fn_1_37CC(float value) { return 60.0f * value; }
