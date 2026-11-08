#ifndef M648_REGISTERED_VALUE_H
#define M648_REGISTERED_VALUE_H

struct Float4 { float x,y,z,w; Float4 *copyFrom(const Float4 *source); };
struct Float3 { float x,y,z; };

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
    void clearFlag7();
    void setFlag7();
    void clearFlag6();
    void setFlag6();
    void setPriority(unsigned long);
    void registerValue();
    void unregisterValue();
};

#endif
