#ifndef REL_M639DLL_PLAYER_H
#define REL_M639DLL_PLAYER_H
// The players: a sampled input history, their motion, and the lists of them the game walks.
#include <vector>
#include <queue>
#include <memory>
#include "REL/framework/game_object.h"
#include "REL/m639DLL/player_control.h"
#include "REL/framework/p_chara.h"
#include "REL/m639DLL/sphere.h"

void selectCharacterMotion(MotionOwner *character, unsigned int motion);
void shiftCharacterMotion(MotionOwner *character, unsigned int motion, float blend, float start, unsigned long attributes);
#include "REL/framework/event.h"

/* The last second of a player's input and how many of its samples were active. */
class InputWindow {
public:
    int activeCount;
    std::queue<bool> queue;
    InputWindow();
    void update(bool active);
    int count() const;
};

class Player : public PlayerConfigSelection, public GameObject {
public:
    virtual ~Player();
    virtual void update();
    void (Player::*callback)();
    char *itemHook;
    int mode;
    int selectionNumber;
    int selectionIndex;
    OwnedModel hookModel;
    unsigned char unknownModelState;
    unsigned char useAlternateMotion;
    Vec rotation;
    float targetTime;
    float previousTime;
    float blend;
    float direction;
    float previousBeat;
    int countdown;
    InputWindow inputHistory;
    std::auto_ptr<PlayerControl> hook;
    Player(long value, long mode, long selector);
    void fn_1_704C();
    void fn_1_7050();
    void setCountdown(int value) { countdown = value; }
    void fn_1_7528();
    void fn_1_77B4();
    void fn_1_7654();
    void fn_1_7704();
};

extern "C" {
void fn_1_5F08(void);
void fn_1_6058(void);
void fn_1_626C(void);
void fn_1_63A4(void);
void fn_1_6444(std::vector<Player *> *actors);
int fn_1_67B4(unsigned int playerNumber);
void fn_1_68D8(std::vector<Sphere> *spheres);
void fn_1_75BC(Player *target, float input);
}

#endif
