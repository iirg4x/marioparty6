#ifndef MP6_C_STRING_H
#define MP6_C_STRING_H
#include "stddef.h"
/* Standard byte-buffer and zero-terminated string operations. */
#ifdef __cplusplus
extern "C" {
#endif
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
size_t strcspn(const char *, const char *);
size_t strspn(const char *, const char *);
char *strpbrk(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *, const char *);
size_t strlen(const char *);
int strcoll(const char *, const char *);
size_t strxfrm(char *, const char *, size_t);
char *strerror(int);
#ifdef __cplusplus
}
namespace std {
using ::memcpy;
using ::memmove;
using ::memset;
using ::memcmp;
using ::memchr;
using ::strcpy;
using ::strncpy;
using ::strcat;
using ::strncat;
using ::strcmp;
using ::strncmp;
using ::strchr;
using ::strrchr;
using ::strcspn;
using ::strspn;
using ::strpbrk;
using ::strstr;
using ::strtok;
using ::strlen;
using ::strcoll;
using ::strxfrm;
using ::strerror;
}
#endif
#endif
