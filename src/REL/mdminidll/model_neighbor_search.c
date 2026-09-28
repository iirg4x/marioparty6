#define _MATH_H
#include "game/main.h"

/* Existing callers use this opaque record pointer; the target consumes nine
 * signed halfwords per row. This view allocates no storage. */
struct _struct_lbl_1_data_89A_0x12;

/* The three expanded searches share the complete recursive operation.
 * Restore that operation and let the configured auto-inliner own its homes. */
s16 fn_1_9348(s16 arg0, s16 arg1, s16 arg2, s16 arg3, struct _struct_lbl_1_data_89A_0x12 (*arg4)[], s16 arg5)
{
    s16 next;
    s16 i;

    if (arg1 == -1) {
        return -1;
    }
    next = ((s16 (*)[9])arg4)[arg0][arg1];
    if (next == -1) {
        next = fn_1_9348(arg0, arg2, arg3, -1, arg4, arg5);
    }
    i = 0;
    while (i < arg5) {
        if ((arg0 != i) && (((s16 (*)[9])arg4)[i][8] != -1) && (next == i)) {
            next = fn_1_9348(next, arg1, arg2, arg3, arg4, arg5);
            if (next == -1) {
                next = fn_1_9348(arg0, arg2, arg3, -1, arg4, arg5);
            }
        }
        i += 1;
    }
    return next;
}
