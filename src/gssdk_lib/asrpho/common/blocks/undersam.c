/* Combines each group of input audio samples into one wider output sample. */
#include "gssdk/blocks.h"
#include "types.h"

#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Frame length and group size for the undersampling block. */
typedef struct {
    struct _BLOCK BaseBlock;     /* Scheduler-owned block header. */
    u16 InputFrameSampleLength; /* Input samples processed in one frame. */
    u8 DownSampleRate;          /* Input samples summed into each output sample. */
} Undersampler;

/* Sums each input group into one output value during the TOS process pass. */
static void ProcessUndersampler(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Undersampler *This = (Undersampler *) _This;
    s16 *pIn = ((void **)pInputs)[0];
    u8 DownSampleRate = This->DownSampleRate;
    s32 *pOut;
    s16 *pInputSample;
    s16 *pIn_LocalEnd;
    s16 *pIn_GlobalEnd;
    s32 Temp;

    if (pIn != NULL) {
        pOut = qEnQueueOne(_This->OutputQueues->pQ);
        pIn_GlobalEnd = pIn + This->InputFrameSampleLength;
        pInputSample = pIn;
        while (pInputSample < pIn_GlobalEnd) {
            pIn_LocalEnd = pInputSample + DownSampleRate;
            Temp = *pInputSample++;
            while (pInputSample < pIn_LocalEnd) {
                Temp += *pInputSample++;
            }
            *pOut++ = Temp;
        }
    }
}

/* Releases the undersampler ports when the scheduler removes the block. */
static unsigned char ControlUndersampler(struct _BLOCK *_This, unsigned char ControlId)
{
    switch (ControlId) {
    case 0xFF:
        tosBaseBlockDestruct(_This);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Loads the input frame length and rate settings, derives the input samples per output sample,
 * rejects a frame length not divisible by that count, and sets the input/output queue byte
 * sizes. */
static unsigned char InitUndersampler(struct _BLOCK *_This)
{
    u16 InputFrameSampleLength;
    Undersampler *This = (Undersampler *) _This;
    u16 InputSampleFrequency;
    u8 UnderSamplerFrequency;
    u16 OutputFrameSampleLength;

    InputFrameSampleLength = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 1, 110);
    This->InputFrameSampleLength = InputFrameSampleLength;
    InputSampleFrequency = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 2, 11000);
    This->DownSampleRate = InputSampleFrequency /
        (u16) _tosGetProfileU32(&This->BaseBlock.Object, 3, 2000);
    UnderSamplerFrequency = This->DownSampleRate;
    if (InputFrameSampleLength % UnderSamplerFrequency != 0) {
        _tosErrorLog(&This->BaseBlock.Object, 0x64);
        return 1;
    }
    OutputFrameSampleLength = InputFrameSampleLength / UnderSamplerFrequency;
    This->BaseBlock.InputQueues->Size = InputFrameSampleLength * sizeof(s16);
    This->BaseBlock.OutputQueues->Size = OutputFrameSampleLength * sizeof(s32);
    return 0;
}

/* Creates the block registered in the speech pipeline's block definitions. */
struct _BLOCK *ConstructUndersampler(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 1, 1,
                                 ProcessUndersampler,
                                 InitUndersampler,
                                 /* ControlUndersampler only reads the command ID. */
                                 (u8 (*)(struct _BLOCK *, u8, u32, u32)) ControlUndersampler,
                                 sizeof(Undersampler));
}
