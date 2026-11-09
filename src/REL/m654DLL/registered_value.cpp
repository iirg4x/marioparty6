#include <vector>

#include "REL/m654DLL/registered_value.h"

extern "C" {
RegisteredValue *lbl_1_bss_F0;
void fn_1_ED78(std::vector<RegisteredValue *> *, long);

}

RegisteredValue::RegisteredValue(long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    fn_1_EF10();
    radius=1.0f;
}

RegisteredValue::RegisteredValue(const RegisteredValue& source)
{
    *this=source;
    fn_1_EF10();
}

RegisteredValue::RegisteredValue(const Float4& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    center.x=source.x; center.y=source.y; center.z=source.z; radius=source.w;
    fn_1_EF10();
}

RegisteredValue::RegisteredValue(const Float3& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    center.x=source.x; center.y=source.y; center.z=source.z;
    fn_1_EF10();
    radius=1.0f;
}

RegisteredValue& RegisteredValue::operator=(const RegisteredValue& source)
{
    center.x=source.center.x; center.y=source.center.y; center.z=source.center.z; radius=source.radius;
    priority=source.priority;
    flag7=source.flag7;
    flag6=source.flag6;
    return *this;
}

Float4 *Float4::fn_1_ECE4(const Float4 *source)
{
    x = source->x; y = source->y;
    z = source->z; w = source->w;
    return this;
}

Float3 *Float3::fn_1_ED08(const Float3 *source)
{
    x = source->x; y = source->y; z = source->z;
    return this;
}

RegisteredValue::~RegisteredValue() { fn_1_EF30(); }

void fn_1_ED78(std::vector<RegisteredValue *> *out, long category)
{
    for (RegisteredValue *value = lbl_1_bss_F0; value; value = value->next) {
        if (value->flag7 && value->priority == category)
            out->push_back(value);
    }
}

void collectCategorySpheres(std::vector<Sphere *> *out, long category)
{
    for (RegisteredValue *value = lbl_1_bss_F0; value; value = value->next) {
        if (value->flag7 && value->flag6 && value->priority == category)
            out->push_back(value);
    }
}

void RegisteredValue::fn_1_EF10()
{
    next = lbl_1_bss_F0;
    lbl_1_bss_F0 = this;
}

void RegisteredValue::fn_1_EF30()
{
    RegisteredValue **cursor = &lbl_1_bss_F0;
    while (*cursor != this) { cursor = &(*cursor)->next; }
    *cursor = next;
}

void RegisteredValue::fn_1_EF6C(unsigned long value) { priority = value; }

void RegisteredValue::fn_1_EF7C() { flag7 = 0; }

void RegisteredValue::fn_1_EF90() { flag7 = 1; }

void RegisteredValue::fn_1_EFA4() { flag6 = 0; }

void RegisteredValue::fn_1_EFB8() { flag6 = 1; }
