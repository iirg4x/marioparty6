#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

typedef struct Stacker {
TosBaseBlock base;
u32 reserved28;
u16 numberInputs;
u16 *elementSizes;
u8 *enabled;
} Stacker;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);

static void ProcessStacker(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
Stacker *block = (Stacker *)baseBlock;
u16 i;
u8 *output;

for (i = 0; i < inputCount; i++) {
    if (inputs[i] != NULL) {
        break;
    }
}
if (i == inputCount) {
    return;
}

output = qEnQueueOne(block->base.output->queue);
for (i = 0; i < inputCount; i++) {
    if (block->enabled[i] != 0) {
        memcpy(output, inputs[i], block->elementSizes[i]);
    }
    output += block->elementSizes[i];
}
}

static u32 ControlStacker(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
Stacker *block = (Stacker *)baseBlock;

switch ((u8)command) {
case 100:
    block->enabled[0] = 0;
    break;
case 101:
    block->enabled[1] = 0;
    break;
case 150:
    block->enabled[0] = 1;
    break;
case 151:
    block->enabled[1] = 1;
    break;
case 255:
    heap_Free(block->base.context->heap, block->elementSizes);
    heap_Free(block->base.context->heap, block->enabled);
    tosBaseBlockDestruct(block);
    break;
default:
    return 0;
}
return 1;
}

static u32 InitStacker(TosBaseBlock *_This)
{
Stacker *This = (Stacker *)_This;
TosContext *context = _This->context;
u16 i;

This->numberInputs = _tosGetProfileU32(This, 3, 2);
This->elementSizes = heap_Calloc(
    context->heap, This->numberInputs, sizeof(u16));
This->enabled = heap_Calloc(
    context->heap, This->numberInputs, sizeof(u8));
if (This->elementSizes == NULL || This->enabled == NULL) {
    _tosErrorLog(This, 2);
    return 1;
}

for (i = 0; i < This->numberInputs; i++) {
    switch (i) {
    case 0:
        This->elementSizes[0] = _tosGetProfileU32(This, 1, 152);
        break;
    case 1:
        This->elementSizes[1] = _tosGetProfileU32(This, 2, 20);
        break;
    default:
        _tosErrorLog(This, 200);
        return 1;
    }
    This->enabled[i] = 1;
}

for (i = 0; i < This->numberInputs; i++) {
    This->base.input[i].inputSize = This->elementSizes[i];
    This->base.output->outputSize =
        This->base.output->outputSize + This->base.input[i].inputSize;
}
return 0;
}

void *ConstructStacker(TosContext *context, u8 blockIndex)
{
return tosBaseBlockConstruct(
    context, blockIndex, 2, 1, ProcessStacker, InitStacker,
    ControlStacker, sizeof(Stacker));
}
