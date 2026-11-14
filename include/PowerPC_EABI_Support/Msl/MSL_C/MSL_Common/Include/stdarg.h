#ifndef MP6_C_STDARG_H
#define MP6_C_STDARG_H
#include "ansi_parms.h"
#include "va_list.h"
/* Compiler intrinsics implement the PowerPC variadic calling convention. */
#ifdef __cplusplus
extern "C" {
#endif
void *__va_arg(void *, int);
#ifdef __cplusplus
}
#endif
#define va_start(list, last) \
    ((void)__builtin_next_arg(last), (void)__builtin_va_info(&(list)))
#define va_arg(list, type) (*(type *)__va_arg(list, __builtin_vargtype(type)))
#define va_end(list) ((void)0)
#define va_copy(dest, source) ((dest)[0] = (source)[0])
#endif
