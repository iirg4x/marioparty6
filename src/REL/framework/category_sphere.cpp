// The category list of registered spheres (the class is in registered_value.h).
#include "REL/framework/category_sphere.h"
#include "REL/framework/unregister_value.h"

// The list head: the sphere registered last.
RegisteredValue *registeredValues;

// Registers a unit sphere in the given category with both flags set.
RegisteredValue::RegisteredValue(long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    registerValue();
    radius = 1.0f;
}

// Copies the sphere and its category flags, then registers the copy.
RegisteredValue::RegisteredValue(const RegisteredValue &source)
{
    *this = source;
    registerValue();
}

// Registers a copy of the sphere in the given category with both flags set.
RegisteredValue::RegisteredValue(const Sphere &sphere, long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    center = sphere.center;
    radius = sphere.radius;
    registerValue();
}

// Registers a unit sphere at the point with the category and both flags set.
RegisteredValue::RegisteredValue(const Point3d &point, long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    center = point;
    registerValue();
    radius = 1.0f;
}

// Copies the center, radius, category and flags without changing registration.
RegisteredValue &RegisteredValue::operator=(const RegisteredValue &source)
{
    center = source.center;
    radius = source.radius;
    priority = source.priority;
    flag7 = source.flag7;
    flag6 = source.flag6;
    return *this;
}

// Copies the center and radius without changing the category or registration.
RegisteredValue &RegisteredValue::operator=(const Sphere &sphere)
{
    center = sphere.center;
    radius = sphere.radius;
    return *this;
}

// Moves the center to the point.
RegisteredValue &RegisteredValue::operator=(const Point3d &point)
{
    center = point;
    return *this;
}

// Removes the sphere from the registered list.
RegisteredValue::~RegisteredValue()
{
    unregisterValue();
}

// Appends the category spheres whose flag7 is set.
void collectCategoryValues(std::vector<Sphere *> *out, long category)
{
    for (RegisteredValue *value = registeredValues; value; value = value->next) {
        if (value->flag7 && value->priority == category)
            out->push_back(value);
    }
}

// Appends the category spheres whose flag7 and flag6 are both set.
void collectCategorySpheres(std::vector<Sphere *> *out, long category)
{
    for (RegisteredValue *value = registeredValues; value; value = value->next) {
        if (value->flag7 && value->flag6 && value->priority == category)
            out->push_back(value);
    }
}

// Prepends the sphere to the registered list.
void RegisteredValue::registerValue()
{
    next = registeredValues;
    registeredValues = this;
}

// Unlinks the sphere from the registered list.
void RegisteredValue::unregisterValue()
{
    unlinkEntry(this, &registeredValues);
}

// Changes the category used to select this sphere.
void RegisteredValue::setPriority(unsigned long category)
{
    priority = category;
}

// Clears flag7 to exclude the sphere from category collection.
void RegisteredValue::clearFlag7()
{
    flag7 = 0;
}

// Sets flag7 to allow the sphere in category collection.
void RegisteredValue::setFlag7()
{
    flag7 = 1;
}

// Clears flag6 to exclude the sphere from the selection requiring both flags.
void RegisteredValue::clearFlag6()
{
    flag6 = 0;
}

// Sets flag6 to allow the sphere in the selection requiring both flags.
void RegisteredValue::setFlag6()
{
    flag6 = 1;
}
