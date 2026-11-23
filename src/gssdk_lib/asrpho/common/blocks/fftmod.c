// Converts incoming audio frames into an amplitude or power spectrum.
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/heap.h"
#include "types.h"

#include "dolphin/math.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/cmath>
#include <string.h>

#include "gssdk/fft.h"
#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* FFT configuration, frame sizes and power-spectrum output state. */
typedef struct {
    struct _BLOCK BaseBlock; // TinyOS block state and audio queue ports.
    u32 WindowLength; // Number of input audio samples in each frame.
    u32 fftN; // Number of samples transformed by the FFT.
    u32 Nlow; // First spectrum bin included in the power total.
    u32 Nhigh; // Last spectrum bin included in the power total.
    f32 EnergyFloor; // Initial value added to the selected-bin power total.
    u8 ZeroPadding; // Nonzero when short frames are zero-padded to the FFT length.
    u32 NbrZeroPadBytes; // Number of trailing zero bytes added to each frame.
    f32 *pFFTIn; // Scratch frame used when input samples need zero-padding.
    s16 SignalLevel; // Smoothed power level in hundredths of a decibel.
    u8 LogECalc; // Nonzero to output band power and a scalar power level.
} FFTMod;

// Called by ProcessFFTMod for each power-mode frame. Fills the spectrum and totals selected bins.
static f32 CalcPowerSpec(FFTMod *This, f32 *pInput, f32 *pOutput)
{
    f32 Energy = This->EnergyFloor;
    f32 *pReal = pInput;
    f32 *pImag = pInput + This->fftN - 1;
    f32 *pLower = pOutput + This->Nlow;
    f32 *pUpper = pOutput + This->Nhigh;
    f32 *pEnd = pOutput + This->fftN / 2;

    *pOutput = *pReal * *pReal;
    pReal++;
    if (pOutput++ == pLower) {
        Energy += *pLower++;
    }

    while (pOutput < pLower) {
        *pOutput++ = *pReal * *pReal + *pImag * *pImag;
        pReal++;
        pImag--;
    }
    while (pOutput <= pUpper) {
        *pOutput = *pReal * *pReal + *pImag * *pImag;
        Energy += *pOutput++;
        pReal++;
        pImag--;
    }
    while (pOutput < pEnd) {
        *pOutput++ = *pReal * *pReal + *pImag * *pImag;
        pReal++;
        pImag--;
    }
    *pOutput = *pReal * *pReal;
    return Energy;
}

// Called by ProcessFFTMod for each amplitude-mode frame. Converts FFT bins to magnitudes.
static void CalcAmplitudeSpec(FFTMod *This, f32 *pInput, f32 *pOutput)
{
    f32 *pReal = pInput;
    f32 *pImag = pInput + This->fftN - 1;
    f32 *pLower = pOutput + This->Nlow;
    f32 *pUpper = pOutput + This->Nhigh;
    f32 *pEnd = pOutput + This->fftN / 2;

    *pOutput = sqrtf(*pReal * *pReal);
    pReal++;
    if (pOutput++ == pLower) {
        pLower++;
    }

    while (pOutput < pLower) {
        *pOutput++ = sqrtf(*pReal * *pReal + *pImag * *pImag);
        pReal++;
        pImag--;
    }
    while (pOutput <= pUpper) {
        *pOutput++ = sqrtf(*pReal * *pReal + *pImag * *pImag);
        pReal++;
        pImag--;
    }
    while (pOutput < pEnd) {
        *pOutput++ = sqrtf(*pReal * *pReal + *pImag * *pImag);
        pReal++;
        pImag--;
    }
    *pOutput = sqrtf(*pReal * *pReal);
}

// Transforms each available input frame and queues its spectrum.
static void ProcessFFTMod(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    FFTMod *This = (FFTMod *)_This;
    f32 *pSamplesIn;
    f32 *pPowSpecOut;

    if (This->ZeroPadding != 0) {
        if (((void **)pInputs)[0] != NULL) {
            memcpy(This->pFFTIn, ((void **)pInputs)[0], This->BaseBlock.InputQueues->Size);
            pSamplesIn = This->pFFTIn;
            // Samples beyond the pSamplesIn frame are zero in the fixed-length FFT.
            memset(pSamplesIn + This->WindowLength, 0, This->NbrZeroPadBytes);
        } else {
            pSamplesIn = NULL;
        }
    } else {
        pSamplesIn = ((void **)pInputs)[0];
    }

    if (pSamplesIn != NULL) {
        if (This->LogECalc != 0) {
            pPowSpecOut = qEnQueueOne(This->BaseBlock.OutputQueues[1].pQ);
        } else {
            pPowSpecOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
        }

        realfft(This->fftN, pSamplesIn);
        if (This->LogECalc != 0) {
            f32 Energy = logf_check(CalcPowerSpec(This, pSamplesIn, pPowSpecOut));
            s16 NewSignalLevel;

            qEnQueue(This->BaseBlock.OutputQueues->pQ, &Energy, 1);
            NewSignalLevel = (s16)((4.3429446f * (Energy - 27.37f)) * 100.0);
            // The reported level is capped at 18 dB and decays gradually for lower readings.
            if (NewSignalLevel > 1800) {
                NewSignalLevel = 1800;
            }

            if (NewSignalLevel > This->SignalLevel) {
                This->SignalLevel = NewSignalLevel;
            } else {
                This->SignalLevel = (((This->SignalLevel + 7200) * 64000) >> 16) - 7200;
            }
        } else {
            CalcAmplitudeSpec(This, pSamplesIn, pPowSpecOut);
        }
    }
}

