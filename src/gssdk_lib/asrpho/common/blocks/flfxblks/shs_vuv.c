/* Computes a summed subharmonic spectrum and a voicing decision. */
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "gssdk/tos.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>

/* Pitch and voicing analysis state with precomputed spectral weights. */
typedef struct SHS_VUV {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u16 FFTsize; /* FFT size for the input spectrum. */
    u16 ActualSampleFrequency; /* Sample rate used to convert frequency to FFT bins. */
    u16 MinimumPitch;    /* Lowest frequency covered by the log spectrum. */
    u16 PointsPerOctave; /* Resolution of the log-frequency grid. */
    u8 NumberOfOctaves; /* Number of octaves in the log-frequency spectrum. */
    u8 NumberOfSubHarmonics; /* Number of harmonics to sum for each pitch estimate. */
    f32 SubHarmFact;         /* Weight decay applied across the harmonics. */
    /* Threshold for the ratio of the strongest summed bin to the signal peak. */
    u32 VUVThreshold;
    u16 NbrOfPoints; /* Number of bins in the log-frequency spectrum. */
    f32 *pPeakEnhancedSpectrum; /* Linear-frequency spectrum around the selected peaks. */
    f32 *pSmoothedSpectrum; /* Linear-frequency spectrum after smoothing. */
    /* Scratch arrays hold differences, two factors, the right-hand side and
     * the values used in back substitution. */
    f32 **ppSpline2Pointers;
    f32 *pSplineCoeff; /* Spline coefficients used to evaluate fractional FFT bins. */
    f32 *pLogSpectrum;                /* Input spectrum mapped to the log-frequency grid. */
    f32 *pPerceptionFilteredSpectrum; /* Log-frequency spectrum after weighting. */
    f32 *pFreqWindow; /* Perceptual weights applied to the log-frequency spectrum. */
    long *pShiftArray; /* Log-bin displacement for each harmonic. */
} SHS_VUV;

/* Emphasizes spectral peaks that contribute to the pitch estimate. */
static void PeakEnhancement(SHS_VUV *This, void *pInputs)
{
    float *pAmpSpecBegin;
    float *pPeakEnhancedSpectrum = This->pPeakEnhancedSpectrum;
    float *pPeakEnhSpec;
    float *pPeakEnhSpecEnd;
    float *pAmplitudeSpectrum;
    float *pAmpSpecEnd;
    float *pAmpSpec;
    float freqRes;
    long skip;
    u32 HalfFFTSize_plus_1;

    pPeakEnhSpec = pPeakEnhancedSpectrum;
    HalfFFTSize_plus_1 = This->FFTsize / 2 + 1;
    pPeakEnhSpecEnd = pPeakEnhancedSpectrum + HalfFFTSize_plus_1;
    while (pPeakEnhSpec < pPeakEnhSpecEnd) {
        *pPeakEnhSpec++ = 0.0f;
    }

    pAmpSpecBegin = ((void **)pInputs)[1];
    freqRes = This->ActualSampleFrequency / (float)This->FFTsize;
    skip = (long)(This->MinimumPitch / freqRes);
    pAmplitudeSpectrum = pAmpSpecBegin + skip;
    pAmpSpecEnd = pAmpSpecBegin + This->FFTsize / 2 - 1;
    for (;; pAmplitudeSpectrum++) {
        if (pAmplitudeSpectrum[-1] <= pAmplitudeSpectrum[0] &&
            pAmplitudeSpectrum[1] <= pAmplitudeSpectrum[0]) {
            pAmpSpec = pAmplitudeSpectrum - 2;
            pPeakEnhSpec = pPeakEnhancedSpectrum + (pAmplitudeSpectrum - pAmpSpecBegin) - 2;
            *pPeakEnhSpec++ = *pAmpSpec++;
            *pPeakEnhSpec++ = *pAmpSpec++;
            *pPeakEnhSpec++ = *pAmpSpec++;
            *pPeakEnhSpec++ = *pAmpSpec++;
            if (pAmplitudeSpectrum == pAmpSpecEnd) {
                break;
            }
            *pPeakEnhSpec = *pAmpSpec;
            continue;
        }
        if (pAmplitudeSpectrum == pAmpSpecEnd) {
            break;
        }
    }
}

/* Smooths the enhanced spectrum before pitch analysis. */
static void SpectrumSmoothing(SHS_VUV *This)
{
    float *pPeakEnhSpec;
    float *SmoothSpec;
    float *SmoothSpecEnd;

    SmoothSpecEnd = This->pSmoothedSpectrum + This->FFTsize / 2;
    SmoothSpec = This->pSmoothedSpectrum;
    *SmoothSpec++ = 0.0f;
    pPeakEnhSpec = This->pPeakEnhancedSpectrum + 1;
    while (SmoothSpec < SmoothSpecEnd) {
        *SmoothSpec++ =
            0.25 * (pPeakEnhSpec[-1] + pPeakEnhSpec[0] + pPeakEnhSpec[0] + pPeakEnhSpec[1]);
        pPeakEnhSpec++;
    }
    *SmoothSpec = 0.5 * (pPeakEnhSpec[-1] + pPeakEnhSpec[0]);
}

