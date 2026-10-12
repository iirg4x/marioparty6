/* Speech-mean adaptation and spectral normalization for the ASR feature graph. */

#include <stddef.h>
#include <string.h>

#include "gssdk/mqueue.h"
#include "gssdk/smoother.h"

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern TosQueue *qQueueInitEx(
    TosQueue *queue, u16 elementSize, u8 initializeReaders,
    u16 maxElements);
extern u8 qQueueControl(
    TosQueue *queue, u32 command, u16 elementSize, void *argument);
extern void qQueueReset(TosQueue *queue);
extern void qFreeUnusedElemements(TosQueue *queue);
extern f32 HLnOnePlusExpHFloat(f32 value);
extern f32 LogAdd(f32 value1, f32 value2);

typedef struct Acne {
    /* TOS graph callbacks and the block's input and output ports. */
    TosBaseBlock base;
    /* FIFO of feature frames waiting for segment normalization. */
    TosQueue *queue;
    /* Queue length at which state 1 emits its oldest frame; the queue itself has no capacity
     * limit. */
    u16 queueEmissionLength;
    /* 0 emits each frame, 1 retains speech frames, and 2 waits for activity. */
    s32 state;
    /* Set while queued frames are being emitted during a drain. */
    u8 drainingQueue;
    /* Enables normalization with smoothedMean; a failed refresh does not clear this flag. */
    u8 meanNormalizationEnabled;
    /* Per-band smoothed adaptive speech mean in log-amplitude units. */
    f32 *smoothedMean;
    /* Per-band speech mean updated from incoming feature frames. */
    f32 *adaptiveMean;
    /* Per-band noise estimate copied from the feature extractor. */
    f32 *noiseEstimate;
    /* Latest scalar reference used when converting every queued frame to output energy. */
    f32 inputReference;
    /* Accumulated weight used while adapting the speech mean. */
    f32 accumulatedWeight;
    /* Count of zero-activity frames since the first input, capped at 50 until reset. */
    s32 zeroFrameCount;
    /* Capped total activity weight used for the uniform correction applied to every band. */
    f32 cumulativeWeight;
    /* Progress toward refreshing the smoothed mean. */
    f32 smoothingProgress;
    /* Number of feature frames received since the last reset. */
    u32 frameCount;
    /* Consecutive zero-activity frames used to end a speech segment. */
    s32 consecutiveZeroFrames;
    /* Number of spectral bands in each feature frame. */
    u32 bandCount;
    /* Smoothing progress required before refreshing smoothedMean. */
    f32 smoothingThreshold;
    /* Maximum silent frames accepted before the block returns to silence. */
    s32 maximumConsecutiveZeroFrames;
    /* Maximum accumulated activity weight before queued frames are drained. */
    f32 maximumAccumulatedWeight;
    /* Minimum scalar energy written to the normalized output. */
    f32 scalarFloor;
    /* Ratio used for positive and negative adaptive-mean and residual corrections. */
    f32 adaptationRatio;
    /* Offset applied to each per-band noise estimate before comparison. */
    f32 noiseFloorOffset;
    /* Lower bound applied to the adjusted-input minus adaptive-mean difference before each mean
     * update. */
    f32 minimumBandDifference;
    /* Unused storage between the per-band settings and initialization weight. */
    u32 unusedStorage;
    /* Initial activity weight used when the speech means are seeded. */
    f32 initialWeight;
    /* Per-band output floors applied with the smoothed mean, or copied as output before it is
     * available. */
    const f32 *noiseFloors;
    /* Initial per-band speech means; initialization always uses the 11 kHz table. */
    const f32 *initialMean;
    /* Smoothing matrices; initialization always uses the 11 kHz set. */
    const Smoother1600Info *smootherInfo;
    /* Smoother used to prepare the initial mean and refresh the adaptive per-band mean. */
    Smoother *smoother;
} Acne;

/* Scalar values saved after the three per-band arrays in an ASR session. */
typedef struct AcneSessionScalars {
    /* Scalar reference used by queued frames. */
    f32 inputReference;
    /* Activity weight accumulated before the session was saved. */
    f32 accumulatedWeight;
    /* Number of silent frames tracked at the time of saving. */
    s32 zeroFrameCount;
    /* Capped activity weight used for mean correction. */
    f32 cumulativeWeight;
    /* Progress toward the next smoothed-mean refresh. */
    f32 smoothingProgress;
    /* Number of feature frames processed before the save. */
    u32 frameCount;
    /* Consecutive silent frames at the time of saving. */
    s32 consecutiveZeroFrames;
} AcneSessionScalars;

