#ifndef MP6_C_STDLIB_H
#define MP6_C_STDLIB_H
#include "stddef.h"
#include "div_t.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/alloc.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/arith.h"
/* Standard allocation, conversion, ordering, and process interfaces. */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 32767
#define MB_CUR_MAX 2
#ifdef __cplusplus
extern "C" {
#endif
double atof(const char *);
int atoi(const char *);
long atol(const char *);
double strtod(const char *, char **);
long strtol(const char *, char **, int);
unsigned long strtoul(const char *, char **, int);
int rand(void);
void srand(unsigned int);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void abort(void);
void exit(int);
int atexit(void (*)(void));
char *getenv(const char *);
int system(const char *);
void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
int abs(int);
long labs(long);
ldiv_t ldiv(long, long);
int mblen(const char *, size_t);
int mbtowc(wchar_t *, const char *, size_t);
int wctomb(char *, wchar_t);
size_t mbstowcs(wchar_t *, const char *, size_t);
size_t wcstombs(char *, const wchar_t *, size_t);
#ifdef __cplusplus
}
namespace std {
using ::atof; using ::atoi; using ::atol; using ::strtod; using ::strtol; using ::strtoul;
using ::rand; using ::srand; using ::malloc; using ::calloc; using ::realloc; using ::free;
using ::abort; using ::exit; using ::atexit; using ::getenv; using ::system;
using ::bsearch; using ::qsort; using ::abs; using ::labs; using ::div; using ::ldiv;
using ::mblen; using ::mbtowc; using ::wctomb; using ::mbstowcs; using ::wcstombs;
}
#endif
#endif
