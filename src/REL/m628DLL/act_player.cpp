#include "REL/framework/event_manager.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/frame_durations.h"
#include "REL/game_gravity.h"
extern "C"
{
#include "dolphin/types.h"
}

extern "C"
{
#include "math.h"
}

extern "C"
{
#include "dolphin/os.h"
}

extern "C"
{
#include "dolphin/mtx.h"
}

#include "REL/framework/game_object.h"

#include "REL/framework/event_hm.h"
#include <new>
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_inline.h"

#include "REL/framework/game_object_man.h"

extern "C"
{
#include "game/hu3d.h"
}

extern "C"
{
#include "game/charman.h"
}

extern "C"
{
#include "game/gamework.h"
}

extern "C"
{
#include "game/mg/actman.h"
}

extern "C"
{
#include "game/audio.h"
}

#include "REL/m628DLL/player_input_binding.h"

#include "REL/m628DLL/range_record.h"

extern "C"
{
#include "game/main.h"
}

extern "C"
{
#include "game/object.h"
}

extern "C"
{
#include "game/wipe.h"
}

extern "C"
{
#include "game/mg/timer.h"
}

#include "REL/framework/range.h"

#include "REL/m628DLL/board_positions.h"

#include <vector>

#include <memory>

#include <functional>

#include "REL/m628DLL/computer_player_controller.h"
#include "REL/framework/sound.h"
#include "REL/framework/pad_control.h"
#include "REL/m628DLL/act_player.h"

extern "C"
{
u32 lbl_1_bss_10;
}

extern "C" const HuVecF lbl_1_rodata_10[4] = {
    {0.0f, 0.0f, 0.0f}, {-200.0f, 0.0f, -200.0f}, {200.0f, 0.0f, -200.0f}, {-200.0f, 0.0f, 200.0f}};

int fn_1_E70(COL_NARROW_PARAM *unused, COL_NARROW_PARAM *state);

void fn_1_F28(ActPlayer *object);

// Selects a pad or computer input controller for the player.
PlayerInputController *fn_1_0(ActPlayer *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0)
    {
        return new ComputerPlayerController(player, lbl_80265728[playerIndex].comDif);
    }

    return new PlayerInputBinding(player);
}

// Creates the player model, motions, collision record and input controller.
ActPlayer::ActPlayer(s32 playerNo, s32 mode)
    : GameObject(0x40), motionTime(0.0f), unk54(0), playerNo(playerNo), playerFlag(mode == 0),
      mode(mode), locked(0), finishComplete(0), state(0), elapsed(0.0f), spawned(0), unkA0(0.0f),
      unkA4(0), unkA8(0), channelOneCallbacks(1), channelZeroCallbacks(0), motionId(-1)
{
    HuVecF start;
    MGACTOR_PARAM actorParam;

    registerEventCallback(
        &channelOneCallbacks, std::bind1st(std::mem_fun(&ActPlayer::fn_1_2F4C), this), 0);
    registerEventCallback(
        &channelOneCallbacks, std::bind1st(std::mem_fun(&ActPlayer::fn_1_3030), this), 2);
    registerEventCallback(
        &channelOneCallbacks, std::bind1st(std::mem_fun(&ActPlayer::fn_1_30D0), this), 3);
    registerEventCallback(
        &channelOneCallbacks, std::bind1st(std::mem_fun(&ActPlayer::fn_1_3254), this), 4);
    registerEventCallback(
        &channelZeroCallbacks, std::bind1st(std::mem_fun(&ActPlayer::fn_1_338C), this), 0);

    if (playerFlag == 1)
    {
        start.x = 0.0f;
        start.y = 0.0f;
        start.z = 200.0f;
    }
    else
    {
        s32 startIndex = this->mode;
        start.x = lbl_1_rodata_10[startIndex].x;
        start.y = lbl_1_rodata_10[startIndex].y;
        start.z = lbl_1_rodata_10[startIndex].z;
    }
    static_cast<Vec &>(position) = record.setPosition(start);
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    unk7C[0] = 0;
    unk7C[1] = 0;
    unk7C[2] = 0;
    rotation = 0.0f;
    hit = 0;

    inputController.reset(fn_1_0(this, playerNo));

    actorParam.height = 150.0f;
    actorParam.radius = (100.0f * 0.6f);
    actorParam.param = mode;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = fn_1_E70;
    actorParam.correctHook = 0;
    player = fn_80080728((s16)playerNo, &actorParam, 4, 1, 0x23, 0);
    fn_80086AA8(player->actor, &start);
    fn_8007ED60(player);
    fn_80086E3C(player);
    modelId = player->actor->mdlId;
    characterModelId = player->charNo;
    fn_80026E44(modelId);
    fn_8005DDEC(7);
    fn_80026B8C(modelId, 7);
    fn_1_1308();
    fn_1_FB4();

    setCallback(&ActPlayer::fn_1_1C8C);
    lbl_1_bss_10 = 0;
}

