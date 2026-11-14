#ifndef MP6_C_STDIO_H
#define MP6_C_STDIO_H
#include "stddef.h"
#include "stdarg.h"
#include "file_struc.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/file_io.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/direct_io.h"
#include "eof.h"
#include "wchar_t.h"
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
/* Remove the named file. */
int remove(const char *);
/* Change a file name. */
int rename(const char *, const char *);
/* Open a temporary binary update stream. */
FILE *tmpfile(void);
/* Produce a temporary file name. */
char *tmpnam(char *);
/* Query or set the stream orientation. */
int fwide(FILE *, int);
/* Open the named file with the requested access mode. */
FILE *fopen(const char *, const char *);
/* Reopen an existing stream on the named file. */
FILE *freopen(const char *, const char *, FILE *);
/* Select a stream buffer with the default extent. */
void setbuf(FILE *, char *);
/* Select the stream buffering mode and extent. */
int setvbuf(FILE *, char *, int, size_t);
/* Write formatted arguments to a stream. */
int fprintf(FILE *, const char *, ...);
/* Read formatted values from a stream. */
int fscanf(FILE *, const char *, ...);
/* Write formatted arguments to standard output. */
int printf(const char *, ...);
/* Read formatted values from standard input. */
int scanf(const char *, ...);
/* Write formatted arguments to a string. */
int sprintf(char *, const char *, ...);
/* Write formatted arguments within a string capacity. */
int snprintf(char *, size_t, const char *, ...);
/* Read formatted values from a string. */
int sscanf(const char *, const char *, ...);
/* Write an argument list to a stream using a format string. */
int vfprintf(FILE *, const char *, va_list);
/* Write an argument list to standard output using a format string. */
int vprintf(const char *, va_list);
/* Write an argument list to a string using a format string. */
int vsprintf(char *, const char *, va_list);
/* Write an argument list within a string capacity using a format string. */
int vsnprintf(char *, size_t, const char *, va_list);
/* Read one character from a stream. */
int fgetc(FILE *);
/* Read a bounded line from a stream. */
char *fgets(char *, int, FILE *);
/* Write one character to a stream. */
int fputc(int, FILE *);
/* Write a string to a stream. */
int fputs(const char *, FILE *);
int getc(FILE *);
/* Read one character from standard input. */
int getchar(void);
/* Read a line from standard input. */
char *gets(char *);
int putc(int, FILE *);
/* Write one character to standard output. */
int putchar(int);
/* Write a string and newline to standard output. */
int puts(const char *);
/* Push one character back onto a stream. */
int ungetc(int, FILE *);
/* Read an array of elements from a stream. */
size_t fread(void *, size_t, size_t, FILE *);
/* Save the current stream position. */
int fgetpos(FILE *, fpos_t *);
/* Set the stream position relative to the requested origin. */
int fseek(FILE *, long, int);
/* Restore a saved stream position. */
int fsetpos(FILE *, const fpos_t *);
/* Return the current stream position. */
long ftell(FILE *);
/* Reset the stream position and clear its error indicator. */
void rewind(FILE *);
/* Clear the stream error and end-of-file indicators. */
void clearerr(FILE *);
/* Test the stream end-of-file indicator. */
int feof(FILE *);
/* Test the stream error indicator. */
int ferror(FILE *);
/* Print a description of the current error to standard error. */
void perror(const char *);
#ifdef __cplusplus
}
namespace std {
using ::FILE;
using ::fpos_t;
using ::remove;
using ::rename;
using ::tmpfile;
using ::tmpnam;
using ::fclose;
using ::fflush;
using ::fopen;
using ::freopen;
using ::setbuf;
using ::setvbuf;
using ::fprintf;
using ::fscanf;
using ::printf;
using ::scanf;
using ::sprintf;
using ::snprintf;
using ::sscanf;
using ::vfprintf;
using ::vprintf;
using ::vsprintf;
using ::vsnprintf;
using ::fgetc;
using ::fgets;
using ::fputc;
using ::fputs;
using ::getc;
using ::getchar;
using ::gets;
using ::putc;
using ::putchar;
using ::puts;
using ::ungetc;
using ::fread;
using ::fwrite;
using ::fgetpos;
using ::fseek;
using ::fsetpos;
using ::ftell;
using ::rewind;
using ::clearerr;
using ::feof;
using ::ferror;
using ::perror;
}
#endif
#endif
