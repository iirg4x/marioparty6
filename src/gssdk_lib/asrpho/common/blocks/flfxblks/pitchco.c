// Combines pitch, normalized energy and voicing histories into delayed speech features.
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include "gssdk/tos.h"

/* Cursors for one circular feature history. */
typedef struct {
    f32 *pBeg; // First sample in the circular history.
    f32 *pRead; // Sample consumed by the delayed feature output.
    f32 *pWrite; // Next history sample to fill.
    f32 *pEnd; // One past the final allocated sample.
} PitchHistory;

/* Combines delayed voicing, pitch and energy features. */
typedef struct {
    /* Speech graph ports, callbacks and owning context. */
    struct _BLOCK BaseBlock;
    PitchHistory NormE; // Circular history of maximum input band energy.
    f32 *pNormE_CalcReadPointer; // First energy sample used by the current filter window.
    PitchHistory LogPitch; // Circular history of logarithmic pitch samples.
    f32 *pVoicedLogPitch; // First pitch sample used by the current voiced window.
    PitchHistory VUV; // Input voicing bytes stored as floats; only 1.0 is voiced.
    PitchHistory DeltaLogPitch; // Circular history of pitch-window slope values.
    PitchHistory MeanLogPitch; // Circular history of pitch-window mean values.
    PitchHistory DeltaNormE; // Circular history of normalized energy slopes.
    f32 NormPitch; // Delayed log pitch minus the mean for its corresponding window.
    f32 *pCoeff; // Seven coefficients for the pitch slope calculation.
    f32 *pCoeffE; // Five coefficients for the energy slope calculation.
    u8 NbrOfCoeff; // Number of pitch samples in a voiced analysis window.
    u8 NbrOfCoeffE; // Number of energy samples in an analysis window.
    u8 FrameQuota; // Number of samples allocated to each circular history.
    u8 VoicedCount; // Frames in the voiced window; decremented after each full window.
    u8 NbrInBuffer; // Delayed input frames not yet emitted as output features.
    u8 VUV_Status; // Whether the current input frame is voiced (1) or unvoiced (0).
    u8 PrevVUV_Status; // Voicing state from the preceding input frame.
    u8 BeginVoicedFlag; // Set at a voiced run's start; cleared only after a complete voiced window.
    u8 EndVoicedFlag; // Set when a voiced run ends with window length minus one frames held.
    u8 VeryFirstTime; // Set while the initial energy slope history is filled.
    u16 DenominatorPitch; // Divisor applied to the pitch slope sum.
    u16 DenominatorEnergy; // Divisor applied to the energy slope sum.
    f32 LastCalculated_MeanLogPitch; // Most recent mean log pitch, repeated after voicing ends.
    f32 LastCalculated_DeltaLogPitch; // Most recent pitch slope, repeated after voicing ends.
    u8 PitchDP_BufLength; /* The game's code never reads this field. */
    u8 NbrOfOutputs; // Number of floats written for each output frame.
    u8 FlushFlag; // Set while queued frames are emitted after input ends.
    /* Pending pitch padding attempt; cleared even when too few frames remain. */
    u8 Last_DeltaAndMeanLogPitchValues_ToWrite;
    u8 LastDeltaNormEsToWrite; // Set until the energy slope history is padded for flushing.
    u8 Increment_pVoicedLogPitch; // Set when a completed voiced window advances the pitch filter.
    u16 nLogPower; // Number of energy bands in each input spectrum.
    f32 Voiced_OutputValue; // Configured output value for a voiced frame.
    f32 Unvoiced_OutputValue; // Configured output value for an unvoiced frame.
} PitchCombiner;

// Clears a feature history and restores its initial read and write positions.
// InitPitchCombiner and PitchCombinerReset call this when preparing the block.
static void InitBuffer(PitchHistory *pBuffer, u8 Length)
{
    u16 i;

    pBuffer->pRead = pBuffer->pWrite = pBuffer->pBeg;
    pBuffer->pEnd = pBuffer->pBeg + Length;
    for (i = 0; i < Length; i++) {
        pBuffer->pBeg[i] = 0.0f;
    }
}

