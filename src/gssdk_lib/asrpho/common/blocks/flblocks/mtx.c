/* Matrix storage and element operations for the speech DSP blocks. */
#include "dolphin/math.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/cmath>
#include "gssdk/mathusage.h"
#include "gssdk/heap.h"
#include "gssdk/mtx.h"

#include <string.h>

/* Allocates a floating-point matrix descriptor. */
struct MTXTag *mtxConstruct(struct _TOS *pTos)
{
    struct MTXTag *pMtx = heap_Calloc(pTos->hHeap, 1, sizeof(struct MTXTag));

    if (pMtx == NULL) {
        return NULL;
    }
    return pMtx;
}

/* Releases a floating-point matrix and its entries. */
void mtxDestruct(struct MTXTag *pMtx, struct _TOS *pTos)
{
    if (pMtx->Owner != 0 && pMtx->Entries != NULL) {
        heap_Free(pTos->hHeap, pMtx->Entries);
    }
    heap_Free(pTos->hHeap, pMtx);
}

/* Allocates an integer matrix descriptor. */
struct INTMTXTag *imtxConstruct(struct _TOS *pTos)
{
    struct INTMTXTag *pMtx = heap_Calloc(pTos->hHeap, 1, sizeof(struct INTMTXTag));

    if (pMtx == NULL) {
        return NULL;
    }
    return pMtx;
}

/* Releases an integer matrix and its entries. */
void imtxDestruct(struct INTMTXTag *pMtx, struct _TOS *pTos)
{
    if (pMtx->Owner != 0 && pMtx->Entries != NULL) {
        heap_Free(pTos->hHeap, pMtx->Entries);
    }
    heap_Free(pTos->hHeap, pMtx);
}

/* Sets the dimensions and allocates zeroed floating-point entries. */
struct MTXTag *mtxInit(
    struct MTXTag *pMtx, unsigned int M, unsigned int N, struct _TOS *pTos)
{
    u32 elementCount = M * N;

    pMtx->M = M;
    pMtx->N = N;
    pMtx->MxN = elementCount;
    pMtx->Entries = heap_Alloc(pTos->hHeap, elementCount * sizeof(f32));
    if (pMtx->Entries == NULL) {
        heap_Free(pTos->hHeap, pMtx);
        return NULL;
    }
    pMtx->Owner = 1;
    pMtx->EndEntries = pMtx->Entries + elementCount;
    memset(pMtx->Entries, 0, elementCount * sizeof(f32));
    return pMtx;
}

/* Sets the dimensions and allocates integer entries. */
struct INTMTXTag *imtxInit(
    struct INTMTXTag *pMtx, unsigned int M, unsigned int N, struct _TOS *pTos)
{
    u32 elementCount = M * N;

    pMtx->M = M;
    pMtx->N = N;
    pMtx->MxN = elementCount;
    pMtx->Entries = heap_Calloc(pTos->hHeap, elementCount, sizeof(int));
    if (pMtx->Entries == NULL) {
        heap_Free(pTos->hHeap, pMtx);
        return NULL;
    }
    pMtx->Owner = 1;
    pMtx->EndEntries = pMtx->Entries + elementCount;
    return pMtx;
}

/* Allocates entries and copies a floating-point matrix. */
struct MTXTag *mtxInitCopy(
    struct MTXTag *pMtxDst, const struct MTXTag *pMtxSrc, struct _TOS *pTos)
{
    pMtxDst->M = pMtxSrc->M;
    pMtxDst->N = pMtxSrc->N;
    pMtxDst->MxN = pMtxSrc->M * pMtxSrc->N;
    pMtxDst->Entries = heap_Alloc(
        pTos->hHeap, pMtxDst->M * pMtxDst->N * sizeof(f32));
    if (pMtxDst->Entries == NULL) {
        heap_Free(pTos->hHeap, pMtxDst);
        return NULL;
    }
    pMtxDst->Owner = 1;
    pMtxDst->EndEntries = pMtxDst->Entries + pMtxDst->M * pMtxDst->N;
    mtxFillCopy(pMtxDst, pMtxSrc);
    return pMtxDst;
}

/* Shifts entries over one deleted column and updates the stored counts. */
void imtxDeleteCol(struct INTMTXTag *pMtx, unsigned int Col)
{
    int *Entries = pMtx->Entries;
    unsigned int M = pMtx->M;
    unsigned int N = pMtx->N;
    int *dest = Entries + M * Col;
    int *src = dest + M;

    memcpy(dest, src, (M * N - M - M * Col) * sizeof(int));
    pMtx->N--;
    /* The stored total is reduced once per deleted column. */
    pMtx->MxN--;
    pMtx->EndEntries -= pMtx->M;
}

/* Associates a caller-supplied entry array with the matrix. */
void mtxAssociateAll(
    struct MTXTag *pMtx, unsigned int m, unsigned int n, const f32 *x)
{
    pMtx->M = m;
    pMtx->N = n;
    pMtx->MxN = m * n;
    pMtx->Entries = (f32 *)x;
    pMtx->EndEntries = (f32 *)x + pMtx->MxN;
}

/* Finds the largest entry in the matrix. */
f32 mtxMax(const struct MTXTag *pMtx)
{
    f32 *x = pMtx->Entries;
    f32 m = *x++;
    u32 i;
    u32 End = pMtx->MxN;

    for (i = 1; i < End; i++) {
        if (*x > m) {
            m = *x;
        }
        x++;
    }
    return m;
}

/* Compresses matrix values above the specified soft limit. */
void mtxCompress(struct MTXTag *pMtx, f32 a)
{
    f32 *src = pMtx->Entries;
    u32 i = 0;
    u32 End = pMtx->M * pMtx->N;

    for (; i < End; i++) {
        src[i] -= (f32)(0.5 * logf_check(1.0f + expf((f32)(2.0 * (src[i] - a)))));
    }
}

/* Deletes a column from the QR factor and applies the rotations to y. */
enum MTXERRtAG QrDeleteCol(struct MTXTag *R, struct MTXTag *y, unsigned int k)
{
    f32 *DirtyCol;
    f32 *End;
    f32 *rEnd = R->Entries + R->M * R->N;
    f32 *yEnd = y->Entries + y->MxN;
    u32 i;

    for (i = k + 1; i < R->N;
         i++) {
        f32 a;
        f32 b;
        f32 aAbs;
        f32 bAbs;
        f32 tau;
        f32 c;
        f32 s;
        f32 *CleanCol;
        u32 j;

        DirtyCol = R->Entries + i * R->M;
        End = DirtyCol + i;
        b = *End--;
        a = *End;
        CleanCol = DirtyCol - R->M;
        aAbs = a > 0.0f ? a : -a;
        bAbs = b > 0.0f ? b : -b;

        if (bAbs > aAbs) {
            tau = -a / b;
            s = 1.0 / sqrtf(1.0f + tau * tau);
            c = s * tau;
        } else {
            tau = -b / a;
            c = 1.0 / sqrtf(1.0f + tau * tau);
            s = c * tau;
        }

        for (j = 1; j < i; j++) {
            *CleanCol++ = *DirtyCol++;
        }
        *CleanCol = c * a - s * b;

        End += R->M;
        while (End < rEnd) {
            a = End[0];
            b = End[1];

            End[0] = c * a - s * b;
            End[1] = s * a + c * b;
            End += R->M;
        }

        End = y->Entries + i;
        while (End < yEnd) {
            a = End[-1];
            b = End[0];

            End[-1] = c * a - s * b;
            End[0] = s * a + c * b;
            End += y->M;
        }
    }
    R->N--;
    return NoError;
}
