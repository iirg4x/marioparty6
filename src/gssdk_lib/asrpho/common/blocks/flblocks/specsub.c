/* Estimates noise and suppresses it by spectral subtraction. */
#include "gssdk/blocks.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/mathusage.h"
#include "gssdk/heap.h"
#include "types.h"

#include <stddef.h>
#include <string.h>

#include "gssdk/mqueue.h"

/* Spectral subtraction state with noise estimates, SNR history and a frame FIFO. */
typedef struct SpecSub {
    struct _BLOCK BaseBlock;    /* TOS callbacks, profile and queues. */
    u16 DelayCtr;              /* Non-speech frame count since the latest speech frame. */
    u16 NoiseUpdateCtr;        /* Count used while updating the noise estimates. */
    f32 *pNoiseSpec;           /* Noise power estimated for each bin. */
    f32 *pLogNoiseSpec;        /* Logarithmic form of the noise estimate. */
    f32 *pSnrPrio;             /* Prior SNR estimate for each bin. */
    f32 *pSnrPost;             /* Posterior SNR estimate for each bin. */
    f32 *pFifoBufStart;        /* First bin of the buffered spectra. */
    f32 *pFifoBufEnd;          /* End of the buffered spectra. */
    f32 *pFifoBufWr;           /* Next FIFO frame to write. */
    f32 *pFifoBufRd;           /* Next FIFO frame to read. */
    u16 Delay;                 /* Number of frames between the FIFO read and write positions. */
    f32 *Gain;                 /* Per-bin suppression gain applied to the current input spectrum. */
    u32 NbOutputBins;          /* Number of bins in a frame. */
    u16 FirstBandEnergy;       /* First bin included in the output energy sum. */
    f32 LambdaSnr;             /* Weight for SNR history. */
    f32 NoiseWindow;           /* Limit on the averaging denominator for noise updates. */
    f32 SnrWindow;             /* The game's code never reads this field. */
    f32 OverEstimate;          /* Multiplier on the noise power in the posterior SNR. */
    f32 GainFloor;             /* Lower bound on the suppression gain. */
    unsigned int Nbr_LR_Bands; /* Number of likelihood-ratio bands. */
    unsigned int *LR_FreqBoundary; /* Bin boundaries for the likelihood-ratio bands. */
    f32 LR_term;                   /* Offset in the likelihood-ratio exponent. */
    f32 LR_factor;                 /* Scale of the maximum mean SNR in the exponent. */
    f32 LR;                /* Recursive likelihood ratio. Its logarithm is the fourth output. */
    f32 InfTest_CritValue; /* Upper bound on the likelihood-ratio exponent. */
    f32 LR_a00;            /* Constant term in the likelihood-history denominator. */
    f32 LR_a01;            /* Constant term in the likelihood-history numerator. */
    f32 LR_a10;            /* Prior likelihood weight in the denominator. */
    f32 LR_a11;            /* Prior likelihood weight in the numerator. */
} SpecSub;

/* Resets the SNR arrays, delay counter and LR value. */
static void Reset(SpecSub *This)
{
    u32 NrBands = This->NbOutputBins;
    f32 *pTmp;

    pTmp = This->pSnrPost;
    while (pTmp < This->pSnrPost + NrBands) {
        *pTmp++ = 1.0f;
    }

    pTmp = This->pSnrPrio;
    while (pTmp < This->pSnrPrio + NrBands) {
        *pTmp++ = 1.0f;
    }

    This->DelayCtr = 0;
    This->LR = 1.0f;
}

