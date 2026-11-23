/* Delays input frames by the configured queue depth. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

/* Circular delay buffer and its priming and flush state. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 TotalBufferSize; /* Number of bytes allocated for the ring. */
    u16 BufferLength; /* Number of frames held in the ring. */
    u16 BufferSize; /* Byte size of a frame. */
    /* Ring bounds and the next frame to emit or replace. */
    u8 *pBufferBegin;
    u8 *pBufferRead;
    u8 *pBufferWrite;
    u8 *pBufferEnd;
    /* Priming state and the number of input frames buffered so far. */
    u8 FirstFramesFlag;
    u16 FirstFramesCounter;
    /* Flush state and the number of frames still to emit. */
    u8 FlushFlag;
    u16 NbrOfFramesStillToFlush;
} DelayBlock;

/* Emits one buffered frame while a flush is pending. */
static unsigned char FlushDelayBlock(DelayBlock *This)
{
    u8 *pOut;

    if (This->NbrOfFramesStillToFlush == 0) {
        This->FlushFlag = 0;
        return 0;
    }
    pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    memcpy(pOut, This->pBufferRead, This->BufferSize);
    This->pBufferRead += This->BufferSize;
    if (This->pBufferRead == This->pBufferEnd) {
        This->pBufferRead = This->pBufferBegin;
    }
    This->NbrOfFramesStillToFlush--;
    return 1;
}

/* Queues delayed frames and drains the buffer when input ends. */
static void ProcessDelayBlock(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    DelayBlock *This = (DelayBlock *) _This;
    u8 *pIn = ((void **)pInputs)[0];
    u8 *pOut;

    if (pIn != NULL) {
        if (This->FirstFramesFlag == 1) {
            memcpy(This->pBufferWrite, pIn, This->BufferSize);
            This->pBufferWrite += This->BufferSize;
            if (This->pBufferWrite == This->pBufferEnd) {
                This->pBufferWrite = This->pBufferBegin;
            }
            if (++This->FirstFramesCounter == This->BufferLength) {
                This->FirstFramesFlag = 0;
            }
        } else {
            pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
            memcpy(pOut, This->pBufferRead, This->BufferSize);
            This->pBufferRead += This->BufferSize;
            if (This->pBufferRead == This->pBufferEnd) {
                This->pBufferRead = This->pBufferBegin;
            }
            memcpy(This->pBufferWrite, pIn, This->BufferSize);
            This->pBufferWrite += This->BufferSize;
            if (This->pBufferWrite == This->pBufferEnd) {
                This->pBufferWrite = This->pBufferBegin;
            }
        }
    } else if (This->FlushFlag == 1) {
        FlushDelayBlock(This);
    }
}

/* Starts a new delay interval and clears the flush state. */
static void DelayBlockReset(DelayBlock *This)
{
    unsigned long i;

    This->pBufferWrite = This->pBufferBegin;
    /* Each iteration writes to the same byte. */
    for (i = 0; i < This->TotalBufferSize; i++) {
        *This->pBufferWrite = 0;
    }
    This->pBufferRead = This->pBufferBegin;
    This->pBufferWrite = This->pBufferBegin;
    This->FirstFramesFlag = 1;
    This->FirstFramesCounter = 0;
    This->FlushFlag = 0;
    This->NbrOfFramesStillToFlush = 0;
}

/* Handles reset and flush commands for the frame delay. */
static u8 ControlDelayBlock(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    DelayBlock *This = (DelayBlock *) _This;

    switch (ControlId) {
    case 0x65:
        This->FlushFlag = 1;
        This->NbrOfFramesStillToFlush = This->FirstFramesCounter;
        break;
    case 0x64:
        DelayBlockReset(This);
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pBufferBegin);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Reads the profile and allocates the frame buffer. */
static unsigned char InitDelayBlock(struct _BLOCK *_This)
{
    DelayBlock *This = (DelayBlock *) _This;
    struct _TOS *pTos;

    pTos = _This->Object.pTos;

    This->BufferLength = _tosGetProfileU32(&This->BaseBlock.Object, 1, 1);
    This->BufferSize = _tosGetProfileU32(&This->BaseBlock.Object, 2, 4);
    This->TotalBufferSize = This->BufferLength * This->BufferSize;
    This->pBufferBegin = heap_Calloc(pTos->hHeap, This->BufferLength, This->BufferSize);
    if (This->pBufferBegin == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }
    This->pBufferRead = This->pBufferBegin;
    This->pBufferWrite = This->pBufferBegin;
    This->pBufferEnd = This->pBufferBegin + This->TotalBufferSize;
    This->BaseBlock.InputQueues->Size = This->BufferSize;
    This->BaseBlock.OutputQueues->Size = This->BufferSize;
    This->FirstFramesFlag = 1;
    This->FirstFramesCounter = 0;
    This->FlushFlag = 0;
    This->NbrOfFramesStillToFlush = 0;
    return 0;
}

/* Allocates and initializes a delay block. */
struct _BLOCK *ConstructDelayBlock(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1, ProcessDelayBlock, InitDelayBlock,
                                 ControlDelayBlock,
                                 sizeof(DelayBlock));
}
