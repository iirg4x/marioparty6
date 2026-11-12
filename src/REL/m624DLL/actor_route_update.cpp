#include "REL/m624DLL/player_math.h"
#include "REL/framework/random.h"
#include "REL/m624DLL/player_setup.h"
#include "REL/m624DLL/player_task.h"
#include "REL/m624DLL/tail_types.h"
#include "REL/framework/singleton.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
extern "C" {
#include <math.h>
}
#include <vector>
#include "REL/m624DLL/sequence_manager.h"

ManualControl::~ManualControl() {}

void ManualControl::update() {}

extern "C" {

typedef struct RoutePoint {
    Vec point;
    f32 radius;
    s32 connections[5];
} RoutePoint;
}

u8 fn_1_911C(PlayerRunner *owner);
extern "C" {
extern "C" RoutePoint lbl_1_data_D0[11] = {
    { { -699.26f, 298.29f, -248.6f }, cmToWorld(310.0f), { 1, 1, 2, 2, 3 } },
    { { -763.9f, 222.85f, 212.83f }, cmToWorld(120.0f), { 2, 2, 2, 2, 2 } },
    { { -574.38f, 145.17f, 336.39f }, cmToWorld(120.0f), { 5, 5, 5, 3, 3 } },
    { { -264.08f, 173.17f, 38.19f }, cmToWorld(83.0f), { 4, 4, 4, 4, 4 } },
    { { 36.7f, 220.55f, 5.27f }, cmToWorld(75.0f), { 6, 6, 8, 8, 8 } },
    { { -492.25f, 145.17f, 536.65f }, cmToWorld(60.0f), { 6, 6, 6, 6, 6 } },
    { { 94.77f, 145.79f, 540.38f }, cmToWorld(190.0f), { 7, 7, 7, 7, 8 } },
    { { 399.89f, 135.35f, 457.79f }, cmToWorld(90.0f), { 8, 8, 8, 8, 8 } },
    { { 701.0f, 129.0f, 86.91f }, cmToWorld(250.0f), { 9, 9, 9, 9, 9 } },
    { { 753.25f, 397.95f, -465.7f }, cmToWorld(150.0f), { 10, 10, 10, 10, 10 } },
    { { 800.0f, 525.0f, -900.0f }, cmToWorld(200.0f), { 10, 10, 10, 10, 10 } },
};

}
extern u8 fn_1_174C(ComControl *owner, Vec *result,
                    f32 maxDistance);
extern s32 fn_1_1954(ComControl *owner, const Vec *target,
                    f32 distanceScale);
extern "C" {
static const f32 lbl_1_rodata_98[4] = { 2.0f, 0.8f, 0.2f, 0.0f };
}

void fn_1_158C(ComControl *owner, Vec *position,
               Vec *direction);

static inline void SetRouteActive(ComControl *state, s32 enabled)
{
    state->active = enabled;
}

ComControl::ComControl(PlayerRunner *item)
    : PlayerControl(item), routeSelection(&item->difficulty), currentRoute(0),
      active(1), control(1, 0.0f), countdown(0)
{
}

ComControl::~ComControl() {}

void ComControl::reset()
{
    fn_1_CA90(&this->control, 0.0f);
    this->currentRoute = 0;
    this->countdown = 0;
}

void ComControl::update()
{
    MGPLAYER *actorManager;
    MGACTOR *actor;
    s32 route;
    s32 countdownActive;
    u8 moving;
    u8 *approaching;
    Vec *targetPosition;
    s32 timer;
    f32 movementMagnitude;

    countdownActive = 0;
    if (this->countdown > 0) {
        if ((this->countdown -= 1) == 0) {
            countdownActive = 1;
        }
    }
    if (this->countdown <= 0) {
    actorManager = this->owner->player;
    actor = actorManager->actor;
    Position position =
        this->owner->current_position;
    Position direction;
    Position collisionNormal;
    Position approachDifference;
    Position nearbyPosition;
    Position separationDifference;
    u32 collisionCode;
    direction.value.x = 0.0f;
    direction.value.y = 0.0f;
    direction.value.z = 0.0f;
    fn_1_158C(this, &position.value, &direction.value);
    moving = fn_1_911C(this->owner);
    if (moving != 0) {
        collisionCode = 0;
        fn_80086DD0(actor, &collisionCode);
        if ((collisionCode & 0x80) != 0) {
            this->control.fn_1_CB28();
        } else if (fn_80086D54(actor, &collisionNormal.value) != 0 &&
                   collisionNormal.value.y < cos(M_PI * 10.0 / 180.0)) {
            this->control.fn_1_CB28();
        }
        if (this->active == 0) {
            timer = (s32)secondsToFrames(RandomFloat(lbl_1_rodata_98[*this->routeSelection]));
            this->countdown = timer;
        }
    } else {
        this->control.fn_1_CB38();
    }
    SetRouteActive(this, moving);

    approaching = &this->owner->collisionPending;
    targetPosition = &this->owner->collisionPosition;
    if (*approaching != 0) {
        *approaching = 0;
        PSVECSubtract(&position.value, targetPosition, &approachDifference.value);
        direction += approachDifference.normalize();
    }
    if (fn_1_174C(this, &nearbyPosition.value, 150.0f) != 0) {
        PSVECSubtract(&position.value, &nearbyPosition.value, &separationDifference.value);
        direction += separationDifference.normalize();
    }
    movementMagnitude = PSVECMag(&direction.value);
    if (movementMagnitude > 0.0f) {
        direction.normalize();
        position += (direction *= 100.0f);
        fn_1_1954(this, &position.value, 1.0f);
    }
    fn_1_CD78(&this->control, actorManager);
    }
}