/* Restores the noise estimates and delayed spectra from a saved session. */
static void SpecSubPutSession(SpecSub *This, const void *pSpecSubSession)
{
    const f32 *pFlt;
    u32 i;
    u32 spectrumSize;
    const u32 *pDWord = pSpecSubSession;
    const f32 *pIn = (const f32 *) (pDWord + 3);

    spectrumSize = This->NbOutputBins;

    This->DelayCtr = *pDWord;
    This->NoiseUpdateCtr = pDWord[1];
    pFlt = (const f32 *) (pDWord + 2);
    This->LR = *pFlt;

    memcpy(This->pNoiseSpec, pIn, spectrumSize * sizeof(f32));
    pIn += spectrumSize;
    memcpy(This->pSnrPrio, pIn, spectrumSize * sizeof(f32));
    pIn += spectrumSize;
    memcpy(This->pSnrPost, pIn, spectrumSize * sizeof(f32));
    pIn += spectrumSize;
    memcpy(This->Gain, pIn, spectrumSize * sizeof(f32));
    pIn += spectrumSize;

    for (i = 0; i < spectrumSize; i++) {
        This->pLogNoiseSpec[i] = 0.5f * logf_check(This->pNoiseSpec[i]);
    }

    pDWord = (const u32 *) pIn;
    This->pFifoBufWr = This->pFifoBufStart + *pDWord;
    This->pFifoBufRd = This->pFifoBufStart + pDWord[1];
    pIn += 2;
    spectrumSize *= This->Delay + 1;
    memcpy(This->pFifoBufStart, pIn, spectrumSize * sizeof(f32));
}

/* Saves the noise estimates and delayed spectra into the session buffer. */
static void SpecSubGetSession(SpecSub *This, void *pSpecSubSession)
{
    u32 *pFifoOffsets;
    f32 *pFlt;
    u32 arraySize;
    u32 spectrumSize;
    u8 *session = (u8 *) pSpecSubSession + 2 * sizeof(u32) + sizeof(f32);
    u32 *pDWord = pSpecSubSession;

    spectrumSize = This->NbOutputBins;
    arraySize = spectrumSize * sizeof(f32);

    *pDWord = This->DelayCtr;
    pDWord[1] = This->NoiseUpdateCtr;
    pFlt = (f32 *) (pDWord + 2);
    *pFlt = This->LR;
    memcpy(session, This->pNoiseSpec, arraySize);
    session += arraySize;
    memcpy(session, This->pSnrPrio, arraySize);
    session += arraySize;
    memcpy(session, This->pSnrPost, arraySize);
    session += arraySize;
    memcpy(session, This->Gain, arraySize);
    session += arraySize;

    pFifoOffsets = (u32 *) session;
    pFifoOffsets[0] = This->pFifoBufWr - This->pFifoBufStart;
    pFifoOffsets[1] = This->pFifoBufRd - This->pFifoBufStart;
    session += 2 * sizeof(u32);
    spectrumSize *= This->Delay + 1;
    memcpy(session, This->pFifoBufStart, spectrumSize * sizeof(f32));
}

