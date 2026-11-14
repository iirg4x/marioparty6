#ifndef MP6_C_FLOAT_H
#define MP6_C_FLOAT_H
#ifdef __cplusplus
extern "C" {
#endif
extern int __double_huge[];
/* Floating-point constants use the runtime value tables. */
#ifndef NAN
extern unsigned long __float_nan[];
#define NAN (*(float*)__float_nan)
#endif
#ifndef INFINITY
extern unsigned long __float_huge[];
#define INFINITY (*(float*)__float_huge)
#endif
extern long double __extended_huge[];
extern double __double_min[], __double_max[], __double_epsilon[];
extern long double __extended_min[], __extended_max[], __extended_epsilon[];
#ifdef __cplusplus
}
#endif
#ifndef HUGE_VAL
#define HUGE_VAL (*(double*)__double_huge)
#endif
#ifndef HUGE_VALF
#define HUGE_VALF INFINITY
#endif
#ifndef HUGE_VALL
#define HUGE_VALL (__extended_huge[0])
#endif
#ifdef __cplusplus
namespace std {
using ::__double_min;
using ::__double_max;
using ::__double_epsilon;
using ::__double_huge;
using ::__float_huge;
using ::__float_nan;
using ::__extended_min;
using ::__extended_max;
using ::__extended_epsilon;
using ::__extended_huge;
}
#endif
/* IEEE binary32 and binary64 limits for the hardware floating-point ABI. */
#define FLT_RADIX 2
#define FLT_ROUNDS 1
#define FLT_MANT_DIG 24
#define FLT_DIG 6
#define FLT_MIN_EXP (-125)
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_EXP 128
#define FLT_MAX_10_EXP 38
#define FLT_MAX 3.4028234663852886e+38F
#define FLT_MIN 1.1754944e-38f
#define FLT_EPSILON 1.1920929e-7f
#define DBL_MANT_DIG 53
#define DBL_DIG 15
#define DBL_MIN_EXP (-1021)
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_EXP 1024
#define DBL_MAX_10_EXP 308
#define DBL_MAX (__double_max[0])
#define DBL_MIN (__double_min[0])
#define DBL_EPSILON (__double_epsilon[0])
#define LDBL_MANT_DIG DBL_MANT_DIG
#define LDBL_DIG DBL_DIG
#define LDBL_MIN_EXP DBL_MIN_EXP
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#define LDBL_MAX_EXP DBL_MAX_EXP
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP
#define LDBL_MAX (__extended_max[0])
#define LDBL_MIN (__extended_min[0])
#define LDBL_EPSILON (__extended_epsilon[0])
#define FP_SNAN 0
#define FP_QNAN 1
#define FP_INFINITE 2
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 5
#define FP_NAN FP_QNAN
#define fpclassify(value) \
    (sizeof(value) == sizeof(float) ? __fpclassifyf(value) : __fpclassifyd(value))
#define signbit(value) (sizeof(value) == sizeof(float) ? __signbitf(value) : __signbitd(value))
#define isfinite(value) (fpclassify(value) > 2)
#define isnan(value) (fpclassify(value) == FP_NAN)
#define isinf(value) (fpclassify(value) == FP_INFINITE)

#ifdef __cplusplus
namespace std {
int __fpclassifyf(float);
int __fpclassifyd(double);
int __signbitf(float);
int __signbitd(double);
}
using std::__fpclassifyf;
using std::__fpclassifyd;
using std::__signbitf;
using std::__signbitd;
#else
int __fpclassifyf(float);
int __fpclassifyd(double);
int __signbitf(float);
int __signbitd(double);
#endif
#include "PowerPC_EABI_Support/Msl/MSL_C/PPC_EABI/Include/math_ppc.h"
#endif
