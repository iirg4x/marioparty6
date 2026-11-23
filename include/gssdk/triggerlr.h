/* Trigger detection and voicing analysis state. */
#ifndef GSSDK_TRIGGERLR_H
#define GSSDK_TRIGGERLR_H

#include "types.h"

#include "gssdk/tos.h"

/* Trigger, voicing, histogram and subsampling state for one block. */
typedef struct {
    /* Common processing block and queue ports. */
    struct _BLOCK BaseBlock;
    /* Number of frames seen since the detector was reset. */
    u32 FrameCounter;
    /* Estimated start frame returned by the trigger. */
    u32 BeginTime;
    /* Selects trigger behavior for waveform input. */
    int bWaveMode;
    /* Enables start detection as the trigger is reset. */
    int bDoStartDetection;
    /* Enables the stop path after a start is detected. */
    int bDoStopDetection;
    /* Tracks whether a speech condition has occurred. */
    int bSpeechSeen;
    /* Enables the garbled-sound control event. */
    int bIssueGarbledSound;
    /* Number of silent frames required for a stop. */
    u32 TSCount;
    /* Stop silence setting in milliseconds. */
    u32 OriTSCount;
    /* Queue depth that limits the trigger jump back. */
    u32 Qdepth;
    /* Number of consecutive silent frames. */
    u32 SilenceDuration;
    /* Largest gap tolerated within a speech segment. */
    u32 MaxSilGap;
    /* Minimum speech duration in frames. */
    u32 MinSpeechDur;
    /* Minimum speech setting in milliseconds. */
    u32 OriMinSpeechDur;
    /* Frame count for the current speech segment. */
    u32 SpeechDuration;
    /* Frame count for the current energy peak. */
    u32 PeakDuration;
    /* Minimum frame count used to accept a peak. */
    u32 MinPeakDuration;
    /* Number of frames to replay when speech is found. */
    u32 JumpBack;
    /* Energy threshold setting in hundredths of a decibel. */
    s32 OriAbsThr;
    /* Absolute threshold in the detector's log energy units. */
    f32 AbsThr;
    /* Lower threshold for the log likelihood ratio. */
    f32 LowThresLogLR;
    /* Upper threshold for the log likelihood ratio. */
    f32 HighThresLogLR;
    /* Minimum log likelihood ratio used in the trigger test. */
    f32 MinimalLogLR;
    /* Maximum log likelihood ratio tracked for the segment. */
    f32 maxLogLR;
    /* Offset applied on the noise side of the trigger test. */
    f32 NoiseSensitivity;
    /* Offset used in comparing energy to the histogram threshold and peak. */
    f32 PeakSensitivity;
    /* Current state of the start and stop detector. */
    enum {
        TRIGGERLR_START_DETECTION,
        TRIGGERLR_TS_DETECTION,
        TRIGGERLR_IDLE,
    } TriggerState;
    /* Current state of the energy buffer. */
    enum {
        BUFFER_STATE_LEADINGSILENCE,
        BUFFER_STATE_MAYBESPEECH,
        BUFFER_STATE_SILENCEAFTERMAYBESPEECH,
        BUFFER_STATE_SPEECH,
        BUFFER_STATE_SILENCEAFTERSPEECH,
    } BufferState;
    /* Quantile used to derive the histogram threshold. */
    f32 Quantile;
    /* Highest log energy considered by the peak tracker. */
    f32 maxE4PeakTracker;
    /* Log energy of the current peak. */
    f32 currentPeak;
    /* Maximum permitted duration of speech in frames. */
    u32 maxSpeechDuration;
    /* Records whether the peak estimate has been updated. */
    int bPeakUpdated;
    /* Indicates whether the stored peak can be used. */
    int bPeakIsValid;
    /* Speech condition before the energy test. */
    int bSpeechCondWithoutE;
    /* Result of the peak test. */
    int bPeakCondition;
    /* Most recent voiced or unvoiced state. */
    int bVoicedState;
    /* Start of the log autocorrelation history. */
    f32 *pLogAutoCorrBufferBase;
    /* End of the log autocorrelation history. */
    f32 *pLogAutoCorrBufferEnd;
    /* Newest entry in the log autocorrelation history. */
    f32 *pLogAutoCorrBufferNewest;
    /* Log autocorrelation threshold for voicing. */
    f32 logVoicingThreshold;
    /* Sensitivity value supplied through the control interface. */
    u32 ExternalSensitivity;
    /* Lower endpoint of the external sensitivity range. */
    u32 ExtSens_Min;
    /* Upper endpoint of the external sensitivity range. */
    u32 ExtSens_Max;
    /* Peak sensitivity at the low end of the external range. */
    s32 PeakSens_Min;
    /* Peak sensitivity at the high end of the external range. */
    s32 PeakSens_Max;
    /* Voicing threshold at the low end of the sensitivity range. */
    f32 VoicThr_Beg;
    /* Voicing threshold at the high end of the sensitivity range. */
    f32 VoicThr_End;

    /* Histogram of recent log energy observations. */

    /* Lower edge of the histogram's value range. */
    f32 BinMin;
    /* Range covered by the histogram bins. */
    f32 BinRng;
    /* Number of histogram bins. */
    u32 NbBins;
    /* Index of the last histogram bin. */
    s32 NbBinsm1;
    /* Width of one histogram bin. */
    f32 BinsWidth;
    /* Bin counts for the current history. */
    u32 *pHisto;
    /* Number of entries currently counted in the histogram. */
    u32 NbInHisto;
    /* Number of observations the histogram history can hold. */
    u32 FifoLength;
    /* First entry in the histogram history. */
    s32 *pFifoStart;
    /* End of the histogram history buffer. */
    s32 *pFifoEnd;
    /* Entry to replace at the next histogram update. */
    s32 *pOldestEntry;

    /* Signal and noise history for voicing. */

    /* Smallest lag tested for autocorrelation. */
    u32 minLag;
    /* Largest lag tested for autocorrelation. */
    u32 maxLag;
    /* Number of samples in a voicing analysis frame. */
    u32 frameSize;
    /* Number of samples the signal history can hold. */
    u32 totalBufferSize;
    /* First sample in the signal history. */
    s16 *pSignalBase;
    /* Oldest sample in the signal history. */
    s16 *pSignalOldest;
    /* End of the signal history buffer. */
    s16 *pSignalEnd;
    /* First entry in the squared-sample history. */
    u32 *pSignalSqrBase;
    /* Oldest entry in the squared-sample history. */
    u32 *pSignalSqrOldest;
    /* End of the squared-sample history. */
    u32 *pSignalSqrEnd;
    /* Current slot in the log noise energy history. */
    f32 *pLogNoiseEnergy;
    /* First slot in the log noise energy history. */
    f32 *pLogNoiseEnergyBase;
    /* End of the log noise energy history. */
    f32 *pLogNoiseEnergyEnd;
    /* Number of noise energy values collected. */
    u32 nrNoiseEnergies;
    /* Capacity of the noise energy history. */
    u32 maxNrNoiseEnergies;
    /* Minimum signal-to-noise ratio for voicing. */
    f32 voicingMinSNR;
    /* Minimum signal energy for voicing. */
    f32 voicingMinEnergy;

    /* Subsampling filter and sample history. */

    /* Number of input samples passed to the subsampler. */
    u32 NrInSamples;
    /* Output buffer for the subsampled signal. */
    s16 *pSubSampledSignal;
    /* Filter length used to size the subsampler. */
    u32 N;
    /* Number of even-position filter entries. */
    u32 Neven;
    /* Number of odd-position filter entries. */
    u32 Nodd;
    /* Start of the subsampling filter response. */
    s32 *pFilterResponse;
    /* End of the subsampling filter response. */
    s32 *pFilterEnd;
    /* Scaling in bits of the filter coefficients. */
    s32 nrBitsInTabCoeff;
    /* Shift used for the center filter sample. */
    s32 centerTabShift;
    /* Start of the odd-sample history. */
    s32 *pOddSamplesBase;
    /* End of the odd-sample history. */
    s32 *pOddSamplesEnd;
    /* Oldest entry in the odd-sample history. */
    s32 *pOddSamplesOldest;
    /* Center entry of the odd-sample filter window. */
    s32 *pOddSamplesCenter;
    /* Start of the even-sample pointer history. */
    s32 **ppEvenSamplesBase;
    /* Oldest pointer in the even-sample history. */
    s32 **ppEvenSamplesOldest;
    /* End of the even-sample pointer history. */
    s32 **ppEvenSamplesEnd;
    /* State of the subsampler between calls. */
    enum { SUBSAMP_STATE_EVEN, SUBSAMP_STATE_ODD } SubSampState;
} TriggerLR;

