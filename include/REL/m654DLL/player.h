// The two contestants: their spawn data, the player object and their movement and collision state.
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

struct DisplayProvider;

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
    unsigned char fn_1_4B68();
    void fn_1_4BC0(bool enabled);
    void fn_1_4C34(unsigned char enabled);

    void (Player::*callback)();
    MGPLAYER *player;
    unsigned char inactive : 1;
    unsigned char collidedThisFrame : 1;
    unsigned char leadsExit : 1;
    unsigned char finishing : 1;
    unsigned char hasWon : 1;
    DisplayProvider *display;
    std::vector<Vec> path;
    Vec position;
    Vec velocity;
    float rotationY;
    std::auto_ptr<PlayerControl> mode;
};

#endif
