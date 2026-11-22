// Updates the moving actor, its path, and its visual effects.

#include "humath.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/frame_time.h"
#include "dolphin/gx.h"
#include "game/animdata.h"
extern "C" {
#include "game/audio.h"
#include "datadir_enum.h"
#include "game/hu3d.h"
#include <math.h>
}
#include "REL/framework/sound.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
#include "REL/m673DLL/scene_resources.h"
#include "REL/m673DLL/moving_actor.h"
#include "REL/m673DLL/movement_vector.h"
#include "REL/framework/event_packet.h"
#include <new>
#include <functional>
#include "REL/m673DLL/bound_camera_event.h"

// XZ positions used to build the moving actor's route.
const Vec lbl_1_rodata_348[16] = {
    { -400.0f, 0.0f, -300.0f },
    { -200.0f, 0.0f, -300.0f },
    { 200.0f, 0.0f, -300.0f },
    { 400.0f, 0.0f, -300.0f },
    { -400.0f, 0.0f, 0.0f },
    { -200.0f, 0.0f, 0.0f },
    { 200.0f, 0.0f, 0.0f },
    { 400.0f, 0.0f, 0.0f },
    { -400.0f, 0.0f, 300.0f },
    { -200.0f, 0.0f, 300.0f },
    { 200.0f, 0.0f, 300.0f },
    { 400.0f, 0.0f, 300.0f },
    { -300.0f, 0.0f, 400.0f },
    { 300.0f, 0.0f, 400.0f },
    { -300.0f, 0.0f, -400.0f },
    { 300.0f, 0.0f, -400.0f }
};

// Animation data numbers for the moving actor.
const unsigned long lbl_1_rodata_408[7] = {
    DATANUM(0x840000, 0x0E), DATANUM(0x840000, 0x12),
    DATANUM(0x840000, 0x0F), DATANUM(0x840000, 0x10),
    DATANUM(0x840000, 0x11), DATANUM(0x840000, 0x13),
    DATANUM(0x840000, 0x14)
};

