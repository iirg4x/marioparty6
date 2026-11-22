// Creates and updates the player actors and their collision callbacks.

#include "REL/framework/event_callback_registration.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/phase_duration_statics.h"
#include "dolphin/types.h"
extern "C" {
#include "dolphin/os.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/audio.h"
#include "datanum/charmot.h"
#include "game/mg/colman.h"
#include <math.h>
}
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "REL/m673DLL/scene_resources.h"
#include "REL/m673DLL/player_actor.h"
#include "humath.h"
#include "REL/framework/minigame_core.h"
#include "REL/m673DLL/position_table.h"
#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/sound.h"
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"
#include "REL/framework/event_callback_inline.h"
#include <functional>
#include <new>

// Creates a computer or pad controller for the selected player.
PlayerController *fn_1_EE2C(PlayerActor *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0) {
        return new ComController(player, lbl_80265728[playerIndex].comDif);
    }
    return new PadController(player);
}

// Creates the player model and controller and registers the player events.
PlayerActor::PlayerActor(s32 playerIndex, const Position3 *position)
    : GameObject(1), playerIndex(playerIndex), ready(0), controller(), component(60.0f),
      active(1), followEffect(), jumpFrames(0), collisionFrames(0), channel0Events(0),
      channel1Events(1), channel2Events(2), currentMotion(-1), motionRecords()
{
    MGACTOR_PARAM actorParam;
    registerEventCallback(&this->channel0Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_1087C), this), 0);
    registerEventCallback(&this->channel0Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_108E4), this), 2);
    registerEventCallback(&this->channel0Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_1095C), this), 4);
    registerEventCallback(&this->channel0Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_10A70), this), 5);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_10BE0), this), 8);
    registerEventCallback(&this->channel1Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_10C14), this), 7);
    registerEventCallback(&this->channel2Events,
                          std::bind1st(std::mem_fun(&PlayerActor::fn_1_10C48), this), 0);
    this->controller.reset(fn_1_EE2C(this, playerIndex));
    actorParam.height = 150.0f;
    actorParam.radius = 0.6f * 100.0f;
    actorParam.param = playerIndex;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = fn_1_10D0C;
    actorParam.correctHook = 0;
    fn_8005DDEC(3);
    this->player = fn_80080728(playerIndex, &actorParam, 8, 1, 1, 0);
    this->modelId = this->player->actor->mdlId;
    this->character = this->player->charNo;
    fn_80026E44(this->modelId);
    fn_80026B8C(this->modelId, 3);
    fn_8005DFC0(this->character, 1);
    fn_80087158(this->player, 1);
    f32 configurations[4][5] = {
        { 580.0f, 40.0f, 630.0f, 400.0f, 3.0f }, { 580.0f, -40.0f, 630.0f, 320.0f, 3.0f },
        { 550.0f, -70.0f, 520.0f, 290.0f, 3.0f }, { 450.0f, -116.0f, 500.0f, 244.0f, 3.0f }
    };
    f32 *configuration = configurations[this->playerIndex];
    this->positionParameters[0] = configuration[0];
    this->positionParameters[1] = configuration[1];
    this->positionParameters[2] = configuration[2];
    this->positionParameters[3] = configuration[3];
    this->positionParameters[4] = configuration[4];
    Vec offset = { 0.0f, 0.0f, 0.0f };
    offset.x = this->positionParameters[2] * sin((M_PI * (f64)this->positionParameters[1]) / 180.0);
    offset.y = configurations[this->playerIndex][0];
    offset.z = this->positionParameters[2] * cos((M_PI * (f64)this->positionParameters[1]) / 180.0);
    this->component.setPosition(*position);
    this->rotationY = 0.0f;
    fn_80086AA8(this->player->actor, &this->component.position.value);
    fn_80026790(this->modelId, &this->component.position.value);
    fn_80086CA8(this->player->actor, this->rotationY);
    this->fn_1_106A0();
    this->fn_1_107B8(0, 1);
    fn_8005E070(this->character, CHARMOT_HSF_c000m1_320, 0);
    this->component.active = 1;
    this->component.relation = 0;
    SceneResources *resources = &SceneResources::getChecked();
    this->effectPosition.set(this->component.position.value.x,
                             100.0f + this->component.position.value.y,
                             this->component.position.value.z);
    this->followEffect.reset(new FollowEffect(&this->effectPosition.value, 350.0f,
                                              resources->animationAt(3)));
    this->followEffect->setAlpha(0.0f);
    setState(&PlayerActor::fn_1_FD68);
}

