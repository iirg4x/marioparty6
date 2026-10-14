// Combines pitch, normalized energy and voicing histories into delayed speech features.
#include "types.h"

#include "gssdk/tos.h"
extern u32 _tosErrorLog(void *block, u32 error);

#define PITCH_COMBINER_RESET_COMMAND 200
#define PITCH_COMBINER_FLUSH_COMMAND 201
#define PITCH_COMBINER_DESTRUCT_COMMAND 255
#define PITCH_COMBINER_BAND_COUNT_PROFILE 1
#define PITCH_COMBINER_VOICED_OUTPUT_PROFILE 2
#define PITCH_COMBINER_UNVOICED_OUTPUT_PROFILE 3

typedef struct PitchHistory {
    f32 *samples; // First sample in the circular history.
    f32 *output; // Sample consumed by the delayed feature output.
    f32 *write; // Next history sample to fill.
    f32 *end; // One past the final allocated sample.
} PitchHistory;

typedef struct PitchCombiner {
    TosBaseBlock base; // Speech graph ports, callbacks and owning context.
    PitchHistory normEnergy; // Circular history of maximum input band energy.
    f32 *normEnergyFilter; // First energy sample used by the current filter window.
    PitchHistory logPitch; // Circular history of logarithmic pitch samples.
    f32 *logPitchFilter; // First pitch sample used by the current voiced window.
    PitchHistory voicing; // Input voicing bytes stored as floats; only 1.0 is voiced.
    PitchHistory pitchSlope; // Circular history of pitch-window slope values.
    PitchHistory meanPitch; // Circular history of pitch-window mean values.
    PitchHistory normEnergySlope; // Circular history of normalized energy slopes.
    f32 pitchDelta; // Delayed log pitch minus the mean for its corresponding window.
    f32 *pitchCoefficients; // Seven coefficients for the pitch slope calculation.
    f32 *normEnergyCoefficients; // Five coefficients for the energy slope calculation.
    u8 pitchWindowLength; // Number of pitch samples in a voiced analysis window.
    u8 normEnergyWindowLength; // Number of energy samples in an analysis window.
    u8 delayLength; // Number of samples allocated to each circular history.
    u8 voicedFrameCount; // Frames in the voiced window; decremented after each full window.
    u8 queuedFrameCount; // Delayed input frames not yet emitted as output features.
    u8 voiced; // Whether the current input frame is voiced (1) or unvoiced (0).
    u8 previousVoiced; // Voicing state from the preceding input frame.
    u8 beginVoiced; // Set at a voiced run's start; cleared only after a complete voiced window.
    u8 endVoiced; // Set when a voiced run ends with window length minus one frames held.
    u8 initializeNormEnergy; // Set while the initial energy slope history is filled.
    u16 pitchCoefficientDivisor; // Divisor applied to the pitch slope sum.
    u16 normEnergyCoefficientDivisor; // Divisor applied to the energy slope sum.
    f32 lastMeanPitch; // Most recent mean log pitch, repeated after voicing ends.
    f32 lastPitchSlope; // Most recent pitch slope, repeated after voicing ends.
    u8 unused; // Not read by the pitch combiner.
    u8 outputFeatureCount; // Number of floats written for each output frame.
    u8 flushing; // Set while queued frames are emitted after input ends.
    u8 flushPitch; // Pending pitch padding attempt; cleared even when too few frames remain.
    u8 flushNormEnergy; // Set until the energy slope history is padded for flushing.
    u8 pitchFilterReady; // Set when a completed voiced window advances the pitch filter.
    u16 bandCount; // Number of energy bands in each input spectrum.
    f32 voicedOutput; // Configured output value for a voiced frame.
    f32 unvoicedOutput; // Configured output value for an unvoiced frame.
} PitchCombiner;

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 logf_check(f32 value);

// Clears a feature history and restores its initial read and write positions.
// InitPitchCombiner and PitchCombinerReset call this when preparing the block.
static inline void ResetPitchHistory(PitchHistory *history, u8 length)
{
    u16 i;

    history->output = history->write = history->samples;
    history->end = history->samples + length;
    for (i = 0; i < length; i++) {
        history->samples[i] = 0.0f;
    }
}

// Stores a new feature sample and advances the circular history's write position.
// ProcessPitchCombiner uses this when recording a frame's maximum band energy.
static inline void PushPitchHistory(PitchHistory *history, f32 value)
{
    *history->write++ = value;
    if (history->write == history->end) {
        history->write = history->samples;
    }
}

