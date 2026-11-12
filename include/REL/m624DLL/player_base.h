#ifndef M624_PLAYER_BASE_H
#define M624_PLAYER_BASE_H
#include "dolphin/types.h"
#include "REL/framework/list.h"
#include <vector>

class Player : public ListNode<Player> {
public:
    typedef void (Player::*Mode)();

    Player(s32 playerNo);
    virtual ~Player();
    virtual void prepare() {}
    virtual void fn_1_7060();
    virtual void moveToStart();
    virtual void startRunning() = 0;
    virtual void showResult() = 0;
    virtual void stopRespawning() = 0;
    virtual void finish() = 0;
    virtual void update() {}

    Mode mode;
    s32 playerNo;
    s32 character;
    s32 group;
    s32 pad;
    s32 difficulty;
    s32 countdown;
    s32 frame;
    std::vector<s16> m_motion_id;
    s16 m_motion;
    void setMotion(s32 motion, f32 startFrame, f32 endFrame, u8 shift);
    Player *next_node() { return static_cast<Player *>(link0); }
    s32 get_group() const { return group; }
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

// The players, in the order they act.
extern ListRoot<Player> lbl_1_bss_9C;

void fn_1_62C8();
void fn_1_635C();
void fn_1_63F8();
void fn_1_6468();
Player *fn_1_65B0(s32 key);

#endif
