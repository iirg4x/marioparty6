#ifndef M672_CONTROLLERS_H
#define M672_CONTROLLERS_H
#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"
#include "REL/framework/range.h"
struct Player;
typedef struct PadState {
    f32 x;
    f32 y;
    f32 magnitude;
    s32 buttonsDown;
    s32 buttons;
    s32 mode;
} PadState;
struct MgPlayer_s;
class VirtualPad {
public:
    u16 type;
    Range<float> bounds;
    float horizontal, vertical; s32 buttonsDown, buttons;
    VirtualPad(u16, float);
    ~VirtualPad();
    void fn_1_12AB8(float);
    void fn_1_12B20(u32);
    void fn_1_12B30(u32);
    void fn_1_12B40();
    void fn_1_12B50();
    void fn_1_12B60();
    void fn_1_12B70(const Vec *, float);
    void fn_1_12DA0(MgPlayer_s *);
    void fn_1_12F3C();
};
struct PlayerController {
    virtual ~PlayerController();
    virtual void update(PadState *) = 0;
    virtual inline void reset();
    PadState values;
    u8 unknown_1C[4]; /* No code names this field. */
    Player *player;
    VirtualPad input;
    // Store the owning player and initialize its virtual pad.
    PlayerController(Player *owner) : player(owner), input(1,0.0f) {}
};
struct ComController : PlayerController {
    ComController(Player *,int);
    virtual ~ComController();
    virtual void update(PadState *);
    virtual void reset();
    float angle, angleStep;
    s32 difficulty;
    Vec destination;
    float elapsed;
    void (ComController::*callback)(PadState *);
    // Select the update callback.
    void setUpdate(void (ComController::*method)(PadState *)) { callback = method; }
    void fn_1_EA0(PadState *);
    void fn_1_EA4(PadState *);
    void fn_1_10B8();
};
struct PadController : PlayerController {
    explicit PadController(Player *);
    virtual inline ~PadController();
    virtual void update(PadState *);
    float angle,angleStep; s32 padNumber;
};
void fn_1_758(PlayerController *);

#endif
