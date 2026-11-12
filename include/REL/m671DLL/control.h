#ifndef REL_M671DLL_CONTROL_H
#define REL_M671DLL_CONTROL_H
#include "REL/m671DLL/tail_types.h"
struct Player;
struct StickState {
    f32 x, y, magnitude;
    u32 pressedButtons, heldButtons;
};
class Controller {
public:
    explicit Controller(Player *owner) : player(owner), angleWork(1, 0.0f) {}
    virtual ~Controller();
    virtual void update(StickState *) = 0;
    virtual void fn_1_28D8();
    StickState stick;
    u8 unknown18[4]; /* No code names this field. */
    Player *player;
    AngleWork angleWork;
};
void fn_1_13C8(Controller *state);
class HumanControl : public Controller {
public:
    explicit HumanControl(Player *owner);
    virtual ~HumanControl() {}
    virtual void update(StickState *state);
    s32 padIndex;
};
class ComputerControl : public Controller {
public:
    ComputerControl(Player *player, int option);
    virtual ~ComputerControl();
    virtual void update(StickState *state);
    s32 option;
    f32 movementParameters[5];
    Vec movement;
    f32 movementScale;
    void (ComputerControl::*callback)(StickState *);
    void setUpdate(void (ComputerControl::*method)(StickState *)) { callback = method; }
    void fn_1_1A30(StickState *state);
};
#endif
