#include "REL/m648DLL/time_conversion.h"
#include "REL/m648DLL/angle_difference.h"
#include "humath.h"
unsigned long RandomUnsigned();
float RandomFloat();
extern "C" {
#include "dolphin/mtx.h"
#include <math.h>
#include "game/hu3d.h"
#include "REL/m648DLL/singleton_32930.h"
}
#include "REL/m648DLL/motion_lists.h"
#include "datadir_enum.h"
#include "REL/m648DLL/animation_owner_17f8.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

#include "include/game/audio.h"

#include "include/game/gamemes.h"

#include "REL/m648DLL/framework/linknode.h"

#include "REL/m648DLL/framework/model.h"

#include "REL/framework/linknode.h"
#include "REL/framework/minigame_singleton.h"
#include "REL/m648DLL/list_object.h"

extern "C" {
#include "dolphin/types.h"

typedef Vec PositionOwner123C;

}

template <class CallbackOwner> struct AnimationCallbackRoot : Node {
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    DrawNode *back() { return static_cast<DrawNode *>(last_node()); }
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    DrawNode *front() { return static_cast<DrawNode *>(first_node()); }
};
typedef AnimationCallbackRoot<SetupAnimationOwner> AnimationSetupRoot;
typedef AnimationCallbackRoot<PlayAnimationOwner> AnimationRoot;

class AnimationSetupOwner : public RequestObject, public MinigameSingleton<AnimationSetupOwner> {
public:
    OwnedModel model0, model1, model2;
    AnimationSetupOwner();
    virtual ~AnimationSetupOwner();
    virtual void clearRequest(int request);
    virtual void update();
};

class AnimationOwner : public RequestObject, public MinigameSingleton<AnimationOwner> {
public:
    OwnedModel model;
    AnimationOwner();
    virtual ~AnimationOwner();
    virtual void clearRequest(int request);
    virtual void update();
};

extern "C" {

extern void __ct__10OwnedModelFUi(void *object, s32 flags);
extern void *__nw__FUl(s32 size);
extern void *fn_1_10208(s32 size);
extern const char lbl_1_data_5C[], lbl_1_data_B4[];

void fn_1_123C(PositionOwner123C *position, s32 index);

}

extern "C" {
void fn_1_23C4(PositionOwner123C *position, s32 index);

extern void __ct__10OwnedModelFUi(void *object, s32 flags);

extern void *__nw__FUl(s32 size);
extern char lbl_1_data_A0[];

}

void fn_1_123C(PositionOwner123C *position, s32 index)
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
    {
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
}

extern const char lbl_1_data_B4[];

extern const char lbl_1_data_5C[];

void fn_1_1AFC();

extern "C" void Replace__5ModelFs(void *, s32);

extern "C" void *__dt__4NodeFv(void *, s16);

void __dl__FPv(void *);

struct AnimationSetupView : Node {
    const void *vtable;
    s32 flags;
    LinkedModel models[3];
};

AnimationSetupOwner::~AnimationSetupOwner()
{
    fn_1_1AFC();
}

extern "C" {

void fn_1_19C(void);
}

void fn_1_1BE8();

void AnimationSetupOwner::update()
{
    fn_1_1BE8();
}

extern "C" {

struct RequestOwner {
    u32 nodeData[3];
    u32 pending;
    u8 active;
};
}

void AnimationSetupOwner::clearRequest(int request)
{
    state &= ~(1UL << request);
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

extern AnimationSetupRoot animationSetupCallbacks;
}

SetupAnimationOwner::SetupAnimationOwner(const LinkedModel *sequenceIndex, s32 initialMode)
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
    model.fn_1_33A3C(&motionPosition);
    fn_1_1D28();

    finalCallback = &SetupAnimationOwner::fn_1_1F14;
    callback = finalCallback;
}

SetupAnimationNode::~SetupAnimationNode() {}

void fn_1_1AFC()
{
    while (!animationSetupCallbacks.empty())
        delete animationSetupCallbacks.back();
}

extern "C" { void fn_1_6ADC(Node *); }

extern "C" { void fn_1_7A60(DrawRoot *, void (*)(Node *)); }

extern "C" { void *fn_1_8644(void *, const long *); }

void *operator new(unsigned long);

void fn_1_1C6C(SetupAnimationOwner *);

extern "C" { void fn_1_33FC(AnimationSetupRoot *, void (*)(SetupAnimationOwner *)); }

