#include "REL/m672DLL/event.h"
extern "C" {
#include "math.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
extern "C" {
#include <math.h>
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/data.h"
}

#include "REL/m672DLL/sprite_transform.h"
#include "REL/m672DLL/owner_9314.h"
extern "C" {
}

#include "REL/m672DLL/resources.h"
#include "REL/framework/event_callback_inline.h"
extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
void fn_1_1A5B0(void *, int);
void fn_1_1FA60(Model9314 *, int);
void fn_1_198E0(EventHandlerManager *, int);
void fn_1_1EE80(Model9314 *, short);
void fn_1_1EC0C(Model9314 *, const Vec *);
void fn_1_1ED2C(Model9314 *, float);
void fn_1_1EE00(Model9314 *, float);
void fn_1_1FCB4(Model9314 *, const unsigned long *, unsigned long);
void fn_1_1F00C(Model9314 *, const Vec *);

extern const u8 lbl_1_data_33C[];
extern Member9314 lbl_1_data_2C8, lbl_1_data_2D4, lbl_1_data_2E0;
extern const unsigned long lbl_1_rodata_3C8[];
extern const VecValue lbl_1_rodata_3DC, lbl_1_rodata_3E8, lbl_1_rodata_3F8;
extern const GXColor lightColor9314;
}
template<class Receiver> void fn_1_A108(StateEventTable *, const StateCallbackBinding &, int);

static inline StateMemberMethod MakeMember9314(Member9314 fn)
{
    return StateMemberMethod(fn);
}
static inline StateCallbackBinding Bind9314(const StateMemberMethod &fn, AnimatedOwner *owner)
{
    return StateCallbackBinding(fn, owner);
}

extern "C" {
const unsigned long lbl_1_rodata_3C8[5] = {0x830013, 0x830018, 0x830015, 0x830019, 0x83001A};
const VecValue lbl_1_rodata_3DC = {{0.0f, 0.0f, 100.0f}};
const VecValue lbl_1_rodata_3E8 = {{0.0f, 1.0f, -1.0f}};
const GXColor lightColor9314 = {200, 200, 200, 255};
const VecValue lbl_1_rodata_3F8 = {{0.7f, 0.7f, 0.7f}};
}

AnimatedOwner::AnimatedOwner(int resource, VecValue *position, float scale)
    : GameObject(4), ModelMotion(resource), AnimatedOwnerSingleton(), position(*position), state(-1), frame(0), active(0), states(0), events(1)
{
    VecValue lightPosition;
    VecValue lightDirection;
    VecValue modelRotation;
    short modelId;
    this->fn_1_33CB0(3);
    this->fn_1_33A3C(&position->value);
    this->fn_1_33B5C(scale);
    this->fn_1_33C30(1.0f);
    fn_1_A108<AnimatedOwner>(&this->states,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9DFC), this), 0);
    fn_1_A108<AnimatedOwner>(&this->states,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9E00), this), 1);
    fn_1_A108<AnimatedOwner>(&this->states,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9E04), this), 2);
    fn_1_A108<AnimatedOwner>(&this->states,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9F38), this), 3);
    fn_1_A108<AnimatedOwner>(&this->states,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9F7C), this), 4);
    fn_1_A108<AnimatedOwner>(&this->events,
        Bind9314(MakeMember9314(&AnimatedOwner::fn_1_9E08), this), 2);
    fn_1_1FCB4(&static_cast<ModelMotion &>(*this), lbl_1_rodata_3C8, 5);
    this->fn_1_9AFC(DATANUM(0x830000, 19), 1);
    this->setUpdate(&AnimatedOwner::fn_1_9B90);
    lightPosition = lbl_1_rodata_3DC;
    lightDirection = lbl_1_rodata_3E8;
    GXColor lightColor = lightColor9314;
    modelId = static_cast<ModelMotion *>(this)->getId();
    this->lightId = fn_800288B4(modelId, &lightPosition.value, &lightDirection.value, &lightColor);
    fn_800296E0(modelId, this->lightId, 1);
    fn_80028B58(modelId, this->lightId);
    modelRotation = lbl_1_rodata_3F8;
    this->fn_1_33E3C(&modelRotation.value);
    Resources *result = &Resources::getChecked();
    fn_80029E44(modelId, *result);
}

AnimatedOwner::~AnimatedOwner() {
    fn_80028CB8(static_cast<ModelMotion *>(this)->getId(), lightId);
}

void AnimatedOwner::update() { if (updateCallback) (this->*updateCallback)(); }

extern "C" {
void fn_1_11218(u32, u32, Vec *);
void fn_1_114E0(u32, u32, Vec *, u32);
}
template <class Time> Time secondsToFrames(Time seconds);

struct MotionManager { s32 state; };
struct MotionContext {
    u8 unknown00[8]; /* No code names this field. */
    MotionManager *manager;
};