// Averages the current voiced window, wrapping across the pitch history's end.
// LogPitchProcessing calls this after a complete voiced pitch window is ready.
static f32 MeanPitchHistory(PitchCombiner *block)
{
    f32 *sample;
    u16 i;
    f32 sum;
    u8 length;

    sample = block->logPitchFilter;
    i = 0;
    sum = 0.0f;
    length = block->pitchWindowLength;
    for (; i < length; i++) {
        sum += *sample++;
        if (sample == block->logPitch.end) {
            sample = block->logPitch.samples;
        }
    }
    sum /= length;
    return sum;
}

// Applies the slope coefficients to the current voiced window.
// LogPitchProcessing calls this after a complete voiced pitch window is ready.
static inline void ComputePitchSlope(PitchCombiner *block, f32 *output)
{
    u8 pitchWindowLength;
    f32 *pitch;
    f32 *coefficient;
    u8 i;
    f32 sum;

    i = 0;
    pitch = block->logPitchFilter;
    coefficient = block->pitchCoefficients;
    sum = 0.0f;
    pitchWindowLength = block->pitchWindowLength;
    for (; i < pitchWindowLength; i++) {
        sum += *pitch++ * *coefficient++;
        if (pitch == block->logPitch.end) {
            pitch = block->logPitch.samples;
        }
    }
    sum /= block->pitchCoefficientDivisor;
    *output = sum;
}

// Returns the largest value among the energy bands in a new input frame.
// ProcessPitchCombiner calls this for each spectrum; even a zero count reads its first value.
static f32 MaxFloatInArray(f32 *values, u16 count)
{
    f32 maximum = values[0];
    f32 *value = values + 1;
    u16 i;

    for (i = 1; i < count; i++, value++) {
        if (*value > maximum) {
            maximum = *value;
        }
    }
    return maximum;
}

// Tracks voiced runs and advances the voicing history for each input frame.
// ProcessPitchCombiner calls this after storing the frame's voicing value.
static void StatusProcessing(PitchCombiner *block)
{
    block->previousVoiced = block->voiced;
    if (*block->voicing.write == 1.0f) {
        if (block->voiced == 0) {
            block->voiced = 1;
            block->beginVoiced = 1;
        }
        block->voicedFrameCount++;
    } else if (block->voiced == 1) {
        if (block->voicedFrameCount == block->pitchWindowLength - 1) {
            block->endVoiced = 1;
        }
        block->voiced = 0;
        block->voicedFrameCount = 0;
    }

    block->voicing.write++;
    if (block->voicing.write == block->voicing.end) {
        block->voicing.write = block->voicing.samples;
    }
}

// Seeds the earlier half-window of pitch results with the first complete result.
// LogPitchProcessing calls this after producing the first complete voiced window.
static void CheckBeginVoicedFlag(PitchCombiner *block)
{
    f32 *destination;
    f32 *source;
    u8 i;

    if (block->beginVoiced != 1) {
        return;
    }

    source = block->meanPitch.write - 1;
    if (source < block->meanPitch.samples) {
        source = block->meanPitch.end - 1;
    }
    destination = source - 1;
    if (destination < block->meanPitch.samples) {
        destination = block->meanPitch.end - 1;
    }
    for (i = 0; i < block->pitchWindowLength / 2; i++) {
        *destination = *source;
        destination--;
        if (destination < block->meanPitch.samples) {
            destination = block->meanPitch.end - 1;
        }
    }

    source = block->pitchSlope.write - 1;
    if (source < block->pitchSlope.samples) {
        source = block->pitchSlope.end - 1;
    }
    destination = source - 1;
    if (destination < block->pitchSlope.samples) {
        destination = block->pitchSlope.end - 1;
    }
    for (i = 0; i < block->pitchWindowLength / 2; i++) {
        *destination = *source;
        destination--;
        if (destination < block->pitchSlope.samples) {
            destination = block->pitchSlope.end - 1;
        }
    }
    block->beginVoiced = 0;
}

