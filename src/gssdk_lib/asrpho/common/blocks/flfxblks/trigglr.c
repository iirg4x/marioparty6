// Detects speech onset and end from audio activity, energy, and voicing features.
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/heap.h"
#include <stdlib.h>

#include "gssdk/mqueue.h"
#include "gssdk/engine_params.h"
#include "gssdk/triggerlr.h"

/* The sliding histogram payload follows this session header. */
struct _TRIGGERLRSESSION {
    int bPeakIsValid; /* Whether the saved histogram can update the peak. */
};

// Returns the middle of three voicing scores while TriggerLR checks a speech candidate.
static f32 TriggerLR_CalcMedian(TriggerLR *This)
{
    f32 LowestVal;
    f32 HighestVal;
    f32 RetVal;

    if (This->pLogAutoCorrBufferBase[0] < This->pLogAutoCorrBufferBase[1]) {
        LowestVal = This->pLogAutoCorrBufferBase[0];
        HighestVal = This->pLogAutoCorrBufferBase[1];
    } else {
        LowestVal = This->pLogAutoCorrBufferBase[1];
        HighestVal = This->pLogAutoCorrBufferBase[0];
    }

    if (This->pLogAutoCorrBufferBase[2] > HighestVal) {
        RetVal = HighestVal;
    } else if (This->pLogAutoCorrBufferBase[2] > LowestVal) {
        RetVal = This->pLogAutoCorrBufferBase[2];
    } else {
        RetVal = LowestVal;
    }
    return RetVal;
}

