#ifndef M652_REGISTERED_VALUE_H
#define M652_REGISTERED_VALUE_H

#include "REL/m652DLL/float4.h"
struct Float3;

struct Sphere : Float4 {
public:
    long priority : 30;
    unsigned long unknownPriorityBits : 2;
    unsigned char flag7 : 1;
    unsigned char flag6 : 1;
    unsigned char unknownFlagBits : 6;
    Sphere *next;
    Sphere(long);
    Sphere(const Sphere&);
    Sphere(const Float4&, long);
    Sphere(const Float3&, long);
    Sphere& operator=(const Sphere&);
    ~Sphere();
    void fn_1_DC70();
    void fn_1_DC90();
    void fn_1_DCCC(unsigned long priority);
    void fn_1_DCDC();
    void fn_1_DCF0();
    void fn_1_DD04();
    void fn_1_DD18();
};

#endif