// Releases the underlying player actor.
PlayerActor::~PlayerActor()
{
    fn_80080F38(player->actor);
}

// Updates the controller and attached effect position before running the state callback.
void PlayerActor::update()
{
    PlayerController *input = controller.get();
    input->fn_1_BDC();
    effectPosition.set(component.position.value.x, 100.0f + component.position.value.y,
                       component.position.value.z);
    if (state) {
        (this->*state)();
    }
}

// Leaves the player unchanged in this state.
void PlayerActor::fn_1_FD68()
{
}

// Returns to the looping motion after the current motion finishes.
void PlayerActor::fn_1_FD6C()
{
    if (this->currentMotion == 2 && fn_8002D75C(this->modelId) != 0) {
        this->fn_1_107B8(0, 1);
    }
}

// Updates the player position and chooses the next action state.
void PlayerActor::fn_1_FDC0()
{
    Vec position;
    Vec *initialPosition;
    Vec *initialPositionCopy;
    Vec *finalPosition;
    Vec *finalPositionCopy;
    s32 contactRelation;
    s32 currentContactRelation;

    fn_80086BC8(this->player->actor, &position);
    initialPosition = &this->component.position.value;
    initialPositionCopy = initialPosition;
    initialPositionCopy->x = position.x;
    initialPositionCopy->y = position.y;
    initialPositionCopy->z = position.z;
    contactRelation = this->component.contactRelation;
    if (contactRelation != -1) {
        currentContactRelation = this->component.contactRelation;
        if (currentContactRelation == 2) {
            fn_8005E070(this->character, CHARMOT_HSF_c000m1_320, 1);
        }
        CharFXPlay(this->character, 0x240);
        this->component.contactRelation = -1;
        this->component.active = 0;
        fn_80086E3C(this->player);
        fn_80026808(this->modelId, 0.0f, 0.0f, 0.0f);
        fn_8006A030(this->playerIndex, 45, 7, 3);
        this->fn_1_107B8(3, 1);
        fn_80026EF0(this->modelId);
        fn_80026B8C(this->modelId, 7);
        setState(&PlayerActor::fn_1_FFE0);
        this->active = 0;
    }
    if (position.y < -10.0f) {
        fn_80026FCC(this->modelId);
        setState(&PlayerActor::fn_1_10164);
        fn_80086E3C(this->player);
        fn_8006A030(this->playerIndex, 45, 7, 3);
        this->currentMotion = -1;
        this->fn_1_107B8(2, 0);
        this->active = 0;
    }
    finalPosition = &this->component.position.value;
    finalPositionCopy = finalPosition;
    finalPositionCopy->x = position.x;
    finalPositionCopy->y = position.y;
    finalPositionCopy->z = position.z;
    fn_80086B04(this->player->actor, &position);
}

// Raises the player while updating its attached effect.
void PlayerActor::fn_1_FFE0()
{
    Vec position;
    Vec *objectPosition;
    Vec *positionDestination;

    position.x = this->component.position.value.x;
    position.y = this->component.position.value.y;
    position.z = this->component.position.value.z;
    if (position.y < 2000.0) {
        Vec offset;
        FollowEffect *effect;
        f32 blend;
        f64 storedX;
        f64 storedZ;

        offset.x = 0.0f;
        offset.y = 50.0f;
        offset.z = 0.0f;
        PSVECAdd(&offset, &position, &position);
        blend = 1.0f - this->component.position.value.y / 2000.0f;
        effect = this->followEffect.get();
        effect->setAlpha(blend);
        storedZ = this->component.position.value.z;
        storedX = this->component.position.value.x;
        this->effectPosition.value.x = (f32)storedX;
        this->effectPosition.value.y = 100.0f;
        this->effectPosition.value.z = (f32)storedZ;
    } else {
        FollowEffect *effect = this->followEffect.get();

        effect->setAlpha(0.0f);
        this->ready = 1;
        fn_800269E8(this->modelId, 0x1);
    }
    objectPosition = &this->component.position.value;
    positionDestination = objectPosition;
    positionDestination->x = position.x;
    positionDestination->y = position.y;
    positionDestination->z = position.z;
    fn_80026790(this->modelId, &this->component.position.value);
}