// Averages the current voiced window, wrapping across the pitch history's end.
// LogPitchProcessing calls this after a complete voiced pitch window is ready.
static f32 CalcMeanLogPitch(PitchCombiner *This)
{
    f32 *pRead;
    u16 i;
    f32 MeanLogPitch;
    u8 length;

    pRead = This->pVoicedLogPitch;
    MeanLogPitch = 0.0f;
    length = This->NbrOfCoeff;
    for (i = 0; i < length; i++) {
        MeanLogPitch += *pRead++;
        if (pRead == This->LogPitch.pEnd) {
            pRead = This->LogPitch.pBeg;
        }
    }
    MeanLogPitch /= length;
    return MeanLogPitch;
}

// Applies the slope coefficients to the current voiced window.
// LogPitchProcessing calls this after a complete voiced pitch window is ready.
static f32 CalcDeltaLogPitch(PitchCombiner *This)
{
    u8 pitchWindowLength;
    f32 *pRead;
    f32 *pCoeff;
    u8 i;
    f32 DeltaLogPitch;

    pRead = This->pVoicedLogPitch;
    pCoeff = This->pCoeff;
    DeltaLogPitch = 0.0f;
    pitchWindowLength = This->NbrOfCoeff;
    for (i = 0; i < pitchWindowLength; i++) {
        DeltaLogPitch += *pRead++ * *pCoeff++;
        if (pRead == This->LogPitch.pEnd) {
            pRead = This->LogPitch.pBeg;
        }
    }
    DeltaLogPitch /= This->DenominatorPitch;
    return DeltaLogPitch;
}

// Returns the largest value among the energy bands in a new input frame.
// ProcessPitchCombiner calls this for each spectrum; even a zero count reads its first value.
static f32 MaxFloatInArray(const f32 *pFlt, u16 NbrOfFlt)
{
    f32 max = pFlt[0];
    const f32 *value = pFlt + 1;
    u16 i;

    for (i = 1; i < NbrOfFlt; i++, value++) {
        if (*value > max) {
            max = *value;
        }
    }
    return max;
}

// Tracks voiced runs and advances the voicing history for each input frame.
// ProcessPitchCombiner calls this after storing the frame's voicing value.
static void StatusProcessing(PitchCombiner *This)
{
    This->PrevVUV_Status = This->VUV_Status;
    if (*This->VUV.pWrite == 1.0f) {
        if (This->VUV_Status == 0) {
            This->VUV_Status = 1;
            This->BeginVoicedFlag = 1;
        }
        This->VoicedCount++;
    } else if (This->VUV_Status == 1) {
        if (This->VoicedCount == This->NbrOfCoeff - 1) {
            This->EndVoicedFlag = 1;
        }
        This->VUV_Status = 0;
        This->VoicedCount = 0;
    }

    This->VUV.pWrite++;
    if (This->VUV.pWrite == This->VUV.pEnd) {
        This->VUV.pWrite = This->VUV.pBeg;
    }
}

// Seeds the earlier half-window of pitch results with the first complete result.
// LogPitchProcessing calls this after producing the first complete voiced window.
static void CheckBeginVoicedFlag(PitchCombiner *This)
{
    f32 *pTmpWrite;
    f32 *pLastWritten;
    u8 i;

    if (This->BeginVoicedFlag != 1) {
        return;
    }

    pLastWritten = This->MeanLogPitch.pWrite - 1;
    if (pLastWritten < This->MeanLogPitch.pBeg) {
        pLastWritten = This->MeanLogPitch.pEnd - 1;
    }
    pTmpWrite = pLastWritten - 1;
    if (pTmpWrite < This->MeanLogPitch.pBeg) {
        pTmpWrite = This->MeanLogPitch.pEnd - 1;
    }
    for (i = 0; i < This->NbrOfCoeff / 2; i++) {
        *pTmpWrite = *pLastWritten;
        pTmpWrite--;
        if (pTmpWrite < This->MeanLogPitch.pBeg) {
            pTmpWrite = This->MeanLogPitch.pEnd - 1;
        }
    }

    pLastWritten = This->DeltaLogPitch.pWrite - 1;
    if (pLastWritten < This->DeltaLogPitch.pBeg) {
        pLastWritten = This->DeltaLogPitch.pEnd - 1;
    }
    pTmpWrite = pLastWritten - 1;
    if (pTmpWrite < This->DeltaLogPitch.pBeg) {
        pTmpWrite = This->DeltaLogPitch.pEnd - 1;
    }
    for (i = 0; i < This->NbrOfCoeff / 2; i++) {
        *pTmpWrite = *pLastWritten;
        pTmpWrite--;
        if (pTmpWrite < This->DeltaLogPitch.pBeg) {
            pTmpWrite = This->DeltaLogPitch.pEnd - 1;
        }
    }
    This->BeginVoicedFlag = 0;
}

