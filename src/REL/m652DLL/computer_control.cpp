#include "REL/m652DLL/map_singleton.h"
#include "REL/framework/singleton.h"
#include "REL/m652DLL/player_functions.h"
#include "REL/m652DLL/player_work.h"
#include "REL/m652DLL/tail_types.h"
void fn_1_ABA8(CameraArcMotion *object, float value);
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
class MapManager;
#include <vector>
#include <cmath>
#include "dolphin/types.h"

extern "C" {
#include "dolphin.h"
#include "humath.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
}

#include "REL/m652DLL/camera_arc_motion.h"

float RandomFloat(float minimum, float maximum);

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

/* What drives a racer; the classes' names are reconstructed. */
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
float RandomFloat();
float RandomFloat(float maximum);

void fn_1_48C(ComputerControl *owner);
void fn_1_990(ComputerControl *work, const HuVecF *position, HuVecF *delta);
s32 fn_1_CBC(ComputerControl *work, const HuVecF *target, float amount);

inline RacerControl::~RacerControl() {}

HumanControl::~HumanControl() {}

ComputerControl::ComputerControl(Racer *racer)
    : RacerControl(racer), index(racer->difficulty()),
      unknown0C(-1), currentStep(0), nextStep(0),
      rotation(1.4f), collisionTimer(0), timer(0), moving(1), steps(2),
      padControl((u16)this->racer->playerMask, 3.1415927f)
{
}

ComputerControl::~ComputerControl() {}

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

    if (index >= 3) {
        for (entryIndex = 0; entryIndex < 2; entryIndex++) {
            const RouteStep *entry = &lbl_1_data_0[entryIndex];
            if (currentStep <= entry->firstPoint && RandomFloat() < 0.6) {
                steps.push_back(*entry);
            }
        }
    } else {
        maximum = &lbl_1_rodata_10[index].stepLimit;
        if (++unknown0C >= *maximum) {
            index = index + 1;
        }
    }
}

void fn_1_48C(ComputerControl *owner)
{
    s32 convertedValue;

    convertedValue = (s32)RandomFloat(secondsToFrames(lbl_1_rodata_10[owner->index].value));
    owner->timer = convertedValue;
}

static inline void RandomFrames4EC(ComputerControl *object, const f32 *angle)
{
    s32 frames;
    frames = (s32)RandomFloat(secondsToFrames(*angle));
    object->collisionTimer = frames;
}

static inline void timer48C(ComputerControl *object)
{
    s32 frames;
    frames = (s32)RandomFloat(secondsToFrames(lbl_1_rodata_10[object->index].value));
    object->timer = frames;
}

static inline f32 Dot4EC(const Point3d *a, const Point3d *b)
{
    return PSVECDotProduct(a, b);
}

static inline f32 Magnitude4EC(const Point3d *value)
{
    return PSVECMag(value);
}

static inline void Frames4EC(ComputerControl *object, f64 value)
{
    s32 frames = (s32)secondsToFrames(value);
    object->collisionTimer = frames;
}

static inline void InitialStep4EC(ComputerControl *object, bool *reset)
{
    *reset = object->currentStep != 0 && object->nextStep == 0;
}

/* The unit direction's dot product with the collision normal. */
static inline f32 NormalizedDot4EC(Point3d *normal, const Point3d *collisionNormal)
{
    PSVECNormalize(normal, normal);
    return Dot4EC(normal, collisionNormal);
}

static inline void Collision4EC(Point3d *normal, Point3d *collisionNormal, f32 radians, bool *hit)
{
    *hit = collisionNormal->y < cos(radians) && NormalizedDot4EC(normal, collisionNormal) < 0.0f;
}

/* Counts a timer down; true on the tick that reaches zero. */
static inline bool Tick4EC(s32 *counter)
{
    return *counter > 0 && --*counter == 0;
}

