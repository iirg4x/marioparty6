#include <vector>

class Float4 {
public:
    float x,y,z,w;
    Float4 *fn_1_11398(const Float4 *source);
};

class Float3 {
public:
    float x,y,z;
    Float3 *fn_1_113BC(const Float3 *source);
};

class Sphere {
public:
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
    void SetPriority(unsigned long);
    void ClearFlag7();
    void SetFlag7();
    void ClearFlag6();
    void SetFlag6();
    void fn_1_115C4();
    void fn_1_115E4();
};

extern "C" {  }

Sphere::Sphere(long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    fn_1_115C4();
    w=1.0f;
}

Sphere::Sphere(const Sphere& source)
{
    *this=source;
    fn_1_115C4();
}

Sphere::Sphere(const Float4& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z; w=source.w;
    fn_1_115C4();
}

Sphere::Sphere(const Float3& source,long category)
{
    priority=category;
    flag7=1;
    flag6=1;
    x=source.x; y=source.y; z=source.z;
    fn_1_115C4();
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

Float4 *Float4::fn_1_11398(const Float4 *source)
{
    x = source->x; y = source->y;
    z = source->z; w = source->w;
    return this;
}

Float3 *Float3::fn_1_113BC(const Float3 *source)
{
    x = source->x; y = source->y; z = source->z;
    return this;
}

Sphere::~Sphere() { fn_1_115E4(); }

extern "C" { Sphere *lbl_1_bss_A88; }

extern "C" {
void fn_1_1142C(std::vector<Sphere *> *out,long category);
}
/* Framework scene_shadow.cpp's SceneShadow::update gathers each selected category's spheres here. */
void collectCategorySpheres(std::vector<Sphere *> *out,long category);

void fn_1_1142C(std::vector<Sphere *> *out,long category)
{
    for(Sphere *value=lbl_1_bss_A88;value;value=value->next) {
        if(value->flag7 && value->priority==category)
            out->push_back(value);
    }
}

void collectCategorySpheres(std::vector<Sphere *> *out,long category)
{
    for(Sphere *value=lbl_1_bss_A88;value;value=value->next) {
        if(value->flag7 && value->flag6 && value->priority==category)
            out->push_back(value);
    }
}

void Sphere::fn_1_115C4()
{
    next = lbl_1_bss_A88;
    lbl_1_bss_A88 = this;
}

void Sphere::fn_1_115E4()
{
    Sphere **cursor = &lbl_1_bss_A88;
    while (*cursor != this) cursor = &(*cursor)->next;
    *cursor = next;
}

void Sphere::SetPriority(unsigned long value)
{
    priority = value;
}

void Sphere::ClearFlag7() { flag7 = 0; }

void Sphere::SetFlag7() { flag7 = 1; }

void Sphere::ClearFlag6() { flag6 = 0; }

void Sphere::SetFlag6() { flag6 = 1; }