void fn_1_1BE8()
{
    fn_1_33FC(&animationSetupCallbacks, fn_1_1C6C);
    DrawNode *node = animationSetupCallbacks.front();
    if (node != animationSetupCallbacks.end_node()) {
        node->unlink();
        node->insert4(&animationSetupCallbacks);
    }
}

void fn_1_1C6C(SetupAnimationOwner *self)
{
    (self->*self->callback)();
    self->fn_1_1CAC();
}

// Turns the model to its angle and moves it to its position.
void SetupAnimationOwner::fn_1_1CAC()
{
    f64 height = angle;
    Vec position;
    position.x = 0.0f;
    position.y = height;
    position.z = 0.0f;
    model.fn_1_33ACC(&position);
    model.fn_1_33A3C(&motionPosition);
}

extern "C" {

typedef Vec SequencePosition;

typedef struct SequenceModel {
    s16 modelId;
} SequenceModel;

}

static inline f32 Constant120_0f(void) { return 120.0f; }

extern "C" {

extern void fn_1_1BCF4(void *object);

extern void fn_1_1BDB8(void *object, void *list);

extern void fn_1_1DBA4(void *model, void *position, f32 angle);

}

/* Chooses the initial destination and easing targets. */
void SetupAnimationOwner::fn_1_1D28()
{
    s32 index;

    framesRemaining = (s32)(60.0f + RandomFloat() * Constant120_0f());
    index = mode;
    fn_1_123C(&destination, index);
    {
        f32 scale = 1.0f;
        scaleTarget = (f32)(scale * RandomFloat() + 0.2 * (f64)scale);
    }
    radialTarget = 0.5f + RandomFloat();
}

/* Chooses a new orbit destination and easing targets for the moving variant. */
static inline void fn_1_2CE0_for_2EDC(PlayAnimationOwner *owner)
{
    f32 angle;
    f32 radius;
    s32 sequence;
    f32 scale;

    owner->framesRemaining = (s32)(60.0f
        + (RandomFloat() * Constant120_0f()));
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
    owner->destination.y = 50.0f + (400.0f * RandomFloat());
    scale = 2.0f;
    owner->scaleTarget = (f32)(scale * RandomFloat()
        + (0.2 * (f64)scale));
    owner->radialTarget = 0.5f + (3.0f * RandomFloat());
}