// Returns the absolute value of the height.
static inline f64 absoluteHeight(f32 height)
{
    DebugHeight(height);
    return fabs(height);
}

// Lowers the player, then hides its effect and enters the idle state.
void PlayerActor::fn_1_10164()
{
    Vec position;
    Vec *positionAddress;

    if (this->jumpFrames < secondsToFrames(0.5f)) {
        f32 displacement =
            100.0 * sin((M_PI * (180.0f * ((f32)this->jumpFrames / 30.0f))) / 180.0);
        this->jumpFrames++;
        FollowEffect *effect = this->followEffect.get();

        effect->scale = 350.0f + displacement;
        if (this->jumpFrames >= secondsToFrames(0.5f)) {
            this->fn_1_107B8(4, 1);
        }
        return;
    }

    position.x = this->component.position.value.x;
    position.y = this->component.position.value.y;
    position.z = this->component.position.value.z;
    if (position.y > -1500.0f) {
        f32 height;
        f64 storedX;
        f64 storedZ;
        position.y = position.y - 25.0;
        height = position.y;
        f32 opacity = 1.0 - (absoluteHeight(height) / 1600.0);
        FollowEffect *effect = this->followEffect.get();

        effect->setAlpha(opacity);
        FollowEffect *scaleEffect = this->followEffect.get();

        scaleEffect->scale = 350.0 * opacity * opacity;
        storedZ = this->component.position.value.z;
        storedX = this->component.position.value.x;
        this->effectPosition.value.x = (f32)storedX;
        this->effectPosition.value.y = 100.0f;
        this->effectPosition.value.z = (f32)storedZ;
    } else {
        FollowEffect *effect = this->followEffect.get();

        effect->setAlpha(0.0f);
        fn_800269E8(this->modelId, 0x1);
        this->ready = 1;
        setState(&PlayerActor::fn_1_FD68);
    }
    positionAddress = &this->component.position.value;
    Vec *positionDestination = positionAddress;
    positionDestination->x = position.x;
    positionDestination->y = position.y;
    positionDestination->z = position.z;
    fn_80026790(this->modelId, &position);
}

// Returns the player's rotation toward zero and resumes the idle state.
void PlayerActor::fn_1_104A0()
{
    f32 rotation;

    fn_80086CB0(this->player->actor, &rotation);
    if (rotation > 0.0f) {
        if (fabs(rotation) > 4.0) {
            rotation -= 4.0f;
        } else {
            rotation = 0.0f;
            this->ready = 1;
            this->fn_1_107B8(0, 1);
            setState(&PlayerActor::fn_1_FD68);
        }
    } else {
        if (fabs(rotation) > 4.0) {
            rotation += 4.0f;
        } else {
            rotation = 0.0f;
            this->ready = 1;
            this->fn_1_107B8(0, 1);
            setState(&PlayerActor::fn_1_FD68);
        }
    }
    fn_80086CA8(this->player->actor, rotation);
}

// Idle callback installed when the ending motion is selected.
void PlayerActor::fn_1_1069C()
{
}

// Loads the player's motion resources.
void PlayerActor::fn_1_106A0()
{
    this->motionRecords.clear();
    this->fn_1_10744(CHARMOT_HSF_c000m1_300);
    this->fn_1_10744(CHARMOT_HSF_c000m1_301);
    this->fn_1_10744(CHARMOT_HSF_c000m1_324);
    this->fn_1_10744(CHARMOT_HSF_c000m1_320);
    this->fn_1_10744(CHARMOT_HSF_c000m1_344);
    this->fn_1_10744(CHARMOT_HSF_c000m1_349);
    this->fn_1_10744(CHARMOT_HSF_c000m1_351);
    fn_8005B4BC(this->character);
}

