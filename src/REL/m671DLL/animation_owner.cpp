#include "REL/m671DLL/game_constants.h"
#include "REL/m671DLL/member_callback.h"
extern "C" {
#include <math.h>
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
#include "math.h"

#include "dolphin/mtx.h"

#include "dolphin/types.h"

#include "REL/m671DLL/animation_owner.h"
#include "REL/framework/sound.h"

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/data.h"
}

#include "REL/m671DLL/resources.h"

#include "REL/framework/event_hm.h"

#include "REL/m671DLL/particle_owner.h"

#include "REL/m671DLL/motion_position.h"

#include "REL/m671DLL/collision.h"

#include "REL/framework/assertion.h"

#include "REL/m671DLL/callback_receiver.h"

#include "humath.h"

#include "REL/m671DLL/math_callback.h"

#include "REL/framework/event_callback_inline.h"
#include <vector>

float lbl_1_data_19C = 6.6666665f;

float lbl_1_data_1A0 = 100.0f;

float lbl_1_data_1A4 = 1500.0f;

float lbl_1_data_1A8 = 400.0f;

float lbl_1_data_1AC = 310.0f;
float lbl_1_bss_34 = lbl_1_data_1A8 + lbl_1_data_1AC;

void fn_1_49C0(Quaternion *result, const Vec *sourceDirection, const Vec *targetDirection)
{
    Vec normalizedSource;
    Vec normalizedTarget;
    Vec crossProduct;
    double dot;
    double magnitude;
    double inverseMagnitude;

    PSVECNormalize((Vec *)sourceDirection, &normalizedSource);
    PSVECNormalize((Vec *)targetDirection, &normalizedTarget);
    dot = PSVECDotProduct(&normalizedSource, &normalizedTarget);
    magnitude = dot;
    inverseMagnitude = sqrt(2.0 + 2.0 * magnitude);
    result->w = (f32)(inverseMagnitude / 2.0);
    inverseMagnitude = 1.0 / inverseMagnitude;
    PSVECCrossProduct(&normalizedSource, &normalizedTarget, &crossProduct);
    result->x = (f32)((double)crossProduct.x * inverseMagnitude);
    result->y = (f32)((double)crossProduct.y * inverseMagnitude);
    result->z = (f32)((double)crossProduct.z * inverseMagnitude);
}

static inline float vectorDot(const Vec *first, const Vec *second)
{
    return PSVECDotProduct(first, second);
}

static inline void vectorScaleTo(const Vec *source, Vec *target,
                                    float scale)
{
    PSVECScale(source, target, scale);
}

static inline void reflectVector(const Vec *normal, Vec *point)
{
    MotionPosition scaledNormal;

    vectorScaleTo(normal, &scaledNormal.value,
                       -2.0f * vectorDot(normal, point));
    PSVECAdd(point, &scaledNormal.value, &scaledNormal.value);
    point->x = scaledNormal.value.x;
    point->y = scaledNormal.value.y;
    point->z = scaledNormal.value.z;
}

void fn_1_4D1C(Vec *normal, Vec *point)
{
    reflectVector(normal, point);
}

void fn_1_4E08(AnimationOwner *, HU3D_DRAW_OBJ *, HSF_MATERIAL *);

void fn_1_4DC8(HU3D_DRAW_OBJ *owner, HSF_MATERIAL *data)
{
    fn_1_4E08(static_cast<AnimationOwner *>(owner->model->hookData), owner, data);
}

HuVecF lbl_1_data_1B0 = { 0.0f, 600.0f, 0.1f };

void fn_1_4E08(AnimationOwner *positionOwner,
                              HU3D_DRAW_OBJ *drawObject,
                              HSF_MATERIAL *material)
{
    HuVecF direction;
    MotionPosition position;
    short lightId;
    Resources * lightSettingOwner;
    lightSettingOwner = &Resources::getChecked();
    lightId = lightSettingOwner->id();
    position = positionOwner->position;
    PSVECSubtract(&position.value, &lbl_1_data_1B0, &direction);
    PSVECNormalize(&direction, &direction);
    Hu3DGLightPosSetV(lightId, &lbl_1_data_1B0, & direction);
    fn_8001A7CC(drawObject, material);
    fn_8001CDF0(drawObject->model, 0U, material->hiliteScale);
}

