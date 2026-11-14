#ifndef _MSL_COMMON_DIRECT_IO_H
#define _MSL_COMMON_DIRECT_IO_H

#include "ansi_files.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t __fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
/* Locate the final matching byte within a bounded memory range. */
unsigned char* __memrchr(unsigned char* buffer, int value, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_DIRECT_IO_H */