typedef char AcneMinimumBandDifferenceOffset[
    (offsetof(Acne, minimumBandDifference) == 124) ? 1 : -1];
typedef char AcneUnusedStorageOffset[
    (offsetof(Acne, unusedStorage) == 128) ? 1 : -1];
typedef char AcneInitialWeightOffset[
    (offsetof(Acne, initialWeight) == 132) ? 1 : -1];
typedef char AcneSmootherOffset[
    (offsetof(Acne, smoother) == 148) ? 1 : -1];
typedef char AcneSize[(sizeof(Acne) == 152) ? 1 : -1];
typedef char AcneSessionScalarsSize[
    (sizeof(AcneSessionScalars) == 28) ? 1 : -1];

static const f32 g_SpeechMeanInit11kHz[21] = {
    7.5637002f,  8.22809982f, 8.59599972f, 8.69009972f,
    8.71949959f, 8.73530006f, 8.74090004f, 8.7439003f,
    8.74450016f, 8.74419975f, 8.74380016f, 8.74300003f,
    8.74139977f, 8.73900032f, 8.73379993f, 8.72389984f,
    8.70829964f, 8.67770004f, 8.62969971f, 8.51099968f,
    8.2116003f,
};

static const f32 g_NoiseFloors11kHz[21] = {
    -1.67426932f, -2.18509817f, -2.52157259f, -2.77288866f,
    -2.87824988f, -2.87824988f, -2.87824988f, -2.87824988f,
    -2.87824988f, -2.87824988f, -2.87824988f, -2.87824988f,
    -2.87824988f, -2.87824988f, -2.87824988f, -2.87824988f,
    -2.87824988f, -2.87824988f, -2.87824988f, -2.98695707f,
    -3.05103707f,
};

static u8 Reset(Acne *block);
static void AcneGetSession(Acne *block, u8 *session);
static void AcnePutSession(Acne *block, const u8 *session);
static u8 NormalizeAndEnQ(Acne *block);
static void ProcessAcne(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount);
static u32 ControlAcne(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize);
static u32 InitAcne(TosBaseBlock *baseBlock);

/* Clears queued frames and seeds the adaptive speech model for a new session. */
static u8 Reset(Acne *block)
{
    qQueueReset(block->queue);
    block->state = 2;
    block->drainingQueue = 0;
    if (block->initialWeight > 0.0f) {
        memcpy(
            block->adaptiveMean, block->initialMean,
            block->bandCount * sizeof(f32));
        memcpy(
            block->smoothedMean, block->initialMean,
            block->bandCount * sizeof(f32));
        /* Smooth the starting mean before processing new frames. Failure keeps the emptied queue,
        * state 2, cleared drain flag, and copied or partly smoothed means; the normalization
        * flag and remaining estimator state are unchanged. */
        if ((u8)Smoothing(block->smoother, block->smoothedMean)) {
            return 1;
        }
        block->meanNormalizationEnabled = 1;
    } else {
        memset(
            block->adaptiveMean, 0,
            block->bandCount * sizeof(f32));
        block->meanNormalizationEnabled = 0;
    }

    memset(
        block->noiseEstimate, 0,
        block->bandCount * sizeof(f32));
    block->inputReference = 100000.0f;
    block->accumulatedWeight = block->initialWeight;
    block->zeroFrameCount = 0;
    block->cumulativeWeight = 0.5f;
    block->smoothingProgress = 0.0f;
    block->frameCount = 0;
    block->consecutiveZeroFrames = 0;
    return 0;
}

/* Saves the estimator state and per-band arrays for a TOS session snapshot. */
static void AcneGetSession(Acne *block, u8 *session)
{
    u32 arraySize = block->bandCount * sizeof(f32);
    AcneSessionScalars *scalars;

    *(u32 *)session = block->state;
    session[4] = block->drainingQueue;
    session[8] = block->meanNormalizationEnabled;
    /* The session header reserves twelve bytes before the per-band arrays. */
    session += 12;

    memcpy(session, block->smoothedMean, arraySize);
    session += arraySize;
    memcpy(session, block->adaptiveMean, arraySize);
    session += arraySize;
    memcpy(session, block->noiseEstimate, arraySize);
    session += arraySize;

    scalars = (AcneSessionScalars *)session;
    scalars->inputReference = block->inputReference;
    scalars->accumulatedWeight = block->accumulatedWeight;
    scalars->zeroFrameCount = block->zeroFrameCount;
    scalars->cumulativeWeight = block->cumulativeWeight;
    scalars->smoothingProgress = block->smoothingProgress;
    scalars->frameCount = block->frameCount;
    scalars->consecutiveZeroFrames = block->consecutiveZeroFrames;
}

