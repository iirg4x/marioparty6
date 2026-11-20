#ifndef REL_M611DLL_SPHERE_H
#define REL_M611DLL_SPHERE_H

#include "dolphin/mtx.h"

// A three-component position copied from a game vector.
struct SpherePoint {
    f32 x, y, z;
    // Copies all three position components.
    SpherePoint &operator=(const Vec &value)
    {
        x = value.x;
        y = value.y;
        z = value.z;
        return *this;
    }
};

// A center and radius used by the camera fit.
struct Sphere {
    SpherePoint center;
    f32 radius;

    // Moves the sphere center to the supplied position.
    void moveTo(const Vec &value)
    {
        SpherePoint &place = center;
        place = value;
    }
    // Sets the sphere radius.
    void setRadius(f32 value) { radius = value; }
};

#endif
