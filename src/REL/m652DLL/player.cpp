#include "REL/framework/event.h"
#include "REL/m652DLL/man_map.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"
#include "REL/framework/player_table_1vs1.h"
#include "REL/framework/singleton.h"
#include "REL/m652DLL/player_functions.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include <vector>
#include <cmath>
#include "dolphin/types.h"
extern "C" {
#include "humath.h"
#include "REL/framework/assertion.h"
#include "game/charman.h"
}
#include "REL/m652DLL/player_work.h"
#include <memory>
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/m652DLL/camera_arc_motion.h"
#include "game/hsfex.h"
#include "REL/m652DLL/model_child.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/p_info.h"
#include <math.h>
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "REL/framework/range.h"
extern "C" {
#include "dolphin/os.h"
}
#include "REL/m652DLL/racer_control.h"
#include "datanum/charmot.h"

// The racer's four character motions, in the order the constructor creates them on its
// model (createCharacterMotions).
extern "C" {
extern const unsigned int lbl_1_rodata_320[4] = {0x930022, 0x93003F, 0x930006, 0x930028};
}

/* The players of the two sides. */

struct Racer;

#include <algorithm>

extern "C" {

#include "REL/m652DLL/game_constants.h"
#include "REL/framework/phase_duration_statics.h"
static const int entry_num = 2;
std::vector<Racer *> sm_players;
int lbl_1_bss_B8;
}

extern "C" {
int fn_1_56A8(int value);
s32 fn_1_5A5C(void);
}

/* Returns a random integer below the supplied limit. */
int fn_1_56A8(int value)
{
    return RandomInteger(value);
}

/* Builds the two-player table, shuffles the player order and creates both racers. */
void fn_1_56D0(void)
{
    static Point3d startingPosition = {cmToWorld(0.0f), cmToWorld(405.0f), cmToWorld(100.0f)};
    s32 players[entry_num];
    makePlayerTable1vs1(players);
    std::vector<int> indices(2);
    indices.clear();
    int insertionIndex;
    int index;
    for (insertionIndex = 0; insertionIndex < 2; ++insertionIndex)
        indices.push_back(insertionIndex);
    std::random_shuffle(
        indices.begin(),
        indices.end(),
        fn_1_56A8
    );
    sm_players.clear();
    for (index = 0; index < 2; ++index) {
        int &selectedIndex = indices.at(index);
        s32 *player = players + selectedIndex;
        Racer *object = new Racer(
            *player, selectedIndex,
            &startingPosition
        );
        sm_players.push_back(object);
    }
    fn_8005DDEC(2);
    lbl_1_bss_B8 = 0;
}
/* Starts both racers' initial control callbacks. */
void fn_1_596C(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_6134);
}
/* Sets the finished-play flag on both racers. */
void fn_1_59BC(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_68B4);
}
/* Starts both racers' result motions. */
void fn_1_5A0C(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_6C30);
}
/* Reports whether both racers have the idle callback. */
s32 fn_1_5A5C(void)
{
    s32 index;
    Racer *player;
    assert(sm_players.size() == entry_num);
    index = 0;
    while (index < 2) {
        player = sm_players[index];
        RacerCallback expected = &Racer::fn_1_5B28;
        if (player->callback != expected) {
            return 0;
        }
        index += 1;
    }
    return 1;
}

/* Returns without changing state. */
void Racer::fn_1_5B28() {}

/* Returns the number of racers that have won. */
int Racer::fn_1_5B2C() { return lbl_1_bss_B8; }
// Runs one update on both racers.
void Racer::fn_1_5B3C(RacerCallback update)
{
    int index;
    Racer *item;
    assert(sm_players.size() == entry_num);
    index = 0;
    while (index < 2) {
        item = sm_players[index];
        (item->*update)();
        index += 1;
    }
}

