#ifndef REL_M671DLL_TAIL_TYPES_H
#define REL_M671DLL_TAIL_TYPES_H

#include "dolphin/mtx.h"
extern "C" {
#include "game/mg/actman.h"
}

#include "REL/framework/linknode.h"
#include "math.h"
#include "REL/framework/range.h"

extern "C" {
BOOL fn_80086DD0(MGACTOR *actorP, u32 *code);
void fn_80087158(MGPLAYER *playerP, u16 attr);
}

class AngleWork {
public:
    AngleWork(u16 type, f32 value);
    ~AngleWork();
    void fn_1_114F8(u32 flags);
    void fn_1_11508(u32 flags);
    void fn_1_11518();
    void fn_1_11528();
    void fn_1_11538();
    void fn_1_11914();
    u16 type;
    Range<float> range;
    f32 angle_primary;
    f32 angle_secondary;
    u32 flags_primary;
    u32 flags_secondary;
};

void fn_1_11548(AngleWork *work, const Vec *movement, f32 amount);
void fn_1_11778(AngleWork *work, MGPLAYER *player);

#endif
