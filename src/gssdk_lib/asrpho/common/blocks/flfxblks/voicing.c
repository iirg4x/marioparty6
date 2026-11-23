/* Estimates voicing from signal autocorrelation and noise energy. */
#include "gssdk/mathusage.h"
#include "gssdk/heap.h"
#include "gssdk/triggerlr.h"

#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>

/* Adds a frame of samples to the voicing analysis buffer. */
int Voicing_AddSignal(TriggerLR *This, s16 *pSamples)
{
    int bRetVal = 0;
    s16 *pSubSampledSignal = This->pSubSampledSignal;
    s16 *pSubSampledSignal_End = pSubSampledSignal + This->frameSize;

    if (Subsampler_Process(This, pSamples) != 0) {
        bRetVal = 1;
    } else {
        while (pSubSampledSignal < pSubSampledSignal_End) {
            s16 SampleValue = *pSubSampledSignal;

            *This->pSignalOldest++ = SampleValue;
            *This->pSignalSqrOldest++ = SampleValue * SampleValue;
            if (This->pSignalOldest >= This->pSignalEnd) {
                This->pSignalOldest = This->pSignalBase;
                This->pSignalSqrOldest = This->pSignalSqrBase;
            }
            pSubSampledSignal++;
        }
    }
    return bRetVal;
}

/* Returns the logarithmic frame power for the requested lag. */
static float Voicing_GetLogPowerInFrame(TriggerLR *This, unsigned int lag)
{
    unsigned long *pSignal;
    float power = 0.0f;
    unsigned int Sample;

    pSignal = This->pSignalSqrOldest - 1 - lag;
    if (pSignal < This->pSignalSqrBase) {
        pSignal += This->totalBufferSize;
    }
    for (Sample = 0; Sample < This->frameSize; Sample++) {
        power += *pSignal--;
        if (pSignal < This->pSignalSqrBase) {
            pSignal += This->totalBufferSize;
        }
    }
    return logf_check(power);
}

/* Updates the noise energy estimate from the recent frames. */
void Voicing_MaintainNoiseEner(TriggerLR *This)
{
    *This->pLogNoiseEnergy++ = Voicing_GetLogPowerInFrame(This, 0);
    if (This->pLogNoiseEnergy >= This->pLogNoiseEnergyEnd) {
        This->pLogNoiseEnergy = This->pLogNoiseEnergyBase;
    }
    if (This->nrNoiseEnergies < This->maxNrNoiseEnergies) {
        This->nrNoiseEnergies++;
    }
}

/* Searches the allowed lags and returns the maximum voicing score. */
f32 Voicing_GetMaxVoicing(TriggerLR *This)
{
    f32 *pLogNoiseEn;
    f32 avgNoisePower;
    f32 logAvgNoisePower;
    f32 logPower1;
    f32 logPower2;
    f32 maxAutoCorr;
    f32 result;
    u32 lagMax;
    u32 Lag;
    u32 Sample;

    result = 0.0f;
    avgNoisePower = 0.0f;

    for (pLogNoiseEn = This->pLogNoiseEnergyBase;
         pLogNoiseEn < This->pLogNoiseEnergyBase + This->nrNoiseEnergies; pLogNoiseEn++) {
        avgNoisePower += expf(*pLogNoiseEn);
    }
    if (This->nrNoiseEnergies != 0) {
        avgNoisePower /= This->nrNoiseEnergies;
    }
    logAvgNoisePower = logf_check(avgNoisePower);

    logPower1 = Voicing_GetLogPowerInFrame(This, 0);

    if (!(logPower1 < logAvgNoisePower + This->voicingMinSNR ||
          logPower1 < This->voicingMinEnergy)) {
        maxAutoCorr = 0.0f;
        lagMax = 0;
        for (Lag = This->minLag; Lag <= This->maxLag; Lag++) {
            f32 sum = 0.0f;
            u32 Pass;

            for (Pass = 0; Pass < 4; Pass++) {
                s16 *pSignal1 = This->pSignalOldest - 1 - Pass;
                s16 *pSignal2;

                if (pSignal1 < This->pSignalBase) {
                    pSignal1 += This->totalBufferSize;
                }
                /* Each tap wraps separately from its unadjusted position. */
                pSignal2 = This->pSignalOldest - 1 - Pass - Lag;
                if (pSignal2 < This->pSignalBase) {
                    pSignal2 += This->totalBufferSize;
                }

                for (Sample = Pass; Sample < This->frameSize; Sample += 4) {
                    sum += (f32) (*pSignal1 * *pSignal2);
                    pSignal1 -= 4;
                    if (pSignal1 < This->pSignalBase) {
                        pSignal1 += This->totalBufferSize;
                    }
                    pSignal2 -= 4;
                    if (pSignal2 < This->pSignalBase) {
                        pSignal2 += This->totalBufferSize;
                    }
                }
                if (sum < 0.0f) {
                    break;
                }
            }

            if (sum > maxAutoCorr) {
                maxAutoCorr = sum;
                lagMax = Lag;
            }
        }

        logPower2 = Voicing_GetLogPowerInFrame(This, lagMax);

        if (!(logPower2 < logAvgNoisePower + This->voicingMinSNR ||
              logPower2 < This->voicingMinEnergy)) {
            logPower2 = logf_check(expf(logPower2) - avgNoisePower) / 2.0f;
            logPower1 = logf_check(expf(logPower1) - avgNoisePower) / 2.0f;

            result = logf_check(maxAutoCorr) - logPower1 - logPower2;
        }
    }
    return result;
}