// Repeats the last pitch results across the trailing half-window of a voiced run.
// LogPitchProcessing calls this while advancing histories without a full voiced window.
static void CheckEndVoicedFlag(PitchCombiner *block)
{
    f32 *destination;
    u8 i;

    if (block->endVoiced != 1) {
        return;
    }

    destination = block->meanPitch.write - 1;
    if (destination < block->meanPitch.samples) {
        destination = block->meanPitch.end - 1;
    }
    for (i = 0; i < block->pitchWindowLength / 2; i++) {
        *destination = block->lastMeanPitch;
        destination++;
        if (destination == block->meanPitch.end) {
            destination = block->meanPitch.samples;
        }
    }

    destination = block->pitchSlope.write - 1;
    if (destination < block->pitchSlope.samples) {
        destination = block->pitchSlope.end - 1;
    }
    for (i = 0; i < block->pitchWindowLength / 2; i++) {
        *destination = block->lastPitchSlope;
        destination++;
        if (destination == block->pitchSlope.end) {
            destination = block->pitchSlope.samples;
        }
    }
    block->endVoiced = 0;
}

// Updates pitch results for full voiced windows and pads their run boundaries.
// ProcessPitchCombiner calls this once the input delay contains a full pitch window.
static void LogPitchProcessing(PitchCombiner *block)
{
    if (block->voicedFrameCount == block->pitchWindowLength) {
        f32 slope;

        block->pitchFilterReady = 1;
        ComputePitchSlope(block, &slope);
        *block->pitchSlope.write = slope;
        block->lastPitchSlope = *block->pitchSlope.write++;
        if (block->pitchSlope.write == block->pitchSlope.end) {
            block->pitchSlope.write = block->pitchSlope.samples;
        }
        *block->meanPitch.write = MeanPitchHistory(block);
        block->lastMeanPitch = *block->meanPitch.write++;
        if (block->meanPitch.write == block->meanPitch.end) {
            block->meanPitch.write = block->meanPitch.samples;
        }

        CheckBeginVoicedFlag(block);
        // Keep the preceding samples so the next voiced frame completes a sliding window.
        block->voicedFrameCount--;
    } else {
        block->pitchSlope.write++;
        if (block->pitchSlope.write == block->pitchSlope.end) {
            block->pitchSlope.write = block->pitchSlope.samples;
        }
        block->meanPitch.write++;
        if (block->meanPitch.write == block->meanPitch.end) {
            block->meanPitch.write = block->meanPitch.samples;
        }
        CheckEndVoicedFlag(block);
    }
}

// Calculates energy slope values and primes their history on the first window.
// ProcessPitchCombiner calls this once enough input frames have been queued.
static void NormE_Processing(PitchCombiner *block)
{
    f32 *energy = block->normEnergyFilter;
    f32 *coefficient = block->normEnergyCoefficients;
    f32 result = 0.0f;
    u8 i;

    for (i = 0; i < block->normEnergyWindowLength; i++) {
        result += *energy++ * *coefficient++;
        if (energy == block->normEnergy.end) {
            energy = block->normEnergy.samples;
        }
    }
    result /= block->normEnergyCoefficientDivisor;
    *block->normEnergySlope.write++ = result;
    if (block->normEnergySlope.write == block->normEnergySlope.end) {
        block->normEnergySlope.write = block->normEnergySlope.samples;
    }

    block->normEnergyFilter++;
    if (block->normEnergyFilter == block->normEnergy.end) {
        block->normEnergyFilter = block->normEnergy.samples;
    }

    if (block->initializeNormEnergy != 0) {
        f32 latest;
        f32 *destination = block->normEnergySlope.samples;

        if (block->normEnergySlope.write == block->normEnergySlope.samples) {
            latest = block->normEnergySlope.end[-1];
        } else {
            latest = block->normEnergySlope.write[-1];
        }
        for (i = 0; i < block->normEnergyWindowLength / 2; i++) {
            *destination++ = latest;
        }
        block->initializeNormEnergy = 0;
    }
}

// Advances every output cursor after one feature frame has been consumed.
// ProcessPitchCombiner and FlushPitchCombiner call this after queueing output.
static void OutputProcedure(PitchCombiner *block)
{
    block->logPitch.output++;
    if (block->logPitch.output == block->logPitch.end) {
        block->logPitch.output = block->logPitch.samples;
    }
    block->meanPitch.output++;
    if (block->meanPitch.output == block->meanPitch.end) {
        block->meanPitch.output = block->meanPitch.samples;
    }
    block->pitchSlope.output++;
    if (block->pitchSlope.output == block->pitchSlope.end) {
        block->pitchSlope.output = block->pitchSlope.samples;
    }
    block->normEnergy.output++;
    if (block->normEnergy.output == block->normEnergy.end) {
        block->normEnergy.output = block->normEnergy.samples;
    }
    block->normEnergySlope.output++;
    if (block->normEnergySlope.output == block->normEnergySlope.end) {
        block->normEnergySlope.output = block->normEnergySlope.samples;
    }
    block->voicing.output++;
    if (block->voicing.output == block->voicing.end) {
        block->voicing.output = block->voicing.samples;
    }
}

