#include "types.h"

#include "gssdk/tos.h"

typedef struct LookAhead {
TosBaseBlock base;
u8 flushActive;
u8 reserved29[3];
u32 lookAhead;
u32 historyLength;
f32 threshold;
f32 *history;
f32 *historyWrite;
f32 *historyEnd;
f32 sum;
u32 queued;
} LookAhead;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);

static void ProcessLookAhead(TosBaseBlock *_This, void *pInputs, int NbrInputs)
{
LookAhead *This = (LookAhead *)_This;
f32 *sample = ((void **)pInputs)[0];
u8 *output;

if (sample != NULL) {
    This->sum -= *This->historyWrite;
    *This->historyWrite++ = *sample;
    This->sum += *sample;
    if (This->historyWrite == This->historyEnd) {
        This->historyWrite = This->history;
    }
    if (This->queued == This->lookAhead) {
        output = qEnQueueOne(This->base.output->queue);
        *output = This->sum > This->threshold;
    } else {
        This->queued++;
    }
} else if (This->flushActive != 0) {
    while (This->queued != 0) {
        output = qEnQueueOne(This->base.output->queue);
        *output = This->sum > This->threshold;
        This->queued--;
    }
    if (This->queued == 0) {
        This->flushActive = 0;
    }
}
}

static u32 ControlLookAhead(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
LookAhead *block = (LookAhead *)baseBlock;

switch ((u8)command) {
case 1:
    block->flushActive = 1;
    break;
case 255:
    heap_Free(block->base.context->heap, block->history);
    tosBaseBlockDestruct(block);
    break;
default:
    return 0;
}
return 1;
}

static u32 InitLookAhead(TosBaseBlock *_This)
{
LookAhead *This = (LookAhead *)_This;
TosContext *context = _This->context;

This->lookAhead = (u16)_tosGetProfileU32(This, 1, 0);
This->historyLength = (u16)_tosGetProfileU32(This, 2, 15);
This->threshold = _tosGetProfileFloat(This, 3, 1.0f);
This->base.input->inputSize = sizeof(f32);
This->base.output->outputSize = sizeof(u8);
This->history = heap_Calloc(
    context->heap, This->historyLength, sizeof(f32));
This->historyWrite = This->history;
This->historyEnd = This->history + This->historyLength;
This->sum = 0.0f;
This->queued = 0;
return 0;
}

void *ConstructLookAhead(TosContext *context, u8 blockIndex)
{
return tosBaseBlockConstruct(
    context, blockIndex, 1, 1, (TosProcessFunction)ProcessLookAhead,
    (TosInitFunction)InitLookAhead, (TosControlFunction)ControlLookAhead,
    sizeof(LookAhead));
}
