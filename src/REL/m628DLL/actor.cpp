#include <new>
#include "dolphin/types.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/event_callback_inline.h"

extern "C"
{
#include "dolphin/os.h"
}

#include "dolphin/mtx.h"

#include "REL/m628DLL/scene_model.h"
#include <functional>

extern "C"
{
#include "game/hu3d.h"
}

extern "C"
{
#include "game/charman.h"
}

#include <math.h>
#include "REL/framework/range.h"

extern "C"
{
#include "game/gamework.h"
}

#include "REL/m628DLL/player_controller.h"

#include "REL/framework/chara_motion.h"

#include "REL/framework/event.h"

extern "C"
{
#include "game/audio.h"
}

#include "REL/framework/sound.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/event_hm.h"

#include "datadir_enum.h"

#include "REL/m628DLL/player_actor.h"

#include "REL/framework/model.h"
#include <memory>

#include "REL/m628DLL/computer_lever_controller.h"

// Selects a pad or computer controller for the player.
ActorController *fn_1_14388(PlayerActor *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0)
    {
        return new ComputerLeverController(player, lbl_80265728[playerIndex].comDif);
    }

    return new LeverPadController(player);
}

void fn_1_14C44(PlayerActor *, u32, u8, u8, f32);

static const unsigned int actorMotions[9] = {DATANUM(0x930000, 0),
                                             DATANUM(0x930000, 0x66),
                                             DATANUM(0x930000, 1),
                                             DATANUM(0x930000, 2),
                                             DATANUM(0x930000, 3),
                                             DATANUM(0x930000, 4),
                                             DATANUM(0x8e0000, 162),
                                             DATANUM(0x930000, 6),
                                             DATANUM(0x930000, 40)};

// Creates the character, its collision record and its two lever controls.
PlayerActor::PlayerActor(s32 playerNo, s32 groupCount, f32 *leftLever, f32 *rightLever)
    : GameObject(0x30), PlayerConfigSelection(playerNo, 2), leftLever(leftLever),
      rightLever(rightLever), controller(fn_1_14388(this, playerNo)), playerNo(playerNo),
      unk3C(groupCount == 0), unk40(groupCount), motion(-1), elapsed(0.0f), active(0), state(0),
      callbacks(1)
{
    HU3D_MODEL *modelData;
    Vec soundPosition;

    createCharacterMotions(&static_cast<MotionOwner &>(*this), actorMotions, 9);
    this->overlayMotion = fn_8005B0B8((s16)this->PlayerInfo::characterNo(), DATANUM(0x8e0000, 163));
    this->setLayer(5);
    this->Model::setShadow();
    this->position.x = 541.0f;
    this->position.y = 200.0f;
    this->position.z = 571.0f;
    this->rotation.x = 0.0f;
    this->rotation.y = -135.0f;
    this->rotation.z = 0.0f;
    setPosition(&position);
    this->setRotation(&this->rotation);
    this->velocity.x = 0.0f;
    this->velocity.y = 0.0f;
    this->velocity.z = 0.0f;
    this->displacement.x = 0.0f;
    this->displacement.y = 0.0f;
    this->displacement.z = 0.0f;
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&PlayerActor::fn_1_155A8), this), 0);
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&PlayerActor::fn_1_155EC), this), 2);
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&PlayerActor::fn_1_15704), this), 3);
    registerEventCallback(
        &callbacks, std::bind1st(std::mem_fun(&PlayerActor::fn_1_157E8), this), 4);
    modelData = &lbl_802C01C8[*this];
    if (this->PlayerInfo::characterNo() != 5)
    {
        modelData->ambR = 1.5f;
        modelData->ambG = 1.5f;
        modelData->ambB = 1.5f;
    }
    else
    {
        modelData->ambR = 1.0f;
        modelData->ambG = 1.0f;
        modelData->ambB = 1.0f;
    }
    *this->leftLever = 20.0f;
    *this->rightLever = 20.0f;
    this->sound = HuAudFXPlay(0x71C);
    soundPosition.x = 0.0f;
    soundPosition.y = 0.0f;
    soundPosition.z = 0.0f;
    soundPanTo(1, this->sound, &soundPosition);
    HuAudFXVolSet(this->sound, 0);
    fn_1_14C44(this, DATANUM(0x930000, 0), 0, 0, 8.0f);
    setCallback(&PlayerActor::fn_1_1503C);
}

// Stops the lever sound and releases the character motions.
PlayerActor::~PlayerActor()
{
    HuAudFXStop(sound);
    fn_8005B420((s16)this->PlayerInfo::characterNo(), overlayMotion);
}

// Runs the current player state.
void PlayerActor::update()
{
    if (this->callback)
    {
        (this->*this->callback)();
    }
}

// Changes the character motion unless the requested motion is already active.
void fn_1_14C44(PlayerActor *object, u32 motion, u8 refresh, u8 mode, f32 startTime)
{
    if (object->motion == motion && refresh == 0)
    {
        return;
    }
    object->setAttribute(0x40000402);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*object),
                         motion,
                         startTime,
                         0.0f,
                         mode == 0 ? ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 9)) : 0);
    object->motion = motion;
}

// Selects the idle character motion.
void PlayerActor::fn_1_14CF4()
{
    fn_1_14C44(this, DATANUM(0x930000, 0x66), 0, 1, 8.0f);
}

