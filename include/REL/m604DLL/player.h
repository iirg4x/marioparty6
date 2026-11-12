// The four players. Player is what both games share: the character model, its motions, the item it carries and the
// step it stands on; DayPlayer climbs in the day game (lbl_802C0278 zero), NightPlayer in the night game. Each player
// is on the a_set list of players from its construction to its destruction.
//
//
//
//
#ifndef REL_M604DLL_PLAYER_H
#define REL_M604DLL_PLAYER_H

#include <memory>
#include <vector>
#include "dolphin/mtx.h"
#include "REL/framework/list.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"

struct PlayerInputBase;
struct PlayerPositionHistory;
struct ModelParticles;
struct PlayerParticleEffect;

class Player : public ListNode<Player> {
public:
    typedef void (Player::*State)();
    enum {
        NUMBEROF_MOTION = 11
    };

    Player(s32 playerIndex);
    void setState(State state) { m_state = state; }
    void fn_1_0();
    void fn_1_4();
    void fn_1_EB98();
    void addMotion(u32 dataNum);
    void selectMotion(s32 index, u8 force, u8 keepAudio, f32 blend);
    s32 getMotion();
    u8 isComputer();
    void attachItem();
    void createItem();
    void releaseItem();
    void fn_1_F198();
    void fn_1_F554();
    s32 getCharacter() { return character; }
    s32 getHeight() { return height; }
    s32 getPlayerIndex() const { return playerIndex; }
    bool hasVerticalPosition() const { return 0.0f == groundTime ? false : true; }

    State m_state;
    s32 playerIndex;
    Vec position;
    s32 step;          // The step the player stands on; -1 in the air.
    s32 motion;        // Index into m_mot_id of the motion playing; -1 before the first.
    s32 animationPhase;
    Vec rotation;      // rotation.y is the facing in degrees.
    Vec velocity;
    s32 character;
    s32 state;
    f32 unknown50;     // the game's code never reads this field
    s32 unknown54;     // the game's code never reads this field
    f32 jumpTime;
    u8 finished;       // Set once the round is over.
    f32 groundTime;
    s32 elapsedFrames;
    s16 nightMode;
    s32 updateCount;
    s32 soundHandle;

    virtual void update() {}
    virtual ~Player();
    virtual void result(u8 won) {}
    virtual void finish() {}
    virtual void ready() { setState(&Player::fn_1_4); }
    virtual void start() { setState(&Player::fn_1_0); }

    std::vector<long> m_mot_id;
    std::auto_ptr<OwnedModel> model;
    std::auto_ptr<OwnedModel> item;
    s32 height;        // The highest step reached, shown as the score.
    f32 rise;
    f32 itemFade;
    f32 itemAngle;
    u8 transitionActive;
    Vec itemPosition;
    f32 fallTime;
    u8 shadowActive;
    Vec itemOffset;
    f32 timer;
};

class DayPlayer : public Player {
public:
    typedef void (DayPlayer::*State)();

    DayPlayer(s32 playerIndex);
    void setState(State state) { m_state = state; }
    virtual void update();
    virtual ~DayPlayer();
    virtual void result(u8 won);
    virtual void finish();
    virtual void ready() { setState(&DayPlayer::fn_1_FF5C); }
    virtual void start() { setState(&DayPlayer::fn_1_104DC); }
    void fn_1_FF5C();
    void fn_1_104DC();
    void fn_1_105D8();
    void fn_1_11574();
    void fn_1_11AE4();

    State m_state;
    std::auto_ptr<PlayerInputBase> controller;
    std::auto_ptr<PlayerPositionHistory> history;
    std::auto_ptr<ModelParticles> particle;
};

class NightPlayer : public Player {
public:
    typedef void (NightPlayer::*State)();

    NightPlayer(s32 playerIndex);
    void setState(State state) { m_state = state; }
    virtual void update();
    virtual ~NightPlayer();
    virtual void result(u8 won);
    virtual void finish();
    virtual void ready() { setState(&NightPlayer::fn_1_128EC); }
    virtual void start() { setState(&NightPlayer::fn_1_12BCC); }
    void fn_1_128EC();
    void fn_1_12BCC();
    void fn_1_12D2C();
    void fn_1_13CE4();
    void fn_1_14254();

    State m_state;
    std::auto_ptr<PlayerInputBase> controller;
    std::auto_ptr<PlayerParticleEffect> effect;
};

#endif