/* Resets the voicing history and noise estimates. */
void Voicing_Reset(TriggerLR *This)
{
    This->pSignalOldest = This->pSignalBase;
    while (This->pSignalOldest < This->pSignalEnd) {
        *This->pSignalOldest = 0;
        This->pSignalOldest++;
    }
    This->pSignalOldest = This->pSignalBase;

    This->pSignalSqrOldest = This->pSignalSqrBase;
    while (This->pSignalSqrOldest < This->pSignalSqrEnd) {
        *This->pSignalSqrOldest = 0;
        This->pSignalSqrOldest++;
    }
    This->pSignalSqrOldest = This->pSignalSqrBase;

    This->pLogNoiseEnergy = This->pLogNoiseEnergyBase;
    This->nrNoiseEnergies = 0;
    Subsampler_Reset(This);
}

/* Allocates and initializes the voicing analysis state. */
int InitVoicing(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    f32 minLagInS;
    f32 maxLagInS;
    f32 frameSizeInS;
    f32 FsInHz;
    f32 voicingMinEnergy;
    f32 voicingMinSNR;
    int bRetVal = 0;

    This->frameSize = 0;
    This->maxLag = 0;
    This->minLag = 0;
    This->pSignalEnd = NULL;
    This->pSignalOldest = NULL;
    This->pSignalBase = NULL;
    This->pSignalSqrEnd = NULL;
    This->pSignalSqrOldest = NULL;
    This->pSignalSqrBase = NULL;
    This->pLogNoiseEnergyEnd = NULL;
    This->pLogNoiseEnergyBase = NULL;
    This->pLogNoiseEnergy = NULL;
    This->maxNrNoiseEnergies = 0;
    This->nrNoiseEnergies = 0;

    minLagInS = _tosGetProfileFloat(&This->BaseBlock.Object, 0x18, 0.005f);
    maxLagInS = _tosGetProfileFloat(&This->BaseBlock.Object, 0x19, 0.014f);
    frameSizeInS = _tosGetProfileFloat(&This->BaseBlock.Object, 0x1A, 0.01f);
    FsInHz = _tosGetProfileFloat(&This->BaseBlock.Object, 0x1B, 11000.0f);
    voicingMinEnergy = _tosGetProfileFloat(&This->BaseBlock.Object, 0x1D, -1000.0f);

    voicingMinSNR = _tosGetProfileFloat(&This->BaseBlock.Object, 0x1C, 6.0f);
    voicingMinSNR *= 0.23025851f;
    This->voicingMinEnergy = voicingMinEnergy;
    This->voicingMinSNR = voicingMinSNR;
    This->minLag = (u32) floorf((minLagInS * FsInHz) / 2.0f);
    This->maxLag = (u32) ceilf((maxLagInS * FsInHz) / 2.0f);
    This->frameSize = (u32) ceilf((frameSizeInS * FsInHz) / 2.0f);
    This->totalBufferSize = This->maxLag + This->frameSize;

    This->pSignalOldest = This->pSignalBase =
        heap_Calloc(pTos->hHeap, This->totalBufferSize, sizeof(s16));
    This->pSignalEnd = This->pSignalBase + This->totalBufferSize;

    This->pSignalSqrOldest = This->pSignalSqrBase =
        heap_Calloc(pTos->hHeap, This->totalBufferSize, sizeof(u32));
    This->pSignalSqrEnd = This->pSignalSqrBase + This->totalBufferSize;

    This->maxNrNoiseEnergies = 16;
    This->pLogNoiseEnergyBase = heap_Calloc(pTos->hHeap, This->maxNrNoiseEnergies, sizeof(f32));
    This->pLogNoiseEnergyEnd = This->pLogNoiseEnergyBase + This->maxNrNoiseEnergies;

    if (This->pSignalBase == NULL || This->pSignalSqrBase == NULL ||
        This->pLogNoiseEnergyBase == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        bRetVal = 1;
    } else {
        Voicing_Reset(This);
        if (Subsampler_Init(This) != 0) {
            bRetVal = 1;
        }
    }
    return bRetVal;
}

/* Frees the voicing analysis buffers. */
void Voicing_Free(TriggerLR *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    if (This->pSignalBase != NULL) {
        heap_Free(pTos->hHeap, This->pSignalBase);
    }
    if (This->pSignalSqrBase != NULL) {
        heap_Free(pTos->hHeap, This->pSignalSqrBase);
    }
    if (This->pLogNoiseEnergyBase != NULL) {
        heap_Free(pTos->hHeap, This->pLogNoiseEnergyBase);
    }
    Subsampler_Free(This);
}
