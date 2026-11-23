/* Applies pre-emphasis and a window to overlapping input frames. */
#include "gssdk/blocks.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

/* Window coefficients, sample history and flush state. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    f32 *pBeginSam; /* Start of the buffered sample history. */
    f32 *pWriteSam; /* Next sample slot to write. */
    f32 *pEndSam; /* End of the sample history allocation. */
    u16 WindowLength; /* Number of samples in an output window. */
    u16 FrameLength; /* Number of new samples supplied per frame. */
    u16 Zeroes; /* The game's code never reads this field. */
    u16 OverLap; /* Sample count shared by adjacent windows. */
    u16 NbrFramesInWindow; /* Number of input frames that fit in one window. */
    f32 PrevSample; /* Previous sample used by pre-emphasis. */
    u8 FirstFrame; /* Marks the first input frame after initialization. */
    u8 EmptyBuffer; /* Marks the sample history as drained. */
    s16 FirstSampleOfLastFrame; /* First sample retained from the most recent frame. */
    s16 LastSampleOfLastFrame; /* Last sample of the most recent frame. */
    u8 FlushFlag; /* Requests output of the remaining samples. */
    f32 *pflWindowCoef; /* Coefficients applied to the output samples. */
} Window;

/* Emits the last windowed samples from the history. */
static void WindowFlush(Window *This)
{
    u16 FrameLength = This->FrameLength;
    f32 *pRead = This->pWriteSam;
    f32 *pWindowedSamples;
    f32 *pWindowCoef = This->pflWindowCoef;
    f32 *pWindowCoefEnd;
    f32 *pBuffer;

    pWindowedSamples = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    pRead -= FrameLength;
    if (pRead < This->pBeginSam) {
        pRead = This->pEndSam - FrameLength;
    }
    memcpy(This->pWriteSam, pRead, FrameLength * sizeof(f32));
    *This->pWriteSam = (f32)This->FirstSampleOfLastFrame - (f32)This->LastSampleOfLastFrame;

    This->pWriteSam += FrameLength;
    if (This->pWriteSam == This->pEndSam) {
        This->pWriteSam = This->pBeginSam;
    }

    pBuffer = This->pWriteSam + FrameLength - This->OverLap;
    pWindowCoefEnd = pWindowCoef + This->OverLap;
    while (pWindowCoef < pWindowCoefEnd) {
        *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
    }
    if (pBuffer == This->pEndSam) {
        pBuffer = This->pBeginSam;
    }

    pWindowCoefEnd = pWindowCoef + FrameLength;
    while (pWindowCoef < pWindowCoefEnd) {
        *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
    }
    if (pBuffer == This->pEndSam) {
        pBuffer = This->pBeginSam;
    }

    pWindowCoefEnd = pWindowCoef + This->OverLap;
    while (pWindowCoef < pWindowCoefEnd) {
        *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
    }

    This->EmptyBuffer = 1;
    This->FlushFlag = 0;
}

