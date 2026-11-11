// Spheres kept on one list by scene category; framework scene_shadow.cpp gathers a category's spheres
// for its shadow cameras. The file, class and member names are descriptive, not recovered: retail keeps
// no name for this unit. It links right after camera_ut1.cpp, so its name sorts before chara.cpp.
#ifndef REL_FRAMEWORK_CATEGORY_SPHERE_H
#define REL_FRAMEWORK_CATEGORY_SPHERE_H

#include <vector>
#include "REL/framework/sphere.h"

// A sphere that joins the category list while it exists. Copies join the list too.
class RegisteredValue : public Sphere {
public:
    long priority : 30;            // The scene category the sphere is gathered under.
    unsigned long : 2;
    unsigned char flag7 : 1;       // Gathered at all while set.
    unsigned char flag6 : 1;       // Gathered for the shadow cameras while set as well.
    unsigned char : 6;
    RegisteredValue *next;         // The next sphere on the list, the latest first.

    RegisteredValue(long category = 0);
    RegisteredValue(const RegisteredValue &source);
    RegisteredValue(const Sphere &sphere, long category = 0);
    RegisteredValue(const Point3d &point, long category = 0);
    // Assigning a sphere or a point keeps the category, the flags and the list place.
    RegisteredValue &operator=(const RegisteredValue &source);
    RegisteredValue &operator=(const Sphere &sphere);
    RegisteredValue &operator=(const Point3d &point);
    ~RegisteredValue();
    void registerValue();
    void unregisterValue();
    void setPriority(unsigned long category);
    void clearFlag7();
    void setFlag7();
    void clearFlag6();
    void setFlag6();
};

// Appends the spheres of one category whose flag7 is set.
void collectCategoryValues(std::vector<Sphere *> *out, long category);
// Appends the spheres of one category whose flag7 and flag6 are set (the shadow cameras' selection).
void collectCategorySpheres(std::vector<Sphere *> *out, long category);

#endif
