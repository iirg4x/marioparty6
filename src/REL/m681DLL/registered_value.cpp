#include <vector>
extern "C" {
#include "game/charman.h"
}

#include "REL/m681DLL/character_motion.h"
#include "REL/m681DLL/character_sequence.h"


struct Float4 { float x,y,z,w; };

struct Float3 { float x,y,z; };

struct Sphere {
    float x,y,z,w;
    long priority : 30;
    unsigned long unknown_10_low : 2;
    unsigned char flag7 : 1;
    unsigned char flag6 : 1;
    unsigned char unknown_14_low : 6;
    Sphere *next;
    Sphere(long);
    Sphere(const Sphere&);
    Sphere(const Float4&,long);
    Sphere(const Float3&,long);
    Sphere& operator=(const Sphere&);
    ~Sphere();
};

extern "C" {}
 void fn_1_D920(Sphere *);
extern "C" { }

Sphere::Sphere(long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    fn_1_D920(this);
    w=1.0f;
}

Sphere::Sphere(const Sphere& source)
{
    *this=source;
    fn_1_D920(this);
}

Sphere::Sphere(const Float4& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z; w=source.w;
    fn_1_D920(this);
}

Sphere::Sphere(const Float3& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z;
    fn_1_D920(this);
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


/* Partial source-selected groups; not claimed original TU boundaries.
 * Selectors include complete definitions and never alter function bodies. */


extern "C" {
extern Sphere *lbl_1_bss_1F8;


}
Float4 *fn_1_D6F4(Float4 *dest, const Float4 *source)
{
    dest->x = source->x; dest->y = source->y;
    dest->z = source->z; dest->w = source->w;
    return dest;
}
extern "C" {

}
Float3 *fn_1_D718(Float3 *dest, const Float3 *source)
{
    dest->x = source->x; dest->y = source->y; dest->z = source->z;
    return dest;
}
extern "C" {


}


extern "C" { void fn_1_27698(Sphere *); }

extern "C" {}
 void fn_1_D940(Sphere *);
extern "C" { }


extern "C" { extern Sphere *lbl_1_bss_1F8; }

Sphere::~Sphere() { fn_1_D940(this); }

typedef std::vector<Sphere *> RegisteredArray;

extern "C" { void fn_1_276B8(Sphere *); }

extern "C" { extern Sphere *lbl_1_bss_D0; }


extern "C" {
}
void fn_1_D788(RegisteredArray *out,long category)
{
    for(Sphere *value=lbl_1_bss_D0;value;value=value->next) {
        if(value->flag7 && value->priority==category)
            out->push_back(value);
    }
}
extern "C" {

}

void collectCategorySpheres(RegisteredArray *out,long category)
{
    for(Sphere *value=lbl_1_bss_D0;value;value=value->next) {
        if(value->flag7 && value->flag6 && value->priority==category)
            out->push_back(value);
    }
}


extern "C" {
extern Sphere *lbl_1_bss_D0;


}
void fn_1_D920(Sphere *object)
{
    object->next = lbl_1_bss_D0;
    lbl_1_bss_D0 = object;
}
extern "C" {

}
void fn_1_D940(Sphere *object)
{
    Sphere **cursor = &lbl_1_bss_D0;
    while (*cursor != object) { cursor = &(*cursor)->next; }
    *cursor = object->next;
}
extern "C" {

}
void fn_1_D97C(Sphere *object, unsigned long priority) { object->priority = priority; }
extern "C" {

}
void fn_1_D98C(Sphere *object) { object->flag7 = 0; }
extern "C" {

}
void fn_1_D9A0(Sphere *object) { object->flag7 = 1; }
extern "C" {

}
void fn_1_D9B4(Sphere *object) { object->flag6 = 0; }
extern "C" {

}
void fn_1_D9C8(Sphere *object) { object->flag6 = 1; }
extern "C" {


}


extern "C" { Sphere *lbl_1_bss_D0; }
