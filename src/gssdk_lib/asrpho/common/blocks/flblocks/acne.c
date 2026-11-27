/* Speech-mean adaptation and spectral normalization for the ASR feature graph. */
#include "gssdk/mathusage.h"
#include "gssdk/blocks.h"
#include "gssdk/heap.h"

#include <stddef.h>
#include <string.h>

#include "gssdk/mqueue.h"
#include "gssdk/smoother.h"

typedef struct Acne {
    /* TOS graph callbacks and the block's input and output ports. */
    struct _BLOCK BaseBlock;
    /* FIFO of feature frames waiting for segment normalization. */
    struct _QUEUE *pInternalQueue;
    /* Queue depth that causes an active speech segment to emit its oldest frame. */
    u16 Qdepth;
    /* Tracks whether the block is waiting for, retaining, or emitting frames. */
    enum { StableCMS, Event, AwaitingEvent } CmsState;
    /* Set while queued frames are being emitted during a drain. */
    u8 FlushFlag;
    /* Enables normalization with pChannelCompensation; a failed refresh does not clear this
     * flag. */
    u8 ValidChannel;
    /* Per-band smoothed adaptive speech mean in log-amplitude units. */
    f32 *pChannelCompensation;
    /* Per-band speech mean updated from incoming feature frames. */
    f32 *pSpeechMean;
    /* Per-band noise estimate copied from the feature extractor. */
    f32 *pSilenceMean;
    /* Latest scalar reference used when converting every queued frame to output energy. */
    f32 SignalPeak;
    /* Accumulated weight used while adapting the speech mean. */
    f32 NbrSpeechFrames;
    /* Count of zero-activity frames since the first input, capped at 50 until reset. */
    s32 NbrSilenceFrames;
    /* Capped total activity weight used for the uniform correction applied to every band. */
    f32 NbrSpeechFramesForLevel;
    /* Progress toward refreshing the smoothed mean. */
    f32 TotUpdate;
    /* Number of feature frames received since the last reset. */
    u32 SpeakerFrameCtr;
    /* Consecutive zero-activity frames used to end a speech segment. */
    s32 SilGap;
    /* Number of spectral bands in each feature frame. */
    u32 nLogPower;
    /* Smoothing progress required before refreshing pChannelCompensation. */
    f32 SmootherRecomputeThr;
    /* Maximum silent frames accepted before the block returns to silence. */
    s32 MaxSilGap;
    /* Maximum accumulated activity weight before queued frames are drained. */
    f32 NbrFramesForStableCMS;
    /* Minimum scalar energy written to the normalized output. */
    f32 LogEMask;
    /* Ratio used for positive and negative adaptive-mean and residual corrections. */
    f32 InBal;
    /* Offset applied to each per-band noise estimate before comparison. */
    f32 PreMaskwrtNoise;
    /* Lower bound applied to the adjusted-input minus adaptive-mean difference before each mean
     * update. */
    f32 PreMaskwrtPeak;
    u32 FrameTypeDelay; /* The game's code never reads this field. */
    /* Initial activity weight used when the speech means are seeded. */
    f32 InitNbrSpeechFrames;
    /* Per-band output floors applied with the smoothed mean, or copied as output before it is
     * available. */
    const f32 *pNoiseFloors;
    /* Initial per-band speech means; initialization always uses the 11 kHz table. */
    const f32 *pSpeechMeanInit;
    /* Smoothing matrices; initialization always uses the 11 kHz set. */
    const struct _SMOOTHERINFO *pSmootherInfo;
    /* Prepares the initial mean and refreshes the adaptive per-band mean. */
    struct BandPassSmootherTag *pSmoother;
} Acne;

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

static u8 Reset(Acne *This);
static void AcneGetSession(Acne *This, void *pAcneSession);
static void AcnePutSession(Acne *This, void *pAcneSession);
static u8 NormalizeAndEnQ(Acne *This);

