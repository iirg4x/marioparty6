#ifndef M624_PLAYER_TASK_H
#define M624_PLAYER_TASK_H

extern "C" {
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
}

struct PlayerRunner;

struct Position {
    Vec value;
    Position &normalize() { PSVECNormalize(&value, &value); return *this; }
    Position &operator*=(f32 scale) { PSVECScale(&value, &value, scale); return *this; }
    Position &operator+=(const Position &other) {
        PSVECAdd(&other.value, &value, &value); return *this;
    }
};

struct PlayerControl {
    virtual ~PlayerControl();
    virtual void reset();
    virtual void update() = 0;
    PlayerRunner *owner;
    PlayerControl(PlayerRunner *owner) : owner(owner) {}
};
struct ControlOwner {
    PlayerControl *value;
    ~ControlOwner();
};

#include "REL/m624DLL/player_base.h"
struct PlayerRunner : Player {
    PlayerRunner(s32, Position *);
    virtual ~PlayerRunner();
    virtual void prepare();
    virtual void moveToStart();
    virtual void startRunning();
    virtual void showResult();
    virtual void stopRespawning();
    virtual void finish();
    virtual void update();
    u8 despawned;
    u8 respawnBlocked;
    u8 hasWon;
    u8 unk47; /* The game's code never reads this member. */
    Position spawn_position;
    Position current_position;
    Position updated_position;
    Vec velocity;
    Vec rotation;
    f32 scale;
    MGPLAYER *player;
    HSF_OBJECT *lastCollisionModel;
    f32 lastYaw;
    s32 countdown;
    ControlOwner cleanup_owner;
    u8 collisionPending;
    Vec collisionPosition;
    void fn_1_9900();
    void fn_1_868C();
    void fn_1_8840();
    void fn_1_89C0();
    void fn_1_7D2C();
    void fn_1_7BD0();
    void fn_1_8BD8();
    void fn_1_8D78();
    // Counts countdown down; true on the frame it reaches zero.
    bool countDown()
    {
        bool expired = false;
        if (countdown > 0) {
            if (--countdown == 0) {
                expired = true;
            }
        }
        return expired;
    }
};

void fn_1_8CF4(PlayerRunner *object);
void fn_1_8B58(PlayerRunner *task);
void fn_1_97DC(PlayerRunner *owner);
u8 fn_1_911C(PlayerRunner *owner);
void fn_1_9078(PlayerRunner *owner, u8 enabled);

#endif
