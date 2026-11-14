#include "REL/m681DLL/game_controller.h"
#include "REL/framework/world_units.h"
#include "REL/framework/random.h"
#include "REL/framework/event.h"
#include <new>

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
#include "REL/m681DLL/game_start.h"
#include "REL/m681DLL/global_state.h"
#include <vector>
#include "REL/m681DLL/player_input.h"
#include "REL/m681DLL/game_constants.h"
#include "REL/framework/frame_durations.h"

void fn_1_7398(PlayerObject *, s16, const void *);
extern "C" {
const unsigned int lbl_1_rodata_288[] = {
    0x930000, 0x930001, 0x930006, 0x8E00C1, 0x930019,
    0x8E0072, 0x8E00D5, 0x8E00C2, 0x8E00C5
};
}

// Creates the player model, input controller, held items and motion state.
PlayerObject::PlayerObject(s32 player, const Vec *initialState)
    : GameObject(0), PlayerConfigSelection(player, 1), EventListener(),
      input(isComputer()
          ? new ModeObject(difficulty())
          : (ModeBase *)new ModeToggle()),
      pipeModel(new ModelResource(player)),
      state(*initialState),
      firstMotionTime(0.0f, 0),
      secondMotionTime(0.0f, 0),
      firstItem(new OwnedModel(DATANUM(0x8c0000, 9))),
      secondItem(new OwnedModel(DATANUM(0x8c0000, 10))),
      elapsed(0),
      fallFrames(0),
      inputFrames(0.0f),
      inputDelay(0.0f),
      landingDelay(2.0f),
      landingFrames(0.0f),
      animationTime(0.2f),
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
    PlayerObject::Callback callback = &PlayerObject::fn_1_5E8C;
    this->callback = callback;
    setLayer(4);
    createCharacterMotions(&static_cast<MotionOwner &>(*this), lbl_1_rodata_288, 9);
    selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 193));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*this), *static_cast<Model *>(this),
                              DATANUM(0x8e0000, 194));
    maximumMotionTime = getOverlayMotionMaxTime();
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

// Releases the input controller, pipeModel and held items.
PlayerObject::~PlayerObject() {}

// Plays the choosing motion and turns the two item arms toward the player's current choices.
inline void PlayerObject::showChoices()
{
    s32 animationMode;
    f32 transitionAngle;
    f32 maximumMotionTime;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 197));
    setAttribute(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*this), *this, DATANUM(0x8e0000, 194));
    setAttribute(((1 << 30) | (1 << 10)));

    transitionAngle = this->maximumMotionTime;
    animationMode = (s32)secondsToFrames(animationTime);
    maximumMotionTime = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (modes.values[0]) {
    case 2:
        break;
    case 0:
        firstMotionTime.set(reducedAngle, animationMode);
        break;
    case 4:
        firstMotionTime.set(maximumMotionTime, animationMode);
        break;
    case 3:
        firstMotionTime.set(0.0f, animationMode);
        break;
    case 1:
        firstMotionTime.set(halfAngle, animationMode);
        break;
    }

    switch (modes.values[1]) {
    case 0:
        secondMotionTime.set(reducedAngle, animationMode);
        break;
    case 4:
        secondMotionTime.set(maximumMotionTime, animationMode);
        break;
    case 3:
        secondMotionTime.set(0.0f, animationMode);
        break;
    case 2:
        secondMotionTime.set(halfAngle, animationMode);
        break;
    }
}

