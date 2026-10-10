// Combines speech energy and band values into a history-based feature frame.
#include "gssdk/tos.h"

#include <string.h>

typedef struct Combiner {
    TosBaseBlock baseBlock; // Queue ports and callbacks used by the speech-processing graph.
    u32 framesProcessed; // Number of input frames seen, adjusted while draining the history.
    u16 writeFrameIndex; // Slot to write next in the eight-frame circular history.
    u16 bandCount; // Number of band values carried by each input frame.
    u16 frameVectorLength; // Float count in each history slot, including energy and derived values.
    f32 *frameHistory[8]; // Eight circular feature-frame slots, indexed by write position and age.
    f32 *historyStorage; // Allocation backing frameHistory; freed when this block is destroyed.
    u8 isFlushing; // Set while delayed output frames are being emitted after a flush request.
} Combiner;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern u8 qCheckInputQueues(TosQueuePort *input, u32 count);

// Sets frame sizes from profile key 2 and allocates the eight-frame history
// when the TOS graph initializes the combiner.
static u32 CombinerInit(TosBaseBlock *baseBlock)
{
    Combiner *block = (Combiner *)baseBlock;
    TosContext *context = baseBlock->context;
    f32 *frameStorage;
    s32 historySlot;

    // Profile key 2 selects the number of speech bands; 12 is the default.
    block->bandCount = _tosGetProfileU32(block, 2, 12);
    block->frameVectorLength = block->bandCount * 3 + 3;
    block->baseBlock.input->inputSize =
        (block->bandCount + 2) * sizeof(f32);
    block->baseBlock.output->outputSize =
        (block->frameVectorLength - 1) * sizeof(f32);
    block->historyStorage = heap_Calloc(
        context->heap, block->frameVectorLength * 8, sizeof(f32));
    if (block->historyStorage == NULL) {
        return 2;
    }

    for (historySlot = 0, frameStorage = block->historyStorage; historySlot < 8;
         historySlot++, frameStorage += block->frameVectorLength) {
        block->frameHistory[historySlot] = frameStorage;
    }
    block->writeFrameIndex = 0;
    return 0;
}