// Releases the character model resources.
ActPlayer::~ActPlayer()
{
    fn_80080F38(player->actor);
}

// Marks the selected player when the collision parameter is active.
int fn_1_E70(COL_NARROW_PARAM *unused, COL_NARROW_PARAM *state)
{
    s32 mode;
    BoardPositions *object;
    ActPlayer *result;

    if (state->type)
    {
        return 0;
    }
    mode = state->paramB;
    if (mode == 0)
    {
        return 0;
    }
    object = &BoardPositions::getChecked();
    result = fn_1_12540(object, mode);
    fn_1_F28(result);
    return 1;
}

// Marks the player as hit.
void fn_1_F28(ActPlayer *object)
{
    object->unkA4 = 1;
    object->unkA8++;
}

// Loads one character motion and records its handle.
void ActPlayer::fn_1_F40(u32 dataNum)
{
    s16 motion = fn_8005B0B8(characterModelId, dataNum);
    m_motion_id.push_back(motion);
}

// Loads the player motion set and installs the initial motion.
void ActPlayer::fn_1_FB4()
{
    m_motion_id.clear();

    fn_1_F40(DATANUM(0x930000, 0x0000));
    fn_1_F40(DATANUM(0x930000, 0x0017));
    fn_1_F40(DATANUM(0x930000, 0x0018));
    fn_1_F40(DATANUM(0x930000, 0x0004));
    fn_1_F40(DATANUM(0x930000, 0x0014));
    fn_1_F40(DATANUM(0x930000, 0x0039));
    fn_1_F40(DATANUM(0x930000, 0x0001));
    fn_1_F40(DATANUM(0x930000, 0x0027));
    fn_1_F40(DATANUM(0x930000, 0x0028));

    m_motion_id.size() == NUMBEROF_MOTION
        ? (void)0
    // The assertion records line 221 in the original source.
        : __msl_assertion_failed("m_motion_id.size() == NUMBEROF_MOTION", "act_player.cpp", 221);
    fn_8005B4BC(characterModelId);
}

// Changes the player motion unless it is already active.
void ActPlayer::fn_1_124C(u32 motionId, u8 refresh, u8 mode, f32 startTime)
{
    s16 motion;

    if (this->motionId == motionId && refresh == 0)
    {
        return;
    }
    motion = m_motion_id.at(motionId);
    fn_8005C430(characterModelId, 0x40000002U);
    fn_8005C2C8(characterModelId, motion, 0.0f, startTime, mode == 0 ? 0x40000001 : 0);
    this->motionId = motionId;
}

// Saves the model material colors.
void ActPlayer::fn_1_1308()
{
    HU3D_MODEL *model = lbl_802C01C8 + modelId;
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++)
    {
        GXColor color;

        color.r = material->color[0];
        color.g = material->color[1];
        color.b = material->color[2];
        colors.push_back(color);
    }
}

