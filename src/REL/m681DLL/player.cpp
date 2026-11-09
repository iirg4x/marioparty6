#include "REL/m681DLL.h"
#include "REL/m681DLL/frame_time.h"
extern "C" {
#include "dolphin/types.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "game/flag.h"
#include "game/hsfex.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamework.h"
}

#include "datadir_enum.h"
#include "REL/m681DLL/owned_model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/m681DLL/work_6b44.h"
#include "REL/m681DLL/global_state.h"
#include "REL/m681DLL/character_sequence.h"
#include "REL/m681DLL/player_input.h"

static inline f32 accelerationPerFrame(f64 value)
{
    return (f32)((value * 100.0) / (60.0 * 60.0));
}
extern "C" {
u32 lbl_1_bss_98 = minutesToFrames(60L);
f32 lbl_1_bss_9C = accelerationPerFrame(9.80665);
u32 lbl_1_bss_A0 = secondsToFrames(2L);
u32 lbl_1_bss_A4 = secondsToFrames(3.5);
u32 lbl_1_bss_A8 = secondsToFrames(3.5);
}

void *operator new(unsigned long);
extern "C" {
}
s32 fn_1_1BDA4(s32 *);
extern "C" {
void selectCharacterMotion__FP11MotionOwnerUi(CharacterMotion *, unsigned int);
void setCharacterMotionOverlay__FP11MotionOwnersUi(CharacterMotion *, s16, unsigned int);
}
s16 fn_1_2508(s32);
extern "C" {

void shiftCharacterMotion__FP11MotionOwnerUiffUl(CharacterMotion *, u32, f32, f32, u32);
}
void sendEvent(int, unsigned long, float);

extern "C" {
}
s32 fn_1_1BDDC(s32 *);
extern "C" {
}
s32 fn_1_1BE08(s32 *);
extern "C" {

void createCharacterMotions__FP11MotionOwnerPUil(CharacterMotion *, const void *, s32);

}
void fn_1_7398(void *, s16, const void *);
extern "C" {

const u32 lbl_1_rodata_288[] = {
    0x930000, 0x930001, 0x930006, 0x8E00C1, 0x930019,
    0x8E0072, 0x8E00D5, 0x8E00C2, 0x8E00C5
};
}

typedef PlayerObject::Callback Callback533C;

PlayerObject::PlayerObject(s32 player, const PositionVector *initialState)
    : GameObject(0), PlayerConfigSelection(player, 1), EventListener()
{
    void *actor = (u8)fn_1_1BDDC(&sequence) != 0
          ? (ModeBase *)new ModeObject(fn_1_1BE08(&sequence))
          : (ModeBase *)new ModeToggle();
    this->resourceNode.reset(actor);
    void *accessory = new ModelResource(player);
    this->resource.reset(accessory);
    state = *initialState;
    firstAngle.set( 0.0f, 0);
    secondAngle.set( 0.0f, 0);
    void *firstItem = new OwnedModel(DATANUM(0x8c0000, 9));
    this->firstItem.reset(firstItem);
    void *createdSecondItem = new OwnedModel(DATANUM(0x8c0000, 10));
    this->secondItem.reset(createdSecondItem);
    elapsed = 0;
    fallFrames = 0;
    progress = 0.0f;
    velocity = 0.0f;
    firstValue = 2.0f;
    secondValue = 0.0f;
    animationScale = 0.2f;
    thirdValue = 6.0f;
    fourthValue = 0.0f;
    fifthValue = 0;
    count = 0;
    active = 1;
    sixthValue = 0;
    firstItemHook = fn_8005C0F0(
        (s16)fn_1_1BDA4(&sequence), 2, 1);
    secondItemHook = fn_8005C0F0(
        (s16)fn_1_1BDA4(&sequence), 2, 0);
    registerChannel(0);
    registerChannel(1);
    {
        Callback533C callback = &PlayerObject::fn_1_5E8C;
        this->callback = callback;
    }
    fn_1_33CB0(4);
    createCharacterMotions__FP11MotionOwnerPUil(&motion, &lbl_1_rodata_288, 9);
    selectCharacterMotion__FP11MotionOwnerUi(&motion, DATANUM(0x8e0000, 193));
    setCharacterMotionOverlay__FP11MotionOwnersUi(&motion, *static_cast<Model *>(this), DATANUM(0x8e0000, 194));
    fullAngle = fn_1_3464C();
    setCharacterMotionOverlay__FP11MotionOwnersUi(&motion, *static_cast<Model *>(this), DATANUM(0x8e0000, 213));
    fn_1_33950(((1 << 30) | (1 << 9)));
    void *firstItemModel = this->firstItem.value;
    fn_1_7398(((u8 *)this), 1, firstItemModel);
    void *secondItemModel = this->secondItem.value;
    fn_1_7398(((u8 *)this), 0, secondItemModel);
    modes.values[0] = 0;
    modes.values[1] = 0;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    fn_1_33A3C(&state.value);
    fn_1_33ACC(&rotation);
    fn_80060F14((s16)fn_1_1BDA4(&sequence), fn_1_2508(player));
}

