/* Estimates speaker gender from weighted acoustic frames and saved session state. */
#include "gssdk/blocks.h"
#include "gssdk/heap.h"
#include "types.h"

#include <float.h>
#include <math.h>
#include <string.h>

#include "gssdk/langdata.h"
#include "gssdk/tos.h"
#include "gssdk/mqueue.h"

/* Gender scores and the queued frames held while choosing an output. */
typedef struct _GENDER {
    struct _BLOCK BaseBlock; /* TOS callbacks, queues and object context. */
    enum { NOT_STABLE_YET, FLUSHING, STABLE_GENDER } State; /* Output state. */
    u32 EnabledGendersMask; /* Enabled gender bits and the high bit for disabling other queues. */
    u16 CurrentGender; /* One-based gender selection, or zero before selection. */
    u8 OnlyOneGender; /* Exactly one gender matches the enabled mask. */
    f32 *Buffers; /* Circular storage for frames awaiting gender selection. */
    f32 *pEnd; /* End of the circular frame buffer. */
    f32 *pRead; /* Next buffered frame to send downstream. */
    f32 *pWrite; /* Slot for the next input frame. */
    f32 *pAccumulatedDistortion; /* Accumulated score for each gender. */
    u32 NbrFramesInBuffer; /* Frames queued for output. */
    f32 SumWeights; /* Total activity weight used during gender selection. */
    s32 FrameCtr; /* Frame count used to scale each score update. */
    f32 GenderUpdateCtr; /* Activity weight since the last gender selection. */
    struct _CODEBOOK CodeBook; /* Language codebook used to score feature vectors. */
    u8 *pTakeInput; /* Input flags indexed by one-based gender. */
    u32 InputVectorSize; /* Upper input bin and width of each stored feature vector. */
    u32 FirstEntry; /* First input bin included in gender scoring. */
    u32 NbrGenders; /* Number of feature input queues. */
    u32 BufferSizeInFrames; /* Number of frame slots in the circular buffer. */
    u32 BufferWidth; /* Float count per frame, including all gender vectors. */
    s32 MaxNbrFramesForGender; /* Frame count limit for accumulating pAccumulatedDistortion. */
    s32 MinNbrFramesToUpdate; /* Weight required before choosing the output gender. */
    s32 MinNbrFramesForStableGender; /* Weight required for a fixed gender choice. */
    struct _LANGUAGE *pLang; /* Language data used by the classifier. */
} _GENDER;

static void ResetGender(_GENDER *This);

/* Saves the current gender estimator state into the session data. */
static void GenderGetSession(_GENDER *This, void *pSessionInfo)
{
    unsigned int i;
    u32 *header = pSessionInfo;
    float *pFlt = pSessionInfo;
    u32 *bufferedFrameCount;
    s32 *frameCount;

    pFlt += 2;
    header[0] = This->State;
    header[1] = This->CurrentGender;
    for (i = 0; i < This->NbrGenders; i++) {
        *pFlt++ = This->pAccumulatedDistortion[i];
    }

    bufferedFrameCount = (u32 *)pFlt;
    frameCount = (s32 *)pFlt;
    *bufferedFrameCount = This->NbrFramesInBuffer;
    pFlt[1] = This->SumWeights;
    frameCount[2] = This->FrameCtr;
    pFlt[3] = This->GenderUpdateCtr;
}

/* Restores the gender estimator state from the session data. */
static void GenderPutSession(_GENDER *This, void *pSessionInfo)
{
    unsigned int i;
    const u32 *header = pSessionInfo;
    const float *pFlt = pSessionInfo;
    const u32 *bufferedFrameCount;
    const s32 *frameCount;

    pFlt += 2;
    This->State = header[0];
    This->CurrentGender = header[1];
    for (i = 0; i < This->NbrGenders; i++) {
        This->pAccumulatedDistortion[i] = *pFlt++;
    }

    bufferedFrameCount = (const u32 *)pFlt;
    frameCount = (const s32 *)pFlt;
    This->NbrFramesInBuffer = *bufferedFrameCount;
    This->SumWeights = pFlt[1];
    This->FrameCtr = frameCount[2];
    This->GenderUpdateCtr = pFlt[3];
}

