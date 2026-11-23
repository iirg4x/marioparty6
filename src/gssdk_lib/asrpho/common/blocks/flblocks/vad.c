/* Voice activity detection, gain control and session state. */
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/float.h>
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/casr.h"
#include "gssdk/blocks.h"
#include "gssdk/engine_params.h"
#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Detector history saved with a recognition session. */
struct _VADSESSION {
    /* Set after initial estimates are available. */
    unsigned char Initialised;
    /* Smoothed peak log energy, relative to the noise level when gain control is on. */
    float SignalPeakTrackerState;
    /* Log-energy threshold for counting consecutive speech frames. */
    float SpeechThr;
    /* Noise-dependent boundary between speech and silence. */
    float SpeechSilenceThr;
    /* Smoothed difference between the upper noise estimate and its floor. */
    float NoiseRangeTrackerState;
    /* Smoothed lower estimate from the energy histogram. */
    float NoiseFloorTrackerState;
    /* Peak candidate for the next signal-peak update. */
    float LocalSignalPeak;
    /* Consecutive frames above the speech threshold. */
    signed short ExceedDur;
    /* Remaining frames in the current peak-tracking interval. */
    signed short OneShotCountDown;
    /* Frames left before the detector accepts activity after reset. */
    signed short TimeLeftAfterReset;
    /* Frames collected since the last peak update within the interval. */
    unsigned short SpeechDwellTime;
    /* Minimum log energy allowed for SpeechThr. */
    float AbsoluteThr;
    /* Past log-energy samples used to decide when to update the noise range. */
    float pLogEDelayLineStart[20];
    /* Next position in the saved log-energy delay line. */
    unsigned short LogEDelayLineIndex;
    /* Number of recent frames in each log-energy bin. */
    signed short Hist[150];
    /* Number of frames represented in the histogram. */
    unsigned long NrInHisto;
    /* Recent bin indices in insertion order. */
    signed short CirBuffStart[40];
    /* Next position in the saved bin-index ring. */
    unsigned short CirIndex;
};

/* Selects the follow-up path after a microphone gain request. */
enum _FOLLOWUPMIKE {
    NOFOLLOWUP = 0x0,
    FOLLOWUP = 0x1,
};

