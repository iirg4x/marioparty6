#ifndef _MSL_COMMON_DIRECT_IO_H
#define _MSL_COMMON_DIRECT_IO_H

#include "ansi_files.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Write complete elements to the stream's internal buffer. */
size_t __fwrite(const void* buffer, size_t size, size_t count, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_DIRECT_IO_H */
