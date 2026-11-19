#include "REL/framework/world_units.h"
#include <new>
#include "REL/framework/p_info.h"
#include "REL/framework/list_object.h"
#include "REL/framework/frame_time.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/framework/event_hm.h"
#include <math.h>
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/tri_colli.h"
#include "REL/m605DLL/player_states.h"
#include "dolphin/mtx.h"

extern "C" {
#include "dolphin/types.h"
#include "game/data.h"
}
#include "dolphin/os.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/game_object.h"
#include "REL/framework/registered_value.h"
#include "REL/m605DLL/player_character.h"
#include "REL/m605DLL/owner_callback.h"
#include "REL/m605DLL/scene_controller.h"
#include "REL/framework/p_chara.h"
#include "REL/framework/misc_diff_deg.h"
#include "REL/framework/minigame_core.h"

extern "C" {
#include "types.h"
}
#include "REL/framework/sphere_type.h"
#include "humath.h"
#include "dolphin/pad.h"
#include "REL/m605DLL/value_bounds.h"
#include <functional>
#include <vector>
#include <algorithm>
#include "REL/framework/frame_durations.h"
#include "REL/m605DLL/motion_controller.h"
#include "REL/m605DLL/scene_stage.h"
#include "REL/m605DLL/falling_rock.h"

// Returns the signed distance from a point to a plane.
static inline float signedDistance(const Plane &plane, const Vector3 &point)
{
    return plane.distance + plane.normal.dot(point);
}

// Places the indexed point at its evenly spaced horizontal position.
void fn_1_7200(Vec *output, s32 index)
{
    static const f32 scale = 1.2f * 100.0f;
    output->x = fn_1_9CC8(index, 4, scale);
    output->y = 0.0f;
    output->z = 0.0f;
}

extern const unsigned int lbl_1_rodata_340[11];

extern void fn_1_7F58();

extern void fn_1_835C(PlayerCharacter *player);

const unsigned int lbl_1_rodata_340[11] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 2),
    DATANUM(0x930000, 3), DATANUM(0x930000, 4), DATANUM(0x930000, 6),
    DATANUM(0x930000, 17), DATANUM(0x930000, 18), DATANUM(0x930000, 19),
    DATANUM(0x930000, 23), DATANUM(0x930000, 24)
};

// Creates the player object, collision state, motions and event handlers.
PlayerCharacter::PlayerCharacter(
    PlayerConfigSelection *playerConfig, TriangleCollision *collisionObject, s32 option)
  : GameObject(0), unk10(option), projectedPosition(0), channel1Events(1), channel0Events(0),
    currentState(0), playerConfig(playerConfig), collisionObject(collisionObject), steering(0)
{
    setPostUpdateHook(getCategory(), fn_1_7F58);
    f32 radius = 0.6f * 100.0f;
    this->body.setRadius(radius);
    f32 projectedRadius = 0.5f * getResource()->height();
    this->projectedPosition.setRadius(projectedRadius);
    this->motionDataNum = -1;
    createCharacterMotions(&static_cast<Character &>(*getResource()), lbl_1_rodata_340, 11);
    s32 playerIndex = getResource()->number();
    fn_1_7200(&body.center, playerIndex);
    this->previousPosition.x = this->body.center.x;
    this->previousPosition.y = this->body.center.y;
    this->previousPosition.z = this->body.center.z;
    fn_1_835C(this);
    this->rayHit = 1;
    this->contactFacesUp = 1;
    this->jumpRequested = 0;
    this->belowFloor = 0;
    this->atFinish = 0;
    this->angle = this->targetAngle = 0.0f;
    this->unk14 = 1;
    this->movementInput.x = 0.0f;
    this->movementInput.y = 0.0f;
    this->movementInput.z = 0.0f;
    this->acceleration.x = 0.0f;
    this->acceleration.y = 0.0f;
    this->acceleration.z = 0.0f;
    this->velocity.x = 0.0f;
    this->velocity.y = 0.0f;
    this->velocity.z = 0.0f;

    fn_1_7A68(this, new IdleState);

    registerEventCallback(&channel1Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8F10), this), 0);
    registerEventCallback(&channel1Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8EB8), this), 0);
    registerEventCallback(&channel1Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8EB8), this), 3);
    registerEventCallback(&channel0Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8BDC), this), 2);
    registerEventCallback(&channel0Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8CE4), this), 3);
    registerEventCallback(&channel0Events,
        std::bind1st(std::mem_fun(&PlayerCharacter::fn_1_8DEC), this), 4);
}