/* Restores the estimator state and per-band arrays when a TOS session resumes. */
static void AcnePutSession(Acne *block, const u8 *session)
{
    u32 arraySize = block->bandCount * sizeof(f32);
    const AcneSessionScalars *scalars;

    block->state = *(const u32 *)session;
    block->drainingQueue = session[4];
    block->meanNormalizationEnabled = session[8];
    /* Skip the saved state and flags before restoring the per-band arrays. */
    session += 12;

    memcpy(block->smoothedMean, session, arraySize);
    session += arraySize;
    memcpy(block->adaptiveMean, session, arraySize);
    session += arraySize;
    memcpy(block->noiseEstimate, session, arraySize);
    session += arraySize;

    scalars = (const AcneSessionScalars *)session;
    block->inputReference = scalars->inputReference;
    block->accumulatedWeight = scalars->accumulatedWeight;
    block->zeroFrameCount = scalars->zeroFrameCount;
    block->cumulativeWeight = scalars->cumulativeWeight;
    block->smoothingProgress = scalars->smoothingProgress;
    block->frameCount = scalars->frameCount;
    block->consecutiveZeroFrames = scalars->consecutiveZeroFrames;
}

/* Attempts to normalize and publish one queued feature frame. A smoothing failure drops the
* frame but retains its queue node; a drain loop stalls if no unread frame remains to release it. */
static u8 NormalizeAndEnQ(Acne *block)
{
    f32 *outputElement;
    const f32 *noiseFloor;
    f32 *output;
    f32 *outputEnd;
    f32 *mean;
    u32 bandCount;
    f32 *input;

    noiseFloor = block->noiseFloors;
    bandCount = block->bandCount;

    input = qDeQueueOne(block->queue, 0);
    if (input == NULL) {
        return 0;
    }

    /* A refresh clears progress and overwrites smoothedMean before smoothing. Failure keeps
     * those changes and leaves meanNormalizationEnabled unchanged. */
    if (block->smoothingProgress > block->smoothingThreshold) {
        block->smoothingProgress = 0.0f;
        memcpy(
            block->smoothedMean, block->adaptiveMean,
            bandCount * sizeof(f32));
        if ((u8)Smoothing(block->smoother, block->smoothedMean)) {
            return 0;
        }
        block->meanNormalizationEnabled = 1;
    }

    outputElement = qEnQueueOne(block->base.output[0].queue);
    /* Queue elements store scalar energy in slot zero and spectral bands after it. */
    output = outputElement + 1;
    if (block->meanNormalizationEnabled) {
        /* Subtract the learned speech mean, then apply the per-band noise floor. */
        outputEnd = output + bandCount;
        memcpy(output, input, bandCount * sizeof(f32));
        mean = block->smoothedMean;

        while (output < outputEnd) {
            *output -= *mean++;
            if (*noiseFloor > *output) {
                *output = *noiseFloor +
                    HLnOnePlusExpHFloat(*output - *noiseFloor);
            } else {
                *output +=
                    HLnOnePlusExpHFloat(*noiseFloor - *output);
            }
            output++;
            noiseFloor++;
        }
    } else {
        memcpy(
            /* With mean normalization disabled, emit the configured floors for the spectral
             * bands. */
            output, noiseFloor,
            bandCount * sizeof(f32));
    }

    outputElement[0] = LogAdd(
        input[bandCount] - block->inputReference,
        block->scalarFloor);
    qFreeUnusedElemements(block->queue);
    return 1;
}

