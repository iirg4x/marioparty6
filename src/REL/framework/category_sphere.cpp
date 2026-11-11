// The category list of registered spheres (see category_sphere.h).
#include "REL/framework/category_sphere.h"

// The list head: the sphere registered last.
RegisteredValue *registeredValues;

RegisteredValue::RegisteredValue(long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    registerValue();
    radius = 1.0f;
}

RegisteredValue::RegisteredValue(const RegisteredValue &source)
{
    *this = source;
    registerValue();
}

RegisteredValue::RegisteredValue(const Sphere &sphere, long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    center = sphere.center;
    radius = sphere.radius;
    registerValue();
}

RegisteredValue::RegisteredValue(const Point3d &point, long category)
{
    priority = category;
    flag7 = 1;
    flag6 = 1;
    center = point;
    registerValue();
    radius = 1.0f;
}

RegisteredValue &RegisteredValue::operator=(const RegisteredValue &source)
{
    center = source.center;
    radius = source.radius;
    priority = source.priority;
    flag7 = source.flag7;
    flag6 = source.flag6;
    return *this;
}

RegisteredValue &RegisteredValue::operator=(const Sphere &sphere)
{
    center = sphere.center;
    radius = sphere.radius;
    return *this;
}

RegisteredValue &RegisteredValue::operator=(const Point3d &point)
{
    center = point;
    return *this;
}

RegisteredValue::~RegisteredValue()
{
    unregisterValue();
}

void collectCategoryValues(std::vector<Sphere *> *out, long category)
{
    for (RegisteredValue *value = registeredValues; value; value = value->next) {
        if (value->flag7 && value->priority == category)
            out->push_back(value);
    }
}

void collectCategorySpheres(std::vector<Sphere *> *out, long category)
{
    for (RegisteredValue *value = registeredValues; value; value = value->next) {
        if (value->flag7 && value->flag6 && value->priority == category)
            out->push_back(value);
    }
}

void RegisteredValue::registerValue()
{
    next = registeredValues;
    registeredValues = this;
}

void RegisteredValue::unregisterValue()
{
    RegisteredValue **link = &registeredValues;
    while (*link != this) {
        link = &(*link)->next;
    }
    *link = next;
}

void RegisteredValue::setPriority(unsigned long category)
{
    priority = category;
}

void RegisteredValue::clearFlag7()
{
    flag7 = 0;
}

void RegisteredValue::setFlag7()
{
    flag7 = 1;
}

void RegisteredValue::clearFlag6()
{
    flag6 = 0;
}

void RegisteredValue::setFlag6()
{
    flag6 = 1;
}
