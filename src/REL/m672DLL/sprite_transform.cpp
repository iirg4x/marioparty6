#include "REL/framework/event.h"
#include "REL/framework/sound.h"
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

#include "REL/m672DLL/animated_owner.h"

#include "REL/m672DLL/resources.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/frame_time.h"
#include "REL/m672DLL/member_handler.h"

extern "C" {

extern const unsigned long lbl_1_rodata_3C8[];
extern const EffectVector lbl_1_rodata_3DC, lbl_1_rodata_3E8, lbl_1_rodata_3F8;
extern const GXColor defaultLightColor;
}

extern "C" {
const unsigned long lbl_1_rodata_3C8[5] = {DATANUM(0x830000, 0x13), DATANUM(0x830000, 0x18),
    DATANUM(0x830000, 0x15), DATANUM(0x830000, 0x19), DATANUM(0x830000, 0x1A)};
const EffectVector lbl_1_rodata_3DC = {{0.0f, 0.0f, 100.0f}};
const EffectVector lbl_1_rodata_3E8 = {{0.0f, 1.0f, -1.0f}};
const GXColor defaultLightColor = {200, 200, 200, 255};
const EffectVector lbl_1_rodata_3F8 = {{0.7f, 0.7f, 0.7f}};
}

// Initializes model motions, handlers, lighting, position/scale, and the initial motion.
AnimatedOwner::AnimatedOwner(int resource, EffectVector *position, float scale)
    : GameObject(4), ModelMotion(resource), AnimatedOwnerSingleton(), position(*position),
        state(-1), frame(0), active(0), states(0), events(1)
{
    EffectVector lightPosition;
    EffectVector lightDirection;
    EffectVector modelRotation;
    short modelId;
    this->setLayer(3);
    this->setPosition(&position->value);
    this->setRotationY(scale);
    this->setUniformScale(1.0f);
    registerMemberHandler<AnimatedOwner>(&this->states,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9DFC), this), 0);
    registerMemberHandler<AnimatedOwner>(&this->states,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9E00), this), 1);
    registerMemberHandler<AnimatedOwner>(&this->states,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9E04), this), 2);
    registerMemberHandler<AnimatedOwner>(&this->states,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9F38), this), 3);
    registerMemberHandler<AnimatedOwner>(&this->states,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9F7C), this), 4);
    registerMemberHandler<AnimatedOwner>(&this->events,
        bindMember(std::mem_fun(&AnimatedOwner::fn_1_9E08), this), 2);
    addMotions(lbl_1_rodata_3C8, 5);
    this->fn_1_9AFC(DATANUM(0x830000, 0x13), 1);
    this->setUpdate(&AnimatedOwner::fn_1_9B90);
    lightPosition = lbl_1_rodata_3DC;
    lightDirection = lbl_1_rodata_3E8;
    GXColor lightColor = defaultLightColor;
    modelId = static_cast<ModelMotion *>(this)->id();
    this->lightId = fn_800288B4(modelId, &lightPosition.value, &lightDirection.value, &lightColor);
    fn_800296E0(modelId, this->lightId, 1);
    fn_80028B58(modelId, this->lightId);
    modelRotation = lbl_1_rodata_3F8;
    this->setAmbient(&modelRotation.value);
    Resources *result = &Resources::getChecked();
    fn_80029E44(modelId, *result);
}

// Releases the model light associated with this animated owner.
AnimatedOwner::~AnimatedOwner() {
    fn_80028CB8(static_cast<ModelMotion *>(this)->id(), lightId);
}

// Invokes the selected animated-owner update callback when present.
void AnimatedOwner::update() { if (updateCallback) (this->*updateCallback)(); }

// Shifts to a motion when it differs from the current state and records the new state.
void AnimatedOwner::fn_1_9AFC(unsigned int motion, unsigned char enabled)
{
    u32 flags;

    if (this->state != motion) {
        if (enabled == 0) {
            flags = 0;
        } else {
            flags = ((1 << 30) | (1 << 0));
        }
        this->shiftMotion(motion, 8.0f,
                   0.0f, flags);
        this->state = motion;
    }
}

