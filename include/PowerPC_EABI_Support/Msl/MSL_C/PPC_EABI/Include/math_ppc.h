#ifndef MP6_POWERPC_CLASSIFICATION
#define MP6_POWERPC_CLASSIFICATION
#ifdef __cplusplus
extern "C++" { namespace std {
#endif
/* Classification follows the exponent and fraction fields of this ABI. */
inline int __fpclassifyf(float input)
{
    switch (*(int *)&input & 0x7F800000) {
    case 0x7F800000:
        if (*(int *)&input & 0x007FFFFF) return FP_QNAN;
        return FP_INFINITE;
    case 0:
        if (*(int *)&input & 0x007FFFFF) return FP_SUBNORMAL;
        return FP_ZERO;
    }
    return FP_NORMAL;
}
inline int __fpclassifyd(double input)
{
    switch (*(int *)&input & 0x7FF00000) {
    case 0x7FF00000:
        if ((*(int *)&input & 0x000FFFFF) || *((int *)&input + 1)) return FP_QNAN;
        return FP_INFINITE;
    case 0:
        if ((*(int *)&input & 0x000FFFFF) || *((int *)&input + 1)) return FP_SUBNORMAL;
        return FP_ZERO;
    }
    return FP_NORMAL;
}
/* C callers inspect their expression directly; C++ also exposes functions. */
#define __signbitf(input) ((int)(*(unsigned long *)&(input) & 0x80000000))
#define __signbitd(input) ((int)(*(unsigned long *)&(input) & 0x80000000))
#ifdef __cplusplus
inline int (__signbitf)(float input) { return __signbitf(input); }
inline int (__signbitd)(double input) { return __signbitd(input); }
#undef __signbitf
#undef __signbitd
#endif
#ifdef __cplusplus
} }
#endif
#endif

/* Math headers also expose the architecture arithmetic operations. */
#if defined(MP6_C_MATH_H) || defined(MP6_PROJECT_MATH_H)
#ifndef MP6_REBUILT_POWERPC_MATH
#define MP6_REBUILT_POWERPC_MATH
#ifdef __cplusplus
extern "C++" { namespace std {
#endif
/* Double-precision absolute value uses the hardware intrinsic. */
inline double fabs(double input) { return __fabs(input); }

#ifdef MP6_C_MATH_H
/* The single-precision result is rounded after the absolute-value operation. */
float fabsf(float);
/* The float remainder uses the runtime's double-precision operation. */
inline float fmodf(float dividend, float divisor) { return fmod(dividend, divisor); }
#ifdef __cplusplus
/* Float overloads round the corresponding double-precision result. */
inline float abs(float input) { return fabsf(input); }
#endif
/* Single-precision interfaces round the double-precision runtime result. */
inline float asinf(float input) { return asin(input); }

#endif
/* Refine the reciprocal estimate before converting it to a square root. */
extern inline double sqrt(double input)
{
    if (input > 0.0) {
        double estimate = __frsqrte(input);
        estimate = 0.5 * estimate * (3.0 - estimate * estimate * input);
        estimate = 0.5 * estimate * (3.0 - estimate * estimate * input);
        estimate = 0.5 * estimate * (3.0 - estimate * estimate * input);
        estimate = 0.5 * estimate * (3.0 - estimate * estimate * input);
        return input * estimate;
    } else if (input == 0) {
        return 0;
    } else if (input) {
        return NAN;
    }
    return HUGE_VALF;
}

#ifdef MP6_C_MATH_H
/* In C units the objects name this function and its local statics as C++ does;
 * only this language region reproduces those names. */
#pragma cplusplus on
/* These constant names are present in the oracle object's symbol table. */
extern inline float sqrtf(float input)
{
    static const double _half = 0.5;
    static const double _three = 3.0;
    /* The objects store the rounded result and reload it before returning. */
    volatile float rounded;
    if (input > 0.0f) {
        double estimate = __frsqrte((double)input);
        estimate = _half * estimate * (_three - estimate * estimate * input);
        estimate = _half * estimate * (_three - estimate * estimate * input);
        estimate = _half * estimate * (_three - estimate * estimate * input);
        rounded = (float)(input * estimate);
        return rounded;
    }
    return input;
}
#pragma cplusplus reset

#endif
#ifdef __cplusplus
} }
#endif
#endif
#endif