// Repeats the last pitch results across the trailing half-window of a voiced run.
// LogPitchProcessing calls this while advancing histories without a full voiced window.
static void CheckEndVoicedFlag(PitchCombiner *This)
{
    f32 *pTmpWrite;
    u8 i;

    if (This->EndVoicedFlag != 1) {
        return;
    }

    pTmpWrite = This->MeanLogPitch.pWrite - 1;
    if (pTmpWrite < This->MeanLogPitch.pBeg) {
        pTmpWrite = This->MeanLogPitch.pEnd - 1;
    }
    for (i = 0; i < This->NbrOfCoeff / 2; i++) {
        *pTmpWrite = This->LastCalculated_MeanLogPitch;
        pTmpWrite++;
        if (pTmpWrite == This->MeanLogPitch.pEnd) {
            pTmpWrite = This->MeanLogPitch.pBeg;
        }
    }

    pTmpWrite = This->DeltaLogPitch.pWrite - 1;
    if (pTmpWrite < This->DeltaLogPitch.pBeg) {
        pTmpWrite = This->DeltaLogPitch.pEnd - 1;
    }
    for (i = 0; i < This->NbrOfCoeff / 2; i++) {
        *pTmpWrite = This->LastCalculated_DeltaLogPitch;
        pTmpWrite++;
        if (pTmpWrite == This->DeltaLogPitch.pEnd) {
            pTmpWrite = This->DeltaLogPitch.pBeg;
        }
    }
    This->EndVoicedFlag = 0;
}

// Updates pitch results for full voiced windows and pads their run boundaries.
// ProcessPitchCombiner calls this once the input delay contains a full pitch window.
static void LogPitchProcessing(PitchCombiner *This)
{
    if (This->VoicedCount == This->NbrOfCoeff) {
        This->Increment_pVoicedLogPitch = 1;
        *This->DeltaLogPitch.pWrite = CalcDeltaLogPitch(This);
        This->LastCalculated_DeltaLogPitch = *This->DeltaLogPitch.pWrite++;
        if (This->DeltaLogPitch.pWrite == This->DeltaLogPitch.pEnd) {
            This->DeltaLogPitch.pWrite = This->DeltaLogPitch.pBeg;
        }
        *This->MeanLogPitch.pWrite = CalcMeanLogPitch(This);
        This->LastCalculated_MeanLogPitch = *This->MeanLogPitch.pWrite++;
        if (This->MeanLogPitch.pWrite == This->MeanLogPitch.pEnd) {
            This->MeanLogPitch.pWrite = This->MeanLogPitch.pBeg;
        }

        CheckBeginVoicedFlag(This);
        /* The next voiced frame completes the window held here. */
        This->VoicedCount--;
    } else {
        This->DeltaLogPitch.pWrite++;
        if (This->DeltaLogPitch.pWrite == This->DeltaLogPitch.pEnd) {
            This->DeltaLogPitch.pWrite = This->DeltaLogPitch.pBeg;
        }
        This->MeanLogPitch.pWrite++;
        if (This->MeanLogPitch.pWrite == This->MeanLogPitch.pEnd) {
            This->MeanLogPitch.pWrite = This->MeanLogPitch.pBeg;
        }
        CheckEndVoicedFlag(This);
    }
}

