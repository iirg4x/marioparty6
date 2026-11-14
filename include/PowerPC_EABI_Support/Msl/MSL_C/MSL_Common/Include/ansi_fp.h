#ifndef MP6_C_ANSI_FP_H
#define MP6_C_ANSI_FP_H

#include "float.h"

/* Maximum number of significant digit bytes requested by decimal conversion. */
#define SIGDIGLEN 36

/* Decimal result stores sign bytes, a short exponent and an embedded significand record. */
typedef struct decimal {
	// Sign flag retained in the decimal conversion result.
	char sign;
	// Unread byte at offset 1; the short exponent begins at offset 2.
	char unk1;
	// Decimal exponent used to place the significand digits.
	short exp;
	/* Significand storage places the length before 36 digit bytes and an unknown byte. */
	struct {
		/* Number of significant digit bytes produced by conversion. */
		unsigned char length;
		/* Decimal digits consumed by formatting. */
		unsigned char text[36];
		/* Unread byte at offset 41; the decimal result occupies 42 bytes. */
		unsigned char unk41;
	} sig;
} decimal;

/* Conversion request stores two style bytes followed by a short digit count. */
typedef struct decform {
	// Output-style selector passed to decimal conversion.
	char style;
	// Unread byte at offset 1; the requested digit count follows at offset 2.
	char unk1;
	// Requested count of decimal digits.
	short digits;
} decform;

#ifdef __cplusplus
extern "C" {
#endif
/* Convert a binary floating-point value to the requested decimal representation. */
void __num2dec(const decform* form, double x, decimal* d);

#ifdef __cplusplus
}
#endif
#endif
