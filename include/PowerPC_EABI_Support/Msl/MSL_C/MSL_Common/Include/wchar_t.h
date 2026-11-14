#ifndef MP6_C_WCHAR_T_H
#define MP6_C_WCHAR_T_H
#include "stddef.h"
/* The oracle mangles wide I/O characters as wchar_t and state as int. */
typedef wchar_t wint_t;
typedef int mbstate_t;
#define WEOF ((wint_t)0xFFFF)
#ifdef __cplusplus
namespace std { using ::wint_t; using ::mbstate_t; }
#endif
#endif