/* Applies the perceptual frequency weighting to the spectrum. */
static void PerceptionFiltering(SHS_VUV *This)
{
    float *pPercFiltSpec = This->pPerceptionFilteredSpectrum;
    float *pPercFiltSpecEnd = pPercFiltSpec + This->NbrOfPoints;
    float *pLogSpec = This->pLogSpectrum;
    float *pFreqWindow = This->pFreqWindow;

    while (pPercFiltSpec < pPercFiltSpecEnd) {
        *pPercFiltSpec++ = *pLogSpec > 0.0f ? *pLogSpec * *pFreqWindow : 0.0f;
        pLogSpec++;
        pFreqWindow++;
    }
}

/* Computes the spline coefficients for resampling the spectrum. */
static void spline2(f32 *spec, f32 *coeff, SHS_VUV *This)
{
    f32 *d = This->ppSpline2Pointers[0];
    f32 *a = This->ppSpline2Pointers[1];
    f32 *b = This->ppSpline2Pointers[2];
    f32 *f = This->ppSpline2Pointers[3];
    f32 *g = This->ppSpline2Pointers[4];
    f32 *p0, *p1, *p2, *p3;
    long i;
    long n = This->FFTsize / 2 + 1;

    p0 = spec;
    p1 = spec + 1;
    p2 = d + 1;
    for (i = n - 1; i > 0; i--) {
        *p2++ = *p1++ - *p0++;
    }
    a[0] = 2.0f;
    p0 = a;
    p1 = a + 1;
    p2 = b + 1;
    for (i = n - 2; i > 0; i--) {
        *p2 = 1.0 / *p0++;
        *p1++ = 4.0f - *p2++;
    }
    *p2 = 1.0 / *p0;
    *p1 = 2.0f - *p0;
    f[0] = 3.0f * d[1];
    p0 = d + 1;
    p1 = d + 2;
    p2 = f + 1;
    for (i = n - 2; i > 0; i--) {
        *p2++ = 3.0f * (*p0++ + *p1++);
    }
    *p2 = 3.0f * *p0;
    g[0] = f[0];
    p0 = b + 1;
    p2 = g;
    p3 = g + 1;
    p1 = f + 1;
    for (i = n - 1; i > 0; i--) {
        *p3++ = *p1++ - *p0++ * *p2++;
    }
    p0 = a + (n - 1);
    p1 = g + (n - 1);
    p2 = coeff + (n - 1);
    *p2 = *p1 / *p0;
    p0--;
    p1--;
    p3 = p2 - 1;
    for (i = n - 2; i >= 0; i--) {
        *p3-- = (*p1-- - *p2--) / *p0--;
    }
}

/* Evaluates the spectral spline at a fractional bin position. */
static float interpolate(float x, const float *spec, const float *coeff)
{
    long i, ip1;
    float d, t, s;
    i = x;
    ip1 = i + 1;
    s = x - i;
    t = 1.0f - s;
    d = spec[ip1] - spec[i];
    return s * spec[ip1] + t * spec[i] + s * t * (t * (coeff[i] - d) - s * (coeff[ip1] - d));
}

/* Resamples the linear spectrum onto a logarithmic frequency axis. */
static void Lin_2_Log(SHS_VUV *This)
{
    unsigned long i, k;
    float linbin, Freq, NextFreqFactor, t;
    unsigned int NumberOfOctaves = This->NumberOfOctaves;
    unsigned int NumberOfOctaves_Minus_1 = (unsigned char) (NumberOfOctaves - 1);
    unsigned int PointsPerOctave = This->PointsPerOctave;
    unsigned int PointsPerOctave_Minus_1 = (unsigned short) (PointsPerOctave - 1);
    float *pLogSpectrum = This->pLogSpectrum;
    unsigned int FFTsize = This->FFTsize;
    float FFTsize_DividedBy_2 = FFTsize / 2.0;
    float *pSmoothedSpectrum = This->pSmoothedSpectrum;
    float *pSplineCoeff = This->pSplineCoeff;

    spline2(pSmoothedSpectrum, pSplineCoeff, This);
    t = (float) FFTsize / This->ActualSampleFrequency;
    NextFreqFactor = powf(2.0f, 1.0 / PointsPerOctave);
    Freq = This->MinimumPitch;
    for (i = 0; i <= NumberOfOctaves_Minus_1; i++) {
        for (k = 0; k <= PointsPerOctave_Minus_1; k++) {
            linbin = Freq * t;
            if (linbin < FFTsize_DividedBy_2) {
                pLogSpectrum[i * PointsPerOctave + k] =
                    interpolate(linbin, pSmoothedSpectrum, pSplineCoeff);
            } else {
                pLogSpectrum[i * PointsPerOctave + k] = 0.0f;
            }
            Freq *= NextFreqFactor;
        }
    }
}

