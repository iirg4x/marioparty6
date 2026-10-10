// Detects speech onset and end from audio activity, energy, and voicing features.
#include <stdlib.h>

#include "gssdk/triggerlr.h"

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 logf_check(f32 value);
extern u8 qQueueControl(
    TosQueue *queue, u32 command, u32 elementSize, void *argument);
extern u8 tosBaseBlockStartInputQueues(TosBaseBlock *block, u32 reader);
extern u8 tosBaseBlockDisableInputQueues(TosBaseBlock *block);

static s32 TriggerLR_FindSpeech(TriggerLR *block, void **inputs);
static void ProcessTriggerLR(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount);
static u32 ControlTriggerLR(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize);
static u32 InitTriggerLR(TosBaseBlock *baseBlock);

// Returns the middle of three voicing scores while TriggerLR checks a speech candidate.
static inline f32 MedianVoicingHistory(const f32 *voicingHistory)
{
    f32 low;
    f32 high;
    f32 third;
    f32 median;

    if (voicingHistory[0] < voicingHistory[1]) {
        low = voicingHistory[0];
        high = voicingHistory[1];
    } else {
        low = voicingHistory[1];
        high = voicingHistory[0];
    }

    third = voicingHistory[2];
    if (third > high) {
        median = high;
    } else if (third > low) {
        median = third;
    } else {
        median = low;
    }
    return median;
}