// Notifies and replaces the callback, then enters the replacement state.
static inline void replaceOwnerCallback(PlayerCharacter *owner, OwnerCallback *replacement)
{
    OwnerCallback *enteringState;

    if (owner->currentState.get() != 0) {
        OwnerCallback *leavingState = owner->currentState.get();
        leavingState->leave(owner);
    }

    owner->currentState.reset(replacement);

    enteringState = owner->currentState.get();
    enteringState->enter(owner);
}

// Notifies and replaces the callback, then enters the replacement state.
void fn_1_7A68(PlayerCharacter *player, OwnerCallback *replacement)
{
    replaceOwnerCallback(player, replacement);
}

typedef struct PlayerCharacterCallbackView PlayerCharacterCallbackView;

struct PlayerCharacterCollisionView;

void fn_1_822C(PlayerCharacter *player);

extern void fn_1_84DC(struct PlayerCharacterCollisionView *player, bool mode);

typedef struct ObjectCallbackOwnerFlags {
    u8 : 2;
    u8 belowFloor:1;
} ObjectCallbackOwnerFlags;

// Exposes the PlayerCharacter fields used by the callback update code.
struct PlayerCharacterCallbackView {
    u8 unk00[0x14]; /* No code names this field. */
    ObjectCallbackOwnerFlags flags14;
    f32 targetX;
    f32 flagStartTime;
    f32 angle;
    f32 targetAngle;
    Sphere body;
    u8 unk38[0xC]; /* No code names this field. */
    RegisteredValue projectedPosition;
    u8 unk60[0x1C]; /* No code names this field. */
    Vec velocity;
    Vec acceleration;
    EventHandlerManager channel1Events;
    EventHandlerManager channel0Events;
    s32 motionDataNum;
    OwnerCallback *currentState;
    // Returns the current character state.
    OwnerCallback *getCurrentState() const { return currentState; }
    PlayerConfigSelection *playerConfig;
    // Returns the player character and controller information.
    PlayerConfigSelection *getResource() const { return playerConfig; }
    // Tests whether the body, raised by the offset, lies below the course floor.
    bool isBelowFloor(f64 heightOffset)
    {
        return fn_1_49D0(body.center.y + heightOffset, body.center.z);
    }
};

void fn_1_8A28(PlayerCharacter *player);

// Enters the hit state after contact with the rock.
inline void PlayerCharacter::enterHitState()
{
    OwnerCallback *const replacement = new HitState(STATE_HIT);
    replaceOwnerCallback(this, replacement);
}