// Restores the saved material colors.
void ActPlayer::fn_1_13B8()
{
    HU3D_MODEL *model = lbl_802C01C8 + modelId;
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++)
    {
        GXColor color = colors.at(materialIndex);
        material->color[0] = color.r;
        material->color[1] = color.g;
        material->color[2] = color.b;
    }

    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;
}

// Applies a color to every model material.
void ActPlayer::fn_1_14A0(f32 red, f32 green, f32 blue)
{
    HU3D_MODEL *model = lbl_802C01C8 + modelId;
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++)
    {
        material->color[0] *= red;
        material->color[1] *= green;
        material->color[2] *= blue;
    }

    model->ambR = red;
    model->ambG = green;
    model->ambB = blue;
}

// Sets the player model attribute.
void ActPlayer::fn_1_15C0()
{
    fn_80026A44(modelId, 1);
}

// Clears the player model attribute.
void ActPlayer::fn_1_15F4()
{
    fn_800269E8(modelId, 1);
}

// Refreshes the player position and its collision record.
void ActPlayer::fn_1_1628()
{
    Vec actorPosition;
    fn_80086BC8(player->actor, &actorPosition);
    static_cast<Vec &>(position) = record.setPosition(actorPosition);
    record.position.y += 50.0f;
}

// Applies the player rotation to its character.
void ActPlayer::fn_1_16D0()
{
    fn_80086CB0(player->actor, &rotation);
}

// Clamps the player to the board bounds and reports any correction.
u8 ActPlayer::fn_1_1708()
{
    u8 clamped;
    f32 xLimit;
    f32 zLimit;

    if (locked == 1)
    {
        return 0;
    }
    clamped = 0;
    xLimit = 390.0f;
    if (position.x < -xLimit)
    {
        position.x = -xLimit;
        clamped = 1;
    }
    if (position.x > xLimit)
    {
        position.x = xLimit;
        clamped = 1;
    }

    zLimit = 390.0f;
    if (position.z < -zLimit)
    {
        position.z = -zLimit;
        clamped = 1;
    }
    if (position.z > zLimit)
    {
        position.z = zLimit;
        clamped = 1;
    }
    return clamped;
}

// Reads input, runs the player state and updates its collision position.
void ActPlayer::update()
{
    PlayerInputController *controller = inputController.get();
    u8 didClamp;
    u8 didChange;

    fn_1_52D4(controller);
    if (callback)
    {
        (this->*callback)();
    }
    didClamp = 0;
    if (position.y < 0.0f)
    {
        position.y = 0.0f;
        didClamp = 1;
    }
    didChange = fn_1_1708();
    if (didClamp == 1 || didChange == 1)
    {
        fn_80086B04(player->actor, &position);
        fn_80026790(modelId, &position);
    }
    record.position.y = 50.0f + position.y;
    this->unk54++;
}

// Updates the active player motion and hit response.
void ActPlayer::fn_1_1970()
{
    f64 verticalVelocity;
    s32 hitState;
    fn_1_1628();
    fn_1_16D0();
    hitState = record.getHitState();
    if (record.isActive() != 0)
    {
        if (hitState == 1)
        {
            motionTime = 0.0f;
            verticalVelocity = 200.0f * gravityPerFrame;
            velocity.x = 0.0f;
            velocity.y = (f32)verticalVelocity;
            velocity.z = 0.0f;
            fn_80086E3C(player);
            fn_80026A44(modelId, 1);
            locked = 1;
            spawned = 0;
            fn_8006A030((s16)playerNo, 90, 4, 4);
            CharFXPlay(characterModelId, 0x240);
            fn_800805CC(player->actor, 0x10);
            fn_80087158(player, 1);
            fn_80087158(player, 0x300U);
            record.active = 0;
            soundPlayAt(1, 0x71D, &position);
            soundPlayAt(1, 0x71E, &position);
            fn_1_124C(4, 0, 0, 4.0f);

            setCallback(&ActPlayer::fn_1_2400);

            return;
        }
        if (hitState == 0)
        {
            hit = 1;
            fn_80087158(player, 1);
            fn_80086E3C(player);
            frame = 0;
            fn_800269E8(modelId, 0x40000002U);
            spawned = 0;
            fn_8006A030((s16)playerNo, 20, 7, 3);
            soundPlayAt(1, 0x71D, &position);

            setCallback(&ActPlayer::fn_1_1E40);
        }
    }
}