/* Saved FIFO index. The serialized bins follow this record. */
struct _SLIDINGHISTOSESSION {
    /* Index of the oldest history entry, relative to the FIFO base. */
    long OldestElemIdx;
};

/* Inserts an observation into the sliding histogram. */
void SlidingHisto_NewItem(TriggerLR *This, f32 LogE);

/* Returns the lower quantile of the current observations. */
f32 SlidingHisto_LowerQuantile(TriggerLR *This, f32 Quantile);

/* Resets the histogram and its FIFO. */
void SlidingHisto_Clear(TriggerLR *This);

/* Allocates histogram bins and history storage. */
int SlidingHisto_Init(TriggerLR *This);

/* Releases histogram storage. */
void SlidingHisto_Free(TriggerLR *This);

/* Returns the byte count needed to save the histogram state. */
u32 SlidingHisto_sizeof_SessionData(TriggerLR *This);

/* Restores the histogram from a saved session. */
void SlidingHisto_PutSession(
    TriggerLR *This, struct _SLIDINGHISTOSESSION *pSlidingHistoSession);

/* Writes the histogram history and position to a session. */
void SlidingHisto_GetSession(
    TriggerLR *This, struct _SLIDINGHISTOSESSION *pSlidingHistoSession);

/* Adds signal samples to the voicing window. */
int Voicing_AddSignal(TriggerLR *This, s16 *pSamples);

/* Updates the noise-energy history. */
void Voicing_MaintainNoiseEner(TriggerLR *This);

/* Returns the maximum voicing measure across the searched lags. */
f32 Voicing_GetMaxVoicing(TriggerLR *This);

/* Clears the voicing history. */
void Voicing_Reset(TriggerLR *This);

/* Allocates the buffers used in voicing analysis. */
int InitVoicing(TriggerLR *This);

/* Releases voicing buffers. */
void Voicing_Free(TriggerLR *This);

/* Feeds samples into the subsampler and returns its current output. */
int Subsampler_Process(TriggerLR *This, s16 *pInSamples);

/* Clears the subsampler's filter history. */
void Subsampler_Reset(TriggerLR *This);

/* Allocates and initializes the subsampling filter. */
int Subsampler_Init(TriggerLR *This);

/* Releases subsampler storage. */
void Subsampler_Free(TriggerLR *This);

#endif