/* Detector state, including gain control, energy history and queues. */
typedef struct {
    /* Scheduler-owned block header. */
    struct _BLOCK BaseBlock;
    /* Set after initial estimates are available. */
    unsigned char Initialised;
    /* Smoothed peak log energy, relative to the noise level when gain control is on. */
    float SignalPeakTrackerState;
    /* Log-energy threshold for counting consecutive speech frames. */
    float SpeechThr;
    /* Noise-dependent boundary between speech and silence. */
    float SpeechSilenceThr;
    /* Smoothed difference between the upper noise estimate and its floor. */
    float NoiseRangeTrackerState;
    /* Smoothed lower estimate from the energy histogram. */
    float NoiseFloorTrackerState;
    /* Peak candidate for the next signal-peak update. */
    float LocalSignalPeak;
    /* Log energy corresponding to the lowest histogram bin. */
    float BinMin;
    /* Consecutive frames above the speech threshold. */
    signed short ExceedDur;
    /* Remaining frames in the current peak-tracking interval. */
    signed short OneShotCountDown;
    /* Frames left before the detector accepts activity after reset. */
    signed short TimeLeftAfterReset;
    /* Frames collected since the last peak update within the interval. */
    unsigned short SpeechDwellTime;
    /* Minimum log energy allowed for SpeechThr. */
    float AbsoluteThr;
    /* Past log-energy samples used to decide when to update the noise range. */
    float pLogEDelayLineStart[20];
    /* One past the last log-energy sample slot. */
    float *pLogEDelayLineEnd;
    /* Next log-energy sample slot in the circular delay line. */
    float *pLogEDelayLine;
    /* Number of recent frames in each log-energy bin. */
    signed short Hist[150];
    /* Energy levels corresponding to the histogram bins. */
    float HistoBins[150];
    /* Number of frames represented in the histogram. */
    unsigned long NrInHisto;
    /* Recent bin indices in insertion order. */
    signed short CirBuffStart[40];
    /* One past the last slot in the bin-index ring. */
    signed short *CirBuffEnd;
    signed short *Cir; /* Next sample position in the circular histogram buffer. */
    /* Enables output of the six signal and noise levels. */
    unsigned char OutputLevels;
    /* Minimum run of above-threshold frames before peak tracking starts. */
    signed short MinSpeechExceedDuration;
    /* Initial countdown for the peak-tracking interval. */
    signed short BlindTime;
    /* Peak-tracking dwell interval in frames. */
    signed short SampleTime;
    /* Peak log energy minus the speech-silence boundary. */
    float SNR;
    /* Log-energy adjustment applied to the absolute threshold. */
    float Sensitivity;
    /* Range below the estimated peak used to classify frame energy. */
    float DetectDynamicRange;
    /* Initial offset used to seed the signal peak. */
    float InitSignalRange;
    /* Voice-activity score clamped to the range from zero to one. */
    float FrameType;
    /* Reference log energy for threshold and gain settings. */
    float LNDB0;
    /* Enables microphone gain control. */
    unsigned char AgcOn;
    /* Log-energy target for microphone gain changes. */
    float LogETarget;
    /* Peak updates counted for the microphone gain check. */
    unsigned short NrUpdates;
    /* Peak update count at the preceding gain check. */
    unsigned short PrevNrUpdates;
    /* Microphone gain most recently reported to the block. */
    unsigned short Mike;
    /* Microphone gain before the current adjustment. */
    unsigned short OldMike;
    /* A gain request is awaiting completion. */
    unsigned char AskGainInProcess;
    /* Frames left before a pending gain request expires. */
    unsigned short AskGainWaitCount;
    /* Settled gain used as the center of the permitted gain range. */
    unsigned short MikeCoarse;
    /* Maximum gain offset from the settled gain. */
    unsigned short MikeRange;
    /* Scale factor for changes in microphone gain. */
    float AGCSpeed;
    /* Selects the wider allowed gain range. */
    unsigned char FarTalk;
    /* Records the gain initialization attempted after the timeout. */
    unsigned char GainInitialised;
    /* Selects a new gain adjustment or completion of a previous request. */
    enum _FOLLOWUPMIKE FollowUpMike;
    /* Number of input frames since the last reset or utterance notification. */
    unsigned long FrameCounter;
    /* Utterance notifications received since the counter was cleared. */
    unsigned short NbrOfUtterances;
    /* Utterance count recorded by the last abnormal-condition check. */
    unsigned short PrevNbrOfUtterances;
    /* Enables abnormal level, activity and timeout checks. */
    unsigned char CheckAbnormal;
    /* Frame limit for reporting a timeout before the first utterance. */
    unsigned long TimeOut;
    /* Selects the alternate histogram floor and frame-score formula. */
    unsigned char Mode_1500;
    /* Stores the mean log energy for a frame with multiple inputs. */
    float EnergyIn;
} VadBlockFields;

/* Reads the current microphone level or requests a different gain. */
static u16 MikeCtrlExternal(VadBlockFields *This, u16 NewMike)
{
    if (This->AskGainInProcess) {
        return 0;
    }
    if (NewMike && This->Mike != NewMike) {
        This->AskGainInProcess = 1;
        This->AskGainWaitCount = 50;
        This->Mike = 0;
        _tosControl(&This->BaseBlock.Object, 0x0B, NewMike, 0);
        return 0;
    }
    return This->Mike;
}

/* Updates the microphone gain from the tracked signal and noise levels. */
static u8 ChangeMike(VadBlockFields *This)
{
    f32 Diff;
    long OldMike;
    long Mike;
    f32 SignalPeak;
    Diff = This->NoiseFloorTrackerState +
           (This->SignalPeakTrackerState + This->NoiseRangeTrackerState) - This->LogETarget;
    OldMike = MikeCtrlExternal(This, 0);
    if (!OldMike) {
        return 0;
    }
    This->OldMike = OldMike;
    This->FollowUpMike = NOFOLLOWUP;
    Mike = OldMike;
    if (fabs(Diff) > 0.2f) {
        Mike = OldMike * (1.0f - 0.05f * This->AGCSpeed * Diff);
        if (Mike > 0xFFFF)
            Mike = 0xFFFF;
        if (This->MikeCoarse) {
            Mike = ((Mike > This->MikeCoarse - This->MikeRange
                         ? Mike
                         : This->MikeCoarse - This->MikeRange) < This->MikeCoarse + This->MikeRange
                        ? (Mike > This->MikeCoarse - This->MikeRange
                               ? Mike
                               : This->MikeCoarse - This->MikeRange)
                        : This->MikeCoarse + This->MikeRange);
        }
        Mike = MikeCtrlExternal(This, Mike);
        if (!Mike) {
            This->FollowUpMike = FOLLOWUP;
            return 0;
        }
    }
    if (!This->MikeCoarse) {
        if (abs(Mike - OldMike) > (long) (0.01f * OldMike))
            This->NrUpdates = 0;
        if (This->NrUpdates > 10) {
            This->MikeCoarse = Mike;
            This->NrUpdates = 0;
            SignalPeak = (This->SignalPeakTrackerState + This->NoiseRangeTrackerState) +
                         This->NoiseFloorTrackerState;
            if ((This->MikeCoarse == 0xFFFF && SignalPeak < This->LogETarget - 2.763102114200592) ||
                (This->MikeCoarse < 0x199B && SignalPeak > 1.381551057100296 + This->LogETarget)) {
                if (_tosControl(&This->BaseBlock.Object, 0x0C, POORMIC, This->FrameCounter))
                    return 1;
            }
        }
    } else
        This->NrUpdates = 0;
    return 0;
}