// Advances the callback and motion, resolves collisions and detects the fall.
void PlayerCharacter::update()
{
    f32 flagDuration;
    u8 wasFlagged;
    f32 offsetFlagClearBefore;
    f32 offsetFlagClearAfter;
    f32 offsetFlagSetBefore;
    f32 offsetFlagSetAfter;

    if (reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor != 0) {
        flagDuration =
            lbl_1_bss_3C - reinterpret_cast<PlayerCharacterCallbackView *>(this)->flagStartTime;
        if (flagDuration > 1.5f) {
            return;
        }
    }
    fn_1_822C(this);
    reinterpret_cast<PlayerCharacterCallbackView *>(this)->getCurrentState()->updatePlayer(this);
    switch (reinterpret_cast<PlayerCharacterCallbackView *>(this)->getCurrentState()->getState())
    {
    case STATE_RUN:
    case STATE_WALK:
        if ((reinterpret_cast<PlayerCharacterCallbackView *>(this)->velocity.x != 0.0f ||
             reinterpret_cast<PlayerCharacterCallbackView *>(this)->velocity.z != 0.0f) &&
            reinterpret_cast<PlayerCharacterCallbackView *>(this)->velocity.z > 0.0f)
        {
            reinterpret_cast<PlayerCharacterCallbackView *>(this)->targetAngle =
                (f32) (180.0 *
                       (atan2(
                          (f64)reinterpret_cast<PlayerCharacterCallbackView *>(this)->velocity.x,
                          (f64)reinterpret_cast<PlayerCharacterCallbackView *>(this)->velocity.z) /
                        M_PI));
        }
        break;
    }
    reinterpret_cast<PlayerCharacterCallbackView *>(this)->acceleration.y += -980.665 / 1200.0;
    fn_1_8A28(this);
    fn_1_84DC(reinterpret_cast<PlayerCharacterCollisionView *>(
                 reinterpret_cast<PlayerCharacterCallbackView *>(this)),
             (0 != reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor)
                 ? false : true);
    wasFlagged = reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor;
    if (reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor == 0)
    {
        offsetFlagClearBefore = reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.radius;
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.center.y +=
            offsetFlagClearBefore;
        if (lbl_1_bss_30 != 0)
        {
            ObjectCallbackOwnerFlags *flags =
                &reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14;
            flags->belowFloor = (u8)(fn_1_1D60(lbl_1_bss_30, &body) | flags->belowFloor);
        }
        offsetFlagClearAfter = reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.radius;
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.center.y -=
            offsetFlagClearAfter;
    }
    else
    {
        offsetFlagSetBefore = reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.radius;
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.center.y += offsetFlagSetBefore;
        fn_1_1D60(lbl_1_bss_30, &body);
        offsetFlagSetAfter = reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.radius;
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->body.center.y -= offsetFlagSetAfter;
    }
    if (reinterpret_cast<PlayerCharacterCallbackView *>(this)->isBelowFloor(20.0))
    {
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->getResource()->resetShadow();
    }
    else
    {
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->getResource()->setShadow();
    }
    if (reinterpret_cast<PlayerCharacterCallbackView *>(this)->isBelowFloor(80.0))
    {
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->projectedPosition.clearFlag7();
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor = 1;
    }
    if (wasFlagged == 0 &&
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->flags14.belowFloor != 0)
    {
        reinterpret_cast<PlayerCharacterCallbackView *>(this)->flagStartTime = lbl_1_bss_3C;
        fn_1_2CB4();
        enterHitState();
    }
}

typedef struct PlayerCharacterSceneView PlayerCharacterSceneView;

// Exposes the PlayerCharacter fields used by ray-plane tests.
struct PlayerCharacterSceneView {
    u8 unk00[0x14]; /* No code names this field. */
    u8 rayHit : 1;
    u8 unk15[0x13]; /* No code names this field. */
    struct {
        Vector3 center;
        f32 radius;
    } body;
    Point3d previousPosition;
    u8 unk44[0x1C]; /* No code names this field. */
    Plane contactPlane;
    u8 unk70[0x40]; /* No code names this field. */
    TriangleCollision *collisionObject;
};

template<class T, class Callback> void fn_1_A2EC(std::vector<T> *list, Callback callback);

extern void fn_1_8058(PlayerCharacterSceneView *player);

extern void fn_1_93B4();

// Updates each object in the collected list and releases its temporary array.
void fn_1_7F58()
{
    std::vector<GameObject *> objects;
    collectGameObjects(0, &objects);
    fn_1_A2EC(&objects, std::ptr_fun(fn_1_999C));
    s32 loopLimit = objects.size();
    s32 index = 0;
    while (index < loopLimit) {
        PlayerCharacterSceneView *object =
            reinterpret_cast<PlayerCharacterSceneView *>(objects[index]);
        fn_1_84DC(reinterpret_cast<PlayerCharacterCollisionView *>(object), 0);
        fn_1_835C(reinterpret_cast<PlayerCharacter *>(object));
        fn_1_8058(object);
        object->previousPosition.x = object->body.center.x;
        object->previousPosition.y = object->body.center.y;
        object->previousPosition.z = object->body.center.z;
        index++;
    }
    fn_1_93B4();
}