OwnedModelOwner::~OwnedModelOwner() { delete value; }

ModelResourceOwner::~ModelResourceOwner() { delete value; }

ModeOwner::~ModeOwner()
{
    delete value;
}

PlayerObject::~PlayerObject() {}

extern "C" {

extern s32 fn_1_33950__5ModelFUl(void *object, s32 flags);

}
void fn_1_5A44(PlayerObject *object)
{
    s16 *modelFlags;
    s16 *modelFlagsAddress;
    s16 rawModelFlag;
    s16 modelFlagArgument;
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion__FP11MotionOwnerUi(&object->motion, DATANUM(0x8e0000, 197));
    fn_1_33950__5ModelFUl(&object->modelId, ((1 << 30) | (1 << 1)));
    modelFlags = (s16 *)object;
    if (modelFlags != 0) {
        modelFlags += 0x14;
    }
    modelFlagsAddress = modelFlags;
    rawModelFlag = (s16)*modelFlagsAddress;
    modelFlagArgument = rawModelFlag;
    setCharacterMotionOverlay__FP11MotionOwnersUi(&object->motion, modelFlagArgument, DATANUM(0x8e0000, 194));
    fn_1_33950__5ModelFUl(&object->modelId, ((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set( 0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set( halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set( 0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set( halfAngle, animationMode);
        break;
    }
}
extern "C" {

}
void fn_1_5C60(PlayerObject *object)
{
    s16 *modelFlags;
    s16 *modelFlagsAddress;
    s16 rawModelFlag;
    s16 modelFlagArgument;
    s32 animationMode;
    f32 transitionAngle;
    f32 reducedAngle;
    f32 halfAngle;
    f32 fullAngle;

    object->modes.values[0] = 0;
    object->modes.values[1] = 0;
    selectCharacterMotion__FP11MotionOwnerUi(&object->motion, DATANUM(0x8e0000, 197));
    fn_1_33950__5ModelFUl(&object->modelId, ((1 << 30) | (1 << 1)));
    modelFlags = (s16 *)object;
    if (modelFlags != 0) {
        modelFlags += 0x14;
    }
    modelFlagsAddress = modelFlags;
    rawModelFlag = (s16)*modelFlagsAddress;
    modelFlagArgument = rawModelFlag;
    setCharacterMotionOverlay__FP11MotionOwnersUi(&object->motion, modelFlagArgument, DATANUM(0x8e0000, 194));
    fn_1_33950__5ModelFUl(&object->modelId, ((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set( 0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set( halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set( 0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set( halfAngle, animationMode);
        break;
    }
}
extern "C" {

}

void PlayerObject::fn_1_5E8C(void)
{
}

extern "C" {

}

void Work6B44::fn_1_5E90()
{
    if (rotation.y <= 180.0f) {
        rotation.y += perFrame(150.0);
    }
}

static inline s16 modelFlag5EF4(const s16 *modelFlags)
{
    return *modelFlags;
}

static inline s32 sequence5EF4(const PlayerObject *object)
{
    return object->sequence;
}

static inline void fn_1_5A44_for_5EF4(PlayerObject *object)
{
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion__FP11MotionOwnerUi(&object->motion, DATANUM(0x8e0000, 197));
    object->fn_1_33950(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay__FP11MotionOwnersUi(&object->motion, *object, DATANUM(0x8e0000, 194));
    object->fn_1_33950(((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set( 0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set( halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set( 0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set( halfAngle, animationMode);
        break;
    }
}

void PlayerObject::fn_1_5EF4()
{
    M681GameState *state;
    PlayerObject::Callback trailingValues;

    state = &MinigameSingleton<M681GameState>::getChecked();

    sendEvent(sequence5EF4(this) + 2, 0, 2.4f);
    this->modes.values[0] = state->textureChoices.values[0];
    this->modes.values[1] = state->textureChoices.values[1];
    fn_1_5A44_for_5EF4(this);

    trailingValues = &PlayerObject::fn_1_5E8C;
    this->callback = trailingValues;
}

void PlayerObject::fn_1_61B8()
{
    if (this->firstAngle.elapsed > this->firstAngle.frames &&
        this->secondAngle.elapsed > this->secondAngle.frames) {
        PlayerObject::Callback trailingValues;

        this->modes.values[0] = 0;
        this->modes.values[1] = 0;
        fn_1_5A44_for_5EF4(this);
        trailingValues = &PlayerObject::fn_1_5E8C;
        this->callback = trailingValues;
    }
}

extern "C" {

void PlayerObject::fn_1_643C()
{
    this->modes.values[0] = 0;
    this->modes.values[1] = 0;
}

}

static inline void fn_1_5A44_for_6450(PlayerObject *object)
{
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion__FP11MotionOwnerUi(&object->motion, DATANUM(0x8e0000, 197));
    object->fn_1_33950(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay__FP11MotionOwnersUi(&object->motion, *object, DATANUM(0x8e0000, 194));
    object->fn_1_33950(((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set( 0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set( halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set( 0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set( halfAngle, animationMode);
        break;
    }
}

/* Whether either mode differs from the previous frame's pair. */
static inline bool ModesChanged6450(PlayerObject *object, const ModePair &previous)
{
    return object->modes.values[0] != previous.values[0] || object->modes.values[1] != previous.values[1];
}

void PlayerObject::fn_1_6450()
{
    ModePair previousModes = this->modes;
    ModeBase *callback;

    if (this->active == 1) {
        callback = this->resourceNode.value;
        callback->update(this);
    }

    if (ModesChanged6450(this, previousModes)) {
        fn_1_5A44_for_6450(this);
    }

    this->progress += 1.0f;
}

extern "C" {

typedef Work6B44::Callback DustCallbackState;

typedef M681GameState HookManager;

void PlayerObject::fn_1_66F0()
{
    HookManager &manager = MinigameSingleton<M681GameState>::getChecked();

    if (this->active == 1) {
        u8 changed = 1;
        if (this->modes.values[0] == manager.textureChoices.values[0] &&
            this->modes.values[1] == manager.textureChoices.values[1]) {
            changed = 0;
        }

        if (changed != 0) {
            this->sixthValue = 1;
            sendEvent(sequence5EF4(this) + 2, 1, 0.0f);
        } else {
            sendEvent(sequence5EF4(this) + 2, 0, 0.0f);
        }
    }

    ++manager.state38;
    if (manager.state38 >= 4) {
        manager.state38 = 4 - manager.minimum;
        sendEvent(0, 9, 0.0f);
    }
}

void PlayerObject::fn_1_6824()
{

    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();

    if (this->sixthValue == 1 && this->active == 1) {
        manager.minimum = manager.minimum - 1;
        this->active = 0;
        this->firstValue = this->firstValue + manager.state40;
        manager.state40 += 0.2f;
        fn_8005C7A4((s16)fn_1_1BDA4(&this->sequence),
                                this->firstItemHook);
        fn_8005C7A4((s16)fn_1_1BDA4(&this->sequence),
                                this->secondItemHook);
        {
            DustCallbackState callbackState = &Work6B44::fn_1_7410;
            this->callback = callbackState;
        }
    }

    manager.state38 = manager.state38 + 1;
    if (manager.state38 >= 4) {
        manager.state38 = 4 - manager.minimum;
        if (manager.minimum <= 1) {
            sendEvent(0, 10, 0.0f);
            return;
        }
        sendEvent(0, 5, 0.0f);
    }
}

}

void Work6B44::update()
{
    (this->*callback)();
    if (firstAngle.elapsed <= firstAngle.frames) {
        motion.fn_1_DAF0(firstAngle.advance());
    }
    if (secondAngle.elapsed <= secondAngle.frames) {
        Model::fn_1_34528(secondAngle.advance());
    }
    Model::fn_1_33A3C(&state.value);
    Model::fn_1_33ACC(&rotation);
}

extern "C" {

typedef Callback533C CallbackState;

void PlayerObject::fn_1_6A50()
{
    CallbackState callback;

    if (this->rotation.y <= 360.0f) {
        this->rotation.y = this->rotation.y +
                      perFrame(240.0);
    } else {
        shiftCharacterMotion__FP11MotionOwnerUiffUl(&this->motion, DATANUM(0x930000, 0), 8.0f,
                  0.0f, ((1 << 30) | (1 << 0)));
        callback = &PlayerObject::fn_1_5E8C;
        this->callback = callback;
    }
    fn_8005C7A4((s16)fn_1_1BDA4(&this->sequence),
                            this->firstItemHook);
    fn_8005C7A4((s16)fn_1_1BDA4(&this->sequence),
                            this->secondItemHook);
}

}

extern "C" {

}
float RandomFloat(float maximum);

static inline s16 modelFlag6B44(const s16 *modelFlags)
{
    return *modelFlags;
}
static inline s32 sequence6B44(const Work6B44 *object)
{
    return object->sequence;
}

static inline void fn_1_5A44_for_6B44(Work6B44 *object)
{
    s16 *modelFlags;
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion__FP11MotionOwnerUi(&object->motion, DATANUM(0x8e0000, 197));
    object->Model::fn_1_33950(((1 << 30) | (1 << 1)));
    modelFlags = (s16 *)object;
    if (modelFlags != 0) {
        modelFlags += 0x14;
    }
    setCharacterMotionOverlay__FP11MotionOwnersUi(&object->motion, modelFlag6B44(modelFlags), DATANUM(0x8e0000, 194));
    object->Model::fn_1_33950(((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set( 0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set( halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set( reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set( fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set( 0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set( halfAngle, animationMode);
        break;
    }
}

void Work6B44::receive(const EventPacket *event)
{
    if (event->channel == 1) {
        if (event->type == 0 && this->active == 0)
            ++this->count;
        return;
    }

    M681GameState *manager = &MinigameSingleton<M681GameState>::getChecked();
    switch (event->type) {
    case 0: {
        Work6B44::Callback next = &Work6B44::fn_1_5E90;
        this->callback = next;
        break;
    }
    case 2: {
        Work6B44::Callback next = &Work6B44::fn_1_5EF4;
        this->callback = next;
        break;
    }
    case 3: {
        this->firstAngle.set(
                    (f32)(0.2 * (f64)this->fullAngle),
                    (s32)secondsToFrames(this->animationScale));
        this->secondAngle.set(
                    (f32)(0.2 * (f64)this->fullAngle),
                    (s32)secondsToFrames(this->animationScale));
        Work6B44::Callback next = &Work6B44::fn_1_61B8;
        this->callback = next;
        break;
    }
    case 5:
        manager->state40 = 0.0f;
        if (this->active == 1) {
            this->modes.values[0] = 0;
            this->modes.values[1] = 0;
            fn_1_5A44_for_6B44(this);
            Work6B44::Callback next = &Work6B44::fn_1_643C;
            this->callback = next;
        }
        if (manager->interval >= 2.0f)
            this->animationScale = 0.2f;
        else if (manager->interval >= 1.0f)
            this->animationScale = 0.1f;
        else
            this->animationScale = 0.05f;
        break;
    case 7:
        if (this->active == 1) {
            Work6B44::Callback next = &Work6B44::fn_1_6450;
            this->callback = next;
        }
        this->progress = 0.0f;
        this->velocity = manager->interval * RandomFloat(0.8f);
        break;
    case 8:
        if (this->active == 1) {
            this->firstAngle.set(
                        (f32)(0.2 * (f64)this->fullAngle),
                        (s32)secondsToFrames(0.2));
            this->secondAngle.set(
                        (f32)(0.2 * (f64)this->fullAngle),
                        (s32)secondsToFrames(0.2));
            Work6B44::Callback next = &Work6B44::fn_1_66F0;
            this->callback = next;
        }
        break;
    case 9:
        if (this->active == 1) {
            Work6B44::Callback next = &Work6B44::fn_1_6824;
            this->callback = next;
        }
        break;
    case 11:
        this->Model::fn_1_34764();
        if (this->active == 1) {
            shiftCharacterMotion__FP11MotionOwnerUiffUl(&this->motion, DATANUM(0x930000, 1),
                       8.0f, 0.0f, ((1 << 30) | (1 << 0)));
            Work6B44::Callback next = &Work6B44::fn_1_6A50;
            this->callback = next;
            M681GameState &state = *&MinigameSingleton<M681GameState>::getChecked();
            state.negativeState = sequence6B44(this);
        }
        break;
    case 12: {
        s32 count = manager->poseActive() ? this->count :
            (manager->state4C == 1 ? 3 : this->count - 1);
        s32 sequence = sequence6B44(this);
        if (!fn_8006A124(65551))
            lbl_80265750[sequence].mgCoinBonus = (s16)count;
        if (this->active != 0) {
            fn_80060F14((s16)fn_1_1BDA4(&this->sequence), 0x40);
            shiftCharacterMotion__FP11MotionOwnerUiffUl(&this->motion, DATANUM(0x930000, 6), 8.0f, 0.0f, 0);
        }
        Work6B44::Callback next = &Work6B44::fn_1_5E8C;
        this->callback = next;
        break;
    }
    case 1:
        break;
    default:
        if (this->active == 1) {
            Work6B44::Callback next = &Work6B44::fn_1_5E8C;
            this->callback = next;
        }
        break;
    }
}

void fn_1_7398(void *owner, s16 item, const void *hookModel)
{
    char *hookName = fn_8005C0F0(
        (s16)fn_1_1BDA4(&((Work6B44 *)owner)->sequence), 2, item);
    s16 hookModelId = ((const Model *)hookModel)->modelId;
    ((Work6B44 *)owner)->Model::fn_1_347D4( hookName, hookModelId);
}

void Work6B44::fn_1_7410()
{
    this->elapsed++;
    if ((f64)this->elapsed >= secondsToFrames(1.0)) {
        shiftCharacterMotion__FP11MotionOwnerUiffUl(&this->motion, 0x930019,
                  8.0f, 0.0f, 0);
        Callback result = &Work6B44::fn_1_74E8;
        this->callback = result;
    }
}

extern "C" {

}
extern void fn_1_1BE24(s32 *playerIndex, s32 frames, s32 duration);
extern "C" {

}

void Work6B44::fn_1_74E8()
{
    Callback callback;

    elapsed += 1;
    if ((f64)elapsed >=
        secondsToFrames(2.0)) {
        s16 pan;

        fn_1_1BE24(&sequence, 2, 0x18);
        shiftCharacterMotion__FP11MotionOwnerUiffUl(&motion, DATANUM(0x8e0000, 114),
                  8.0f, 0.0f, 0);
        Model::fn_1_340D8();
        pan = fn_1_2508(fn_1_1BDA4(&sequence));
        CharFXPlayPan((s16)fn_1_1BDA4(&sequence), 0x24C, pan);
        callback = &Work6B44::fn_1_7608;
        this->callback = callback;
    }
}

extern "C" {
}
void fn_1_5168(Model *);
extern "C" {
}
void Work6B44::fn_1_7608()
{
    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();
    fallFrames = fallFrames + 1;
    state.value.y = state.value.y - (1.5f * fallFrames);
    if (state.value.y <= -300.0f) {
        secondValue += 1.0f;
        if (secondValue >= secondsToFrames(firstValue)) {
            fn_1_5168(this->resource);
            {
                Callback callbackWork = &Work6B44::fn_1_77E4;
                this->callback = callbackWork;
            }
            Model::fn_1_33CB0(1);
            selectCharacterMotion__FP11MotionOwnerUi(&motion, DATANUM(0x8e0000, 197));
            Model::fn_1_33950(((1 << 30) | (1 << 1)));
            firstAngle.set( fullAngle, (s32)secondsToFrames(0.1f));
            secondAngle.set( fullAngle, (s32)secondsToFrames(0.1f));
        }
    }
}

extern "C" {
void fn_1_524C(void *model, Mtx matrix);
}

void Work6B44::fn_1_77E4()
{
    void *model = this->resource.value;
    Mtx matrix;

    fn_1_524C(model, matrix);
    Hu3DMtxTransGet(matrix, &this->state.value);
    Hu3DMtxRotGet(matrix, &this->rotation);
}

extern "C" {

}

extern "C" {
u8 lbl_1_bss_AC[4];
}
