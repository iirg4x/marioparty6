#ifndef REL_M639DLL_LINE_OBJECT_H
#define REL_M639DLL_LINE_OBJECT_H
// Lines that fall from the map's lights: each is a pair of points on a stretched spring.
#include "REL/framework/game_object2.h"
#include <list>
#include "REL/framework/vector3.h"

class LinePoint : public Vector3 {
public:
    LinePoint &operator=(const LinePoint &other);
    LinePoint &operator*=(f32 scale);
};

class Line {
public:
    LinePoint points[2];
    LinePoint velocities[2];
    Vec target;
    float distanceLimit;
    float accumulatedDistance;
    Line(const Vec *position, const Vec *target);
    f32 measureDistance(Vec &difference);
    void constrain();
    bool update();
};

class LineObject : public RenderObject {
public:
    std::list<Line> lines;
    LineObject(const Vec *position, const Vec *target);
    virtual ~LineObject();
    virtual void update();
    virtual void draw(Mtx *matrix);
};

#endif
