/* The computer player's steering: it reads its position, picks a state at random and steers. */
#include "dolphin/mtx.h"
#include <math.h>
#include "REL/framework/p_info.h"

#include "REL/m653DLL/car.h"
#include "REL/framework/vector3.h"
#include "REL/m653DLL/boundary_plane.h"
#include "REL/m653DLL/random.h"
#include "REL/m653DLL.h"

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
    : difficulty(upperValue), skill((upperValue + 1) / 4.0f), car(owner), state(0), carIndex(lowerValue)
{
}

void fn_1_2558(Car *owner)
{
    if (owner->work.get() == 0) {
        owner->work.reset(
            new M653SteeringWork(owner->setting->difficulty(), owner->setting->groupNo(), owner));
    }
    fn_1_2A30(owner->work.get());
}

void fn_1_268C(M653SteeringWork *work)
{
    fn_1_1008(work->car, &work->position);
    fn_1_1024(work->car, &work->forward);
    fn_1_1040(work->car, &work->side);
    fn_1_1008(lbl_1_bss_5C[work->carIndex ^ 1], &work->opponentPosition);
    work->position.y = 0.0f;
    work->opponentPosition.y = 0.0f;
    fn_1_2300(&work->forward, &work->forward);
    fn_1_2300(&work->side, &work->side);
    work->wallDistance = fn_1_9D30(work->position.x, work->position.z, &work->wallNormal);
}

void fn_1_2A30(M653SteeringWork *work)
{
    fn_1_268C(work);
    if (work->wallDistance < 200.0f) {
        fn_1_2E60(work, 1);
    }

    switch (work->state) {
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
    work->state = state;
    if (state == 1) {
        work->turnFlag = 0;
        duration = (s32)((1.0f - work->skill) * fn_1_37CC(RandomFloat(0.2f, 0.8f)));
        work->turnTimer.value = duration;
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
        object->stateTimer.value = firstDuration;
        fn_1_2E60(object, 2);
        return;
    }
    if (RandomFloat(0.0f, 1.0f) < 0.5) {
        secondDuration = (s32)fn_1_37CC(RandomFloat(1.0f, 2.0f));
        object->stateTimer.value = secondDuration;
        fn_1_2E60(object, 4);
        return;
    }
    scaledDuration = (s32)(object->skill * fn_1_37CC(RandomFloat(3.0f, 4.0f)));
    object->stateTimer.value = scaledDuration;
    fn_1_2E60(object, 3);
}

void fn_1_3040(M653SteeringWork *state)
{
    Vec targetPosition;
    Vec difference;
    f32 facing;
    f32 fade;

    if (state->wallDistance > 300.0f) {
        fn_1_2E60(state, 0);
        return;
    }

    fn_1_9EF4(state->position.x, state->position.z, &state->wallNormal, 300.0f);
    targetPosition.x = 0.0f;
    targetPosition.z = 243.14915f;
    targetPosition.y = 0.0f;
    PSVECSubtract(&targetPosition, &state->position, &difference);
    fn_1_2300(&difference, &difference);
    fn_1_36CC(state, &targetPosition);

    facing = fn_1_24C4(&state->forward, &state->wallNormal);
    fade = 1.0f - state->skill;
    fade = 1.0f - (fade * fade);
    if (facing > 0.6f) {
        state->car->fn_1_183C(fade, fade);
    }
    if (facing < -0.6f) {
        state->car->fn_1_183C(-fade, -fade);
    }
}

void fn_1_333C(M653SteeringWork *state)
{
    Vec difference;
    f32 facingForThreshold;
    u8 timerExpired = 0;

    if (state->stateTimer.value > 0) {
        if ((state->stateTimer.value -= 1) == 0) timerExpired = 1;
    }
    if (timerExpired != 0) fn_1_2E60(state, 0);

    PSVECSubtract(&state->opponentPosition, &state->position, &difference);
    facingForThreshold = fn_1_24C4(&state->forward, &difference);
    if (facingForThreshold > 0.6f) {
        state->car->fn_1_183C(1.0f, 1.0f);
        return;
    }
    fn_1_36CC(state, &state->opponentPosition);
}

// Reads the car's position and heading; with withSide set, also its side axis, and its distance to
// the nearest boundary (kept in chargeTimer with keepDistance set).
static inline void readCarPose(M653SteeringWork *owner, Vec *position, Vec *direction, s32 withSide,
                               s32 keepDistance)
{
    s32 state;

    fn_1_1008(owner->car, position);
    fn_1_1024(owner->car, direction);
    if (withSide != 0) {
        fn_1_1040(owner->car, direction);
        fn_1_1008(owner->car, position);
        state = (s32)fn_1_9D30(position->x, position->z, direction);
        if (keepDistance != 0) {
            owner->chargeTimer.value = state;
        }
    }
}

void fn_1_3444(M653SteeringWork *owner)
{
    Vector3 direction;
    Vec positionTarget;
    Vec position;
    Vector3 difference;
    f32 unitDistance;
    f32 alignmentThreshold;
    u8 timerExpired = 0;
    s32 cooldownExpired;
    s32 duration;

    if (owner->stateTimer.value > 0) {
        if ((owner->stateTimer.value -= 1) == 0) {
            timerExpired = 1;
        }
    }
    if (timerExpired != 0) {
        owner->state = 0;
    }

    readCarPose(owner, &position, &direction, 0, 1);
    PSVECSubtract(&owner->opponentPosition, &position, &difference);
    difference.y = 0.0f;
    unitDistance = difference.length();
    if (!(unitDistance <= 20.0f)) {
        PSVECScale(&difference, &difference, 1.0f / unitDistance);
        alignmentThreshold = direction.dot(difference);
        if ((alignmentThreshold > 0.5f + 0.48f * owner->skill) || owner->chargeTimer.get() != 0) {
            if (owner->chargeTimer.get() == 0) {
                duration = (s32)fn_1_37CC(RandomFloat(0.5f, 1.0f));
                owner->chargeTimer.value = duration;
            }
            owner->car->fn_1_183C(1.0f, 1.0f);
        } else {
            fn_1_36CC(owner, &positionTarget);
        }

        cooldownExpired = 0;
        if (owner->chargeTimer.value > 0) {
            if ((owner->chargeTimer.value -= 1) == 0) {
                cooldownExpired = 1;
            }
        }
    }
}

void fn_1_364C(M653SteeringWork *work)
{
    u8 finished = 0;
    if (work->stateTimer.value > 0) {
        if (--work->stateTimer.value == 0) {
            finished = 1;
        }
    }
    if (finished != 0) {
        fn_1_2E60(work, 0);
    }
    fn_1_36CC(work, &work->opponentPosition);
}

void fn_1_36CC(M653SteeringWork *source, const Vec *direction)
{
    f32 sourceProjection = fn_1_24C4(&source->side, &source->position);
    f32 directionProjection = fn_1_24C4(&source->side, direction);

    if (directionProjection > sourceProjection) {
        source->car->fn_1_183C(1.0f, -1.0f);
        return;
    }
    source->car->fn_1_183C(-1.0f, 1.0f);
}

f32 fn_1_37CC(f32 value) { return 60.0f * value; }
