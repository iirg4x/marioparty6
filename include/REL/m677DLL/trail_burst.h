// Creates particle bursts and draws the trails they leave.

#ifndef REL_SEER_TERROR_TRAIL_BURST_H
#define REL_SEER_TERROR_TRAIL_BURST_H

#include "REL/framework/vector_math.h"
#include <list>
#include <vector>
#include "REL/framework/game_object2.h"

// Tracks a trail's recent positions, velocity and remaining travel distance.
struct Trail {
    std::vector<Vec> points;
    Vec velocity;
    f32 life;
    f32 remaining;
    u32 head;

    Trail(const Vec &start, const Vec &velocityValue, float lifeValue);
    // Returns the trail point at the given index.
    Vec *pointAt(s32 index) { return &points[index]; }
    // Returns the magnitude of the trail velocity.
    float speed() { return vectorMagnitude(&velocity); }
    bool update();
    void draw();
};

// Owns and updates the trails and draws each as a line segment.
struct TrailBurst : RenderObject {
    std::list<Trail> trails;

    TrailBurst(const Vec *origin, float radius, float duration, int trailCount);
    virtual ~TrailBurst();
    virtual void update();
    virtual void draw(Mtx *view);
};

#endif