// Answers power-level queries only in power mode (logging an error otherwise) and frees padding
// storage and the block on shutdown.
static unsigned char ControlFFTMod(struct _BLOCK *_This, unsigned char ControlId,
                                   unsigned long CParam)
{
    FFTMod *This = (FFTMod *)_This;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    switch (ControlId) {
    case 0x01:
        if (This->LogECalc == 0) {
            _tosErrorLog(&This->BaseBlock.Object, 0x66);
            return 1;
        }
        *(s16 **)CParam = &This->SignalLevel;
        break;
    case 0xFF:
        if (This->ZeroPadding != 0) {
            heap_Free(pTos->hHeap, This->pFFTIn);
        }
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

// Called when the audio graph starts this block. Reads its profile and sizes its queues.
static unsigned char InitFFTMod(struct _BLOCK *_This)
{
    FFTMod *This = (FFTMod *)_This;
    u32 fftN;
    struct _TOS *pTos = _This->Object.pTos;
    s32 fNyquist;
    f32 Nlow;
    f32 Nhigh;

    fNyquist = (s16)_tosGetProfileU32(&This->BaseBlock.Object, 3, 5500);
    fftN = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 4, 512);
    This->fftN = fftN;
    This->WindowLength = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 6, 512);
    if (This->WindowLength > fftN) {
        return 1;
    }

    if (fftN > This->WindowLength) {
        This->ZeroPadding = 1;
        This->NbrZeroPadBytes = (fftN - This->WindowLength) * sizeof(f32);
        This->pFFTIn = heap_Alloc(pTos->hHeap, fftN * sizeof(f32));
    }

    This->BaseBlock.InputQueues->Size = This->WindowLength * sizeof(f32);
    if (This->LogECalc == 0) {
        This->BaseBlock.OutputQueues->Size = (fftN / 2 + 1) * sizeof(f32);
    } else {
        This->BaseBlock.OutputQueues->Size = sizeof(f32);
        This->BaseBlock.OutputQueues[1].Size = (fftN / 2 + 1) * sizeof(f32);
    }

    Nlow = _tosGetProfileFloat(&This->BaseBlock.Object, 1, 300.0f);
    This->Nlow = (u32)floorf((f32)This->fftN * Nlow / (2.0f * (f32)fNyquist));
    This->Nlow = This->Nlow > 0 ? This->Nlow : 0;
    This->Nlow = This->Nlow < This->fftN / 2 - 1 ? This->Nlow : This->fftN / 2 - 2;

    Nhigh = _tosGetProfileFloat(&This->BaseBlock.Object, 2, 4500.0f);
    This->Nhigh = (u32)floorf(0.9999f + (This->fftN * Nhigh) / (2.0f * (f32)fNyquist));
    This->Nhigh = This->Nhigh > 0 ? This->Nhigh : 0;
    This->Nhigh = This->Nhigh < This->fftN / 2 - 1 ? This->Nhigh : This->fftN / 2 - 2;
    This->Nhigh = This->Nhigh > This->Nlow ? This->Nhigh : This->Nlow;

    This->EnergyFloor = _tosGetProfileFloat(&This->BaseBlock.Object, 5, 0.0f);
    return 0;
}

// Called by the audio profile loader to create an FFT block with one or two output queues.
struct _BLOCK *ConstructFFTMod(struct _TOS *pTos, u8 BlockID)
{
    struct _OBJECT Object;
    FFTMod *pFFTMod;
    u8 LogECalc;

    Object.pTos = pTos;
    Object.BlockID = BlockID;
    LogECalc = (u8)_tosGetProfileU32(&Object, 7, 1);
    pFFTMod = (FFTMod *)tosBaseBlockConstruct(
        pTos, BlockID, 1, LogECalc != 0 ? 2 : 1,
        ProcessFFTMod, InitFFTMod,
        /* ControlFFTMod reads the command and the first payload word. */
        (u8 (*)(struct _BLOCK *, u8, u32, u32))ControlFFTMod, sizeof(FFTMod));
    pFFTMod->LogECalc = LogECalc;
    return &pFFTMod->BaseBlock;
}