/* Clears a transient microphone switch and updates its settled gain level. */
static void FollowUpMike(VadBlockFields *This)
{
    if (This->Mike != 0 && This->FollowUpMike == FOLLOWUP) {
        This->FollowUpMike = NOFOLLOWUP;
        if (This->MikeCoarse == 0) {
            if (abs(This->Mike - This->OldMike) > (u16) (0.01f * (f32) This->OldMike)) {
                This->NrUpdates = 0;
            }
            if (This->NrUpdates > 10) {
                This->MikeCoarse = This->Mike;
                This->NrUpdates = 0;
            }
        } else {
            This->NrUpdates = 0;
        }
    }
}

/* Reports sustained low signal-to-noise ratio, level excursions and an input timeout. */
static u8 CheckForAbnormalConditions(struct _BLOCK *This, f32 Energy)
{
    VadBlockFields *vad = (VadBlockFields *) This;
    f32 SignalPeak;
    u8 agcOn;

    agcOn = vad->AgcOn;
    if (agcOn != 0) {
        if (vad->MikeCoarse != 0) {
            goto checkSignalLevels;
        }
    }
    if (agcOn != 0) {
        goto checkTimeout;
    }
checkSignalLevels:
    if (vad->SNR < 5.7564627379179 &&
        (s32) vad->NbrOfUtterances - (s32) vad->PrevNbrOfUtterances > 3) {
        if (_tosControl(&This->Object, 0x0C, BADSNR, vad->FrameCounter) != 0) {
            return 1;
        }
        vad->PrevNbrOfUtterances = vad->NbrOfUtterances;
    }

    if (Energy - vad->LNDB0 > 0.6907755f && vad->PrevNbrOfUtterances != vad->NbrOfUtterances) {
        vad->PrevNbrOfUtterances = vad->NbrOfUtterances;
        if (_tosControl(&This->Object, 0x0C, OVERLOAD, vad->FrameCounter) != 0) {
            return 1;
        }
    } else {
        SignalPeak = vad->SignalPeakTrackerState;
        if (vad->AgcOn != 0) {
            SignalPeak += vad->NoiseRangeTrackerState + vad->NoiseFloorTrackerState;
        }
        SignalPeak -= vad->LNDB0;
        if (SignalPeak < -6.9077554f &&
            (s32) vad->NbrOfUtterances - (s32) vad->PrevNbrOfUtterances > 3) {
            if (_tosControl(&This->Object, 0x0C, TOOQUIET, vad->FrameCounter) != 0) {
                return 1;
            }
            vad->PrevNbrOfUtterances = vad->NbrOfUtterances;
        }
    }

checkTimeout:
    if (vad->AgcOn != 0) {
        if (vad->TimeOut != 0 && vad->FrameCounter > vad->TimeOut && vad->NbrOfUtterances == 0) {
            if (_tosControl(&This->Object, 0x0C, NOSIGNAL, vad->FrameCounter) != 0) {
                return 1;
            }
            if (vad->GainInitialised == 0 && vad->AgcOn != 0) {
                vad->GainInitialised = 1;
                if (MikeCtrlExternal(vad, 0xBFFF) != 0) {
                    return 1;
                }
            }
            vad->FrameCounter = 0;
        }
    }
    return 0;
}