// Leaves the player unchanged while waiting for a state.
void ActPlayer::fn_1_1C8C() {}

// Advances the timed player animation sequence.
void ActPlayer::fn_1_1C90()
{
    switch (state)
    {
    case 0:
        elapsed += (1.0 / 60.0);
        if (elapsed < 2.5f)
        {
            break;
        }
        fn_8006A030(playerNo, 20, 7, 3);
        fn_1_124C(2, 0, 1, 4.0f);
        state++;
    case 1:
        if (!fn_8005C240(characterModelId))
        {
            break;
        }
        fn_1_124C(0, 0, 0, 4.0f);
        state++;
    case 2:
        state++;
        break;
    }
}

// Marks the finish animation complete.
void ActPlayer::fn_1_1E34()
{
    finishComplete = 1;
}

// Spawns the player and advances its opening state.
void ActPlayer::fn_1_1E40()
{
    if ((frame & 2) == 0)
    {
        fn_1_14A0(1.0f, 0.5f, 0.5f);
    }
    else
    {
        fn_1_13B8();
    }
    frame++;
    if (!((f32)frame < 12.0f))
    {
        frame = 0;
        fn_1_13B8();
        fn_1_1628();
        fn_80086F00(player, &position);
        fn_80087168(player, 1);
        fn_80026A44(modelId, 0x40000002U);
        spawned = 1;
        setCallback(&ActPlayer::fn_1_2298);
    }
}

// Flashes the player model for nine frames and refreshes its collision position.
void ActPlayer::fn_1_2298()
{
    StateCallback nextState;
    if ((frame & 1) == 0)
    {
        fn_800269E8(modelId, 1);
    }
    else
    {
        fn_80026A44(modelId, 1);
    }
    fn_1_1628();
    frame++;
    if (!((f32)frame < 9.0f))
    {
        fn_80026A44(modelId, 1);
        hit = 0;

        nextState = &ActPlayer::fn_1_1970;
        callback = nextState;
    }
}

// Updates the moving player and its vertical motion.
void ActPlayer::fn_1_2400()
{
    f32 verticalVelocity = velocity.y;
    velocity.y += -gravityPerFrame;
    verticalVelocity = 0.5f * (verticalVelocity + velocity.y);
    position.z += 30.0;
    position.y += verticalVelocity;
    fn_80026790(modelId, &position);
    fn_80086B04(player->actor, &position);
    fn_800805CC(player->actor, 0x10);
    motionTime += (1.0 / 60.0);
    if (!(motionTime < 2.0f))
    {
        fn_1_13B8();
        f64 depthPosition = -(100.0f * ((f32)(mode - 1) * 1.5f)) - (100.0f * 0.6f);
        position.x = -650.0f;
        position.y = 1000.0f;
        position.z = depthPosition;
        fn_80086B04(player->actor, &position);
        fn_80086F00(player, &position);
        fn_8007EEBC(player, 0);
        fn_8008697C(player, &position);
        setCallback(&ActPlayer::fn_1_2AD8);
    }
}

// Restores player colors and selects the following state.
void ActPlayer::fn_1_26C4()
{
    fn_80026A44(modelId, 0x40000002U);
    fn_1_13B8();
    setCallback(&ActPlayer::fn_1_1C8C);
}

