#ifndef REL_M655DLL_ROPE_H
#define REL_M655DLL_ROPE_H

#include <vector>
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/m655DLL/rope_point.h"
extern "C" {
#include "game/sprite.h"
}

struct RopeStart {
    Vec position;
    s32 dataNumber;
};
struct Rope : GameObject {
    OwnedModel tube;
    OwnedModel ends;
    ANIMDATA *animation;
    f32 targetLength;
    u8 pinned : 1;
    std::vector<RopePoint> points;
    Rope(s32 mode, const Vec *start, Vec *end, f32 length, s32 dataNumber);
    ~Rope();
    void update();
};

#endif
