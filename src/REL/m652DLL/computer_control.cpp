#include "REL/framework/singleton.h"
#include "REL/m652DLL/player_functions.h"
#include "REL/m652DLL/player_work.h"
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/m652DLL/camera_arc_motion.h"
#include "REL/m652DLL/man_map.h"
#include "REL/framework/random.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/sphere.h"
#include <vector>
#include <cmath>
#include "dolphin/types.h"

extern "C" {
#include "dolphin.h"
#include "humath.h"
#include "game/hu3d.h"
#include "game/mg/timer.h"
}

using std::cos;

/* One leg of a computer player's route. */
struct DifficultyParameter {
    float minimumRotation;
    float maximumRotation;
    float collisionAngle;
    float collisionDelay;
    float value;
    s32 stepLimit;
};

#include "REL/m652DLL/racer_control.h"
extern "C" {

RouteStep lbl_1_data_0[2] = { { 0, 6, 9, cmToWorld(-1800.0f) }, { 1, 7, 11, cmToWorld(-4300.0f) } };
const DifficultyParameter lbl_1_rodata_10[] = {
    {1.2f, 2.2f, 40.0f, 1.0f, 1.5f, 5},
    {1.2f, 2.0f, 38.0f, 0.8f, 0.8f, 3},
    {1.0f, 1.6f, 35.0f, 0.6f, 0.4f, 2},
    {1.0f, 1.4f, 30.0f, 0.3f, 0.1f, 0}
};

}

void fn_1_48C(ComputerControl *owner);
void fn_1_990(ComputerControl *work, const HuVecF *position, HuVecF *delta);
s32 fn_1_CBC(ComputerControl *work, const HuVecF *target, float amount);

/* Releases the object through its member and base destructors. */
inline RacerControl::~RacerControl() {}

/* Releases the object through its member and base destructors. */
HumanControl::~HumanControl() {}

/* Initializes the computer player's difficulty, route, timers and pad control. */
ComputerControl::ComputerControl(Racer *racer)
    : RacerControl(racer), difficulty(racer->difficulty()),
      resetCount(-1), currentStep(0), nextStep(0),
      stepDistanceFactor(1.4f), collisionTimer(0), timer(0), moving(1), steps(2),
      padControl((u16)this->racer->playerMask, 3.1415927f)
{
}

/* Releases the object through its member and base destructors. */
ComputerControl::~ComputerControl() {}

/* Resets pad control and timers, then selects or advances the route for this difficulty. */
void ComputerControl::reset()
{
    const s32 *maximum;
    int entryIndex;
    int activeFlag;

    fn_1_ABA8(&padControl, 3.1415927f);
    if (racer->flags.collision4 != 0) {
        activeFlag = 1;
    } else {
        activeFlag = 0;
    }
    currentStep = activeFlag;
    nextStep = 0;
    collisionTimer = 0;
    fn_1_48C(this);
    steps.clear();

    if (difficulty >= 3) {
        for (entryIndex = 0; entryIndex < 2; entryIndex++) {
            const RouteStep *entry = &lbl_1_data_0[entryIndex];
            if (currentStep <= entry->firstPoint && RandomFloat() < 0.6) {
                steps.push_back(*entry);
            }
        }
    } else {
        maximum = &lbl_1_rodata_10[difficulty].stepLimit;
        if (++resetCount >= *maximum) {
            difficulty = difficulty + 1;
        }
    }
}

/* Chooses a random movement timer from the current difficulty parameters. */
void fn_1_48C(ComputerControl *owner)
{
    s32 convertedValue;

    convertedValue = (s32)RandomFloat(secondsToFrames(lbl_1_rodata_10[owner->difficulty].value));
    owner->timer = convertedValue;
}

/* Chooses a random collision timer from the supplied duration in seconds. */
static inline void chooseCollisionFrames(ComputerControl *object, const f32 *angle)
{
    s32 frames;
    frames = (s32)RandomFloat(secondsToFrames(*angle));
    object->collisionTimer = frames;
}

