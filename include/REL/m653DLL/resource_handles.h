#ifndef M653_RESOURCE_HANDLES_H
#define M653_RESOURCE_HANDLES_H
#include <memory>
#include "REL/framework/model.h"
struct ResourceVirtual {
    virtual ~ResourceVirtual() {}
    virtual void update() = 0;
};
template<class T> struct ResourcePointer {
    T *value;
    ResourcePointer(T *initial = 0) : value(initial) {}
    ~ResourcePointer() { delete value; }
    T *get() const { return value; }
    void reset(T *replacement) {
        if (value != replacement) { delete value; value = replacement; }
    }
};

#endif
