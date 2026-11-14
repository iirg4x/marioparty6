#ifndef MP6_MATH_INTERFACE
#define MP6_MATH_INTERFACE 3
#endif
#ifndef MP6_C_MATH_H
#define MP6_C_MATH_H
#include "cmath"
#include "float.h"
#include "math_ppc.h"
#ifdef __cplusplus
extern "C++" {
    namespace std {
#endif
/* Float interfaces round the double-precision runtime result. */
inline float acosf(float input) { return acos(input); }
inline float atanf(float input) { return atan(input); }
inline float atan2f(float y, float x) { return (float)atan2(y, x); }
inline float sinf(float input) { return sin(input); }
inline float cosf(float input) { return cos(input); }
inline float tanf(float input) { return tan(input); }
inline float expf(float input) { return exp(input); }
inline float logf(float input) { return log(input); }
inline float powf(float base, float exponent) { return pow(base, exponent); }
inline float ceilf(float input) { return ceil(input); }
inline float floorf(float input) { return floor(input); }
/* The single-precision result is rounded after the absolute-value operation. */
inline float fabsf(float input) { return fabs(input); }
#ifdef __cplusplus
} }
#endif
#endif
#if defined(__cplusplus) && MP6_MATH_INTERFACE == 3
using std::acos;
using std::asin;
using std::atan;
using std::atan2;
using std::copysign;
using std::cos;
using std::sin;
using std::tan;
using std::cosh;
using std::sinh;
using std::tanh;
using std::exp;
using std::frexp;
using std::ldexp;
using std::log;
using std::log10;
using std::modf;
using std::pow;
using std::sqrt;
using std::ceil;
using std::fabs;
using std::floor;
using std::fmod;
using std::acosf;
using std::asinf;
using std::atanf;
using std::atan2f;
using std::cosf;
using std::sinf;
using std::tanf;
using std::expf;
using std::logf;
using std::powf;
using std::ceilf;
using std::fabsf;
using std::floorf;
using std::fmodf;
using std::sqrtf;
using std::__fpclassifyf;
using std::__fpclassifyd;
using std::__signbitf;
using std::__signbitd;
#endif
#undef MP6_MATH_INTERFACE