/* Sets the collision timer from a duration in seconds. */
static inline void setCollisionSeconds(ComputerControl *object, f64 value)
{
    s32 frames = (s32)secondsToFrames(value);
    object->collisionTimer = frames;
}

/* Reports whether the route is on its initial nonzero step. */
static inline void isInitialRouteStep(ComputerControl *object, bool *reset)
{
    *reset = object->currentStep != 0 && object->nextStep == 0;
}

/* The unit direction's dot product with the collision normal. */
static inline f32 normalizedCollisionDot(Point3d *normal, const Point3d *collisionNormal)
{
    PSVECNormalize(normal, normal);
    return PSVECDotProduct(normal, collisionNormal);
}

/* Tests the collision normal against the angle and normalized movement direction. */
static inline void testMovementCollision(Point3d *normal, Point3d *collisionNormal, f32 radians, bool *hit)
{
    *hit = collisionNormal->y < cos(radians) && normalizedCollisionDot(normal, collisionNormal) < 0.0f;
}

/* Counts a timer down; true on the tick that reaches zero. */
static inline bool tickPositiveCounter(s32 *counter)
{
    return *counter > 0 && --*counter == 0;
}

/* Stores whether the computer player is moving. */
static inline void setMoving(ComputerControl *object, u32 moving)
{
    object->moving = moving;
}

/* Advance the movement timers and apply collision or path transitions. */
void ComputerControl::update()
{
    f32 radians;
    bool targetMatches;
    MGPLAYER *owner;
    u8 moving;
    const f32 *threshold;
    const f32 *angle;
    MGACTOR *actor;
    bool hit;
    bool reset;
    tickPositiveCounter(&this->timer);
    if (this->timer <= 0) {
        owner = racer->player;
        actor = owner->actor;
        Point3d position = racer->position;
        Point3d direction;
        Point3d collisionNormal;
        targetMatches = !steps.empty();
        if (targetMatches != 0) {
            std::vector<RouteStep>::iterator target = steps.begin();
            if (currentStep != target->firstPoint || nextStep != target->secondPoint) {
                targetMatches = 0;
            }
        }
        if (targetMatches == 0) {
            fn_1_990(this, &position, &direction);
        } else {
            f64 fallbackZ = -600.0;
            direction.x = 0.0f;
            direction.y = 0.0f;
            direction.z = fallbackZ;
        }
        moving = fn_1_6FAC(racer);
        if (moving != 0) {
            if (tickPositiveCounter(&collisionTimer)) padControl.fn_1_AC40();
            if (collisionTimer <= 0 && fn_80086D54(actor, &collisionNormal) != 0) {
                threshold = &lbl_1_rodata_10[difficulty].collisionAngle;
                angle = &lbl_1_rodata_10[difficulty].collisionDelay;
                Point3d normal = {0.0f, 0.0f, 0.0f};
                normal.x = direction.x;
                normal.z = direction.z;
                radians = M_PI * *threshold / 180.0;
                testMovementCollision(&normal, &collisionNormal, radians, &hit);
                isInitialRouteStep(this, &reset);
                if (hit != 0) chooseCollisionFrames(this, angle);
                if (reset != 0) collisionTimer = 1;
                if (targetMatches != 0) {
                    setCollisionSeconds(this, 0.2);
                }
                if (difficulty >= 2 && fn_1_7020(racer) != 0) {
                    collisionTimer = 0;
                    padControl.fn_1_AC40();
                }
            }
            if (targetMatches != 0) {
                std::vector<RouteStep>::iterator target = steps.begin();
                if (position.z < target->endZ) {
                    nextStep = target->nextPoint;
                    steps.erase(target);
                }
            }
            if (this->moving == 0) fn_1_48C(this);
        } else {
            padControl.fn_1_AC50();
        }
        setMoving(this, moving);
        if (vectorMagnitude(&direction) > 0.0f) {
            PSVECAdd(&direction, &position, &position);
            fn_1_CBC(this, &position, 1.0f);
        }
        padControl.fn_1_AE90(owner);
    }
}

