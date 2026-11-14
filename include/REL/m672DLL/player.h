#ifndef REL_M672DLL_PLAYER_H
#define REL_M672DLL_PLAYER_H
#include "REL/framework/model.h"
#include "REL/m672DLL/member_callback.h"
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <functional>
#include "REL/framework/event_hm.h"
#include "REL/m672DLL/ground_sprite.h"
#include "REL/m672DLL/particle_groups.h"
#include "REL/m672DLL/controllers.h"
extern "C" {
#include "game/mg/actman.h"
}

struct Player;
typedef void (Player::*PlayerHandler)(EventMessage *);
typedef void (Player::*PlayerUpdate)();

#include <new>
#include "REL/framework/model_motion.h"

typedef std::mem_fun1_t<void, Player, EventMessage *> PlayerMemberFunction;
typedef CallbackBinding<Player> PlayerCallbackBinding;
/* One material colour. */
struct VectorElement {
    u8 red;
    u8 green;
    u8 blue;
    u8 unidentified;
};
struct Player : GameObject {
    Player(int, const EffectVector *);
    virtual ~Player();
    virtual void update();
    // Returns the player's position vector by value.
    Vec positionValue() const { return position.value; }
    short modelId;
    short charNo;
    int playerNo;
    MGPLAYER *player;
    std::auto_ptr<PlayerController> controller;
    EffectVector position;
    EffectVector velocity;
    float rotation;
    int state;
    float elapsed;
    float delay;
    unsigned char active;
    float fade;
    int phase;
    unsigned char finished;
    float movementElapsed;
    int soundFrame;
    int soundHandle;
    std::auto_ptr<RingParticles> ringParticles;
    std::auto_ptr<GroundSprite> groundSprite;
    std::auto_ptr<ModelMotion> model;
    EffectVector origin;
    float modelYaw;
    EventHandlerManager callbacks;
    PlayerUpdate frameUpdate;
    // Stores the player's frame-update callback.
    void setUpdate(PlayerUpdate method) { frameUpdate = method; }
    void fn_1_EB5C(EventMessage *);
    void fn_1_EBF4(EventMessage *);
    void fn_1_EC80(EventMessage *);
    void fn_1_ED84(EventMessage *);
    void fn_1_DC00();
    void fn_1_D21C();
    void fn_1_DBF4();
    void fn_1_DC04();
    void fn_1_E29C();
    void fn_1_E5EC();
    void fn_1_E718();
    void fn_1_E95C();
    void fn_1_E960();
    void fn_1_EA24(s32 dataNum);
    void fn_1_CEC0();
    void fn_1_D120();
    void fn_1_D8CC();
    void fn_1_D97C();
    void fn_1_DA64(f32 redScale, f32 greenScale, f32 blueScale);
    unsigned int currentMotionIndex;
    std::vector<s16> motionIds;
    std::vector<VectorElement> materialColors;
};
#endif
