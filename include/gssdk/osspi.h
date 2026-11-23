/* Pointer validation used by the Voice SDK handle checker. */
#ifndef GSSDK_OSSPI_H
#define GSSDK_OSSPI_H

#include "types.h"

u32 osspi_IsGoodReadPtr(const void *ptr, u32 size);
u32 osspi_IsGoodWritePtr(void *ptr, u32 size);

#endif