// Updates speech-detection state from the current frame's onset, energy, and pitch features.
// ProcessTriggerLR calls this once for each available input frame.
static int TriggerLR_FindSpeech(TriggerLR *This, void *pInputs)
{
    s32 bRetVal = 0;
    f32 finalThr;
    f32 smoothLogE;
    f32 peakThr;
    f32 log_lr;
    f32 logE;
    f32 noiseThr;
    f32 *pLevels = ((f32 **)pInputs)[1];

    /* VAD sends the speech-silence threshold in its fourth level slot. */
    noiseThr = pLevels[3] + This->NoiseSensitivity;
    log_lr = *((f32 **)pInputs)[2];
    logE = pLevels[0];
    smoothLogE = *((f32 **)pInputs)[3];
    finalThr = noiseThr > This->AbsThr ? noiseThr : This->AbsThr;
    if (This->bPeakIsValid == 0) {
        This->currentPeak = finalThr + This->PeakSensitivity;
    }

    peakThr = This->currentPeak - This->PeakSensitivity;
    peakThr = peakThr > finalThr ? peakThr : finalThr;

    // Tracks gaps on either side of the point where speech is first detected.
    switch (This->BufferState) {
    case BUFFER_STATE_LEADINGSILENCE:
        if (This->JumpBack > This->MaxSilGap) {
            This->JumpBack = This->MaxSilGap;
        }

        if (log_lr > This->HighThresLogLR) {
            This->BufferState = BUFFER_STATE_MAYBESPEECH;
            This->bPeakUpdated = 0;
            This->bSpeechCondWithoutE = 0;
            This->bPeakCondition = 0;
            This->bIssueGarbledSound = 0;
            This->SpeechDuration = 1;
            This->SilenceDuration = 0;
            This->PeakDuration = logE >= peakThr ? 1 : 0;
            This->maxE4PeakTracker = smoothLogE;
            This->maxLogLR = log_lr;

            if (This->TriggerState == TRIGGERLR_START_DETECTION) {
                /* Unfilled voicing slots start below measured scores. */
                This->pLogAutoCorrBufferBase[0] = -2147483600.0f;
                This->pLogAutoCorrBufferBase[1] = -2147483600.0f;
                This->pLogAutoCorrBufferBase[2] = -2147483600.0f;
                This->pLogAutoCorrBufferNewest = This->pLogAutoCorrBufferBase;

                if (logE >= peakThr) {
                    *This->pLogAutoCorrBufferNewest = Voicing_GetMaxVoicing(This);
                }
                This->bVoicedState = 0;
            }
        } else {
            if (This->bSpeechSeen != 0 && This->bDoStopDetection != 0) {
                This->SilenceDuration++;
            }
            if (This->TriggerState == TRIGGERLR_START_DETECTION) {
                Voicing_MaintainNoiseEner(This);
            }
        }
        break;

    case BUFFER_STATE_MAYBESPEECH:
        if (This->bVoicedState == 0) {
            if (++This->pLogAutoCorrBufferNewest >= This->pLogAutoCorrBufferEnd) {
                This->pLogAutoCorrBufferNewest = This->pLogAutoCorrBufferBase;
            }
            // An unavailable voicing measurement must not satisfy the median threshold.
            *This->pLogAutoCorrBufferNewest = -2147483600.0f;
        }

        if (log_lr > This->LowThresLogLR) {
            This->maxE4PeakTracker =
                smoothLogE > This->maxE4PeakTracker ? smoothLogE : This->maxE4PeakTracker;
            This->maxLogLR = log_lr > This->maxLogLR ? log_lr : This->maxLogLR;
            This->SpeechDuration++;

            if (logE >= peakThr) {
                This->PeakDuration++;
                This->bIssueGarbledSound = 1;
            } else {
                This->PeakDuration = 0;
            }

            This->bPeakCondition =
                This->bPeakCondition != 0 || This->PeakDuration >= This->MinPeakDuration;

            if (logE >= peakThr && This->bVoicedState == 0) {
                *This->pLogAutoCorrBufferNewest = Voicing_GetMaxVoicing(This);
                if (TriggerLR_CalcMedian(This) > This->logVoicingThreshold) {
                    This->bVoicedState = 1;
                }
            }

            This->bSpeechCondWithoutE = This->bSpeechCondWithoutE != 0 ||
                                        (This->SpeechDuration >= This->MinSpeechDur &&
                                         This->maxLogLR > This->MinimalLogLR &&
                                         This->maxE4PeakTracker >= finalThr);

            if (This->SpeechDuration >= This->maxSpeechDuration &&
                This->bSpeechCondWithoutE != 0) {
                SlidingHisto_NewItem(This, This->maxE4PeakTracker);
                This->bPeakUpdated = 1;
                This->bPeakIsValid = 1;
                This->currentPeak = SlidingHisto_LowerQuantile(This, This->Quantile);
                This->SpeechDuration = This->MinSpeechDur;
                This->maxE4PeakTracker = smoothLogE;
            }

            if (This->bSpeechCondWithoutE != 0 && This->bPeakCondition != 0 &&
                This->bVoicedState != 0) {
                This->BufferState = BUFFER_STATE_SPEECH;
                This->bSpeechSeen = 1;
            }
        } else {
            This->BufferState = BUFFER_STATE_SILENCEAFTERMAYBESPEECH;
            This->SilenceDuration = 1;
        }
        break;

    case BUFFER_STATE_SILENCEAFTERMAYBESPEECH:
        if (This->bVoicedState == 0) {
            if (++This->pLogAutoCorrBufferNewest >= This->pLogAutoCorrBufferEnd) {
                This->pLogAutoCorrBufferNewest = This->pLogAutoCorrBufferBase;
            }
            // Silence also advances the three-frame history with an unavailable score.
            *This->pLogAutoCorrBufferNewest = -2147483600.0f;
        }

        if (log_lr > This->HighThresLogLR) {
            This->BufferState = BUFFER_STATE_MAYBESPEECH;
            This->SpeechDuration = 1;
            This->maxE4PeakTracker =
                smoothLogE > This->maxE4PeakTracker ? smoothLogE : This->maxE4PeakTracker;
            This->maxLogLR = log_lr;
            This->bSpeechCondWithoutE = 0;
            This->bPeakCondition = 0;
            This->PeakDuration = logE >= peakThr ? 1 : 0;

            if (logE >= peakThr && This->bVoicedState == 0) {
                *This->pLogAutoCorrBufferNewest = Voicing_GetMaxVoicing(This);
                if (TriggerLR_CalcMedian(This) > This->logVoicingThreshold) {
                    This->bVoicedState = 1;
                }
            }
        } else {
            if (++This->SilenceDuration > This->MaxSilGap) {
                This->BufferState = BUFFER_STATE_LEADINGSILENCE;
                /* Reports the garbled-sound condition after the silence gap. */
                /* The abnormal-condition callback maps the garbled-speech code to event 4. */
                if (This->bIssueGarbledSound != 0 &&
                    _tosControl(&This->BaseBlock.Object, 0xCA, 0x46, This->FrameCounter)) {
                    bRetVal = 1;
                    break;
                }
                if (This->bPeakUpdated == 0 && This->bSpeechCondWithoutE != 0) {
                    SlidingHisto_NewItem(This, This->maxE4PeakTracker);
                    This->bPeakUpdated = 1;
                    This->bPeakIsValid = 1;
                    This->currentPeak = SlidingHisto_LowerQuantile(This, This->Quantile);
                }
            }
        }
        break;

    case BUFFER_STATE_SPEECH:
        if (log_lr > This->LowThresLogLR) {
            This->maxE4PeakTracker =
                smoothLogE > This->maxE4PeakTracker ? smoothLogE : This->maxE4PeakTracker;
            if (++This->SpeechDuration >= This->maxSpeechDuration) {
                SlidingHisto_NewItem(This, This->maxE4PeakTracker);
                This->bPeakUpdated = 1;
                This->bPeakIsValid = 1;
                This->currentPeak = SlidingHisto_LowerQuantile(This, This->Quantile);
                This->SpeechDuration = 0;
                This->maxE4PeakTracker = smoothLogE;
            }
        } else {
            This->BufferState = BUFFER_STATE_SILENCEAFTERSPEECH;
            This->SilenceDuration = 1;
        }
        break;

    case BUFFER_STATE_SILENCEAFTERSPEECH:
        if (log_lr > This->HighThresLogLR) {
            This->BufferState = BUFFER_STATE_SPEECH;
            This->SpeechDuration++;
            This->maxE4PeakTracker =
                smoothLogE > This->maxE4PeakTracker ? smoothLogE : This->maxE4PeakTracker;
        } else {
            if (++This->SilenceDuration > This->MaxSilGap) {
                This->BufferState = BUFFER_STATE_LEADINGSILENCE;
                if (This->bPeakUpdated == 0) {
                    SlidingHisto_NewItem(This, This->maxE4PeakTracker);
                    This->bPeakUpdated = 1;
                    This->bPeakIsValid = 1;
                    This->currentPeak = SlidingHisto_LowerQuantile(This, This->Quantile);
                }
            }
        }
        break;
    }

    return bRetVal;
}

