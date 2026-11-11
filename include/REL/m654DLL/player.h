// The two contestants: their spawn data, the player object and the views of other objects' memory it reads.
#ifndef M654_PLAYER_H
#define M654_PLAYER_H
#include <vector>
#include <memory>
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/framework/chara_motion.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object.h"
#include "REL/framework/p_info.h"
#include "REL/m654DLL/mode_object.h"

struct PlayerInitializationData {
    Vec position;
    Vec path[2];
};

class Player : public PlayerInfo, public MotionOwner, public GameObject, public EventListener {
public:
    Player(int playerId, PlayerInitializationData *spawnData);
    virtual ~Player();
    virtual void update();
    virtual void receive(const EventPacket *event);
    void fn_1_35F4();
    void fn_1_3E84();
    void fn_1_3F20();
    void fn_1_3FE0();
    void fn_1_404C();
    void fn_1_411C();
    void fn_1_41A8();
    void fn_1_4250();
    void fn_1_42DC();
    void fn_1_43A0();
    void fn_1_4428();
    void fn_1_44FC();
    void fn_1_4610();
    void fn_1_4654();
    void fn_1_46BC();
    void fn_1_4858();
    void fn_1_4A0C();
    void fn_1_4ACC();

    void (Player::*callback)();
    MGPLAYER *player;
    unsigned char inactive : 1;
    unsigned char started : 1;
    unsigned char reachedPlace : 1;
    unsigned char attributeEnabled : 1;
    unsigned char transitionStarted : 1;
    unsigned char flagsLow : 3;
    GameObject *display;
    std::vector<Vec> path;
    Vec position;
    Vec velocity;
    float rotationY;
    std::auto_ptr<PlayerControl> mode;
};

struct DisplayState {
    unsigned char unknown00[0x34]; /* No code names this field. */
    int progress;
};

struct PlayerModelMotion {
    unsigned char unknown00[0x1C]; /* No code names this field. */
    short motionId;
};

struct PlayerMotionResource {
    unsigned char unknown00[0x48]; /* No code names this field. */
    PlayerModelMotion *model;
};

struct PlayerFilter {
    int unknown00; /* the game's code never reads this field */
    int playerId;
    int flags;
};

#endif