/* Scales the route distance factor by one hundred. */
static inline double routeStepDistance(double stepDistanceFactor)
{
    return 100.0 * stepDistanceFactor;
}

/* Selects a route target and computes the direction toward it, advancing completed steps. */
void fn_1_990(ComputerControl *work, const HuVecF *position, HuVecF *delta)
{
    HuVecF target;
    const float *waypointX;
    const float *waypointY;
    HuVecF *fallbackTarget;
    unsigned char hasTarget;
    unsigned int waypointIndex;
    float speed;

    static HuVecF fallbackTargets[2] = {
        {cmToWorld(0.0f), cmToWorld(362.0f), cmToWorld(-2700.0f)},
        {cmToWorld(0.0f), cmToWorld(280.0f), cmToWorld(-5100.0f)}
    };

    waypointX = &lbl_1_rodata_10[work->difficulty].minimumRotation;
    waypointY = &lbl_1_rodata_10[work->difficulty].maximumRotation;
    if ((double)RandomFloat((float)secondsToFrames(0.4)) <
        1.0) {
        work->stepDistanceFactor = RandomFloat(*waypointX, *waypointY);
    }

    hasTarget = fn_1_5190(&MinigameSingleton<MapManager>::getChecked(), work->currentStep,
                          work->nextStep, &target);
    speed = (float)routeStepDistance(hasTarget != 0 ? work->stepDistanceFactor : 3.5);

    if (hasTarget == 0) {
        int selector = work->currentStep;
        if (selector < 0) {
            waypointIndex = 0;
        } else if (selector > 1) {
            waypointIndex = 1;
        } else {
            waypointIndex = (unsigned int)selector;
        }
        fallbackTarget = fallbackTargets + waypointIndex;
        target.x = fallbackTarget->x;
        target.y = fallbackTarget->y;
        target.z = fallbackTarget->z;
    }

    PSVECSubtract(&target, position, delta);
    HuVecF difference = {0.0f, 0.0f, 0.0f};
    difference.x = delta->x;
    difference.z = delta->z;
    if (vectorMagnitude(&difference) < speed) {
        if (hasTarget != 0) {
            work->nextStep++;
        } else {
            work->currentStep++;
            work->nextStep = 0;
        }
    }
}

/* Nothing in the module refers to this four-byte object, aligned to four.
 * Its type is not shown. */
int lbl_1_bss_14;

/* Moves pad control toward the target with a clamped movement amount. */
s32 fn_1_CBC(ComputerControl *work, const HuVecF *target, float amount)
{
    float clampedAmount;
    float maxDistance;
    float distance;
    double horizontalDelta;
    double verticalDelta;
    float magnitude;
    HuVecF delta;
    HuVecF *currentPosition;

    if (amount < 0.0f) {
        clampedAmount = 0.0f;
    } else if (amount > 1.0f) {
        clampedAmount = 1.0f;
    } else {
        clampedAmount = amount;
    }
    maxDistance = 10.0f * clampedAmount;
    currentPosition = &work->racer->position;
    verticalDelta = target->z - currentPosition->z;
    horizontalDelta = target->x - currentPosition->x;
    delta.x = horizontalDelta;
    delta.y = 0.0f;
    delta.z = verticalDelta;
    magnitude = PSVECMag(&delta);
    distance = magnitude;
    if (distance < maxDistance) {
        return 1;
    }
    PSVECScale(&delta, &delta, maxDistance / distance);
    work->padControl.fn_1_AC60(&delta, 0.1f);
    return 0;
}

/* Returns without changing state. */
void RacerControl::reset() {}
/* Returns without changing state. */
void HumanControl::update() {}