// Processes each scheduled audio frame, raising trigger events and updating input queues.
// TinyOS calls this through the process callback installed by ConstructTriggerLR.
static void ProcessTriggerLR(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    TriggerLR *This = (TriggerLR *) _This;
    s16 *pSamples;

    if (((void **)pInputs)[0] == NULL) {
        return;
    }

    This->FrameCounter++;
    if (This->TriggerState == TRIGGERLR_START_DETECTION) {
        pSamples = ((void **)pInputs)[0];
        if (Voicing_AddSignal(This, pSamples)) {
            return;
        }
        if (++This->JumpBack > This->Qdepth) {
            This->JumpBack = This->Qdepth;
        }
    }

    if (TriggerLR_FindSpeech(This, pInputs)) {
        return;
    }

    // Reports a stop once the configured silence duration has elapsed.
    if (This->TriggerState == TRIGGERLR_START_DETECTION) {
        if (This->BufferState != BUFFER_STATE_SPEECH) {
            return;
        }

        This->BeginTime = This->FrameCounter - This->JumpBack;
        // Queue command 8 disables this reader; a failure leaves the trigger unreported.
        if (This->bDoStopDetection == 0 &&
            !qQueueControl(This->BaseBlock.InputQueues[0].pQ, 8,
                           This->BaseBlock.InputQueues[0].ReadPos, 0)) {
            return;
        }

        if (This->bWaveMode != 0) {
            if (_tosControl(&_This->Object, 0xCB, This->JumpBack, This->FrameCounter)) {
                goto done;
            }
        } else {
            if (_tosControl(&_This->Object, 0xC8, This->JumpBack, This->FrameCounter)) {
                goto done;
            }
        }

        if (This->bDoStopDetection != 0) {
            This->TriggerState = TRIGGERLR_TS_DETECTION;
        } else {
            tosBaseBlockDisableInputQueues(_This);
        }
    } else if (This->TriggerState == TRIGGERLR_TS_DETECTION && This->bSpeechSeen != 0 &&
               This->SilenceDuration > This->TSCount) {
        This->bSpeechSeen = 0;
        if (_tosControl(&_This->Object, 0xC9, This->FrameCounter, 0)) {
            return;
        }
        This->TriggerState = TRIGGERLR_IDLE;
    }
done:
    return;
}