// Creates the moving actor and its model, collision body, and effects.
MovingActor::MovingActor(s32 dataNumber, Position3 *position, f32 angle)
    : GameObject(2), ModelMotion(dataNumber), MinigameSingleton<MovingActor>(), component(100.0f),
      headingAngle(angle), targetPosition(*position), targetAngle(0.0f), movementState(0),
      movementSpeed(50.0f), drawAlpha(0.0f), ready(0),
      fallVelocityY(1600.0 / 60.0), elapsedFrames(0),
      delayFrames(0), transitionSeconds(0.0f), remainingSeconds(30.0f), approachSoundFrames(0),
      crossingSoundFrames(0), soundHandle(0), model(new OwnedModel(DATANUM(0x840000, 0x0A))),
      pointIndex(0), currentMotion(-1), channel0Events(0), channel1Events(1),
      motionResource(DATANUM(0x840000, 0x0E))
{
    ModelMotion::setLayer(3);
    ModelMotion::setPosition(&position->value);
    ModelMotion::setRotationY(angle);
    ModelMotion::setUniformScale(1.0f);
    this->effectPosition = *position;
    this->component.setPosition(*position);
    this->component.active = 1;
    this->component.relation = 1;
    this->velocity.x = 0.0f;
    this->velocity.y = 0.0f;
    this->velocity.z = 0.0f;
    this->fallVelocityX = 0.0f;
    this->unk98 = 0.0f;
    this->fallVelocityZ = 0.0f;
    this->headingAngle = angle;
    std::vector<Vec> availablePoints;
    s32 remaining;
    for (s32 i = 0; i < 16; i++) {
        availablePoints.push_back(lbl_1_rodata_348[i]);
    }
    remaining = 16;
    for (s32 i = 0; i < 16; i++) {
        unsigned long index = (s32)RandomUnsigned() % remaining;
        Vec selected = availablePoints.at(index);
        this->points.push_back(selected);
        availablePoints.erase(&availablePoints.at(index));
        remaining--;
    }
    this->motionState = 0;
    this->nextMotionCallback = 0;
    this->motionFrames = 0;
    this->timedMotionChance = 30;
    this->crossingMotionChance = 0;
    registerEventCallback(&this->channel0Events,
        std::bind1st(std::mem_fun(&MovingActor::fn_1_AE54), this), 0);
    registerEventCallback(&this->channel0Events,
        std::bind1st(std::mem_fun(&MovingActor::fn_1_AF1C), this), 1);
    registerEventCallback(&this->channel0Events,
        std::bind1st(std::mem_fun(&MovingActor::fn_1_AF5C), this), 2);
    registerEventCallback(&this->channel0Events,
        std::bind1st(std::mem_fun(&MovingActor::fn_1_AFD8), this), 4);
    registerEventCallback(&this->channel0Events,
        std::bind1st(std::mem_fun(&MovingActor::fn_1_B05C), this), 5);
    ModelMotion::addMotions(lbl_1_rodata_408, 7);
    fn_1_9B44(DATANUM(0x840000, 0x0E), 1);
    this->modelPosition = *position;
    this->model->setPosition(&position->value);
    this->model->setLayer(3);
    ModelMotion::setShadow();
    SceneResources &resources = SceneResources::getChecked();
    this->collidingParticles.reset(new BurstEmitter(
        resources.animationAt(0), 30, 2, *position));
    this->followEffect.reset(new FollowEffect(&this->effectPosition.value, 300.0f,
        resources.animationAt(4)));
    this->followEffect->setAlpha(this->drawAlpha);
    static const GXColor colorValue = { 0xFF, 0xA0, 0xA0, 0x7F };
    GXColor color = colorValue;
    this->followEffect->setColor(color);
    this->setState(0);
    Position3 trajectory = this->component.position;
    f32 verticalVelocity = this->fallVelocityY;
    f32 time = 0.0f;
    while (trajectory.value.y > 0.0f) {
        f32 previous = verticalVelocity;
        verticalVelocity = verticalVelocity - 8.0 * 980.665 / 60.0 / 60.0;
        f32 displacement = 0.5f * (verticalVelocity + previous);
        trajectory.value.y += displacement;
        time += 1.0f;
    }
    static const Vec zero = { 0.0f, 0.0f, 0.0f };
    HuSubVecF(&trajectory.value, &zero, &trajectory.value);
    PSVECScale(&trajectory.value, &trajectory.value, 1.0f / time);
    this->fallVelocityX = trajectory.value.x;
    this->fallVelocityZ = trajectory.value.z;
}

// Releases the actor model, effects, and event handlers.
MovingActor::~MovingActor()
{
}

// Advances the actor state and updates its model and particle group.
void MovingActor::update()
{
    BurstEmitter *emitter;

    if (state != 0) {
        (this->*state)();
    }
    model->setPosition(&modelPosition.value);
    emitter = collidingParticles.get();
    // The constructor leaves the offset untouched. This read also runs before the first
    // burst explicitly assigns its components.
    PSVECAdd(&emitter->offset, &component.position.value, &emitter->position);
    ModelMotion::setRotationY(headingAngle);
    ModelMotion::setPosition(&component.position.value);
}

// Shifts to the requested actor motion when it changes.
inline void MovingActor::fn_1_9B44(u32 motion, u8 enabled)
{
    if (currentMotion != motion) {
        setAttribute(0x40000081);
        ModelMotion::shiftMotion(motion, 16.0f, 0.0f, enabled == 0 ? 0 : 0x40000041);
        currentMotion = motion;
    }
}

// Marks the actor ready.
void MovingActor::fn_1_9BE8()
{
    ready = 1;
}

// Advances the delayed fall and updates the associated model.
void MovingActor::fn_1_9BF4()
{
    f32 previousVelocityY;
    f32 verticalDisplacement;

    static s32 fallDelayFrames = (s32)secondsToFrames(0.5);
    static s32 modelRiseDelayFrames = (s32)secondsToFrames(0.5);
    if (this->elapsedFrames++ >= fallDelayFrames) {
        if (this->delayFrames < modelRiseDelayFrames) {
            this->delayFrames++;
        } else {
            this->modelPosition.value.y += 800.0f / 60.0f;
            if (this->modelPosition.value.y > 450.0f) {
                this->model->setLayer(7);
            }
        }
        previousVelocityY = this->fallVelocityY;
        if (this->component.position.value.y > 0.0f) {
            this->component.position.value.x += this->fallVelocityX;
            this->component.position.value.z += this->fallVelocityZ;
            this->fallVelocityY = this->fallVelocityY - 8.0 * 980.665 / 60.0 / 60.0;
            verticalDisplacement = 0.5f * (this->fallVelocityY + previousVelocityY);
            this->component.position.value.y += verticalDisplacement;
            if (this->component.position.value.y < 0.0f) {
                this->component.position.value.y = 0.0f;
                soundPlayAt(1, 0x8AA, &this->component.position.value);
                sendEvent(1, 0, 0.0f);
                sendEvent(2, 0, &this->component.position.value, 0.0f);
                this->fn_1_9B44(DATANUM(0x840000, 0x0E), 1);
            }
        }
    }
}

