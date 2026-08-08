#ifndef _MSL_COMMON_ALLOC_H
#define _MSL_COMMON_ALLOC_H

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/ansi_files.h"

#ifdef __cplusplus
extern "C" {
#endif

void free(void* ptr);
void* malloc(size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_ALLOC_H */
