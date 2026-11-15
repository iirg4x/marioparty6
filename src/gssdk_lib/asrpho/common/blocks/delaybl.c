#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

typedef struct DelayBlock {
TosBaseBlock base;
u32 bufferSize;
u16 delayCount;
u16 elementSize;
u8 *buffer;
u8 *readPtr;
u8 *writePtr;
u8 *bufferEnd;
u8 priming;
u16 filledFrames;
u8 flushActive;
u16 flushRemaining;
} DelayBlock;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);

static void ProcessDelayBlock(
    TosBaseBlock *baseBlock, void **input, s32 inputCount)
{
DelayBlock *block = (DelayBlock *)baseBlock;
void *source = *input;
void *destination;

if (source != NULL) {
    if (block->priming == 1) {
        memcpy(block->writePtr, source, block->elementSize);
        block->writePtr += block->elementSize;
        if (block->writePtr == block->bufferEnd) {
            block->writePtr = block->buffer;
        }
        if (++block->filledFrames == block->delayCount) {
            block->priming = 0;
        }
    } else {
        destination = qEnQueueOne(block->base.output->queue);
        memcpy(destination, block->readPtr, block->elementSize);
        block->readPtr += block->elementSize;
        if (block->readPtr == block->bufferEnd) {
            block->readPtr = block->buffer;
        }
        memcpy(block->writePtr, source, block->elementSize);
        block->writePtr += block->elementSize;
        if (block->writePtr == block->bufferEnd) {
            block->writePtr = block->buffer;
        }
    }
} else if (block->flushActive == 1) {
    if (block->flushRemaining == 0) {
        block->flushActive = 0;
    } else {
        destination = qEnQueueOne(block->base.output->queue);
        memcpy(destination, block->readPtr, block->elementSize);
        block->readPtr += block->elementSize;
        if (block->readPtr == block->bufferEnd) {
            block->readPtr = block->buffer;
        }
        block->flushRemaining--;
    }
}
}

static void FillDelayBlock(DelayBlock *block, u8 value)
{
    u32 i;

    for (i = 0; i < block->bufferSize; i++) {
        *block->writePtr = value;
    }
}

static u32 ControlDelayBlock(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
DelayBlock *block = (DelayBlock *)baseBlock;

switch ((u8)command) {
case 101:
    block->flushActive = 1;
    block->flushRemaining = block->filledFrames;
    break;
case 100:
    block->writePtr = block->buffer;
    FillDelayBlock(block, 0);
    block->readPtr = block->buffer;
    block->writePtr = block->buffer;
    block->priming = 1;
    block->filledFrames = 0;
    block->flushActive = 0;
    block->flushRemaining = 0;
    break;
case 255:
    heap_Free(block->base.context->heap, block->buffer);
    tosBaseBlockDestruct(block);
    break;
default:
    return 0;
}
return 1;
}

static u32 InitDelayBlock(TosBaseBlock *_This)
{
DelayBlock *This = (DelayBlock *)_This;
TosContext *context;

context = _This->context;

This->delayCount = _tosGetProfileU32(This, 1, 1);
This->elementSize = _tosGetProfileU32(This, 2, 4);
This->bufferSize = This->delayCount * This->elementSize;
This->buffer = heap_Calloc(
    context->heap, This->delayCount, This->elementSize);
if (This->buffer == NULL) {
    _tosErrorLog(This, 2);
    return 1;
}
This->readPtr = This->buffer;
This->writePtr = This->buffer;
This->bufferEnd = This->buffer + This->bufferSize;
This->base.input->inputSize = This->elementSize;
This->base.output->outputSize = This->elementSize;
This->priming = 1;
This->filledFrames = 0;
This->flushActive = 0;
This->flushRemaining = 0;
return 0;
}

void *ConstructDelayBlock(TosContext *context, u8 blockIndex)
{
return tosBaseBlockConstruct(
    context, blockIndex, 1, 1, (TosProcessFunction)ProcessDelayBlock,
    (TosInitFunction)InitDelayBlock, (TosControlFunction)ControlDelayBlock,
    sizeof(DelayBlock));
}
