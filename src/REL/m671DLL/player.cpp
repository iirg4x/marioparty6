#include "REL/m671DLL/member_callback.h"
#include "REL/m671DLL/position_table.h"
#include "REL/framework/phase_durations.h"
#include "REL/framework/sound.h"
/* The player: its controller, collision sphere, motions and event handlers. */
#include <cmath>
#include <memory>
#include <vector>

#include "dolphin.h"
#include "humath.h"
extern "C" {
#include "game/gamework.h"
#include "game/data.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "datanum/charmot.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/audio.h"
}
}

#include "REL/framework/event_callback_inline.h"
#include "REL/m671DLL/player_object.h"
#include "REL/framework/model.h"
#include "REL/framework/event.h"
#include "REL/framework/minigame_core.h"

extern "C" {

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

}

using std::cos;
using std::fabs;
using std::sin;

Controller *fn_1_C4DC(Player *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0) {
        return new ComputerControl(player, lbl_80265728[playerIndex].comDif);
    }

    return new HumanControl(player);
}

template <class Owner, class Payload>
void addCallback(EventHandlerManager *owner, const Payload &payload, int key)
{
    owner->addHandler(new MemberCallback<Owner>(payload), key);
}

extern const unsigned long MotionResources[2] = {DATANUM(0x820000, 6), DATANUM(0x820000, 7)};

struct PlayerPlacement {
    f32 height;
    f32 angle;
    f32 radius;
    f32 perPlayerEffectValue;
    f32 commonEffectValue;
};

Player::Player(int playerNo)
    : GameObject(0), playerNo(playerNo), finished(0), controller(0), collision(60.0f),
      motionEnabled(1), initializedByte(0), updateCount(0), callbacks(0), secondaryCallbacks(1),
      motionResource(-1), motionIds(), effectMotion(-1)
{
    MGACTOR_PARAM actorParam;
    PlayerPlacement *position;

    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::fn_1_DF34), this), 1);
    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::fn_1_DEF0), this), 0);
    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::onStart), this), 3);
    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::fn_1_E09C), this), 5);
    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::fn_1_E174), this), 6);
    addCallback<Player>(&callbacks, std::bind1st(std::mem_fun(&Player::fn_1_E268), this), 7);
    addCallback<Player>(&secondaryCallbacks, std::bind1st(std::mem_fun(&Player::fn_1_E3D8), this), 0);
    controller.reset(fn_1_C4DC(this, playerNo));
    actorParam.height = 150.0f;
    actorParam.radius = 60.000004f;
    actorParam.param = playerNo;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = 0;
    actorParam.correctHook = 0;
    fn_8005DDEC(6);
    static unsigned int motions[] = {
        DATANUM(0x930000, 0),
        DATANUM(0x930000, 1),
        DATANUM(0x930000, 2),
        DATANUM(0x930000, 3),
        DATANUM(0x930000, 4),
        DATANUM(0x930000, 5),
        DATANUM(0x930000, 7),
        DATANUM(0x930000, 8),
        DATANUM(0x930000, 10),
        DATANUM(0x930000, 9),
        DATANUM(0x930000, 15),
        DATANUM(0x930000, 16),
        DATANUM(0x930000, 17),
        DATANUM(0x930000, 18),
        DATANUM(0x930000, 22),
        DATANUM(0x930000, 22),
        0,
    };
    this->player = fn_80080728((s16)playerNo, &actorParam, 4, 1, 1, motions);
    this->modelId = this->player->actor->mdlId;
    this->charNo = this->player->charNo;
    fn_80026E44(this->modelId);
    fn_80026B8C(this->modelId, 3);
    fn_8005DFC0(this->charNo, 1);
    fn_80087158(this->player, (1 << 0));
    PlayerPlacement placements[4] = {
        {580.0f, 40.0f, 630.0f, 400.0f, 3.0f},
        {580.0f, -40.0f, 630.0f, 320.0f, 3.0f},
        {550.0f, -70.0f, 520.0f, 290.0f, 3.0f},
        {450.0f, -116.0f, 500.0f, 244.0f, 3.0f},
    };
    position = &placements[this->playerNo];
    effectHeight = position->height;
    effectAngle = position->angle;
    effectRadius = position->radius;
    effectParameter3 = position->perPlayerEffectValue;
    effectParameter4 = position->commonEffectValue;
    effect = new ModelMotion(DATANUM(0x820000, 5));
    effect->setRotationY(90.0f + effectAngle);
    Vec effectPosition = {0.0f, 0.0f, 0.0f};
    effectPosition.x = effectRadius * sin((M_PI * (f64)effectAngle) / 180.0);
    effectPosition.y = placements[this->playerNo].height;
    effectPosition.z = effectRadius * cos((M_PI * (f64)effectAngle) / 180.0);
    effect->addMotions(MotionResources, 2);
    effect->setPosition(&effectPosition);
    effect->setLayer(3);
    effect->setAttribute(1);
    effect->setShadow();
    collision.value.x = 0.0f;
    collision.value.y = 0.0f;
    collision.value.z = 0.0f;
    unknown60.x = 0.0f;
    unknown60.y = 0.0f;
    unknown60.z = 0.0f;
    movementVelocity.x = 0.0f;
    movementVelocity.y = 0.0f;
    movementVelocity.z = 0.0f;
    switch (playerNo) {
    case 0:
        collision.value.x = -200.0f;
        collision.value.z = -200.0f;
        rotation = 45.0f;
        break;
    case 1:
        collision.value.x = 200.0f;
        collision.value.z = -200.0f;
        rotation = -45.0f;
        break;
    case 2:
        collision.value.x = -200.0f;
        collision.value.z = 200.0f;
        rotation = 135.0f;
        break;
    case 3:
        collision.value.x = 200.0f;
        collision.value.z = 200.0f;
        rotation = -135.0f;
        break;
    }
    fn_80086AA8(this->player->actor, &collision.value);
    fn_80086CA8(this->player->actor, rotation);
    fn_1_DC70();
    fn_8005E070(this->charNo, DATANUM(0x930000, 20), 0);
    collision.active = 1;
    collision.relation = 0;
    setUpdate(&Player::fn_1_D3A0);
}

