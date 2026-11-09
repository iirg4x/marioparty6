#include "REL/m652DLL/float3.h"
#include <vector>
#include "REL/m652DLL/float4.h"
#include "REL/m652DLL/registered_value.h"

Sphere::Sphere(long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    fn_1_DC70();
    w=1.0f;
}

Sphere::Sphere(const Sphere& source)
{
    *this=source;
    fn_1_DC70();
}

Sphere::Sphere(const Float4& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z; w=source.w;
    fn_1_DC70();
}

Sphere::Sphere(const Float3& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z;
    fn_1_DC70();
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

Float4 *Float4::fn_1_DA44(const Float4 *source)
{
    x = source->x; y = source->y;
    z = source->z; w = source->w;
    return this;
}

Float4 *Float4::fn_1_DA68(const Vec *source)
{
    x = source->x; y = source->y; z = source->z;
    return this;
}

Sphere::~Sphere() { fn_1_DC90(); }

extern "C" { Sphere *lbl_1_bss_D0; }

extern "C" void fn_1_DAD8(std::vector<Sphere *> *,long);

void fn_1_DAD8(std::vector<Sphere *> *out,long category)
{
    for(Sphere *value=lbl_1_bss_D0;value;value=value->next) {
        if(value->flag7 && value->priority==category)
            out->push_back(value);
    }
}

void collectCategorySpheres(std::vector<Sphere *> *out,long category)
{
    for(Sphere *value=lbl_1_bss_D0;value;value=value->next) {
        if(value->flag7 && value->flag6 && value->priority==category)
            out->push_back(value);
    }
}

void Sphere::fn_1_DC70()
{
    next = lbl_1_bss_D0;
    lbl_1_bss_D0 = this;
}

void Sphere::fn_1_DC90()
{
    Sphere **cursor = &lbl_1_bss_D0;
    while (*cursor != this) { cursor = &(*cursor)->next; }
    *cursor = next;
}

void Sphere::fn_1_DCCC(unsigned long priority) { this->priority = priority; }

void Sphere::fn_1_DCDC() { flag7 = 0; }

void Sphere::fn_1_DCF0() { flag7 = 1; }

void Sphere::fn_1_DD04() { flag6 = 0; }

void Sphere::fn_1_DD18() { flag6 = 1; }
