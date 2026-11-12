#ifndef REL_FRAMEWORK_PLANE_H
#define REL_FRAMEWORK_PLANE_H
#include "dolphin/mtx.h"

class Plane {
public:
    Point3d normal;
    f32 distanceFromOrigin;
    void setFromPoints(Point3d *, Point3d *, Point3d *);
    void setFromNormalAndPoint(Point3d *, Point3d *);
    void fn_1_8C84(f32, f32);
};
#endif