// Calculates energy slope values and primes their history on the first window.
// ProcessPitchCombiner calls this once enough input frames have been queued.
static void NormE_Processing(PitchCombiner *This)
{
    f32 *pRead = This->pNormE_CalcReadPointer;
    f32 *pCoeffE = This->pCoeffE;
    f32 DeltaNormE = 0.0f;
    u8 i;

    for (i = 0; i < This->NbrOfCoeffE; i++) {
        DeltaNormE += *pRead++ * *pCoeffE++;
        if (pRead == This->NormE.pEnd) {
            pRead = This->NormE.pBeg;
        }
    }
    DeltaNormE /= This->DenominatorEnergy;
    *This->DeltaNormE.pWrite++ = DeltaNormE;
    if (This->DeltaNormE.pWrite == This->DeltaNormE.pEnd) {
        This->DeltaNormE.pWrite = This->DeltaNormE.pBeg;
    }

    This->pNormE_CalcReadPointer++;
    if (This->pNormE_CalcReadPointer == This->NormE.pEnd) {
        This->pNormE_CalcReadPointer = This->NormE.pBeg;
    }

    if (This->VeryFirstTime != 0) {
        f32 ValueFirstWrittenField;
        f32 *pDeltaNormE = This->DeltaNormE.pBeg;

        if (This->DeltaNormE.pWrite == This->DeltaNormE.pBeg) {
            ValueFirstWrittenField = This->DeltaNormE.pEnd[-1];
        } else {
            ValueFirstWrittenField = This->DeltaNormE.pWrite[-1];
        }
        for (i = 0; i < This->NbrOfCoeffE / 2; i++) {
            *pDeltaNormE++ = ValueFirstWrittenField;
        }
        This->VeryFirstTime = 0;
    }
}

// Advances every output cursor after one feature frame has been consumed.
// ProcessPitchCombiner and FlushPitchCombiner call this after queueing output.
static void Increment_Normal_ReadPointers(PitchCombiner *This)
{
    This->LogPitch.pRead++;
    if (This->LogPitch.pRead == This->LogPitch.pEnd) {
        This->LogPitch.pRead = This->LogPitch.pBeg;
    }
    This->MeanLogPitch.pRead++;
    if (This->MeanLogPitch.pRead == This->MeanLogPitch.pEnd) {
        This->MeanLogPitch.pRead = This->MeanLogPitch.pBeg;
    }
    This->DeltaLogPitch.pRead++;
    if (This->DeltaLogPitch.pRead == This->DeltaLogPitch.pEnd) {
        This->DeltaLogPitch.pRead = This->DeltaLogPitch.pBeg;
    }
    This->NormE.pRead++;
    if (This->NormE.pRead == This->NormE.pEnd) {
        This->NormE.pRead = This->NormE.pBeg;
    }
    This->DeltaNormE.pRead++;
    if (This->DeltaNormE.pRead == This->DeltaNormE.pEnd) {
        This->DeltaNormE.pRead = This->DeltaNormE.pBeg;
    }
    This->VUV.pRead++;
    if (This->VUV.pRead == This->VUV.pEnd) {
        This->VUV.pRead = This->VUV.pBeg;
    }
}

/* Writes the five delayed features, then advances their history cursors. */
static void OutputProcedure(PitchCombiner *This)
{
    f32 *pOut;

    This->NormPitch = *This->LogPitch.pRead - *This->MeanLogPitch.pRead;
    pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    *pOut++ = 32.0f * This->NormPitch;
    *pOut++ = 53.0f * *This->DeltaLogPitch.pRead;
    *pOut++ = 1.107 * *This->NormE.pRead;
    *pOut++ = 0.832 * *This->DeltaNormE.pRead;
    if (*This->VUV.pRead == 1.0f) {
        *pOut = This->Voiced_OutputValue;
    } else {
        *pOut = This->Unvoiced_OutputValue;
    }

    Increment_Normal_ReadPointers(This);
}

