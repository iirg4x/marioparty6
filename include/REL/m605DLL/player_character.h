#ifndef REL_M605DLL_PLAYER_CHARACTER_H
#define REL_M605DLL_PLAYER_CHARACTER_H
#include "REL/framework/game_object.h"
#include "REL/framework/plane.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/registered_value.h"
#include "REL/framework/p_chara.h"
#include "REL/framework/owned_pointer.h"
class TriangleCollision;
struct MotionController;

class OwnerCallback;

// Stores a player's movement, collision and motion state.
struct PlayerCharacter : GameObject {
    s32 unk10; /* The game's code never reads this field. */
    u8 rayHit : 1;
    u8 contactFacesUp : 1;
    u8 belowFloor : 1;
    u8 atFinish : 1;
    bool jumpRequested : 1;
    u8 unk14 : 1; /* The game's code never reads this field. */
    f32 targetX;
    f32 flagStartTime;
    f32 angle;
    f32 targetAngle;
    Sphere body;
    Vec previousPosition;
    RegisteredValue projectedPosition;
    Plane contactPlane;
    Vec movementInput;
    Vec velocity;
    Vec acceleration;
    EventHandlerManager channel1Events;
    EventHandlerManager channel0Events;
    s32 motionDataNum;
    OwnedPointer<OwnerCallback> currentState;
    OwnedPointer<PlayerConfigSelection> playerConfig;
    OwnedPointer<TriangleCollision> collisionObject;
    OwnedPointer<MotionController> steering;
    // Returns the player information consumed by the character operations.
    PlayerConfigSelection *getResource() const { return playerConfig.get(); }
    // Returns the player's collision object.
    TriangleCollision *GetCollisionObject() { return collisionObject.get(); }
    // Returns the player information used by the motion controller.
    PlayerInfo *GetPlayerConfiguration() { return playerConfig.pointer; }
    PlayerCharacter(PlayerConfigSelection *playerConfig, TriangleCollision *collisionObject,
                    s32 option);
    virtual ~PlayerCharacter();
    virtual void update();
    void enterHitState();
    void fn_1_8F10(const EventPacket &);
    void fn_1_8EB8(const EventPacket &);
    void fn_1_8BDC(const EventPacket &);
    void fn_1_8CE4(const EventPacket &);
    void fn_1_8DEC(const EventPacket &);
};

void fn_1_8B20(PlayerCharacter *player, u32 value, u8 enabled);

void fn_1_7A68(PlayerCharacter *player, OwnerCallback *replacement);

bool fn_1_9018();
s32 fn_1_999C(GameObject *left, GameObject *right);
unsigned char fn_1_960C(const GameObject *state);
bool fn_1_959C(GameObject *object);
bool fn_1_9120();
void fn_1_92EC();

bool fn_1_9630(const GameObject *, const GameObject *);

f32 fn_1_9CC8(s32 index, s32 count, f32 spacing);

#endif
