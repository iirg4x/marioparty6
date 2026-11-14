#ifndef MP6_C_CTYPE_H
#define MP6_C_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <eof.h>

/* Byte-indexed masks used by character-classification operations. */
extern unsigned char __ctype_map[];
/* Unsigned-byte lookup maps uppercase input to its lowercase representation. */
extern unsigned char __lower_map[];
/* Byte table supplies uppercase replacements for lowercase input. */
extern unsigned char __upper_map[];

/* Individual character-class bits in the classification table. */
/* Classification bit for non-motion control characters. */
#define __control_char 0x01
/* Classification bit for whitespace that moves the print position. */
#define __motion_char  0x02
/* Classification bit for the space character. */
#define __space_char   0x04
/* Classification bit for punctuation bytes. */
#define __punctuation  0x08
/* Classification bit for decimal digits. */
#define __digit        0x10
/* Classification bit for hexadecimal letters and digits. */
#define __hex_digit    0x20
/* Classification bit for lowercase letters. */
#define __lower_case   0x40
/* Classification bit for uppercase letters. */
#define __upper_case   0x80

/* Combine the individual bits for the public character classes. */
/* Mask accepts either lowercase or uppercase alphabetic entries. */
#define __letter       (__lower_case | __upper_case)
/* Mask combines letters with decimal digits. */
#define __alphanumeric (__letter | __digit)
/* Mask includes visible non-space characters. */
#define __graphic      (__alphanumeric | __punctuation)
/* Mask adds space to the visible character classes. */
#define __printable    (__graphic | __space_char)
/* Mask accepts space and print-position motion entries. */
#define __whitespace   (__motion_char | __space_char)
/* Mask combines motion entries with other control characters. */
#define __control      (__motion_char | __control_char)
/* Convert an input byte to a nonnegative table index. */
#define __zero_fill(c) ((int)(unsigned char)(c))

/* Test for an alphabetic character or decimal digit. */
int isalnum(int);
/* Test for an alphabetic character. */
int isalpha(int);
/* Test for a control character. */
int iscntrl(int);
/* Test for a printing character other than space. */
int isgraph(int);
/* Test for a lowercase character. */
int islower(int);
/* Test for a printing character including space. */
int isprint(int);
/* Test for a punctuation character. */
int ispunct(int);
/* Convert an uppercase character to lowercase, preserving EOF. */
int tolower(int c);
/* Convert a lowercase character to uppercase, preserving EOF. */
int toupper(int c);

/* Test the alphabetic classification bits. */
inline int _isalpha(int c)
{
	return (int)(__ctype_map[(unsigned char)c] & __letter);
}
/* Test the decimal-digit classification bit. */
inline int isdigit(int c)
{
	return (int)(__ctype_map[(unsigned char)c] & __digit);
}
/* Test the whitespace classification bits. */
inline int isspace(int c)
{
	return (int)(__ctype_map[(unsigned char)c] & __whitespace);
}
/* Test the uppercase classification bit. */
inline int isupper(int c)
{
	return (int)(__ctype_map[(unsigned char)c] & __upper_case);
}
/* Test the hexadecimal-digit classification bit. */
inline int isxdigit(int c)
{
	return (int)(__ctype_map[(unsigned char)c] & __hex_digit);
}

/* Look up the lowercase character, preserving EOF. */
inline int _tolower(int c)
{
	return (c == -1 ? -1 : (int)__lower_map[(unsigned char)c]);
}

#ifdef __cplusplus
};

namespace std {
using ::isalnum;
using ::isalpha;
using ::iscntrl;
using ::isgraph;
using ::islower;
using ::isprint;
using ::ispunct;
using ::isdigit;
using ::isspace;
using ::isupper;
using ::isxdigit;
inline int tolower(int c)
{
	return (c == -1 ? -1 : (int)__lower_map[(unsigned char)c]);
}
inline int toupper(int c)
{
	return (c == -1 ? -1 : (int)__upper_map[(unsigned char)c]);
}
}; // namespace std

#endif

#endif /* MP6_C_CTYPE_H */