// Resets utterance detection and the voicing history.
static void TriggerLR_Reset(TriggerLR *This)
{
    This->FrameCounter = 0;
    This->BeginTime = 0;
    This->JumpBack = 0;
    This->SpeechDuration = 0;
    This->SilenceDuration = 0;
    This->bSpeechSeen = 0;
    This->bVoicedState = 0;
    This->BufferState = BUFFER_STATE_LEADINGSILENCE;
    Voicing_Reset(This);
}

// Clears the histogram, voicing history, and peak-validity flag.
static void TriggerLR_SessionReset(TriggerLR *This)
{
    SlidingHisto_Clear(This);
    Voicing_Reset(This);
    This->bPeakIsValid = 0;
}

// Saves the peak-validity flag and histogram.
static void TriggerGetSession(TriggerLR *This, struct _TRIGGERLRSESSION *pTriggerSession)
{
    pTriggerSession->bPeakIsValid = This->bPeakIsValid;
    SlidingHisto_GetSession(This, (struct _SLIDINGHISTOSESSION *)(pTriggerSession + 1));
}

// Restores the histogram and recomputes the peak when valid.
static void TriggerPutSession(TriggerLR *This, struct _TRIGGERLRSESSION *pTriggerSession)
{
    This->bPeakIsValid = pTriggerSession->bPeakIsValid;
    SlidingHisto_PutSession(This, (struct _SLIDINGHISTOSESSION *)(pTriggerSession + 1));
    if (This->bPeakIsValid != 0) {
        This->currentPeak = SlidingHisto_LowerQuantile(This, This->Quantile);
    }
}

// Returns the size of the peak-validity flag and saved histogram.
static inline unsigned long TriggerLR_sizeof_SessionData(TriggerLR *This)
{
    return SlidingHisto_sizeof_SessionData(This) + sizeof(This->bPeakIsValid);
}