// Updates speech-detection state from the current frame's onset, energy, and pitch features.
// ProcessTriggerLR calls this once for each available input frame.
static s32 TriggerLR_FindSpeech(TriggerLR *block, void **inputs)
{
    s32 controlStatus = 0;
    f32 histogramThreshold;
    f32 peakMetric;
    f32 energyThreshold;
    f32 onsetMetric;
    f32 activity;
    f32 noiseThreshold;
    f32 *featureValues = (f32 *)inputs[1];

    noiseThreshold = featureValues[3] + block->noiseFloorOffsetLog;
    onsetMetric = *(f32 *)inputs[2];
    activity = featureValues[0];
    peakMetric = *(f32 *)inputs[3];
    histogramThreshold = noiseThreshold > block->energyFloorLog ?
        noiseThreshold : block->energyFloorLog;
    if ((s32)block->histogramValid == 0) {
        block->histogramLowerQuantile =
            histogramThreshold + block->histogramThresholdOffsetLog;
    }

    energyThreshold =
        block->histogramLowerQuantile - block->histogramThresholdOffsetLog;
    energyThreshold = energyThreshold > histogramThreshold ?
        energyThreshold : histogramThreshold;

    switch (block->speechState) {
    case 0:
        if (block->triggerLookbackFrames > block->silenceFrameLimit) {
            block->triggerLookbackFrames = block->silenceFrameLimit;
        }

        if (onsetMetric > block->onsetThresholdLog) {
            block->speechState = 1;
            block->histogramUpdated = 0;
            block->energyCriterionMet = 0;
            block->consecutiveCriterionMet = 0;
            block->activitySeen = 0;
            block->candidateFrames = 1;
            block->silenceFrames = 0;
            block->consecutiveEnergeticFrames =
                activity >= energyThreshold ? 1 : 0;
            block->histogramCandidatePeak = peakMetric;
            block->peakActivityLog = onsetMetric;

            if ((s32)block->processMode == 0) {
                // Keep empty voicing slots below measured scores until speech supplies a value.
                block->voicingDecisionHistory[0] = -2147483600.0f;
                block->voicingDecisionHistory[1] = -2147483600.0f;
                block->voicingDecisionHistory[2] = -2147483600.0f;
                block->voicingDecisionHistoryWrite =
                    block->voicingDecisionHistory;

                if (activity >= energyThreshold) {
                    *block->voicingDecisionHistoryWrite =
                        Voicing_GetMaxVoicing(block);
                }
                block->voicingCriterionMet = 0;
            }
        } else {
            if ((s32)block->speechActive != 0 &&
                (s32)block->holdInputQueuesAfterTrigger != 0) {
                block->silenceFrames++;
            }
            if ((s32)block->processMode == 0) {
                Voicing_MaintainNoiseEner(block);
            }
        }
        break;

    case 1:
        if ((s32)block->voicingCriterionMet == 0) {
            if (++block->voicingDecisionHistoryWrite >=
                block->voicingDecisionHistoryEnd) {
                block->voicingDecisionHistoryWrite =
                    block->voicingDecisionHistory;
            }
            // An unavailable voicing measurement must not satisfy the median threshold.
            *block->voicingDecisionHistoryWrite = -2147483600.0f;
        }

        if (onsetMetric > block->activityThresholdLog) {
            block->histogramCandidatePeak = peakMetric > block->histogramCandidatePeak ?
                peakMetric : block->histogramCandidatePeak;
            block->peakActivityLog = onsetMetric > block->peakActivityLog ?
                onsetMetric : block->peakActivityLog;
            block->candidateFrames++;

            if (activity >= energyThreshold) {
                block->consecutiveEnergeticFrames++;
                block->activitySeen = 1;
            } else {
                block->consecutiveEnergeticFrames = 0;
            }

            block->consecutiveCriterionMet =
                (s32)block->consecutiveCriterionMet != 0 ||
                block->consecutiveEnergeticFrames >= block->consecutiveEnergeticLimit;

            if (activity >= energyThreshold &&
                (s32)block->voicingCriterionMet == 0) {
                *block->voicingDecisionHistoryWrite =
                    Voicing_GetMaxVoicing(block);
                if (MedianVoicingHistory(block->voicingDecisionHistory) >
                    block->voicingDecisionThreshold) {
                    block->voicingCriterionMet = 1;
                }
            }

            block->energyCriterionMet = (s32)block->energyCriterionMet != 0 ||
                ((block->candidateFrames >= block->minimumSpeechFrames &&
                    block->peakActivityLog > block->peakActivityThresholdLog) &&
                    block->histogramCandidatePeak >= histogramThreshold);

            if (block->candidateFrames >= block->histogramUpdateFrames &&
                (s32)block->energyCriterionMet != 0) {
                SlidingHisto_NewItem(block, block->histogramCandidatePeak);
                block->histogramUpdated = 1;
                block->histogramValid = 1;
                block->histogramLowerQuantile =
                    SlidingHisto_LowerQuantile(
                        block, block->histogramQuantile);
                block->candidateFrames = block->minimumSpeechFrames;
                block->histogramCandidatePeak = peakMetric;
            }

            if ((s32)block->energyCriterionMet != 0 &&
                (s32)block->consecutiveCriterionMet != 0 &&
                (s32)block->voicingCriterionMet != 0) {
                block->speechState = 3;
                block->speechActive = 1;
            }
        } else {
            block->speechState = 2;
            block->silenceFrames = 1;
        }
        break;

    case 2:
        if ((s32)block->voicingCriterionMet == 0) {
            if (++block->voicingDecisionHistoryWrite >=
                block->voicingDecisionHistoryEnd) {
                block->voicingDecisionHistoryWrite =
                    block->voicingDecisionHistory;
            }
            // Silence also advances the three-frame history with an unavailable score.
            *block->voicingDecisionHistoryWrite = -2147483600.0f;
        }

        if (onsetMetric > block->onsetThresholdLog) {
            block->speechState = 1;
            block->candidateFrames = 1;
            block->histogramCandidatePeak = peakMetric > block->histogramCandidatePeak ?
                peakMetric : block->histogramCandidatePeak;
            block->peakActivityLog = onsetMetric;
            block->energyCriterionMet = 0;
            block->consecutiveCriterionMet = 0;
            block->consecutiveEnergeticFrames =
                activity >= energyThreshold ? 1 : 0;

            if (activity >= energyThreshold &&
                (s32)block->voicingCriterionMet == 0) {
                *block->voicingDecisionHistoryWrite =
                    Voicing_GetMaxVoicing(block);
                if (MedianVoicingHistory(block->voicingDecisionHistory) >
                    block->voicingDecisionThreshold) {
                    block->voicingCriterionMet = 1;
                }
            }
        } else {
            if (++block->silenceFrames > block->silenceFrameLimit) {
                block->speechState = 0;
                if ((s32)block->activitySeen != 0 &&
                    _tosControl(
                        &block->base, 202, 70, block->frameCount)) {
                    controlStatus = 1;
                    break;
                }
                if ((s32)block->histogramUpdated == 0 &&
                    (s32)block->energyCriterionMet != 0) {
                    SlidingHisto_NewItem(
                        block, block->histogramCandidatePeak);
                    block->histogramUpdated = 1;
                    block->histogramValid = 1;
                    block->histogramLowerQuantile =
                        SlidingHisto_LowerQuantile(
                            block, block->histogramQuantile);
                }
            }
        }
        break;

    case 3:
        if (onsetMetric > block->activityThresholdLog) {
            block->histogramCandidatePeak = peakMetric > block->histogramCandidatePeak ?
                peakMetric : block->histogramCandidatePeak;
            if (++block->candidateFrames >= block->histogramUpdateFrames) {
                SlidingHisto_NewItem(block, block->histogramCandidatePeak);
                block->histogramUpdated = 1;
                block->histogramValid = 1;
                block->histogramLowerQuantile =
                    SlidingHisto_LowerQuantile(
                        block, block->histogramQuantile);
                block->candidateFrames = 0;
                block->histogramCandidatePeak = peakMetric;
            }
        } else {
            block->speechState = 4;
            block->silenceFrames = 1;
        }
        break;

    case 4:
        if (onsetMetric > block->onsetThresholdLog) {
            block->speechState = 3;
            block->candidateFrames++;
            block->histogramCandidatePeak = peakMetric > block->histogramCandidatePeak ?
                peakMetric : block->histogramCandidatePeak;
        } else {
            if (++block->silenceFrames > block->silenceFrameLimit) {
                block->speechState = 0;
                if ((s32)block->histogramUpdated == 0) {
                    SlidingHisto_NewItem(
                        block, block->histogramCandidatePeak);
                    block->histogramUpdated = 1;
                    block->histogramValid = 1;
                    block->histogramLowerQuantile =
                        SlidingHisto_LowerQuantile(
                            block, block->histogramQuantile);
                }
            }
        }
        break;
    }

    return controlStatus;
}

