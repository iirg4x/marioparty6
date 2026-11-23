/* Validation and comparison of typed Voice SDK handles. */
#include "gssdk/osspi.h"
#include "gssdk/safeh.h"

#include <stddef.h>

#define SAFEH_INVALID_HANDLE 0x80170189

const SafeHandle g_lhNullHandle = { NULL, 0 };

/* Checks the handle type and validates its address for reading and writing. */
u32 safeh_HandleCheck(SafeHandle handle, u32 type, u32 size)
{
    u32 result;

    if (handle.type != type) {
        return SAFEH_INVALID_HANDLE;
    }
    if (handle.address == NULL) {
        return SAFEH_INVALID_HANDLE;
    }

    result = osspi_IsGoodReadPtr(handle.address, size);
    if (IsError(result)) {
        return result;
    }
    return osspi_IsGoodWritePtr(handle.address, size);
}

/* Compares both the handle type and its address. */
BOOL safeh_HandlesEqual(SafeHandle left, SafeHandle right)
{
    if (left.type != right.type) {
        return FALSE;
    }
    if (left.address != right.address) {
        return FALSE;
    }
    return TRUE;
}