// Advances movement, counts down the remaining time, and updates the follow effect.
void MovingActor::fn_1_9EB0()
{
    Vec scaledVelocity;
    f32 scale;
    f32 effectAlpha;

    remainingSeconds = remainingSeconds - 1.0 / 60.0;
    if (motionState != 0) {
        (this->*motionState)();
    }
    scale = movementSpeed;
    PSVECScale(&velocity, &scaledVelocity, scale);
    PSVECAdd(&component.position.value, &scaledVelocity, &effectPosition.value);
    effectPosition.value.y += 100.0f;
    effectAlpha = drawAlpha;
    followEffect->setAlpha(effectAlpha);
}

// Selects the turning state and the next path point.
void MovingActor::fn_1_9F90()
{
    State motionCallback;

    movementState = 2;
    motionCallback = &MovingActor::fn_1_A9D8;
    motionState = motionCallback;
    fn_1_AB5C(this);
}

// Moves toward the selected position and switches state on arrival.
void MovingActor::fn_1_A000()
{
    Vec difference;
    Vec scaledVelocity;
    State motionCallback;
    f32 distance;

    HuSubVecF(&difference, &component.position.value, &targetPosition.value);
    movementState = 1;
    distance = PSVECMag(&difference);
    if (distance < 400.0f / 60.0f) {
        component.setPosition(targetPosition);
        motionCallback = &MovingActor::fn_1_A718;
        motionState = motionCallback;
    } else {
        PSVECScale(&velocity, &scaledVelocity, 400.0f / 60.0f);
        PSVECAdd(&component.position.value, &scaledVelocity, &component.position.value);
    }
    if (approachSoundFrames % 30 == 0) {
        soundPlayAt(1, 0x3C9, &component.position.value);
    }
    approachSoundFrames++;
}

// Selects the turning state and the next path point.
void MovingActor::fn_1_A144()
{
    State motionCallback;

    movementState = 2;
    motionCallback = &MovingActor::fn_1_A9D8;
    motionState = motionCallback;
    fn_1_AB5C(this);
}

// Advances movement and returns to motion selection at the arena boundary.
void MovingActor::fn_1_A1B4()
{
    Vec displacement;
    State motionCallback;

    movementState = 3;
    if (component.position.value.x > 4.8f * 100.0f ||
        component.position.value.x < -(4.8f * 100.0f) ||
        component.position.value.z > 4.2f * 100.0f ||
        component.position.value.z < -(4.2f * 100.0f)) {
        motionCallback = &MovingActor::fn_1_A718;
        motionState = motionCallback;
    } else {
        PSVECScale(&velocity, &displacement, 800.0f / 60.0f);
        PSVECAdd(&component.position.value, &displacement, &component.position.value);
    }
    if ((crossingSoundFrames & 0xF) == 0) {
        soundPlayAt(1, 0x3CA, &component.position.value);
    }
    crossingSoundFrames++;
}

// Resets the motion timer and selects the next path point.
void MovingActor::fn_1_A2D4()
{
    State motionCallback;

    movementState = 2;
    motionFrames = 0;
    motionCallback = &MovingActor::fn_1_A9D8;
    motionState = motionCallback;
    fn_1_AB5C(this);
}