// Selects the pose and interpolated item angles for the supplied player choices.
void applyPlayerChoices(PlayerObject *object)
{
    s32 animationMode;
    f32 transitionAngle;
    f32 maximumMotionTime;
    f32 halfAngle;
    f32 reducedAngle;

    selectCharacterMotion(&static_cast<MotionOwner &>(*object), DATANUM(0x8e0000, 197));
    object->setAttribute(((1 << 30) | (1 << 1)));
    setCharacterMotionOverlay(&static_cast<MotionOwner &>(*object), *object, DATANUM(0x8e0000, 194));
    object->setAttribute(((1 << 30) | (1 << 10)));

    transitionAngle = object->maximumMotionTime;
    animationMode = (s32)secondsToFrames(object->animationTime);
    maximumMotionTime = transitionAngle;
    halfAngle = (f32)(0.5 * (f64)transitionAngle);
    reducedAngle = (f32)(0.2 * (f64)transitionAngle);

    switch (object->modes.values[0]) {
    case 2:
        break;
    case 0:
        object->firstMotionTime.set(reducedAngle, animationMode);
        break;
    case 4:
        object->firstMotionTime.set(maximumMotionTime, animationMode);
        break;
    case 3:
        object->firstMotionTime.set(0.0f, animationMode);
        break;
    case 1:
        object->firstMotionTime.set(halfAngle, animationMode);
        break;
    }

    switch (object->modes.values[1]) {
    case 0:
        object->secondMotionTime.set(reducedAngle, animationMode);
        break;
    case 4:
        object->secondMotionTime.set(maximumMotionTime, animationMode);
        break;
    case 3:
        object->secondMotionTime.set(0.0f, animationMode);
        break;
    case 2:
        object->secondMotionTime.set(halfAngle, animationMode);
        break;
    }
}

// Clears both choices and updates their displayed pose.
void clearPlayerChoices(PlayerObject *object)
{
    object->modes.values[0] = 0;
    object->modes.values[1] = 0;
    object->showChoices();
}

// Performs no player update.
void PlayerObject::fn_1_5E8C(void)
{
}

// Turns the player toward 180 degrees.
void PlayerObject::fn_1_5E90()
{
    if (rotation.y <= 180.0f) {
        rotation.y += framesToSeconds(150.0);
    }
}

// Copies the round target choices and displays them.
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

// Clears the choices after both angle interpolators finish.
void PlayerObject::fn_1_61B8()
{
    if (this->firstMotionTime.isFinished() &&
        this->secondMotionTime.isFinished()) {
        PlayerObject::Callback trailingValues;

        this->modes.values[0] = 0;
        this->modes.values[1] = 0;
        showChoices();
        trailingValues = &PlayerObject::fn_1_5E8C;
        this->callback = trailingValues;
    }
}

// Clears both choice values.
void PlayerObject::fn_1_643C()
{
    this->modes.values[0] = 0;
    this->modes.values[1] = 0;
}

/* Whether either mode differs from the previous frame's pair. */
static inline bool choicesChanged(PlayerObject *object, const ModePair &previous)
{
    return object->modes.values[0] != previous.values[0] || object->modes.values[1] != previous.values[1];
}

// Updates active-player input and refreshes the displayed choices when needed.
void PlayerObject::fn_1_6450()
{
    ModePair previousModes = this->modes;

    if (this->active == 1) {
        input->update(this);
    }

    if (choicesChanged(this, previousModes)) {
        showChoices();
    }

    this->inputFrames += 1.0f;
}

// Checks the answer and counts players that have answered.
void PlayerObject::fn_1_66F0()
{
    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();

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

    ++manager.answeredPlayers;
    if (manager.answeredPlayers >= 4) {
        manager.answeredPlayers = 4 - manager.playersRemaining;
        sendEvent(0, 9, 0.0f);
    }
}

// Eliminates an incorrect player and advances the round after all answers.
void PlayerObject::fn_1_6824()
{
    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();

    if (this->choseWrong == 1 && this->active == 1) {
        manager.playersRemaining = manager.playersRemaining - 1;
        this->active = 0;
        this->landingDelay = this->landingDelay + manager.additionalLandingDelay;
        manager.additionalLandingDelay += 0.2f;
        fn_8005C7A4((s16)characterNo(),
                                this->firstItemHook);
        fn_8005C7A4((s16)characterNo(),
                                this->secondItemHook);
        {
            PlayerObject::Callback callbackState = &PlayerObject::fn_1_7410;
            this->callback = callbackState;
        }
    }

    manager.answeredPlayers = manager.answeredPlayers + 1;
    if (manager.answeredPlayers >= 4) {
        manager.answeredPlayers = 4 - manager.playersRemaining;
        if (manager.playersRemaining <= 1) {
            sendEvent(0, 10, 0.0f);
            return;
        }
        sendEvent(0, 5, 0.0f);
    }
}

