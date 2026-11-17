#ifndef GSSDK_RSRC_H
#define GSSDK_RSRC_H

#include "gssdk/safeh.h"

struct RSRC_S {
    unsigned short u16HandleValidityCheck;
    void *hHeap;
    void *hCSClass;
    void *hBrk;
    void *hDbg;
};

u32 InitRsrcFunction(SafeHandle hRsrc, const struct RSRC_S **ppRsrc);

#endif