/* Queues and adapts a fresh feature frame unless a pending drain emits one queued frame and
* returns first. TOS has already dequeued fresh inputs, so that drain callback discards them. */
static void ProcessAcne(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
    Acne *block = (Acne *)baseBlock;
    u32 bandCount;
    f32 *input;
    f32 *featureInput;
    f32 *queuedInput;
    f32 activity;
    f32 scalarInput;
    u32 arraySize;

    (void)inputCount;
    if (block->accumulatedWeight >= block->maximumAccumulatedWeight) {
        block->state = 0;
        block->drainingQueue = 1;
        while (qQueueNbrElements(block->queue) != 0) {
            NormalizeAndEnQ(block);
        }
    }

    if (block->drainingQueue &&
        qQueueNbrElements(block->queue) == 0) {
        block->drainingQueue = 0;
    }
    if (block->drainingQueue) {
        NormalizeAndEnQ(block);
        return;
    }

    if (inputs[0] == NULL) {
        return;
    }

    input = (f32 *)inputs[0];
    featureInput = (f32 *)inputs[3];
    /* The side input's first two values are scalar frame energy and its reference; activity is
     * input 2. */
    activity = *(f32 *)inputs[2];
    bandCount = block->bandCount;
    scalarInput = featureInput[0];
    block->inputReference = featureInput[1];
    arraySize = bandCount * sizeof(f32);
    memcpy(block->noiseEstimate, inputs[1], arraySize);

    queuedInput = qEnQueueOne(block->queue);
    memcpy(queuedInput, input, arraySize);
    queuedInput[bandCount] = scalarInput;
    block->frameCount++;

    if (activity > 0.001f) {
        f32 weight;
        f32 positiveRate;
        f32 negativeRate;
        f32 meanCorrection;
        f32 *noise = block->noiseEstimate;
        f32 *mean = block->adaptiveMean;
        f32 *meanEnd = mean + bandCount;
        f32 *queued = queuedInput;

        block->accumulatedWeight += activity;
        if (block->accumulatedWeight >=
            block->maximumAccumulatedWeight) {
            block->accumulatedWeight =
                block->maximumAccumulatedWeight;
        }

        /* Activity controls how quickly the adaptive model follows this frame. */
        weight = activity / block->accumulatedWeight;
        block->smoothingProgress += weight;
        positiveRate = weight * block->adaptationRatio;
        if (positiveRate > 1.0f) {
            positiveRate = 1.0f;
        }
        negativeRate = weight / block->adaptationRatio;
        if (negativeRate > 1.0f) {
            negativeRate = 1.0f;
        }

        meanCorrection = 0.0f;

        for (; mean < meanEnd; mean++, queued++, noise++) {
            f32 noiseFloor =
                *noise + block->noiseFloorOffset;
            f32 adjustedInput = *queued;
            f32 difference;
            f32 update;
            f32 residual;

            if (adjustedInput < noiseFloor) {
                adjustedInput = noiseFloor;
            }
            difference = adjustedInput - *mean;
            if (difference < block->minimumBandDifference) {
                difference = block->minimumBandDifference;
            }
            if (difference > 0.0f) {
                update = difference * positiveRate;
            } else {
                update = difference * negativeRate;
            }
            *mean += update;

            residual = adjustedInput - *mean;
            if (residual > 0.0f) {
                update = residual * block->adaptationRatio;
            } else {
                update = residual / block->adaptationRatio;
            }
            meanCorrection += update;
        }

        /* Apply a weighted band-average residual correction to every band, capped at +/-0.7. */
        block->cumulativeWeight += activity;
        if (block->cumulativeWeight > 200.0f) {
            block->cumulativeWeight = 200.0f;
        }
        weight = activity / block->cumulativeWeight;
        block->smoothingProgress += weight;
        meanCorrection *= weight / (f32)bandCount;
        if (meanCorrection > 0.7f) {
            meanCorrection = 0.7f;
        }
        if (meanCorrection < -0.7f) {
            meanCorrection = -0.7f;
        }

        mean = block->adaptiveMean;
        while (mean < meanEnd) {
            *mean += meanCorrection;
            mean++;
        }
    }

    if (!activity && block->frameCount != 0) {
        block->consecutiveZeroFrames++;
        block->zeroFrameCount++;
        if (block->zeroFrameCount > 50) {
            block->zeroFrameCount = 50;
        }
    } else {
        block->consecutiveZeroFrames = 0;
    }

    /* State 1 holds speech frames; the other states flush or wait for them. */
    switch (block->state) {
    case 0:
        NormalizeAndEnQ(block);
        break;
    case 2:
        if (activity > 0.001f) {
            block->state = 1;
        } else {
            NormalizeAndEnQ(block);
        }
        break;
    case 1:
        if (block->consecutiveZeroFrames >
            block->maximumConsecutiveZeroFrames) {
            block->state = 2;
            while (qQueueNbrElements(block->queue) != 0) {
                NormalizeAndEnQ(block);
            }
        } else if (qQueueNbrElements(block->queue) ==
            block->queueEmissionLength) {
            NormalizeAndEnQ(block);
        }
        break;
    }
}

/* Command 2 requests a drain on the next process callback, 3 saves when given a session buffer, 4
 * restores or resets, and 255 destroys the block. */