void fn_1_57C4(AnimationOwner *owner);

void fn_1_4DC8(HU3D_DRAW_OBJ *, HSF_MATERIAL *);

template<class Payload>
void fn_1_6964(EventHandlerManager *owner, const Payload &payload, int key)
{
    owner->addHandler(new MemberCallback<AnimationOwner>(payload), key);
}

AnimationOwner::AnimationOwner(int model, int mode)
    : GameObject(1), OwnedModel((unsigned int)model), position(lbl_1_data_1A0), enabled(0), argument(mode), elapsedTime(0.0f),
      transitionComplete(0), preserveSpeed(0), particle(0), attribute(0), callbacks(0)
{
    const Model *modelIdPointer;
    ParticleOwner *firstResourceCopy;
    AttributeOwner *secondResourceCopy;
    double initialScale;

    initialScale = lbl_1_data_1A4;
    this->position.value.x = 0.0f;
    this->position.value.y = (float)initialScale;
    this->position.value.z = 0.0f;
    this->previousPosition.value.x = 0.0f;
    this->previousPosition.value.y = 0.0f;
    this->previousPosition.value.z = 0.0f;
    this->descentSpeed = 0.0f;
    this->speedStep = 0.0f;
    this->modelScale = 1.0f;
    this->setCamera(1);
    this->resetAttribute(((1 << 30) | (1 << 1)));
    this->setMotionTime(0.0f);
    this->setLayer(5);
    this->setHookData(this);
    modelIdPointer = this;
    s16 modelId = modelIdPointer->modelId;
    s16 hookModelId = modelId;
    fn_80027884(hookModelId, fn_1_4DC8);
    fn_1_57C4(this);
    this->setPosition(&this->position.value);
    this->setUniformScale(this->modelScale);
    this->position.active = 0;
    this->position.relation = 1;
    PSMTXIdentity(this->matrix);
    PSMTXRotRad(this->matrix, 0x79, 0.0f);
    this->position.matrix = &this->matrix;
    this->rotation.x = this->rotation.y = this->rotation.z = 0.0f;
    this->rotation.w = 1.0f;
    this->complete = 0;

    fn_1_6964(&this->callbacks, std::bind1st(
        std::mem_fun(&AnimationOwner::fn_1_62E8), this), 4);
    fn_1_6964(&this->callbacks, std::bind1st(
        std::mem_fun(&AnimationOwner::fn_1_634C), this), 5);

    firstResourceCopy = new ParticleOwner;
    this->particle.reset(firstResourceCopy);
    secondResourceCopy = new AttributeOwner;
    this->attribute.reset(secondResourceCopy);
    this->entered = 0;
    this->setCallback(&AnimationOwner::fn_1_6008);
}

AnimationOwner::~AnimationOwner()
{
}

void AnimationOwner::update()
{
    MotionPosition position;
    AttributeOwner *positionValues;
    AttributeOwner *verticalValues;
    AttributeOwner *blendValues;
    float progress;
    float vertical;
    float blend;

    if (this->callback != 0) {
        (this->*this->callback)();
    }
    this->setPosition(&this->position.value);
    if (this->enabled && this->complete != 1 &&
        this->position.value.y > 100.0f) {
        position = this->position;
        position.value.y = 0.0f;
        positionValues = this->attribute.get();
        positionValues->position.value.x = position.value.x;
        positionValues->position.value.y = position.value.y;
        positionValues->position.value.z = position.value.z;
        progress = this->position.value.y / 1000.0f;
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        vertical = 0.5f * (1.0f - progress);
        verticalValues = this->attribute.get();
        verticalValues->alphaScale = vertical;
        blend = 1.4f * progress;
        if (blend < 0.7f) {
            blend = 0.7f;
        }
        blendValues = this->attribute.get();
        blendValues->scaleXZ = blend;
        fn_1_41B4(this->attribute.get(), 1);
    }
}

