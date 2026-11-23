/* Decimation filter and sample history for the voicing detector. */
#include "gssdk/heap.h"
#include "gssdk/triggerlr.h"

#include <stdlib.h>

/* Filters the input into pSubSampledSignal and returns zero. */
int Subsampler_Process(TriggerLR *This, s16 *pInSamples)
{
    if (pInSamples != NULL) {
        s16 *pInSamples_End = pInSamples + This->NrInSamples;
        s16 *pOutSamples = This->pSubSampledSignal;
        s16 *pSample;

        for (pSample = pInSamples; pSample < pInSamples_End; pSample++) {
            s16 inputSample = *pSample;

            if (This->SubSampState == SUBSAMP_STATE_ODD) {
                *This->pOddSamplesOldest++ = inputSample << This->centerTabShift;
                if (This->pOddSamplesOldest >= This->pOddSamplesEnd) {
                    This->pOddSamplesOldest = This->pOddSamplesBase;
                }

                This->pOddSamplesCenter++;
                if (This->pOddSamplesCenter >= This->pOddSamplesEnd) {
                    This->pOddSamplesCenter = This->pOddSamplesBase;
                }
                This->SubSampState = SUBSAMP_STATE_EVEN;
            } else {
                s32 *pLower = *This->ppEvenSamplesOldest;
                s32 *pUpper = pLower + This->Neven;
                s32 *pFilter = This->pFilterResponse;
                s32 sum = 0;
                s32 **ppLong;
                u32 index;

                while (pFilter < This->pFilterEnd) {
                    s32 Sample = inputSample * *pFilter++;

                    *--pUpper = Sample;
                    *pLower++ = Sample;
                }

                This->ppEvenSamplesOldest++;
                if (This->ppEvenSamplesOldest >= This->ppEvenSamplesEnd) {
                    This->ppEvenSamplesOldest = This->ppEvenSamplesBase;
                }

                ppLong = This->ppEvenSamplesOldest;
                for (index = 0; index < This->Neven; index++) {
                    sum += (*ppLong)[index];
                    ppLong++;
                    if (ppLong >= This->ppEvenSamplesEnd) {
                        ppLong = This->ppEvenSamplesBase;
                    }
                }

                sum += *This->pOddSamplesCenter;
                *pOutSamples++ = (s16)(sum >> This->nrBitsInTabCoeff);
                This->SubSampState = SUBSAMP_STATE_ODD;
            }
        }
    }
    return 0;
}

/* Checks the length and symmetry of the half-band filter response. */
static int Subsampler_CheckImpulseResponse(TriggerLR *This, const f64 *pImpResp, unsigned int N)
{
    const f64 tolerance = 1e-15;
    const f64 *pTop;
    const f64 *pBottom;
    s32 bRetVal = 0;

    if ((N & 1) == 0) {
        bRetVal = 1;
        goto done;
    }

    This->Nodd = N >> 1;
    This->Neven = This->Nodd + 1;
    if ((This->Nodd & 1) == 0) {
        bRetVal = 1;
        goto done;
    }
    if ((This->Neven & 1) == 1) {
        bRetVal = 1;
        goto done;
    }

    pTop = pImpResp;
    pBottom = pImpResp + N - 1;
    while (pTop < pBottom) {
        if (abs(*pTop - *pBottom) > tolerance) {
            bRetVal = 1;
            goto done;
        }
        pTop++;
        pBottom--;
    }

    if (abs(*pTop - 0.5) > tolerance) {
        bRetVal = 1;
        goto done;
    }

    pTop = pImpResp + 1;
    while (pTop < pBottom) {
        if (abs(*pTop) > tolerance) {
            bRetVal = 1;
            goto done;
        }
        pTop += 2;
    }

done:
    if (bRetVal != 0) {
        _tosErrorLog(&This->BaseBlock.Object, 300);
    }
    return bRetVal;
}

/* Clears the filter histories and resets their read and write positions. */
void Subsampler_Reset(TriggerLR *This)
{
    s32 *pLong;
    s32 **ppLong;

    for (pLong = This->pOddSamplesBase; pLong < This->pOddSamplesEnd; pLong++) {
        *pLong = 0;
    }
    This->pOddSamplesOldest = This->pOddSamplesBase;
    This->pOddSamplesCenter = This->pOddSamplesOldest + This->Nodd / 2;

    for (ppLong = This->ppEvenSamplesBase; ppLong < This->ppEvenSamplesEnd; ppLong++) {
        for (pLong = *ppLong; pLong < *ppLong + This->Neven; pLong++) {
            *pLong = 0;
        }
    }
    This->ppEvenSamplesOldest = This->ppEvenSamplesBase;
    This->SubSampState = SUBSAMP_STATE_EVEN;
}