/* Copies the VAD's rolling statistics and history into a session record. */
static void VadGetSession(VadBlockFields *This, struct _VADSESSION *pVadSession)
{
    int i;

    pVadSession->Initialised = This->Initialised;
    pVadSession->SignalPeakTrackerState = This->SignalPeakTrackerState;
    pVadSession->SpeechThr = This->SpeechThr;
    pVadSession->SpeechSilenceThr = This->SpeechSilenceThr;
    pVadSession->NoiseRangeTrackerState = This->NoiseRangeTrackerState;
    pVadSession->NoiseFloorTrackerState = This->NoiseFloorTrackerState;
    pVadSession->LocalSignalPeak = This->LocalSignalPeak;
    pVadSession->ExceedDur = This->ExceedDur;
    pVadSession->OneShotCountDown = This->OneShotCountDown;
    pVadSession->TimeLeftAfterReset = This->TimeLeftAfterReset;
    pVadSession->SpeechDwellTime = This->SpeechDwellTime;
    pVadSession->AbsoluteThr = This->AbsoluteThr;
    for (i = 0; i < 20; i++) {
        pVadSession->pLogEDelayLineStart[i] = This->pLogEDelayLineStart[i];
    }
    pVadSession->LogEDelayLineIndex = (u16) (This->pLogEDelayLine - This->pLogEDelayLineStart);
    for (i = 0; i < 150; i++) {
        pVadSession->Hist[i] = This->Hist[i];
    }
    pVadSession->NrInHisto = This->NrInHisto;
    for (i = 0; i < 40; i++) {
        pVadSession->CirBuffStart[i] = This->CirBuffStart[i];
    }
    pVadSession->CirIndex = (u16) (This->Cir - This->CirBuffStart);
}

/* Restores the VAD statistics and history from the session record. */
static void VadPutSession(VadBlockFields *This, struct _VADSESSION *pVadSession)
{
    int i;

    This->Initialised = pVadSession->Initialised;
    This->SignalPeakTrackerState = pVadSession->SignalPeakTrackerState;
    This->SpeechThr = pVadSession->SpeechThr;
    This->SpeechSilenceThr = pVadSession->SpeechSilenceThr;
    This->NoiseRangeTrackerState = pVadSession->NoiseRangeTrackerState;
    This->NoiseFloorTrackerState = pVadSession->NoiseFloorTrackerState;
    This->LocalSignalPeak = pVadSession->LocalSignalPeak;
    This->ExceedDur = pVadSession->ExceedDur;
    This->OneShotCountDown = pVadSession->OneShotCountDown;
    This->TimeLeftAfterReset = pVadSession->TimeLeftAfterReset;
    This->SpeechDwellTime = pVadSession->SpeechDwellTime;
    This->AbsoluteThr = pVadSession->AbsoluteThr;
    for (i = 0; i < 20; i++) {
        This->pLogEDelayLineStart[i] = pVadSession->pLogEDelayLineStart[i];
    }
    This->pLogEDelayLine = This->pLogEDelayLineStart + pVadSession->LogEDelayLineIndex;
    for (i = 0; i < 150; i++) {
        This->Hist[i] = pVadSession->Hist[i];
    }
    This->NrInHisto = pVadSession->NrInHisto;
    for (i = 0; i < 40; i++) {
        This->CirBuffStart[i] = pVadSession->CirBuffStart[i];
    }
    This->Cir = This->CirBuffStart + pVadSession->CirIndex;
}

/* Resets the automatic gain control state. */
static void ResetAgc(VadBlockFields *This)
{
    This->NbrOfUtterances = 0;
    This->PrevNbrOfUtterances = 0;
    This->MikeCoarse = 0;
}

/* Fill value for the log-energy delay line on reset. */
extern const f32 VadRestartValue;

/* Resets the detector counters and energy history. */
static inline void VadReset(VadBlockFields *This)
{
    unsigned long i;
    This->Initialised = 0;
    This->TimeLeftAfterReset = _tosGetProfileU32(&This->BaseBlock.Object, 6, 2);
    for (i = 0; i < 20; i++)
        This->pLogEDelayLineStart[i] = VadRestartValue;
    This->pLogEDelayLine = This->pLogEDelayLineStart;
    ResetAgc(This);
}

const f32 VadRestartValue = FLT_MAX;

