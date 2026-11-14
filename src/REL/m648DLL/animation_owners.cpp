#include "REL/framework/random.h"
#include "REL/framework/frame_time_inline.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/misc_diff_deg.h"
#include "humath.h"
extern "C" {
#include "dolphin/mtx.h"
#include <math.h>
#include "game/hu3d.h"
}
#include "datadir_enum.h"
#include "REL/m648DLL/animation_owners.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

#include "include/game/audio.h"

#include "include/game/gamemes.h"

#include "REL/framework/model.h"

#include "REL/framework/linknode.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"

extern "C" {
#include "dolphin/types.h"

}

#include "REL/m648DLL/animation_setup_owner.h"

class AnimationOwner : public RequestObject, public MinigameSingleton<AnimationOwner> {
public:
    OwnedModel model;
    AnimationOwner();
    virtual ~AnimationOwner();
    virtual void flagEnabled(int request);
    virtual void update();
};

extern "C" {

void fn_1_123C(Vec *position, s32 index);
}

extern "C" {
void fn_1_23C4(Vec *position, s32 index);

}

/* Chooses a random radial position for the indexed model. */
void fn_1_123C(Vec *position, s32 index)
{
    f32 angle;
    f32 distance;
    angle = 40.0f
          + (((f32)(index * 10) + (30.0f * RandomFloat()))
             - 180.0f);
    distance = 1200.0f + (600.0f * RandomFloat());
    position->x = distance * cos((M_PI * (f64)angle) / 180.0);
    position->z = distance * sin((M_PI * (f64)angle) / 180.0);
    position->y = 50.0f + (200.0f * RandomFloat());
}

