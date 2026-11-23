/* Manages TOS queues, their reader positions and their element storage. */
#include "gssdk/fastallo.h"
#include "gssdk/heap.h"
#include "gssdk/mqueue.h"

#include <stddef.h>
#include <string.h>

/* Returns a queue element to its small-block pool or the heap. */
static void qFreeElem(struct _QUEUE *Q, struct _QUEUEELEM *pElem)
{
    u32 Size = Q->Size + offsetof(struct _QUEUEELEM, Data);

    if (Size < 0x20) {
        struct _FASTALLO_FREE_NODE **pTemp = Q->Object.pTos->FastAllocHeap.pFreeNodes +
            ((Size + 3) >> 2);

        ((struct _FASTALLO_FREE_NODE *)pElem)->pNext = *pTemp;
        *pTemp = (struct _FASTALLO_FREE_NODE *)pElem;
    } else {
        heap_Free(Q->Object.pTos->hHeap, pElem);
    }
}

/* Changes the queue state or a reader position for a control command. */
u8 qQueueControl(struct _QUEUE *Q, u8 ControlID, u32 CParam, u32 DParam)
{
    struct _QUEUEELEM *reader;
    u32 i;
    u8 RetVal = 1;

    switch (ControlID) {
    case 0x1:
    case 0xB:
        Q->State = 0x1;
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
        }
        break;
    case 0x2:
        Q->State = 0x1;
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
        }
        RetVal = qQueueJumpBack(Q, Q->MaxJumpBack);
        break;
    case 0x3:
        Q->State = 0x1;
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
        }
        if (Q->MaxJumpBack != 0) {
            RetVal = qQueueJumpBack(Q, CParam);
        }
        break;
    case 0x7:
        Q->State = 0x1;
        reader = Q->pReadPtrs[CParam];
        if (reader == NULL || reader == (struct _QUEUEELEM *)&Q->pElements) {
            Q->pReadPtrs[CParam] = (struct _QUEUEELEM *)Q;
            if (Q->MaxJumpBack != 0) {
                RetVal = qQueueJumpBackOne(Q, CParam, DParam);
            }
        }
        break;
    case 0x5:
    case 0xA:
        Q->State = 0x1;
        reader = Q->pReadPtrs[CParam];
        if (reader == NULL || reader == (struct _QUEUEELEM *)&Q->pElements) {
            Q->pReadPtrs[CParam] = (struct _QUEUEELEM *)Q;
        }
        break;
    case 0x4:
        Q->State = 0x2;
        for (i = 0; i < Q->NbrReaders; i++) {
            reader = Q->pReadPtrs[i];
            if (reader != NULL &&
                reader != (struct _QUEUEELEM *)&Q->pElements) {
                Q->pReadPtrs[i] = NULL;
            }
        }
        break;
    case 0x9:
        Q->State = 0x2;
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)&Q->pElements;
        }
        break;
    case 0x6:
        Q->pReadPtrs[CParam] = NULL;
        i = 0;
        while (i < Q->NbrReaders) {
            reader = Q->pReadPtrs[i];
            if (reader == NULL || reader == (struct _QUEUEELEM *)&Q->pElements) {
                i++;
            } else {
                break;
            }
        }
        if (i == Q->NbrReaders) {
            Q->State = 0x2;
        }
        break;
    case 0x8:
        Q->pReadPtrs[CParam] = (struct _QUEUEELEM *)&Q->pElements;
        i = 0;
        while (i < Q->NbrReaders) {
            reader = Q->pReadPtrs[i];
            if (reader == NULL || reader == (struct _QUEUEELEM *)&Q->pElements) {
                i++;
            } else {
                break;
            }
        }
        if (i == Q->NbrReaders) {
            Q->State = 0x2;
        }
        break;
    case 0xF8:
        break;
    case 0xFF:
        if (DestructQueue(Q) != 0) {
            RetVal = 0;
        }
        break;
    default:
        RetVal = 0;
        break;
    }
    return RetVal;
}

