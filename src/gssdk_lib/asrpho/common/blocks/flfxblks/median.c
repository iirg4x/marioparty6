/* Sliding median filter for the recognition pipeline. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include "gssdk/tos.h"

/* A sample and its neighbors in ascending value order. */
typedef struct _MEDIANLIST {
    struct _MEDIANLIST *pNext; /* Next sample in ascending value order. */
    struct _MEDIANLIST *pPrevious; /* Previous sample in ascending value order. */
    f32 Value; /* Value held in this ring slot. */
} MedianNode;

/* Ring of samples with links ordered by value. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 Depth; /* Number of slots in the sample ring. */
    u32 NrElements; /* Number of samples held in the ring. */
    u32 IndexOldestElem; /* Ring slot overwritten on the next input. */
    MedianNode *pMedianList; /* Ring allocation. */
    MedianNode *pFirst; /* Lowest-valued sample. */
    MedianNode *pLast; /* Highest-valued sample. */
    MedianNode *pMedian; /* Median node in the sorted list. */
} Median;

/* Queues the median of the samples held in the ring. */
static void ProcessMedian(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Median *This = (Median *)_This;
    MedianNode *pCurrElement;
    MedianNode *pPointer;
    f32 Input;
    f32 *pOut;
    u32 Index;
    u32 StopIndex;
    f32 *input = ((void **)pInputs)[0]; /* Frame log energy at the start of the VAD level packet. */

    if (input != NULL) {
        pCurrElement = &This->pMedianList[This->IndexOldestElem];
        Input = *input;

        if (This->NrElements == This->Depth) {
            if (pCurrElement->pNext != NULL) {
                pCurrElement->pNext->pPrevious = pCurrElement->pPrevious;
            } else {
                This->pLast = pCurrElement->pPrevious;
            }

            if (pCurrElement->pPrevious != NULL) {
                pCurrElement->pPrevious->pNext = pCurrElement->pNext;
            } else {
                This->pFirst = pCurrElement->pNext;
            }

            This->NrElements--;

            if (pCurrElement == This->pMedian) {
                if (pCurrElement->pNext != NULL) {
                    This->pMedian = pCurrElement->pNext;
                } else if (pCurrElement->pPrevious != NULL) {
                    This->pMedian = pCurrElement->pPrevious;
                } else {
                    This->pMedian = NULL;
                }
            }
        }

        pCurrElement->Value = Input;

        if (This->NrElements == 0) {
            This->pMedian = pCurrElement;
            This->pLast = pCurrElement;
            This->pFirst = pCurrElement;
            pCurrElement->pPrevious = NULL;
            pCurrElement->pNext = NULL;
        } else if (Input < This->pMedian->Value) {
            pPointer = This->pMedian->pPrevious;
            while (pPointer != NULL && Input < pPointer->Value) {
                pPointer = pPointer->pPrevious;
            }

            if (pPointer != NULL) {
                pCurrElement->pNext = pPointer->pNext;
                pPointer->pNext = pCurrElement;
                pCurrElement->pPrevious = pPointer;
                pCurrElement->pNext->pPrevious = pCurrElement;
            } else {
                pCurrElement->pNext = This->pFirst;
                pCurrElement->pPrevious = NULL;
                This->pFirst->pPrevious = pCurrElement;
                This->pFirst = pCurrElement;
            }
        } else {
            pPointer = This->pMedian->pNext;
            while (pPointer != NULL && Input >= pPointer->Value) {
                pPointer = pPointer->pNext;
            }

            if (pPointer != NULL) {
                pCurrElement->pNext = pPointer;
                pCurrElement->pPrevious = pPointer->pPrevious;
                pPointer->pPrevious = pCurrElement;
                pCurrElement->pPrevious->pNext = pCurrElement;
            } else {
                pCurrElement->pNext = NULL;
                pCurrElement->pPrevious = This->pLast;
                This->pLast->pNext = pCurrElement;
                This->pLast = pCurrElement;
            }
        }

        if (++This->IndexOldestElem == This->Depth) {
            This->IndexOldestElem = 0;
        }

        This->NrElements++;
        This->pMedian = This->pFirst;
        StopIndex = This->NrElements >> 1;
        for (Index = 0; Index < StopIndex; Index++) {
            This->pMedian = This->pMedian->pNext;
        }

        pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
        if (pOut != NULL) {
            *pOut = This->pMedian->Value;
        }
    }
}

/* Releases the sorted sample ring on the destruction command. */
static u8 ControlMedian(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    Median *This = (Median *)_This;

    switch (ControlId) {
    case 0xFF:
        if (This->pMedianList != NULL) {
            heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pMedianList);
        }
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }

    return 1;
}

/* Clears the sorted list when starting a new sample ring. */
static void Median_Reset(Median *This)
{
    This->pMedian = NULL;
    This->pLast = NULL;
    This->pFirst = NULL;
    This->IndexOldestElem = 0;
    This->NrElements = 0;
}

/* Configures the median block and allocates an odd number of sample slots. */
static u8 InitMedian(struct _BLOCK *_This)
{
    Median *This = (Median *)_This;
    u8 bRetVal = 0;

    This->Depth = 0;
    This->NrElements = 0;
    This->IndexOldestElem = 0;
    This->pMedianList = NULL;
    This->pFirst = NULL;
    This->pLast = NULL;
    This->pMedian = NULL;

    This->BaseBlock.InputQueues->Size = 6 * sizeof(f32); /* Only the first float is read. */
    This->BaseBlock.OutputQueues->Size = sizeof(f32);

    This->Depth = _tosGetProfileU32(&This->BaseBlock.Object, 1, 1);
    if ((This->Depth & 1) == 0) {
        _tosErrorLog(&This->BaseBlock.Object, 0xC8);
        bRetVal = 1;
    } else {
        This->pMedianList = heap_Calloc(
            This->BaseBlock.Object.pTos->hHeap,
            This->Depth,
            sizeof(MedianNode));

        Median_Reset(This);
    }

    return bRetVal;
}

/* Registers a median block with the speech graph. */
struct _BLOCK *ConstructMedian(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 1, 1,
        ProcessMedian,
        /* The dispatcher reads only the low byte of the returned word. */
        InitMedian,
        ControlMedian,
        sizeof(Median));
}
