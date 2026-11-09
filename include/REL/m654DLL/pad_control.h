#ifndef M654DLL_PAD_CONTROL_H
#define M654DLL_PAD_CONTROL_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m654DLL/framework/range.h"

struct MgPlayer_s;

class PadControl {
public:
    PadControl(unsigned short, float);
    ~PadControl();
    u16 cameraBit;
    Range<float> range;
    f32 angle_primary;
    f32 angle_secondary;
    u32 flags_primary;
    u32 flags_secondary;

    void fn_1_C698(u32 flags);
    void fn_1_C6A8(u32 flags);
    void fn_1_C6B8();
    void fn_1_C6C8();
    void fn_1_C6D8();
    void fn_1_C6E8(const Vec *movement, f32 amount);
    void fn_1_C918(MgPlayer_s *player);
    void fn_1_CAB4();
};

void fn_1_C630(PadControl *, float);

#endif
