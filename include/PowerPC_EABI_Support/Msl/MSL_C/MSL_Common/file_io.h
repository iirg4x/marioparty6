#ifndef _MSL_COMMON_FILE_IO_H
#define _MSL_COMMON_FILE_IO_H

#include "ansi_files.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compare stream mode text without case, stopping after the supplied byte count. */
int __msl_strnicmp(const char* str1, const char* str2, int n);

#ifdef __cplusplus
}
#endif

#endif /* _MSL_COMMON_FILE_IO_H */