/* Positions active readers at the requested distance from the end of the queue. */
u8 qQueueJumpBack(struct _QUEUE *Q, u16 JumpBack)
{
    struct _QUEUEELEM *pElem;
    u32 NbrElems;
    u32 i;

    if (JumpBack != 0) {
        NbrElems = 0;
        pElem = Q->pElements;
        while (pElem != NULL) {
            pElem = pElem->pNext;
            NbrElems++;
        }
        if (NbrElems < JumpBack) {
            return 0;
        }

        pElem = Q->pElements;
        NbrElems -= JumpBack;
        for (i = 0; i < NbrElems; i++) {
            pElem = pElem->pNext;
        }

        for (i = 0; i < Q->NbrReaders; i++) {
            if (Q->pReadPtrs[i] != (struct _QUEUEELEM *)&Q->pElements &&
                Q->pReadPtrs[i] != NULL) {
                Q->pReadPtrs[i] = pElem;
            }
        }
        qFreeUnusedElemements(Q);
    }
    return 1;
}

/* Moves one reader position back by the requested element count. */
u8 qQueueJumpBackOne(struct _QUEUE *Q, u16 ReadPos, u16 JumpBack)
{
    struct _QUEUEELEM *pElem;
    u32 NbrElems;
    u32 i;

    if (JumpBack != 0) {
        NbrElems = 0;
        pElem = Q->pElements;
        while (pElem != NULL) {
            pElem = pElem->pNext;
            NbrElems++;
        }
        if (NbrElems < JumpBack) {
            return 0;
        }

        pElem = Q->pElements;
        NbrElems -= JumpBack;
        for (i = 0; i < NbrElems; i++) {
            pElem = pElem->pNext;
        }
        Q->pReadPtrs[ReadPos] = pElem;
    }
    return 1;
}

/* Checks the element size, resets the queue and sets the reader cursors. */
struct _QUEUE *qQueueInit(struct _QUEUE *Q)
{
    u32 profileElementSize;
    u32 i;

    profileElementSize = _tosGetProfileU32(&Q->Object, 2, -1);
    if (profileElementSize != (u32)-1 &&
        profileElementSize != Q->Size) {
        return NULL;
    }

    qQueueReset(Q);
    if ((u8)_tosGetProfileU32(&Q->Object, 3, 1) != 0) {
        for (i = 0; i < Q->NbrReaders; i++) {
            if (Q->pReadPtrs[i] != (struct _QUEUEELEM *)&Q->pElements) {
                Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
            }
        }
    } else {
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = NULL;
        }
    }
    Q->MaxJumpBack = (u8)_tosGetProfileU32(&Q->Object, 4, 0);
    return Q;
}

/* Initializes the queue's element size, backtrack limit and reader cursors. */
struct _QUEUE *qQueueInitEx(struct _QUEUE *Q, u16 NewSize, u8 PassThrough, u16 NumberToJumpBack)
{
    u32 i;

    NewSize = NewSize != 0
        ? NewSize
        : (u16)_tosGetProfileU32(&Q->Object, 2, -1);
    Q->Size = NewSize;
    qQueueReset(Q);

    if (PassThrough == 0xFF) {
        PassThrough = (u8)_tosGetProfileU32(&Q->Object, 3, 1);
    }
    if (PassThrough != 0) {
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
        }
    } else {
        for (i = 0; i < Q->NbrReaders; i++) {
            Q->pReadPtrs[i] = NULL;
        }
    }

    if (NumberToJumpBack != 0xFFFF) {
        Q->MaxJumpBack = NumberToJumpBack;
    } else {
        Q->MaxJumpBack = _tosGetProfileU32(&Q->Object, 4, 0);
    }
    return Q;
}

/* Frees the queued elements and sets the new element size. */
struct _QUEUE *qQueueReDimension(struct _QUEUE *Q, u16 NewSize)
{
    u16 newElementSize;
    struct _QUEUEELEM *element;

