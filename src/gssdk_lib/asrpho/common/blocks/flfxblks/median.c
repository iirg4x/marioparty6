#include "types.h"

#include "gssdk/tos.h"

typedef struct MedianNode {
    struct MedianNode *higher;
    struct MedianNode *lower;
    f32 value;
} MedianNode;

typedef struct Median {
    TosBaseBlock base;
    u32 capacity;
    u32 count;
    u32 ringIndex;
    MedianNode *nodes;
    MedianNode *minimum;
    MedianNode *maximum;
    MedianNode *median;
} Median;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);

static void ProcessMedian(TosBaseBlock *_This, void *pInputs, int NbrInputs)
{
    Median *This = (Median *)_This;
    MedianNode *node;
    MedianNode *cursor;
    f32 sample;
    f32 *output;
    u32 i;
    u32 halfCount;

    if (*(void **)pInputs != NULL) {
        node = &This->nodes[This->ringIndex];
        sample = *(f32 *)*(void **)pInputs;

        if (This->count == This->capacity) {
            if (node->higher != NULL) {
                node->higher->lower = node->lower;
            } else {
                This->maximum = node->lower;
            }

            if (node->lower != NULL) {
                node->lower->higher = node->higher;
            } else {
                This->minimum = node->higher;
            }

            This->count--;

            if (node == This->median) {
                if (node->higher != NULL) {
                    This->median = node->higher;
                } else if (node->lower != NULL) {
                    This->median = node->lower;
                } else {
                    This->median = NULL;
                }
            }
        }

        node->value = sample;

        if (This->count == 0) {
            This->median = node;
            This->maximum = node;
            This->minimum = node;
            node->lower = NULL;
            node->higher = NULL;
        } else if (sample < This->median->value) {
            cursor = This->median->lower;
            while (cursor != NULL && sample < cursor->value) {
                cursor = cursor->lower;
            }

            if (cursor != NULL) {
                node->higher = cursor->higher;
                cursor->higher = node;
                node->lower = cursor;
                node->higher->lower = node;
            } else {
                node->higher = This->minimum;
                node->lower = NULL;
                This->minimum->lower = node;
                This->minimum = node;
            }
        } else {
            cursor = This->median->higher;
            while (cursor != NULL && sample >= cursor->value) {
                cursor = cursor->higher;
            }

            if (cursor != NULL) {
                node->higher = cursor;
                node->lower = cursor->lower;
                cursor->lower = node;
                node->lower->higher = node;
            } else {
                node->higher = NULL;
                node->lower = This->maximum;
                This->maximum->higher = node;
                This->maximum = node;
            }
        }

        if (++This->ringIndex == This->capacity) {
            This->ringIndex = 0;
        }

        This->count++;
        This->median = This->minimum;
        halfCount = This->count >> 1;
        for (i = 0; i < halfCount; i++) {
            This->median = This->median->higher;
        }

        output = qEnQueueOne(This->base.output->queue);
        if (output != NULL) {
            *output = This->median->value;
        }
    }
}

static u32 ControlMedian(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    Median *block = (Median *)baseBlock;

    switch ((u8)command) {
    case 255:
        if (block->nodes != NULL) {
            heap_Free(block->base.context->heap, block->nodes);
        }
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }

    return 1;
}

u32 InitMedian(TosBaseBlock *_This)
{
    Median *This = (Median *)_This;
    u32 result = 0;

    This->capacity = 0;
    This->count = 0;
    This->ringIndex = 0;
    This->nodes = NULL;
    This->minimum = NULL;
    This->maximum = NULL;
    This->median = NULL;

    This->base.input->inputSize = 24;
    This->base.output->outputSize = sizeof(f32);

    This->capacity = _tosGetProfileU32(This, 1, 1);
    if ((This->capacity & 1) == 0) {
        _tosErrorLog(This, 200);
        result = 1;
    } else {
        This->nodes = heap_Calloc(
            This->base.context->heap,
            This->capacity,
            sizeof(MedianNode));

        This->median = NULL;
        This->maximum = NULL;
        This->minimum = NULL;
        This->ringIndex = 0;
        This->count = 0;
    }

    return result;
}

void *ConstructMedian(TosContext *context, u8 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 1, 1,
        (TosProcessFunction)ProcessMedian,
        (TosInitFunction)InitMedian,
        (TosControlFunction)ControlMedian,
        sizeof(Median));
}