/* Loads the gender classifier's codebook from the language data. */
static u8 LoadCodeBook(_GENDER *This, struct _LANGUAGE *pLang)
{
    struct _CDBINFO *pCodeBookInfo;

    if (pLang->langCheckBitField(pLang, 4)) {
        pCodeBookInfo = pLang->langGetpCodeBook(
            pLang, pLang->langGetNbrCodeBook(pLang));
    } else {
        pCodeBookInfo = pLang->langGetpCodeBook(pLang, 1);
    }

    This->CodeBook.CodeBookDim = pLang->cdbGetCodeBookDim(pCodeBookInfo);
    This->CodeBook.FirstCdbSize = pLang->cdbGetFirstCdbSize(pCodeBookInfo);
    This->CodeBook.NbrInSecSearch =
        pLang->cdbGetNbrInSecSearch(pCodeBookInfo);
    This->CodeBook.SecondCdbSize =
        pLang->cdbGetSecondCdbSize(pCodeBookInfo);
    This->CodeBook.CompStart =
        (u8)pLang->cdbGetCompStart(pCodeBookInfo);
    This->CodeBook.pFirstCdb = (f32 *)pLang->cdbGetpFirstCdb(pCodeBookInfo);
    This->CodeBook.pIndexInSecCdb = (u32 *)pLang->cdbGetpIndexInSecCdb(pCodeBookInfo);
    This->CodeBook.pSecondCdb = (f32 *)pLang->cdbGetpSecondCdb(pCodeBookInfo);

    if (This->CodeBook.CodeBookDim > This->InputVectorSize - This->FirstEntry) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x66);
    }
    This->pLang = pLang;
    return 0;
}

/* Sets the enabled genders and handles a fixed selection. */
static void GenderFixedGender(_GENDER *This, u32 EnabledGendersMask)
{
    const struct _WARPINFO *pWarpFactors;
    u32 NrActiveGenders;
    u16 FixedGender;
    u8 OnlyOneGender;
    u32 i;

    pWarpFactors = This->pLang->langGetpWarpFactors(This->pLang);
    NrActiveGenders = 0;
    FixedGender = 0;
    for (i = 0; i < This->NbrGenders; pWarpFactors++, i++) {
        if (EnabledGendersMask & pWarpFactors->GenderMask) {
            FixedGender = i;
            NrActiveGenders++;
        }
    }

    OnlyOneGender = NrActiveGenders == 1;
    if (OnlyOneGender != This->OnlyOneGender) {
        ResetGender(This);
    }
    if (OnlyOneGender) {
        This->CurrentGender = FixedGender + 1;
        if (This->State == NOT_STABLE_YET) {
            This->State = FLUSHING;
        }
    }
    This->OnlyOneGender = OnlyOneGender;
    This->EnabledGendersMask = EnabledGendersMask;
}