Player::~Player()
{
    delete effect;
    fn_80080F38(player->actor);
}

void Player::update()
{
    Controller *manager = controller.get();
    fn_1_13C8(manager);
    if (currentCallback) {
        (this->*currentCallback)();
    }
}

void Player::fn_1_D3A0()
{
}

void Player::fn_1_D3A4()
{
}

void Player::fn_1_D3A8()
{
    int id = charNo;

    if (fn_8005C240(id) != 0) {
        fn_1_DD88(0, 1);
        setUpdate(&Player::fn_1_D3A0);
    }
}

extern const Vec lbl_1_rodata_4EC = {0.0f, 0.0f, 0.0f};

void Player::fn_1_D434()
{
    Vec actorPosition;
    Vec *positionStorage;
    Vec *position;
    Vec *positionResultStorage;
    Vec *positionResult;
    PositionTable &positionTable = PositionTable::getChecked();
    fn_80086BC8(player->actor, &actorPosition);
    positionStorage = &collision.value;
    position = positionStorage;
    position->x = actorPosition.x;
    position->y = actorPosition.y;
    position->z = actorPosition.z;
    {
        if (collision.getState() != -1) {
            Vec hitNormal;
            Vec hitPosition = collision.value;
            f32 motionScale;
            f32 distance;
            hitPosition.y = 100.0f;
            fn_8005D864(1, &hitPosition, &hitNormal);
            CharFXPlay(charNo, 0x240);
            soundPlayAt(1, 0x8A0, &collision.value);
            collision.state = -1;
            collision.active = 0;
            fn_80086E3C(player);
            fn_80026808(modelId, 0.0f, 0.0f, 0.0f);
            fn_8006A030((s16)playerNo, 0x2D, 7, 3);
            fn_1_DD88(3, 1);
            fn_80026EF0(modelId);
            setUpdate(&Player::fn_1_D6DC);
            PSVECSubtract(&lbl_1_rodata_4EC, &collision.value, &movementVelocity);
            movementVelocity.y = 0.0f;
            distance = PSVECMag(&movementVelocity);
            motionScale = (f32)(0.016666666666666666 * (2.0 * (f64)distance));
            if (movementVelocity.x != 0.0f || 0.0f != movementVelocity.z) {
                PSVECNormalize(&movementVelocity, &movementVelocity);
            }
            PSVECScale(&movementVelocity, &movementVelocity, motionScale);
            motionEnabled = 0;
        }
    }
    positionResultStorage = &collision.value;
    positionResult = positionResultStorage;
    positionResult->x = actorPosition.x;
    positionResult->y = actorPosition.y;
    positionResult->z = actorPosition.z;
    fn_80086B04(player->actor, &actorPosition);
}