/* Estimates noise and applies spectral gain to suppress it. */
static void SpectralSubtraction(SpecSub *This, u8 FrameType, f32 *pInSpec, f32 *pOutSpec,
                                f32 *pEnergyOut)
{
    f32 *pIn;
    f32 *pNs;
    f32 *pGain;
    f32 *pOut;
    f32 *pPri;
    f32 *pPst;
    f32 *pEnd;
    u32 NrBands = This->NbOutputBins;
    f32 *BufWr = This->pFifoBufWr;
    f32 *BufEnd = This->pFifoBufEnd;
    f32 *BufSpIn = This->pFifoBufStart;
    unsigned int Delay = This->Delay;
    unsigned int NoiseUpdateCtr = This->NoiseUpdateCtr;
    unsigned int DelayCtr = This->DelayCtr;
    f32 NoiseWindow = This->NoiseWindow;
    f32 *SpecNs = This->pNoiseSpec;
    f32 LambdaSnr = This->LambdaSnr;
    f32 EnergyOut = 0.0f;
    f32 *pLogNoiseSpec; /* Logarithmic form of the noise estimate. */

    memcpy(BufWr, pInSpec, NrBands * sizeof(f32));
    BufWr += NrBands;
    if (BufWr == BufEnd) {
        BufWr = BufSpIn;
    }

    if (FrameType) {
        DelayCtr = 0;
    } else {
        if (!NoiseUpdateCtr) {
            pLogNoiseSpec = This->pLogNoiseSpec;
            pIn = pInSpec;
            pNs = SpecNs;
            while (pNs < SpecNs + NrBands) {
                *pNs = *pIn;
                *pLogNoiseSpec = 0.5f * logf_check(*pIn);
                pIn++;
                pNs++;
                pLogNoiseSpec++;
            }
        }
        if (DelayCtr <= Delay) {
            DelayCtr++;
        }
        if (DelayCtr == Delay + 1) {
            f32 oneminlambda;
            f32 diff;
            f32 *BufRd;

            if (NoiseUpdateCtr >= NoiseWindow) {
                oneminlambda = 1.0f / NoiseWindow;
            } else {
                NoiseUpdateCtr++;
                oneminlambda = 1.0f / NoiseUpdateCtr;
            }
            BufRd = BufWr;
            pLogNoiseSpec = This->pLogNoiseSpec;
            pNs = SpecNs;
            while (pNs < SpecNs + NrBands) {
                diff = *BufRd++ - *pNs;
                if (diff > *pNs) {
                    diff = *pNs;
                }
                *pNs += oneminlambda * diff;
                *pLogNoiseSpec = 0.5f * logf_check(*pNs);
                pNs++;
                pLogNoiseSpec++;
            }
        }
    }

    if (NoiseUpdateCtr) {
        f32 OneMinLambdaSnr;
        f32 OverEstimate;

        OneMinLambdaSnr = 1.0f - LambdaSnr;
        OverEstimate = This->OverEstimate;
        pGain = This->Gain;
        pPst = This->pSnrPost;
        pPri = This->pSnrPrio;
        pEnd = pPri + NrBands;
        while (pPri < pEnd) {
            *pPri = LambdaSnr * (1.0f + *pPst * *pGain);
            pPri++;
            pPst++;
            pGain++;
        }
        pIn = pInSpec;
        pNs = SpecNs;
        pPst = This->pSnrPost;
        pEnd = pPst + NrBands;
        while (pPst < pEnd) {
            *pPst = (0.000001f + *pIn) / (OverEstimate * *pNs + 0.000001f);
            pPst++;
            pIn++;
            pNs++;
        }
        pPst = This->pSnrPost;
        pPri = This->pSnrPrio;
        pEnd = pPri + NrBands;
        while (pPri < pEnd) {
            *pPri++ += OneMinLambdaSnr * *pPst++;
        }
    }
    {
        f32 *pSNR;
        f32 MaxMeanSNR;
        f32 theLR;
        f32 CandidateArgument;
        unsigned int i;
        f32 *pSNR_End;
        f32 SumSNR;
        f32 MeanSNR;
        unsigned int FreqCounter;

        MaxMeanSNR = 0.0f;
        pSNR = This->pSnrPost;
        for (i = 0; i < This->Nbr_LR_Bands; i++) {
            pSNR_End = This->pSnrPost + This->LR_FreqBoundary[i];
            SumSNR = 0.0f;
            FreqCounter = 0;
            while (pSNR < pSNR_End) {
                SumSNR += *pSNR++;
                FreqCounter++;
            }
            MeanSNR = SumSNR / FreqCounter;
            if (MeanSNR > MaxMeanSNR) {
                MaxMeanSNR = MeanSNR;
            }
        }
        CandidateArgument = This->LR_term + MaxMeanSNR * This->LR_factor;
        if (CandidateArgument > This->InfTest_CritValue) {
            CandidateArgument = This->InfTest_CritValue;
        }
        CandidateArgument = expf(CandidateArgument);
        theLR = This->LR;
        This->LR = CandidateArgument *
                   ((This->LR_a01 + This->LR_a11 * theLR) / (This->LR_a00 + This->LR_a10 * theLR));
    }

    if (!NoiseUpdateCtr) {
        pGain = This->Gain;
        pEnd = pGain + NrBands;
        while (pGain < pEnd) {
            *pGain++ = 1.0f;
        }
    } else {
        f32 GainFloor = This->GainFloor;
        f32 Tmp;
        pPri = This->pSnrPrio;
        pEnd = pPri + NrBands;
        pGain = This->Gain;
        while (pPri < pEnd) {
            if (*pPri > 1.0f) {
                Tmp = 1.0f - 1.0f / *pPri;
            } else {
                Tmp = 0.0f;
            }
            Tmp *= Tmp;
            *pGain++ = Tmp > GainFloor ? Tmp : GainFloor;
            pPri++;
        }
    }

    pOut = pOutSpec;
    pEnd = pOut + This->FirstBandEnergy;
    pGain = This->Gain;
    pIn = pInSpec;
    while (pOut < pEnd) {
        *pOut = 0.5f * logf_check(*pGain++ * *pIn++);
        pOut++;
    }
    pEnd = pOutSpec + NrBands;
    while (pOut < pEnd) {
        f32 tmp = *pGain++ * *pIn++;
        EnergyOut += tmp;
        *pOut = 0.5f * logf_check(tmp);
        pOut++;
    }
    *pEnergyOut = logf_check(EnergyOut);

    This->Delay = Delay;
    This->NoiseUpdateCtr = NoiseUpdateCtr;
    This->DelayCtr = DelayCtr;
    This->pFifoBufWr = BufWr;
}