/* Clears queued frames and seeds the adaptive speech model for a new session. */
static u8 Reset(Acne *This)
{
    qQueueReset(This->pInternalQueue);
    This->CmsState = AwaitingEvent;
    This->FlushFlag = 0;
    if (This->InitNbrSpeechFrames > 0.0f) {
        memcpy(
            This->pSpeechMean, This->pSpeechMeanInit,
            This->nLogPower * sizeof(f32));
        memcpy(
            This->pChannelCompensation, This->pSpeechMeanInit,
            This->nLogPower * sizeof(f32));
        /* A failed smoothing pass leaves the queue empty, the state set to AwaitingEvent,
         * and the copied or partly smoothed means in place. */
        if (Smoothing(This->pSmoother, This->pChannelCompensation)) {
            return 1;
        }
        This->ValidChannel = 1;
    } else {
        memset(
            This->pSpeechMean, 0,
            This->nLogPower * sizeof(f32));
        This->ValidChannel = 0;
    }

    memset(
        This->pSilenceMean, 0,
        This->nLogPower * sizeof(f32));
    This->SignalPeak = 100000.0f;
    This->NbrSpeechFrames = This->InitNbrSpeechFrames;
    This->NbrSilenceFrames = 0;
    This->NbrSpeechFramesForLevel = 0.5f;
    This->TotUpdate = 0.0f;
    This->SpeakerFrameCtr = 0;
    This->SilGap = 0;
    return 0;
}

/* Saves the estimator state and per-band arrays for a TOS session snapshot. */
static void AcneGetSession(Acne *This, void *pAcneSession)
{
    u32 bandBytes = This->nLogPower * sizeof(f32);
    u8 *session = pAcneSession;
    u32 *sessionWords = pAcneSession;
    float *pIn;

    sessionWords[0] = This->CmsState;
    session[4] = This->FlushFlag;
    session[8] = This->ValidChannel;
    /* The session header reserves twelve bytes before the per-band arrays. */
    session += 12;

    memcpy(session, This->pChannelCompensation, bandBytes);
    session += bandBytes;
    memcpy(session, This->pSpeechMean, bandBytes);
    session += bandBytes;
    memcpy(session, This->pSilenceMean, bandBytes);
    session += bandBytes;

    pIn = (float *)session;
    sessionWords = (u32 *)pIn;
    pIn[0] = This->SignalPeak;
    pIn[1] = This->NbrSpeechFrames;
    sessionWords[2] = This->NbrSilenceFrames;
    pIn[3] = This->NbrSpeechFramesForLevel;
    pIn[4] = This->TotUpdate;
    sessionWords[5] = This->SpeakerFrameCtr;
    sessionWords[6] = This->SilGap;
}

/* Restores the estimator state and per-band arrays when a TOS session resumes. */
static void AcnePutSession(Acne *This, void *pAcneSession)
{
    u32 bandBytes = This->nLogPower * sizeof(f32);
    const u8 *session = pAcneSession;
    const u32 *sessionWords = pAcneSession;
    const float *pIn;

    This->CmsState = sessionWords[0];
    This->FlushFlag = session[4];
    This->ValidChannel = session[8];
    /* Skip the saved state and flags before restoring the per-band arrays. */
    session += 12;

    memcpy(This->pChannelCompensation, session, bandBytes);
    session += bandBytes;
    memcpy(This->pSpeechMean, session, bandBytes);
    session += bandBytes;
    memcpy(This->pSilenceMean, session, bandBytes);
    session += bandBytes;

    pIn = (const float *)session;
    sessionWords = (const u32 *)pIn;
    This->SignalPeak = pIn[0];
    This->NbrSpeechFrames = pIn[1];
    This->NbrSilenceFrames = (s32)sessionWords[2];
    This->NbrSpeechFramesForLevel = pIn[3];
    This->TotUpdate = pIn[4];
    This->SpeakerFrameCtr = sessionWords[5];
    This->SilGap = (s32)sessionWords[6];
}

/* Attempts to normalize and publish one queued feature frame. A smoothing failure drops the
 * frame but retains its queue node. A drain loop stalls if no unread frame remains
 * to release the node. */