static inline void SetMoving4EC(ComputerControl *object, u32 moving)
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
    Tick4EC(&this->timer);
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
            if (Tick4EC(&collisionTimer)) padControl.fn_1_AC40();
            if (collisionTimer <= 0 && fn_80086D54(actor, &collisionNormal) != 0) {
                threshold = &lbl_1_rodata_10[index].collisionAngle;
                angle = &lbl_1_rodata_10[index].collisionDelay;
                Point3d normal = {0.0f, 0.0f, 0.0f};
                normal.x = direction.x;
                normal.z = direction.z;
                radians = M_PI * *threshold / 180.0;
                Collision4EC(&normal, &collisionNormal, radians, &hit);
                InitialStep4EC(this, &reset);
                if (hit != 0) RandomFrames4EC(this, angle);
                if (reset != 0) collisionTimer = 1;
                if (targetMatches != 0) {
                    Frames4EC(this, 0.2);
                }
                if (index >= 2 && fn_1_7020(racer) != 0) {
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
            if (this->moving == 0) timer48C(this);
        } else {
            padControl.fn_1_AC50();
        }
        SetMoving4EC(this, moving);
        if (Magnitude4EC(&direction) > 0.0f) {
            PSVECAdd(&direction, &position, &position);
            fn_1_CBC(this, &position, 1.0f);
        }
        padControl.fn_1_AE90(owner);
    }
}

extern "C" {

// Route initialization state.
signed char lbl_1_bss_10[0x8];
}
template<> MapManager *MinigameSingleton<MapManager>::self = 0;
extern "C" {

HuVecF lbl_1_data_20[2] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

}
unsigned char fn_1_5190(MapManager *owner, int currentStep, int nextStep, Vec *position);

static inline double StepDistance990(double rotation)
{
    return 100.0 * rotation;
}

static inline float Magnitude990(const HuVecF *value)
{
    return PSVECMag(value);
}

void fn_1_990(ComputerControl *work, const HuVecF *position, HuVecF *delta)
{
    HuVecF target;
    const float *waypointX;
    const float *waypointY;
    HuVecF *fallbackTarget;
    unsigned char hasTarget;
    unsigned int waypointIndex;
    float speed;

    if (lbl_1_bss_10[0] == 0) {
        lbl_1_data_20[0].x = 0.0f;
        lbl_1_data_20[0].y = 362.0f;
        lbl_1_data_20[0].z = -2700.0f;
        lbl_1_data_20[1].x = 0.0f;
        lbl_1_data_20[1].y = 280.0f;
        lbl_1_data_20[1].z = -5100.0f;
        lbl_1_bss_10[0] = 1;
    }

    waypointX = &lbl_1_rodata_10[work->index].minimumRotation;
    waypointY = &lbl_1_rodata_10[work->index].maximumRotation;
    if ((double)RandomFloat((float)secondsToFrames(0.4)) <
        1.0) {
        work->rotation = RandomFloat(*waypointX, *waypointY);
    }

    hasTarget = fn_1_5190(&MinigameSingleton<MapManager>::getChecked(), work->currentStep,
                          work->nextStep, &target);
    speed = (float)StepDistance990(hasTarget != 0 ? work->rotation : 3.5);

    if (hasTarget == 0) {
        int selector = work->currentStep;
        if (selector < 0) {
            waypointIndex = 0;
        } else if (selector > 1) {
            waypointIndex = 1;
        } else {
            waypointIndex = (unsigned int)selector;
        }
        fallbackTarget = lbl_1_data_20 + waypointIndex;
        target.x = fallbackTarget->x;
        target.y = fallbackTarget->y;
        target.z = fallbackTarget->z;
    }

    PSVECSubtract(&target, position, delta);
    {
        HuVecF difference = {0.0f, 0.0f, 0.0f};
        difference.x = delta->x;
        difference.z = delta->z;
        if (Magnitude990(&difference) < speed) {
            if (hasTarget != 0) {
                work->nextStep++;
            } else {
                work->currentStep++;
                work->nextStep = 0;
            }
        }
    }
}

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

void RacerControl::reset() {}
void HumanControl::update() {}
