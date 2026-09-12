#ifndef GSSDK_FFT_H
#define GSSDK_FFT_H

#include "types.h"

void fht(f32 *values, u32 length);
void realfft(u32 length, f32 *values);

#endif
