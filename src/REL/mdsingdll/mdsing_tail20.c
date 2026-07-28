#include "dolphin/types.h"

typedef struct Fn19998Entry {
    s16 value[8];
    s16 marker;
} FN_1_9998_ENTRY;

s16 fn_1_9998(s16 arg0, s16 arg1, s16 arg2, s16 arg3,
    FN_1_9998_ENTRY *arg4, s16 arg5)
{
    s16 result;
    s16 j;
    s16 i;
    s16 k;
    s16 l;

    if (arg1 == -1) {
        return -1;
    }
    result = arg4[arg0].value[arg1];
    if (result == -1) {
        s16 routed;

        if (arg2 == -1) {
            routed = -1;
        } else {
            s16 next;

            next = arg4[arg0].value[arg2];
            if (next == -1) {
                next = fn_1_9998(arg0, arg3, -1, -1, arg4, arg5);
            }
            for (i = 0; i < arg5; i++) {
                if (arg0 != i && arg4[i].marker != -1 && next == i) {
                    next = fn_1_9998(
                        next, arg2, arg3, -1, arg4, arg5);
                    if (next == -1) {
                        next = fn_1_9998(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                }
            }
            routed = next;
        }
        result = routed;
    }

    for (j = 0; j < arg5; j++) {
        if (arg0 != j && arg4[j].marker != -1 && result == j) {
            s16 routed;

            if (arg1 == -1) {
                routed = -1;
            } else {
                s16 next;

                next = arg4[result].value[arg1];
                if (next == -1) {
                    next = fn_1_9998(
                        result, arg2, arg3, -1, arg4, arg5);
                }
                for (k = 0; k < arg5; k++) {
                    if (result != k && arg4[k].marker != -1
                        && next == k) {
                        next = fn_1_9998(
                            next, arg1, arg2, arg3, arg4, arg5);
                        if (next == -1) {
                            next = fn_1_9998(
                                result, arg2, arg3, -1, arg4, arg5);
                        }
                    }
                }
                routed = next;
            }
            result = routed;
            if (result == -1) {
                s16 fallback;

                if (arg2 == -1) {
                    fallback = -1;
                } else {
                    s16 next;

                    next = arg4[arg0].value[arg2];
                    if (next == -1) {
                        next = fn_1_9998(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                    for (l = 0; l < arg5; l++) {
                        if (arg0 != l && arg4[l].marker != -1
                            && next == l) {
                            next = fn_1_9998(
                                next, arg2, arg3, -1, arg4, arg5);
                            if (next == -1) {
                                next = fn_1_9998(
                                    arg0, arg3, -1, -1, arg4, arg5);
                            }
                        }
                    }
                    fallback = next;
                }
                result = fallback;
            }
        }
    }
    return result;
}