void fn_1_158C(ComControl *owner, Vec *position,
                         Vec *direction)
{
    f32 distance;
    s32 routeChoice;
    RoutePoint *route;

    if (owner->currentRoute < 11) {
        route = &lbl_1_data_D0[owner->currentRoute];
        PSVECSubtract(&route->point, position, direction);
        if (fn_1_911C(owner->owner) != 0) {
            distance = PSVECMag(direction);
            if (distance < route->radius) {
                if (RandomFloat() < 0.5) {
                    routeChoice = 0;
                } else {
                    routeChoice = 1;
                }
                owner->currentRoute = route->connections[
                    *owner->routeSelection + routeChoice];
            }
        }
        if (Square(direction->x) + Square(direction->z) < Square(0.5 * route->radius)) {
            direction->x = 0.0f;
            direction->y = 0.0f;
            direction->z = 0.0f;
            return;
        }
        PSVECNormalize(direction, direction);
    }
}

class SequenceActor;
Vec *fn_1_22F0(SequenceActor *, Vec *);
std::vector<void *> *fn_1_2264(ActorSequence *);

u8 fn_1_174C(ComControl *owner, Vec *result, f32 maximumDistance)
{
    std::vector<void *> *array;
    void * * begin;
    void * * current;
    void * candidate;
    Vec candidatePosition;
    Vec difference;
    s32 matchCount;
    Vec * actorPosition;
    f64 horizontalDelta;
    f64 depthDelta;
    f32 distance;
    u8 found;
    array = fn_1_2264(&ActorSequence::getChecked());
    current = array->begin();
    begin = array->end();
    found = 0;
    matchCount = 0;
    actorPosition = & owner->owner->current_position.value;
    result->x = 0.0f;
    result->y = 0.0f;
    result->z = 0.0f;
    while (current != begin)
    {
        candidate = fn_1_5E24(* current);
        fn_1_22F0(static_cast<SequenceActor *>(candidate), & candidatePosition);
        depthDelta = candidatePosition.z - actorPosition->z;
        horizontalDelta = candidatePosition.x - actorPosition->x;
        difference.x = horizontalDelta;
        difference.y = 0.0f;
        difference.z = depthDelta;
        distance = PSVECMag(& difference);
        if (distance <= maximumDistance)
        {
            PSVECAdd(& candidatePosition, result, result);
            matchCount++;
            found = 1;
        }
        current++;
    }
    if (matchCount > 0)
    {
        PSVECScale(result, result, (f32)(1.0 / (f64) matchCount));
    }
    return found;
}

s32 fn_1_1954(ComControl *owner, const Vec *target,
              f32 distanceScale)
{
    f32 clampedScale;
    f32 maximumDistance;
    f32 distance;
    f64 horizontalDelta;
    f64 verticalDelta;
    f32 magnitude;
    Vec direction;
    Vec *currentPosition;

    if (distanceScale < 0.0f) {
        clampedScale = 0.0f;
    } else if (distanceScale > 1.0f) {
        clampedScale = 1.0f;
    } else {
        clampedScale = distanceScale;
    }
    maximumDistance = 10.0f * clampedScale;
    currentPosition = &owner->owner->current_position.value;
    verticalDelta = target->z - currentPosition->z;
    horizontalDelta = target->x - currentPosition->x;
    direction.x = horizontalDelta;
    direction.y = 0.0f;
    direction.z = verticalDelta;
    magnitude = PSVECMag(&direction);
    distance = magnitude;
    if (distance < maximumDistance) {
        return 1;
    }
    PSVECScale(&direction, &direction, maximumDistance / distance);
    fn_1_CB48(&owner->control, &direction, 0.1f);
    return 0;
}

void PlayerControl::reset() {}