// Applies reset, configuration, session, and destruction commands to the speech trigger.
// TinyOS invokes this callback when the block receives a control command.
static unsigned char ControlTriggerLR(struct _BLOCK *_This, unsigned char ControlId,
                                      unsigned long CParam, unsigned long DParam)
{
    TriggerLR *This = (TriggerLR *) _This;
    struct _TRIGGERLRSESSION *sessionBuffer;
    u32 RetVal;
    f32 OriPeakSensitivity;
    f32 OriVoicingThreshold;
    f32 Factor;

    switch (ControlId) {
    case 0x64:
        TriggerLR_Reset(This);

        if (This->bDoStartDetection != 0) {
            This->TriggerState = TRIGGERLR_START_DETECTION;
            if (tosBaseBlockStartInputQueues(_This, 0)) {
                return 0;
            }
        } else if (This->bDoStopDetection != 0) {
            This->TriggerState = TRIGGERLR_TS_DETECTION;
            RetVal =
                !qQueueControl(_This->InputQueues[1].pQ, 5, _This->InputQueues[1].ReadPos, 0);
            RetVal |=
                !qQueueControl(_This->InputQueues[2].pQ, 5, _This->InputQueues[2].ReadPos, 0);
            RetVal |=
                !qQueueControl(_This->InputQueues[3].pQ, 5, _This->InputQueues[3].ReadPos, 0);
            if (RetVal) {
                return 0;
            }
        } else {
            This->TriggerState = TRIGGERLR_IDLE;
            if (tosBaseBlockDisableInputQueues(_This)) {
                return 0;
            }
        }
        break;

    case 0x67:
        This->bDoStartDetection = ((TriggerControlParameters *)CParam)->DoStartDetection;
        This->bDoStopDetection = ((TriggerControlParameters *)CParam)->DoStopDetection;
        This->ExternalSensitivity = ((TriggerControlParameters *)CParam)->Sensitivity;
        OriPeakSensitivity = (f32) (This->ExternalSensitivity - This->ExtSens_Min) /
                           (f32) (This->ExtSens_Max - This->ExtSens_Min);
        Factor = (f32) This->PeakSens_Min;
        Factor += OriPeakSensitivity * (f32) (This->PeakSens_Max - This->PeakSens_Min);
        Factor *= 0.23025851f;
        OriVoicingThreshold = This->VoicThr_Beg;
        OriVoicingThreshold += OriPeakSensitivity * (This->VoicThr_End - This->VoicThr_Beg);
        This->PeakSensitivity = Factor;
        This->logVoicingThreshold = logf_check(OriVoicingThreshold);
        This->OriTSCount = ((TriggerControlParameters *)CParam)->TSCount;
        This->OriMinSpeechDur = ((TriggerControlParameters *)CParam)->MinSpeechDur;
        This->TSCount = This->OriTSCount / 10;
        This->MinSpeechDur = This->OriMinSpeechDur / 10;
        This->OriAbsThr = ((TriggerControlParameters *)CParam)->AbsTriggThreshold;
        This->AbsThr = 27.37f + (f32) This->OriAbsThr / 4.3429446f / 100.0f;
        break;

    case 0x66:
        ((TriggerControlParameters *)CParam)->DoStartDetection = This->bDoStartDetection;
        ((TriggerControlParameters *)CParam)->DoStopDetection = This->bDoStopDetection;
        ((TriggerControlParameters *)CParam)->TSCount = This->OriTSCount;
        ((TriggerControlParameters *)CParam)->Sensitivity = This->ExternalSensitivity;
        ((TriggerControlParameters *)CParam)->MinSpeechDur = This->OriMinSpeechDur;
        ((TriggerControlParameters *)CParam)->AbsTriggThreshold = This->OriAbsThr;
        break;

    case 0x65:
        *(u32 **) CParam = &This->BeginTime;
        break;

    case 0x68:
        This->bWaveMode = CParam;
        break;

    case 0x69:
        sessionBuffer = (struct _TRIGGERLRSESSION *) CParam;
        if (sessionBuffer != NULL) {
            sessionBuffer = (struct _TRIGGERLRSESSION *) ((u8 *) sessionBuffer + *(u32 *)DParam);
            TriggerLR_SessionReset(This);
            TriggerPutSession(This, sessionBuffer);
            *(u32 *)DParam += TriggerLR_sizeof_SessionData(This);
        } else {
            TriggerLR_SessionReset(This);
        }
        break;

    case 0x6A:
        sessionBuffer = (struct _TRIGGERLRSESSION *) CParam;
        if (sessionBuffer != NULL) {
            sessionBuffer = (struct _TRIGGERLRSESSION *) ((u8 *) sessionBuffer + *(u32 *)DParam);
            TriggerGetSession(This, sessionBuffer);
        }
        *(u32 *)DParam += TriggerLR_sizeof_SessionData(This);
        break;

    case 0xFF: {
        struct _TOS *pTos = This->BaseBlock.Object.pTos;
        if (This->pLogAutoCorrBufferBase != NULL) {
            heap_Free(pTos->hHeap, This->pLogAutoCorrBufferBase);
        }
        SlidingHisto_Free(This);
        Voicing_Free(This);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    }

    default:
        return 0;
    }

    return 1;
}

