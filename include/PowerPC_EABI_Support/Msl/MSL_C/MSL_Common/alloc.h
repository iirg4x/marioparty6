#ifndef _MSL_COMMON_ALLOC_H
#define _MSL_COMMON_ALLOC_H
#include "ansi_files.h"
#ifdef __cplusplus
extern "C" {
#endif
void free(void *);
void *malloc(size_t);
#ifdef __cplusplus
}
#endif
#endif
