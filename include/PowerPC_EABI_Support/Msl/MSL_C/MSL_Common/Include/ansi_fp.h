#ifndef MP6_C_ANSI_FP_H
#define MP6_C_ANSI_FP_H

#include "float.h"

#define SIGDIGLEN 36

/* The shared conversion record retains the project's existing member interface. */
typedef struct decimal {
	char sign;
	char unk1;
	short exp;
	struct {
		unsigned char length;
		unsigned char text[36];
		unsigned char unk41;
	} sig;
} decimal;

/* Conversion requests carry the output style and digit count. */
typedef struct decform {
	char style;
	char unk1;
	short digits;
} decform;

#ifdef __cplusplus
extern "C" {
#endif
void __num2dec(const decform* form, double x, decimal* d);

#ifdef __cplusplus
}
#endif
#endif
