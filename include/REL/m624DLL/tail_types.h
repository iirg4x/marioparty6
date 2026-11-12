#ifndef REL_M624DLL_TAIL_TYPES_H
#define REL_M624DLL_TAIL_TYPES_H

#include "dolphin/mtx.h"
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/framework/linknode.h"
#include "REL/m624DLL/range.h"

/* CA90/CF60/CD78 consume a Range<float> followed by two angles and two pad-state words. */
class M624PadControl {
public:
    M624PadControl(u16, f32);
    ~M624PadControl();
    u16 type;
    u8 unknown02[2]; /* No code names this field. */
    Range<float> range;
    f32 angle_primary;
    f32 angle_secondary;
    u32 flags_primary;
    u32 flags_secondary;
    void fn_1_CAF8(u32 flags);
    void fn_1_CB08(u32 flags);
    void fn_1_CB18();
    void fn_1_CB28();
    void fn_1_CB38();
};

void fn_1_CA90(M624PadControl *object, f32 value);
void fn_1_CB48(M624PadControl *, const Vec *, f32);
void fn_1_CD78(M624PadControl *, MGPLAYER *);
void fn_1_CF60(Range<float> *self, f32 value);

struct HandleListObject;
void *fn_1_5E1C(HandleListObject *);
void *fn_1_5E24(void *handle);

#endif
