// A collision sphere and the overlap and contact tests of sphere.cpp.
#ifndef REL_FRAMEWORK_SPHERE_H
#define REL_FRAMEWORK_SPHERE_H

#include "REL/framework/sphere_type.h"

bool spheresOverlap(Sphere *first, Sphere *second);
bool resolveSphereContact(Sphere *first, Sphere *second, f32 firstShare, f32 correctionScale);
s32 intersectSpherePaths(f32 *contactTime, Point3d *firstStart, Sphere *first,
                         Point3d *secondStart, Sphere *second);

#endif
