#include "REL/m681DLL.h"
#include "REL/framework/frame_time.h"
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
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/m681DLL/player_object.h"
#include "REL/m681DLL/global_state.h"
#include "REL/m681DLL/character_sequence.h"
#include "REL/m681DLL/player_input.h"
#include "REL/framework/world_units.h"

extern "C" {
u32 lbl_1_bss_98 = minutesToFrames(60L);
f32 lbl_1_bss_9C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
u32 lbl_1_bss_A0 = secondsToFrames(2L);
u32 lbl_1_bss_A4 = secondsToFrames(3.5);
u32 lbl_1_bss_A8 = secondsToFrames(3.5);
}

void *operator new(unsigned long);
void setCharacterMotionOverlay(CharacterMotion *character, s16 modelId, unsigned int dataNum);
s16 fn_1_2508(s32);
void shiftCharacterMotion(CharacterMotion *character, unsigned int dataNum, float shift, float start,
                          unsigned long attributes);
void sendEvent(int, unsigned long, float);

void fn_1_7398(void *, s16, const void *);
extern "C" {
const unsigned int lbl_1_rodata_288[] = {
    0x930000, 0x930001, 0x930006, 0x8E00C1, 0x930019,
    0x8E0072, 0x8E00D5, 0x8E00C2, 0x8E00C5
};
}

typedef PlayerObject::Callback Callback533C;

PlayerObject::PlayerObject(s32 player, const Vec *initialState)
    : GameObject(0), PlayerConfigSelection(player, 1), EventListener(),
      resourceNode(isComputer()
          ? (ModeBase *)new ModeObject(difficulty())
          : (ModeBase *)new ModeToggle()),
      resource(new ModelResource(player)),
      state(*initialState),
      firstAngle(0.0f, 0),
      secondAngle(0.0f, 0),
      firstItem(new OwnedModel(DATANUM(0x8c0000, 9))),
      secondItem(new OwnedModel(DATANUM(0x8c0000, 10))),
      elapsed(0),
      fallFrames(0),
      progress(0.0f),
      velocity(0.0f),
      landingDelay(2.0f),
      landingFrames(0.0f),
      animationScale(0.2f),
      unkAC(6.0f),
      unkB0(0.0f),
      unkB8(0),
      count(0),
      active(1),
      choseWrong(0),
      firstItemHook(fn_8005C0F0((s16)characterNo(), 2, 1)),
      secondItemHook(fn_8005C0F0((s16)characterNo(), 2, 0))
{
    registerChannel(0);
    registerChannel(1);
    {
        Callback533C callback = &PlayerObject::fn_1_5E8C;
        this->callback = callback;
    }
    setLayer(4);
    createCharacterMotions(&static_cast<MotionOwner &>(*this),
                           const_cast<unsigned int *>(lbl_1_rodata_288), 9);
    selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 193));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*this), *static_cast<Model *>(this),
                              DATANUM(0x8e0000, 194));
    fullAngle = getOverlayMotionMaxTime();
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*this), *static_cast<Model *>(this),
                              DATANUM(0x8e0000, 213));
    setAttribute(((1 << 30) | (1 << 9)));
    fn_1_7398(this, 1, firstItem.get());
    fn_1_7398(this, 0, secondItem.get());
    modes.values[0] = 0;
    modes.values[1] = 0;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    setPosition(&state);
    setRotation(&rotation);
    fn_80060F14((s16)characterNo(), fn_1_2508(player));
}

PlayerObject::~PlayerObject() {}

// Plays the choosing motion and turns the two item arms toward the player's current choices.
inline void PlayerObject::showChoices()
{
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 197));
    setAttribute(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*this), *this, DATANUM(0x8e0000, 194));
    setAttribute(((1 << 30) | (1 << 10)));

    transitionAngle = this->fullAngle;
    animationMode = (s32)secondsToFrames(animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (modes.values[0]) {
    case 2:
        break;
    case 0:
        firstAngle.set(reducedAngle, animationMode);
        break;
    case 4:
        firstAngle.set(fullAngle, animationMode);
        break;
    case 3:
        firstAngle.set(0.0f, animationMode);
        break;
    case 1:
        firstAngle.set(halfAngle, animationMode);
        break;
    }

    switch (modes.values[1]) {
    case 0:
        secondAngle.set(reducedAngle, animationMode);
        break;
    case 4:
        secondAngle.set(fullAngle, animationMode);
        break;
    case 3:
        secondAngle.set(0.0f, animationMode);
        break;
    case 2:
        secondAngle.set(halfAngle, animationMode);
        break;
    }
}

