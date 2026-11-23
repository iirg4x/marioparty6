// Simulates and draws the ropes the player can pull.

#ifndef REL_SEER_TERROR_ROPE_H
#define REL_SEER_TERROR_ROPE_H

#include <vector>
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/sprite.h"
}

struct Hu3DModel_s;
struct RopePoint;

// Owns the rope's models and the points used to simulate its motion.
struct Rope : GameObject {
    OwnedModel tube;
    OwnedModel ends;
    ANIMDATA *animation;
    f32 targetLength;
    u8 pinned : 1;
    std::vector<RopePoint> points;
    Vec target;

    Rope(s32 ropeNumber, const Vec *start, const Vec *end, f32 length, s32 dataNumber);
    ~Rope();
    void update();
};

extern "C" {
Rope *fn_1_1DEB8(s32 id);
void fn_1_1DFA8();
void fn_1_1E638(Rope *rope, const Vec *goal, f64 blend);
void fn_1_1E7F0(Rope *rope);
}

void fn_1_1EF94(Hu3DModel_s *model, Mtx *matrix);
void fn_1_1EFF4(Hu3DModel_s *model, Mtx *matrix);
void fn_1_1E8BC(Rope *rope, RopePoint *first, RopePoint *last);
void fn_1_1E9AC(Rope *rope);
void fn_1_1EDEC(Rope *rope);
void fn_1_1E928(Rope *rope, RopePoint *first, RopePoint *last);

void fn_1_1F054(Rope *, Mtx);
void fn_1_1F150(Rope *);
void fn_1_1F1F0(Rope *);
extern "C" {
void fn_1_1F514(Rope *, Mtx);
}

#endif