// Pads pending feature histories and emits at most one delayed frame per call after input ends.
// ProcessPitchCombiner calls this on an empty input while flushing is active.
static u8 FlushPitchCombiner(PitchCombiner *block)
{
    if (block->queuedFrameCount != 0) {
        f32 *source;
        f32 *destination;
        f32 *output;
        u16 i;
        u8 halfLength;

        if (block->flushPitch != 0) {
            // Extend pitch results only when at least window length minus one frames remain.
            if (block->queuedFrameCount >=
                (u8)(block->pitchWindowLength - 1)) {
                halfLength = block->pitchWindowLength / 2;
                source = block->pitchSlope.write - 1;
                if (source < block->pitchSlope.samples) {
                    source = block->pitchSlope.end - 1;
                }
                for (i = 0; i < halfLength; i++) {
                    *block->pitchSlope.write++ = *source;
                    if (block->pitchSlope.write == block->pitchSlope.end) {
                        block->pitchSlope.write = block->pitchSlope.samples;
                    }
                }

                source = block->meanPitch.write - 1;
                if (source < block->meanPitch.samples) {
                    source = block->meanPitch.end - 1;
                }
                for (i = 0; i < halfLength; i++) {
                    *block->meanPitch.write++ = *source;
                    if (block->meanPitch.write == block->meanPitch.end) {
                        block->meanPitch.write = block->meanPitch.samples;
                    }
                }
            }
            block->flushPitch = 0;
        }

        if (block->flushNormEnergy != 0) {
            source = block->normEnergySlope.write - 1;
            destination = block->normEnergySlope.write;
            halfLength = block->normEnergyWindowLength / 2;
            if (source < block->normEnergySlope.samples) {
                source = block->normEnergySlope.end - 1;
            }
            for (i = 0; i < halfLength; i++) {
                *destination++ = *source;
                if (destination == block->normEnergySlope.end) {
                    destination = block->normEnergySlope.samples;
                }
            }
            block->flushNormEnergy = 0;
        }

        block->pitchDelta =
            *block->logPitch.output - *block->meanPitch.output;
        output = qEnQueueOne(block->base.output->queue);
        *output++ = 32.0f * block->pitchDelta;
        *output++ = 53.0f * *block->pitchSlope.output;
        *output++ = 1.107 * *block->normEnergy.output;
        *output++ = 0.832 * *block->normEnergySlope.output;
        if (*block->voicing.output == 1.0f) {
            *output = block->voicedOutput;
        } else {
            *output = block->unvoicedOutput;
        }

        OutputProcedure(block);
        block->queuedFrameCount--;
        if (block->queuedFrameCount != 0) {
            return 1;
        }
    }

    block->flushing = 0;
    return 0;
}

