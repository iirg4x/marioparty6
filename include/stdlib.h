#ifndef _STDLIB_H
#define _STDLIB_H

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/size_t.h"

#define abs(x) __abs(x)

#ifdef __cplusplus
extern "C" {
#endif
/* Sorts elements in place using the supplied comparator. */
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
#ifdef __cplusplus
}
#endif

#endif
