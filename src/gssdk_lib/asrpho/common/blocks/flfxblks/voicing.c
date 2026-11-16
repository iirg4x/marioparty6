#include "gssdk/triggerlr.h"

extern void *heap_Calloc(void *heap, u32 count, u32 size);
extern void heap_Free(void *heap, void *ptr);
extern f32 expf(f32 value);
extern f32 floorf(f32 value);
extern f32 ceilf(f32 value);
extern f32 logf_check(f32 value);
extern u32 _tosErrorLog(void *block, u32 error);

s32 Voicing_AddSignal(TriggerLR *This, s16 *pSamples)
{
    s32 result = 0;
    s16 *output = This->subsamplerOutput;
    s16 *outputEnd = output + This->frameLength;

    if (Subsampler_Process(This, pSamples) != 0) {
        result = 1;
    } else {
        while (output < outputEnd) {
            s16 sample = *output;

            *This->signalWrite++ = sample;
            *This->energyWrite++ = sample * sample;
            if (This->signalWrite >= This->signalEnd) {
                This->signalWrite = This->signalHistory;
                This->energyWrite = This->energyHistory;
            }
            output++;
        }
    }
    return result;
}

void Voicing_MaintainNoiseEner(TriggerLR *This)
{
    u32 *energy;
    u32 *energyHistory;
    f32 energySum = 0.0f;
    u32 i;

    if ((energy = This->energyWrite - 1) <
        (energyHistory = This->energyHistory)) {
        energy += This->historyLength;
    }
    for (i = 0; i < This->frameLength; i++) {
        energySum += *energy--;
        if (energy < energyHistory) {
            energy += This->historyLength;
        }
    }

    *This->noiseEnergyWrite++ = logf_check(energySum);
    if (This->noiseEnergyWrite >= This->noiseEnergyEnd) {
        This->noiseEnergyWrite = This->noiseEnergyHistory;
    }
    if (This->noiseEnergyCount < This->noiseEnergyCapacity) {
        This->noiseEnergyCount++;
    }
}

f32 Voicing_GetMaxVoicing(TriggerLR *This)
{
    f32 *noiseEnergy;
    f32 noiseMean;
    f32 noiseLog;
    f32 currentEnergy;
    f32 logPower1;
    f32 laggedEnergy;
    f32 maximumCorrelation;
    f32 maximumVoicing;
    u32 bestLag;
    u32 lag;
    u32 i;

    maximumVoicing = 0.0f;
    noiseMean = maximumVoicing;

    for (noiseEnergy = This->noiseEnergyHistory;
         noiseEnergy < This->noiseEnergyHistory + This->noiseEnergyCount;
         noiseEnergy++) {
        noiseMean += expf(*noiseEnergy);
    }
    if (This->noiseEnergyCount != 0) {
        noiseMean /= This->noiseEnergyCount;
    }
    noiseLog = logf_check(noiseMean);

    {
        u32 *energy;
        u32 *energyHistory;
        f32 energySum = 0.0f;

        if ((energy = This->energyWrite - 1) <
            (energyHistory = This->energyHistory)) {
            energy += This->historyLength;
        }
        for (i = 0; i < This->frameLength; i++) {
            energySum += *energy--;
            if (energy < energyHistory) {
                energy += This->historyLength;
            }
        }
        logPower1 = logf_check(energySum);
        currentEnergy = logPower1;
    }

    if (!(logPower1 < noiseLog + This->voicingThreshold ||
          logPower1 < This->minimumLogEnergy)) {
        maximumCorrelation = 0.0f;
        bestLag = 0;
        for (lag = This->minimumLag; lag <= This->maximumLag; lag++) {
            f32 correlation = 0.0f;
            u32 phase;

            for (phase = 0; phase < 4; phase++) {
                s16 *firstRaw = This->signalWrite - 1 - phase;
                s16 *first = firstRaw;
                s16 *second;

                if (firstRaw < This->signalHistory) {
                    first += This->historyLength;
                }
                second = firstRaw - lag;
                if (second < This->signalHistory) {
                    second += This->historyLength;
                }

                for (i = phase; i < This->frameLength; i += 4) {
                    correlation += (f32)(*first * *second);
                    first -= 4;
                    if (first < This->signalHistory) {
                        first += This->historyLength;
                    }
                    second -= 4;
                    if (second < This->signalHistory) {
                        second += This->historyLength;
                    }
                }
                if (correlation < 0.0f) {
                    break;
                }
            }

            if (correlation > maximumCorrelation) {
                maximumCorrelation = correlation;
                bestLag = lag;
            }
        }

        {
            u32 *energy;
            u32 *energyHistory;
            f32 energySum = 0.0f;

            if ((energy = This->energyWrite - 1 - bestLag) <
                (energyHistory = This->energyHistory)) {
                energy += This->historyLength;
            }
            for (i = 0; i < This->frameLength; i++) {
                energySum += *energy--;
                if (energy < energyHistory) {
                    energy += This->historyLength;
                }
            }
            laggedEnergy = logf_check(energySum);
        }

        if (!(laggedEnergy < noiseLog + This->voicingThreshold ||
              laggedEnergy < This->minimumLogEnergy)) {
            f32 laggedSignalEnergy =
                logf_check(expf(laggedEnergy) - noiseMean) / 2.0f;
            currentEnergy =
                logf_check(expf(currentEnergy) - noiseMean) / 2.0f;

            maximumVoicing = logf_check(maximumCorrelation) -
                             currentEnergy - laggedSignalEnergy;
        }
    }
    return maximumVoicing;
}