    element = Q->pElements;
    if (NewSize != 0) {
        newElementSize = NewSize;
    } else {
        newElementSize = (u16)_tosGetProfileU32(&Q->Object, 2, -1);
    }
    while (element != NULL) {
        struct _QUEUEELEM *current = element;

        element = element->pNext;
        qFreeElem(Q, current);
    }
    Q->pElements = NULL;
    Q->Size = newElementSize;
    return Q;
}

/* Trims older elements and advances readers waiting for new elements. */
static void qUpdateReadPtrsAndIncNbrOfEnqueues(struct _QUEUE *Q, struct _QUEUEELEM *pElem)
{
    struct _QUEUEELEM *pInnerElem;
    u32 NbrElems;
    u32 i;

    if (Q->MaxJumpBack != 0) {
        NbrElems = 0;
        pInnerElem = Q->pElements;
        while (pInnerElem != NULL) {
            pInnerElem = pInnerElem->pNext;
            NbrElems++;
        }

        if (NbrElems > Q->MaxJumpBack) {
            NbrElems -= Q->MaxJumpBack;
            pInnerElem = Q->pElements;
            for (i = 0; i < NbrElems; i++) {
                unsigned long k;
                struct _QUEUEELEM *pToFree = pInnerElem;
                for (k = 0; k < Q->NbrReaders; k++) {
                    if (Q->pReadPtrs[k] == pToFree) {
                        break;
                    }
                }
                if (k < Q->NbrReaders) {
                    break;
                }
                pInnerElem = pInnerElem->pNext;
                {
                    u32 size = Q->Size +
                        offsetof(struct _QUEUEELEM, Data);

                    if (size < 0x20) {
                        struct _FASTALLO_FREE_NODE **freeList =
                            Q->Object.pTos->FastAllocHeap.pFreeNodes +
                            ((size + 3) >> 2);

                        ((struct _FASTALLO_FREE_NODE *)pToFree)->pNext = *freeList;
                        *freeList = (struct _FASTALLO_FREE_NODE *)pToFree;
                    } else {
                        heap_Free(Q->Object.pTos->hHeap, pToFree);
                    }
                }
            }
            Q->pElements = pInnerElem;
        }
    }

    for (i = 0; i < Q->NbrReaders; i++) {
        if (Q->pReadPtrs[i] == (struct _QUEUEELEM *)Q) {
            Q->pReadPtrs[i] = pElem;
        }
    }
    Q->Object.pTos->NbrOfEnqueues++;
}

/* Obtains an element slot from the queue's storage. */
static struct _QUEUEELEM *qAllocElem(struct _QUEUE *Q)
{
    struct _QUEUEELEM *pElem;
    unsigned long Size = Q->Size + offsetof(struct _QUEUEELEM, Data);

    if (Size < 0x20) {
        unsigned int TempSize = (Size + 3) >> 2;
        struct _FASTALLO_FREE_NODE **pTemp =
            Q->Object.pTos->FastAllocHeap.pFreeNodes + TempSize;
        struct _QUEUEELEM **ppElem = &pElem;

        if (*pTemp != NULL) {
            *ppElem = (void *)*pTemp;
            *pTemp = (*pTemp)->pNext;
        } else {
            *ppElem = fastallo_AllocateMemoryFromChunk(
                &Q->Object.pTos->FastAllocHeap, TempSize);
        }
    } else {
        pElem = heap_Alloc(Q->Object.pTos->hHeap, Size);
    }
    return pElem;
}

