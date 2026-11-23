/* Voice SDK resource handles and resource access. */
#ifndef GSSDK_RSRC_H
#define GSSDK_RSRC_H

#include "types.h"
#include "gssdk/safeh.h"

/* Handles carried by a voice SDK resource object. */
struct RSRC_S {
    /* Value used to validate the resource handle. */
    unsigned short u16HandleValidityCheck;
    /* Heap associated with the resource. */
    void *hHeap;
    /* Critical-section class handle. */
    void *hCSClass;
    /* Break handler handle. */
    void *hBrk;
    /* Debug handle. */
    void *hDbg;
};

/* Validates the handle and returns its resource record. */
long InitRsrcFunction(SafeHandle hRsrc, const struct RSRC_S **ppRsrc);

/* Opens a resource and gives its handle to the caller. */
long rsrc_Open(void *hHeap, void *hCSClass, void *hBrk, void *hDbg, SafeHandle *phRsrc);

/* Releases the resource validated by this handle. */
long rsrc_Close(SafeHandle hRsrc);

#endif
