#ifndef MP6_C_STDIO_H
#define MP6_C_STDIO_H
#include "stddef.h"
#include "stdarg.h"
#include "file_struc.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/file_io.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/direct_io.h"
#include "eof.h"
#include "wchar_t.h"
/* Standard streams, formatted I/O, and file positioning. */
/* __StringWrite consumes a destination pointer, extent and current count. */
typedef struct {
    char *CharStr;
    size_t MaxCharCount;
    size_t CharsWritten;
} __OutStrCtrl;
#ifdef __cplusplus
extern "C" {
#endif
#define stdin (&__files[0])
#define stdout (&__files[1])
#define stderr (&__files[2])
int remove(const char *);
int rename(const char *, const char *);
FILE *tmpfile(void);
char *tmpnam(char *);
/* Stream orientation is queried by formatted output. */
int fwide(FILE *, int);
FILE *fopen(const char *, const char *);
FILE *freopen(const char *, const char *, FILE *);
void setbuf(FILE *, char *);
int setvbuf(FILE *, char *, int, size_t);
int fprintf(FILE *, const char *, ...);
int fscanf(FILE *, const char *, ...);
int printf(const char *, ...);
int scanf(const char *, ...);
int sprintf(char *, const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int sscanf(const char *, const char *, ...);
int vfprintf(FILE *, const char *, va_list);
int vprintf(const char *, va_list);
int vsprintf(char *, const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
int fgetc(FILE *);
char *fgets(char *, int, FILE *);
int fputc(int, FILE *);
int fputs(const char *, FILE *);
int getc(FILE *);
int getchar(void);
char *gets(char *);
int putc(int, FILE *);
int putchar(int);
int puts(const char *);
int ungetc(int, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
int fgetpos(FILE *, fpos_t *);
int fseek(FILE *, long, int);
int fsetpos(FILE *, const fpos_t *);
long ftell(FILE *);
void rewind(FILE *);
void clearerr(FILE *);
int feof(FILE *);
int ferror(FILE *);
void perror(const char *);
#ifdef __cplusplus
}
namespace std {
using ::FILE; using ::fpos_t; using ::remove; using ::rename; using ::tmpfile; using ::tmpnam;
using ::fclose; using ::fflush; using ::fopen; using ::freopen; using ::setbuf; using ::setvbuf;
using ::fprintf; using ::fscanf; using ::printf; using ::scanf; using ::sprintf; using ::snprintf;
using ::sscanf; using ::vfprintf; using ::vprintf; using ::vsprintf; using ::vsnprintf;
using ::fgetc; using ::fgets; using ::fputc; using ::fputs; using ::getc; using ::getchar;
using ::gets;
using ::putc;
using ::putchar;
using ::puts;
using ::ungetc;
using ::fread;
using ::fwrite;
using ::fgetpos; using ::fseek; using ::fsetpos; using ::ftell; using ::rewind;
using ::clearerr; using ::feof; using ::ferror; using ::perror;
}
#endif
#endif