// Loads the speech profile and allocates histogram and voicing history for a new block.
static u32 InitTriggerLR(struct _BLOCK *_This)
{
    TriggerLR *This = (TriggerLR *) _This;
    u32 RetVal = 0;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    f32 OriPeakSensitivity;
    f32 OriNoiseSensitivity;
    f32 OriVoicingThreshold;
    f32 Factor;

    This->BaseBlock.InputQueues[2].Size = sizeof(f32);
    This->BaseBlock.InputQueues[3].Size = sizeof(f32);
    This->BaseBlock.InputQueues[1].Size = 6 * sizeof(f32); /* VAD log energy and tracked levels. */

    This->MinSpeechDur = _tosGetProfileU32(&This->BaseBlock.Object, 0x01, 6);
    This->OriMinSpeechDur = This->MinSpeechDur * 10;
    This->MinPeakDuration = _tosGetProfileU32(&This->BaseBlock.Object, 0x02, 3);
    This->MaxSilGap = _tosGetProfileU32(&This->BaseBlock.Object, 0x03, 15);
    This->Qdepth = _tosGetProfileU32(&This->BaseBlock.Object, 0x04, 45);
    This->OriAbsThr = _tosGetProfileU32(&This->BaseBlock.Object, 0x05, -7200);
    This->bWaveMode = _tosGetProfileU32(&This->BaseBlock.Object, 0x1F, 0);
    This->AbsThr = 27.37f + (f32) This->OriAbsThr / 4.3429446f / 100.0f;
    This->LowThresLogLR = logf_check(_tosGetProfileFloat(&This->BaseBlock.Object, 0x08, 1.0f));
    This->HighThresLogLR = logf_check(_tosGetProfileFloat(&This->BaseBlock.Object, 0x09, 5.0f));
    This->MinimalLogLR = logf_check(_tosGetProfileFloat(&This->BaseBlock.Object, 0x0A, 100.0f));
    OriNoiseSensitivity = (f32) abs((s32) _tosGetProfileU32(&This->BaseBlock.Object, 0x06, 17));
    OriNoiseSensitivity *= 0.23025851f;
    This->NoiseSensitivity = OriNoiseSensitivity;
    This->ExtSens_Min = _tosGetProfileU32(&This->BaseBlock.Object, 0x12, 0);
    This->ExtSens_Max = _tosGetProfileU32(&This->BaseBlock.Object, 0x13, 100);
    This->PeakSens_Min = (s32) _tosGetProfileU32(&This->BaseBlock.Object, 0x14, 5);
    This->PeakSens_Max = (s32) _tosGetProfileU32(&This->BaseBlock.Object, 0x15, 25);
    This->VoicThr_Beg = _tosGetProfileFloat(&This->BaseBlock.Object, 0x16, 0.75f);
    This->VoicThr_End = _tosGetProfileFloat(&This->BaseBlock.Object, 0x17, 0.65f);
    This->ExternalSensitivity = _tosGetProfileU32(&This->BaseBlock.Object, 0x07, 50);

    OriPeakSensitivity = (f32) (This->ExternalSensitivity - This->ExtSens_Min) /
                       (f32) (This->ExtSens_Max - This->ExtSens_Min);
    Factor = (f32) This->PeakSens_Min;
    Factor += OriPeakSensitivity * (f32) (This->PeakSens_Max - This->PeakSens_Min);
    Factor *= 0.23025851f;
    OriVoicingThreshold = This->VoicThr_Beg;
    OriVoicingThreshold += OriPeakSensitivity * (This->VoicThr_End - This->VoicThr_Beg);
    This->PeakSensitivity = Factor;
    This->logVoicingThreshold = logf_check(OriVoicingThreshold);
    This->Quantile = _tosGetProfileFloat(&This->BaseBlock.Object, 0x0F, 0.5f);
    This->maxSpeechDuration = _tosGetProfileU32(&This->BaseBlock.Object, 0x10, 100);
    This->bPeakIsValid = 0;

    if (SlidingHisto_Init(This) != 0) {
        RetVal = 1;
    } else {
        This->pLogAutoCorrBufferBase = heap_Calloc(pTos->hHeap, 3, sizeof(f32));
        if (This->pLogAutoCorrBufferBase == NULL) {
            _tosErrorLog(&This->BaseBlock.Object, 2);
            RetVal = 1;
        } else {
            This->pLogAutoCorrBufferNewest = This->pLogAutoCorrBufferBase;
            This->pLogAutoCorrBufferEnd = This->pLogAutoCorrBufferBase + 3;

            if (InitVoicing(This)) {
                RetVal = 1;
            } else {
                This->BaseBlock.InputQueues[0].Size = (u16) (This->NrInSamples * sizeof(s16));
                This->bDoStartDetection = 1;
                This->bDoStopDetection = 1;
                This->TSCount = _tosGetProfileU32(&This->BaseBlock.Object, 0x11, 50);
                This->OriTSCount = This->TSCount * 10;
            }
        }
    }
    return RetVal;
}

// Creates a four-input TriggerLR block for the configured speech-recognition pipeline.
// The ASR profile's block table calls this constructor when building that pipeline.
struct _BLOCK *ConstructTriggerLR(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 4, 0,
                                 ProcessTriggerLR,
                                 /* The dispatcher reads only the low byte of the returned word. */
                                 (u8 (*)(struct _BLOCK *)) InitTriggerLR, ControlTriggerLR,
                                 sizeof(TriggerLR));
}
