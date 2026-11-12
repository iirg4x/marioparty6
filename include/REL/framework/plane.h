// A plane equation and how it is built from points or from a normal (plane.cpp).
#ifndef REL_FRAMEWORK_PLANE_H
#define REL_FRAMEWORK_PLANE_H

#include "REL/framework/vector3.h"

// Stores a unit normal and the plane's signed offset from the origin.
struct Plane {
    Vector3 normal; // Unit direction perpendicular to the plane.
    float distance; // Signed offset in the same units as the points it was built from.

    void setFromPoints(Vec *first, Vec *second, Vec *third);
    void setFromNormalAndPoint(Vec *normal, Vec *point);
};

#endif