// Selects the lever motion and resets both lever angles.
void PlayerActor::fn_1_14D78()
{
    this->rightAngle = 0.0f;
    this->leftAngle = 0.0f;
    fn_1_14C44(this, DATANUM(0x8e0000, 162), 0, 0, 8.0f);
    fn_8002CAA8(*this, (s16)this->overlayMotion);
}

// Sets the lever rotations and character and lever motion times.
void PlayerActor::fn_1_14E58()
{
    *this->rightLever = 20.0f + ((20.0f / 7.0f) * this->rightAngle);
    fn_8005C16C((s16)this->PlayerInfo::characterNo(), 8.0f + this->rightAngle);
    *this->leftLever = 20.0f - ((20.0f / 7.0f) * this->leftAngle);
    fn_8002CB64(*this, 8.0f + this->leftAngle);
}

// Selects the result motion and records a winning player.
void PlayerActor::fn_1_14F40(u8 won)
{
    if (won != 0)
    {
        fn_1_14C44(this, DATANUM(0x930000, 6), 0, 1, 8.0f);
        MinigameCore::addWinner(this->playerNo);
        return;
    }
    fn_1_14C44(this, DATANUM(0x930000, 40), 0, 1, 8.0f);
}

// Leaves the player idle until a state callback is selected.
void PlayerActor::fn_1_1503C() {}

// Updates the controller, lever motion and movement events.
void PlayerActor::fn_1_15040()
{
    s32 initialState;
    s32 soundVolume;
    ActorController *inputController;

    inputController = this->controller.get();
    inputController->fn_1_525C();
    initialState = inputController->state.pressedButtons;
    this->velocity.x = (f32)(((100.0f * 2.8f) / 60.0) * (f64)inputController->state.x);
    this->velocity.z = (f32)(((100.0f * 2.8f) / 60.0) * -inputController->state.y);
    this->velocity.y = inputController->state.magnitude;
    this->displacement.x = this->displacement.x + this->velocity.x;
    this->displacement.z = this->displacement.z + this->velocity.z;
    if (this->displacement.x < -400.0f)
    {
        this->displacement.x = -400.0f;
        this->velocity.x = 0.0f;
    }
    if (this->displacement.x > 400.0f)
    {
        this->displacement.x = 400.0f;
        this->velocity.x = 0.0f;
    }
    if (this->displacement.z < -360.0f)
    {
        this->displacement.z = -360.0f;
        this->velocity.z = 0.0f;
    }
    if (this->displacement.z > 360.0f)
    {
        this->displacement.z = 360.0f;
        this->velocity.z = 0.0f;
    }
    this->leftAngle = (f32)(7.0f * -inputController->state.x);
    this->rightAngle = (f32)(7.0f * -inputController->state.y);
    fn_1_14E58();
    sendEvent(0, 0, &this->velocity.x, 0.0f);
    soundVolume = 0x7F;
    if ((this->velocity.x == 0.0f) && (this->velocity.z == 0.0f))
    {
        soundVolume = 0;
    }
    HuAudFXVolSet(this->sound, (s16)soundVolume);
}

// Waits before restoring the idle motion.
void PlayerActor::fn_1_1531C()
{
    if (this->elapsed > 0.8f && this->active == 0)
    {
        fn_1_14CF4();
        this->active = 1;
    }
    this->elapsed += (1.0 / 60.0);
}

// Turns the character toward its final facing direction.
void PlayerActor::fn_1_153E8()
{
    this->state++;
    if (this->state >= 8)
    {
        this->rotation.y += 0.05f * -this->rotation.y;
        f32 distance;
        if (this->rotation.y < 0.0f)
        {
            distance = -this->rotation.y;
        }
        else
        {
            distance = this->rotation.y;
        }
        if (distance < 2.0f)
        {
            this->rotation.y = 0.0f;
            fn_1_14C44(this, DATANUM(0x930000, 0), 0, 0, 8.0f);
            this->setCallback(&PlayerActor::fn_1_1503C);
        }
        else
        {
            fn_1_14C44(this, DATANUM(0x930000, 1), 0, 0, 8.0f);
        }
        this->setRotation(&this->rotation);
    }
}

// Selects the idle player state for the opening event.
void PlayerActor::fn_1_155A8(const EventPacket &)
{
    this->setCallback(&PlayerActor::fn_1_1531C);
}

// Starts the lever motion and active controller state.
void PlayerActor::fn_1_155EC(const EventPacket &)
{
    fn_1_14D78();
    this->setCallback(&PlayerActor::fn_1_15040);
}

// Stops the lever sound and prepares the finish motion.
void PlayerActor::fn_1_15704(const EventPacket &)
{
    s32 attributes;
    s32 motionId;

    attributes = 0x40000402;
    this->setAttribute(attributes);
    fn_8002CAF0(*this);
    motionId = DATANUM(0x930000, 0x66);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), motionId, 8.0f, 0.0f, 0x40000004);
    this->motion = motionId;
    HuAudFXStop(this->sound);
    this->setCallback(&PlayerActor::fn_1_153E8);
}

// Selects the player result from the finish event.
void PlayerActor::fn_1_157E8(const EventPacket &event)
{
    const u8 *finishEvent = static_cast<const u8 *>(event.context);

    if (*finishEvent == 1)
    {
        fn_1_14F40(1);
        return;
    }
    fn_1_14F40(0);
}