/* Creates the player actor, character motions, registered spheres, controller and item. */
Racer::Racer(int character, int index, Point3d *position)
    : PlayerInfo(character), MotionOwner(characterNo()), GameObject(1),
      callback(&Racer::fn_1_5B28), index(index), playerMask((unsigned short)(1U << index)),
      position(*position), rotation(180.0f),
      firstRegistered(index), secondRegistered(index + 2),
      child(isComputer() ? (RacerControl *)new ComputerControl(this) : new HumanControl(this))
{
    MGACTOR_PARAM actorParam = {0};
    float firstRadius;
    float secondRadius;
    short modelId;
    actorParam.height = height();
    actorParam.radius = 42.0f;
    actorParam.param = character;
    player = fn_80080728((s16) character, &actorParam, 1, (unsigned short) playerMask, 3U, 0);
    fn_80087158(player, (1 << 1));
    fn_80087158(player, (1 << 0));
    fn_800805CC(player->actor, 4U);
    fn_8007ED60(player);
    modelId = (short)player->actor->mdlId;
    fn_80026B8C(modelId, 2);
    createCharacterMotions(&static_cast<MotionOwner &>(*this), lbl_1_rodata_320, 4);
    fn_80086AA8(player->actor, position);
    motion.x = 0.0f;
    motion.y = 0.0f;
    motion.z = 0.0f;
    firstRadius = 350.0f;
    firstRegistered.radius = firstRadius;
    secondRadius = 150.0f;
    secondRegistered.radius = secondRadius;
    firstRegistered = *position;
    secondRegistered = *position;
    fn_80086CA8(player->actor, rotation);
    fn_1_7094(this);
    void(Racer::* callback)() = & Racer::fn_1_5B28;
    this->callback = callback;
    new PlayerItem(this, index);
}

/* Releases the object through its member and base destructors. */
Racer::~Racer() {}

/* Runs the current callback, updates the shadow and synchronizes the actor position. */
void Racer::update()
{
    (this->*callback)();
    fn_1_7124(this);
    fn_1_6CE8(this);
}

/* Enables the player, resets its controller and starts the play callback. */
void Racer::fn_1_6134()
{
    fn_80087168(player, 1);
    child->reset();
    void (Racer::*nextCallback)();
    nextCallback = &Racer::fn_1_61C0;
    callback = nextCallback;
}

/* Updates player control and checks for falls, collisions and a completed finish. */
void Racer::fn_1_61C0()
{
    if (flags.attributeEnabled != 0) {
        fn_1_6A38();
    } else {
        child->update();
        if (fn_1_6258() == 0) {
            fn_1_6300();
            if (fn_1_63E8()) {
                return;
            }
        }
    }
}

extern "C" {
class MapManager;

}

/* Starts the falling callback when the player is below height minus 200. */
unsigned char Racer::fn_1_6258()
{
    if (position.y < -200.0f) {
        fn_1_64F0();
        return 1;
    }
    return 0;
}

/* Returns the actor collision code, or zero if no collision is reported. */
unsigned int Racer::fn_1_62AC()
{
    u32 code = 0;
    if (fn_80086DD0(player->actor, &code)) return code;
    return 0;
}

/* Shows the selected course model when collision bit 0x4 is set. */
void Racer::fn_1_6300()
{
    if (this->flags.collision4 == 0) {
        u32 collisionCode = 0;
        unsigned int code;
        if (fn_80086DD0(this->player->actor, &collisionCode) != 0) {
            code = collisionCode;
        } else {
            code = 0;
        }
        if ((code & 0x4u) != 0u) {
            fn_1_524C(&MinigameSingleton<MapManager>::getChecked(), this->index);
            this->flags.collision4 = 1;
        }
    }
}

