#include <vector>
#include "REL/m648DLL/registered_value.h"

extern "C" {
Sphere *lbl_1_bss_618;
}

Sphere::Sphere(long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    registerValue();
    w=1.0f;
}

Sphere::Sphere(const Sphere& source)
{
    *this=source;
    registerValue();
}

Sphere::Sphere(const Float4& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z; w=source.w;
    registerValue();
}

Sphere::Sphere(const Float3& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z;
    registerValue();
    w=1.0f;
}

Sphere& Sphere::operator=(const Sphere& source)
{
    x=source.x; y=source.y; z=source.z; w=source.w;
    priority=source.priority;
    flag7=source.flag7;
    flag6=source.flag6;
    return *this;
}

Float4 *Float4::copyFrom(const Float4 *source)
{
    x = source->x; y = source->y;
    z = source->z; w = source->w;
    return this;
}

Float3 *fn_1_131B8(Float3 *dest, const Float3 *source)
{
    dest->x = source->x; dest->y = source->y; dest->z = source->z;
    return dest;
}

Sphere::~Sphere() { unregisterValue(); }

extern "C" {
void fn_1_13228(std::vector<Sphere *> *out, long category);
}
/* Framework scene_shadow.cpp's SceneShadow::update gathers each selected category's spheres here. */
void collectCategorySpheres(std::vector<Sphere *> *out, long category);

void fn_1_13228(std::vector<Sphere *> *out, long category)
{
    for (Sphere *value = lbl_1_bss_618; value; value = value->next) {
        if (value->flag7 && value->priority == category)
            out->push_back(value);
    }
}

void collectCategorySpheres(std::vector<Sphere *> *out, long category)
{
    for (Sphere *value = lbl_1_bss_618; value; value = value->next) {
        if (value->flag7 && value->flag6 && value->priority == category)
            out->push_back(value);
    }
}

void Sphere::registerValue()
{
    next = lbl_1_bss_618;
    lbl_1_bss_618 = this;
}

void Sphere::unregisterValue()
{
    Sphere **cursor = &lbl_1_bss_618;
    while (*cursor != this) { cursor = &(*cursor)->next; }
    *cursor = next;
}

void Sphere::setPriority(unsigned long value) { priority = value; }

void Sphere::clearFlag7() { flag7 = 0; }

void Sphere::setFlag7() { flag7 = 1; }

void Sphere::clearFlag6() { flag6 = 0; }

void Sphere::setFlag6() { flag6 = 1; }