void AnimatedOwner::fn_1_9AFC(unsigned int motion, unsigned char enabled)
{
    u32 flags;

    if (this->state != motion) {
        if (enabled == 0) {
            flags = 0;
        } else {
            flags = ((1 << 30) | (1 << 0));
        }
        this->fn_1_1FDD8(motion, 8.0f,
                   0.0f, flags);
        this->state = motion;
    }
}

void AnimatedOwner::fn_1_9B90()
{
    HU3D_MODELID motionModelId = *this;

    if (state != DATANUM(0x830000, 19) &&
        fn_8002D75C(motionModelId)) {
        if (state != DATANUM(0x830000, 19)) {
            fn_1_1FDD8(DATANUM(0x830000, 19), 8.0f,
                       0.0f, ((1 << 30) | (1 << 0)));
            state = DATANUM(0x830000, 19);
        }
    }

    if (frame != 0) {
        frame--;
        if (frame == 0) {
            sendEvent(1, 4, 0.0f);
        }
    }
}

void fn_1_9C8C(void)
{
}

void fn_1_9C90(void)
{
}

void AnimatedOwner::fn_1_9C94()
{
    HU3D_MODELID motionModelId = *this;

    if (active == 0) {
        if (fn_8002D75C(motionModelId) != 0 ||
            state == DATANUM(0x830000, 19)) {
            if (state != DATANUM(0x830000, 21)) {
                fn_1_1FDD8(DATANUM(0x830000, 21),
                           8.0f, 0.0f, 0);
                state = DATANUM(0x830000, 21);
            }
            active = 1;
        }
    } else if (fn_8002D75C(motionModelId) != 0) {
        {
            Vec position = {0.0f, 0.0f, -1350.0f};
            Update9314 values = &AnimatedOwner::fn_1_9B90;
            updateCallback = values;
            frame = (unsigned int)secondsToFrames(0.2f);
            fn_1_11218(1, 0x8A7, &position);
        }
    }
}

void AnimatedOwner::fn_1_9DF8()
{
}

void AnimatedOwner::fn_1_9DFC(EventMessage *)
{
}

void AnimatedOwner::fn_1_9E00(EventMessage *)
{
}

void AnimatedOwner::fn_1_9E04(EventMessage *)
{
}

void AnimatedOwner::fn_1_9E08(EventMessage *)
{
    HU3D_MODELID motionModelId = *this;
    if (!fn_8002D75C(motionModelId) &&
        state != DATANUM(0x830000, 19)) {
        return;
    }

    if (state != DATANUM(0x830000, 24)) {
        fn_1_1FDD8(DATANUM(0x830000, 24), 8.0f,
                   0.0f, 0);
        state = DATANUM(0x830000, 24);
    }

    {
        Vec effectPosition = position.value;
        fn_1_11218(1, 0x3CC, &effectPosition);
    }
    fn_1_114E0(1, 0x3C9, &position.value, 0x14);
    {
        Update9314 values = &AnimatedOwner::fn_1_9B90;
        updateCallback = values;
    }
}

void AnimatedOwner::fn_1_9F38(EventMessage *)
{
    {
        Update9314 value = &AnimatedOwner::fn_1_9C94;
        updateCallback = value;
    }
}

void AnimatedOwner::fn_1_9F7C(EventMessage *message)
{
    Update9314 values = &AnimatedOwner::fn_1_9DF8;
    MotionManager *manager;

    this->updateCallback = values;
    manager = static_cast<MotionManager *>(message->context);
    if (manager->state == 4) {
        if (this->state != DATANUM(0x830000, 25)) {
            this->fn_1_1FDD8(DATANUM(0x830000, 25),
                       8.0f, 0.0f, 0);
            this->state = DATANUM(0x830000, 25);
        }
        fn_1_114E0(1, 0x3CB, &this->position.value, 0x1E);
        fn_1_114E0(1, 0x3CD, &this->position.value, 0x6E);
    } else {
        if (this->state != DATANUM(0x830000, 26)) {
            this->fn_1_1FDD8(DATANUM(0x830000, 26),
                       8.0f, 0.0f, 0);
            this->state = DATANUM(0x830000, 26);
        }
        fn_1_11218(1, 0x3CF, &this->position.value);
        fn_1_114E0(1, 0x3C9, &this->position.value, 0x41);
        fn_1_114E0(1, 0x3C9, &this->position.value, 0x5A);
        fn_1_114E0(1, 0x3CC, &this->position.value, 0x78);
    }
}

template<class Receiver>
void fn_1_A108(StateEventTable *owner, const StateCallbackBinding &payload, int key)
{
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}

/* Shared secondsToFrames template; native instances multiply by 60. */
template<class Time> Time secondsToFrames(Time seconds) { return 60 * seconds; }
