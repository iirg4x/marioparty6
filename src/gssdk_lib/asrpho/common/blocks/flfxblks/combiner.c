// Combines speech energy and band values into a history-based feature frame.
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "gssdk/tos.h"

#include <string.h>

typedef struct Combiner {
    struct _BLOCK BaseBlock; // Queue ports and callbacks used by the speech-processing graph.
    u32 FrameCounter; // Number of input frames seen, adjusted while draining the history.
    u16 Frame; // Slot to write next in the eight-frame circular history.
    u16 DimInVector; // Number of band values carried by each input frame.
    u16 DimOutVector; // Float count in each history slot, including energy and derived values.
    f32 *VectBufPtrs[8]; // Eight circular feature-frame slots, indexed by write position and age.
    f32 *VectBuf; // Allocation backing VectBufPtrs; freed when this block is destroyed.
    u8 Flushing; // Set while delayed output frames are being emitted after a flush request.
} Combiner;

// Sets frame sizes from profile key 2 and allocates the eight-frame history
// when the TOS graph initializes the combiner.
static unsigned char CombinerInit(struct _BLOCK *_This)
{
    Combiner *This = (Combiner *)_This;
    struct _TOS *pTos = _This->Object.pTos;
    f32 *Ptr;
    s32 i;

    // Profile key 2 selects the number of speech bands; 12 is the default.
    This->DimInVector = _tosGetProfileU32(&This->BaseBlock.Object, 2, 12);
    This->DimOutVector = This->DimInVector * 3 + 3;
    This->BaseBlock.InputQueues->Size =
        (This->DimInVector + 2) * sizeof(f32);
    This->BaseBlock.OutputQueues->Size =
        (This->DimOutVector - 1) * sizeof(f32);
    This->VectBuf = heap_Calloc(
        pTos->hHeap, This->DimOutVector * 8, sizeof(f32));
    if (This->VectBuf == NULL) {
        return 2;
    }

    for (i = 0, Ptr = This->VectBuf; i < 8;
         i++, Ptr += This->DimOutVector) {
        This->VectBufPtrs[i] = Ptr;
    }
    This->Frame = 0;
    return 0;
}

// Records input history and queues aligned features after warm-up.
// The TOS scheduler calls this on each graph pass, including input-free flush passes.
static void CombinerProcess(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Combiner *This = (Combiner *)_This;
    f32 *pOutput = NULL;
    f32 *Vect = ((void **)pInputs)[0];
    struct _QUEUE *pOutQ = _This->OutputQueues->pQ;
    s32 i;
    f32 *Ptr;
    f32 *Ptr1;
    f32 *Ptr2;
    f32 *deltaValues;
    f32 *Ptr3;
    f32 *Ptr4;
    f32 Energy;
    u16 Frame;

    if (Vect != NULL) {
        Frame = This->Frame;
        Energy = *Vect++;
        Vect++; // The input layout reserves one float between energy and band data.
        Ptr = This->VectBufPtrs[Frame & 7];
        Ptr1 = This->VectBufPtrs[(Frame + 7) & 7];
        Ptr2 = This->VectBufPtrs[(Frame + 6) & 7];
        deltaValues = Ptr2 + 1;
        Ptr3 = This->VectBufPtrs[(Frame + 5) & 7];
        Ptr4 = This->VectBufPtrs[(Frame + 4) & 7];

        This->FrameCounter++;
        Ptr[0] = Energy;
        pOutput = Ptr3 + 1;
        // Store a weighted four-frame difference for energy and each band; the next
        // feature group stores the change in that difference.
        Ptr2[1] = 0.2f *
                           (2.0f * Energy + Ptr1[0] - Ptr3[0] -
                            2.0f * Ptr4[0]);
        Ptr3[2] = Ptr2[1] - Ptr4[1];

        Ptr += 3;
        Ptr1 += 3;
        Ptr3 += 3;
        Ptr4 += 3;
        deltaValues += This->DimInVector + 2;
        i = 0;
        while (i < This->DimInVector) {
            *Ptr++ = *Vect;
            *deltaValues++ = 0.375f *
                             (2.0f * *Vect++ + *Ptr1++ - *Ptr3++ -
                              2.0f * *Ptr4++);
            i++;
        }

        deltaValues -= This->DimInVector;
        Ptr3 += This->DimInVector;
        for (i = 0; i < This->DimInVector; i++) {
            *Ptr3++ = *deltaValues++ - *Ptr4++;
        }
    }

    if (This->Flushing != 0) {
        This->FrameCounter--;
        if (This->FrameCounter == 4) {
            This->Flushing = 0;
        }
        pOutput = This->VectBufPtrs[(This->Frame + 5) & 7] + 1;
    }

    if (pOutput != NULL && This->FrameCounter > 3) {
        // Writes through the returned slot, which requires an active output queue.
        f32 *OutVect = qEnQueueOne(pOutQ);

        // Warm-up and flush substitute zeroes for unavailable history.
        // Six frames supply the first energy difference and two band groups.
        // Four or five frames supply only the first band group.
        if (This->FrameCounter > 6) {
            for (i = 0; i < This->DimOutVector - 1; i++) {
                *OutVect++ = *pOutput++;
            }
        } else if (This->FrameCounter == 6) {
            *OutVect++ = *pOutput;
            pOutput += 2;
            *OutVect++ = 0.0f;
            for (i = 0; i < This->DimInVector * 2; i++) {
                *OutVect++ = *pOutput++;
            }
            for (i = 0; i < This->DimInVector; i++) {
                *OutVect++ = 0.0f;
            }
        } else {
            *OutVect++ = 0.0f;
            *OutVect++ = 0.0f;
            pOutput += 2;
            for (i = 0; i < This->DimInVector; i++) {
                *OutVect++ = *pOutput++;
            }
            for (i = 0; i < This->DimInVector * 2; i++) {
                *OutVect++ = 0.0f;
            }
        }
    }

    if (pOutput != NULL) {
        This->Frame = (This->Frame + 1) & 7;
    }
}

// Handles flush, reset, and destruction commands sent by the TOS graph.
static unsigned char CombinerControl(struct _BLOCK *_This, unsigned char ControlID,
                                     unsigned long CParam)
{
    Combiner *This = (Combiner *)_This;

    switch (ControlID) {
    case 0x02:
        if (This->FrameCounter == 0) {
            if (CParam == 0 ||
                qCheckInputQueues(This->BaseBlock.InputQueues, 1) == 0) {
                break;
            }
        }
        This->Flushing = 1;
        if (This->FrameCounter < 3) {
            This->FrameCounter += 4;
        } else {
            This->FrameCounter = 7;
        }
        break;
    case 0x01:
        This->FrameCounter = 0;
        memset(
            This->VectBuf, 0,
            This->DimOutVector * 8 * sizeof(f32));
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->VectBuf);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

// Creates the one-input, one-output block used by the speech recognizer.
struct _BLOCK *ConstructCombiner(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 1, 1, CombinerProcess,
        CombinerInit,
        /* CombinerControl reads the command and the first payload word. */
        (u8 (*)(struct _BLOCK *, u8, u32, u32))CombinerControl, sizeof(Combiner));
}