// Pads pending feature histories and emits at most one delayed frame per call after input ends.
// ProcessPitchCombiner calls this on an empty input while flushing is active.
static u8 FlushPitchCombiner(PitchCombiner *This)
{
    if (This->NbrInBuffer != 0) {
        f32 *pLastWritten;
        f32 *pDeltaNormE;
        u16 i;
        u8 NbrOfCopiesLeftToBeMade;

        if (This->Last_DeltaAndMeanLogPitchValues_ToWrite != 0) {
            /* Pitch results extend when at least a window minus one frames remain. */
            if (This->NbrInBuffer >= (u8) (This->NbrOfCoeff - 1)) {
                NbrOfCopiesLeftToBeMade = This->NbrOfCoeff / 2;
                pLastWritten = This->DeltaLogPitch.pWrite - 1;
                if (pLastWritten < This->DeltaLogPitch.pBeg) {
                    pLastWritten = This->DeltaLogPitch.pEnd - 1;
                }
                for (i = 0; i < NbrOfCopiesLeftToBeMade; i++) {
                    *This->DeltaLogPitch.pWrite++ = *pLastWritten;
                    if (This->DeltaLogPitch.pWrite == This->DeltaLogPitch.pEnd) {
                        This->DeltaLogPitch.pWrite = This->DeltaLogPitch.pBeg;
                    }
                }

                pLastWritten = This->MeanLogPitch.pWrite - 1;
                if (pLastWritten < This->MeanLogPitch.pBeg) {
                    pLastWritten = This->MeanLogPitch.pEnd - 1;
                }
                for (i = 0; i < NbrOfCopiesLeftToBeMade; i++) {
                    *This->MeanLogPitch.pWrite++ = *pLastWritten;
                    if (This->MeanLogPitch.pWrite == This->MeanLogPitch.pEnd) {
                        This->MeanLogPitch.pWrite = This->MeanLogPitch.pBeg;
                    }
                }
            }
            This->Last_DeltaAndMeanLogPitchValues_ToWrite = 0;
        }

        if (This->LastDeltaNormEsToWrite != 0) {
            pLastWritten = This->DeltaNormE.pWrite - 1;
            pDeltaNormE = This->DeltaNormE.pWrite;
            NbrOfCopiesLeftToBeMade = This->NbrOfCoeffE / 2;
            if (pLastWritten < This->DeltaNormE.pBeg) {
                pLastWritten = This->DeltaNormE.pEnd - 1;
            }
            for (i = 0; i < NbrOfCopiesLeftToBeMade; i++) {
                *pDeltaNormE++ = *pLastWritten;
                if (pDeltaNormE == This->DeltaNormE.pEnd) {
                    pDeltaNormE = This->DeltaNormE.pBeg;
                }
            }
            This->LastDeltaNormEsToWrite = 0;
        }

        OutputProcedure(This);
        This->NbrInBuffer--;
        if (This->NbrInBuffer != 0) {
            return 1;
        }
    }

    This->FlushFlag = 0;
    return 0;
}

/* Reads the selected ACNE frame, dynamic-programming pitch and SHS voicing byte. */
static void ReadInputs(PitchCombiner *This, void *pInputs)
{
    f32 *pIn;
    u8 *pIn_VUV;

    /* Analyzes nLogPower bands after the leading input float. */
    pIn = ((void **) pInputs)[2];
    *This->NormE.pWrite++ = MaxFloatInArray(pIn + 1, This->nLogPower);
    if (This->NormE.pWrite == This->NormE.pEnd) {
        This->NormE.pWrite = This->NormE.pBeg;
    }

    pIn = ((void **) pInputs)[0];
    *This->LogPitch.pWrite++ = logf_check(*pIn);
    if (This->LogPitch.pWrite == This->LogPitch.pEnd) {
        This->LogPitch.pWrite = This->LogPitch.pBeg;
    }

    pIn_VUV = ((void **) pInputs)[1];
    *This->VUV.pWrite = *pIn_VUV;
}

/*
 * Emits features from pitch, voicing and spectrum frames.
 * The scheduler presents a float pitch sample on port 0, a voicing byte on
 * port 1 and one leading float followed by nLogPower band floats on port 2.
 * A null port 0 marks a scheduler call with no new frame.
 */