// Selects the ray-hit plane or the stage floor below the object.
void fn_1_8058(PlayerCharacterSceneView *player)
{
    Point3d rayTarget;
    TriangleCollision * collision;
    Point3d * floorPoint;
    f32 lowerRayHeight;
    collision = player->collisionObject;
    TriangleCollision * rayCollision = collision;
    Vector3 rayOrigin = player->body.center;
    Vector3 lowerRayTarget = player->body.center;
    f32 upperHeight = player->body.center.y + player->body.radius;
    Point3d * floorNormal;
    rayOrigin.y = upperHeight;
    if ((u8) fn_1_4AA4(player->body.center.z) != 0)
    {
        f32 lowerHeight = player->body.center.y - player->body.radius;
        lowerRayTarget.y = lowerHeight;
    }
    if (rayCollision->castRay(&rayOrigin, &lowerRayTarget)) {
        f32 rayFraction = rayCollision->maximumDistance;
        if (rayFraction < 1.05f)
            goto havePlane;
    }
    {
        player->rayHit = 0;
        lowerRayHeight = fn_1_4A0C(player->body.center.z);
        Point3d normal;
        Point3d point;
        point.x = 0.0f;
        point.y = lowerRayHeight;
        point.z = 0.0f;
        floorPoint = &point;
        normal.x = 0.0f;
        normal.y = 1.0f;
        normal.z = 0.0f;
        floorNormal = &normal;
        player->contactPlane.setFromNormalAndPoint(floorNormal, floorPoint);
        return;
    }
havePlane:
    player->rayHit = 1;
    TriangleCollision * hitCollision = rayCollision;
    TriangleCollision * planeCollision = hitCollision;
    player->contactPlane.normal.x = planeCollision->faceNormal.x;
    player->contactPlane.normal.y = planeCollision->faceNormal.y;
    player->contactPlane.normal.z = planeCollision->faceNormal.z;
    player->contactPlane.distance = planeCollision->planeDistance;
}

// Reads movement input from the controller or computer player.
void fn_1_822C(PlayerCharacter *player)
{
    f64 horizontalInput;
    f64 depthInput;
    u32 stateFlags;
    PlayerInfo *predicateState;
    PlayerInfo *stateFlagsInput;
    PlayerInfo *horizontalInputConfig;
    PlayerInfo *depthInputConfig;

    player->movementInput.x = 0.0f;
    player->movementInput.y = 0.0f;
    player->movementInput.z = 0.0f;
    player->jumpRequested = 0;
    if (currentRequest() == 2) {
        predicateState = player->playerConfig.pointer;
        if (predicateState->isComputer()) {
            fn_1_B2AC(player);
            return;
        }
        stateFlagsInput = player->playerConfig.pointer;
        stateFlags = stateFlagsInput->buttonsDown();
        horizontalInputConfig = player->playerConfig.pointer;
        horizontalInput = horizontalInputConfig->stickX();
        depthInputConfig = player->playerConfig.pointer;
        depthInput = depthInputConfig->stickY();
        player->movementInput.x = static_cast<f32>(horizontalInput);
        player->movementInput.y = 0.0f;
        player->movementInput.z = -depthInput;
        player->jumpRequested = (stateFlags & PAD_BUTTON_A) != 0;
    }
}

