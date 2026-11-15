#include "types.h"

#include "gssdk/tos.h"

typedef struct Stationarity {
    TosBaseBlock base;
    u32 drain;
    u32 queued;
    u32 span;
    f32 threshold;
    f32 *history;
    f32 *historyEnd;
    f32 *historyWrite;
    f32 *delayed;
    f32 *delayedEnd;
    f32 *delayedWrite;
} Stationarity;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 _tosGetProfileFloat(void *block, u32 key, f32 defaultValue);

static f32 StationarityRange(f32 *cursor, f32 *end)
{
    f32 maximum;
    f32 minimum = *cursor;

    maximum = minimum;
    while (++cursor < end) {
        f32 value = *cursor;

        if (value < minimum) {
            minimum = value;
        }
        if (value > maximum) {
            maximum = value;
        }
    }
    return maximum - minimum;
}

static void ProcessStationarity(TosBaseBlock *_This, void *pInputs)
{
    Stationarity *This = (Stationarity *)_This;
    f32 sample;
    f32 incoming = 0.0f;
    f32 delayed;
    f32 *cursor;
    f32 *output;

    if (((void **)pInputs)[0] != NULL) {
        sample = *(f32 *)((void **)pInputs)[0];
        incoming = *(f32 *)((void **)pInputs)[1];
        if (This->queued == 0) {
            cursor = This->history;
            while (cursor < This->historyEnd) {
                *cursor = sample;
                cursor++;
            }
            This->historyWrite = This->history;
        } else {
            *This->historyWrite++ = sample;
            if (This->historyWrite == This->historyEnd) {
                This->historyWrite = This->history;
            }
        }
        This->queued++;
    } else if (This->drain == 0) {
        return;
    }

    delayed = *This->delayedWrite;
    *This->delayedWrite++ = incoming;
    if (This->delayedWrite == This->delayedEnd) {
        This->delayedWrite = This->delayed;
    }
    if (This->drain != 0 && This->queued == 0) {
        This->drain = 0;
    }
    if (This->queued <= This->span && This->drain == 0) {
        return;
    }

    output = qEnQueueOne(This->base.output->queue);
    This->queued--;
    if (delayed > 0.0f) {
        if (!(StationarityRange(This->history, This->historyEnd) <
        This->threshold)) {
            delayed *= 0.1f;
        }
    }
    *output = delayed;
}

static u32 ControlStationarity(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    Stationarity *block = (Stationarity *)baseBlock;
    TosContext *context = baseBlock->context;

    switch ((u8)command) {
    case 1:
        block->drain = 1;
        break;
    case 255:
        heap_Free(context->heap, block->history);
        heap_Free(context->heap, block->delayed);
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }
    return 1;
}

static u32 InitStationarity(TosBaseBlock *_This)
{
    Stationarity *This = (Stationarity *)_This;
    TosContext *context = This->base.context;
    u32 i;

    This->base.input[0].inputSize = 24;
    This->base.input[1].inputSize = sizeof(f32);
    This->base.output->outputSize = sizeof(f32);
    This->span = (u8)_tosGetProfileU32(This, 1, 2);
    This->threshold = _tosGetProfileFloat(This, 2, 2.0f);
    This->history = heap_Calloc(
    context->heap, This->span * 2 + 1, sizeof(f32));
    This->delayed = heap_Calloc(context->heap, This->span, sizeof(f32));
    if (This->history == NULL || This->delayed == NULL) {
        return _tosErrorLog(This, 2);
    }
    for (i = 0; i < This->span * 2 + 1; i++) {
        This->history[i] = 0.0f;
    }
    This->historyEnd = This->history + This->span * 2 + 1;
    This->historyWrite = This->history;
    This->delayedEnd = This->delayed + This->span;
    This->delayedWrite = This->delayed;
    return 0;
}

void *ConstructStationarity(TosContext *context, u8 blockIndex)
{
    return tosBaseBlockConstruct(
    context, blockIndex, 2, 1, (TosProcessFunction)ProcessStationarity,
    (TosInitFunction)InitStationarity,
    (TosControlFunction)ControlStationarity, sizeof(Stationarity));
}