static void ProcessPitchCombiner(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    PitchCombiner *This = (PitchCombiner *) _This;

    if (((void **)pInputs)[0] != NULL) {
        ReadInputs(This, pInputs);
        This->NbrInBuffer++;
        StatusProcessing(This);

        if (This->NbrInBuffer >= This->NbrOfCoeff) {
            LogPitchProcessing(This);
        }
        if (This->VUV_Status == 0) {
            This->pVoicedLogPitch = This->LogPitch.pWrite;
        } else if (This->Increment_pVoicedLogPitch == 1) {
            This->pVoicedLogPitch++;
            if (This->pVoicedLogPitch == This->LogPitch.pEnd) {
                This->pVoicedLogPitch = This->LogPitch.pBeg;
            }
            This->Increment_pVoicedLogPitch = 0;
        }

        if (This->NbrInBuffer >= This->NbrOfCoeffE) {
            NormE_Processing(This);
        }

        if (This->NbrInBuffer == This->FrameQuota) {
            OutputProcedure(This);
            This->NbrInBuffer--;
        }
    } else if (This->FlushFlag == 1) {
        FlushPitchCombiner(This);
    }
}

// Clears histories and frame state when the TOS block receives its reset command.
// The cached LastCalculated_DeltaLogPitch is left unchanged.
static void PitchCombinerReset(PitchCombiner *This)
{
    InitBuffer(&This->VUV, This->FrameQuota);
    InitBuffer(&This->NormE, This->FrameQuota);
    InitBuffer(&This->LogPitch, This->FrameQuota);
    InitBuffer(&This->MeanLogPitch, This->FrameQuota);
    InitBuffer(&This->DeltaLogPitch, This->FrameQuota);
    InitBuffer(&This->DeltaNormE, This->FrameQuota);

    This->MeanLogPitch.pWrite = This->MeanLogPitch.pBeg + This->NbrOfCoeff / 2;
    This->DeltaLogPitch.pWrite = This->DeltaLogPitch.pBeg + This->NbrOfCoeff / 2;
    This->DeltaNormE.pWrite = This->DeltaNormE.pBeg + This->NbrOfCoeffE / 2;
    This->pVoicedLogPitch = This->LogPitch.pRead;
    This->pNormE_CalcReadPointer = This->NormE.pRead;

    This->NormPitch = 0.0f;
    This->LastCalculated_MeanLogPitch = 0.0f;
    This->VoicedCount = 0;
    This->VUV_Status = 0;
    This->PrevVUV_Status = 0;
    This->NbrInBuffer = 0;
    This->BeginVoicedFlag = 0;
    This->EndVoicedFlag = 0;
    This->VeryFirstTime = 1;
    This->FlushFlag = 0;
    This->Last_DeltaAndMeanLogPitchValues_ToWrite = 1;
    This->LastDeltaNormEsToWrite = 1;
    This->Increment_pVoicedLogPitch = 0;
}

/* Handles reset, flush and destruction commands encoded in the low byte. */
static u8 ControlPitchCombiner(struct _BLOCK *_This, u8 ControlId, u32 CParam,
                               u32 DParam)
{
    PitchCombiner *This = (PitchCombiner *) _This;

    switch (ControlId) {
    case 0xC8:
        PitchCombinerReset(This);
        break;
    case 0xC9:
        This->FlushFlag = 1;
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->NormE.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->LogPitch.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->VUV.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->DeltaLogPitch.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->MeanLogPitch.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->DeltaNormE.pBeg);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pCoeff);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pCoeffE);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