/* Initializes the trigger's subsampler and filter coefficients. */
int Subsampler_Init(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    s32 bRetVal = 0;
    f64 ImpulseResp[23] = {
        -0.0044691990096899963, -2.8698779710312522e-16,
        0.011551957698517620, 3.9956647417786528e-16,
        -0.024138391579451441, -5.5259703582695518e-16,
        0.046702085726844770, 6.5110922026792407e-16,
        -0.095170806790360677, -7.6770895142885015e-16,
        0.31453233358223820, 0.50000000000000078,
        0.31453233358223820, -7.6770895142885015e-16,
        -0.095170806790360677, 6.5110922026792407e-16,
        0.046702085726844770, -5.5259703582695518e-16,
        -0.024138391579451441, 3.9956647417786528e-16,
        0.011551957698517620, -2.8698779710312522e-16,
        -0.0044691990096899963,
    };
    s32 **ppLong;
    f64 Factor;
    s32 *pIntegerResp;
    f64 *pDoubleResp;

    This->N = 23;
    if (Subsampler_CheckImpulseResponse(This, ImpulseResp, This->N) != 0) {
        bRetVal = 1;
        goto done;
    }

    This->NrInSamples = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 30, 110);
    This->pSubSampledSignal = heap_Calloc(pTos->hHeap, This->frameSize, sizeof(s16));
    This->nrBitsInTabCoeff = 12;
    This->pOddSamplesOldest = NULL;
    This->pOddSamplesCenter = NULL;
    This->ppEvenSamplesOldest = NULL;
    This->ppEvenSamplesEnd = NULL;

    This->pOddSamplesBase = heap_Calloc(pTos->hHeap, This->Nodd, sizeof(s32));
    This->pOddSamplesEnd = This->pOddSamplesBase + This->Nodd;

    This->ppEvenSamplesBase = heap_Calloc(pTos->hHeap, This->Neven, sizeof(s32 *));
    This->ppEvenSamplesEnd = This->ppEvenSamplesBase + This->Neven;

    if (This->pSubSampledSignal == NULL || This->pOddSamplesBase == NULL ||
        This->ppEvenSamplesBase == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        bRetVal = 1;
        goto done;
    }

    ppLong = This->ppEvenSamplesBase;
    while (ppLong < This->ppEvenSamplesEnd) {
        *ppLong = heap_Calloc(pTos->hHeap, This->Neven, sizeof(s32));
        /* The check is on the table cursor. */
        if (ppLong == NULL) {
            _tosErrorLog(&This->BaseBlock.Object, 2);
            bRetVal = 1;
            goto done;
        }
        ppLong++;
    }

    This->pFilterResponse = heap_Calloc(pTos->hHeap, This->Neven / 2, sizeof(s32));
    This->pFilterEnd = This->pFilterResponse + This->Neven / 2;
    if (This->pFilterResponse == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        bRetVal = 1;
        goto done;
    }

    Factor = 1 << This->nrBitsInTabCoeff;
    pDoubleResp = ImpulseResp;
    pIntegerResp = This->pFilterResponse;
    while (pIntegerResp < This->pFilterEnd) {
        *pIntegerResp++ = (s32)(*pDoubleResp * Factor);
        pDoubleResp += 2;
    }
    This->centerTabShift = This->nrBitsInTabCoeff - 1;
    Subsampler_Reset(This);

done:
    return bRetVal;
}

/* Frees the filter coefficients and the odd and even sample buffers. */
void Subsampler_Free(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    s32 **ppLong;

    if (This->pFilterResponse != NULL) {
        heap_Free(pTos->hHeap, This->pFilterResponse);
    }
    if (This->pOddSamplesBase != NULL) {
        heap_Free(pTos->hHeap, This->pOddSamplesBase);
    }
    for (ppLong = This->ppEvenSamplesBase; ppLong < This->ppEvenSamplesEnd; ppLong++) {
        heap_Free(pTos->hHeap, *ppLong);
    }
    if (This->ppEvenSamplesBase != NULL) {
        heap_Free(pTos->hHeap, This->ppEvenSamplesBase);
    }
    if (This->pSubSampledSignal != NULL) {
        heap_Free(pTos->hHeap, This->pSubSampledSignal);
    }
}
