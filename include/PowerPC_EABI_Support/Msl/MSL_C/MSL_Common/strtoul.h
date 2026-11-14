#ifndef _MSL_COMMON_STRTOUL_H
#define _MSL_COMMON_STRTOUL_H

#include "Include/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long __strtoul(int base, int max_width,
                        int (*ReadProc)(void*, int, int), void* ReadProcArg,
                        int* chars_scanned, int* negative, int* overflow);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_STRTOUL_H */
