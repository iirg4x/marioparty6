#ifndef M654DLL_REGISTERED_VALUE_H
#define M654DLL_REGISTERED_VALUE_H
#include <vector>
// A value on the scene's registered list, kept by category. Copies join the list too.

class Float4 {
public:
    float x, y, z, w;
    Float4 *fn_1_ECE4(const Float4 *source);
};

class Float3 {
public:
    float x, y, z;
    Float3 *fn_1_ED08(const Float3 *source);
};

#include "dolphin/mtx.h"
struct Sphere {
    Point3d center;
    f32 radius;
    f32 getRadius() const { return radius; }
    void translate(Point3d *offset) { PSVECAdd(offset, &center, &center); }
};

class RegisteredValue : public Sphere {
public:
    long priority : 30;
    unsigned long unknown_10_low : 2;
    unsigned char flag7 : 1;
    unsigned char flag6 : 1;
    unsigned char unknown_14_low : 6;
    RegisteredValue *next;

    RegisteredValue(long category = 0);
    RegisteredValue(const RegisteredValue &source);
    RegisteredValue(const Float4 &source, long category = 0);
    RegisteredValue(const Float3 &source, long category = 0);
    RegisteredValue &operator=(const RegisteredValue &source);
    ~RegisteredValue();
    void fn_1_EF10();
    void fn_1_EF30();
    void fn_1_EF6C(unsigned long value);
    void fn_1_EF7C();
    void fn_1_EF90();
    void fn_1_EFA4();
    void fn_1_EFB8();
};

void collectCategorySpheres(std::vector<Sphere *> *, long);

#endif