// Processes each scheduled audio frame, raising trigger events and updating input queues.
// TinyOS calls this through the process callback installed by ConstructTriggerLR.
static void ProcessTriggerLR(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount)
{
    TriggerLR *block = (TriggerLR *)baseBlock;
    s16 *signal;

    (void)inputCount;
    if (inputs[0] == NULL) {
        return;
    }

    block->frameCount++;
    if ((s32)block->processMode == 0) {
        signal = (s16 *)inputs[0];
        if (Voicing_AddSignal(block, signal)) {
            return;
        }
        if (++block->triggerLookbackFrames >
            block->triggerLookbackLimit) {
            block->triggerLookbackFrames = block->triggerLookbackLimit;
        }
    }

    if (TriggerLR_FindSpeech(block, inputs)) {
        return;
    }

    if ((s32)block->processMode == 0) {
        if ((s32)block->speechState != 3) {
            return;
        }

        block->speechStartFrame =
            block->frameCount - block->triggerLookbackFrames;
        // Queue command 8 disables this reader; a failure leaves the trigger unreported.
        if ((s32)block->holdInputQueuesAfterTrigger == 0 &&
            !qQueueControl(
                block->base.input[0].queue, 8,
                block->base.input[0].outputSize, NULL)) {
            return;
        }

        if ((s32)block->triggerEventMode != 0) {
            if (_tosControl(
                    baseBlock, 203, block->triggerLookbackFrames,
                    block->frameCount)) {
                goto done;
            }
        } else {
            if (_tosControl(
                    baseBlock, 200, block->triggerLookbackFrames,
                    block->frameCount)) {
                goto done;
            }
        }

        if ((s32)block->holdInputQueuesAfterTrigger != 0) {
            block->processMode = 1;
        } else {
            tosBaseBlockDisableInputQueues(baseBlock);
        }
    } else if ((s32)block->processMode == 1 &&
        (s32)block->speechActive != 0 &&
        block->silenceFrames > block->endSilenceFrames) {
        block->speechActive = 0;
        if (_tosControl(baseBlock, 201, block->frameCount, 0)) {
            return;
        }
        block->processMode = 2;
    }
done:
    return;
}