/* Updates rolling level estimates and the voice probability for one frame. */
static void VadFastTrackProcess(VadBlockFields *This, f32 logE)
{
    s16 Bin;
    s16 *Ptr;
    s16 CumSum;
    f32 Diff;
    f32 DelayedLogE;
    f32 FrameType;
    f32 RealSignalPeak;
    f32 EnerToTrack;
    f32 NoiseLevel;
    s16 QuantThr;
    f32 LowerLogE;
    f32 UpperLogE;
    f32 LocSpeechThr;

    This->FrameType = 0.01f;
    if (This->Initialised == 0) {
        This->NoiseRangeTrackerState = 1.0f;
        This->NoiseFloorTrackerState = logE;
        if (This->AgcOn != 0) {
            This->SignalPeakTrackerState = This->InitSignalRange;
        } else {
            This->SignalPeakTrackerState = 1.0f + logE + This->InitSignalRange;
        }
        This->SNR = 3.0f;
        This->SpeechSilenceThr = This->NoiseFloorTrackerState + 1.5f * This->NoiseRangeTrackerState;
        This->SpeechThr = 1.1513 + This->SpeechSilenceThr;
        This->LocalSignalPeak = -1.0e20f;
        This->NrUpdates = 0;
        This->Initialised = 1;
        memset(This->Hist, 0, sizeof(This->Hist));
        This->NrInHisto = 0;
    }

    if (This->TimeLeftAfterReset > 0) {
        if (This->TimeLeftAfterReset != 0) {
            This->Initialised = 0;
        }
        This->TimeLeftAfterReset--;
        return;
    }

    if (This->NrInHisto < 40) {
        This->NrInHisto++;
        QuantThr = (s16) ceilf(0.001 + 0.15f * (f32) This->NrInHisto);
    } else {
        QuantThr = 7;
        This->Hist[*This->Cir]--;
    }

    Bin = 0.5f + 149.0f * (logE - This->BinMin) / 20.5f;
    if (Bin < 0) {
        Bin = 0;
    }
    if (Bin > 149) {
        Bin = 149;
    }
    This->Hist[Bin]++;
    *This->Cir++ = Bin;
    if (This->Cir == This->CirBuffEnd) {
        This->Cir = This->CirBuffStart;
    }

    Ptr = This->Hist;
    CumSum = 0;
    do {
        CumSum += *Ptr++;
    } while (CumSum < QuantThr);
    LowerLogE = This->HistoBins[(u16) (Ptr - This->Hist - 1)];
    Ptr = This->Hist + 149;
    CumSum = 0;
    do {
        CumSum += *Ptr--;
    } while (CumSum < QuantThr);
    UpperLogE = This->HistoBins[(u16) (Ptr - This->Hist + 1)];
    Diff = LowerLogE - This->NoiseFloorTrackerState;
    if (Diff > 0.5f) {
        Diff = 0.5f;
    }
    if (Diff > 0.0f) {
        Diff = 0.01f * Diff;
    } else {
        Diff = 0.05f * Diff;
    }
    This->NoiseFloorTrackerState += Diff;

    DelayedLogE = *This->pLogEDelayLine;
    *This->pLogEDelayLine++ = logE;
    if (This->pLogEDelayLine >= This->pLogEDelayLineEnd) {
        This->pLogEDelayLine = This->pLogEDelayLineStart;
    }
    if (DelayedLogE - This->NoiseFloorTrackerState < 0.5 + This->NoiseRangeTrackerState) {
        Diff = (UpperLogE - This->NoiseFloorTrackerState) - This->NoiseRangeTrackerState;
        if (Diff > 0.1f) {
            Diff = 0.1f;
        }
        if (Diff > 0.0f) {
            Diff = 0.02f * Diff;
        } else {
            Diff = 0.2f * Diff;
        }
        This->NoiseRangeTrackerState += Diff;
        if (This->NoiseRangeTrackerState < 0.46) {
            This->NoiseRangeTrackerState = 0.46f;
        }
    }
    This->SpeechSilenceThr = 1.5f * This->NoiseRangeTrackerState + This->NoiseFloorTrackerState;
    EnerToTrack =
        This->AgcOn != 0 ? This->NoiseRangeTrackerState + This->NoiseFloorTrackerState : 0.0f;
    This->LocalSignalPeak -=
        0.01f * ((This->SignalPeakTrackerState + EnerToTrack) - This->SpeechSilenceThr);
    NoiseLevel = UpperLogE - EnerToTrack;
    if (UpperLogE > This->LocalSignalPeak) {
        This->LocalSignalPeak = NoiseLevel;
    }
    This->SpeechThr = 1.1513 + This->SpeechSilenceThr;
    This->AbsoluteThr = (-7.0f + (This->Sensitivity + This->LNDB0)) - 1000.0f;
    if (This->SpeechThr < This->AbsoluteThr) {
        This->SpeechThr = This->AbsoluteThr;
    }
    if (logE > This->SpeechThr) {
        This->ExceedDur++;
    } else {
        This->ExceedDur = 0;
    }
    if (This->ExceedDur > This->MinSpeechExceedDuration) {
        if (This->OneShotCountDown == 0) {
            This->LocalSignalPeak = This->SpeechSilenceThr - EnerToTrack;
        }
        This->OneShotCountDown = This->BlindTime;
    }
    if (This->OneShotCountDown-- > 0) {
        if (++This->SpeechDwellTime == This->SampleTime || This->OneShotCountDown == 0 ||
            This->LocalSignalPeak > 1.0f + This->SignalPeakTrackerState) {
            NoiseLevel = This->LocalSignalPeak - This->SignalPeakTrackerState;
            This->SpeechDwellTime = 0;
            if (NoiseLevel > 0.0f) {
                This->SignalPeakTrackerState =
                    This->SignalPeakTrackerState + 0.3f * NoiseLevel;
            } else {
                This->SignalPeakTrackerState =
                    This->SignalPeakTrackerState + 0.05f * NoiseLevel;
            }
            This->NrUpdates++;
        }
    } else {
        This->OneShotCountDown = 0;
        This->SpeechDwellTime = 0;
    }
    RealSignalPeak = This->SignalPeakTrackerState + EnerToTrack;
    This->SNR = RealSignalPeak - This->SpeechSilenceThr;
    if (This->Mode_1500 != 0) {
        LocSpeechThr = 0.5 + This->SpeechSilenceThr > RealSignalPeak - This->DetectDynamicRange
                           ? 0.5 + This->SpeechSilenceThr
                           : RealSignalPeak - This->DetectDynamicRange;
        FrameType = (logE - This->SpeechSilenceThr) / (LocSpeechThr - This->SpeechSilenceThr);
    } else if (logE > This->SpeechSilenceThr) {
        FrameType =
            1.0f / (1.0f + expf(-2.0f * (This->DetectDynamicRange + (logE - RealSignalPeak))));
    } else {
        FrameType = 0.0f;
    }
    if (FrameType < 0.0) {
        FrameType = 0.0f;
    } else if (FrameType > 1.0) {
        FrameType = 1.0f;
    }
    This->FrameType = FrameType;
}