/* Records a winner and starts the finish callback when collision bit 0x2 is set. */
unsigned char Racer::fn_1_63E8()
{
    u32 collisionCode = 0;
    unsigned int code;
    int object;
    if (fn_80086DD0(this->player->actor, &collisionCode) != 0) {
        code = collisionCode;
    } else {
        code = 0;
    }
    if ((code & 0x2u) == 0) {
        return 0;
    }
    if (lbl_1_bss_B8 < 1) {
        object = this->playerNo;
        MinigameCore::addWinner(object);
        if (!isComputer()) {
            sendEvent(0, 1, 0.0f);
        }
        this->flags.hasWon = 1;
        lbl_1_bss_B8++;
        this->fn_1_68F8();
    } else {
        this->fn_1_6A38();
    }
    return 1;
}

/* Stops horizontal motion, changes the actor state and starts the falling motion. */
void Racer::fn_1_64F0()
{
    int mode;
    int actorState;
    void (Racer::*values)();
    this->motion.x = 0.0f;
    this->flags.stateFlag = 1;
    fn_80087158(this->player, 1);
    fn_80086E3C(this->player);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), CHARMOT_HSF_c000m1_344,
              (float)secondsToFrames(0.3), 0.0f,
              ((1 << 30) | (1 << 0)));
    if (this->index == 0) { mode = 0x80F; }
    else { mode = 0x810; }
    actorState = mode;
    soundPlayAt((short)this->playerMask, actorState, &this->position);
    values = &Racer::fn_1_65FC;
    this->callback = values;
}

/* Applies falling gravity and sends the player event after passing height minus 2000. */
void Racer::fn_1_65FC()
{
    void (Racer::*nextCallback)();

    motion.y -= gravity;
    if (this->position.y > -2000.0f) {
        return;
    }
    motion.x = 0.0f;
    motion.y = 0.0f;
    motion.z = 0.0f;
    fn_1_7370(this, 0);
    rumble(2, 0x18);
    sendEvent(index + 2, 0, 1.0f);
    nextCallback = &Racer::fn_1_5B28;
    this->callback = nextCallback;
}

/* Checks collisions and falls while waiting to restart player control. */
void Racer::fn_1_66F4()
{
    if (fn_1_6FAC(this) != 0) {
        if (flags.attributeEnabled != 0) {
            fn_1_6A38();
            return;
        }
        fn_1_6134();
        return;
    }
    fn_1_6258();
}

/* Enables the player and sets the finished-play flag. */
void Racer::fn_1_68B4() {
    fn_80087158(player, 1);
    flags.attributeEnabled = 1;
}

/* Changes the actor motion state and starts the finish approach callback. */
void Racer::fn_1_68F8()
{
    void (Racer::*values)();
    fn_80086CBC(player->actor, 1.0f);
    fn_8007E818(player);
    values = &Racer::fn_1_6978;
    callback = values;
}

/* Moves toward the finish target and sends the camera event on arrival. */
void Racer::fn_1_6978()
{
    HuVecF target = {0.0f, 0.0f, 0.0f};
    target.y = 405.0f;
    target.z = -5050.0f;
    if (fn_8007E8B4(player, &target, (float)framesToSeconds(200.0), 30.0f) != 0) {
        sendEvent(index + 1, 2, 0.0f);
        fn_1_6A38();
    }
}

/* Enables the player and starts the camera-facing callback. */
void Racer::fn_1_6A38()
{
    void (Racer::*copy)();
    fn_80087158(player, 1);
    copy = &Racer::fn_1_6AA4;
    callback = copy;
}

/* Rotates the player toward the camera target and waits for the turn to complete. */
void Racer::fn_1_6AA4()
{
    Vec *actorPosition;
    Vec cameraPosition;
    Vec cameraTarget;
    float rotation;
    float targetAngle;
    float difference;
    float absoluteDifference;
    float error;

    fn_80027CD0(playerMask, &cameraPosition, &cameraTarget,
                     &cameraTarget);
    actorPosition = &player->actor->pos;
    targetAngle = (float)(180.0 *
                          (atan2((double)(cameraPosition.x - actorPosition->x),
                                 (double)(cameraPosition.z - actorPosition->z)) /
                           M_PI));
    fn_80086CB0(player->actor, &rotation);
    Range<float> range(0.0f, 360.0f, rotation);
    fn_80086CA8(player->actor,
                   range.step(targetAngle, 0.2));
    difference = range.distanceTo(targetAngle);
    absoluteDifference = __fabsf(difference);
    error = absoluteDifference;
    if (error < 1.0) {
        RacerCallback defaults = &Racer::fn_1_5B28;
        callback = defaults;
    }
}