void fn_1_5754(AnimationOwner *camera, const Vec *position)
{
    Vec *cameraPosition;
    Vec *targetPosition;

    cameraPosition = &camera->position.value;
    targetPosition = cameraPosition;
    targetPosition->x = position->x;
    targetPosition->y = position->y;
    targetPosition->z = position->z;
    camera->setPosition(&camera->position.value);
}

void fn_1_57C4(AnimationOwner *owner)
{
    owner->setAttribute(1);
}

void fn_1_57F4(AnimationOwner *owner)
{
    owner->resetAttribute(1);
}

void fn_1_5824(AnimationOwner *owner, const Vec *position)
{
    Vec *targetPosition;
    Vec *currentPosition;

    targetPosition = &owner->position.value;
    currentPosition = targetPosition;
    currentPosition->x = position->x;
    currentPosition->y = position->y;
    currentPosition->z = position->z;
    owner->position.value.y = lbl_1_data_1A4;
    owner->resetAttribute(1);
    owner->entered = 0;
    owner->setCallback(&AnimationOwner::fn_1_600C);
}

void fn_1_58E4(AnimationOwner *owner, const Vec *position)
{

    owner->previousPosition.value.x = position->x;
    owner->previousPosition.value.y = position->y;
    owner->previousPosition.value.z = position->z;
    owner->position.active = 1;
    owner->enabled = 1;
    owner->setCallback(&AnimationOwner::fn_1_6158);
}

void fn_1_5F00(AnimationOwner *owner, const Vec *position);

void fn_1_3EC0(ParticleOwner *, const Vec *);

static inline void vectorSet(Vec *vector, double x, double y, double z)
{
    vector->x = x;
    vector->y = y;
    vector->z = z;
}

static inline float vectorLength(const Vec *vector)
{
    return PSVECMag(vector);
}

static inline void vectorScale(Vec *vector, float scale)
{
    PSVECScale(vector, vector, scale);
}

static inline void vectorCopy(MotionPosition *target, const MotionPosition &source)
{
    const MotionPosition *const copySource = &source;
    *target = *copySource;
}

static inline MotionPosition getEffectPosition(CallbackReceiver *owner)
{
    return owner->position;
}

static inline ParticleOwner *effectEmitterGet(AnimationOwner *owner)
{
    return owner->particle.get();
}

static inline AttributeOwner *effectModelGet(AnimationOwner *owner)
{
    return owner->attribute.get();
}