/* Handles recognition controls and saved state for the gender classifier. */
static u8 ControlGender(
    struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    u8 RetVal = 1;
    _GENDER *This = (_GENDER *)_This;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    switch (ControlId) {
    case 0x03:
        if (This->NbrFramesInBuffer == 0 && CParam == 0) {
            break;
        }
        if (This->CurrentGender == 0) {
            This->GenderUpdateCtr = 1.0f + This->MinNbrFramesToUpdate;
        }
        This->State = FLUSHING;
        break;
    case 0x06:
        if (CParam != 0) {
            GenderPutSession(This, (u8 *)CParam + *(u32 *)DParam);
            *(u32 *)DParam += (This->NbrGenders + 6) * sizeof(u32);
        } else {
            ResetGender(This);
        }
        break;
    case 0x07:
        if (CParam != 0) {
            GenderGetSession(This, (u8 *)CParam + *(u32 *)DParam);
        }
        *(u32 *)DParam += (This->NbrGenders + 6) * sizeof(u32);
        break;
    case 0x04:
        RetVal = qQueueControl(
            This->BaseBlock.InputQueues->pQ, 7, This->BaseBlock.InputQueues->ReadPos,
            CParam);
        break;
    case 0x05:
        RetVal = qQueueControl(
            This->BaseBlock.InputQueues->pQ, 6, This->BaseBlock.InputQueues->ReadPos,
            CParam);
        break;
    case 0x08:
        GenderFixedGender(This, *(u32 *)CParam);
        break;
    case 0x09:
        *(u32 *)CParam = This->EnabledGendersMask;
        break;
    case 0x01:
        RetVal = !LoadCodeBook(This, (struct _LANGUAGE *)CParam);
        break;
    case 0x02:
        This->pTakeInput[CParam + 1] = 1;
        break;
    case 0x0A:
        This->pTakeInput[CParam + 1] = 0;
        break;
    case 0xFF:
        heap_Free(pTos->hHeap, This->pAccumulatedDistortion);
        heap_Free(pTos->hHeap, This->Buffers);
        heap_Free(pTos->hHeap, This->pTakeInput);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        RetVal = 0;
        break;
    }
    return RetVal;
}

/* Resets the gender score accumulators and frame counters. */
static void ResetGender(_GENDER *This)
{
    u32 i;

    This->State = NOT_STABLE_YET;
    if (This->OnlyOneGender) {
        This->State = FLUSHING;
    } else {
        This->CurrentGender = 0;
    }
    This->pRead = This->pWrite = This->Buffers;
    for (i = 0; i < This->NbrGenders; i++) {
        This->pAccumulatedDistortion[i] = 0.0f;
    }
    This->NbrFramesInBuffer = 0;
    This->SumWeights = 0.0f;
    This->FrameCtr = 0;
    This->GenderUpdateCtr = 0.0f;
}

/* Accumulates weighted codebook distances for the input feature vector. */
static void GenderDataCompute(_GENDER *This, f32 *pInput, f32 Weight)
{
    s32 j;
    f32 lambda;
    u32 i;

    This->SumWeights += Weight;
    This->GenderUpdateCtr += Weight;
    if (This->FrameCtr < This->MaxNbrFramesForGender) {
        This->FrameCtr++;
    }
    lambda = 1.0f / This->FrameCtr;
    pInput += This->FirstEntry;
    for (i = 0; i < This->NbrGenders; i++) {
        if (This->pTakeInput[i + 1]) {
            f32 *inend;
            u32 *pIndexInSecCdb;
            s32 BestFirstLevelCentroid = -1;
            f32 BestDistortion;
            f32 *cdbptr;

            j = 0;
            BestDistortion = FLT_MAX;
            inend = pInput + This->InputVectorSize - This->FirstEntry;
            cdbptr = This->CodeBook.pFirstCdb;
            while (j < This->CodeBook.FirstCdbSize) {
                f32 TempDistortion = 0.0f;
                f32 *inptr;
                f32 temp;
                inptr = pInput;
                while (inptr < inend) {
                    temp = *inptr - *cdbptr;
                    TempDistortion += temp * temp;
                    if (TempDistortion > BestDistortion) {
                        break;
                    }
                    inptr++;
                    cdbptr++;
                }
                if (inptr == inend) {
                    BestDistortion = TempDistortion;
                    BestFirstLevelCentroid = j;
                } else {
                    cdbptr += inend - inptr;
                }
                j++;
            }

            if (This->CodeBook.NbrInSecSearch != 0) {
                pIndexInSecCdb = This->CodeBook.pIndexInSecCdb +
                    This->CodeBook.NbrInSecSearch * BestFirstLevelCentroid;
                BestDistortion = FLT_MAX;
                for (j = 0;
                     j < This->CodeBook.NbrInSecSearch;
                     j++) {
                    f32 TempDistortion;
                    f32 *inptr;
                    f32 diff;
                    cdbptr = This->CodeBook.pSecondCdb + *pIndexInSecCdb;
                    inptr = pInput;
                    TempDistortion = 0.0f;
                    while (inptr < inend) {
                        diff = *inptr - *cdbptr;
                        inptr++;
                        cdbptr++;
                        TempDistortion +=
                            diff * diff;
                        if (TempDistortion > BestDistortion) {
                            break;
                        }
                    }
                    if (inptr == inend) {
                        BestDistortion = TempDistortion;
                    }
                    pIndexInSecCdb++;
                }
            }

            This->pAccumulatedDistortion[i] += lambda *
                (Weight * sqrtf(BestDistortion) - This->pAccumulatedDistortion[i]);
        }
        pInput += This->InputVectorSize;
    }
}