static u8 NormalizeAndEnQ(Acne *This)
{
    f32 *pOut;
    const f32 *pNoiseFloor;
    f32 *pSpecOut;
    f32 *pEnd;
    f32 *pChannelCompensation;
    u32 nLogPower;
    f32 *Qread;

    pNoiseFloor = This->pNoiseFloors;
    nLogPower = This->nLogPower;

    Qread = qDeQueueOne(This->pInternalQueue, 0);
    if (Qread == NULL) {
        return 0;
    }

    /* A failed refresh keeps the updated pChannelCompensation and leaves ValidChannel set
     * as it was before smoothing. */
    if (This->TotUpdate > This->SmootherRecomputeThr) {
        This->TotUpdate = 0.0f;
        memcpy(
            This->pChannelCompensation, This->pSpeechMean,
            nLogPower * sizeof(f32));
        if (Smoothing(This->pSmoother, This->pChannelCompensation)) {
            return 0;
        }
        This->ValidChannel = 1;
    }

    pOut = qEnQueueOne(This->BaseBlock.OutputQueues[0].pQ);
    /* Queue elements store scalar energy in slot zero and spectral bands after it. */
    pSpecOut = pOut + 1;
    if (This->ValidChannel) {
        /* Subtract the learned speech mean, then apply the per-band noise floor. */
        pEnd = pSpecOut + nLogPower;
        memcpy(pSpecOut, Qread, nLogPower * sizeof(f32));
        pChannelCompensation = This->pChannelCompensation;

        while (pSpecOut < pEnd) {
            *pSpecOut -= *pChannelCompensation++;
            if (*pNoiseFloor > *pSpecOut) {
                *pSpecOut = *pNoiseFloor +
                    HLnOnePlusExpHFloat(*pSpecOut - *pNoiseFloor);
            } else {
                *pSpecOut +=
                    HLnOnePlusExpHFloat(*pNoiseFloor - *pSpecOut);
            }
            pSpecOut++;
            pNoiseFloor++;
        }
    } else {
        /* Emit the configured spectral floors when normalization is disabled. */
        memcpy(pSpecOut, pNoiseFloor, nLogPower * sizeof(f32));
    }

    pOut[0] = LogAdd(
        Qread[nLogPower] - This->SignalPeak,
        This->LogEMask);
    qFreeUnusedElemements(This->pInternalQueue);
    return 1;
}

/* Queues and adapts a fresh feature frame unless a pending drain emits one queued frame and
 * returns first. TOS has already dequeued fresh inputs, so that drain callback discards them. */
