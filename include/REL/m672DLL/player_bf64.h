#ifndef REL_M672DLL_PLAYER_BF64_H
#define REL_M672DLL_PLAYER_BF64_H
#include "REL/m672DLL/model_bf64.h"
#include "REL/m672DLL/early_callback_8980.h"
#include "REL/m672DLL/framework/owner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <functional>
#include "REL/framework/event_hm.h"
#include "REL/m672DLL/effect_bf64.h"
#include "REL/m672DLL/sequence_bf64.h"
#include "REL/m672DLL/controllers_bf64.h"
extern "C" {
#include "game/mg/actman.h"
}
class VectorValue {
public:
    Vec value;
    VectorValue &setScaled(const Vec *source, f32 scale) {
        PSVECScale(source, &value, scale);
        return *this;
    }
};

typedef GameObject SceneBF64;
struct Player;
struct PolyNode;
typedef void (Player::*PlayerMethodBF64)(EventMessage *);
typedef void (Player::*PlayerUpdateBF64)();

void *operator new(unsigned long);
#include "REL/m672DLL/model_motion_bf64.h"

typedef EventHandlerManager CallbackTableBF64;
typedef std::mem_fun1_t<void, Player, EventMessage *> MemberBF64;
typedef CallbackBinding<Player> BoundBF64;
/* One material colour. */
struct VectorElement {
    u8 byte0;
    u8 byte1;
    u8 byte2;
    u8 byte3;
};
struct Player : GameObject {
    Player(int, const WorldPosition *);
    virtual ~Player();
    virtual void update();
    Vec positionValue() const { return position.value; }
    short modelId;
    short charNo;
    int playerNo;
    MGPLAYER *player;
    ScenePointer672<PlayerController> controller;
    VectorValue position;
    VectorValue velocity;
    float rotation;
    int state;
    float elapsed;
    float delay;
    unsigned char active;
    float fade;
    int phase;
    unsigned char unknown54;
    float movementElapsed;
    int soundFrame;
    int soundHandle;
    ScenePointer672<SceneBF64> sequence;
    ScenePointer672<GroundSprite> effect;
    ModelMotionPointer model;
    WorldPosition origin;
    float modelScale;
    CallbackTableBF64 callbacks;
    PlayerUpdateBF64 frameUpdate;
    void setUpdate(PlayerUpdateBF64 method) { frameUpdate = method; }
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
    std::vector<s16> firstList;
    std::vector<VectorElement> secondList;
};
#endif
