#ifndef REL_M671DLL_PLAYER_OBJECT_H
#define REL_M671DLL_PLAYER_OBJECT_H

#include <memory>
#include "REL/m671DLL/control.h"
#include <vector>
#include "REL/framework/event_hm.h"
#include "REL/framework/game_object.h"
#include "REL/m671DLL/collision.h"
#include "REL/framework/model_motion.h"

#include "game/mg/actman.h"

struct Player;
typedef void (Player::*PlayerMethod)(EventMessage *);
typedef void (Player::*PlayerUpdate)();

/* What drives a player: the pad or the computer. */

struct Player : GameObject {
    explicit Player(int playerNo);
    virtual ~Player();
    virtual void update();
    s32 getIndex() const { return playerNo; }
    MGPLAYER *getPlayer() const { return player; }
    void getPosition(Vec &out) const
    {
        out.x = collision.value.x;
        out.y = collision.value.y;
        out.z = collision.value.z;
    }
    void setUpdate(PlayerUpdate method) { currentCallback = method; }
    void fn_1_D3A0();
    void fn_1_D3A4();
    void fn_1_D3A8();
    void fn_1_D434();
    void fn_1_D6DC();
    void fn_1_D978();
    void fn_1_DA70();
    void fn_1_DC6C();
    void fn_1_DC70();
    void fn_1_DD14(unsigned long dataNum);
    void fn_1_DD88(unsigned long motion, unsigned char loop);
    void fn_1_DE4C(unsigned long motion, unsigned char enabled);
    void fn_1_DEF0(EventMessage *);
    void fn_1_DF34(EventMessage *);
    void onStart(EventMessage *);
    void fn_1_E09C(EventMessage *);
    void fn_1_E174(EventMessage *);
    void fn_1_E268(EventMessage *);
    void fn_1_E3D8(EventMessage *);

    short modelId;
    short charNo;
    int playerNo;
    unsigned char finished;
    MGPLAYER *player;
    std::auto_ptr<Controller> controller;
    CollisionObject collision;
    float rotation;
    unsigned char motionEnabled;
    unsigned char isActive() { return motionEnabled; }
    Vec movementVelocity;
    Vec unknown60;
    unsigned char initializedByte; // The game never reads this value.
    ModelMotion *effect;
    int updateCount;
    float effectHeight;
    float effectAngle;
    float effectRadius;
    float effectParameter3;
    float effectParameter4;
    int soundId;
    EventHandlerManager callbacks;
    EventHandlerManager secondaryCallbacks;
    PlayerUpdate currentCallback;
    unsigned long motionResource;
    std::vector<short> motionIds;
    unsigned long effectMotion;
};

#endif