// Runs the player callback, advances item angles and applies the transform.
void PlayerObject::update()
{
    (this->*callback)();
    if (!firstMotionTime.isFinished()) {
        fn_1_DAF0(firstMotionTime.update());
    }
    if (!secondMotionTime.isFinished()) {
        Model::setOverlayMotionTime(secondMotionTime.update());
    }
    Model::setPosition(&state);
    Model::setRotation(&rotation);
}

// Turns the player through the winner animation and releases held items.
void PlayerObject::fn_1_6A50()
{
    PlayerObject::Callback callback;

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

// Selects player callbacks and motions for game events.
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
        this->firstMotionTime.set(
                    (f32)(0.2 * (f64)this->maximumMotionTime),
                    (s32)secondsToFrames(this->animationTime));
        this->secondMotionTime.set(
                    (f32)(0.2 * (f64)this->maximumMotionTime),
                    (s32)secondsToFrames(this->animationTime));
        PlayerObject::Callback next = &PlayerObject::fn_1_61B8;
        this->callback = next;
        break;
    }
    case 5:
        manager->additionalLandingDelay = 0.0f;
        if (this->active == 1) {
            this->modes.values[0] = 0;
            this->modes.values[1] = 0;
            showChoices();
            PlayerObject::Callback next = &PlayerObject::fn_1_643C;
            this->callback = next;
        }
        if (manager->interval >= 2.0f)
            this->animationTime = 0.2f;
        else if (manager->interval >= 1.0f)
            this->animationTime = 0.1f;
        else
            this->animationTime = 0.05f;
        break;
    case 7:
        if (this->active == 1) {
            PlayerObject::Callback next = &PlayerObject::fn_1_6450;
            this->callback = next;
        }
        this->inputFrames = 0.0f;
        this->inputDelay = manager->interval * RandomFloat(0.8f);
        break;
    case 8:
        if (this->active == 1) {
            this->firstMotionTime.set(
                        (f32)(0.2 * (f64)this->maximumMotionTime),
                        (s32)secondsToFrames(0.2));
            this->secondMotionTime.set(
                        (f32)(0.2 * (f64)this->maximumMotionTime),
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
            M681GameState &state = MinigameSingleton<M681GameState>::getChecked();
            state.winnerPlayer = number();
        }
        break;
    case 12: {
        s32 count = manager->hasWinner() ? this->count :
            (manager->roundCount == 1 ? 3 : this->count - 1);
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

// Attaches the supplied owned model to the selected character hook.
void fn_1_7398(PlayerObject *owner, s16 item, const void *hookModel)
{
    char *hookName = fn_8005C0F0(
        (s16)owner->characterNo(), 2, item);
    s16 hookModelId = ((const Model *)hookModel)->modelId;
    owner->Model::setHook( hookName, hookModelId);
}

// Starts the falling motion after the first delay.
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

// Starts the landing motion, rumble and sound after the second delay.
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

// Moves the eliminated player down and switches to the pipeModel model when ready.
void PlayerObject::fn_1_7608()
{
    M681GameState &manager = MinigameSingleton<M681GameState>::getChecked();
    fallFrames = fallFrames + 1;
    state.y = state.y - (1.5f * fallFrames);
    if (state.y <= -300.0f) {
        landingFrames += 1.0f;
        if (landingFrames >= secondsToFrames(landingDelay)) {
            fn_1_5168(this->pipeModel.get());
            {
                Callback callbackWork = &PlayerObject::fn_1_77E4;
                this->callback = callbackWork;
            }
            Model::setLayer(1);
            selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 197));
            Model::setAttribute(((1 << 30) | (1 << 1)));
            firstMotionTime.set( maximumMotionTime, (s32)secondsToFrames(0.1f));
            secondMotionTime.set( maximumMotionTime, (s32)secondsToFrames(0.1f));
        }
    }
}

// Copies the attachment matrix translation and rotation to the player.
void PlayerObject::fn_1_77E4()
{
    Mtx matrix;

    fn_1_524C(pipeModel.get(), matrix);
    Hu3DMtxTransGet(matrix, &this->state);
    Hu3DMtxRotGet(matrix, &this->rotation);
}

extern "C" {
// Four bytes of unknown purpose; no code reads or writes them.
u8 lbl_1_bss_AC[4];
}