// Restores motion 19 after completion and counts down the pending event frame timer.
void AnimatedOwner::fn_1_9B90()
{
    HU3D_MODELID motionModelId = *this;

    if (state != DATANUM(0x830000, 0x13) &&
        fn_8002D75C(motionModelId)) {
        // This nested test repeats the enclosing state != motion-19 condition.
        if (state != DATANUM(0x830000, 0x13)) {
            shiftMotion(DATANUM(0x830000, 0x13), 8.0f,
                       0.0f, ((1 << 30) | (1 << 0)));
            state = DATANUM(0x830000, 0x13);
        }
    }

    if (frame != 0) {
        frame--;
        if (frame == 0) {
            sendEvent(1, 4, 0.0f);
        }
    }
}

// Empty animated-owner callback.
void fn_1_9C8C(void)
{
}

// Empty animated-owner callback.
void fn_1_9C90(void)
{
}

// Advances the active motion state and starts the timed sound/update path.
void AnimatedOwner::fn_1_9C94()
{
    HU3D_MODELID motionModelId = *this;

    if (active == 0) {
        if (fn_8002D75C(motionModelId) != 0 ||
            state == DATANUM(0x830000, 0x13)) {
            if (state != DATANUM(0x830000, 0x15)) {
                shiftMotion(DATANUM(0x830000, 0x15),
                           8.0f, 0.0f, 0);
                state = DATANUM(0x830000, 0x15);
            }
            active = 1;
        }
    } else if (fn_8002D75C(motionModelId) != 0) {
        Vec position = {0.0f, 0.0f, -1350.0f};
        setUpdate(&AnimatedOwner::fn_1_9B90);
        frame = (unsigned int)secondsToFrames(0.2f);
        soundPlayAt(1, 0x8A7, &position);
    }
}

// Empty animated-owner callback used after result events.
void AnimatedOwner::fn_1_9DF8()
{
}

// Empty state callback for handler key 0.
void AnimatedOwner::fn_1_9DFC(EventMessage *)
{
}

// Empty state callback for handler key 1.
void AnimatedOwner::fn_1_9E00(EventMessage *)
{
}

// Empty state callback for handler key 2.
void AnimatedOwner::fn_1_9E04(EventMessage *)
{
}

// Shifts to motion 24, plays sounds, and returns to the motion-19 update path.
void AnimatedOwner::fn_1_9E08(EventMessage *)
{
    HU3D_MODELID motionModelId = *this;
    if (!fn_8002D75C(motionModelId) &&
        state != DATANUM(0x830000, 0x13)) {
        return;
    }

    if (state != DATANUM(0x830000, 0x18)) {
        shiftMotion(DATANUM(0x830000, 0x18), 8.0f,
                   0.0f, 0);
        state = DATANUM(0x830000, 0x18);
    }

    Vec effectPosition = position.value;
    soundPlayAt(1, 0x3CC, &effectPosition);
    soundPlayAtDelayed(1, 0x3C9, &position.value, 0x14);
    setUpdate(&AnimatedOwner::fn_1_9B90);
}

// Selects the active-motion update callback.
void AnimatedOwner::fn_1_9F38(EventMessage *)
{
    setUpdate(&AnimatedOwner::fn_1_9C94);
}

// Selects the result callback and plays the sound/motion sequence for result code 4 or other codes.
void AnimatedOwner::fn_1_9F7C(EventMessage *message)
{
    s32 *resultCode;

    setUpdate(&AnimatedOwner::fn_1_9DF8);
    resultCode = static_cast<s32 *>(message->context);
    if (*resultCode == 4) {
        if (this->state != DATANUM(0x830000, 0x19)) {
            this->shiftMotion(DATANUM(0x830000, 0x19),
                       8.0f, 0.0f, 0);
            this->state = DATANUM(0x830000, 0x19);
        }
        soundPlayAtDelayed(1, 0x3CB, &this->position.value, 0x1E);
        soundPlayAtDelayed(1, 0x3CD, &this->position.value, 0x6E);
    } else {
        if (this->state != DATANUM(0x830000, 0x1A)) {
            this->shiftMotion(DATANUM(0x830000, 0x1A),
                       8.0f, 0.0f, 0);
            this->state = DATANUM(0x830000, 0x1A);
        }
        soundPlayAt(1, 0x3CF, &this->position.value);
        soundPlayAtDelayed(1, 0x3C9, &this->position.value, 0x41);
        soundPlayAtDelayed(1, 0x3C9, &this->position.value, 0x5A);
        soundPlayAtDelayed(1, 0x3CC, &this->position.value, 0x78);
    }
}
