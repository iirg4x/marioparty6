// Converts incoming audio frames into an amplitude or power spectrum.
#include "types.h"

#include "dolphin/math.h"
#include <string.h>

#include "gssdk/fft.h"
#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

typedef struct FFTMod {
    TosBaseBlock base; // TinyOS block state and audio queue ports.
    u32 inputLength; // Number of input audio samples in each frame.
    u32 fftLength; // Number of samples transformed by the FFT.
    u32 firstPowerBin; // First spectrum bin included in the power total.
    u32 lastPowerBin; // Last spectrum bin included in the power total.
    f32 powerFloor; // Initial value added to the selected-bin power total.
    u8 padInput; // Nonzero when short frames are zero-padded to the FFT length.
    u32 paddingBytes; // Number of trailing zero bytes added to each frame.
    f32 *buffer; // Scratch frame used when input samples need zero-padding.
    s16 powerLevel; // Smoothed power level in hundredths of a decibel.
    u8 powerSpectrum; // Nonzero to output band power and a scalar power level.
} FFTMod;

extern void *heap_Alloc(void *heap, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 logf_check(f32 value);
extern f32 floorf(f32 value);

// Called by ProcessFFTMod for each power-mode frame. Fills the spectrum and totals selected bins.
static f32 CalcPowerSpec(FFTMod *block, f32 *input, f32 *output)
{
    f32 *firstSpectrumValue;
    f32 power = block->powerFloor;
    f32 *realValues = input;
    f32 *imaginaryValues = input + block->fftLength - 1;
    f32 *firstPowerValue = output + block->firstPowerBin;
    f32 *lastPowerValue = output + block->lastPowerBin;
    f32 *lastInteriorValue = output + block->fftLength / 2;

    firstSpectrumValue = output;
    *output = *realValues * *realValues;
    realValues++;
    output++;
    if (firstSpectrumValue == firstPowerValue) {
        power += *firstPowerValue++;
    }

    while (output < firstPowerValue) {
        *output++ = *realValues * *realValues + *imaginaryValues * *imaginaryValues;
        realValues++;
        imaginaryValues--;
    }
    while (output <= lastPowerValue) {
        *output = *realValues * *realValues + *imaginaryValues * *imaginaryValues;
        power += *output++;
        realValues++;
        imaginaryValues--;
    }
    while (output < lastInteriorValue) {
        *output++ = *realValues * *realValues + *imaginaryValues * *imaginaryValues;
        realValues++;
        imaginaryValues--;
    }
    *output = *realValues * *realValues;
    return power;
}

// Called by ProcessFFTMod for each amplitude-mode frame. Converts FFT bins to magnitudes.
static void CalcAmplitudeSpec(FFTMod *block, f32 *input, f32 *output)
{
    f32 *realValues = input;
    f32 *imaginaryValues = input + block->fftLength - 1;
    f32 *firstPowerValue = output + block->firstPowerBin;
    f32 *lastPowerValue = output + block->lastPowerBin;
    f32 *lastInteriorValue = output + block->fftLength / 2;

    *output = sqrtf(*realValues * *realValues);
    realValues++;
    if (output++ == firstPowerValue) {
        firstPowerValue++;
    }

    while (output < firstPowerValue) {
        *output++ = sqrtf(*realValues * *realValues + *imaginaryValues * *imaginaryValues);
        realValues++;
        imaginaryValues--;
    }
    while (output <= lastPowerValue) {
        *output++ = sqrtf(*realValues * *realValues + *imaginaryValues * *imaginaryValues);
        realValues++;
        imaginaryValues--;
    }
    while (output < lastInteriorValue) {
        *output++ = sqrtf(*realValues * *realValues + *imaginaryValues * *imaginaryValues);
        realValues++;
        imaginaryValues--;
    }
    *output = sqrtf(*realValues * *realValues);
}

// Called by the audio graph per frame. Skips NULL input; otherwise transforms it and queues its
// spectrum output.
static void ProcessFFTMod(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
    FFTMod *block = (FFTMod *)baseBlock;
    f32 *input;
    f32 *spectrum;

    if (block->padInput != 0) {
        if (inputs[0] != NULL) {
            memcpy(block->buffer, inputs[0], block->base.input->inputSize);
            input = block->buffer;
            // The FFT uses a fixed frame length, so fill the remaining samples with silence.
            memset(
                input + block->inputLength, 0, block->paddingBytes);
        } else {
            input = NULL;
        }
    } else {
        input = inputs[0];
    }

    if (input != NULL) {
        if (block->powerSpectrum != 0) {
            spectrum = qEnQueueOne(block->base.output[1].queue);
        } else {
            spectrum = qEnQueueOne(block->base.output->queue);
        }

        realfft(block->fftLength, input);
        if (block->powerSpectrum != 0) {
            f32 logPower = logf_check(CalcPowerSpec(block, input, spectrum));
            s16 powerLevel;

            qEnQueue(block->base.output->queue, &logPower, 1);
            powerLevel =
                (s16)((4.3429446f * (logPower - 27.37f)) * 100.0);
            // Cap the reported level at 18 dB; lower readings decay the stored level gradually.
            if (powerLevel > 1800) {
                powerLevel = 1800;
            }

            if (powerLevel > block->powerLevel) {
                block->powerLevel = powerLevel;
            } else {
                block->powerLevel =
                    (((block->powerLevel + 7200) * 64000) >> 16) - 7200;
            }
        } else {
            CalcAmplitudeSpec(block, input, spectrum);
        }
    }
}

// Answers power-level queries only in power mode (logging an error otherwise) and frees padding
// storage and the block on shutdown.
static u32 ControlFFTMod(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    FFTMod *block = (FFTMod *)baseBlock;
    TosContext *context = block->base.context;

    switch ((u8)command) {
    case 1:
        if (block->powerSpectrum == 0) {
            _tosErrorLog(block, 102);
            return 1;
        }
        *(s16 **)argument = &block->powerLevel;
        break;
    case 255:
        if (block->padInput != 0) {
            heap_Free(context->heap, block->buffer);
        }
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }
    return 1;
}

// Called when the audio graph starts this block. Reads its profile and sizes its queues.
static u32 InitFFTMod(TosBaseBlock *baseBlock)
{
    FFTMod *block = (FFTMod *)baseBlock;
    u16 fftLength;
    TosContext *context = baseBlock->context;
    s32 sampleRate;
    f32 scale;
    f32 frequency;

    sampleRate = (s16)_tosGetProfileU32(block, 3, 5500);
    fftLength = (u16)_tosGetProfileU32(block, 4, 512);
    block->fftLength = fftLength;
    block->inputLength = (u16)_tosGetProfileU32(block, 6, 512);
    if (block->inputLength > (u32)fftLength) {
        return 1;
    }

    if ((u32)fftLength > block->inputLength) {
        block->padInput = 1;
        block->paddingBytes =
            ((u32)fftLength - block->inputLength) * sizeof(f32);
        block->buffer =
            heap_Alloc(context->heap, (u32)fftLength * sizeof(f32));
    }

    block->base.input->inputSize = block->inputLength * sizeof(f32);
    if (block->powerSpectrum == 0) {
        block->base.output->outputSize =
            ((u32)fftLength / 2 + 1) * sizeof(f32);
    } else {
        block->base.output->outputSize = sizeof(f32);
        block->base.output[1].outputSize =
            ((u32)fftLength / 2 + 1) * sizeof(f32);
    }

    frequency = _tosGetProfileFloat(block, 1, 300.0f);
    frequency = (f32)block->fftLength * frequency;
    scale = 2.0f * (f32)sampleRate;
    block->firstPowerBin = (u32)floorf(frequency / scale);
    block->firstPowerBin = block->firstPowerBin > 0 ? block->firstPowerBin : 0;
    block->firstPowerBin =
        block->firstPowerBin < block->fftLength / 2 - 1
            ? block->firstPowerBin
            : block->fftLength / 2 - 2;

    frequency = _tosGetProfileFloat(block, 2, 4500.0f);
    block->lastPowerBin =
        (u32)floorf(
            0.9999f + (block->fftLength * frequency) / scale);
    block->lastPowerBin = block->lastPowerBin > 0 ? block->lastPowerBin : 0;
    block->lastPowerBin =
        block->lastPowerBin < block->fftLength / 2 - 1
            ? block->lastPowerBin
            : block->fftLength / 2 - 2;
    block->lastPowerBin = block->lastPowerBin > block->firstPowerBin
        ? block->lastPowerBin
        : block->firstPowerBin;

    block->powerFloor = _tosGetProfileFloat(block, 5, 0.0f);
    return 0;
}

// Called by the audio profile loader to create an FFT block with one or two output queues.
void *ConstructFFTMod(TosContext *context, u8 blockIndex)
{
    struct TosProfileBlock {
        TosContext *context;
        u8 blockIndex;
    } profileBlock;
    FFTMod *block;
    u8 powerSpectrum;

    profileBlock.context = context;
    profileBlock.blockIndex = blockIndex;
    powerSpectrum =
        (u8)_tosGetProfileU32(&profileBlock, 7, 1);
    block = (FFTMod *)tosBaseBlockConstruct(
        context, blockIndex, 1, powerSpectrum != 0 ? 2 : 1,
        ProcessFFTMod, InitFFTMod, ControlFFTMod, sizeof(FFTMod));
    block->powerSpectrum = powerSpectrum;
    return block;
}
