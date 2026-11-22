// Declares vector operations used by the moving actor and the computer controller.

#ifndef REL_M673DLL_MOVEMENT_VECTOR_H
#define REL_M673DLL_MOVEMENT_VECTOR_H
#include "dolphin/mtx.h"

// Stores the components of a movement vector.
struct MovementVector : Vec {
    // Leaves the vector components uninitialized.
    MovementVector()
    {
    }

    // Initializes the vector components from the supplied values.
    MovementVector(f64 x, f64 y, f64 z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    // Copies the supplied vector components.
    MovementVector &operator=(const Vec &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    // Copies the supplied vector components.
    MovementVector &operator=(const MovementVector &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    // Sets all three vector components.
    void set(f64 x, f64 y, f64 z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    // Returns the vector magnitude.
    f32 length() const
    {
        return PSVECMag(this);
    }

    // Adds the supplied vector in place.
    MovementVector &operator+=(const Vec &other)
    {
        PSVECAdd(&other, this, this);
        return *this;
    }

    // Normalizes this vector in place.
    MovementVector &normalize()
    {
        PSVECNormalize(this, this);
        return *this;
    }
};

// Multiplies the vector by the supplied scale in place.
inline Vec &operator*=(Vec &value, f32 scale)
{
    PSVECScale(&value, &value, scale);
    return value;
}

#endif
