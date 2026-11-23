#ifndef GSSDK_SAFEH_H
#define GSSDK_SAFEH_H

#include "types.h"

typedef struct SafeHandle {
    void *address;
    u32 type;
} SafeHandle;

extern const SafeHandle g_lhNullHandle;

u32 safeh_HandleCheck(SafeHandle handle, u32 type, u32 size);
BOOL safeh_HandlesEqual(SafeHandle left, SafeHandle right);

/* Tests the high bit used to mark an error. */
static inline BOOL IsError(u32 result)
{
    return result >= 0x80000000 ? TRUE : FALSE;
}

#endif
