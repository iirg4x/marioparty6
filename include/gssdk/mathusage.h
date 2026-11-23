/* Checked logarithm used by the voice SDK. */
#ifndef GSSDK_MATHUSAGE_H
#define GSSDK_MATHUSAGE_H

#include "types.h"

/* Returns negative infinity for zero and the natural logarithm otherwise. */
f32 logf_check(f32 x);

/* Returns half the natural log of one plus the exponential of twice the input. */
f32 HLnOnePlusExpHFloat(f32 value);

/* Adds two quantities represented as log amplitudes. */
f32 LogAdd(f32 value1, f32 value2);

#endif