void fn_1_5A44(PlayerObject *object)
{
    s32 animationMode;
    f32 transitionAngle;
    f32 fullAngle;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion(&static_cast<MotionOwner &>(*object), DATANUM(0x8e0000, 197));
    object->setAttribute(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*object), *object, DATANUM(0x8e0000, 194));
    object->setAttribute(((1 << 30) | (1 << 10)));

    transitionAngle = object->fullAngle;
    animationMode = (s32)secondsToFrames(object->animationScale);
    fullAngle = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstAngle.set(reducedAngle, animationMode);
        break;
    case 4:
        object->firstAngle.set(fullAngle, animationMode);
        break;
    case 3:
        object->firstAngle.set(0.0f, animationMode);
        break;
    case 1:
        object->firstAngle.set(halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondAngle.set(reducedAngle, animationMode);
        break;
    case 4:
        object->secondAngle.set(fullAngle, animationMode);
        break;
    case 3:
        object->secondAngle.set(0.0f, animationMode);
        break;
    case 2:
        object->secondAngle.set(halfAngle, animationMode);
        break;
    }
}

void fn_1_5C60(PlayerObject *object)
{
    object->modes.values[0] = 0;
    object->modes.values[1] = 0;
    object->showChoices();
}

void PlayerObject::fn_1_5E8C(void)
{
}

void PlayerObject::fn_1_5E90()
{
    if (rotation.y <= 180.0f) {
        rotation.y += framesToSeconds(150.0);
    }
}

void PlayerObject::fn_1_5EF4()
{
    M681GameState *state;
    PlayerObject::Callback trailingValues;

    state = &MinigameSingleton<M681GameState>::getChecked();

    sendEvent(number() + 2, 0, 2.4f);
    this->modes.values[0] = state->textureChoices.values[0];
    this->modes.values[1] = state->textureChoices.values[1];
    showChoices();

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
        showChoices();
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

/* Whether either mode differs from the previous frame's pair. */
static inline bool ModesChanged6450(PlayerObject *object, const ModePair &previous)
{
    return object->modes.values[0] != previous.values[0] || object->modes.values[1] != previous.values[1];
}

void PlayerObject::fn_1_6450()
{
    ModePair previousModes = this->modes;

    if (this->active == 1) {
        resourceNode->update(this);
    }

    if (ModesChanged6450(this, previousModes)) {
        showChoices();
    }

    this->progress += 1.0f;
}

extern "C" {
typedef PlayerObject::Callback DustCallbackState;

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
            this->choseWrong = 1;
            sendEvent(number() + 2, 1, 0.0f);
        } else {
            sendEvent(number() + 2, 0, 0.0f);
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

    if (this->choseWrong == 1 && this->active == 1) {
        manager.minimum = manager.minimum - 1;
        this->active = 0;
        this->landingDelay = this->landingDelay + manager.state40;
        manager.state40 += 0.2f;
        fn_8005C7A4((s16)characterNo(),
                                this->firstItemHook);
        fn_8005C7A4((s16)characterNo(),
                                this->secondItemHook);
        {
            DustCallbackState callbackState = &PlayerObject::fn_1_7410;
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

void PlayerObject::update()
{
    (this->*callback)();
    if (firstAngle.elapsed <= firstAngle.frames) {
        fn_1_DAF0(firstAngle.advance());
    }
    if (secondAngle.elapsed <= secondAngle.frames) {
        Model::setOverlayMotionTime(secondAngle.advance());
    }
    Model::setPosition(&state);
    Model::setRotation(&rotation);
}

extern "C" {
typedef Callback533C CallbackState;

void PlayerObject::fn_1_6A50()
{
    CallbackState callback;

    if (this->rotation.y <= 360.0f) {
        this->rotation.y = this->rotation.y +
                      framesToSeconds(240.0);
    } else {
        shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x930000, 0), 8.0f,
                  0.0f, ((1 << 30) | (1 << 0)));
        callback = &PlayerObject::fn_1_5E8C;
        this->callback = callback;
    }
    fn_8005C7A4((s16)characterNo(),
                            this->firstItemHook);
    fn_8005C7A4((s16)characterNo(),
                            this->secondItemHook);
}
}

float RandomFloat(float maximum);

void PlayerObject::receive(const EventPacket *event)
{
    if (event->channel == 1) {
        if (event->type == 0 && this->active == 0)
            ++this->count;
        return;
    }

    M681GameState *manager = &MinigameSingleton<M681GameState>::getChecked();
    switch (event->type) {
    case 0: {
        PlayerObject::Callback next = &PlayerObject::fn_1_5E90;
        this->callback = next;
        break;
    }
    case 2: {
        PlayerObject::Callback next = &PlayerObject::fn_1_5EF4;
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
        PlayerObject::Callback next = &PlayerObject::fn_1_61B8;
        this->callback = next;
        break;
    }
    case 5:
        manager->state40 = 0.0f;
        if (this->active == 1) {
            this->modes.values[0] = 0;
            this->modes.values[1] = 0;
            showChoices();
            PlayerObject::Callback next = &PlayerObject::fn_1_643C;
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
            PlayerObject::Callback next = &PlayerObject::fn_1_6450;
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
            PlayerObject::Callback next = &PlayerObject::fn_1_66F0;
            this->callback = next;
        }
        break;
    case 9:
        if (this->active == 1) {
            PlayerObject::Callback next = &PlayerObject::fn_1_6824;
            this->callback = next;
        }
        break;
    case 11:
        this->Model::clearOverlayMotion();
        if (this->active == 1) {
            shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x930000, 1),
                       8.0f, 0.0f, ((1 << 30) | (1 << 0)));
            PlayerObject::Callback next = &PlayerObject::fn_1_6A50;
            this->callback = next;
            M681GameState &state = *&MinigameSingleton<M681GameState>::getChecked();
            state.negativeState = number();
        }
        break;
    case 12: {
        s32 count = manager->poseActive() ? this->count :
            (manager->state4C == 1 ? 3 : this->count - 1);
        s32 sequence = number();
        if (!fn_8006A124(65551))
            lbl_80265750[sequence].mgCoinBonus = (s16)count;
        if (this->active != 0) {
            fn_80060F14((s16)characterNo(), 0x40);
            shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x930000, 6), 8.0f,
                                 0.0f, 0);
        }
        PlayerObject::Callback next = &PlayerObject::fn_1_5E8C;
        this->callback = next;
        break;
    }
    case 1:
        break;
    default:
        if (this->active == 1) {
            PlayerObject::Callback next = &PlayerObject::fn_1_5E8C;
            this->callback = next;
        }
        break;
    }
}