// Turns and moves the player through its finish sequence.
void ActPlayer::fn_1_27FC()
{
    Vec rotationVector;
    fn_1_1628();
    if (!(position.y > 10.0f) && player->motNo == 0)
    {
        position.y = 0.0f;
        fn_80026790(modelId, &position);
        fn_80086B04(player->actor, &position);
        fn_1_124C(6, 0, 0, 4.0f);
        if ((rotation < 0.0f ? -rotation : rotation) < 2.0f)
        {
            rotation = 0.0f;
            fn_1_124C(0, 0, 0, 4.0f);
            setCallback(&ActPlayer::fn_1_1E34);
        }
        rotation = rotation + (-rotation / 12.0f);
        fn_80086CA8(player->actor, rotation);
        f64 finalRotation = rotation;
        rotationVector.x = 0.0f;
        rotationVector.y = finalRotation;
        rotationVector.z = 0.0f;
        fn_80026838(modelId, &rotationVector);
    }
}

// Updates the player position and motion during its result sequence.
void ActPlayer::fn_1_2AD8()
{
    Vec rotationVector;
    fn_1_1628();
    position.x = -650.0f;
    position.z = -(100.0f * ((f32)(mode - 1) * 1.5f)) - (100.0f * 0.6f);
    fn_80026790(modelId, &position);
    fn_80086B04(player->actor, &position);
    if (position.y > 200.0f)
    {
        return;
    }
    position.y = 200.0f;
    fn_80086E3C(player);
    fn_80026790(modelId, &position);
    fn_80086B04(player->actor, &position);
    rotationVector.x = 0.0f;
    rotationVector.y = 0.0f;
    rotationVector.z = 0.0f;
    fn_80026838(modelId, &rotationVector);
    fn_8005F968(characterModelId, &position);
    fn_1_124C(3, 0, 1, 4.0f);
    setCallback(&ActPlayer::fn_1_2D40);
}

// Waits for a motion to finish before selecting the next state.
void ActPlayer::fn_1_2D40()
{
    if (fn_8005C240(characterModelId))
    {
        fn_1_124C(5, 0, 0, 4.0f);
        setCallback(&ActPlayer::fn_1_1E34);
    }
}

// Selects the result motion and records a winning player.
void ActPlayer::fn_1_2E28(u8 active)
{
    fn_80086E3C(player);
    spawned = 0;
    if (active != 0)
    {
        fn_1_124C(7, 0, 1, 4.0f);
        MinigameCore::addWinner(playerNo);
        return;
    }
    fn_1_124C(8, 0, 1, 4.0f);
}

// Selects the opening player motion.
void ActPlayer::fn_1_2F4C(const EventPacket &event)
{
    fn_80026790(modelId, &position);
    fn_1_124C(1, 0, 0, 4.0f);
    setCallback(&ActPlayer::fn_1_1C90);
}

// Activates the player and its collision record.
void ActPlayer::fn_1_3030(const EventPacket &event)
{
    fn_80086F00(player, &position);
    spawned = 1;
    inputController->invoke();
    record.active = 1;
    setCallback(&ActPlayer::fn_1_1970);
}

// Leaves the input unchanged when no override is installed.
void PlayerInputController::invoke() {}

// Chooses the next player state from the round event.
void ActPlayer::fn_1_30D0(const EventPacket &event)
{
    if (locked != 1)
    {
        s32 eventValue = *(s32 *)event.context;

        fn_80086F00(player, &position);
        fn_80087158(player, 1);
        fn_1_13B8();
        fn_80026A44(modelId, 1);
        setCallback(&ActPlayer::fn_1_27FC);
        lbl_1_bss_10++;
    }
}

// Chooses the winning or losing result from the event.
void ActPlayer::fn_1_3254(const EventPacket &event)
{
    u8 *active = (u8 *)event.context;

    if (*active == 0)
    {
        fn_1_2E28(1);
    }
    else
    {
        fn_1_2E28(0);
    }
}

// Leaves the player unchanged for this event.
void ActPlayer::fn_1_338C(const EventPacket &event) {}