static inline void fn_1_1D28_for_1F14(SetupAnimationOwner *owner)
{
    f32 angle;
    f32 radius;
    s32 sequence;
    f32 scale;

    owner->framesRemaining = (s32)(60.0f
        + (RandomFloat() * Constant120_0f()));
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

void SetupAnimationOwner::fn_1_1F14()
{
    f32 desiredAngle;

    if (--framesRemaining < 0) {
        fn_1_1D28_for_1F14(this);
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
    angle += (f32)fn_1_377C(desiredAngle, angle, scale);
    fn_1_2290();
}

extern "C" {

extern s8 lbl_1_bss_60;

extern f64 lbl_1_bss_58;

}

void SetupAnimationOwner::fn_1_2290()
{
    Mtx rotation;
    Vec source;
    Vec transformed;
    if (lbl_1_bss_60 == 0) {
        lbl_1_bss_58 = 60.0;
        lbl_1_bss_60 = 1;
    }
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = (f32)((f64)(100.0f * radialValue) / lbl_1_bss_58);
    PSMTXRotRad(rotation, 'y', MTXDegToRad(angle));
    PSMTXMultVec(rotation, &source, &transformed);
    transformed.y = 0.01f * (destination.y - motionPosition.y);
    motionPosition.x += transformed.x;
    motionPosition.y += transformed.y;
    motionPosition.z += transformed.z;
}

void fn_1_23C4(PositionOwner123C *position, s32 index)
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

struct AnimationView : Node {
    const void *vtable;
    s32 flags;
    LinkedModel model;
};

AnimationOwner::~AnimationOwner()
{
    fn_1_2AB4();
}

void fn_1_2BA0();

void AnimationOwner::update()
{
    fn_1_2BA0();
}

void AnimationOwner::clearRequest(int request)
{
    state &= ~(1UL << request);
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

extern AnimationRoot animationCallbacks;

}

PlayAnimationOwner::PlayAnimationOwner(const LinkedModel *sequenceIndex, s32 initialMode)
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
    model.fn_1_33A3C(&motionPosition);
    fn_1_2CE0();

    finalCallback = &PlayAnimationOwner::fn_1_2EDC;
    callback = finalCallback;
}

PlayAnimationNode::~PlayAnimationNode() {}

void fn_1_2AB4()
{
    while (!animationCallbacks.empty())
        delete animationCallbacks.back();
}

void fn_1_2C24(PlayAnimationOwner *);

extern "C" { void fn_1_339C(AnimationRoot *, void (*)(PlayAnimationOwner *)); }

void fn_1_2BA0()
{
    fn_1_339C(&animationCallbacks, fn_1_2C24);
    DrawNode *node = animationCallbacks.front();
    if (node != animationCallbacks.end_node()) {
        node->unlink();
        node->insert4(&animationCallbacks);
    }
}

void fn_1_2C24(PlayAnimationOwner *self)
{
    (self->*self->callback)();
    self->fn_1_2C64();
}

// Turns the model to its angle and moves it to its position.
void PlayAnimationOwner::fn_1_2C64()
{
    f64 height = angle;
    Vec position;
    position.x = 0.0f;
    position.y = height;
    position.z = 0.0f;
    model.fn_1_33ACC(&position);
    model.fn_1_33A3C(&motionPosition);
}

static inline void fn_1_1D28_for_2CE0(PlayAnimationOwner *owner)
{
    owner->framesRemaining = (s32)(60.0f + RandomFloat() * Constant120_0f());
    fn_1_23C4(&owner->destination, owner->mode);
    {
        f32 scale = 2.0f;
        owner->scaleTarget = (f32)(scale * RandomFloat() + 0.2 * (f64)scale);
    }
    owner->radialTarget = 0.5f + 3.0f * RandomFloat();
}

void PlayAnimationOwner::fn_1_2CE0()
{
    fn_1_1D28_for_2CE0(this);
}

void PlayAnimationOwner::fn_1_2EDC()
{
    f32 desiredAngle;

    if (--framesRemaining < 0) {
        fn_1_2CE0_for_2EDC(this);
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
    angle += (f32)fn_1_377C(desiredAngle, angle, scale);
    fn_1_3268();
}

extern "C" {

extern s8 lbl_1_bss_80;

extern f64 lbl_1_bss_78;

}

void PlayAnimationOwner::fn_1_3268()
{
    Mtx rotation;
    Vec source;
    Vec transformed;
    if (lbl_1_bss_80 == 0) {
        lbl_1_bss_78 = 60.0;
        lbl_1_bss_80 = 1;
    }
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = (f32)((f64)(100.0f * radialValue) / lbl_1_bss_78);
    PSMTXRotRad(rotation, 'y', MTXDegToRad(angle));
    PSMTXMultVec(rotation, &source, &transformed);
    transformed.y = 0.01f * (destination.y - motionPosition.y);
    motionPosition.x += transformed.x;
    motionPosition.y += transformed.y;
    motionPosition.z += transformed.z;
}

extern "C" {
void fn_1_345C(Node *current, Node *sentinel, void (*callback)(PlayAnimationOwner *));
void fn_1_34BC(Node *current, Node *sentinel, void (*callback)(SetupAnimationOwner *));
}

void fn_1_339C(AnimationRoot *list, void (*visitor)(PlayAnimationOwner *))
{
    fn_1_345C(list->front(), list->end_node(), visitor);
}

void fn_1_33FC(AnimationSetupRoot *list, void (*visitor)(SetupAnimationOwner *))
{
    fn_1_34BC(list->front(), list->end_node(), visitor);
}

void fn_1_345C(Node *current, Node *sentinel, void (*callback)(PlayAnimationOwner *))
{
    while (current != sentinel) {
        callback(static_cast<PlayAnimationOwner *>(current));
        Node *next = current->link0;
        current = next;
    }
}

void fn_1_34BC(Node *current, Node *sentinel, void (*callback)(SetupAnimationOwner *))
{
    while (current != sentinel) {
        callback(static_cast<SetupAnimationOwner *>(current));
        Node *next = current->link0;
        current = next;
    }
}

extern "C" {

s32 lbl_1_bss_38 = secondsToFrames(2L);
s32 lbl_1_bss_3C = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_40 = (s32)secondsToFrames(3.5);
AnimationSetupRoot animationSetupCallbacks;
f64 lbl_1_bss_58;
s8 lbl_1_bss_60;
AnimationRoot animationCallbacks;
f64 lbl_1_bss_78;
s8 lbl_1_bss_80;
}