static void ProcessAcne(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Acne *This = (Acne *)_This;
    u32 nLogPower;
    f32 *pSpecIn;
    f32 *Qwrite;
    f32 FrameType;
    f32 Energy;

    if (This->NbrSpeechFrames >= This->NbrFramesForStableCMS) {
        This->CmsState = StableCMS;
        This->FlushFlag = 1;
        while (qQueueNbrElements(This->pInternalQueue) != 0) {
            NormalizeAndEnQ(This);
        }
    }

    if (This->FlushFlag &&
        qQueueNbrElements(This->pInternalQueue) == 0) {
        This->FlushFlag = 0;
    }
    if (This->FlushFlag) {
        NormalizeAndEnQ(This);
        return;
    }

    if (((void **)pInputs)[0] == NULL) {
        return;
    }

    pSpecIn = ((void **)pInputs)[0];
    /* Stationarity supplies the activity score. The VAD level packet starts with
     * frame log energy and the tracked signal peak used as its reference here. */
    FrameType = *(f32 *)((void **)pInputs)[2];
    nLogPower = This->nLogPower;
    Energy = ((f32 *)((void **)pInputs)[3])[0];
    This->SignalPeak = ((f32 *)((void **)pInputs)[3])[1];
    memcpy(This->pSilenceMean, ((void **)pInputs)[1], nLogPower * sizeof(f32));

    Qwrite = qEnQueueOne(This->pInternalQueue);
    memcpy(Qwrite, pSpecIn, nLogPower * sizeof(f32));
    Qwrite[nLogPower] = Energy;
    This->SpeakerFrameCtr++;

    if (FrameType > 0.001f) {
        f32 LevelAdapt;
        f32 Gain_p;
        f32 Gain_n;
        f32 Update;
        f32 *pNoise = This->pSilenceMean;
        f32 *pDst = This->pSpeechMean;
        f32 *pEnd = pDst + nLogPower;
        f32 *pSrc = Qwrite;

        This->NbrSpeechFrames += FrameType;
        if (This->NbrSpeechFrames >=
            This->NbrFramesForStableCMS) {
            This->NbrSpeechFrames =
                This->NbrFramesForStableCMS;
        }

        /* Activity controls how quickly the adaptive model follows this frame. */
        LevelAdapt = FrameType / This->NbrSpeechFrames;
        This->TotUpdate += LevelAdapt;
        Gain_p = LevelAdapt * This->InBal;
        if (Gain_p > 1.0f) {
            Gain_p = 1.0f;
        }
        Gain_n = LevelAdapt / This->InBal;
        if (Gain_n > 1.0f) {
            Gain_n = 1.0f;
        }

        Update = 0.0f;

        for (; pDst < pEnd; pDst++, pSrc++, pNoise++) {
            f32 FlooredSpeech = *pSrc;
            f32 Diff;
            f32 update;

            if (FlooredSpeech < *pNoise + This->PreMaskwrtNoise) {
                FlooredSpeech = *pNoise + This->PreMaskwrtNoise;
            }
            Diff = FlooredSpeech - *pDst;
            if (Diff < This->PreMaskwrtPeak) {
                Diff = This->PreMaskwrtPeak;
            }
            if (Diff > 0.0f) {
                update = Diff * Gain_p;
            } else {
                update = Diff * Gain_n;
            }
            *pDst += update;

            Diff = FlooredSpeech - *pDst;
            if (Diff > 0.0f) {
                update = Diff * This->InBal;
            } else {
                update = Diff / This->InBal;
            }
            Update += update;
        }

        /* Apply a weighted band-average residual correction to every band, capped at +/-0.7. */
        This->NbrSpeechFramesForLevel += FrameType;
        if (This->NbrSpeechFramesForLevel > 200.0f) {
            This->NbrSpeechFramesForLevel = 200.0f;
        }
        LevelAdapt = FrameType / This->NbrSpeechFramesForLevel;
        This->TotUpdate += LevelAdapt;
        Update *= LevelAdapt / (f32)nLogPower;
        if (Update > 0.7f) {
            Update = 0.7f;
        }
        if (Update < -0.7f) {
            Update = -0.7f;
        }

        pDst = This->pSpeechMean;
        while (pDst < pEnd) {
            *pDst += Update;
            pDst++;
        }
    }

    if (!FrameType && This->SpeakerFrameCtr != 0) {
        This->SilGap++;
        This->NbrSilenceFrames++;
        if (This->NbrSilenceFrames > 50) {
            This->NbrSilenceFrames = 50;
        }
    } else {
        This->SilGap = 0;
    }

    /* Holds frames during a speech event until silence or the queue depth requires output. */
    switch (This->CmsState) {
    case StableCMS:
        NormalizeAndEnQ(This);
        break;
    case AwaitingEvent:
        if (FrameType > 0.001f) {
            This->CmsState = Event;
        } else {
            NormalizeAndEnQ(This);
        }
        break;
    case Event:
        if (This->SilGap >
            This->MaxSilGap) {
            This->CmsState = AwaitingEvent;
            while (qQueueNbrElements(This->pInternalQueue) != 0) {
                NormalizeAndEnQ(This);
            }
        } else if (qQueueNbrElements(This->pInternalQueue) ==
            This->Qdepth) {
            NormalizeAndEnQ(This);
        }
        break;
    }
}

/* Frees ACNE queues, model storage and the smoother. */
static void FreeResourcesAcne(Acne *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    qQueueControl(This->pInternalQueue, 0xFF, 0, 0);
    heap_Free(pTos->hHeap, This->pSpeechMean);
    smtDestruct(This->pSmoother, pTos);
    tosBaseBlockDestruct(&This->BaseBlock);
}

/* Command 2 requests a drain on the next process callback, 3 saves when given a session buffer, 4
 * restores or resets, and 0xFF destroys the block. */