// Projects the registered position and updates the character collision.
void fn_1_835C(PlayerCharacter *player)
{
    Vec correction;
    f32 correctionScale;
    Vec *position = &player->body.center;
    PlayerConfigSelection *target;
    PlayerConfigSelection *valueTarget;

    player->projectedPosition = *position;
    f32 projection = PSVECDotProduct(&player->contactPlane.normal, position);
    f32 distance = player->contactPlane.distance + projection;
    if (distance < 0.0f) {
        f32 registeredProjection =
            PSVECDotProduct(&player->contactPlane.normal, &player->projectedPosition.center);
        f32 correctionDistance = player->contactPlane.distance + registeredProjection;
        correctionScale = correctionDistance;

        PSVECScale(&player->contactPlane.normal, &correction,
                   -correctionScale);
        PSVECAdd(&player->projectedPosition.center, &correction, &player->projectedPosition.center);
    }

    player->projectedPosition.center.z += 400.0f;
    f32 verticalOffset = player->projectedPosition.radius;
    player->projectedPosition.center.y = player->projectedPosition.center.y + verticalOffset;
    target = player->playerConfig.pointer;
    target->setPosition(&player->body.center);
    valueTarget = player->playerConfig.pointer;
    valueTarget->setRotationY(player->angle +=
        limitedDegreeDifference(player->targetAngle, player->angle, (82.0f / 6.0f)));
}

typedef std::vector<PlaneRecord> CollisionList;

typedef struct CollisionObjectFlags {
    u8 : 1;
    u8 contactFacesUp:1;
    u8 belowFloor:1;
} CollisionObjectFlags;

// Exposes the PlayerCharacter fields used by swept-sphere collision resolution.
typedef struct PlayerCharacterCollisionView {
    u8 unk00[0x14]; /* No code names this field. */
    CollisionObjectFlags flags14;
    f32 targetX;
    f32 flagStartTime;
    f32 angle;
    f32 targetAngle;
    struct {
        Vector3 center;
        f32 radius;
    } body;
    Vector3 previousPosition;
    RegisteredValue projectedPosition;
    Plane contactPlane;
    Vec movementInput;
    Vector3 velocity;
    f32 accelerationX;
    u8 unk8C[0x24]; /* No code names this field. */
    TriangleCollision *collisionObject;
    TriangleCollision *GetCollisionObject() { return collisionObject; }
} PlayerCharacterCollisionView;

// Constrains the object to the stage and resolves swept-sphere contacts.
void fn_1_84DC(PlayerCharacterCollisionView *player, bool mode)
{
    PlaneRecord *plane;

    f32 depth;
    f32 clampedX;
    f32 positionX;
    f32 correctionDepth;
    s32 index;

    if (player->flags14.belowFloor == 0 && player->body.center.z < 46000.0f) {
        player->body.center.z = maximumValue(player->body.center.z, lbl_1_bss_34 - 100.0f);
        player->body.center.z =
            minimumValue(player->body.center.z, lbl_1_bss_34 + cmToWorld(1200.0f));
    }
    depth = (player->body.center.z - 35700.0f) / 1200.0f;
    if (depth > 0.8f && depth < 1.8f) {
        player->body.center.y =
            maximumValue(player->body.center.y, fn_1_4A0C(player->body.center.z));
    }
    if (depth >= 0.0f) {
        positionX = player->body.center.x;
        if (positionX < -467.0f) {
            clampedX = -467.0f;
        } else if (positionX > 467.0f) {
            clampedX = 467.0f;
        } else {
            clampedX = positionX;
        }
        player->body.center.x = clampedX;
    }

    TriangleCollision *collision = player->GetCollisionObject();
    CollisionList collisions;
    f32 radius = 40.0f;
    player->body.center.y += radius;
    player->previousPosition.y += radius;
    collision->sweepSphere(&player->previousPosition, &player->body.center, radius,
               &collisions);
    player->flags14.contactFacesUp = 0;

    index = 0;
    s32 count = collisions.size();
    while (index < count) {
        plane = &collisions[index];
        if (plane->normal.y > 0.2f) {
            player->flags14.contactFacesUp = 1;
        }

        if (!(signedDistance(*plane, player->body.center) >= 0.0f)) {
            Vector3 originalPosition = player->body.center;
            Vector3 correction;
            correctionDepth = signedDistance(*plane, player->body.center);
            PSVECScale(&plane->normal, &correction, -correctionDepth);
            PSVECAdd(&player->body.center, &correction, &player->body.center);
            if (mode != 0) {
                PSVECSubtract(&player->body.center, &originalPosition, &originalPosition);
                PSVECAdd(&originalPosition, &player->velocity,
                         &player->velocity);
            }
        }
        index++;
    }

    if (player->flags14.contactFacesUp != 0) {
        player->velocity.y = 0.0f;
    }
    player->body.center.y -= radius;
    player->previousPosition.y -= radius;

}

