#ifndef REL_M634DLL_VALUE_STORE_H
#define REL_M634DLL_VALUE_STORE_H

#include <stddef.h>
#include "dolphin/types.h"

struct M634ValueStore {

    unsigned char unknown_00[0x18]; /* No code names this field. */

    s32 primaryCount;
    s32 secondaryCount;
    s32 values[4];
};

typedef char m634_value_store_primary_count_check[
    (offsetof(M634ValueStore, primaryCount) == 0x18) ? 1 : -1];
typedef char m634_value_store_secondary_count_check[
    (offsetof(M634ValueStore, secondaryCount) == 0x1C) ? 1 : -1];
typedef char m634_value_store_values_check[
    (offsetof(M634ValueStore, values) == 0x20) ? 1 : -1];

#endif
