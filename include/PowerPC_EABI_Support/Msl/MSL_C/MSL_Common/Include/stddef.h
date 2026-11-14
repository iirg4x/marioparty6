#ifndef MP6_C_STDDEF_H
#define MP6_C_STDDEF_H
#include "size_t.h"
#include "null.h"
/* Differences between pointers use the signed EABI word. */
typedef long ptrdiff_t;
#ifndef __cplusplus
typedef unsigned short wchar_t;
#else
namespace std { using ::ptrdiff_t; }
#endif
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#endif