void fn_1_5950(AnimationOwner *owner)
{
    MotionPosition initialPosition;
    MotionPosition actorPosition;
    MotionPosition effectPosition;
    MotionPosition radialDirection;
    MotionPosition singletonPosition;
    MotionPosition effectOffset;
    MotionPosition reflectedVelocity;
    MotionPosition bounceNormal;
    MotionPosition effectTarget;
    MotionPosition inversePosition;
    float velocityMagnitude;
    float maximumDistance;
    float distanceCorrection;
    float effectMagnitude;

    if (owner->position.value.y < 100.0f) {
        owner->previousPosition.value.y = 0.0f;
        owner->position.value.y = 100.0f;
    }
    static float limit = 500.0f;
    vectorSet(&initialPosition.value, owner->position.value.x, 0.0f, owner->position.value.z);
    velocityMagnitude = vectorLength(&owner->position.value);
    if (!(owner->position.value.y > 100.0f)) {
        if (owner->complete == 0) {
            actorPosition = owner->position;
            actorPosition.value.y = 0.0f;
            owner->previousPosition.value.y = 0.0f;
            effectPosition = owner->previousPosition;
            if (effectPosition.value.x == 0.0f && effectPosition.value.z == 0.0f) {
                CallbackReceiver &sourceOwner = MinigameSingleton<CallbackReceiver>::getChecked();
                vectorCopy(&singletonPosition, getEffectPosition(&sourceOwner));
                PSVECSubtract(&owner->position.value, &singletonPosition.value, &radialDirection.value);
                radialDirection.value.y = 0.0f;
                PSVECNormalize(&radialDirection.value, &effectPosition.value);
            }
            PSVECNormalize(&effectPosition.value, &effectPosition.value);
            vectorScale(&effectPosition.value, lbl_1_data_19C);
            fn_1_5F00(owner, &effectPosition.value);
            sendEvent(2, 0, &actorPosition.value, 0.0f);
            sendEvent(1, 0, 0.0f);
            fn_1_3EC0(effectEmitterGet(owner), &owner->position.value);
            fn_1_41B4(effectModelGet(owner), 0);
            owner->setShadow();
            owner->complete = 1;
            soundPlayAt(1, 0x89D, &actorPosition.value);
        }
        maximumDistance = limit - lbl_1_data_1A0;
        if (velocityMagnitude > maximumDistance) {
            distanceCorrection = maximumDistance - velocityMagnitude;
            PSVECScale(&initialPosition.value, &effectOffset.value,
                       distanceCorrection / velocityMagnitude);
            PSVECAdd(&effectOffset.value, &owner->position.value, &owner->position.value);
            reflectedVelocity = owner->previousPosition;
            reflectedVelocity.value.y = 0.0f;
            PSVECNormalize(&reflectedVelocity.value, &reflectedVelocity.value);
            bounceNormal = owner->position;
            bounceNormal.value.y = 0.0f;
            PSVECNormalize(&bounceNormal.value, &bounceNormal.value);
            PSVECScale(&bounceNormal.value, &bounceNormal.value, -1.0f);
            reflectVector(&bounceNormal.value, &reflectedVelocity.value);
            vectorScaleTo(&reflectedVelocity.value, &owner->previousPosition.value, owner->descentSpeed);
            if (owner->descentSpeed != 0.0f) {
                effectTarget = owner->position;
                effectMagnitude = lbl_1_data_1A0 + vectorLength(&effectTarget.value);
                PSVECNormalize(&effectTarget.value, &effectTarget.value);
                inversePosition = effectTarget;
                PSVECScale(&effectTarget.value, &effectTarget.value, effectMagnitude);
                vectorSet(&inversePosition.value, effectTarget.value.z, 0.0f, -effectTarget.value.x);
                fn_8005D864(1, &effectTarget.value, &inversePosition.value);
                soundPlayAt(1, 0x89F, &effectTarget.value);
            }
        }
    }
}

void fn_1_5F00(AnimationOwner *owner, const Vec *position)
{
    float positionMagnitude;

    owner->previousPosition.value.x = position->x;
    owner->previousPosition.value.y = position->y;
    owner->previousPosition.value.z = position->z;
    positionMagnitude = PSVECMag(&owner->previousPosition.value);
    owner->descentSpeed = positionMagnitude;
}

void fn_1_5F6C(AnimationOwner *owner, float angle)
{
    float rotation[3][4];
    float (*modelMatrix)[4];

    PSMTXIdentity(owner->matrix);
    PSMTXRotRad(rotation, 0x59, MTXDegToRad(angle));
    PSMTXConcat(owner->matrix, rotation, owner->matrix);
    modelMatrix = reinterpret_cast<MtxPtr>(static_cast<Model *>(owner)->getMatrix());
    PSMTXCopy(owner->matrix, modelMatrix);
}

void AnimationOwner::fn_1_6008()
{
}

void AnimationOwner::fn_1_600C()
{
    Vec motionHeightCopy = CallbackReceiver::getChecked().getMotionHeight();
    f32 frameStep = (lbl_1_data_1A4 - lbl_1_bss_34) / 30.0f;
    this->position.value.y -= frameStep;
    if (this->position.value.y < lbl_1_bss_34 + motionHeightCopy.y) {
        this->position.value.y = lbl_1_bss_34 + motionHeightCopy.y;
    }
    if (!this->entered) {
        soundPlayAt(1, 0x8A1, &this->position.value);
        this->entered = 1;
    }
}