/* Enqueues a batch of elements from the caller's data buffer. */
u8 qEnQueue(struct _QUEUE *Q, void *pData, u32 NbrOfElements)
{
    struct _QUEUEELEM *pLast;
    struct _QUEUEELEM *pFirstEnq = NULL;
    struct _QUEUEELEM *pElem;
    u32 i;
    u8 *DataPtr = pData;
    struct _QUEUEELEM *pInnerElem;

    if (Q->State == 0x2 && Q->MaxJumpBack == 0) {
        return 0;
    }

    pElem = Q->pElements;
    if (pElem != NULL) {
        while (pElem->pNext != NULL) {
            pElem = pElem->pNext;
        }
        pLast = pElem;
    } else {
        pLast = NULL;
    }

    for (i = 0; i < NbrOfElements; i++) {
        pInnerElem = qAllocElem(Q);
        if (pFirstEnq == NULL) {
            pFirstEnq = pInnerElem;
        }
        if (pLast != NULL) {
            pLast->pNext = pInnerElem;
        } else {
            Q->pElements = pInnerElem;
        }
        pLast = pInnerElem;
        memcpy(pInnerElem->Data, DataPtr, Q->Size);
        DataPtr += Q->Size;
    }
    if (pLast != NULL) {
        pLast->pNext = NULL;
    }
    qUpdateReadPtrsAndIncNbrOfEnqueues(Q, pFirstEnq);
    return 1;
}

/* Appends one queue element and returns its data slot. */
void *qEnQueueOne(struct _QUEUE *Q)
{
    struct _QUEUEELEM *element;
    struct _QUEUEELEM *tail;

    if (Q->State == 0x2 && Q->MaxJumpBack == 0) {
        return NULL;
    }

    element = qAllocElem(Q);
    tail = Q->pElements;
    if (tail != NULL) {
        while (tail->pNext != NULL) {
            tail = tail->pNext;
        }
        tail->pNext = element;
    } else {
        Q->pElements = element;
    }
    element->pNext = NULL;
    qUpdateReadPtrsAndIncNbrOfEnqueues(Q, element);
    return element->Data;
}

/* Reads the next element for the specified queue reader. */
void *qDeQueueOne(struct _QUEUE *Q, u32 Pos)
{
    struct _QUEUEELEM *element = Q->pReadPtrs[Pos];

    if (element == (struct _QUEUEELEM *)&Q->pElements || element == NULL ||
        element == (struct _QUEUEELEM *)Q) {
        return NULL;
    }

    if (element->pNext != NULL) {
        Q->pReadPtrs[Pos] = element->pNext;
    } else {
        Q->pReadPtrs[Pos] = (struct _QUEUEELEM *)Q;
    }
    return element->Data;
}

/* Releases the queued elements that are no longer needed by any reader. */
void qFreeUnusedElemements(struct _QUEUE *Q)
{
    struct _QUEUEELEM *element = Q->pElements;
    struct _QUEUEELEM *current;
    u32 i;

    if (element == NULL) {
        return;
    }

    if (Q->MaxJumpBack != 0) {
        for (i = 0; i < Q->NbrReaders; i++) {
            struct _QUEUEELEM *pElem = Q->pReadPtrs[i];

            if (pElem != (struct _QUEUEELEM *)&Q->pElements && pElem == NULL) {
                return;
            }
        }
    }

    while (element != NULL) {
        for (i = 0; i < Q->NbrReaders; i++) {
            if (Q->pReadPtrs[i] == element) {
                break;
            }
        }
        if (i != Q->NbrReaders) {
            break;
        }
        current = element;
        element = element->pNext;
        qFreeElem(Q, current);
    }
    if (element == NULL) {
        Q->pElements = NULL;
        return;
    }
    Q->pElements = element;
}

/* Dequeues one element per input port when the input queues are ready. */
u8 qCheckDeQueueOne(const struct _INPUTQUEUE *InputQueues, u32 NbrQueues, void *ppData)
{
    struct _QUEUE *Q;
    struct _QUEUEELEM *reader;
    unsigned int i;

    if (qCheckInputQueues(InputQueues, NbrQueues) == 0) {
        memset(ppData, 0, NbrQueues * sizeof(void *));
        return 0;
    }

    for (i = 0; i < NbrQueues; i++) {
        void *pData = NULL;

        Q = InputQueues->pQ;
        if (Q != NULL) {
            reader = Q->pReadPtrs[InputQueues->ReadPos];
            if (reader == (struct _QUEUEELEM *)&Q->pElements || reader == NULL ||
                reader == (struct _QUEUEELEM *)Q) {
                pData = NULL;
            } else {
                if (reader->pNext != NULL) {
                    Q->pReadPtrs[InputQueues->ReadPos] = reader->pNext;
                } else {
                    Q->pReadPtrs[InputQueues->ReadPos] =
                        (struct _QUEUEELEM *)Q;
                }
                pData = reader->Data;
            }
        }
        *(void **)ppData = pData;
        ppData = (void **)ppData + 1;
        InputQueues++;
    }
    return 1;
}

