/* Combines each group of input audio samples into one wider output sample. */
#include "types.h"

#include "gssdk/tos.h"

#define UNDERSAMPLER_DESTRUCT_COMMAND 255

typedef struct Undersampler {
TosBaseBlock base;
u16 inputSamples; /* Input samples processed in one frame. */
u8 samplesPerOutput; /* Input samples summed into each output sample. */
u8 reserved; /* Unused byte retained in the block layout. */
} Undersampler;

/* Sums each input group into one output value during the TOS process pass. */
static void ProcessUndersampler(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
Undersampler *block = (Undersampler *)baseBlock;
s16 *inputSamples = inputs[0];
u8 samplesPerOutput = block->samplesPerOutput;
s32 *outputSamples;
s16 *sampleCursor;
s16 *groupEnd;
s16 *inputEnd;
s32 sum;

if (inputSamples != NULL) {
    outputSamples = qEnQueueOne(baseBlock->output->queue);
    inputEnd = inputSamples + block->inputSamples;
    sampleCursor = inputSamples;
    while (sampleCursor < inputEnd) {
        groupEnd = sampleCursor + samplesPerOutput;
        sum = *sampleCursor++;
        while (sampleCursor < groupEnd) {
            sum += *sampleCursor++;
        }
        *outputSamples++ = sum;
    }
}
}

/* Calls the base-block destructor when the low byte of the TOS control command is 255. */
static u32 ControlUndersampler(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
switch ((u8)command) {
case UNDERSAMPLER_DESTRUCT_COMMAND:
    tosBaseBlockDestruct(baseBlock);
    break;
default:
    return 0;
}
return 1;
}

/* Loads the input frame length and rate settings, derives the input samples per output sample,
 * rejects a frame length not divisible by that count, and sets the input/output queue byte
 * sizes. */
static u32 InitUndersampler(TosBaseBlock *_This)
{
u16 inputSamples;
Undersampler *This = (Undersampler *)_This;
u16 inputRate;
u8 samplesPerOutput;
u16 outputSamples;

inputSamples = (u16)_tosGetProfileU32(This, 1, 110);
This->inputSamples = inputSamples;
inputRate = (u16)_tosGetProfileU32(This, 2, 11000);
This->samplesPerOutput = inputRate / (u16)_tosGetProfileU32(This, 3, 2000);
samplesPerOutput = This->samplesPerOutput;
if (inputSamples % samplesPerOutput != 0) {
    _tosErrorLog(This, 100);
    return 1;
}
outputSamples = inputSamples / samplesPerOutput;
This->base.input->inputSize = inputSamples * sizeof(s16);
This->base.output->outputSize = outputSamples * sizeof(s32);
return 0;
}

/* Creates the block registered in the speech pipeline's block definitions. */
void *ConstructUndersampler(TosContext *context, u8 blockIndex)
{
return tosBaseBlockConstruct(
    context, blockIndex, 1, 1, (TosProcessFunction)ProcessUndersampler,
    (TosInitFunction)InitUndersampler,
    (TosControlFunction)ControlUndersampler, sizeof(Undersampler));
}