void fn_1_7398(void *owner, s16 item, const void *hookModel)
{
    char *hookName = fn_8005C0F0(
        (s16)((PlayerObject *)owner)->characterNo(), 2, item);
    s16 hookModelId = ((const Model *)hookModel)->modelId;
    ((PlayerObject *)owner)->Model::setHook( hookName, hookModelId);
}

void PlayerObject::fn_1_7410()
{
    this->elapsed++;
    if ((f64)this->elapsed >= secondsToFrames(1.0)) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(*this), 0x930019,
                  8.0f, 0.0f, 0);
        Callback result = &PlayerObject::fn_1_74E8;
        this->callback = result;
    }
}

void PlayerObject::fn_1_74E8()
{
    Callback callback;

    elapsed += 1;
    if ((f64)elapsed >=
        secondsToFrames(2.0)) {
        s16 pan;

        rumble(2, 0x18);
        shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 114),
                  8.0f, 0.0f, 0);
        Model::resetShadow();
        pan = fn_1_2508(characterNo());
        CharFXPlayPan((s16)characterNo(), 0x24C, pan);
        callback = &PlayerObject::fn_1_7608;
        this->callback = callback;
    }
}

void fn_1_5168(Model *);
void PlayerObject::fn_1_7608()
{
    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();
    fallFrames = fallFrames + 1;
    state.y = state.y - (1.5f * fallFrames);
    if (state.y <= -300.0f) {
        landingFrames += 1.0f;
        if (landingFrames >= secondsToFrames(landingDelay)) {
            fn_1_5168(this->resource.get());
            {
                Callback callbackWork = &PlayerObject::fn_1_77E4;
                this->callback = callbackWork;
            }
            Model::setLayer(1);
            selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 197));
            Model::setAttribute(((1 << 30) | (1 << 1)));
            firstAngle.set( fullAngle, (s32)secondsToFrames(0.1f));
            secondAngle.set( fullAngle, (s32)secondsToFrames(0.1f));
        }
    }
}

extern "C" {
void fn_1_524C(void *model, Mtx matrix);
}

void PlayerObject::fn_1_77E4()
{
    Mtx matrix;

    fn_1_524C(resource.get(), matrix);
    Hu3DMtxTransGet(matrix, &this->state);
    Hu3DMtxRotGet(matrix, &this->rotation);
}

extern "C" {
u8 lbl_1_bss_AC[4];
}