// Integrates acceleration and velocity with the average velocity step.
void fn_1_8A28(PlayerCharacter *player)
{
    Point3d acceleration = player->acceleration;
    player->acceleration.x = 0.0f;
    player->acceleration.y = 0.0f;
    player->acceleration.z = 0.0f;

    Point3d velocity = player->velocity;
    Point3d halfVelocity;
    PSVECAdd(&acceleration, &player->velocity, &player->velocity);
    if (player->velocity.y > 26.0f) {
        player->velocity.y = 26.0f;
    }
    PSVECAdd(&velocity, &player->velocity, &halfVelocity);
    PSVECScale(&halfVelocity, &halfVelocity, 0.5f);
    PSVECAdd(&player->body.center, &halfVelocity, &player->body.center);
}

// Starts the requested character motion when it needs changing.
void fn_1_8B20(PlayerCharacter *player, u32 value, u8 enabled)
{
    if (player->motionDataNum == value) {
        if (characterMotionEnded(&player->getResource()->charNo) == 0)
            return;
    }
    player->motionDataNum = value;
    player->getResource()->setAttribute(0x40000002);
    shiftCharacterMotion(&static_cast<Character &>(*player->getResource()), value, 16.0f, 0.0f,
               enabled != 0U ? 0x40000001 : 0);
}

// Replaces the active callback with AlternateIdleState.
void PlayerCharacter::fn_1_8BDC(const EventPacket &)
{
    OwnerCallback *replacement = new AlternateIdleState;
    replaceOwnerCallback(this, replacement);
}

// Enters the run state and clears the target angle.
void PlayerCharacter::fn_1_8CE4(const EventPacket &)
{
    OwnerCallback *replacement = new RunState;
    replaceOwnerCallback(this, replacement);
    this->targetAngle = 0.0f;
}

// Starts motion 0x930006 while the player is above the floor.
void PlayerCharacter::fn_1_8DEC(const EventPacket &)
{
    if (belowFloor == 0) {
        fn_1_8B20(this, DATANUM(0x930000, 6), 0);
    }
}

// Rumbles the player controller while the player is above the floor.
void PlayerCharacter::fn_1_8EB8(const EventPacket &)
{
    if ((u8)belowFloor == 0U) {
        PlayerConfigSelection *configuration = playerConfig.pointer;
        configuration->rumble(2, 24);
    }
}

// Replaces the active callback with ReverseIdleState.
void PlayerCharacter::fn_1_8F10(const EventPacket &)
{
    OwnerCallback *replacement = new ReverseIdleState;
    replaceOwnerCallback(this, replacement);
}

// Tests whether every player has been below the floor for more than half a second.
bool fn_1_9018()
{
    std::vector<GameObject *> owners;
    collectGameObjects(0, &owners);
    s32 accepted = std::count_if(owners.begin(), owners.end(), std::ptr_fun(fn_1_959C));
    return accepted == owners.size();
}

// Tests whether every player still above the floor has reached the finish.
bool fn_1_9120()
{
    std::vector<GameObject *> owners;
    collectGameObjects(0, &owners);
    s32 activeCount = std::count_if(owners.begin(), owners.end(), std::ptr_fun(fn_1_960C));
    s32 configuredCount = std::count_if(owners.begin(), owners.end(), std::ptr_fun(fn_1_959C));
    s32 remaining = owners.size() - configuredCount;
    return remaining != 0 && remaining == activeCount;
}

