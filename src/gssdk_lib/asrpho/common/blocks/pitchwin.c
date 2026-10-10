// Buffers incoming speech samples and applies a Hamming window.
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

#define M_PI 3.141592653589793

#define PITCH_WINDOW_RESET_COMMAND 200
#define PITCH_WINDOW_DESTRUCT_COMMAND 255

typedef struct PitchWindow {
    // TOS block callbacks, context, and connected input and output ports.
    TosBaseBlock base;
    f32 *windowCoefficients; // Hamming weight for each sample in the window.
    s32 *historySamples; // Circular buffer holding one complete analysis window.
    s32 *historyRead; // First sample used for the next windowed output frame.
    s32 *historyWrite; // First slot for the next input frame.
    s32 *historyEnd; // One past the final sample in historySamples.
    u16 frameCount; // Number of input frames that make up one analysis window.
    u16 frameLength; // Samples per input frame and per output-frame advance.
    u16 windowLength; // Samples in the analysis window and coefficient array.
} PitchWindow;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 cosf(f32 value);

// Add each input frame to history, then enqueue the current windowed analysis window.
static void ProcessPitchWindow(
    TosBaseBlock *baseBlock, void **inputs, s32 inputPortCount)
{
    PitchWindow *block = (PitchWindow *)baseBlock;
    s32 *inputSamples = inputs[0];
    f32 *windowWeight;
    f32 *outputSamples;
    u16 frameIndex;
    u16 sampleIndex;

    if (inputSamples != NULL) {
        memcpy(block->historyWrite, inputSamples, block->frameLength * sizeof(s32));
        block->historyWrite += block->frameLength;
        if (block->historyWrite == block->historyEnd) {
            block->historyWrite = block->historySamples;
        }

        windowWeight = block->windowCoefficients;
        outputSamples = qEnQueueOne(block->base.output->queue);
        for (frameIndex = 0; frameIndex < block->frameCount; frameIndex++) {
            for (sampleIndex = 0; sampleIndex < block->frameLength; sampleIndex++) {
                *outputSamples++ = *block->historyRead++ * *windowWeight++;
            }
            if (block->historyRead == block->historyEnd) {
                block->historyRead = block->historySamples;
            }
        }

        block->historyRead += block->frameLength;
        if (block->historyRead == block->historyEnd) {
            block->historyRead = block->historySamples;
        }
    }
}

// Clear every history sample when the block's control path resets its analysis window.
static inline void FillPitchHistory(PitchWindow *block, s32 value)
{
    u16 sampleIndex;

    for (sampleIndex = 0; sampleIndex < block->windowLength; sampleIndex++) {
        *block->historyWrite++ = value;
    }
}

// Handle the block reset command and TOS destruction command.
static u32 ControlPitchWindow(
    TosBaseBlock *baseBlock, u32 command, void *controlArgument,
    u32 controlArgumentSize)
{
    PitchWindow *block = (PitchWindow *)baseBlock;

    switch ((u8)command) {
    case PITCH_WINDOW_RESET_COMMAND:
        block->historyWrite = block->historySamples;
        FillPitchHistory(block, 0);
        block->historyRead = block->historySamples;
        block->historyWrite =
            block->historySamples + (block->frameCount - 1) * block->frameLength;
        block->historyEnd = block->historySamples + block->windowLength;
        break;
    case PITCH_WINDOW_DESTRUCT_COMMAND:
        heap_Free(block->base.context->heap, block->windowCoefficients);
        heap_Free(block->base.context->heap, block->historySamples);
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }
    return 1;
}

// Build Hamming weights during block initialization.
static void CreateWindow(f32 *windowCoefficients, u16 sampleRate, u16 windowLength)
{
    int sampleOffset;
    u16 coefficientCount = windowLength;
    u16 midpointIndex;
    u16 samplesPerSecond = sampleRate;
    f32 angularFrequency = (2.0 * M_PI) / ((f32)coefficientCount / samplesPerSecond);
    f32 samplePeriod = 1.0f / samplesPerSecond;
    f32 sampleTime;

    midpointIndex = coefficientCount / 2;
    sampleTime = (midpointIndex - 1) * samplePeriod;
    windowCoefficients[midpointIndex] = 1.0f;

    for (sampleOffset = 1; sampleOffset < midpointIndex; sampleOffset++) {
        windowCoefficients[midpointIndex - sampleOffset] =
            0.54 - 0.46 * cosf(angularFrequency * sampleTime);
        windowCoefficients[midpointIndex + sampleOffset] =
            windowCoefficients[midpointIndex - sampleOffset];
        sampleTime -= samplePeriod;
    }
    windowCoefficients[0] = 0.54 - 0.46 * cosf(angularFrequency * sampleTime);
}

// During block initialization, read the rec1600 profile and prepare the analysis window.
static u32 InitPitchWindow(TosBaseBlock *baseBlock)
{
    PitchWindow *block = (PitchWindow *)baseBlock;
    TosContext *context = baseBlock->context;
    u16 sampleRate;
    f32 windowDuration;
    u16 inputFrameLength;
    u16 inputSampleRate;
    s16 windowLength;

    // Profile keys 1-4 select output rate, duration, input frame size, and input rate.
    sampleRate = (u16)_tosGetProfileU32(block, 1, 2200);
    windowDuration = _tosGetProfileFloat(block, 2, 0.04f);
    inputFrameLength = (u16)_tosGetProfileU32(block, 3, 110);
    inputSampleRate = (u16)_tosGetProfileU32(block, 4, 11000);

    windowLength = block->windowLength = sampleRate * windowDuration + 0.5;
    // Require the configured input window to contain a whole number of input frames.
    if ((u32)(0.5 + windowDuration * inputSampleRate) % inputFrameLength != 0) {
        _tosErrorLog(block, 100);
        return 1;
    }

    block->frameCount = (u32)(0.5 + windowDuration * inputSampleRate) / inputFrameLength;
    block->frameLength = (u16)windowLength / block->frameCount;
    block->base.input->inputSize = block->frameLength * sizeof(s32);
    block->base.output->outputSize = (u16)windowLength * sizeof(f32);

    block->windowCoefficients =
        heap_Calloc(context->heap, (u16)windowLength, sizeof(f32));
    block->historySamples =
        heap_Calloc(context->heap, (u16)windowLength, sizeof(s32));
    if (block->windowCoefficients == NULL || block->historySamples == NULL) {
        _tosErrorLog(block, 2);
        return 1;
    }

    CreateWindow(block->windowCoefficients, sampleRate, windowLength);
    block->historyRead = block->historySamples;
    block->historyWrite =
        block->historySamples + (block->frameCount - 1) * block->frameLength;
    block->historyEnd = block->historySamples + block->windowLength;
    return 0;
}

// Construct the rec1600 pitch-window block with one input and one output.
void *ConstructPitchWindow(TosContext *context, u32 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 1, 1, ProcessPitchWindow, InitPitchWindow,
        ControlPitchWindow, sizeof(PitchWindow));
}