/* Queues the voicing decision byte on output 1. */
static void VUVDetermination(SHS_VUV *This, float *pSHS, void *pInputs)
{
    float *pSignalPeak = ((void **)pInputs)[0];
    unsigned char *pVUV = qEnQueueOne(This->BaseBlock.OutputQueues[1].pQ);
    float *pSHSEnd = pSHS + This->NbrOfPoints;
    float SHSExtremum = *pSHS++;

    while (pSHS < pSHSEnd) {
        if (*pSHS > SHSExtremum) {
            SHSExtremum = *pSHS;
        }
        pSHS++;
    }
    /* VAD supplies the tracked signal peak in the second float of its level packet. */
    if (SHSExtremum / pSignalPeak[1] < (float) This->VUVThreshold) {
        *pVUV = 0;
    } else {
        *pVUV = 1;
    }
}

/* Sums the subharmonics and uses the resulting spectrum to decide voicing. */
static void SubHarmonicSummation_and_VUVDetermination(SHS_VUV *This, void *pInputs)
{
    float SubHarmFact = This->SubHarmFact;
    float SHFact = SubHarmFact;
    float *d, *dend, *e;
    float *pSubHarmSumSpecBegin = qEnQueueOne(This->BaseBlock.OutputQueues[0].pQ);
    float *pSubHarmSumSpec;
    unsigned short NbrOfPoints = This->NbrOfPoints;
    float *pSubHarmSumSpecEnd = pSubHarmSumSpecBegin + NbrOfPoints;
    float *pPercFiltSpec = This->pPerceptionFilteredSpectrum;
    long *pShiftArr = This->pShiftArray;
    long *pShiftArrEnd = pShiftArr + This->NumberOfSubHarmonics;

    pSubHarmSumSpec = pSubHarmSumSpecBegin;
    while (pSubHarmSumSpec < pSubHarmSumSpecEnd) {
        *pSubHarmSumSpec++ = *pPercFiltSpec++;
    }
    pPercFiltSpec = This->pPerceptionFilteredSpectrum;
    while (pShiftArr < pShiftArrEnd) {
        e = pPercFiltSpec + *pShiftArr;
        d = pSubHarmSumSpecBegin;
        dend = pSubHarmSumSpecBegin + NbrOfPoints - *pShiftArr++;
        while (d < dend) {
            *d++ += SHFact * *e++;
        }
        SHFact *= SubHarmFact;
    }
    VUVDetermination(This, pSubHarmSumSpecBegin, pInputs);
}

/* Processes a frame and writes the summed spectrum to output 0 and voicing to output 1. */
static void ProcessSHS_VUV(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    SHS_VUV *This = (SHS_VUV *)_This;

    if (((void **)pInputs)[0] != NULL) {
        PeakEnhancement(This, pInputs);
        SpectrumSmoothing(This);
        Lin_2_Log(This);
        PerceptionFiltering(This);
        SubHarmonicSummation_and_VUVDetermination(This, pInputs);
    }
}

/* Releases the pitch and voicing block on teardown. */
static u8 ControlSHS_VUV(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    SHS_VUV *This = (SHS_VUV *) _This;

    switch (ControlId) {
    case 0xFF: {
        struct _TOS *pTos = This->BaseBlock.Object.pTos;

        heap_Free(pTos->hHeap, This->pPeakEnhancedSpectrum);
        heap_Free(pTos->hHeap, This->pSmoothedSpectrum);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers[0]);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers[1]);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers[2]);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers[3]);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers[4]);
        heap_Free(pTos->hHeap, This->ppSpline2Pointers);
        heap_Free(pTos->hHeap, This->pSplineCoeff);
        heap_Free(pTos->hHeap, This->pLogSpectrum);
        heap_Free(pTos->hHeap, This->pPerceptionFilteredSpectrum);
        heap_Free(pTos->hHeap, This->pFreqWindow);
        heap_Free(pTos->hHeap, This->pShiftArray);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    }
    default:
        return 0;
    }
    return 1;
}