// Consumes one pitch, voicing, and spectrum frame and emits ready features.
// tosRun invokes this for input frames and empty-input ticks; inputCount is unused.
static void ProcessPitchCombiner(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
    PitchCombiner *block = (PitchCombiner *)baseBlock;

    if (inputs[0] != NULL) {
        f32 *output;

        // Skip the spectrum's leading value and take the maximum of its configured bands.
        PushPitchHistory(&block->normEnergy,
            MaxFloatInArray((f32 *)inputs[2] + 1, block->bandCount));

        *block->logPitch.write++ = logf_check(*(f32 *)inputs[0]);
        if (block->logPitch.write == block->logPitch.end) {
            block->logPitch.write = block->logPitch.samples;
        }

        *block->voicing.write = *(u8 *)inputs[1];
        block->queuedFrameCount++;
        StatusProcessing(block);

        if (block->queuedFrameCount >= block->pitchWindowLength) {
            LogPitchProcessing(block);
        }
        if (block->voiced == 0) {
            block->logPitchFilter = block->logPitch.write;
        } else if (block->pitchFilterReady == 1) {
            block->logPitchFilter++;
            if (block->logPitchFilter == block->logPitch.end) {
                block->logPitchFilter = block->logPitch.samples;
            }
            block->pitchFilterReady = 0;
        }

        if (block->queuedFrameCount >= block->normEnergyWindowLength) {
            NormE_Processing(block);
        }

        if (block->queuedFrameCount == block->delayLength) {
            // Emit scaled pitch deviation, pitch slope, band energy, energy slope and voicing.
            block->pitchDelta =
                *block->logPitch.output - *block->meanPitch.output;
            output = qEnQueueOne(block->base.output->queue);
            *output++ = 32.0f * block->pitchDelta;
            *output++ = 53.0f * *block->pitchSlope.output;
            *output++ = 1.107 * *block->normEnergy.output;
            *output++ = 0.832 * *block->normEnergySlope.output;
            if (*block->voicing.output == 1.0f) {
                *output = block->voicedOutput;
            } else {
                *output = block->unvoicedOutput;
            }

            OutputProcedure(block);
            block->queuedFrameCount--;
        }
    } else if (block->flushing == 1) {
        FlushPitchCombiner(block);
    }
}

// Clears histories and frame state when the TOS block receives its reset command.
// The cached lastPitchSlope is left unchanged.
static void PitchCombinerReset(PitchCombiner *block)
{
    ResetPitchHistory(&block->voicing, block->delayLength);
    ResetPitchHistory(&block->normEnergy, block->delayLength);
    ResetPitchHistory(&block->logPitch, block->delayLength);
    ResetPitchHistory(&block->meanPitch, block->delayLength);
    ResetPitchHistory(&block->pitchSlope, block->delayLength);
    ResetPitchHistory(&block->normEnergySlope, block->delayLength);

    block->meanPitch.write =
        block->meanPitch.samples + block->pitchWindowLength / 2;
    block->pitchSlope.write =
        block->pitchSlope.samples + block->pitchWindowLength / 2;
    block->normEnergySlope.write =
        block->normEnergySlope.samples + block->normEnergyWindowLength / 2;
    block->logPitchFilter = block->logPitch.output;
    block->normEnergyFilter = block->normEnergy.output;

    block->pitchDelta = 0.0f;
    block->lastMeanPitch = 0.0f;
    block->voicedFrameCount = 0;
    block->voiced = 0;
    block->previousVoiced = 0;
    block->queuedFrameCount = 0;
    block->beginVoiced = 0;
    block->endVoiced = 0;
    block->initializeNormEnergy = 1;
    block->flushing = 0;
    block->flushPitch = 1;
    block->flushNormEnergy = 1;
    block->pitchFilterReady = 0;
}

// Handles reset, flush, and destruction through the speech graph's control callback.
// Only the command's low byte is used; the argument pointer and size are ignored.
static u32 ControlPitchCombiner(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    PitchCombiner *block = (PitchCombiner *)baseBlock;

    switch ((u8)command) {
    case PITCH_COMBINER_RESET_COMMAND:
        PitchCombinerReset(block);
        break;
    case PITCH_COMBINER_FLUSH_COMMAND:
        block->flushing = 1;
        break;
    case PITCH_COMBINER_DESTRUCT_COMMAND:
        heap_Free(block->base.context->heap, block->normEnergy.samples);
        heap_Free(block->base.context->heap, block->logPitch.samples);
        heap_Free(block->base.context->heap, block->voicing.samples);
        heap_Free(block->base.context->heap, block->pitchSlope.samples);
        heap_Free(block->base.context->heap, block->meanPitch.samples);
        heap_Free(block->base.context->heap, block->normEnergySlope.samples);
        heap_Free(block->base.context->heap, block->pitchCoefficients);
        heap_Free(block->base.context->heap, block->normEnergyCoefficients);
        tosBaseBlockDestruct(block);
        break;
    default:
        return 0;
    }
    return 1;
}