// Process an input frame into the history and queue an aligned feature vector after warm-up.
// The TOS scheduler calls this on each graph pass, including input-free flush passes.
static void CombinerProcess(
    TosBaseBlock *baseBlock, void **inputs, s32 inputPortCount)
{
    Combiner *block = (Combiner *)baseBlock;
    f32 *outputFrame = NULL;
    f32 *inputFrame = inputs[0];
    TosQueue *outputQueue = baseBlock->output->queue;
    s32 bandIndex;
    f32 *currentHistoryFrame;
    f32 *oneFrameBack;
    f32 *twoFramesBack;
    f32 *deltaValues;
    f32 *threeFramesBack;
    f32 *fourFramesBack;
    f32 energyValue;
    u16 frameIndex;

    if (inputFrame != NULL) {
        frameIndex = block->writeFrameIndex;
        energyValue = *inputFrame++;
        inputFrame++; // The input layout reserves one float between energy and band data.
        currentHistoryFrame = block->frameHistory[frameIndex & 7];
        oneFrameBack = block->frameHistory[(frameIndex + 7) & 7];
        twoFramesBack = block->frameHistory[(frameIndex + 6) & 7];
        deltaValues = twoFramesBack + 1;
        threeFramesBack = block->frameHistory[(frameIndex + 5) & 7];
        fourFramesBack = block->frameHistory[(frameIndex + 4) & 7];

        block->framesProcessed++;
        currentHistoryFrame[0] = energyValue;
        outputFrame = threeFramesBack + 1;
        // Store a weighted four-frame difference for energy and each band; the next
        // feature group stores the change in that difference.
        twoFramesBack[1] = 0.2f *
                           (2.0f * energyValue + oneFrameBack[0] - threeFramesBack[0] -
                            2.0f * fourFramesBack[0]);
        threeFramesBack[2] = twoFramesBack[1] - fourFramesBack[1];

        currentHistoryFrame += 3;
        oneFrameBack += 3;
        threeFramesBack += 3;
        fourFramesBack += 3;
        deltaValues += block->bandCount + 2;
        bandIndex = 0;
        while (bandIndex < block->bandCount) {
            *currentHistoryFrame++ = *inputFrame;
            *deltaValues++ = 0.375f *
                             (2.0f * *inputFrame++ + *oneFrameBack++ - *threeFramesBack++ -
                              2.0f * *fourFramesBack++);
            bandIndex++;
        }

        deltaValues -= block->bandCount;
        threeFramesBack += block->bandCount;
        for (bandIndex = 0; bandIndex < block->bandCount; bandIndex++) {
            *threeFramesBack++ = *deltaValues++ - *fourFramesBack++;
        }
    }

    if (block->isFlushing != 0) {
        block->framesProcessed--;
        if (block->framesProcessed == 4) {
            block->isFlushing = 0;
        }
        outputFrame = block->frameHistory[(block->writeFrameIndex + 5) & 7] + 1;
    }

    if (outputFrame != NULL && block->framesProcessed > 3) {
        // qEnQueueOne returns NULL for an idle unbounded queue; this block writes
        // the returned slot directly, so the graph must activate its output queue.
        f32 *outputValues = qEnQueueOne(outputQueue);

        // During warm-up and flush, zero-fill missing history: counts 4–5 lack both
        // energy-difference slots and the final two band groups; count 6 lacks the
        // second energy-difference slot and final band group.
        if (block->framesProcessed > 6) {
            for (bandIndex = 0; bandIndex < block->frameVectorLength - 1; bandIndex++) {
                *outputValues++ = *outputFrame++;
            }
        } else if (block->framesProcessed == 6) {
            *outputValues++ = *outputFrame;
            outputFrame += 2;
            *outputValues++ = 0.0f;
            for (bandIndex = 0; bandIndex < block->bandCount * 2; bandIndex++) {
                *outputValues++ = *outputFrame++;
            }
            for (bandIndex = 0; bandIndex < block->bandCount; bandIndex++) {
                *outputValues++ = 0.0f;
            }
        } else {
            *outputValues++ = 0.0f;
            *outputValues++ = 0.0f;
            outputFrame += 2;
            for (bandIndex = 0; bandIndex < block->bandCount; bandIndex++) {
                *outputValues++ = *outputFrame++;
            }
            for (bandIndex = 0; bandIndex < block->bandCount * 2; bandIndex++) {
                *outputValues++ = 0.0f;
            }
        }
    }

    if (outputFrame != NULL) {
        block->writeFrameIndex = (block->writeFrameIndex + 1) & 7;
    }
}

// Handles flush, reset, and destruction commands sent by the TOS graph.
static u32 CombinerControl(
    TosBaseBlock *baseBlock, u32 command, void *controlArgument, u32 controlArgumentSize)
{
    Combiner *block = (Combiner *)baseBlock;

    switch ((u8)command) {
    case 2:
        if (block->framesProcessed == 0) {
            if (controlArgument == NULL ||
                qCheckInputQueues(block->baseBlock.input, 1) == 0) {
                break;
            }
        }
        block->isFlushing = 1;
        if (block->framesProcessed < 3) {
            block->framesProcessed += 4;
        } else {
            block->framesProcessed = 7;
        }
        break;
    case 1:
        block->framesProcessed = 0;
        memset(
            block->historyStorage, 0,
            block->frameVectorLength * 8 * sizeof(f32));
        break;
    case 255:
        heap_Free(block->baseBlock.context->heap, block->historyStorage);
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }
    return 1;
}

// Creates the one-input, one-output block used by the speech recognizer.
void *ConstructCombiner(TosContext *context, u32 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 1, 1, CombinerProcess, CombinerInit,
        CombinerControl, sizeof(Combiner));
}