static u32 ControlAcne(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    u32 *sessionOffset = (u32 *)argumentSize;

    switch ((u8)command) {
    case 2:
        ((Acne *)baseBlock)->drainingQueue = 1;
        break;

    case 4:
        if (argument != NULL) {
            AcnePutSession(
                (Acne *)baseBlock, (u8 *)argument + *sessionOffset);
            /* The session payload includes a 12-byte header, three band arrays, and seven saved
             * scalar values. */
            *sessionOffset +=
                (((Acne *)baseBlock)->bandCount * 3 + 10) * sizeof(u32);
        } else if (Reset((Acne *)baseBlock)) {
            return 0;
        }
        break;

    case 3:
        if (argument != NULL) {
            AcneGetSession(
                (Acne *)baseBlock, (u8 *)argument + *sessionOffset);
        }
        /* Advance over the header, three band arrays, and seven scalars even without a session
         * buffer. */
        *sessionOffset +=
            (((Acne *)baseBlock)->bandCount * 3 + 10) * sizeof(u32);
        break;

    case 255: {
        TosContext *context = baseBlock->context;

        qQueueControl(((Acne *)baseBlock)->queue, 255, 0, NULL);
        heap_Free(context->heap, ((Acne *)baseBlock)->adaptiveMean);
        smtDestruct(((Acne *)baseBlock)->smoother, context);
        tosBaseBlockDestruct(baseBlock);
        break;
    }

    default:
        return 0;
    }

    return 1;
}

/* Allocates the block queues and speech-model buffers when the graph starts. */
static u32 InitAcne(TosBaseBlock *baseBlock)
{
    Acne *block = (Acne *)baseBlock;
    TosContext *context = baseBlock->context;
    u32 bandCount;
    u32 arraySize;
    f32 knotSpacing;
    const Smoother1600Info *smootherInfo;

    /* Profile values select the feature width and the queued-frame threshold for emission. */
    bandCount = block->bandCount =
        (u16)_tosGetProfileU32(block, 9, 21);
    arraySize = bandCount * sizeof(f32);
    baseBlock->input[0].inputSize = (u16)arraySize;
    baseBlock->input[1].inputSize = (u16)arraySize;
    baseBlock->input[2].inputSize = sizeof(f32);
    baseBlock->input[3].inputSize = 24;
    baseBlock->output[0].outputSize =
        (u16)((bandCount + 1) * sizeof(f32));

    block->queueEmissionLength =
        (u16)_tosGetProfileU32(block, 2, 100);
    block->queue = qQueueConstruct(context, 255, 1);
    if (block->queue == NULL) {
        return 1;
    }
    if (qQueueInitEx(
            block->queue, (u16)((bandCount + 1) * sizeof(f32)), 1, 0) == NULL) {
        return 1;
    }

    block->adaptiveMean =
        heap_Calloc(context->heap, bandCount * 3, sizeof(f32));
    block->noiseEstimate = block->adaptiveMean + bandCount;
    block->smoothedMean = block->noiseEstimate + bandCount;

    block->noiseFloorOffset =
        0.11513f * _tosGetProfileFloat(block, 3, -1000.0f);
    block->minimumBandDifference =
        0.11513f * _tosGetProfileFloat(block, 4, -1000.0f);
    block->adaptationRatio = 4.0f;
    block->maximumAccumulatedWeight =
        _tosGetProfileFloat(block, 5, 50.0f);
    block->smoothingThreshold =
        10.0f / block->maximumAccumulatedWeight;
    block->maximumConsecutiveZeroFrames =
        (s16)_tosGetProfileU32(block, 6, 15);
    block->scalarFloor =
        0.23026f * _tosGetProfileFloat(block, 7, -30.0f);

    block->initialWeight = 5.0f;
    block->noiseFloors = g_NoiseFloors11kHz;
    block->initialMean = g_SpeechMeanInit11kHz;
    block->smootherInfo = &Smoother1600Info11kHz;
    knotSpacing = _tosGetProfileFloat(block, 8, 2.6f);
    smootherInfo = block->smootherInfo;
    block->smoother = smtConstruct(
        context, bandCount,
        knotSpacing, 0.0f,
        smootherInfo);

    if (Reset(block)) {
        return 1;
    }
    return 0;
}

/* Allocates the adaptive feature block and installs its TOS callbacks during graph construction. */
TosBaseBlock *ConstructAcne(TosContext *context, u8 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 4, 1, ProcessAcne, InitAcne,
        ControlAcne, sizeof(Acne));
}