// Advances the timed motion sequence and its sound effects.
void MovingActor::fn_1_A34C()
{
    State motionCallback;

    static s32 burstFrame = (s32)secondsToFrames(0.9f);
    static s32 fadeStartFrame = (s32)secondsToFrames(1.0f);
    static s32 fadeOutFrame = (s32)secondsToFrames(2.0f);
    static s32 finishMotionFrame = (s32)secondsToFrames(4.5f);
    movementState = 4;
    motionFrames++;
    if (motionFrames == burstFrame) {
        fn_1_3F74(collidingParticles.get(), headingAngle);
        soundHandle = soundPlayAt(1, 0x8AB, &component.position.value);
    }
    if (drawAlpha < 1.0f && motionFrames < fadeOutFrame &&
        motionFrames >= fadeStartFrame) {
        drawAlpha += 5.0 / 60.0;
        if (drawAlpha > 1.0f) {
            drawAlpha = 1.0f;
        }
    }
    if (drawAlpha > 0.0f && motionFrames >= fadeOutFrame) {
        drawAlpha = drawAlpha - 5.0 / 60.0;
        if (drawAlpha < 0.0f) {
            drawAlpha = 0.0f;
        }
        return;
    }
    if (currentMotion == DATANUM(0x840000, 0x11)) {
        if (fn_8002D75C(*this) != 0) {
            fn_1_9B44(DATANUM(0x840000, 0x0E), 1);
        }
    }
    if (motionFrames >= finishMotionFrame) {
        motionFrames = 0;
        fn_1_406C(collidingParticles.get());
        HuAudFXStop(soundHandle);
        motionCallback = &MovingActor::fn_1_A718;
        motionState = motionCallback;
    }
}

// Chooses the next motion using the remaining time and motion probabilities.
void MovingActor::fn_1_A718()
{
    s16 loadedModelId = *this;
    if ((s32)RandomUnsigned() % 100 < timedMotionChance && remainingSeconds > 5.0f) {
        motionResource = DATANUM(0x840000, 0x11);
        nextMotionCallback = &MovingActor::fn_1_A34C;
        State motionCallback = &MovingActor::fn_1_A2D4;
        motionState = motionCallback;
        timedMotionChance = 0;
    } else if ((s32)RandomUnsigned() % 100 < crossingMotionChance && remainingSeconds > 3.0f) {
        motionResource = DATANUM(0x840000, 0x10);
        nextMotionCallback = &MovingActor::fn_1_A1B4;
        crossingSoundFrames = 0;
        State motionCallback = &MovingActor::fn_1_A144;
        motionState = motionCallback;
        timedMotionChance += 33;
        crossingMotionChance = 0;
    } else {
        motionResource = DATANUM(0x840000, 0x0F);
        nextMotionCallback = &MovingActor::fn_1_A000;
        approachSoundFrames = 0;
        State motionCallback = &MovingActor::fn_1_9F90;
        motionState = motionCallback;
        timedMotionChance += 20;
        crossingMotionChance += 20;
    }
    approachSoundFrames = 0;
    fn_1_9B44(DATANUM(0x840000, 0x0F), 1);
}

// Turns toward the target heading and selects the transition or steady motion.
void MovingActor::fn_1_A9D8()
{
    if (fn_1_B208(&headingAngle, targetAngle) != 0) {
        State motionCallback = nextMotionCallback;
        motionState = motionCallback;
        if (nextMotionCallback != &MovingActor::fn_1_A34C) {
            fn_1_9B44(motionResource, 1);
        } else {
            fn_1_9B44(motionResource, 0);
        }
        approachSoundFrames = 0;
    }
    if (approachSoundFrames % 30 == 0) {
        soundPlayAt(1, 0x3C9, &component.position.value);
    }
    approachSoundFrames++;
}

// Selects the next path point and updates the movement direction and target heading.
void fn_1_AB5C(MovingActor *object)
{
    MovementVector direction;
    Point3d *selectedPoint = &object->points.at(object->pointIndex);

    object->targetPosition.value.x = selectedPoint->x;
    object->targetPosition.value.y = selectedPoint->y;
    object->targetPosition.value.z = selectedPoint->z;
    object->pointIndex++;
    object->pointIndex %= 16;

    HuSubVecF(&direction, &object->targetPosition.value, &object->component.position.value);
    Vec *heading = &direction.normalize();
    object->velocity.x = heading->x;
    object->velocity.y = heading->y;
    object->velocity.z = heading->z;
    object->targetAngle = fn_1_B4F4(&object->component.position.value,
        &object->targetPosition.value);
}