// Allocates histories and sets window coefficients during speech graph initialization.
// The graph invokes this through the initializer registered by ConstructPitchCombiner.
static unsigned char InitPitchCombiner(struct _BLOCK *_This)
{
    PitchCombiner *This = (PitchCombiner *) _This;
    struct _TOS *pTos = _This->Object.pTos;
    u16 nLogPower;

    nLogPower = _tosGetProfileU32(&This->BaseBlock.Object, 1, 21);
    This->nLogPower = nLogPower;
    This->LastCalculated_MeanLogPitch = 0.0f;
    This->NbrOfOutputs = 5;
    This->BaseBlock.InputQueues[0].Size = sizeof(f32);
    This->BaseBlock.InputQueues[1].Size = sizeof(u8);
    This->BaseBlock.InputQueues[2].Size = (nLogPower + 1) * sizeof(f32);
    This->BaseBlock.OutputQueues->Size = This->NbrOfOutputs * sizeof(f32);

    This->NbrOfCoeff = 7;
    This->NbrOfCoeffE = 5;
    This->FrameQuota = This->NbrOfCoeff > This->NbrOfCoeffE ? This->NbrOfCoeff : This->NbrOfCoeffE;

    This->NormE.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->LogPitch.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->VUV.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->DeltaLogPitch.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->MeanLogPitch.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->DeltaNormE.pBeg = heap_Calloc(pTos->hHeap, This->FrameQuota, sizeof(f32));
    This->pCoeff = heap_Calloc(pTos->hHeap, This->NbrOfCoeff, sizeof(f32));
    This->pCoeffE = heap_Calloc(pTos->hHeap, This->NbrOfCoeffE, sizeof(f32));

    if (This->NormE.pBeg == NULL || This->LogPitch.pBeg == NULL || This->VUV.pBeg == NULL ||
        This->DeltaLogPitch.pBeg == NULL || This->MeanLogPitch.pBeg == NULL ||
        This->DeltaNormE.pBeg == NULL || This->pCoeff == NULL || This->pCoeffE == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }

    InitBuffer(&This->VUV, This->FrameQuota);
    InitBuffer(&This->NormE, This->FrameQuota);
    InitBuffer(&This->LogPitch, This->FrameQuota);
    InitBuffer(&This->MeanLogPitch, This->FrameQuota);
    InitBuffer(&This->DeltaLogPitch, This->FrameQuota);
    InitBuffer(&This->DeltaNormE, This->FrameQuota);

    This->MeanLogPitch.pWrite = This->MeanLogPitch.pBeg + This->NbrOfCoeff / 2;
    This->DeltaLogPitch.pWrite = This->DeltaLogPitch.pBeg + This->NbrOfCoeff / 2;
    This->DeltaNormE.pWrite = This->DeltaNormE.pBeg + This->NbrOfCoeffE / 2;
    This->pVoicedLogPitch = This->LogPitch.pRead;
    This->pNormE_CalcReadPointer = This->NormE.pRead;
    This->NormPitch = 0.0f;

    This->pCoeff[0] = -3.0f;
    This->pCoeff[1] = -2.0f;
    This->pCoeff[2] = -1.0f;
    This->pCoeff[3] = 0.0f;
    This->pCoeff[4] = 1.0f;
    This->pCoeff[5] = 2.0f;
    This->pCoeff[6] = 3.0f;
    This->DenominatorPitch = 28;

    This->pCoeffE[0] = -2.0f;
    This->pCoeffE[1] = -1.0f;
    This->pCoeffE[2] = 0.0f;
    This->pCoeffE[3] = 1.0f;
    This->pCoeffE[4] = 2.0f;
    This->DenominatorEnergy = 10;

    This->VoicedCount = 0;
    This->VUV_Status = 0;
    This->PrevVUV_Status = 0;
    This->NbrInBuffer = 0;
    This->BeginVoicedFlag = 0;
    This->EndVoicedFlag = 0;
    This->VeryFirstTime = 1;
    This->FlushFlag = 0;
    This->Last_DeltaAndMeanLogPitchValues_ToWrite = 1;
    This->LastDeltaNormEsToWrite = 1;
    This->Increment_pVoicedLogPitch = 0;

    This->Voiced_OutputValue = _tosGetProfileFloat(&This->BaseBlock.Object, 2, 0.0f);
    This->Unvoiced_OutputValue = _tosGetProfileFloat(&This->BaseBlock.Object, 3, 100000.0f);
    return 0;
}

// Creates a speech feature block with pitch, voicing and spectrum inputs and one output.
// Speech graph construction uses this to register the processing and control callbacks.
struct _BLOCK *ConstructPitchCombiner(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 3, 1, ProcessPitchCombiner, InitPitchCombiner,
                                 ControlPitchCombiner,
                                 sizeof(PitchCombiner));
}