/* Publishes the filtered spectrum, energy, noise estimate and log likelihood ratio. */
static void ProcessSpecSub(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    SpecSub *This = (SpecSub *) _This;
    f32 *pSpecIn;
    f32 *pSpecOut;
    f32 *pNoiseSpecOut;
    f32 *pEnergyOut;
    f32 *pLogLR;
    u8 *pFrameType;

    if (((void **) pInputs)[0] == NULL) {
        return;
    }
    pSpecIn = ((void **) pInputs)[0];

    pFrameType = ((void **) pInputs)[1];

    pEnergyOut = qEnQueueOne(This->BaseBlock.OutputQueues[0].pQ);
    pSpecOut = qEnQueueOne(This->BaseBlock.OutputQueues[1].pQ);
    pNoiseSpecOut = qEnQueueOne(This->BaseBlock.OutputQueues[2].pQ);
    pLogLR = qEnQueueOne(This->BaseBlock.OutputQueues[3].pQ);

    SpectralSubtraction(This, *pFrameType, pSpecIn, pSpecOut, pEnergyOut);
    memcpy(pNoiseSpecOut, This->pLogNoiseSpec, This->NbOutputBins * sizeof(f32));
    *pLogLR = logf_check(This->LR);
}

/* Reinitializes the spectral subtraction state. */
static void ResetSpecSub(SpecSub *This)
{
    f32 *pTmp;
    u32 NrBands = This->NbOutputBins;

    pTmp = This->pNoiseSpec;
    while (pTmp < This->pNoiseSpec + NrBands + This->NbOutputBins) {
        *pTmp++ = 1.0f;
    }
    This->NoiseUpdateCtr = 0;
    Reset(This);
}

/* Handles queue controls, saved state and teardown for spectral subtraction. */
static u8 ControlSpecSub(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    SpecSub *This = (SpecSub *) _This;
    u8 RetVal = 1;

    switch (ControlId) {
    case 2:
        if (CParam != 0) {
            SpecSubPutSession(This, (const u8 *) CParam + *(u32 *) DParam);
            *(u32 *) DParam += (((This->Delay + 5) * This->NbOutputBins) + 5) * sizeof(u32);
        } else {
            ResetSpecSub(This);
        }
        break;
    case 1:
        if (CParam != 0) {
            SpecSubGetSession(This, (u8 *) CParam + *(u32 *) DParam);
        }
        *(u32 *) DParam += (((This->Delay + 5) * This->NbOutputBins) + 5) * sizeof(u32);
        break;
    case 3:
        RetVal = qQueueControl(_This->InputQueues[1].pQ, 5, _This->InputQueues[1].ReadPos, 0);
        break;
    case 4:
        RetVal = qQueueControl(_This->InputQueues[1].pQ, 6, _This->InputQueues[1].ReadPos, 0);
        break;
    case 0xFF:
        heap_Free(_This->Object.pTos->hHeap, This->pFifoBufStart);
        heap_Free(_This->Object.pTos->hHeap, This->LR_FreqBoundary);
        tosBaseBlockDestruct(_This);
        break;
    default:
        return 0;
    }
    return RetVal;
}

