#ifndef M639_SPHERE_H
#define M639_SPHERE_H
// The camera's spheres are the framework's Sphere.
#include "REL/framework/sphere.h"
#include <vector>

extern "C" {
void fn_1_DECC(std::vector<Sphere> *positions);
}

#endif
