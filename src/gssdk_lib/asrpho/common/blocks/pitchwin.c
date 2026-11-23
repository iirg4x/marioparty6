/* Buffers incoming speech samples and applies a Hamming window. */
#include "dolphin/math.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/cmath>
#include "gssdk/blocks.h"
#include "gssdk/mathusage.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

/* Hamming coefficients and sample history for one analysis window. */
typedef struct {
    struct _BLOCK BaseBlock;          /* Scheduler-owned block header. */
    f32 *pWindow;                    /* Hamming weight for each sample in the window. */
    s32 *pSampleBegin;               /* Circular buffer holding one complete analysis window. */
    s32 *pSampleRead;                /* First sample used for the next windowed output frame. */
    s32 *pSampleWrite;               /* First slot for the next input frame. */
    s32 *pSampleEnd;                 /* One past the final sample in pSampleBegin. */
    u16 NbrOfInputFrames;            /* Number of input frames that make up one analysis window. */
    u16 SampleLengthOfOneInputFrame; /* Samples per input frame and per output-frame advance. */
    u16 PitchWindowSampleLength;     /* Samples in the analysis window and coefficient array. */
} PitchWindow;

/* Adds the input frame to history and queues the windowed samples. */
static void ProcessPitchWindow(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    PitchWindow *This = (PitchWindow *) _This;
    s32 *pIn = ((void **)pInputs)[0];
    f32 *pWindow;
    f32 *pOut;
    u16 i;
    u16 j;

    if (pIn != NULL) {
        memcpy(This->pSampleWrite, pIn, This->SampleLengthOfOneInputFrame * sizeof(s32));
        This->pSampleWrite += This->SampleLengthOfOneInputFrame;
        if (This->pSampleWrite == This->pSampleEnd) {
            This->pSampleWrite = This->pSampleBegin;
        }

        pWindow = This->pWindow;
        pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
        for (i = 0; i < This->NbrOfInputFrames; i++) {
            for (j = 0; j < This->SampleLengthOfOneInputFrame; j++) {
                *pOut++ = *This->pSampleRead++ * *pWindow++;
            }
            if (This->pSampleRead == This->pSampleEnd) {
                This->pSampleRead = This->pSampleBegin;
            }
        }

        This->pSampleRead += This->SampleLengthOfOneInputFrame;
        if (This->pSampleRead == This->pSampleEnd) {
            This->pSampleRead = This->pSampleBegin;
        }
    }
}

/* Clears the sample history and resets the read and write positions. */
static void PitchWindowReset(PitchWindow *This)
{
    u16 i;

    This->pSampleWrite = This->pSampleBegin;
    for (i = 0; i < This->PitchWindowSampleLength; i++) {
        *This->pSampleWrite++ = 0;
    }
    This->pSampleRead = This->pSampleBegin;
    This->pSampleWrite = This->pSampleBegin +
                          (This->NbrOfInputFrames - 1) * This->SampleLengthOfOneInputFrame;
    This->pSampleEnd = This->pSampleBegin + This->PitchWindowSampleLength;
}

/* Handles reset and destruction commands. */
static u8 ControlPitchWindow(struct _BLOCK *_This, u8 ControlId, u32 CParam,
                             u32 DParam)
{
    PitchWindow *This = (PitchWindow *) _This;

    switch (ControlId) {
    case 0xC8:
        PitchWindowReset(This);
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pWindow);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->pSampleBegin);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Computes the Hamming weights used by the window. */
static void CreateWindow(f32 *WindowPointer, u16 SampleFrequency, u16 WindowSampleLength)
{
    int i;
    u16 coefficientCount = WindowSampleLength;
    u16 MidWindow;
    u16 samplesPerSecond = SampleFrequency;
    f32 r = (2.0 * M_PI) / ((f32) coefficientCount / samplesPerSecond);
    f32 dt = 1.0f / samplesPerSecond;
    f32 t;

    MidWindow = coefficientCount / 2;
    t = (MidWindow - 1) * dt;
    WindowPointer[MidWindow] = 1.0f;

    for (i = 1; i < MidWindow; i++) {
        WindowPointer[MidWindow - i] = 0.54 - 0.46 * cosf(r * t);
        WindowPointer[MidWindow + i] = WindowPointer[MidWindow - i];
        t -= dt;
    }
    WindowPointer[0] = 0.54 - 0.46 * cosf(r * t);
}

/* Reads the rec1600 profile and prepares the analysis window. */
static unsigned char InitPitchWindow(struct _BLOCK *_This)
{
    PitchWindow *This = (PitchWindow *) _This;
    struct _TOS *pTos = _This->Object.pTos;
    u16 InputFrequency;
    f32 PitchWindowTimeLength;
    u16 EngineNbrSamplesPerFrame;
    u16 EngineInputSampleFrequency;
    u16 PitchWindowSampleLength;

    /* Profile keys 0x01-0x04 select output rate, duration, input frame size, and input rate. */
    InputFrequency = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x01, 2200);
    PitchWindowTimeLength = _tosGetProfileFloat(&This->BaseBlock.Object, 0x02, 0.04f);
    EngineNbrSamplesPerFrame = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x03, 110);
    EngineInputSampleFrequency = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 0x04, 11000);

    PitchWindowSampleLength = This->PitchWindowSampleLength =
        InputFrequency * PitchWindowTimeLength + 0.5;
    /* The configured input window must contain a whole number of input frames. */
    if ((u32) (0.5 + PitchWindowTimeLength * EngineInputSampleFrequency) %
            EngineNbrSamplesPerFrame != 0) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        return 1;
    }

    This->NbrOfInputFrames =
        (u32) (0.5 + PitchWindowTimeLength * EngineInputSampleFrequency) / EngineNbrSamplesPerFrame;
    This->SampleLengthOfOneInputFrame = PitchWindowSampleLength / This->NbrOfInputFrames;
    This->BaseBlock.InputQueues->Size = This->SampleLengthOfOneInputFrame * sizeof(s32);
    This->BaseBlock.OutputQueues->Size = PitchWindowSampleLength * sizeof(f32);

    This->pWindow = heap_Calloc(pTos->hHeap, PitchWindowSampleLength, sizeof(f32));
    This->pSampleBegin = heap_Calloc(pTos->hHeap, PitchWindowSampleLength, sizeof(s32));
    if (This->pWindow == NULL || This->pSampleBegin == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x02);
        return 1;
    }

    CreateWindow(This->pWindow, InputFrequency, PitchWindowSampleLength);
    This->pSampleRead = This->pSampleBegin;
    This->pSampleWrite =
        This->pSampleBegin + (This->NbrOfInputFrames - 1) * This->SampleLengthOfOneInputFrame;
    This->pSampleEnd = This->pSampleBegin + This->PitchWindowSampleLength;
    return 0;
}

/* Registers the pitch-window block with the scheduler. */
struct _BLOCK *ConstructPitchWindow(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1,
                                 ProcessPitchWindow,
                                 InitPitchWindow,
                                 ControlPitchWindow,
                                 sizeof(PitchWindow));
}