/* Initializes spectral subtraction from the block's profile. */
static u8 InitSpecSub(struct _BLOCK *_This)
{
    SpecSub *This = (SpecSub *) _This;
    struct _TOS *pTos = _This->Object.pTos;
    u32 NbOutputBins;
    u32 arraySize;
    f32 R1;
    f32 R2;
    f32 R;
    unsigned int i;
    unsigned int Candidate;

    NbOutputBins = This->NbOutputBins = _tosGetProfileU32(&This->BaseBlock.Object, 0x08, 21);
    arraySize = NbOutputBins * sizeof(f32);

    _This->InputQueues[0].Size = arraySize;
    _This->InputQueues[1].Size = 1;
    _This->OutputQueues[0].Size = sizeof(f32);
    _This->OutputQueues[1].Size = arraySize;
    _This->OutputQueues[2].Size = arraySize;
    _This->OutputQueues[3].Size = sizeof(f32);

    This->FirstBandEnergy = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x06, 1) - 1;
    This->Delay = _tosGetProfileU32(&This->BaseBlock.Object, 0x01, 3);
    This->GainFloor = _tosGetProfileFloat(&This->BaseBlock.Object, 0x02, 0.01f);
    This->OverEstimate = _tosGetProfileFloat(&This->BaseBlock.Object, 0x03, 1.0f);
    This->NoiseWindow = _tosGetProfileFloat(&This->BaseBlock.Object, 0x04, 40.0f);
    This->LambdaSnr = _tosGetProfileFloat(&This->BaseBlock.Object, 0x07, 0.99f);

    This->pFifoBufStart =
        heap_Calloc(pTos->hHeap, NbOutputBins * (This->Delay + 6), sizeof(f32));
    This->pFifoBufWr = This->pFifoBufStart;
    This->pFifoBufEnd = This->pFifoBufStart + NbOutputBins * (This->Delay + 1);
    This->pNoiseSpec = This->pFifoBufEnd;
    This->pLogNoiseSpec = This->pNoiseSpec + NbOutputBins;
    This->pSnrPrio = This->pLogNoiseSpec + NbOutputBins;
    This->pSnrPost = This->pSnrPrio + NbOutputBins;
    This->Gain = This->pSnrPost + NbOutputBins;

    This->LR_a00 = 0.8f;
    This->LR_a01 = 0.2f;
    This->LR_a10 = 0.1f;
    This->LR_a11 = 0.9f;

    R1 = This->LR_a11 / This->LR_a10;
    R2 = This->LR_a01 / This->LR_a00;
    R = R1 > R2 ? R1 : R2;
    This->InfTest_CritValue = logf_check(3.4028e38f / R);
    This->LR = 1.0f;
    This->LR_term = -logf_check(11.0f);
    This->LR_factor = 0.90909094f;

    This->Nbr_LR_Bands = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x1E, 1);
    This->LR_FreqBoundary = heap_Calloc(pTos->hHeap, This->Nbr_LR_Bands, sizeof(u32));

    for (i = 0; i < This->Nbr_LR_Bands; i++) {
        switch (i) {
        case 0:
            Candidate = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x1F, NbOutputBins);
            if (Candidate > NbOutputBins) {
                Candidate = NbOutputBins;
            }
            This->LR_FreqBoundary[0] = Candidate;
            break;
        case 1:
            Candidate = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x20, NbOutputBins);
            if (Candidate > NbOutputBins) {
                Candidate = NbOutputBins;
            }
            This->LR_FreqBoundary[1] = Candidate;
            break;
        case 2:
            Candidate = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x21, NbOutputBins);
            if (Candidate > NbOutputBins) {
                Candidate = NbOutputBins;
            }
            This->LR_FreqBoundary[2] = Candidate;
            break;
        default:
            _tosErrorLog(&This->BaseBlock.Object, 0x67);
            return 1;
        }
    }

    /* The allocation is checked after the derived buffer pointers are set up. */
    if (This->pFifoBufStart == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x66);
    }

    ResetSpecSub(This);
    return 0;
}

/* Constructs a TOS block for spectral subtraction. */
struct _BLOCK *ConstructSpecSub(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 2, 4, ProcessSpecSub, InitSpecSub, ControlSpecSub,
                                 sizeof(SpecSub));
}
