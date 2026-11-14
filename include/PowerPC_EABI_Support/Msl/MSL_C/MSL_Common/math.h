#ifndef MP6_PROJECT_MATH_H
#define MP6_PROJECT_MATH_H
/* The compiled objects math_ppc.o and board/pause.o use runtime arithmetic. */
#define MP6_MATH_INTERFACE 1
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/cmath"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/float.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/PPC_EABI/Include/math_ppc.h"
#ifndef M_PI
#define M_PI 3.1415927f
#endif
/* Single-precision approximation used for the square root of three. */
#define M_SQRT3 1.73205f
#undef MP6_MATH_INTERFACE
#endif
