#include "REL/m655DLL/game_constants.h"
#include "REL/framework/random.h"
#include "REL/framework/frame_time.h"
/* The goal post: its model, its light and the shuffled order of its three positions. */
extern "C" {
#include "dolphin/types.h"
}
#include "REL/framework/model.h"
#include <vector>
#include <algorithm>
#include "datadir_enum.h"
#include "REL/m655DLL/goal.h"
#include "REL/framework/world_units.h"
enum { NUMBER_OF_STATE = 3 };
Goal::Goal() : std::vector<int>(NUMBER_OF_STATE), effectObject(DATANUM(0x720000, 1)), light(1) {
    float range;
    effectObject.setLayer(1);
    effectObject.setAttribute(0x40000001);
    effectObject.setShadowMap();
    effectObject.resetObjectShadowMap("shaft");
    light.clearFlag7();
    range = 70.0f;
    light.radius = range;
    registerChannel(0);
}
Goal::~Goal() {}
/* Reduce the signed random word by the upper bound. */
int fn_1_3178(s32 maximum) {
    return (s32)RandomUnsigned() % maximum;
}
struct RandomBound {
    int operator()(int bound) {
        return (s32)RandomUnsigned() % bound;
    }
};
void Goal::fn_1_31AC() {
    f32 height = cmToWorld(-100.0f);
    static Vec positions[3] = {{cmToWorld(-250.0f), height, cmToWorld(1600.0f)},
                               {cmToWorld(0.0f), height, cmToWorld(1600.0f)},
                               {cmToWorld(250.0f), height, cmToWorld(1600.0f)}};
    int state;
    clear();
    state = 0;
    while (state < NUMBER_OF_STATE) {
        push_back(state);
        state++;
    }
    Vec *position;
    RandomBound random;
    std::random_shuffle(begin(), end(), random);

/* Retain assertion source location.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
    assert(size() == NUMBER_OF_STATE);
    position = &positions[(*this)[0]];
    effectObject.setPosition(position);
    light = *position;
}

void Goal::fn_1_3430(float end) {
    light.center.x = end;
    effectObject.setPosition(&light.center);
}
Vec *Goal::fn_1_3470() {
    return &light.center;
}

void Goal::receive(const EventPacket *packet) {
    if (packet->type == 5) {
        light.setFlag7();
    }
}