static u8 ControlAcne(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    Acne *This = (Acne *)_This;

    switch (ControlId) {
    case 2:
        This->FlushFlag = 1;
        break;

    case 4:
        if (CParam != 0) {
            u8 *session = (u8 *)CParam;

            AcnePutSession(This, session + *(u32 *)DParam);
            /* The session payload includes a 12-byte header, three band arrays, and seven saved
             * scalar values. */
            *(u32 *)DParam +=
                (This->nLogPower * 3 + 10) * sizeof(u32);
        } else if (Reset(This)) {
            return 0;
        }
        break;

    case 3:
        if (CParam != 0) {
            u8 *session = (u8 *)CParam;

            AcneGetSession(This, session + *(u32 *)DParam);
        }
        /* Advance over the header, three band arrays, and seven scalars even without a session
         * buffer. */
        *(u32 *)DParam +=
            (This->nLogPower * 3 + 10) * sizeof(u32);
        break;

    case 255:
        FreeResourcesAcne(This);
        break;

    default:
        return 0;
    }

    return 1;
}

/* Allocates the block queues and speech-model buffers when the graph starts. */
static u8 InitAcne(struct _BLOCK *_This)
{
    Acne *This = (Acne *)_This;
    struct _TOS *pTos = _This->Object.pTos;
    u32 nLogPower;
    f32 SegWidth;

    /* Profile values select the feature width and the queued-frame threshold for emission. */
    nLogPower = This->nLogPower =
        (u16)_tosGetProfileU32(&This->BaseBlock.Object, 9, 21);
    _This->InputQueues[0].Size = (u16)(nLogPower * sizeof(f32));
    _This->InputQueues[1].Size = (u16)(nLogPower * sizeof(f32));
    _This->InputQueues[2].Size = sizeof(f32);
    _This->InputQueues[3].Size = 6 * sizeof(f32); /* VAD level packet. */
    _This->OutputQueues[0].Size =
        (u16)((nLogPower + 1) * sizeof(f32));

    This->Qdepth =
        (u16)_tosGetProfileU32(&This->BaseBlock.Object, 2, 100);
    This->pInternalQueue = qQueueConstruct(pTos, 0xFF, 1);
    if (This->pInternalQueue == NULL) {
        return 1;
    }
    if (qQueueInitEx(
            This->pInternalQueue, (u16)((nLogPower + 1) * sizeof(f32)), 1, 0) == NULL) {
        return 1;
    }

    This->pSpeechMean =
        heap_Calloc(pTos->hHeap, nLogPower * 3, sizeof(f32));
    This->pSilenceMean = This->pSpeechMean + nLogPower;
    This->pChannelCompensation = This->pSilenceMean + nLogPower;

    This->PreMaskwrtNoise =
        0.11513f * _tosGetProfileFloat(&This->BaseBlock.Object, 3, -1000.0f);
    This->PreMaskwrtPeak =
        0.11513f * _tosGetProfileFloat(&This->BaseBlock.Object, 4, -1000.0f);
    This->InBal = 4.0f;
    This->NbrFramesForStableCMS =
        _tosGetProfileFloat(&This->BaseBlock.Object, 5, 50.0f);
    This->SmootherRecomputeThr =
        10.0f / This->NbrFramesForStableCMS;
    This->MaxSilGap =
        (s16)_tosGetProfileU32(&This->BaseBlock.Object, 6, 15);
    This->LogEMask =
        0.23026f * _tosGetProfileFloat(&This->BaseBlock.Object, 7, -30.0f);

    This->InitNbrSpeechFrames = 5.0f;
    This->pNoiseFloors = g_NoiseFloors11kHz;
    This->pSpeechMeanInit = g_SpeechMeanInit11kHz;
    This->pSmootherInfo = &Smoother1600Info11kHz;
    SegWidth = _tosGetProfileFloat(&This->BaseBlock.Object, 8, 2.6f);
    This->pSmoother = smtConstruct(
        pTos, nLogPower,
        SegWidth, This->pSmootherInfo, 0.0f);

    if (Reset(This)) {
        return 1;
    }
    return 0;
}

/* Allocates the adaptive feature block and installs its TOS callbacks during graph construction. */
struct _BLOCK *ConstructAcne(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 4, 1, ProcessAcne, InitAcne,
        ControlAcne, sizeof(Acne));
}