/* Checks whether the input queues are ready for processing. */
u8 qCheckInputQueues(const struct _INPUTQUEUE *InputQueues, u32 NbrQueues)
{
    const struct _INPUTQUEUE *InQueue;
    const struct _INPUTQUEUE *InQueueEnd;
    struct _QUEUE *Q;
    struct _QUEUEELEM *reader;
    u8 Empty = 1;

    InQueue = InputQueues;
    InQueueEnd = InputQueues + NbrQueues;
    while (InQueue < InQueueEnd) {
        Q = InQueue->pQ;
        if (Q != NULL) {
            if (Q->pReadPtrs[InQueue->ReadPos] != NULL &&
                Q->pReadPtrs[InQueue->ReadPos] != (struct _QUEUEELEM *)&Q->pElements) {
                Empty = Q->pReadPtrs[InQueue->ReadPos] == (struct _QUEUEELEM *)Q;
                break;
            }
        }
        InQueue++;
    }
    /* The second scan starts after the first eligible reader. */
    InQueue++;

    for (; InQueue < InQueueEnd; InQueue++) {
        Q = InQueue->pQ;
        if (Q == NULL) {
            continue;
        }
        reader = Q->pReadPtrs[InQueue->ReadPos];
        if (reader == NULL || reader == (struct _QUEUEELEM *)&Q->pElements) {
            continue;
        }
        {
            u8 readerWaiting = reader == (struct _QUEUEELEM *)Q;

            if (Empty != readerWaiting) {
                break;
            }
        }
    }
    if (InQueue != InQueueEnd) {
        return 0;
    }
    return !Empty;
}

/* Resets the queue to its empty state. */
void qQueueReset(struct _QUEUE *Q)
{
    struct _QUEUEELEM *element;
    struct _QUEUEELEM *current;
    u32 i;

    element = Q->pElements;
    while (element != NULL) {
        current = element;
        element = element->pNext;
        qFreeElem(Q, current);
    }

    for (i = 0; i < Q->NbrReaders; i++) {
        if (Q->pReadPtrs[i] != (struct _QUEUEELEM *)&Q->pElements) {
            Q->pReadPtrs[i] = (struct _QUEUEELEM *)Q;
        }
    }
    Q->pElements = NULL;
    Q->State = 0x1;
}

/* Returns the current count of queued elements. */
u32 qQueueNbrElements(struct _QUEUE *Q)
{
    struct _QUEUEELEM *element = Q->pElements;
    u32 count = 0;

    while (element != NULL) {
        element = element->pNext;
        count++;
    }
    return count;
}

/* Allocates a queue record with room for all its reader cursors. */
struct _QUEUE *qQueueConstruct(struct _TOS *pTos, u8 BlockID, u8 NbrReaders)
{
    struct _OBJECT Object;
    struct _QUEUE *Q;

    Q = heap_Calloc(pTos->hHeap, 1,
        sizeof(struct _QUEUE) + (NbrReaders - 1) * sizeof(struct _QUEUEELEM *));
    if (Q == NULL) {
        Object.pTos = pTos;
        Object.BlockID = BlockID;
        _tosErrorLog(&Object, 0x2);
        return NULL;
    }

    Q->NbrReaders = NbrReaders;
    Q->Object.BlockID = BlockID;
    Q->Object.pTos = pTos;
    return Q;
}
