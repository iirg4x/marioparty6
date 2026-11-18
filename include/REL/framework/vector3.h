// The vector type collision code and the minigames share.
#ifndef REL_FRAMEWORK_VECTOR3_H
#define REL_FRAMEWORK_VECTOR3_H
extern "C" {
#include "dolphin/mtx.h"
// The build defines NDEBUG for every object; the original kept these asserts.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
}

// A position or direction with the vector operations collision code uses.
struct Vector3 : Vec {
    // Adds the other vector to this vector.
    Vector3 &operator+=(const Vector3 &other)
    {
        PSVECAdd(this, &other, this);
        return *this;
    }
    // Subtracts the other vector from this vector.
    Vector3 &operator-=(const Vector3 &other)
    {
        PSVECSubtract(this, &other, this);
        return *this;
    }
    // Scales this vector by the supplied scalar.
    template<class real>
    Vector3 &operator*=(real s)
    {
        PSVECScale(this, this, s);
        return *this;
    }
    // Writes source divided by a scalar into result; the scalar must not be zero.
    template<class real>
    static Vec &divide(const Vec &source, Vec &result, real s)
    {
        assert(s != real(0));
        PSVECScale(&source, &result, real(1) / s);
        return result;
    }

    // Divides this vector by a scalar in place.
    template<class real>
    Vector3 &operator/=(real s)
    {
        divide(*this, *this, s);
        return *this;
    }

    // Returns the dot product.
    float dot(const Vector3 &other) const
    {
        return PSVECDotProduct(this, &other);
    }
    // Returns this vector as an SDK vector pointer.
    operator const Vec *() const
    {
        const Vec *const vector = this;
        return vector;
    }
    // Returns the vector magnitude.
    float length() const
    {
        return PSVECMag(this);
    }
};

// Returns the vector scaled by the supplied scalar.
template<class real>
inline Vector3 operator*(const Vector3 &a, real b)
{
    Vector3 result;
    PSVECScale(&a, &result, b);
    return result;
}

// Returns the vector scaled by the reciprocal of a nonzero divisor. m610 also divides its
// ProjectionVector, derived from Vector3, so the operand type is a parameter.
// The original's asserts read "b != real(0)" on line 83 and "s != real(0)" on line 36 (divide),
// so the scalar parameter keeps the name real and neither assert may move.
template<class Vector, class real>
inline Vector operator/(const Vector &a, real b)
{
    assert(b != real(0));
    Vector result;
    PSVECScale(&a, &result, real(1) / b);
    return result;
}

// Returns the sum of two vectors.
inline Vector3 operator+(const Vector3 &a, const Vector3 &b)
{
    Vector3 result;
    PSVECAdd(&a, &b, &result);
    return result;
}

// Returns the difference of two vectors.
inline Vector3 operator-(const Vector3 &a, const Vector3 &b)
{
    Vector3 result;
    PSVECSubtract(&a, &b, &result);
    return result;
}

#endif
