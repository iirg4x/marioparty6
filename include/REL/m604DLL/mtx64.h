// Double-precision vectors: the counterpart of the SDK's mtx.h vector functions (PSVECNormalize, PSVECScale,
// PSVECMag, PSVECCrossProduct, PSVECDotProduct, PSVECSubtract, PSVECAdd) for a vector of T.
//
//
//
//
#ifndef REL_M604DLL_MTX64_H
#define REL_M604DLL_MTX64_H

#undef NDEBUG // the build defines NDEBUG for every object; this assert is compiled in
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
extern "C" {
#include "dolphin/mtx.h"
#include "math.h"
}

template <class T> struct Vec64 {
    T x;
    T y;
    T z;

    // dst = this scaled to length 1.
    void normalize(Vec64 *dst) const;
    // dst = this * scale.
    Vec64 *scale(Vec64 *dst, T scale) const;
    // dst = this * scale, for a vector that may change.
    Vec64 *scale(Vec64 *dst, T scale);
    // The length.
    T mag() const;
    // The length squared.
    T squareMag() const;
    // dst = this x other.
    Vec64 *cross(const Vec64 &other, Vec64 *dst) const;
    // this . other.
    T dot(const Vec64 &other) const;
};

// dst = a + b.
template <class T> Vec64<T> *add(const Vec *a, const Vec64<T> *b, Vec64<T> *dst);
// dst = a - b, in T.
template <class T> Vec64<T> *subtract(const Vec *a, const Vec *b, Vec64<T> *dst);
// a . b, with a single-precision b.
template <class T> T dot(const Vec64<T> &a, const Vec &b);
// a . b.
template <class T> T dot(const Vec64<T> &a, const Vec64<T> &b);

template <class T> Vec64<T> *Vec64<T>::scale(Vec64 *dst, T scale) const
{
    dst->x = x * scale;
    dst->y = y * scale;
    dst->z = z * scale;
    return dst;
}

// The mutable entry point reads this object through the const operation.
// Both overloads write the scaled components through dst.
// dst may equal this: each component is read before its own store.
template <class T> Vec64<T> *Vec64<T>::scale(Vec64 *dst, T scale)
{
    return static_cast<const Vec64 *>(this)->scale(dst, scale);
}

template <class T> T Vec64<T>::mag() const
{
    return sqrt(squareMag());
}

template <class T> T Vec64<T>::squareMag() const
{
    return dot(*this);
}

// The components in the SDK's order: y z - z y, z x - x z, x y - y x.
template <class T> Vec64<T> *Vec64<T>::cross(const Vec64 &other, Vec64 *dst) const
{
    dst->x = y * other.z - z * other.y;
    dst->y = z * other.x - x * other.z;
    dst->z = x * other.y - y * other.x;
    return dst;
}

template <class T> T Vec64<T>::dot(const Vec64 &other) const
{
    return x * other.x + y * other.y + z * other.z;
}

// The single-precision a widened to T component by component.
template <class T> Vec64<T> *add(const Vec *a, const Vec64<T> *b, Vec64<T> *dst)
{
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
    return dst;
}

// Each component widened to T before the subtraction, so the difference keeps T's precision.
template <class T> Vec64<T> *subtract(const Vec *a, const Vec *b, Vec64<T> *dst)
{
    dst->x = (T)a->x - (T)b->x;
    dst->y = (T)a->y - (T)b->y;
    dst->z = (T)a->z - (T)b->z;
    return dst;
}

template <class T> T dot(const Vec64<T> &a, const Vec &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T> T dot(const Vec64<T> &a, const Vec64<T> &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Defined after the templates it calls; asserts the vector is not zero.
template <class T> void Vec64<T>::normalize(Vec64 *dst) const
{
    T mag = this->mag();
    assert(mag != 0.0);
    scale(dst, 1.0 / mag);
}

// The single-precision forms: the SDK's functions, returning dst so that calls chain.
inline Vec *add(const Vec *a, const Vec *b, Vec *dst)
{
    PSVECAdd(a, b, dst);
    return dst;
}

inline Vec *subtract(const Vec *a, const Vec *b, Vec *dst)
{
    PSVECSubtract(a, b, dst);
    return dst;
}

inline Vec *scale(const Vec *src, Vec *dst, f32 scale)
{
    PSVECScale(src, dst, scale);
    return dst;
}

#endif