/* Averages the available input levels for the detector. */
static void Mean(VadBlockFields *This, f32 **ppResultMean, void *pInputs, int NbrInputs)
{
    f32 **ppr = pInputs;
    f32 Sum = 0.0f;
    f32 Divider = 0.0f;
    s32 i;

    *ppResultMean = NULL;
    for (i = 0; i < NbrInputs; i++) {
        if (ppr[i] != NULL) {
            break;
        }
    }
    if (i == NbrInputs) {
        return;
    }

    for (i = 0; i < NbrInputs; i++) {
        if (ppr[i] != NULL) {
            Sum += *ppr[i];
            Divider += 1.0f;
        }
    }
    This->EnergyIn = Sum / Divider;
    *ppResultMean = &This->EnergyIn;
}

/* Clears a pending request when its countdown expires, then checks whether gain tracking should
 * run. */
static u8 TestChangeMike(VadBlockFields *This)
{
    int NrUpdatesThreshold;
    if (This->AskGainInProcess != 0) {
        if (--This->AskGainWaitCount == 0) {
            This->AskGainInProcess = 0;
            This->FollowUpMike = NOFOLLOWUP;
            This->NrUpdates = 0;
        }
    }
    NrUpdatesThreshold = This->MikeCoarse != 0 ? 2 : 1;
    if (This->NrUpdates >= NrUpdatesThreshold && This->NrUpdates != This->PrevNrUpdates) {
        if (This->FollowUpMike == NOFOLLOWUP) {
            if (ChangeMike(This) != 0) {
                return 1;
            }
        } else {
            FollowUpMike(This);
        }
    }
    This->PrevNrUpdates = This->NrUpdates;
    return 0;
}