/* Selects the winner or loser character motion and starts the idle callback. */
void Racer::fn_1_6C30()
{
    unsigned int motion;
    if (flags.hasWon) motion = CHARMOT_HSF_c000m1_306;
    else motion = CHARMOT_HSF_c000m1_351;
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), motion, 16.0f, 0.0f, 0);
    void (Racer::*action)() = &Racer::fn_1_5B28;
    callback = action;
}

extern "C" {

static const Vec rotationOrigin = {0.0f, 0.0f, 0.0f};

}

/* Synchronizes the player actor and model position while applying movement limits. */
void fn_1_6CE8(Racer *object)
{
    float clampedX;
    float positionX;
    float clampedY;
    float positionY;
    float clampedZ;
    float positionZ;
    int modelId = (short)object->player->actor->mdlId;

    if (object->flags.stateFlag == 0U) {
        Vec previousPosition = object->position;
        fn_80086BC8(object->player->actor, &object->position);
        PSVECSubtract(&object->position, &previousPosition, &object->motion);
        fn_80086CB0(object->player->actor, &object->rotation);
    } else {
        PSVECAdd(&object->motion, &object->position, &object->position);
        fn_80026790((short)modelId, &object->position);
        fn_80086AA8(object->player->actor, &object->position);
        Vec rotation = rotationOrigin;
        rotation.y = object->rotation;
        fn_80026838((short)modelId, &rotation);
        fn_80086CA8(object->player->actor, object->rotation);
    }

    positionX = object->position.x;
    if (positionX < -550.0f) {
        clampedX = -550.0f;
    } else if (positionX > 550.0f) {
        clampedX = 550.0f;
    } else {
        clampedX = positionX;
    }
    object->firstRegistered.center.x = clampedX;
    positionY = object->position.y + cmToWorld(100.0f);
    if (positionY < -500.0f) {
        clampedY = -500.0f;
    } else if (positionY > 500.0f) {
        clampedY = 500.0f;
    } else {
        clampedY = positionY;
    }
    object->firstRegistered.center.y = clampedY;
    positionZ = object->position.z;
    if (positionZ < -5600.0f) {
        clampedZ = -5600.0f;
    } else if (positionZ > -120.00001f) {
        clampedZ = -120.00001f;
    } else {
        clampedZ = positionZ;
    }
    object->firstRegistered.center.z = clampedZ;
    object->secondRegistered = object->position;
    float verticalSpeed = object->secondRegistered.radius;
    object->secondRegistered.center.y += verticalSpeed;
    soundPanCharacterVoice(object->playerMask, (short)object->characterNo(),
              &object->position);
}

/* Tests actor collision bits 0x407F unless the player state disables collisions. */
u8 fn_1_6FAC(Racer *self) {
    u32 colCode;

    if (self->flags.stateFlag) {
        return 0U;
    }
    colCode = 0;
    fn_80086DD0(self->player->actor, &colCode);
    return (u8) ((colCode & 0x407F) != 0);
}

/* Tests actor collision bits 0xBF80 unless the player state disables collisions. */
u8 fn_1_7020(Racer *playerState) {
    u32 collisionCode;

    if (playerState->flags.stateFlag) {
        return 0U;
    }
    collisionCode = 0;
    fn_80086DD0(playerState->player->actor, &collisionCode);
    return (u8) ((collisionCode & 0xBF80) != 0);
}

/* Enables the player model's shadow state. */
void fn_1_7094(Racer *actor)
{
    short shadowModelId = (short)actor->player->actor->mdlId;
    fn_80026E44(shadowModelId);
}