// Appends a loaded character motion to the player's motion list.
void PlayerActor::fn_1_10744(u32 dataNum)
{
    HU3D_MOTIONID motionId = fn_8005B0B8(this->character, dataNum);
    this->motionRecords.push_back(motionId);
}

// Shifts the player's character motion when the requested motion changes.
void PlayerActor::fn_1_107B8(u32 motionId, u8 loop)
{
    s16 motion;

    if (this->currentMotion != motionId) {
        motion = this->motionRecords.at(motionId);
        fn_800269E8(this->modelId, 0x40000002);
        fn_8005C2C8(this->character, motion, 0.0f, 8.0f,
                    loop != 0 ? 0x40000001 : 0);
        fn_8005C390(this->character, 1.0f);
        this->currentMotion = motionId;
    }
}

// Resets the player actor and selects the motion-completion state.
void PlayerActor::fn_1_1087C(const EventPacket &)
{
    fn_80086E3C(this->player);
    setState(&PlayerActor::fn_1_FD6C);
}

// Starts the player action at the current collision position.
void PlayerActor::fn_1_108E4(const EventPacket &)
{
    fn_80086F00(this->player, &this->component.position.value);
    setState(&PlayerActor::fn_1_FDC0);
    fn_80087168(this->player, 1);
}

// Disables collision and starts the player's return rotation if the player remains in play.
void PlayerActor::fn_1_1095C(const EventPacket &)
{
    this->component.active = 0;
    fn_80087158(this->player, 1);
    if (this->active != 0) {
        fn_8008697C(this->player, &this->component.position.value);
        this->fn_1_107B8(1, 1);
        setState(&PlayerActor::fn_1_104A0);
    }
}

// Records an active player as a winner and selects the ending motion.
void PlayerActor::fn_1_10A70(const EventPacket &)
{
    setState(&PlayerActor::fn_1_1069C);
    fn_80086E3C(this->player);
    if (this->active != 0) {
        MinigameCore::addWinner(this->playerIndex);
        this->fn_1_107B8(5, 0);
    } else {
        this->fn_1_107B8(6, 0);
    }
}

// Shows the player's attached effect.
void PlayerActor::fn_1_10BE0(const EventPacket &)
{
    FollowEffect *effect = this->followEffect.get();
    effect->setAlpha(1.0f);
}

// Hides the player's attached effect.
void PlayerActor::fn_1_10C14(const EventPacket &)
{
    FollowEffect *effect = this->followEffect.get();
    effect->setAlpha(0.0f);
}

// Calls fn_8006A030 for the player and requests motion 2.
void PlayerActor::fn_1_10C48(const EventPacket &)
{
    fn_8006A030(this->playerIndex, 45, 7, 3);
    this->fn_1_107B8(2, 0);
}

// Handles player collisions and increments the selected player's collision count.
int fn_1_10D0C(COL_NARROW_PARAM *otherContact, COL_NARROW_PARAM *contact)
{
    ScenePositionTable *table;
    PlayerActor *player;
    int index;
    ScenePositionTable *singletonTable;
    ScenePositionTable *registeredTable;
    PlayerActor *selectedPlayer;
    PlayerActor *collidingPlayer;

    if (contact->type != 0) {
        return 0;
    }
    (MinigameSingleton<ScenePositionTable>::self != 0) ? (void)0
        : __msl_assertion_failed("self != 0", "singleton.h", 38);
    singletonTable = MinigameSingleton<ScenePositionTable>::self;
    registeredTable = singletonTable;
    table = registeredTable;
    index = contact->paramB;
    selectedPlayer = table->players.at(index);
    collidingPlayer = selectedPlayer;
    player = collidingPlayer;
    player->fn_1_10DC0();
    return 1;
}

// Increments the player's collision count.
void PlayerActor::fn_1_10DC0()
{
    this->collisionFrames += 1;
}