/* Processes microphone input for voice activity and automatic gain control. */
static void ProcessVad(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    VadBlockFields *This = (VadBlockFields *) _This;
    f32 *pEnergyIn;
    f32 *pFrameTypeOut;
    f32 *pLevelsOut;

    if (This->BaseBlock.NrInputs != 1) {
        Mean(This, &pEnergyIn, pInputs, NbrInputs);
    } else {
        pEnergyIn = *(f32 **) pInputs;
    }
    if (pEnergyIn != NULL) {
        This->FrameCounter++;
        VadFastTrackProcess(This, *pEnergyIn);
        pFrameTypeOut = (f32 *) qEnQueueOne(This->BaseBlock.OutputQueues[0].pQ);
        *pFrameTypeOut = This->FrameType;

        if (This->OutputLevels != 0) {
            pLevelsOut = (f32 *) qEnQueueOne(This->BaseBlock.OutputQueues[1].pQ);
            *pLevelsOut++ = *pEnergyIn;
            *pLevelsOut++ = This->SignalPeakTrackerState;
            *pLevelsOut++ = This->SpeechThr;
            *pLevelsOut++ = This->SpeechSilenceThr;
            *pLevelsOut++ = This->NoiseFloorTrackerState;
            *pLevelsOut = This->NoiseRangeTrackerState;
        }

        if (This->AgcOn && TestChangeMike(This)) {
            return;
        }
        if (This->CheckAbnormal != 0) {
            CheckForAbnormalConditions(&This->BaseBlock, *pEnergyIn);
            return;
        }
    }
}

/* Enables or disables automatic gain control. */
static u8 EnableAGC(VadBlockFields *This, u8 What)
{
    if (This->AgcOn == What)
        return 0;
    {
        This->AgcOn = What;
        if (What) {
            This->Mike = 0;
            This->AskGainInProcess = 0;
            if (_tosControl(&This->BaseBlock.Object, 0x0A, 0, 0))
                return 1;
            ResetAgc(This);
            This->SignalPeakTrackerState -=
                This->NoiseRangeTrackerState + This->NoiseFloorTrackerState;
        } else
            This->SignalPeakTrackerState +=
                This->NoiseRangeTrackerState + This->NoiseFloorTrackerState;
    }
    return 0;
}

