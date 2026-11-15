#ifndef REL_M634DLL_POSITION_LIST_H
#define REL_M634DLL_POSITION_LIST_H

#include "dolphin/types.h"

typedef struct M634PositionList {
    void *owner;
    s32 count;
    void **entries;
} M634PositionList;

typedef struct M634PositionListOwner {

    u8 unk00[0x78]; /* No code names this field. */

    M634PositionList positions;
} M634PositionListOwner;

extern void **fn_1_5F6C(void *list, s32 index);

#endif
