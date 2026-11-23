/* Converts input spectra into mel-frequency bands. */
#include "gssdk/blocks.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

/* Mel filter bank and the warp supplied by control 0x01. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 NbOutputBins;       /* Number of output bands. */
    f32 **Weights;          /* Weights for each band's contiguous run of input bins. */
    u32 *StartIdx;          /* First input bin for each band. */
    u32 *Lengths;           /* Number of weighted input bins in each band. */
    f32 Warp;               /* Frequency warp used when computing the filter weights. */
} MelBlock;

static u8 MelInitBands(MelBlock *This, f32 Warp);

/* Sums the weighted spectrum bins for each mel band. */
static void Mel(MelBlock *This, f32 *pInput, f32 *pOutput)
{
    f32 *Wght;
    f32 *End;
    f32 *Spec;
    unsigned int i;
    for (i = 0; i < This->NbOutputBins; i++, pOutput++) {
        *pOutput = 10.0f;
        Wght = This->Weights[i];
        End = Wght + This->Lengths[i];
        Spec = pInput + This->StartIdx[i];
        while (Wght < End) {
            *pOutput += *Wght++ * *Spec++;
        }
    }
}

/* Applies the mel filters to the input spectrum and queues the resulting bands. */
static void ProcessMel(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    MelBlock *This;
    f32 *pSpecIn;
    f32 *pMelOut;
    This = (MelBlock *) _This;
    if ((pSpecIn = ((void **) pInputs)[0]) != NULL) {
        pMelOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
        Mel(This, pSpecIn, pMelOut);
    }
}

/* Releases the mel band's filter tables. */
static void FreeResourcesMel(MelBlock *This)
{
    struct _TOS *pTos;
    u32 i;

    if (This->Weights != NULL) {
        pTos = This->BaseBlock.Object.pTos;
        for (i = 0; i < This->NbOutputBins; i++) {
            heap_Free(pTos->hHeap, This->Weights[i]);
        }
        heap_Free(pTos->hHeap, This->Weights);
        heap_Free(pTos->hHeap, This->StartIdx);
        heap_Free(pTos->hHeap, This->Lengths);
        This->Weights = NULL;
    }
}

/* Handles control commands for the mel feature block. */
static u8 ControlMel(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    MelBlock *This = (MelBlock *) _This;
    u8 RetVal = 1;

    switch (ControlId) {
    case 0x01:
        if (This->Weights != NULL) {
            FreeResourcesMel(This);
        }
        RetVal = !MelInitBands(This, *(f32 *) &DParam);
        break;

    case 0xFF:
        FreeResourcesMel(This);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;

    default:
        RetVal = 0;
        break;
    }
    return RetVal;
}