// Finishes the active movement and turns back toward the default heading.
void MovingActor::fn_1_AC34()
{
    if (motionState == &MovingActor::fn_1_A34C || motionState == &MovingActor::fn_1_A1B4) {
        (this->*motionState)();
        if (motionState == &MovingActor::fn_1_A718 && fabs(headingAngle) > 10.0) {
            fn_1_9B44(DATANUM(0x840000, 0x0F), 1);
        }
        return;
    }
    if (fn_1_B208(&headingAngle, 0.0f) != 0) {
        fn_1_9B44(DATANUM(0x840000, 0x0E), 1);
        transitionSeconds += 1.0 / 60.0;
        if (!(transitionSeconds < 1.0f)) {
            State stateCallback = &MovingActor::fn_1_9BE8;
            state = stateCallback;
        }
    }
}

// Leaves the moving actor unchanged in its idle state.
void MovingActor::fn_1_AE50()
{
}

// Starts the falling state, sound, and motion.
void MovingActor::fn_1_AE54(const EventPacket &)
{
    State stateCallback = &MovingActor::fn_1_9BF4;
    state = stateCallback;
    soundPlayAt(1, 0x3CE, &component.position.value);
    fn_1_9B44(DATANUM(0x840000, 0x12), 0);
}

// Hides the owned model.
void MovingActor::fn_1_AF1C(const EventPacket &)
{
    model->setAttribute(1);
}

// Starts movement and motion selection.
void MovingActor::fn_1_AF5C(const EventPacket &)
{
    State stateCallback = &MovingActor::fn_1_9EB0;
    state = stateCallback;
    State motionCallback = &MovingActor::fn_1_A718;
    motionState = motionCallback;
}

// Hides the follow effect and starts the final turn.
void MovingActor::fn_1_AFD8(const EventPacket &)
{
    drawAlpha = 0.0f;
    followEffect->setAlpha(drawAlpha);
    State stateCallback = &MovingActor::fn_1_AC34;
    state = stateCallback;
}

// Chooses the ending motion from the event result.
void MovingActor::fn_1_B05C(const EventPacket &event)
{
    State stateCallback = &MovingActor::fn_1_AE50;
    const s32 *result;

    state = stateCallback;
    result = static_cast<const s32 *>(event.context);
    if (*result == 4) {
        fn_1_9B44(DATANUM(0x840000, 0x13), 0);
        soundPlayAtDelayed(1, 0x3CB, &component.position.value, 30);
        soundPlayAtDelayed(1, 0x3CD, &component.position.value, 110);
    } else {
        fn_1_9B44(DATANUM(0x840000, 0x14), 0);
        soundPlayAt(1, 0x3CF, &component.position.value);
        soundPlayAtDelayed(1, 0x3C9, &component.position.value, 65);
        soundPlayAtDelayed(1, 0x3C9, &component.position.value, 90);
        soundPlayAtDelayed(1, 0x3CC, &component.position.value, 120);
    }
}

// Turns an angle toward its target using the shorter direction.
u8 MovingActor::fn_1_B208(f32 *angle, f32 target)
{
    int direction = 1;
    if (*angle < 0.0f) {
        direction *= -1;
    }
    while (fabs(*angle) > 180.0) {
        *angle -= 360.0f * direction;
    }
    f32 distance = fabs(*angle - target);
    int stepDirection = 1;
    if (*angle < target) {
        stepDirection *= -1;
    }
    if (distance > 180.0f) {
        stepDirection *= -1;
    }
    f32 fraction = 0.0f;
    if (distance > 180.0f) {
        fraction = (360.0 - fabs(*angle - target)) / 180.0;
    } else {
        fraction = fabs(*angle - target) / 180.0;
    }
    if (fraction < 0.2f) {
        fraction = 0.2f;
    }
    f32 maximumStep = 12.0f * fraction;
    f32 minimumStep = 2.0f * fraction;
    if (minimumStep < 1.0f) {
        minimumStep = 1.0f;
    }
    if (distance > maximumStep) {
        *angle -= maximumStep * stepDirection;
    } else {
        *angle = target;
        return 1;
    }
    return 0;
}

// Returns the angle from one point to another in the XZ plane.
f32 fn_1_B4F4(Point3d *from, Point3d *to)
{
    Point3d delta;

    HuSubVecF(&delta, to, from);
    return 180.0 * (atan2(delta.x, delta.z) / M_PI);
}