// Allocates histories and sets window coefficients during speech graph initialization.
// The graph invokes this through the initializer registered by ConstructPitchCombiner.
static u32 InitPitchCombiner(TosBaseBlock *baseBlock)
{
    PitchCombiner *block = (PitchCombiner *)baseBlock;
    TosContext *context = baseBlock->context;
    u32 bandCount;

    bandCount = _tosGetProfileU32(block, PITCH_COMBINER_BAND_COUNT_PROFILE, 21);
    block->bandCount = (u16)bandCount;
    block->lastMeanPitch = 0.0f;
    block->outputFeatureCount = 5;
    block->base.input[0].inputSize = sizeof(f32);
    block->base.input[1].inputSize = sizeof(u8);
    block->base.input[2].inputSize =
        ((u16)bandCount + 1) * sizeof(f32);
    block->base.output->outputSize =
        block->outputFeatureCount * sizeof(f32);

    block->pitchWindowLength = 7;
    block->normEnergyWindowLength = 5;
    block->delayLength =
        block->pitchWindowLength > block->normEnergyWindowLength
            ? block->pitchWindowLength
            : block->normEnergyWindowLength;

    block->normEnergy.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->logPitch.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->voicing.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->pitchSlope.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->meanPitch.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->normEnergySlope.samples =
        heap_Calloc(context->heap, block->delayLength, sizeof(f32));
    block->pitchCoefficients =
        heap_Calloc(context->heap, block->pitchWindowLength, sizeof(f32));
    block->normEnergyCoefficients = heap_Calloc(
        context->heap, block->normEnergyWindowLength, sizeof(f32));

    if (block->normEnergy.samples == NULL ||
        block->logPitch.samples == NULL ||
        block->voicing.samples == NULL ||
        block->pitchSlope.samples == NULL ||
        block->meanPitch.samples == NULL ||
        block->normEnergySlope.samples == NULL ||
        block->pitchCoefficients == NULL ||
        block->normEnergyCoefficients == NULL) {
        _tosErrorLog(block, 2);
        return 1;
    }

    ResetPitchHistory(&block->voicing, block->delayLength);
    ResetPitchHistory(&block->normEnergy, block->delayLength);
    ResetPitchHistory(&block->logPitch, block->delayLength);
    ResetPitchHistory(&block->meanPitch, block->delayLength);
    ResetPitchHistory(&block->pitchSlope, block->delayLength);
    ResetPitchHistory(&block->normEnergySlope, block->delayLength);

    block->meanPitch.write =
        block->meanPitch.samples + block->pitchWindowLength / 2;
    block->pitchSlope.write =
        block->pitchSlope.samples + block->pitchWindowLength / 2;
    block->normEnergySlope.write =
        block->normEnergySlope.samples + block->normEnergyWindowLength / 2;
    block->logPitchFilter = block->logPitch.output;
    block->normEnergyFilter = block->normEnergy.output;
    block->pitchDelta = 0.0f;

    block->pitchCoefficients[0] = -3.0f;
    block->pitchCoefficients[1] = -2.0f;
    block->pitchCoefficients[2] = -1.0f;
    block->pitchCoefficients[3] = 0.0f;
    block->pitchCoefficients[4] = 1.0f;
    block->pitchCoefficients[5] = 2.0f;
    block->pitchCoefficients[6] = 3.0f;
    block->pitchCoefficientDivisor = 28;

    block->normEnergyCoefficients[0] = -2.0f;
    block->normEnergyCoefficients[1] = -1.0f;
    block->normEnergyCoefficients[2] = 0.0f;
    block->normEnergyCoefficients[3] = 1.0f;
    block->normEnergyCoefficients[4] = 2.0f;
    block->normEnergyCoefficientDivisor = 10;

    block->voicedFrameCount = 0;
    block->voiced = 0;
    block->previousVoiced = 0;
    block->queuedFrameCount = 0;
    block->beginVoiced = 0;
    block->endVoiced = 0;
    block->initializeNormEnergy = 1;
    block->flushing = 0;
    block->flushPitch = 1;
    block->flushNormEnergy = 1;
    block->pitchFilterReady = 0;

    block->voicedOutput =
        _tosGetProfileFloat(block, PITCH_COMBINER_VOICED_OUTPUT_PROFILE, 0.0f);
    block->unvoicedOutput =
        _tosGetProfileFloat(block, PITCH_COMBINER_UNVOICED_OUTPUT_PROFILE, 100000.0f);
    return 0;
}

// Creates a speech feature block with pitch, voicing and spectrum inputs and one output.
// Speech graph construction uses this to register the processing and control callbacks.
void *ConstructPitchCombiner(TosContext *context, u8 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 3, 1,
        ProcessPitchCombiner, InitPitchCombiner, ControlPitchCombiner,
        sizeof(PitchCombiner));
}