void Player::fn_1_D6DC()
{
    Vec position;
    Vec offset;
    Vec *positionStorage;
    Vec *ownerPosition;
    position.x = collision.value.x;
    position.y = collision.value.y;
    position.z = collision.value.z;
    if (position.y < 1500.0) {
        f64 offsetX;
        f64 offsetZ = movementVelocity.z;
        offsetX = movementVelocity.x;
        offset.x = offsetX;
        offset.y = 50.0f;
        offset.z = offsetZ;
        PSVECAdd(&offset, &position, &position);
    } else {
        Vec motionPosition = *effect->getPosition(&motionPosition);
        position.x = motionPosition.x;
        position.y = motionPosition.y;
        position.z = motionPosition.z;
        setUpdate(&Player::fn_1_D978);
        soundId = soundPlayAt(1, 0x8A3, &motionPosition);
        fn_8008697C(player, &position);
        sendEvent(0, 8, 0.0f);
        effect->resetAttribute(1);
        fn_1_DD88(4, 1);
        fn_1_DE4C(0x820006, 0);
        fn_8002D414(charNo, 30.0f);
        fn_8005C390(charNo, 1.5f);
        fn_80026808(modelId, 0.0f, 90.0f, 0.0f);
        collision.value.x = 0.0f;
        collision.value.y = 0.0f;
        collision.value.z = 0.0f;
        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        fn_8002741C(*effect, "itemhook_char", modelId);
    }
    positionStorage = &collision.value;
    ownerPosition = positionStorage;
    ownerPosition->x = position.x;
    ownerPosition->y = position.y;
    ownerPosition->z = position.z;
    fn_80026790(modelId, &collision.value);
}

void Player::fn_1_D978()
{
    ModelMotion *model;
    s32 completedMotionId;
    s32 motionId;
    s32 motionIdCopy;

    effect->getPosition(&collision.value);
    model = effect;
    motionId = (s16)model->modelId;
    motionIdCopy = motionId;
    completedMotionId = motionIdCopy;
    static const char *hookName = "char";
    ++updateCount;
    if (updateCount == 0xB4) {
        Vec soundPosition = *effect->getPosition(&soundPosition);
        HuAudFXStop(soundId);
        soundPlayAt(1, 0x8A4, &soundPosition);
    }
    if (fn_8002D75C(completedMotionId) != 0) {
        finished = 1;
    }
}

