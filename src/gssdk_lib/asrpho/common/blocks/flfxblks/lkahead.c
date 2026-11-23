/* Look-ahead block that delays frames until future context is available. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include "gssdk/tos.h"

/* Rolling sum and queued decisions for the look-ahead block. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u8 Flushing; /* Set while queued decisions are drained. */
    u32 LookAhead; /* Number of frames held before emitting decisions. */
    u32 Span; /* Length of the rolling sample history. */
    f32 Threshold; /* Threshold compared with the rolling sum. */
    f32 *pBegin; /* First entry in the sample history. */
    f32 *pWrite; /* Next sample to replace. */
    f32 *pEnd; /* One past the last history entry. */
    f32 Sum; /* Rolling sum of the sample history. */
    u32 NbrFrames; /* Frames held for delayed output. */
} LookAhead;

/* Updates the rolling sum and queues threshold decisions. */
static void ProcessLookAhead(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    LookAhead *This = (LookAhead *) _This;
    f32 *pFrameType = ((void **) pInputs)[0];
    u8 *pBoolOut;

    if (pFrameType != NULL) {
        This->Sum -= *This->pWrite;
        *This->pWrite++ = *pFrameType;
        This->Sum += *pFrameType;
        if (This->pWrite == This->pEnd) {
            This->pWrite = This->pBegin;
        }
        if (This->NbrFrames == This->LookAhead) {
            pBoolOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
            *pBoolOut = This->Sum > This->Threshold;
        } else {
            This->NbrFrames++;
        }
    } else if (This->Flushing != 0) {
        while (This->NbrFrames != 0) {
            pBoolOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
            *pBoolOut = This->Sum > This->Threshold;
            This->NbrFrames--;
        }
        if (This->NbrFrames == 0) {
            This->Flushing = 0;
        }
    }
}

/* Requests a flush or releases the sample history. */
static u8 ControlLookAhead(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    LookAhead *This = (LookAhead *) _This;

    switch (ControlId) {
    case 0x01:
        This->Flushing = 1;
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pBegin);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Loads the look-ahead profile and allocates the sample history. */
static unsigned char InitLookAhead(struct _BLOCK *_This)
{
    LookAhead *This = (LookAhead *) _This;
    struct _TOS *pTos = _This->Object.pTos;

    This->LookAhead = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 1, 0);
    This->Span = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 2, 15);
    This->Threshold = _tosGetProfileFloat(&This->BaseBlock.Object, 3, 1.0f);
    This->BaseBlock.InputQueues->Size = sizeof(f32);
    This->BaseBlock.OutputQueues->Size = sizeof(u8);
    This->pBegin = heap_Calloc(pTos->hHeap, This->Span, sizeof(f32));
    This->pWrite = This->pBegin;
    This->pEnd = This->pBegin + This->Span;
    This->Sum = 0.0f;
    This->NbrFrames = 0;
    return 0;
}

/* Registers the look-ahead block with the scheduler. */
struct _BLOCK *ConstructLookAhead(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1, ProcessLookAhead, InitLookAhead,
                                 ControlLookAhead,
                                 sizeof(LookAhead));
}