/* Resets the player model's shadow state. */
void fn_1_70DC(Racer *actor)
{
    short shadowModelId = (short)actor->player->actor->mdlId;
    fn_80026EF0(shadowModelId);
}

/* Samples three nearby ground rays to decide the player model's shadow state. */
void fn_1_7124(Racer *actor)
{
    MGACTOR_COLMAP_POLY polygon;
    HuVecF point;
    HuVecF groundPoint;
    short shadowModelId;
    short hitModelId;
    short resetModelId;
    float radius;
    float baseAngle;
    float angle;
    HuVecF *pointPointer;
    HuVecF *sourcePointer;
    int index;

    if (fn_1_6FAC(actor) != 0) {
        shadowModelId = actor->player->actor->mdlId;
        fn_80026E44(shadowModelId);
        return;
    }

    radius = 100.0f;
    baseAngle = RandomFloat(6.2831855f);
    for (index = 0; index < 3; index++) {
        double sineOffset;
        double cosineOffset;

        angle = baseAngle + ((M_PI * 360.0 / 180.0) * (double)index) /
                3.0;
        cosineOffset = radius * std::cos(angle);
        sineOffset = radius * std::sin(angle);
        point.x = sineOffset;
        point.y = 10.0f;
        point.z = cosineOffset;
        pointPointer = &point;
        sourcePointer = pointPointer;
        groundPoint.x = sourcePointer->x;
        groundPoint.y = sourcePointer->y;
        groundPoint.z = sourcePointer->z;
        PSVECAdd(&actor->position, &point, &point);
        PSVECAdd(&actor->position, &groundPoint, &groundPoint);
        groundPoint.y = -1000.0f;
        if (fn_80086910(&point, &groundPoint, 1, &polygon) != 0) {
            hitModelId = actor->player->actor->mdlId;
            fn_80026E44(hitModelId);
            return;
        }
    }

    resetModelId = actor->player->actor->mdlId;
    fn_80026EF0(resetModelId);
}

/* Enables or disables the character effect and its associated flag. */
void fn_1_7370(Racer *self, unsigned char enabled) {
    int character = self->characterNo();
    if (enabled) {
        fn_8005C47C(character, 1);
    } else {
        fn_8005C430(character, 1);
    }
    fn_8005DFC0(character, enabled);
}

/* Resets rotation, starts the character motion and installs the idle callback. */
void fn_1_73E4(Racer *work)
{
    int character;
    RacerCallback copy;

    work->rotation = 0.0f;
    shiftCharacterMotion(&static_cast<MotionOwner &>(*work), CHARMOT_HSF_c000m1_376,
                         0.0f,
              0.0f, ((1 << 30) | (1 << 0)));
    character = work->characterNo();
    fn_8005C47C((short)character, 1);
    fn_8005DFC0((short)character, 1);
    copy = &Racer::fn_1_74B0;
    work->callback = copy;
}

/* Returns without changing state. */
void Racer::fn_1_74B0() {}

/* Places the player relative to its character hook and the supplied item position. */
void fn_1_74B4(Racer *work, const Vec *offset)
{
    int character = (short)work->player->actor->mdlId;
    char *hook = fn_8005C0F0((short)work->characterNo(), 1, 4);
    Vec objectPosition;
    Vec *positionForAddition;
    Vec *position;

    Hu3DModelObjPosGet(character, hook, &objectPosition);
    PSVECSubtract(&work->position, &objectPosition, &objectPosition);
    position = &objectPosition;
    positionForAddition = position;
    PSVECAdd(offset, positionForAddition, &work->position);
}

/* Resets actor position and motion state before restarting collision checks. */
void fn_1_754C(Racer *work)
{
    void (Racer::*values)();
    work->flags.stateFlag = 0;
    fn_80086F00(work->player, &work->position);
    fn_8008697C(work->player, &work->position);
    fn_80086CC4(work->player->actor, 0.0f);
    values = &Racer::fn_1_66F4;
    work->callback = values;
}