/* Turns the player toward the nearest zero-angle position. */
void Player::fn_1_DA70()
{
    f32 angle;

    fn_80086CB0(player->actor, &angle);
    if (angle > 0.0f) {
        if (fabs(angle) > 4.0) {
            angle -= 4.0f;
        } else {
            angle = 0.0f;
            finished = 1;
            fn_1_DD88(0, 0);
            setUpdate(&Player::fn_1_D3A0);
        }
    } else {
        if (fabs(angle) > 4.0) {
            angle += 4.0f;
        } else {
            angle = 0.0f;
            finished = 1;
            fn_1_DD88(0, 0);
            setUpdate(&Player::fn_1_D3A0);
        }
    }
    fn_80086CA8(player->actor, angle);
}

void Player::fn_1_DC6C()
{
}

void Player::fn_1_DC70()
{
    motionIds.clear();
    fn_1_DD14(CHARMOT_HSF_c000m1_300);
    fn_1_DD14(CHARMOT_HSF_c000m1_301);
    fn_1_DD14(CHARMOT_HSF_c000m1_324);
    fn_1_DD14(CHARMOT_HSF_c000m1_320);
    fn_1_DD14(CHARMOT_HSF_c000m1_368);
    fn_1_DD14(CHARMOT_HSF_c000m1_349);
    fn_1_DD14(CHARMOT_HSF_c000m1_351);
    fn_8005B4BC(charNo);
}

void Player::fn_1_DD14(unsigned long dataNum)
{
    motionIds.push_back(fn_8005B0B8(charNo, dataNum));
}

void Player::fn_1_DD88(unsigned long motion, unsigned char loop)
{
    s16 motionId;

    if (motionResource != motion) {
        motionId = motionIds.at(motion);
        fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8(charNo, motionId, 0.0f, 16.0f, loop != 0 ? ((1 << 30) | (1 << 0)) : 0);
        fn_8005C390(charNo, 1.0f);
        motionResource = motion;
    }
}

void Player::fn_1_DE4C(unsigned long motion, unsigned char enabled)
{
    if (effectMotion != motion) {
        effect->setAttribute(0x40000002U);
        effect->shiftMotion(motion, 0.0f, 0.0f, enabled != 0 ? 0x40000001U : 0);
        effectMotion = motion;
    }
}

void Player::fn_1_DEF0(EventMessage *)
{
    setUpdate(&Player::fn_1_D3A4);
}

void Player::fn_1_DF34(EventMessage *)
{
    fn_8006A030((s16)playerNo, 0x2D, 4, 4);
    fn_1_DD88(2, 0);
    setUpdate(&Player::fn_1_D3A8);
}

/* Sets the initial callback and resets the player control attribute. */
void Player::onStart(EventMessage *)
{
    setUpdate(&Player::fn_1_D434);
    fn_80087168(player, (1 << 0));
}

void Player::fn_1_E09C(EventMessage *)
{
    collision.active = 0;
    fn_8008697C(player, &collision.value);
    fn_80087158(player, (1 << 0));
    if (motionEnabled != 0) {
        fn_1_DD88(0, 0);
    }
}

void Player::fn_1_E174(EventMessage *)
{
    if (motionEnabled) {
        setUpdate(&Player::fn_1_DA70);
        fn_1_DD88(1, 1);
    }
}

/* Resets a departing player and selects its idle motion from the current state. */
void Player::fn_1_E268(EventMessage *)
{
    setUpdate(&Player::fn_1_DC6C);
    fn_80086E3C(player);
    if (motionEnabled) {
        MinigameCore::addWinner(playerNo);
        fn_1_DD88(5, 0);
    } else {
        fn_1_DD88(6, 0);
    }
}

void Player::fn_1_E3D8(EventMessage *)
{
    ModelMotion *model;
    short completedMotionId;
    short motionId;
    short motionIdCopy;

    if (motionEnabled) {
        fn_8006A030((s16)playerNo, 0x2D, 4, 4);
        return;
    }
    model = effect;
    motionId = (int)model->modelId;
    motionIdCopy = motionId;
    completedMotionId = motionIdCopy;
    if (fn_8002D75C(completedMotionId) != 0) {
        fn_1_DE4C(0x820007, 0);
    }
}
