#ifndef M672_CONTROLLERS_BF64_H
#define M672_CONTROLLERS_BF64_H
#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"
#include "REL/m672DLL/framework/range.h"
struct Player;
typedef struct PadState8F4 {
    f32 x;
    f32 y;
    f32 magnitude;
    s32 buttonsDown;
    s32 buttons;
    s32 mode;
} PadState8F4;
struct MgPlayer_s;
class VirtualPad {
public:
    u16 type;
    Range bounds;
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
extern "C" void *fn_1_129D8(VirtualPad *, u16, float);
struct PlayerController {
    virtual ~PlayerController();
    virtual void update(PadState8F4 *) = 0;
    virtual inline void reset();
    PadState8F4 values;
    u8 unknown_1C[4]; /* No code names this field. */
    Player *player;
    VirtualPad input;
    PlayerController(Player *owner) : player(owner), input(1,0.0f) {}
};
struct ComController : PlayerController {
    ComController(Player *,int);
    virtual ~ComController();
    virtual void update(PadState8F4 *);
    virtual void reset();
    float angle, angleStep;
    s32 difficulty;
    Vec destination;
    float elapsed;
    void (ComController::*callback)(PadState8F4 *);
    void fn_1_EA0(PadState8F4 *);
    void fn_1_EA4(PadState8F4 *);
    void fn_1_10B8();
};
struct PadController : PlayerController {
    explicit PadController(Player *);
    virtual inline ~PadController();
    virtual void update(PadState8F4 *);
    float angle,angleStep; s32 padNumber;
};
#endif