/* Reads the activity weight and feature inputs for the current frame. */
static void ReadFrame(_GENDER *This, void *pInputs, int NbrInputs)
{
    _GENDER *block = This;
    void **inputData;
    u32 input;
    f32 *frame;
    f32 activity;

    frame = block->pWrite;
    if (((void **)pInputs)[0] != NULL) {
        inputData = (void **)pInputs + 1;
        for (input = 1; input <= block->NbrGenders; input++) {
            if (block->pTakeInput[input]) {
                memcpy(frame, *inputData, block->InputVectorSize * sizeof(f32));
            }
            inputData++;
            frame += block->InputVectorSize;
        }

        activity = *(f32 *)((void **)pInputs)[0];
        if (!block->OnlyOneGender && activity > 0.001f) {
            GenderDataCompute(block, block->pWrite, activity);
        }
        block->NbrFramesInBuffer++;
        block->pWrite = frame == block->pEnd
            ? block->Buffers : frame;
    }
}

/* Selects the lowest-scoring gender and may disable the other gender queues. */
static u16 GenderSelection(_GENDER *This)
{
    /* Control IDs that disable competing output queues after selection. */
    static const u8 ControlID_Disable[] = { 0xD, 0xE, 0xF };
    u32 _Gender;
    f32 BestDistortion;
    u32 i;

    _Gender = 0;
    BestDistortion = FLT_MAX;
    for (i = 0; i < This->NbrGenders; i++) {
        if (This->pTakeInput[i + 1] &&
            This->pAccumulatedDistortion[i] < BestDistortion) {
            BestDistortion = This->pAccumulatedDistortion[i];
            _Gender = i;
        }
    }

    if (This->SumWeights > This->MinNbrFramesForStableGender &&
        This->SumWeights > 300.0f &&
        (This->EnabledGendersMask >> 31) != 0) {
        for (i = 0; i < This->NbrGenders; i++) {
            if (i != _Gender) {
                _tosControl(
                    &This->BaseBlock.Object, ControlID_Disable[i], i, _Gender);
            }
        }
        switch (_Gender + 1) {
        case 1:
            This->EnabledGendersMask = 1;
            break;
        case 2:
            This->EnabledGendersMask = 2;
            break;
        case 3:
            This->EnabledGendersMask = 4;
            break;
        default:
            return 0;
        }
        This->OnlyOneGender = 1;
        This->GenderUpdateCtr = 0.0f;
        This->State = FLUSHING;
        return _Gender + 1;
    } else {
        This->GenderUpdateCtr = 0.0f;
        return _Gender + 1;
    }
}

/* Outputs the current frame and advances the frame state. */
static inline void WriteFrameToOutput(_GENDER *This)
{
    f32 *pOutput;

    pOutput = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    if (pOutput == NULL) {
        return;
    }
    if (!This->OnlyOneGender &&
        (This->State == NOT_STABLE_YET ||
         This->GenderUpdateCtr > This->MinNbrFramesToUpdate)) {
        This->CurrentGender = GenderSelection(This);
    }
    memcpy(
        pOutput,
        This->pRead +
            (This->CurrentGender - 1) * This->InputVectorSize,
        This->InputVectorSize * sizeof(f32));
    This->pRead += This->BufferWidth;
    if (This->pRead == This->pEnd) {
        This->pRead = This->Buffers;
    }
    This->NbrFramesInBuffer--;
}