// Applies reset, configuration, session, and destruction commands to the speech trigger.
// TinyOS invokes this callback when the block receives a control command.
static u32 ControlTriggerLR(
    TosBaseBlock *baseBlock, u32 command, void *argument, u32 argumentSize)
{
    TriggerLR *block = (TriggerLR *)baseBlock;
    TriggerLRControlData *controlData;
    u32 *sessionOffset;
    u32 sessionSize;
    u8 *sessionBuffer;
    u32 queueControlFailed;
    f32 sensitivityScale;
    f32 voicingThreshold;
    f32 histogramOffset;

    switch ((u8)command) {
    case 100:
        block->frameCount = 0;
        block->speechStartFrame = 0;
        block->triggerLookbackFrames = 0;
        block->candidateFrames = 0;
        block->silenceFrames = 0;
        block->speechActive = 0;
        block->voicingCriterionMet = 0;
        block->speechState = 0;
        Voicing_Reset(block);

        if ((s32)block->resetStartsInputQueues != 0) {
            block->processMode = 0;
            if (tosBaseBlockStartInputQueues(baseBlock, 0)) {
                return 0;
            }
        } else if ((s32)block->holdInputQueuesAfterTrigger != 0) {
            block->processMode = 1;
            queueControlFailed = !qQueueControl(
                baseBlock->input[1].queue, 5,
                baseBlock->input[1].outputSize, NULL);
            queueControlFailed |= !qQueueControl(
                baseBlock->input[2].queue, 5,
                baseBlock->input[2].outputSize, NULL);
            queueControlFailed |= !qQueueControl(
                baseBlock->input[3].queue, 5,
                baseBlock->input[3].outputSize, NULL);
            if (queueControlFailed) {
                return 0;
            }
        } else {
            block->processMode = 2;
            if (tosBaseBlockDisableInputQueues(baseBlock)) {
                return 0;
            }
        }
        break;

    case 103:
        controlData = (TriggerLRControlData *)argument;
        block->resetStartsInputQueues = controlData->resetStartsInputQueues;
        block->holdInputQueuesAfterTrigger = controlData->holdInputQueuesAfterTrigger;
        block->sensitivity = controlData->sensitivity;
        sensitivityScale =
            (f32)(block->sensitivity - block->sensitivityMinimum) /
            (f32)(block->sensitivityMaximum - block->sensitivityMinimum);
        histogramOffset = (f32)block->histogramOffsetMinimum;
        histogramOffset += sensitivityScale *
            (f32)(block->histogramOffsetMaximum - block->histogramOffsetMinimum);
        histogramOffset *= 0.23025851f;
        voicingThreshold = block->voicingThresholdMinimum;
        voicingThreshold += sensitivityScale *
            (block->voicingThresholdMaximum - block->voicingThresholdMinimum);
        block->histogramThresholdOffsetLog = histogramOffset;
        block->voicingDecisionThreshold = logf_check(voicingThreshold);
        block->endSilenceMilliseconds = controlData->endSilenceMilliseconds;
        block->minimumSpeechMilliseconds = controlData->minimumSpeechMilliseconds;
        block->endSilenceFrames = block->endSilenceMilliseconds / 10;
        block->minimumSpeechFrames = block->minimumSpeechMilliseconds / 10;
        block->energyFloorDbHundredths = controlData->energyFloorDbHundredths;
        block->energyFloorLog =
            27.37f + (f32)block->energyFloorDbHundredths / 4.3429446f / 100.0f;
        break;

    case 102:
        controlData = (TriggerLRControlData *)argument;
        controlData->resetStartsInputQueues = block->resetStartsInputQueues;
        controlData->holdInputQueuesAfterTrigger = block->holdInputQueuesAfterTrigger;
        controlData->endSilenceMilliseconds = block->endSilenceMilliseconds;
        controlData->sensitivity = block->sensitivity;
        controlData->minimumSpeechMilliseconds = block->minimumSpeechMilliseconds;
        controlData->energyFloorDbHundredths = block->energyFloorDbHundredths;
        break;

    case 101:
        *(u32 **)argument = &block->speechStartFrame;
        break;

    case 104:
        block->triggerEventMode = (u32)argument;
        break;

    case 105:
        sessionOffset = (u32 *)argumentSize;
        sessionBuffer = (u8 *)argument;
        if (sessionBuffer != NULL) {
            sessionBuffer += *sessionOffset;
            SlidingHisto_Clear(block);
            Voicing_Reset(block);
            // A session stores histogram validity first, followed by its rolling history.
            block->histogramValid = 0;
            block->histogramValid = *(u32 *)sessionBuffer;
            SlidingHisto_PutSession(
                block, (SlidingHistoSessionData *)(sessionBuffer + 4));
            if ((s32)block->histogramValid != 0) {
                block->histogramLowerQuantile =
                    SlidingHisto_LowerQuantile(block, block->histogramQuantile);
            }
            sessionSize = SlidingHisto_sizeof_SessionData(block) + 4;
            *sessionOffset += sessionSize;
        } else {
            SlidingHisto_Clear(block);
            Voicing_Reset(block);
            block->histogramValid = 0;
        }
        break;

    case 106:
        sessionOffset = (u32 *)argumentSize;
        sessionBuffer = (u8 *)argument;
        if (sessionBuffer != NULL) {
            sessionBuffer += *sessionOffset;
            *(u32 *)sessionBuffer = block->histogramValid;
            SlidingHisto_GetSession(
                block, (SlidingHistoSessionData *)(sessionBuffer + 4));
        }
        sessionSize = SlidingHisto_sizeof_SessionData(block) + 4;
            *sessionOffset += sessionSize;
        break;

    case 255:
        {
            TosContext *context = block->base.context;
            if (block->voicingDecisionHistory != NULL) {
                heap_Free(context->heap, block->voicingDecisionHistory);
            }
        }
        SlidingHisto_Free(block);
        Voicing_Free(block);
        tosBaseBlockDestruct(block);
        break;

    default:
        return 0;
    }

    return 1;
}