/* Initializes the three model slots and shuffles their resource assignment. */
AnimationSetupOwner::AnimationSetupOwner()
    : model0(DATANUM(0x6b0000, 14)), model1(DATANUM(0x6b0000, 15)), model2(DATANUM(0x6b0000, 16))
{
    s16 modelId;
    s32 shuffleIndex;
    s32 resourceIndex;
    modelId = model0;
    fn_800269E8(modelId, (1 << 0));
    modelId = model1;
    fn_800269E8(modelId, (1 << 0));
    modelId = model2;
    fn_800269E8(modelId, (1 << 0));
    s32 order[9] = { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
    s32 slot;
    s32 swapIndex;
    s32 firstValue;
    s32 secondValue;
    s32 selectedType;
    for (shuffleIndex = 0; shuffleIndex < 9; ++shuffleIndex)
    {
        swapIndex = ((u16) RandomUnsigned() * 9) >> 16;
        firstValue = order[shuffleIndex];
        secondValue = order[swapIndex];
        order[shuffleIndex] = secondValue;
        order[swapIndex] = firstValue;
    }
    for (resourceIndex = 0; resourceIndex < 8; ++resourceIndex)
    {
        slot = resourceIndex & 7;
        selectedType = order[resourceIndex];
        switch (selectedType)
        {
        case 0 :
            new SetupAnimationOwner(&model0, slot);
            break;
        case 1 :
            new SetupAnimationOwner(&model1, slot);
            break;
        case 2 :
            new SetupAnimationOwner(&model2, slot);
            break;
        }
    }
}

void fn_1_1AFC();

/* Deletes all setup animation objects. */
AnimationSetupOwner::~AnimationSetupOwner()
{
    fn_1_1AFC();
}

void fn_1_1BE8();

/* Updates and rotates the setup animation list. */
void AnimationSetupOwner::update()
{
    fn_1_1BE8();
}

/* Clears the requested flag. */
void AnimationSetupOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

extern "C" {
ListRoot<SetupAnimationOwner> animationSetupCallbacks;
}

/* Links a model instance and initializes its position, motion and callback. */
SetupAnimationOwner::SetupAnimationOwner(const OwnedModel *sequenceIndex, s32 initialMode)
    : mode(initialMode), model(*sequenceIndex)
{
    Callback finalCallback;
    s16 currentModel;

    radialValue = 1.0f;
    scale = 0.0f;
    insert4(&animationSetupCallbacks);

    fn_1_123C(&motionPosition, mode);

    currentModel = model;
    fn_80026A44(currentModel, (1 << 0));
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_800269E8(currentModel, ((1 << 30) | (1 << 0)));
    fn_8002D414(currentModel, RandomFloat() * fn_8002D530(currentModel));
    fn_80026B8C(currentModel, 2);
    fn_80026B64(currentModel, 1);
    angle = (360.0f * RandomFloat()) - 180.0f;
    model.setPosition(&motionPosition);
    fn_1_1D28();

    finalCallback = &SetupAnimationOwner::fn_1_1F14;
    callback = finalCallback;
}

/* Deletes every setup animation object. */
void fn_1_1AFC()
{
    while (!animationSetupCallbacks.empty())
        delete animationSetupCallbacks.back();
}

void fn_1_1C6C(SetupAnimationOwner *);

/* Updates the setup animation objects and moves the first entry to the back. */
void fn_1_1BE8()
{
    animationSetupCallbacks.visit(fn_1_1C6C);
    SetupAnimationOwner *node = animationSetupCallbacks.front();
    if (node != animationSetupCallbacks.end_node()) {
        node->unlink();
        node->insert4(&animationSetupCallbacks);
    }
}

/* Runs the stored callback and applies the model transform. */
void fn_1_1C6C(SetupAnimationOwner *self)
{
    (self->*self->callback)();
    self->fn_1_1CAC();
}

// Turns the model to its angle and moves it to its position.
/* Applies the shared animation transform. */
template<class Owner> static inline void applyAnimationTransform(Owner &owner)
{
    f64 height = owner.angle;
    Vec position;
    position.x = 0.0f;
    position.y = height;
    position.z = 0.0f;
    owner.model.setRotation(&position);
    owner.model.setPosition(&owner.motionPosition);
}

void SetupAnimationOwner::fn_1_1CAC()
{
    applyAnimationTransform(*this);
}

/* Chooses the initial destination and easing targets. */
void SetupAnimationOwner::fn_1_1D28()
{
    s32 index;

    framesRemaining = (s32)(secondsToFrames(1.0f) + RandomFloat() * secondsToFrames(2.0f));
    index = mode;
    fn_1_123C(&destination, index);
    f32 scale = 1.0f;
    scaleTarget = (f32)(scale * RandomFloat() + 0.2 * (f64)scale);
    radialTarget = 0.5f + RandomFloat();
}

/* Chooses the next position, scale and radial targets for a setup model. */
static inline void chooseSetupDestination(SetupAnimationOwner *owner)
{
    f32 angle;
    f32 radius;
    s32 sequence;
    f32 scale;

    owner->framesRemaining = (s32)(secondsToFrames(1.0f) + RandomFloat() * secondsToFrames(2.0f));
    sequence = owner->mode;
    angle = 40.0f
          + (((f32)(sequence * 10)
              + (RandomFloat() * 30.0f))
             - 180.0f);
    radius = 1200.0f + (600.0f * RandomFloat());
    owner->destination.x = (f32)((f64)radius
        * cos((M_PI * (f64)angle) / 180.0));
    owner->destination.z = (f32)((f64)radius
        * sin((M_PI * (f64)angle) / 180.0));
    owner->destination.y = 50.0f + (200.0f * RandomFloat());
    scale = 1.0f;
    owner->scaleTarget = (f32)(scale * RandomFloat()
        + (0.2 * (f64)scale));
    owner->radialTarget = 0.5f + RandomFloat();
}

/* Eases the model scale, radius and heading toward its current destination. */
void SetupAnimationOwner::fn_1_1F14()
{
    f32 desiredAngle;

    if (--framesRemaining < 0) {
        chooseSetupDestination(this);
    }

    radialValue += 0.05f * (radialTarget - radialValue);
    scale += 0.05f * (scaleTarget - scale);
    if (fabs((f64)(destination.x - motionPosition.x)) < 0.1
        && fabs((f64)(destination.z - motionPosition.z)) < 0.1) {
        desiredAngle = angle;
    } else {
        desiredAngle = (f32)(180.0
            * (atan2((f64)(destination.x - motionPosition.x),
                     (f64)(destination.z - motionPosition.z))
               / M_PI));
    }
    angle += limitedDegreeDifference(desiredAngle, angle, scale);
    fn_1_2290();
}

/* Moves the model along its heading and eases its height toward the destination. */
void SetupAnimationOwner::fn_1_2290()
{
    Mtx rotation;
    Vec source;
    Vec transformed;
    static f64 framesPerSecond = 60.0;
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = (f32)((f64)(100.0f * radialValue) / framesPerSecond);
    PSMTXRotRad(rotation, 'y', MTXDegToRad(angle));
    PSMTXMultVec(rotation, &source, &transformed);
    transformed.y = 0.01f * (destination.y - motionPosition.y);
    motionPosition.x += transformed.x;
    motionPosition.y += transformed.y;
    motionPosition.z += transformed.z;
}

/* Chooses a random radial position for the indexed model. */
void fn_1_23C4(Vec *position, s32 index)
{
    f32 angle;
    f32 distance;
    angle = 40.0f
          + (((f32)(index * 10) + (30.0f * RandomFloat()))
             - 180.0f);
    distance = 1200.0f + (600.0f * RandomFloat());
    position->x = distance * cos((M_PI * (f64)angle) / 180.0);
    position->z = distance * sin((M_PI * (f64)angle) / 180.0);
    position->y = 50.0f + (400.0f * RandomFloat());
}

/* Creates sixteen moving animation objects from the shared model. */
AnimationOwner::AnimationOwner() : model(DATANUM(0x6b0000, 17))
{
    s16 modelId;
    s32 slot;
    s32 index;

    modelId = model;
    fn_800269E8(modelId, (1 << 0));
    for (index = 0; index < 0x10; index++) {
        slot = index & 7;
        new PlayAnimationOwner(&model, slot);
    }
}

void fn_1_2AB4();

/* Deletes all moving animation objects. */
AnimationOwner::~AnimationOwner()
{
    fn_1_2AB4();
}

void fn_1_2BA0();

/* Updates and rotates the moving animation list. */
void AnimationOwner::update()
{
    fn_1_2BA0();
}

/* Clears the requested flag. */
void AnimationOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

extern "C" {
ListRoot<PlayAnimationOwner> animationCallbacks;
}

/* Links a model instance and initializes its position, motion and callback. */
PlayAnimationOwner::PlayAnimationOwner(const OwnedModel *sequenceIndex, s32 initialMode)
    : mode(initialMode), model(*sequenceIndex)
{
    Callback finalCallback;
    s16 currentModel;

    radialValue = 1.0f;
    scale = 0.0f;
    insert4(&animationCallbacks);

    fn_1_23C4(&motionPosition, mode);

    currentModel = model;
    fn_80026A44(currentModel, (1 << 0));
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_800269E8(currentModel, ((1 << 30) | (1 << 0)));
    fn_8002D414(currentModel, RandomFloat() * fn_8002D530(currentModel));
    fn_80026B8C(currentModel, 2);
    fn_80026B64(currentModel, 1);
    angle = (360.0f * RandomFloat()) - 180.0f;
    model.setPosition(&motionPosition);
    fn_1_2CE0();

    finalCallback = &PlayAnimationOwner::fn_1_2EDC;
    callback = finalCallback;
}

/* Deletes every moving animation object. */
void fn_1_2AB4()
{
    while (!animationCallbacks.empty())
        delete animationCallbacks.back();
}

void fn_1_2C24(PlayAnimationOwner *);

/* Updates the moving animation objects and moves the first entry to the back. */
void fn_1_2BA0()
{
    animationCallbacks.visit(fn_1_2C24);
    PlayAnimationOwner *node = animationCallbacks.front();
    if (node != animationCallbacks.end_node()) {
        node->unlink();
        node->insert4(&animationCallbacks);
    }
}

/* Runs the stored callback and applies the model transform. */
void fn_1_2C24(PlayAnimationOwner *self)
{
    (self->*self->callback)();
    self->fn_1_2C64();
}

// Turns the model to its angle and moves it to its position.
void PlayAnimationOwner::fn_1_2C64()
{
    applyAnimationTransform(*this);
}

/* Chooses the next destination, scale and radial targets for a moving model. */
void PlayAnimationOwner::fn_1_2CE0()
{

    framesRemaining = (s32)(secondsToFrames(1.0f) + RandomFloat() * secondsToFrames(2.0f));
    fn_1_23C4(&destination, mode);
    f32 scale = 2.0f;
    scaleTarget = (f32)(scale * RandomFloat() + 0.2 * (f64)scale);
    radialTarget = 0.5f + 3.0f * RandomFloat();

}

/* Eases the model scale, radius and heading toward its current destination. */
void PlayAnimationOwner::fn_1_2EDC()
{
    f32 desiredAngle;

    if (--framesRemaining < 0) {
        fn_1_2CE0();
    }

    radialValue += 0.05f * (radialTarget - radialValue);
    scale += 0.05f * (scaleTarget - scale);
    if (fabs((f64)(destination.x - motionPosition.x)) < 0.1
        && fabs((f64)(destination.z - motionPosition.z)) < 0.1) {
        desiredAngle = angle;
    } else {
        desiredAngle = (f32)(180.0
            * (atan2((f64)(destination.x - motionPosition.x),
                     (f64)(destination.z - motionPosition.z))
               / M_PI));
    }
    angle += limitedDegreeDifference(desiredAngle, angle, scale);
    fn_1_3268();
}

/* Moves the model along its heading and eases its height toward the destination. */
void PlayAnimationOwner::fn_1_3268()
{
    Mtx rotation;
    Vec source;
    Vec transformed;
    static f64 framesPerSecond = 60.0;
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = (f32)((f64)(100.0f * radialValue) / framesPerSecond);
    PSMTXRotRad(rotation, 'y', MTXDegToRad(angle));
    PSMTXMultVec(rotation, &source, &transformed);
    transformed.y = 0.01f * (destination.y - motionPosition.y);
    motionPosition.x += transformed.x;
    motionPosition.y += transformed.y;
    motionPosition.z += transformed.z;
}
