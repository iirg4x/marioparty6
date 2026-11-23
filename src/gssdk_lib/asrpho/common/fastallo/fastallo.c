/* Chunk allocation and size-class free lists for small allocations. */
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/fastallo.h"

/* Allocates Size words from the current chunk, obtaining a new chunk if needed. */
void *fastallo_AllocateMemoryFromChunk(struct _FASTALLO_INST *pInst, unsigned short Size)
{
    void *pRet = NULL;
    unsigned long NbrOfArrays;

    if (Size == 0 || Size > 64) {
        goto done;
    }

    if (pInst->FreeMemorySize < Size) {
        void *pNewArrs;
        void *pExtra;

        NbrOfArrays = pInst->NbrArrs;
        if (pInst->FreeMemorySize != 0) {
            struct _FASTALLO_FREE_NODE **pTemp = &pInst->pFreeNodes[
                ((u32)pInst->FreeMemorySize + 3) >> 2];

            (pInst->pFreeMemory)->pNext = *pTemp;
            *pTemp = pInst->pFreeMemory;
        }

        pNewArrs = heap_Realloc(pInst->hHeap, pInst->pArrs,
                              (NbrOfArrays + 1) * sizeof(void *));
        if (pNewArrs == NULL) {
            goto done;
        }
        pInst->pArrs = pNewArrs;

        pExtra = heap_Alloc(pInst->hHeap, pInst->ChunkSize);
        if (pExtra == NULL) {
            goto done;
        }
        ((void **)pInst->pArrs)[NbrOfArrays] = pExtra;
        pInst->NbrArrs = ++NbrOfArrays;
        pInst->pFreeMemory = pExtra;
        pInst->FreeMemorySize = (u32)pInst->ChunkSize >> 2;
    }

    pRet = pInst->pFreeMemory;
    pInst->FreeMemorySize -= (u32)Size;
    pInst->pFreeMemory = (struct _FASTALLO_FREE_NODE *)((void **)pInst->pFreeMemory + Size);

done:
    return pRet;
}

/* Clears the allocator state and records its heap and chunk size in bytes. */
void fastallo_Init(struct _FASTALLO_INST *pInst, void *hHeap, unsigned short ChunkSize)
{
    memset(pInst, 0, sizeof(struct _FASTALLO_INST));
    pInst->ChunkSize = ChunkSize;
    pInst->hHeap = hHeap;
}

/* Frees all chunks and the array of chunk pointers. */
void fastallo_Terminate(struct _FASTALLO_INST *pInst)
{
    u32 i;
    u32 NbrArrs = pInst->NbrArrs;

    for (i = 0; i < NbrArrs; i++) {
        heap_Free(pInst->hHeap, ((void **)pInst->pArrs)[i]);
    }
    if (pInst->pArrs != NULL) {
        heap_Free(pInst->hHeap, pInst->pArrs);
    }
}