/* Initializes the gender analysis block from the profile. */
static u8 InitGender(struct _BLOCK *_This)
{
    int i;
    _GENDER *This = (_GENDER *)_This;
    _GENDER *pBlock = This;
    struct _TOS *pTos = pBlock->BaseBlock.Object.pTos;

    This->NbrGenders = This->BaseBlock.NrInputs - 1;
    This->FirstEntry = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 5, 1);
    This->InputVectorSize = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 6, 0);
    This->BufferSizeInFrames = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 1, 50);
    This->MaxNbrFramesForGender = (s16)_tosGetProfileU32(&This->BaseBlock.Object, 2, 600);
    This->MinNbrFramesToUpdate = (s16)_tosGetProfileU32(&This->BaseBlock.Object, 3, 10);
    This->MinNbrFramesForStableGender = (s16)_tosGetProfileU32(&This->BaseBlock.Object, 4, 20);
    This->BufferWidth = This->NbrGenders * This->InputVectorSize;
    This->Buffers =
        heap_Calloc(pTos->hHeap, This->BufferSizeInFrames * This->BufferWidth, sizeof(f32));
    This->pEnd = This->Buffers + This->BufferSizeInFrames * This->BufferWidth;
    This->pAccumulatedDistortion = heap_Alloc(pTos->hHeap, This->NbrGenders * sizeof(f32));
    This->pTakeInput = heap_Alloc(pTos->hHeap, This->NbrGenders + 1);
    if (This->Buffers == NULL || This->pAccumulatedDistortion == NULL ||
        This->pTakeInput == NULL) {
        /* Earlier allocations remain owned by the block. */
        return 1;
    }
    memset(This->pTakeInput, 1, This->NbrGenders + 1);
    This->BaseBlock.InputQueues[0].Size = sizeof(f32);
    This->BaseBlock.InputQueues[1].Size = This->InputVectorSize * sizeof(f32);
    This->BaseBlock.InputQueues[2].Size = This->InputVectorSize * sizeof(f32);
    This->BaseBlock.InputQueues[3].Size = This->InputVectorSize * sizeof(f32);
    This->BaseBlock.OutputQueues->Size = This->InputVectorSize * sizeof(f32);
    This->EnabledGendersMask = 0x7FFFFFFF; /* Enable all genders below the high bit. */
    ResetGender(This);
    return 0;
}

/* Buffers startup frames, scores gender candidates and emits frames according to the current
 * state. */
static void ProcessGender(
    struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    _GENDER *This = (_GENDER *)_This;

    switch (This->State) {
    case NOT_STABLE_YET:
        if (This->NbrFramesInBuffer == This->BufferSizeInFrames &&
            This->NbrFramesInBuffer != 0) {
            WriteFrameToOutput(This);
        }
        if (This->NbrFramesInBuffer < This->BufferSizeInFrames) {
            ReadFrame(This, pInputs, NbrInputs);
        }
        if (This->SumWeights > This->MinNbrFramesForStableGender) {
            This->State = FLUSHING;
        }
        break;
    case FLUSHING:
        if (This->NbrFramesInBuffer != 0) {
            WriteFrameToOutput(This);
        }
        if (((void **)pInputs)[0] != NULL) {
            ReadFrame(This, pInputs, NbrInputs);
        }
        if (This->NbrFramesInBuffer == 0) {
            This->State = STABLE_GENDER;
        }
        break;
    case STABLE_GENDER:
        if (This->NbrFramesInBuffer < This->BufferSizeInFrames) {
            ReadFrame(This, pInputs, NbrInputs);
        }
        if (This->NbrFramesInBuffer != 0) {
            WriteFrameToOutput(This);
        }
        break;
    }
}

/* Constructs a TOS block for gender analysis. */
struct _BLOCK *ConstructGender(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 4, 1, ProcessGender, InitGender,
        ControlGender, sizeof(_GENDER));
}
