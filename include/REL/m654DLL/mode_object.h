#ifndef M654_MODE_OBJECT_H
#define M654_MODE_OBJECT_H
#include "REL/m654DLL/pad_control.h"
class Player;

class PlayerControl {
public:
    PlayerControl(Player *owner) : work(owner) {}
    virtual ~PlayerControl();
    virtual void enter();
    virtual void update() = 0;
    Player *work;
};

class HumanControl : public PlayerControl {
public:
    HumanControl(Player *owner) : PlayerControl(owner) {}
    virtual ~HumanControl();
    virtual void update();
};

struct ComputerControl;
typedef void (ComputerControl::*Callback29C)();
struct ComputerControl : PlayerControl {
    Callback29C callback;
    unsigned int weightIndex;
    Vec position;
    PadControl cameraControl;
    ComputerControl(void *);
    virtual ~ComputerControl();
    void fn_1_170();
    virtual void enter();
    virtual void update();
    void fn_1_29C();
    void fn_1_50C();
    void fn_1_7C8();
    void fn_1_830();
};

#endif