// Registers each active owner as a minigame winner.
void fn_1_92EC()
{
    std::vector<GameObject *> owners;
    collectGameObjects(0, &owners);
    u32 index = 0;
    while (index < owners.size()) {
        GameObject *owner = owners[index];
        if (fn_1_960C(owner) != 0) {
            MinigameCore::addWinner(static_cast<PlayerCharacter *>(owner)->getResource()->number());
        }
        index++;
    }
}

// Sorts players who have stayed above the floor and assigns their horizontal targets.
void fn_1_93B4()
{
    std::vector<GameObject *> owners;
    std::vector<GameObject *> orderedOwners;
    collectGameObjects(0, &owners);

    u32 index = 0;
    while (index < owners.size()) {
        GameObject *owner = owners[index];
        if (static_cast<PlayerCharacter *>(owner)->belowFloor == 0) {
            orderedOwners.push_back(owners[index]);
        }
        index++;
    }

    std::sort(orderedOwners.begin(), orderedOwners.end(),
              std::ptr_fun(fn_1_9630));
    s32 orderedCount = orderedOwners.size();

    s32 orderIndex = 0;
    while (orderIndex < orderedCount) {
        PlayerCharacter *owner = static_cast<PlayerCharacter *>(orderedOwners[orderIndex]);
        owner->targetX =
            fn_1_9CC8(orderIndex, orderedCount, 160.0f);
        orderIndex++;
    }
}

// Tests whether the player has been below the floor for more than half a second.
bool fn_1_959C(GameObject *object)
{
    PlayerCharacter *owner = static_cast<PlayerCharacter *>(object);
    if (!owner->belowFloor) return false;
    f32 remaining = lbl_1_bss_3C - owner->flagStartTime;
    return remaining > 0.5f;
}

// Returns whether the player has reached the finish.
unsigned char fn_1_960C(const GameObject *state)
{
    const PlayerCharacter *owner = static_cast<const PlayerCharacter *>(state);
    return owner->atFinish;
}

// Compares the horizontal positions of two objects.
bool fn_1_9630(const GameObject *left, const GameObject *right) {
    const PlayerCharacter *leftOwner = static_cast<const PlayerCharacter *>(left);
    const PlayerCharacter *rightOwner = static_cast<const PlayerCharacter *>(right);
    return leftOwner->body.center.x < rightOwner->body.center.x;
}

// Identifies the point that sphere separation adjusts.
class SphereVector {
public:
    Point3d vector;
    Point3d *getVector() { return &vector; }
    // Returns the point used for the adjustment.
    operator Point3d *() { return getVector(); }
};

// Separates overlapping spheres horizontally. No code calls this function.
s32 fn_1_9668(Sphere *first, Sphere *second, f32 firstWeight, f32 separationScale,
              Point3d *direction)
{
    Point3d difference;
    SphereVector adjustment;
    f32 measuredSeparation;
    f32 totalRadius;
    f32 firstAdjustmentWeight;
    f32 remainingWeight;
    f32 secondRadius;
    f32 firstRadius;
    f32 separation;
    f32 planarMagnitude;

    secondRadius = second->radius;
    firstRadius = first->radius;
    totalRadius = firstRadius + secondRadius;
    PSVECSubtract(&second->center, &first->center, &difference);
    separation = PSVECMag(&difference);
    measuredSeparation = separation;
    if (measuredSeparation > 0.00001f) {
        PSVECScale(&difference, direction, 1.0f / measuredSeparation);
    }

    difference.y = 0.0f;
    planarMagnitude = PSVECMag(&difference);
    measuredSeparation = planarMagnitude;
    if (measuredSeparation >= totalRadius) {
        return 0;
    }

    firstAdjustmentWeight = firstWeight;
    remainingWeight = 1.0f - firstWeight;
    PSVECScale(&difference, &difference,
               (separationScale * (totalRadius - measuredSeparation)) /
                   measuredSeparation);
    PSVECScale(&difference, &adjustment.vector, -firstAdjustmentWeight);
    first->translate(adjustment);
    PSVECScale(&difference, &adjustment.vector, remainingWeight);
    second->translate(adjustment);
    return 1;
}