/* Builds the frequency weights for the logarithmic spectrum. */
static void CreateFreqWindow(SHS_VUV *This)
{
    unsigned short PointsPerOctave;
    float *pFreqWindow;
    unsigned short NbrOfPoints;
    float s;
    long i;

    PointsPerOctave = This->PointsPerOctave;
    pFreqWindow = This->pFreqWindow;
    NbrOfPoints = This->NbrOfPoints;
    s = PointsPerOctave * logf_check((f32) (65.0 / This->MinimumPitch)) / 0.6931472f;
    for (i = 0; i < NbrOfPoints; i++) {
        pFreqWindow[i] =
            (f32) (0.5 + atanf((f32) (3.0 * (i - s) / PointsPerOctave)) / 3.141592653589793);
        if (pFreqWindow[i] < 0.0f) {
            pFreqWindow[i] = 0.0f;
        }
    }
}

/* Builds the bin shifts used to accumulate the subharmonics. */
static void CreateShiftArr(SHS_VUV *This)
{
    long i;
    u8 NumberOfSubHarmonics;
    u16 PointsPerOctave;
    long *pShiftArray;

    NumberOfSubHarmonics = This->NumberOfSubHarmonics;
    PointsPerOctave = This->PointsPerOctave;
    pShiftArray = This->pShiftArray;
    for (i = 0; i < NumberOfSubHarmonics; i++) {
        pShiftArray[i] = 0.5 + (f32) PointsPerOctave * logf_check((f32) (2.0 + i)) / 0.6931472f;
    }
}

/* Allocates and initializes the subharmonic analysis tables and buffers. */
static u8 InitSHS_VUV(struct _BLOCK *_This)
{
    SHS_VUV *This = (SHS_VUV *) _This;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    u16 HalfFFTSize_plus_1;

    HalfFFTSize_plus_1 = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 1, 65);
    This->FFTsize = (HalfFFTSize_plus_1 - 1) * 2;
    This->ActualSampleFrequency = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 2, 2200);
    This->MinimumPitch = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 3, 50);
    This->PointsPerOctave = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 4, 50);
    This->NumberOfOctaves = (u8) _tosGetProfileU32(&This->BaseBlock.Object, 5, 5);
    This->NbrOfPoints = This->NumberOfOctaves * This->PointsPerOctave;
    This->VUVThreshold = _tosGetProfileU32(&This->BaseBlock.Object, 8, 5000);

    This->BaseBlock.InputQueues[0].Size = 6 * sizeof(f32); /* VAD level packet. */
    This->BaseBlock.InputQueues[1].Size = HalfFFTSize_plus_1 * sizeof(f32);
    This->BaseBlock.OutputQueues[0].Size = This->NbrOfPoints * sizeof(f32);
    This->BaseBlock.OutputQueues[1].Size = sizeof(u8);

    This->pPeakEnhancedSpectrum = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->pSmoothedSpectrum = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->ppSpline2Pointers = heap_Calloc(pTos->hHeap, 5, sizeof(f32 *));
    if (This->pPeakEnhancedSpectrum == NULL || This->pSmoothedSpectrum == NULL ||
        This->ppSpline2Pointers == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }

    This->ppSpline2Pointers[0] = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->ppSpline2Pointers[1] = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->ppSpline2Pointers[2] = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->ppSpline2Pointers[3] = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->ppSpline2Pointers[4] = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->pSplineCoeff = heap_Calloc(pTos->hHeap, HalfFFTSize_plus_1, sizeof(f32));
    This->pLogSpectrum = heap_Calloc(pTos->hHeap, This->NbrOfPoints, sizeof(f32));
    This->pPerceptionFilteredSpectrum =
        heap_Calloc(pTos->hHeap, This->NbrOfPoints, sizeof(f32));
    This->pFreqWindow = heap_Calloc(pTos->hHeap, This->NbrOfPoints, sizeof(f32));

    CreateFreqWindow(This);
    This->NumberOfSubHarmonics = (u8) _tosGetProfileU32(&This->BaseBlock.Object, 6, 15);
    This->pShiftArray = heap_Calloc(pTos->hHeap, This->NumberOfSubHarmonics, sizeof(long));
    CreateShiftArr(This);
    This->SubHarmFact = _tosGetProfileFloat(&This->BaseBlock.Object, 7, 0.84f);

    if (This->ppSpline2Pointers[0] == NULL || This->ppSpline2Pointers[1] == NULL ||
        This->ppSpline2Pointers[2] == NULL || This->ppSpline2Pointers[3] == NULL ||
        This->ppSpline2Pointers[4] == NULL || This->pSplineCoeff == NULL ||
        This->pLogSpectrum == NULL || This->pPerceptionFilteredSpectrum == NULL ||
        This->pFreqWindow == NULL || This->pShiftArray == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }
    return 0;
}

/* Constructs a TOS block for pitch and voicing estimation. */
struct _BLOCK *ConstructSHS_VUV(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 2, 2, ProcessSHS_VUV, InitSHS_VUV, ControlSHS_VUV,
                                 sizeof(SHS_VUV));
}