/* Builds the mel-band filter tables for the requested frequency warp. */
static u8 MelInitBands(MelBlock *This, f32 Warp)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    f32 *MelAxis;
    f32 f;
    f32 fBreak;
    f32 df;
    f32 MelCentre;
    f32 *MelScan;
    f32 *Wght;
    f32 fNyquist = _tosGetProfileFloat(&This->BaseBlock.Object, 0x02, 4000.0f);
    f32 *fCentre;
    unsigned int FirstBand;
    unsigned int OutBin;
    unsigned int InputSize;
    unsigned int InBin;
    unsigned int Bingo;
    FirstBand = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x05, 0);
    InputSize = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x03, 0);
    This->Warp = Warp;

    MelAxis = heap_Alloc(pTos->hHeap, InputSize * sizeof(f32));
    if (MelAxis == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        return 1;
    }

    df = fNyquist / (InputSize - 1);
    MelScan = MelAxis;
    fBreak = fNyquist > 1000.0f ? 1000.0f : fNyquist;
    for (f = 0.0f; f < fBreak; f += df) {
        *MelScan++ = f / (100.0f * Warp);
    }
    /* The logarithmic section spans the remaining input bins. */
    fBreak = fNyquist + df / 2.0f;
    for (; f < fBreak; f += df) {
        *MelScan++ = 10.0f + 5.0f * (logf_check(f / (1000.0f * Warp)) / 0.693147f);
    }

    if (fNyquist < 1000.0f) {
        This->NbOutputBins = fNyquist / 100.0 - 0.95;
    } else {
        This->NbOutputBins = (10.0f + 5.0f * logf_check(f / 1000.0) / 0.693147f) - 0.95;
    }
    This->NbOutputBins -= FirstBand;

    /* Error returns below leave the input-bin allocation in place. */
    if (This->NbOutputBins != (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x04, 21)) {
        return 1;
    }

    This->Weights = heap_Calloc(pTos->hHeap, This->NbOutputBins, sizeof(f32 *));
    if (This->Weights == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        return 1;
    }

    This->StartIdx = heap_Calloc(pTos->hHeap, This->NbOutputBins, sizeof(u32));
    if (This->StartIdx == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        heap_Free(pTos->hHeap, This->Weights);
        return 1;
    }

    This->Lengths = heap_Calloc(pTos->hHeap, This->NbOutputBins, sizeof(u32));
    if (This->Lengths == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        heap_Free(pTos->hHeap, This->Weights);
        heap_Free(pTos->hHeap, This->StartIdx);
        return 1;
    }

    fCentre = heap_Calloc(pTos->hHeap, This->NbOutputBins, sizeof(f32));
    if (fCentre == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        heap_Free(pTos->hHeap, This->Weights);
        heap_Free(pTos->hHeap, This->StartIdx);
        heap_Free(pTos->hHeap, This->Lengths);
        return 1;
    }

    /* The center frequencies are recorded here but never used to compute the weights. */
    for (OutBin = 0; OutBin < This->NbOutputBins; OutBin++) {
        This->Lengths[OutBin] = 0;
        This->StartIdx[OutBin] = -1;
        MelCentre = 1 + OutBin + FirstBand;
        if (OutBin + FirstBand < 10) {
            fCentre[OutBin] = 100.0f * MelCentre;
        } else {
            fCentre[OutBin] = 1000.0f * powf(2.0f, MelCentre / 5.0f - 2.0);
        }

        for (InBin = 1; InBin < InputSize; InBin++) {
            f32 fMel = MelAxis[InBin];
            Bingo = fMel > MelCentre - 1.0 && fMel < 1.0 + MelCentre;

            This->Lengths[OutBin] += Bingo;
            if (Bingo != 0 && This->StartIdx[OutBin] == -1) {
                This->StartIdx[OutBin] = InBin;
            }
        }

        This->Weights[OutBin] = heap_Calloc(pTos->hHeap, This->Lengths[OutBin], sizeof(f32));
        if (This->Weights[OutBin] == NULL) {
            _tosErrorLog(&This->BaseBlock.Object, 0x64);
            heap_Free(pTos->hHeap, fCentre);
            return 1;
        }

        Wght = This->Weights[OutBin];
        for (InBin = 0; InBin < This->Lengths[OutBin]; Wght++, InBin++) {
            f32 fMel = MelAxis[This->StartIdx[OutBin] + InBin];
            *Wght = fMel < MelCentre ? 1.0f - MelCentre + fMel : 1.0f - fMel + MelCentre;
        }
    }

    heap_Free(pTos->hHeap, fCentre);
    heap_Free(pTos->hHeap, MelAxis);
    return 0;
}

/* Sets the input and output port sizes. */
static u8 InitMel(struct _BLOCK *_This)
{
    MelBlock *This = (MelBlock *) _This;
    u16 NrInputBands = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x03, 0);
    u16 NbOutputBins = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x04, 21);

    This->BaseBlock.InputQueues->Size = NrInputBands * sizeof(f32);
    This->BaseBlock.OutputQueues->Size = NbOutputBins * sizeof(f32);
    return 0;
}

/* Constructs a TOS block for mel filtering. */
struct _BLOCK *ConstructMel(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1, ProcessMel, InitMel, ControlMel,
                                 sizeof(MelBlock));
}