/* Handles state changes and teardown for the voice activity detector. */
static u8 ControlVad(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    VadBlockFields *This = (VadBlockFields *) _This;
    VadControlParameters *pVadParam;

    switch (ControlId) {
    case 1:
        *(s16 *) CParam = (s16) (100.0 * (4.3429446f * This->SNR));
        break;
    case 8:
        *(void **) CParam = &This->SNR;
        break;
    case 2:
        This->AskGainInProcess = 0;
        This->Mike = CParam;
        break;
    case 5: {
        if (CParam != 0) {
            VadPutSession(This, (struct _VADSESSION *) ((u8 *) CParam + *(u32 *) DParam));
            *(u32 *) DParam += sizeof(struct _VADSESSION);
        } else {
            VadReset(This);
        }
        break;
    }
    case 6: {
        if (CParam != 0) {
            VadGetSession(This, (struct _VADSESSION *) ((u8 *) CParam + *(u32 *) DParam));
        }
        *(u32 *) DParam += sizeof(struct _VADSESSION);
        break;
    }
    case 3: {
        pVadParam = (VadControlParameters *) CParam;
        if (EnableAGC(This, pVadParam->On))
            return 0;
        if (This->FarTalk != (u8) pVadParam->FarTalk) {
            This->FarTalk = pVadParam->FarTalk;
            if (pVadParam->FarTalk != 0) {
                This->MikeRange = 0x3333;
            } else {
                This->MikeRange = 0x1EB8;
            }
            ResetAgc(This);
        }
        This->TimeOut = pVadParam->TimeOut * 100;
        break;
    }
    case 4: {
        pVadParam = (VadControlParameters *) CParam;
        pVadParam->On = This->AgcOn;
        pVadParam->FarTalk = This->FarTalk;
        pVadParam->TimeOut = (f32) This->TimeOut / 100.0f;
        break;
    }
    case 7:
        This->NbrOfUtterances++;
        This->FrameCounter = 0;
        break;
    case 0xFF:
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Initializes VAD queue sizes, thresholds, rolling buffers, and profile settings. */
static u8 InitVad(struct _BLOCK *_This)
{
    VadBlockFields *This = (VadBlockFields *) _This;
    f32 BinStart;
    f32 Range;
    int i;
    s16 Sensitivity;

    for (i = This->BaseBlock.NrInputs - 1; i >= 0; i--) {
        This->BaseBlock.InputQueues[i].Size = sizeof(f32);
    }
    This->BaseBlock.OutputQueues[0].Size = sizeof(f32);
    if (This->OutputLevels != 0) {
        This->BaseBlock.OutputQueues[1].Size = 6 * sizeof(f32);
    }

    This->Mode_1500 = (u8) _tosGetProfileU32(&This->BaseBlock.Object, 0x2E, 0);
    if (This->Mode_1500 != 0) {
        This->BinMin = 6.5f;
    } else {
        This->BinMin = 10.5f;
    }
    BinStart = This->BinMin;
    Range = 20.5f;
    for (i = 0; i <= 149; i++) {
        This->HistoBins[i] = BinStart + ((f32) i * Range) / 149.0f;
    }

    This->CirBuffEnd = This->CirBuffStart + 40;
    This->Cir = This->CirBuffStart;
    This->LNDB0 = _tosGetProfileFloat(&This->BaseBlock.Object, 0x2B, 0.0f);
    This->TimeLeftAfterReset = (s16) _tosGetProfileU32(&This->BaseBlock.Object, 6, 2);
    This->BlindTime = (s16) _tosGetProfileU32(&This->BaseBlock.Object, 8, 40);
    This->SampleTime = (s16) _tosGetProfileU32(&This->BaseBlock.Object, 9, 50);
    This->MinSpeechExceedDuration = (s16) _tosGetProfileU32(&This->BaseBlock.Object, 0x0A, 10);
    This->DetectDynamicRange = _tosGetProfileFloat(&This->BaseBlock.Object, 0x0B, 8.69f) / 4.3429;
    Sensitivity = _tosGetProfileU32(&This->BaseBlock.Object, 0xD, 1000);
    This->Sensitivity = 0.0023025852f * Sensitivity;
    This->InitSignalRange =
        (_tosGetProfileFloat(&This->BaseBlock.Object, 0x24, 21.7145f) / 4.3429) - 1.0;
    This->AbsoluteThr = _tosGetProfileFloat(&This->BaseBlock.Object, 0x1B, -1000.0f);
    This->AbsoluteThr = (This->AbsoluteThr / 4.3429446f) + This->LNDB0;
    This->pLogEDelayLineEnd = This->pLogEDelayLineStart + 20;
    for (i = 0; i < 20; i++) {
        This->pLogEDelayLineStart[i] = 2147483648.0f;
    }
    This->pLogEDelayLine = This->pLogEDelayLineStart;
    This->GainInitialised = 0;
    This->AGCSpeed = _tosGetProfileFloat(&This->BaseBlock.Object, 0x29, 100.0f) / 100.0f;
    This->TimeOut = _tosGetProfileU32(&This->BaseBlock.Object, 0x28, 10) * 100;
    This->LogETarget = This->LNDB0 - 1.8420681f;
    This->MikeCoarse = 0;
    This->FarTalk = (s16) _tosGetProfileU32(&This->BaseBlock.Object, 0x27, 0);
    if (This->FarTalk != 0) {
        This->MikeRange = 0x3333;
    } else {
        This->MikeRange = 0x1EB8;
    }
    This->CheckAbnormal = (u8) _tosGetProfileU32(&This->BaseBlock.Object, 0x2A, 0);
    This->EnergyIn = 0.0f;
    return 0;
}

/* Allocates the VAD block using its profile-selected queue counts. */
struct _BLOCK *ConstructVad(struct _TOS *pTos, u8 BlockID)
{
    struct _OBJECT Object;
    u8 OutputLevels;
    VadBlockFields *pVad;

    Object.pTos = pTos;
    Object.BlockID = BlockID;
    OutputLevels = (u8) _tosGetProfileU32(&Object, 0x25, 0);
    pVad = (VadBlockFields *)tosBaseBlockConstruct(
        pTos, BlockID, (u8)_tosGetProfileU32(&Object, 0x2D, 1), OutputLevels != 0 ? 2 : 1,
        ProcessVad, InitVad, ControlVad, sizeof(VadBlockFields));
    pVad->OutputLevels = OutputLevels;
    return &pVad->BaseBlock;
}
