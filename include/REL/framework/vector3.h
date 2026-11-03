// The vector type collision code and the minigames share.
#ifndef REL_FRAMEWORK_VECTOR3_H
#define REL_FRAMEWORK_VECTOR3_H

extern "C" {
#include "dolphin/mtx.h"
}

// A position or direction with the vector operations collision code uses.
struct Vector3 : Vec {
    float dot(const Vector3 &other) const {
        return PSVECDotProduct(this, &other);
    }
    operator const Vec *() const {
        const Vec *const vector = this;
        return vector;
    }
    float length() const { return PSVECMag(this); }
};

#endif
