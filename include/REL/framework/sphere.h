// A collision sphere and the overlap and contact tests of sphere.cpp.
#ifndef REL_FRAMEWORK_SPHERE_H
#define REL_FRAMEWORK_SPHERE_H

#include <dolphin/mtx.h>

// A sphere in the caller's coordinate system; the camera fit leaves out a nonpositive radius.
struct Sphere {
    Point3d center; // Center position in the caller's coordinate units.
    f32 radius;     // Radius in the same units as the center coordinates.

    Sphere() {}
    Sphere(const Point3d &point, f32 extent) : center(point), radius(extent) {}
    Sphere(const Sphere &other) : center(other.center), radius(other.radius) {}
    Sphere &operator=(const Sphere &other)
    {
        center = other.center;
        radius = other.radius;
        return *this;
    }
    void setRadius(f32 value) { radius = value; }
    f32 getRadius() const { return radius; }
    void translate(Point3d *offset) { PSVECAdd(offset, &center, &center); }
};

bool spheresOverlap(Sphere *first, Sphere *second);
bool resolveSphereContact(Sphere *first, Sphere *second, f32 firstShare, f32 correctionScale);
s32 intersectSpherePaths(f32 *contactTime, Point3d *firstStart, Sphere *first,
                         Point3d *secondStart, Sphere *second);

#endif
