/* Allocation, validation and release of resource handles. */
#include "gssdk/heap.h"
#include "gssdk/rsrc.h"
#include "gssdk/osspi.h"

#include <stddef.h>

#define RSRC_TYPE 0x1E61
#define RSRC_INVALID_RESOURCE 0x80160101
#define RSRC_OUT_OF_MEMORY 0x80160186

const struct RSRC_S g_NullResource = { RSRC_TYPE };

/* Validates a handle and returns its resource record. */
long InitRsrcFunction(SafeHandle hRsrc, const struct RSRC_S **ppRsrc)
{
    long lhError = 0;

    if (safeh_HandlesEqual(hRsrc, g_lhNullHandle)) {
        *ppRsrc = &g_NullResource;
    } else {
        lhError = safeh_HandleCheck(hRsrc, RSRC_TYPE, sizeof(struct RSRC_S));
        if (IsError(lhError)) {
            /* A bad handle type becomes an invalid-resource error. */
            if ((lhError & 0x7FU) == 9) {
                lhError = RSRC_INVALID_RESOURCE;
            }
            return lhError;
        }
        *ppRsrc = hRsrc.address;
        if (*ppRsrc == NULL || (*ppRsrc)->u16HandleValidityCheck != RSRC_TYPE) {
            return RSRC_INVALID_RESOURCE;
        }
    }
    return lhError;
}

/* Clears the validity marker before returning the resource to its heap. */
static void FreeRsrc(struct RSRC_S *pRsrc)
{
    if (pRsrc != NULL) {
        pRsrc->u16HandleValidityCheck = 0;
        heap_Free(pRsrc->hHeap, pRsrc);
    }
}

/* Allocates and initializes the resource attached to a new handle. */
long rsrc_Open(void *hHeap, void *hCSClass, void *hBrk, void *hDbg, SafeHandle *phRsrc)
{
    struct RSRC_S *pRsrc = NULL;
    long lhError;

    lhError = osspi_IsGoodWritePtr(phRsrc, sizeof(SafeHandle));
    if (!IsError(lhError)) {
        pRsrc = heap_Calloc(hHeap, sizeof(struct RSRC_S), 1);
        if (pRsrc == NULL) {
            lhError = RSRC_OUT_OF_MEMORY;
        } else {
            pRsrc->hHeap = hHeap;
            pRsrc->hBrk = hBrk;
            pRsrc->hCSClass = hCSClass;
            pRsrc->hDbg = hDbg;
            phRsrc->address = pRsrc;
            phRsrc->type = RSRC_TYPE;
            pRsrc->hHeap = hHeap;
            pRsrc->hBrk = hBrk;
            pRsrc->hCSClass = hCSClass;
            pRsrc->hDbg = hDbg;
            pRsrc->u16HandleValidityCheck = RSRC_TYPE;
        }
    }
    if (IsError(lhError)) {
        FreeRsrc(pRsrc);
    }
    return lhError;
}

/* Validates the handle before releasing the resource. */
long rsrc_Close(SafeHandle hRsrc)
{
    long lhError;
    struct RSRC_S *pRsrc;

    lhError = InitRsrcFunction(hRsrc, (const struct RSRC_S **)&pRsrc);
    if (!IsError(lhError)) {
        FreeRsrc(pRsrc);
    }
    return lhError;
}