void fn_1_3E7C(ParticleOwner *sequence, const Vec *position);

void fn_1_5950(AnimationOwner *owner);

extern void fn_1_6604(AnimationOwner *owner, const Vec *position, const Vec *target);

/* Updates the motion position and completes its timed descent. */
void AnimationOwner::fn_1_6158()
{
    Vec positionSnapshot = this->position.value;
    Vec position = this->previousPosition.value;
    this->previousPosition.value.y += 4.0f * -gravity;
    position.y = 0.5f *
                 (position.y + this->previousPosition.value.y);
    PSVECAdd(&position, &this->position.value, &this->position.value);
    fn_1_5950(this);
    fn_1_6604(this, &positionSnapshot, &this->position.value);
    this->position.state = -1;
    fn_1_3E7C(this->particle.get(), &this->position.value);
    this->elapsedTime += 0.016666666666666666;
    if (this->argument == 0 && this->transitionComplete != 1 &&
        this->elapsedTime > 5.0f) {
        this->descentSpeed -= lbl_1_data_19C / 300.0f;
        if (this->descentSpeed < 0.16666666666666666) {
            this->descentSpeed = 0.16666667f;
            this->transitionComplete = 1;
        }
    }
}

/* Debug hook: empty in the release build (the compiler still makes its argument). */
static inline void DebugCallback(const AnimationOwner::Callback &) {}

void AnimationOwner::fn_1_62E8(EventMessage *)
{
    speedStep = descentSpeed / 300.0f;
    preserveSpeed = 1;
    setCallback(&AnimationOwner::fn_1_6350);
}

void AnimationOwner::fn_1_634C(EventMessage *)
{
    DebugCallback(&AnimationOwner::fn_1_65C0);
}

void fn_1_4078(ParticleOwner *);

void AnimationOwner::fn_1_6350()
{
    float descentSpeed;

    this->descentSpeed -= this->speedStep;
    if (this->descentSpeed < 0.0f) {
        this->descentSpeed = 0.0f;
    }
    this->previousPosition.value.y = 0.0f;
    PSVECNormalize(&this->previousPosition.value, &this->previousPosition.value);
    descentSpeed = this->descentSpeed;
    PSVECScale(&this->previousPosition.value, &this->previousPosition.value,
               descentSpeed);
    this->fn_1_6158();
    if (!(this->descentSpeed > 0.0f)) {
        fn_1_4078(this->particle.get());
        setCallback(&AnimationOwner::fn_1_65BC);
    }
}

void AnimationOwner::fn_1_65BC()
{
}

void AnimationOwner::fn_1_65C0()
{
    setCallback(&AnimationOwner::fn_1_6008);
}

void fn_1_49C0(Quaternion *, const Vec *, const Vec *);

void fn_1_6604(AnimationOwner *owner, const Vec *target,
               const Vec *position)
{
    Vec targetDirection;
    Vec down;
    Quaternion delta;
    f64 height = lbl_1_data_1A0;
    f64 xDifference;
    f64 zDifference;
    MtxPtr matrix;

    zDifference = target->z - position->z;
    xDifference = target->x - position->x;
    targetDirection.x = xDifference;
    targetDirection.y = (f32)-height;
    targetDirection.z = zDifference;
    down.x = 0.0f;
    down.y = (f32)-height;
    down.z = 0.0f;
    fn_1_49C0(&delta, &down, &targetDirection);
    PSQUATMultiply(&delta, &owner->rotation, &owner->rotation);
    PSQUATNormalize(&owner->rotation, &owner->rotation);
    PSMTXQuat(owner->matrix, &owner->rotation);
    matrix = reinterpret_cast<MtxPtr>(static_cast<Model *>(owner)->getMatrix());
    PSMTXCopy(owner->matrix, matrix);
}
