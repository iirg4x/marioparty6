// Moves an object through a queue of waypoints.

#ifndef REL_SEER_TERROR_PATH_FOLLOWER_H
#define REL_SEER_TERROR_PATH_FOLLOWER_H

#include <list>
#include <vector>
#include "dolphin/mtx.h"

// Advances a position along the current route.
struct PathFollower {
    Vec position;
    // Returns the address of the current position.
    Vec *getPosition() { return &position; }
    u8 unk0C[0xC]; /* No code names this field. */
    f32 speed;
    f32 angle;
    std::list<Vec> path;

    PathFollower();
    PathFollower(const Vec &start, f32 initialSpeed);
};

extern "C" {
void fn_1_22150(PathFollower *self, float value);
float fn_1_221AC(PathFollower *self);
bool fn_1_221B4(PathFollower *self);
void fn_1_221D8(PathFollower *self, const Vec *point);
void fn_1_22254(PathFollower *self, std::vector<Vec> *route);
double fn_1_222F8(Vec *position, Vec *target, double requestedDistance);
bool fn_1_22454(PathFollower *self);
}

float fn_1_226FC(float value);

#endif
