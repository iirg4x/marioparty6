#ifndef M654DLL_PAD_CONTROL_H
#define M654DLL_PAD_CONTROL_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/range.h"

struct MgPlayer_s;

class PadControl {
public:
    PadControl(unsigned short, float);
    ~PadControl();
    void fn_1_C630(float factor);
    u16 cameraBit;
    Range<float> range;
    f32 stickX;
    f32 stickY;
    u32 pressedButtons;
    u32 heldButtons;

    void fn_1_C698(u32 flags);
    void fn_1_C6A8(u32 flags);
    void fn_1_C6B8();
    void fn_1_C6C8();
    void fn_1_C6D8();
    void fn_1_C6E8(const Vec *movement, f32 amount);
    void fn_1_C918(MgPlayer_s *player);
    void fn_1_CAB4();
};

#endif
