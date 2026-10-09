/* Classifies floating-point values used by the minigame's math helpers. */
#ifndef M646_PRIVATE_FLOAT_CLASSIFICATION_H
#define M646_PRIVATE_FLOAT_CLASSIFICATION_H
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#define FP_SNAN      0
#define FP_QNAN      1
#define FP_INFINITE  2
#define FP_ZERO      3
#define FP_NORMAL    4
#define FP_SUBNORMAL 5

#define FP_NAN FP_QNAN

#define fpclassify(x)                                                          \
	((sizeof(x) == sizeof(float)) ? __fpclassifyf(x) : __fpclassifyd(x))
#define signbit(x)                                                             \
	((sizeof(x) == sizeof(float)) ? __signbitf(x) : __signbitd(x))
#define isfinite(x) ((fpclassify(x) > 2))
#define isnan(x)    ((fpclassify(x) == FP_NAN))
#define isinf(x)    ((fpclassify(x) == FP_INFINITE))

#define M646_FLOAT_SIGN_MASK 0x80000000
#define M646_FLOAT_EXPONENT_MASK 0x7F800000
#define M646_FLOAT_FRACTION_MASK 0x007FFFFF
#define M646_DOUBLE_EXPONENT_MASK 0x7FF00000
#define M646_DOUBLE_HIGH_FRACTION_MASK 0x000FFFFF
#define M646_DOUBLE_LOW_FRACTION_MASK 0xFFFFFFFF

#define __signbitf(x) ((int)(__HI(x) & M646_FLOAT_SIGN_MASK))

#define __signbitd(x) ((int)(__HI(x) & M646_FLOAT_SIGN_MASK))
/* Math predicates classify a float by its exponent and fraction; all NaNs return FP_QNAN. */
inline int __fpclassifyf(float value)
{
	switch (__HI(value) & M646_FLOAT_EXPONENT_MASK) {
	case M646_FLOAT_EXPONENT_MASK:
		if ((__HI(value) & M646_FLOAT_FRACTION_MASK) != 0) {
			return FP_QNAN;
		}
		return FP_INFINITE;

	case 0:
		if ((__HI(value) & M646_FLOAT_FRACTION_MASK) != 0) {
			return FP_SUBNORMAL;
		}
		return FP_ZERO;
	}

	return FP_NORMAL;
}

/* Double-precision math predicates inspect both fraction words; all NaNs return FP_QNAN. */
inline int __fpclassifyd(double value)
{
	switch (__HI(value) & M646_DOUBLE_EXPONENT_MASK) {
	case M646_DOUBLE_EXPONENT_MASK: {
            if ((__HI(value) & M646_DOUBLE_HIGH_FRACTION_MASK) ||
                (__LO(value) & M646_DOUBLE_LOW_FRACTION_MASK))
                return FP_QNAN;
            else
                return FP_INFINITE;
            break;
	}
	case 0: {
            if ((__HI(value) & M646_DOUBLE_HIGH_FRACTION_MASK) ||
                (__LO(value) & M646_DOUBLE_LOW_FRACTION_MASK))
                return FP_SUBNORMAL;
            else
                return FP_ZERO;
            break;
	}
	}
	return FP_NORMAL;
}

#endif
