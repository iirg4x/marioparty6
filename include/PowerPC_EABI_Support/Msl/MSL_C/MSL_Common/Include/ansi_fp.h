#ifndef MP6_C_ANSI_FP_H
#define MP6_C_ANSI_FP_H

#include "float.h"

/* Maximum digit-byte extent of the decimal significand record. */
#define SIGDIGLEN 36

/* Decimal result stores sign bytes, a short exponent and an embedded significand record. */
typedef struct decimal {
	// Sign flag retained in the decimal conversion result.
	char sign;
	// Byte with no identified meaning in the shared conversion record.
	char unk1;
	// Decimal exponent used to place the significand digits.
	short exp;
	/* Significand storage places the length before 36 digit bytes and an unknown byte. */
	struct {
		/* Number of significant digit bytes produced by conversion. */
		unsigned char length;
		/* Decimal digits consumed by formatting. */
		unsigned char text[36];
		/* Trailing byte whose meaning has not been identified. */
		unsigned char unk41;
	} sig;
} decimal;

/* Conversion request stores two style bytes followed by a short digit count. */
typedef struct decform {
	// Output-style selector passed to decimal conversion.
	char style;
	// Unidentified byte retained between the style and requested digit count.
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