// Builds the normalized direction from the first point to the second.
void fn_1_984C(const Point3d *first, const Point3d *second, Point3d *direction)
{
    f32 length;
    f32 magnitude;

    PSVECSubtract(second, first, direction);
    magnitude = PSVECMag(direction);
    length = magnitude;
    if (!(length <= 0.000001f)) {
        PSVECScale(direction, direction, 1.0f / length);
    }
}

// Removes the nonnegative source projection from horizontal target motion.
void fn_1_98F0(const Vec *source, Vec *target)
{
    f32 dot = PSVECDotProduct(source, target);
    f32 compareDot = dot;

    if (!(compareDot < 0.0f)) {
        Vec correction;

        PSVECScale(source, &correction, -compareDot);
        correction.y = 0.0f;
        PSVECAdd(&correction, target, target);
    }
}

// Separates overlapping spheres horizontally for fn_1_999C and keeps both
// scale factors in locals.
inline s32 separateHorizontal(Sphere *first, Sphere *second, f32 firstWeight,
                              f32 separationScale, Point3d *direction)
{
    Point3d difference;
    SphereVector adjustment;
    f32 measuredSeparation;
    f32 totalRadius;
    f32 planarMagnitude;
    f32 separation;
    f32 firstRadius;
    f32 secondRadius;
    f32 adjustmentScale;
    f32 normalizationScale;
    f32 firstAdjustmentWeight;
    f32 remainingWeight;

    secondRadius = second->radius;
    firstRadius = first->radius;
    totalRadius = firstRadius + secondRadius;
    PSVECSubtract(&second->center, &first->center, &difference);
    separation = PSVECMag(&difference);
    measuredSeparation = separation;
    if (measuredSeparation > 0.00001f) {
        normalizationScale = 1.0f / measuredSeparation;
        PSVECScale(&difference, direction, normalizationScale);
    }

    difference.y = 0.0f;
    planarMagnitude = PSVECMag(&difference);
    measuredSeparation = planarMagnitude;
    if (measuredSeparation >= totalRadius) {
        return 0;
    }

    firstAdjustmentWeight = firstWeight;
    remainingWeight = 1.0f - firstWeight;
    adjustmentScale = (separationScale * (totalRadius - measuredSeparation)) / measuredSeparation;
    PSVECScale(&difference, &difference, adjustmentScale);
    PSVECScale(&difference, &adjustment.vector, -firstAdjustmentWeight);
    first->translate(adjustment);
    PSVECScale(&difference, &adjustment.vector, remainingWeight);
    second->translate(adjustment);
    return 1;
}

// Repeatedly separates two object bodies until they stop overlapping.
s32 fn_1_999C(GameObject *first, GameObject *second)
{
    Point3d direction;
    PlayerCharacter *firstPoint = static_cast<PlayerCharacter *>(first);
    PlayerCharacter *secondPoint = static_cast<PlayerCharacter *>(second);
    s32 remaining = 10;
    while (--remaining != 0) {
        if ((u8) separateHorizontal(&firstPoint->body, &secondPoint->body, 0.5f, 1.0f,
                                    &direction) == 0) {
            break;
        }
    }
    return 0;
}

// Destroys the object and its owned members.
PlayerCharacter::~PlayerCharacter() {}

// Returns the centered position of an evenly spaced item.
f32 fn_1_9CC8(s32 index, s32 count, f32 spacing)
{
    --count;
    return spacing * index - spacing * count / 2.0f;
}

// Calls the supplied comparator for each pair in the collected object list.
template<class T, class Callback>
void fn_1_A2EC(std::vector<T> *list, Callback callback)
{
    T *first;
    T *current;
    T *endCopy;
    T *pairStart;
    T *last;
    T currentValue;
    T firstValue;

    first = list->begin();
    endCopy = list->end();
    last = endCopy - 1;
    while (first != last) {
        pairStart = first;
        first += 1;
        current = first;
        while (current != endCopy) {
            firstValue = *current;
            current += 1;
            currentValue = *pairStart;
            callback(currentValue, firstValue);
        }
    }
}