void Voicing_Reset(TriggerLR *This)
{
    This->signalWrite = This->signalHistory;
    while (This->signalWrite < This->signalEnd) {
        *This->signalWrite = 0;
        This->signalWrite++;
    }
    This->signalWrite = This->signalHistory;

    This->energyWrite = This->energyHistory;
    while (This->energyWrite < This->energyEnd) {
        *This->energyWrite = 0;
        This->energyWrite++;
    }
    This->energyWrite = This->energyHistory;

    This->noiseEnergyWrite = This->noiseEnergyHistory;
    This->noiseEnergyCount = 0;
    Subsampler_Reset(This);
}

s32 InitVoicing(TriggerLR *This)
{
    TosContext *context = This->base.context;
    f32 minimumPeriod;
    f32 maximumPeriod;
    f32 frameDuration;
    f32 sampleRate;
    f32 minimumLogEnergy;
    f32 voicingThreshold;
    f32 minimumPeriodSamples;
    f32 maximumPeriodSamples;
    f32 frameSamples;
    u32 result = 0;

    This->frameLength = 0;
    This->maximumLag = 0;
    This->minimumLag = 0;
    This->signalEnd = NULL;
    This->signalWrite = NULL;
    This->signalHistory = NULL;
    This->energyEnd = NULL;
    This->energyWrite = NULL;
    This->energyHistory = NULL;
    This->noiseEnergyEnd = NULL;
    This->noiseEnergyHistory = NULL;
    This->noiseEnergyWrite = NULL;
    This->noiseEnergyCapacity = 0;
    This->noiseEnergyCount = 0;

    minimumPeriod = _tosGetProfileFloat(This, 24, 0.005f);
    maximumPeriod = _tosGetProfileFloat(This, 25, 0.014f);
    frameDuration = _tosGetProfileFloat(This, 26, 0.01f);
    sampleRate = _tosGetProfileFloat(This, 27, 11000.0f);
    minimumLogEnergy = _tosGetProfileFloat(This, 29, -1000.0f);

    voicingThreshold = _tosGetProfileFloat(This, 28, 6.0f);
    voicingThreshold *= 0.23025851f;
    This->minimumLogEnergy = minimumLogEnergy;
    This->voicingThreshold = voicingThreshold;
    minimumPeriodSamples = minimumPeriod * sampleRate;
    This->minimumLag =
        (u32)floorf(minimumPeriodSamples / 2.0f);
    maximumPeriodSamples = maximumPeriod * sampleRate;
    This->maximumLag = (u32)ceilf(maximumPeriodSamples / 2.0f);
    frameSamples = frameDuration * sampleRate;
    This->frameLength = (u32)ceilf(frameSamples / 2.0f);
    This->historyLength = This->maximumLag + This->frameLength;

    This->signalWrite = This->signalHistory =
        heap_Calloc(context->heap, This->historyLength, sizeof(s16));
    This->signalEnd = This->signalHistory + This->historyLength;

    This->energyWrite = This->energyHistory =
        heap_Calloc(context->heap, This->historyLength, sizeof(u32));
    This->energyEnd = This->energyHistory + This->historyLength;

    This->noiseEnergyCapacity = 16;
    This->noiseEnergyHistory = heap_Calloc(
        context->heap, This->noiseEnergyCapacity, sizeof(f32));
    This->noiseEnergyEnd =
        This->noiseEnergyHistory + This->noiseEnergyCapacity;

    if (This->signalHistory == NULL || This->energyHistory == NULL ||
        This->noiseEnergyHistory == NULL) {
        _tosErrorLog(This, 2);
        result = 1;
    } else {
        Voicing_Reset(This);
        if (Subsampler_Init(This) != 0) {
            result = 1;
        }
    }
    return result;
}

void Voicing_Free(TriggerLR *This)
{
    TosContext *context = This->base.context;

    if (This->signalHistory != NULL) {
        heap_Free(context->heap, This->signalHistory);
    }
    if (This->energyHistory != NULL) {
        heap_Free(context->heap, This->energyHistory);
    }
    if (This->noiseEnergyHistory != NULL) {
        heap_Free(context->heap, This->noiseEnergyHistory);
    }
    Subsampler_Free(This);
}
