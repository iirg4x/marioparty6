/* Pointer checks used when validating a Voice SDK handle. */
#include "gssdk/osspi.h"

#define OSSPI_INVALID_POINTER 0x80060183

/* Rejects null pointers supplied to a read operation. */
u32 osspi_IsGoodReadPtr(const void *ptr, u32 size)
{
    if (ptr == NULL) {
        return OSSPI_INVALID_POINTER;
    }
    return 0;
}

/* Rejects null pointers supplied to a write operation. */
u32 osspi_IsGoodWritePtr(void *ptr, u32 size)
{
    if (ptr == NULL) {
        return OSSPI_INVALID_POINTER;
    }
    return 0;
}