// Loads the speech profile and allocates histogram and voicing history for a new block.
// tosBaseBlockConstruct invokes this initialization callback.
static u32 InitTriggerLR(TosBaseBlock *baseBlock)
{
    TriggerLR *block = (TriggerLR *)baseBlock;
    u32 initializationFailed = 0;
    TosContext *context = block->base.context;
    f32 sensitivityScale;
    f32 noiseOffset;
    f32 voicingThreshold;
    f32 histogramOffset;

    block->base.input[2].inputSize = sizeof(f32);
    block->base.input[3].inputSize = sizeof(f32);
    block->base.input[1].inputSize = 24;

    block->minimumSpeechFrames = _tosGetProfileU32(block, 1, 6);
    block->minimumSpeechMilliseconds = block->minimumSpeechFrames * 10;
    block->consecutiveEnergeticLimit = _tosGetProfileU32(block, 2, 3);
    block->silenceFrameLimit = _tosGetProfileU32(block, 3, 15);
    block->triggerLookbackLimit = _tosGetProfileU32(block, 4, 45);
    block->energyFloorDbHundredths =
        (s32)_tosGetProfileU32(block, 5, -7200);
    block->triggerEventMode = _tosGetProfileU32(block, 31, 0);
    block->energyFloorLog =
        27.37f +
        (f32)block->energyFloorDbHundredths / 4.3429446f / 100.0f;
    block->activityThresholdLog =
        logf_check(_tosGetProfileFloat(block, 8, 1.0f));
    block->onsetThresholdLog =
        logf_check(_tosGetProfileFloat(block, 9, 5.0f));
    block->peakActivityThresholdLog =
        logf_check(_tosGetProfileFloat(block, 10, 100.0f));
    noiseOffset = (f32)abs((s32)_tosGetProfileU32(block, 6, 17));
    noiseOffset *= 0.23025851f;
    block->noiseFloorOffsetLog = noiseOffset;
    block->sensitivityMinimum = _tosGetProfileU32(block, 18, 0);
    block->sensitivityMaximum = _tosGetProfileU32(block, 19, 100);
    block->histogramOffsetMinimum =
        (s32)_tosGetProfileU32(block, 20, 5);
    block->histogramOffsetMaximum =
        (s32)_tosGetProfileU32(block, 21, 25);
    block->voicingThresholdMinimum =
        _tosGetProfileFloat(block, 22, 0.75f);
    block->voicingThresholdMaximum =
        _tosGetProfileFloat(block, 23, 0.65f);
    block->sensitivity = _tosGetProfileU32(block, 7, 50);

    sensitivityScale =
        (f32)(block->sensitivity - block->sensitivityMinimum) /
        (f32)(block->sensitivityMaximum - block->sensitivityMinimum);
    histogramOffset = (f32)block->histogramOffsetMinimum;
    histogramOffset += sensitivityScale *
        (f32)(block->histogramOffsetMaximum - block->histogramOffsetMinimum);
    histogramOffset *= 0.23025851f;
    voicingThreshold = block->voicingThresholdMinimum;
    voicingThreshold += sensitivityScale *
        (block->voicingThresholdMaximum - block->voicingThresholdMinimum);
    block->histogramThresholdOffsetLog = histogramOffset;
    block->voicingDecisionThreshold = logf_check(voicingThreshold);
    block->histogramQuantile =
        _tosGetProfileFloat(block, 15, 0.5f);
    block->histogramUpdateFrames = _tosGetProfileU32(block, 16, 100);
    block->histogramValid = 0;

    if ((s32)SlidingHisto_Init(block) != 0) {
        initializationFailed = 1;
    } else {
        block->voicingDecisionHistory =
            heap_Calloc(context->heap, 3, sizeof(f32));
        if (block->voicingDecisionHistory == NULL) {
            _tosErrorLog(block, 2);
            initializationFailed = 1;
        } else {
            block->voicingDecisionHistoryWrite =
                block->voicingDecisionHistory;
            block->voicingDecisionHistoryEnd =
                block->voicingDecisionHistory + 3;

            if (InitVoicing(block)) {
                initializationFailed = 1;
            } else {
                block->base.input[0].inputSize =
                    (u16)(block->subsamplerInputCount * sizeof(s16));
                block->resetStartsInputQueues = 1;
                block->holdInputQueuesAfterTrigger = 1;
                block->endSilenceFrames =
                    _tosGetProfileU32(block, 17, 50);
                block->endSilenceMilliseconds =
                    block->endSilenceFrames * 10;
            }
        }
    }
    return initializationFailed;
}

// Creates a four-input TriggerLR block for the configured speech-recognition pipeline.
// The ASR profile's block table calls this constructor when building that pipeline.
TosBaseBlock *ConstructTriggerLR(TosContext *context, u32 blockIndex)
{
    return tosBaseBlockConstruct(
        context, blockIndex, 4, 0, ProcessTriggerLR, InitTriggerLR,
        ControlTriggerLR, sizeof(TriggerLR));
}
