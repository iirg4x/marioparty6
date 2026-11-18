#ifndef REL_FRAMEWORK_SPHERE_TYPE_H
#define REL_FRAMEWORK_SPHERE_TYPE_H

#include "REL/framework/vector_math.h"

// A sphere in the caller's coordinate system; the camera fit leaves out a nonpositive radius.
struct Sphere {
    Point3d center; // Center position in the caller's coordinate units.
    f32 radius;     // Radius in the same units as the center coordinates.

    // Constructs a sphere without assigning its center or radius.
    Sphere() {}
    // Initializes the sphere with a center and radius.
    Sphere(const Point3d &point, f32 extent) : center(point), radius(extent) {}
    // Copies the center and radius from another sphere.
    Sphere(const Sphere &other) : center(other.center), radius(other.radius) {}
    // Copies the sphere center and radius.
    Sphere &operator=(const Sphere &other)
    {
        center = other.center;
        radius = other.radius;
        return *this;
    }
    // Sets the sphere radius.
    void setRadius(f32 value) { radius = value; }
    f32 getRadius() const { return radius; }
    // Adds the offset to the sphere center.
    void translate(Point3d *offset) { PSVECAdd(offset, &center, &center); }
};

#endif
