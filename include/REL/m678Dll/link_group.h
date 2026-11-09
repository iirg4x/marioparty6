#ifndef M678_LINK_GROUP_H
#define M678_LINK_GROUP_H

#include "dolphin/types.h"

struct M678LinkEntry;

typedef struct M678ChainGroup {
    s32 index;
    struct M678ChainGroup *parent;
    s32 count;
    s32 maximum;
    s32 minimum;
    struct M678LinkEntry *first;
    struct M678LinkEntry *last;
} M678ChainGroup;

struct _struct_lbl_1_bss_2F88_0xB28 {
    M678ChainGroup entries[102];
};

typedef struct _struct_lbl_1_bss_2F88_0xB28 M678LinkGroupSet;

#endif