/* Adds a PCM frame to the history and emits the overlapped window. */
static void ProcessWindow(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Window *This = (Window *)_This;
    s16 *pSamplesIn = ((void **)pInputs)[0];
    f32 *pWindowCoef;
    f32 *pBuffer;
    f32 *pWindowedSamples;
    f32 *pWindowCoefEnd;
    f32 PrevSam;
    f32 Sam;
    u16 FrameLength;
    s32 i;

    if (pSamplesIn != NULL) {
        FrameLength = This->FrameLength;
        pWindowCoef = This->pflWindowCoef;
        pBuffer = This->pWriteSam;
        This->FirstSampleOfLastFrame = *pSamplesIn;
        PrevSam = This->PrevSample;

        for (i = 0; i < FrameLength; i++) {
            Sam = (f32)*pSamplesIn++;
            *pBuffer++ = Sam - PrevSam;
            PrevSam = Sam;
        }
        This->LastSampleOfLastFrame = pSamplesIn[-1];
        This->PrevSample = Sam;

        if (pBuffer == This->pEndSam) {
            pBuffer = This->pBeginSam;
        }

        if (This->FirstFrame != 0) {
            f32 *pSam = This->pWriteSam;
            *pSam = 0.0f;
            memcpy(pBuffer, pSam, FrameLength * sizeof(f32));
            pBuffer += FrameLength;
            if (pBuffer == This->pEndSam) {
                pBuffer = This->pBeginSam;
            }
            This->FirstFrame = 0;
            This->EmptyBuffer = 0;
            This->pWriteSam = pBuffer;
        } else {
            pWindowedSamples = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
            This->pWriteSam = pBuffer;
            pBuffer += FrameLength - This->OverLap;

            pWindowCoefEnd = pWindowCoef + This->OverLap;
            while (pWindowCoef < pWindowCoefEnd) {
                *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
            }
            if (pBuffer == This->pEndSam) {
                pBuffer = This->pBeginSam;
            }

            pWindowCoefEnd = pWindowCoef + FrameLength;
            while (pWindowCoef < pWindowCoefEnd) {
                *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
            }
            if (pBuffer == This->pEndSam) {
                pBuffer = This->pBeginSam;
            }

            pWindowCoefEnd = pWindowCoef + This->OverLap;
            while (pWindowCoef < pWindowCoefEnd) {
                *pWindowedSamples++ = *pWindowCoef++ * *pBuffer++;
            }
        }
    }

    if (This->FlushFlag != 0) {
        WindowFlush(This);
    }
}

/* Resets sample history, requests a flush, or destroys the window block. */
static u8 ControlWindow(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    Window *This = (Window *)_This;

    switch (ControlId) {
    case 0x01:
        This->PrevSample = 0.0f;
        This->FirstFrame = 1;
        This->EmptyBuffer = 1;
        break;
    case 0x02:
        if (This->EmptyBuffer == 0) {
            This->FlushFlag = 1;
        }
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pBeginSam);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pflWindowCoef);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Reads frame dimensions and allocates history and window coefficients. */
static unsigned char InitWindow(struct _BLOCK *_This)
{
    Window *This = (Window *)_This;
    struct _TOS *pTos = _This->Object.pTos;
    u16 FrameLength;
    u16 WindowLength;
    s32 i;

    FrameLength = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 1, 110);
    This->FrameLength = FrameLength;
    WindowLength = This->WindowLength = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 2, 330);
    This->OverLap = (WindowLength - FrameLength) / 2;

    if (FrameLength > WindowLength || WindowLength > FrameLength * 3) {
        return 1;
    }

    This->BaseBlock.InputQueues->Size = FrameLength * sizeof(s16);
    This->BaseBlock.OutputQueues->Size = WindowLength * sizeof(f32);
    This->NbrFramesInWindow = 3;
    This->pBeginSam = heap_Calloc(
        pTos->hHeap, This->FrameLength * 3, sizeof(f32));
    This->pflWindowCoef = heap_Calloc(pTos->hHeap, WindowLength, sizeof(f32));
    if (This->pBeginSam == NULL || This->pflWindowCoef == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }

    This->pWriteSam = This->pBeginSam;
    This->pEndSam = This->pBeginSam + FrameLength * 3;
    for (i = 0; i < WindowLength; i++) {
        This->pflWindowCoef[i] =
            0.54f -
            0.46f * cosf((6.283185308 * (f32)i) / (f32)(WindowLength - 1));
    }

    This->PrevSample = 0.0f;
    This->FirstFrame = 1;
    This->EmptyBuffer = 1;
    This->FlushFlag = 0;
    return 0;
}

/* Allocates a windowing block with one input and one output. */
struct _BLOCK *ConstructWindow(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 1, 1, ProcessWindow, InitWindow,
        ControlWindow,
        sizeof(Window));
}
