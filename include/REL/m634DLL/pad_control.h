#ifndef REL_M634DLL_PAD_CONTROL_H
#define REL_M634DLL_PAD_CONTROL_H
#include "dolphin/mtx.h"
#include "math.h"
#include "REL/framework/range.h"
extern "C" {
#include "game/mg/actman.h"
}
class PadControl {
public:
    PadControl(u16, f32);
    ~PadControl();
    u16 cameraBit;
    u16 unknown;
    Range<float> range;
    f32 angle_primary;
    f32 angle_secondary;
    u32 flags_primary;
    u32 flags_secondary;
    void fn_1_15E24(u32 flags);
    void fn_1_15E34(u32 flags);
    void fn_1_15E44();
    void fn_1_15E54();
    void fn_1_15E64();
    void fn_1_16240();
};

void fn_1_15E74(PadControl *, const Vec *, f32);

#endif
