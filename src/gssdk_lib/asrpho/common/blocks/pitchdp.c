/* Tracks the most likely pitch sequence using dynamic programming. */
#include "gssdk/blocks.h"
#include "gssdk/heap.h"
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>
#include "gssdk/mathusage.h"
#include "types.h"

#include "gssdk/tos.h"
#include "gssdk/mqueue.h"

/* Observation, cumulative score and predecessor for one pitch state. */
typedef struct {
    f32 Ampl; /* Observation for this pitch state and frame. */
    f32 AmplSum; /* Best cumulative score at this cell. */
    long BackTrack; /* Pitch state selected in the preceding frame. */
} PitchDPCell;

/* Buffers observations and the best path through successive pitch frames. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u16 Maxk; /* Number of pitch states. */
    u8 BufferLength; /* Fresh frames in each analysis window after startup. */
    u8 BufferOverlap; /* Frames overlapped on each side of the full window. */
    u8 NbrOfOldFrames_AllreadyFilledIn; /* Retained frames preceding the current input. */
    u32 Beam; /* Divisor used to set the pruning threshold below the best score. */
    u16 Neighbours; /* Pitch bins searched on either side of the current state. */
    f32 BufferID; /* Selects the first or subsequent pass through the pitch window. */
    u16 MaxPitchChange; /* Pitch-change rate used with frame duration to set transition costs. */
    u16 LineLength; /* Length of each pitch state history in frames. */
    f32 FrameDuration; /* Time represented by one pitch frame. */
    u16 NbrOfTotalFrames; /* Frames in the current dynamic-programming window. */
    u16 NbrOfFreshFrames; /* New frames required to run the next pass. */
    u16 NbrOfFreshFrames_AllreadyFilledIn; /* New frames already stored in the window. */
    u16 NbrToFlush; /* Number of frames covered by the flush pass. */
    u16 FlushCounter; /* Current output index during flush. */
    u8 FlushFlag; /* Allows input-free scheduler ticks to drain the pending pitch path. */
    u8 FirstFlush; /* Runs the final scoring pass before the first flush output. */
    u8 FirstDP; /* Seeds cumulative scores when the first window is complete. */
    PitchDPCell *pAllMeas; /* Contiguous pitch-state and frame histories. */
    PitchDPCell **ppMeas; /* Start of each pitch state history. */
    f32 *pIndexPitch; /* Pitch value corresponding to each state. */
    f32 **ppJumpFactor; /* Score weights for transitions between pitch states. */
    f32 *pOutDummy; /* Reconstructed pitch path awaiting output. */
} PitchDP;

/* Stores the current observation in each pitch state history. */
static void PreDP(float *pIn, PitchDP *This)
{
    unsigned short k;
    unsigned short Maxk = This->Maxk;
    PitchDPCell **ppMeas = This->ppMeas;
    unsigned short NbrOfFreshFrames_AllreadyFilledIn = This->NbrOfFreshFrames_AllreadyFilledIn;
    unsigned char NbrOfOldFrames_AllreadyFilledIn;
    if (This->BufferID == 2.0) {
        This->NbrOfTotalFrames = This->BufferLength + This->BufferOverlap * 2;
        This->NbrOfFreshFrames = This->BufferLength;
        This->NbrOfOldFrames_AllreadyFilledIn = This->BufferOverlap * 2;
    }
    NbrOfOldFrames_AllreadyFilledIn = This->NbrOfOldFrames_AllreadyFilledIn;
    for (k = 0; k < Maxk; k++) {
        ppMeas[k][NbrOfFreshFrames_AllreadyFilledIn +
                  NbrOfOldFrames_AllreadyFilledIn].Ampl = pIn[k];
    }
}

/* Finds the score threshold for retaining candidate states. */
static float ThresholdDetermination(PitchDP *This, unsigned short Frame)
{
    unsigned short j;
    unsigned short Maxk = This->Maxk;
    PitchDPCell *pMeas = This->pAllMeas + Frame;
    float Threshold = 0.0f;
    unsigned short LineLength = This->LineLength;
    for (j = 0; j < Maxk; j++, pMeas += LineLength) {
        if (Threshold < pMeas->AmplSum) {
            Threshold = pMeas->AmplSum;
        }
    }
    Threshold = Threshold - Threshold / This->Beam;
    return Threshold;
}

/* Scores each pitch state and records its best predecessor. */
static void Actual_DP(PitchDP *This, unsigned int Maxk, unsigned int NbrOfFrames)
{
    unsigned int j;
    PitchDPCell *pMeas_j;
    PitchDPCell *pMeas_k;
    float *pJumpFactor;
    unsigned int k;
    unsigned int Min;
    unsigned short Max;
    unsigned int Neighbours = This->Neighbours;
    float maxAmpl;
    unsigned int Frame;
    float Threshold;
    unsigned int LineLength = This->LineLength;
    PitchDPCell *pAllMeas = This->pAllMeas;
    float **ppJumpFactor = This->ppJumpFactor;
    PitchDPCell **ppMeas = This->ppMeas;
    for (Frame = 1; Frame < NbrOfFrames; Frame++) {
        Threshold = ThresholdDetermination(This, Frame - 1);
        for (j = 0; j < Maxk; j++) {
            pMeas_j = pAllMeas + j * LineLength + Frame - 1;
            if (pMeas_j->AmplSum > Threshold) {
                if (j < Neighbours) {
                    Min = 0;
                } else {
                    Min = (unsigned short)(j - Neighbours);
                }
                if (Maxk - j < Neighbours) {
                    Max = Maxk - 1;
                } else {
                    Max = j + Neighbours;
                }
                pJumpFactor = ppJumpFactor[j] + Min;
                for (k = Min; k < Max; k++, pJumpFactor++) {
                    if (*pJumpFactor > 0.0f) {
                        pMeas_k = ppMeas[k] + Frame;
                        maxAmpl = pMeas_j->AmplSum + *pJumpFactor * pMeas_k->Ampl;
                        if (maxAmpl > pMeas_k->AmplSum) {
                            pMeas_k->AmplSum = maxAmpl;
                            pMeas_k->BackTrack = j;
                        }
                    }
                }
            }
        }
    }
}

/* Follows winning predecessors to reconstruct the pitch path. */
static void BackTracking(PitchDP *This, unsigned int NbrOfTotalFrames)
{
    unsigned long Track;
    unsigned int Frame;
    float maxAmpl;
    PitchDPCell *pMeas;
    unsigned int k;
    unsigned int Maxk = This->Maxk;
    float *pOutDummy = This->pOutDummy;
    PitchDPCell **ppMeas = This->ppMeas;
    float *pIndexPitch = This->pIndexPitch;
    if (NbrOfTotalFrames > 1) {
        Frame = (unsigned short)(NbrOfTotalFrames - 1);
        maxAmpl = ppMeas[0][Frame].AmplSum;
        Track = 0;
        pOutDummy[Frame] = pIndexPitch[0];
        for (k = 1; k < Maxk; k++) {
            pMeas = ppMeas[k] + Frame;
            if (pMeas->AmplSum > maxAmpl) {
                maxAmpl = pMeas->AmplSum;
                Track = k;
                pOutDummy[Frame] = pIndexPitch[k];
            }
        }
        pMeas = ppMeas[Track] + NbrOfTotalFrames - 1;
        for (Frame = (unsigned short)(NbrOfTotalFrames - 1); Frame > 0;) {
            --Frame;
            pOutDummy[Frame] = pIndexPitch[pMeas->BackTrack];
            pMeas = ppMeas[pMeas->BackTrack] + Frame;
        }
    } else {
        pOutDummy[0] = 0.0f;
        maxAmpl = ppMeas[0]->Ampl;
        for (k = 1; k < Maxk; k++) {
            if (ppMeas[k]->Ampl > maxAmpl) {
                maxAmpl = ppMeas[k]->Ampl;
                pOutDummy[0] = pIndexPitch[k];
            }
        }
    }
}

/* Queues the pitch values from the reconstructed path. */
static void Output(PitchDP *This)
{
    u8 BufferLength = This->BufferLength;
    u8 i;
    f32 *pOutDummy = This->pOutDummy;
    u8 BufferOverlap = This->BufferOverlap;
    struct _BLOCK BaseBlock = This->BaseBlock;
    if (This->BufferID == 1.0) {
        for (i = 0; i < BufferLength; i++) {
            f32 *pOut = qEnQueueOne(BaseBlock.OutputQueues->pQ);
            *pOut = pOutDummy[i];
        }
    } else {
        for (i = 0; i < BufferLength; i++) {
            f32 *pOut = qEnQueueOne(BaseBlock.OutputQueues->pQ);
            *pOut = pOutDummy[BufferOverlap + i];
        }
    }
}

/* Retains observations shared by consecutive pitch windows. */
static void PostDP(PitchDP *This)
{
    unsigned int i;
    unsigned int j;
    unsigned int Maxk = This->Maxk;
    unsigned int BufferLength = This->BufferLength;
    unsigned int BufferOverlap = This->BufferOverlap;
    PitchDPCell **ppMeas = This->ppMeas;

    if (This->BufferID == 1.0) {
        for (i = 0; i < BufferOverlap * 2; i++) {
            for (j = 0; j < Maxk; j++) {
                ppMeas[j][i] = ppMeas[j][i + (BufferLength - BufferOverlap)];
            }
        }
    } else {
        for (i = 0; i < BufferOverlap * 2; i++) {
            for (j = 0; j < Maxk; j++) {
                ppMeas[j][i] = ppMeas[j][i + BufferLength];
            }
        }
    }
}

/* Completes pending output after input has stopped. */
static u8 FlushPitchDP(PitchDP *This)
{
    u8 bRetVal = 0;
    u32 Maxk = This->Maxk;
    u32 k;
    PitchDPCell **ppMeas;
    f32 *pOut;

    if (This->FirstFlush != 0) {
        if (This->BufferID == 1.0) {
            This->NbrToFlush = This->NbrOfFreshFrames_AllreadyFilledIn;
        } else {
            This->NbrToFlush = This->BufferOverlap + This->NbrOfFreshFrames_AllreadyFilledIn;
        }

        if (This->NbrToFlush == 0) {
            This->FlushFlag = 0;
            goto flushComplete;
        } else {
            ppMeas = This->ppMeas;
            for (k = 0; k < Maxk; k++) {
                (*ppMeas++)[0].BackTrack = 0;
            }
        }
        Actual_DP(This, Maxk, This->NbrToFlush);
        BackTracking(This, This->NbrToFlush);
        if (This->BufferID == 1.0) {
            This->BufferOverlap = 0;
        }
        This->FirstFlush = 0;
    }

    pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    *pOut = This->pOutDummy[This->BufferOverlap + This->FlushCounter];
    if (++This->FlushCounter == This->NbrToFlush) {
        This->FlushFlag = 0;
    } else {
        bRetVal = 1;
    }
flushComplete:
    return bRetVal;
}

/* Stores a frame of pitch observations or advances a flush. */
static void ProcessPitchDP(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    PitchDP *This = (PitchDP *)_This;
    unsigned int k;
    PitchDPCell **ppMeas;
    PitchDPCell *pMeas;
    unsigned int Maxk = This->Maxk;
    f32 *pIn = ((void **)pInputs)[0];
    unsigned int NbrOfTotalFrames = This->NbrOfTotalFrames;

    if (pIn != NULL) {
        PreDP(pIn, This);

        if (This->NbrOfFreshFrames_AllreadyFilledIn < This->NbrOfFreshFrames - 1) {
            This->NbrOfFreshFrames_AllreadyFilledIn++;
            return;
        }

        if (This->FirstDP != 0) {
            ppMeas = This->ppMeas;
            for (k = 0; k < Maxk; k++) {
                pMeas = *ppMeas++;

                pMeas->AmplSum = pMeas->Ampl;
                pMeas->BackTrack = 0;
            }
            This->FirstDP = 0;
        }

        Actual_DP(This, Maxk, NbrOfTotalFrames);
        BackTracking(This, NbrOfTotalFrames);
        Output(This);
        PostDP(This);
        This->NbrOfFreshFrames_AllreadyFilledIn = 0;
        This->BufferID += 1.0f;
    } else if (This->FlushFlag == 1) {
        FlushPitchDP(This);
    }
}

/* Resets frame and path state for a new sequence. */
static void PitchDPReset(PitchDP *This)
{
    PitchDPCell *frameCell;
    u16 LineLength = This->LineLength;
    u16 Maxk = This->Maxk;
    u16 j;
    PitchDPCell *pAllMeas = This->pAllMeas;

    This->BufferOverlap = (u8)_tosGetProfileU32(&This->BaseBlock.Object, 0x05, 6);
    This->NbrOfFreshFrames = This->NbrOfTotalFrames = This->BufferLength + This->BufferOverlap;
    This->NbrOfFreshFrames_AllreadyFilledIn = 0;
    This->BufferID = 1.0f;
    This->NbrOfOldFrames_AllreadyFilledIn = 0;
    This->NbrToFlush = 0;
    This->FlushFlag = 0;
    This->FirstFlush = 1;
    This->FirstDP = 1;
    This->FlushCounter = 0;

    for (j = 0; j < LineLength; j++) {
        u16 k;

        frameCell = &pAllMeas[j];
        for (k = 0; k < Maxk; k++) {
            frameCell[k * LineLength].AmplSum = 0.0f;
        }
    }
}

/* Handles reset, flush and destruction commands. */
static unsigned char ControlPitchDP(struct _BLOCK *_This, unsigned char ControlId,
                                    unsigned long CParam, unsigned long DParam)
{
    PitchDP *This = (PitchDP *)_This;

    switch (ControlId) {
    case 0xC9:
        PitchDPReset(This);
        break;
    case 0xC8:
        This->FlushFlag = 1;
        break;
    case 0xFF: {
        u16 i;
        u16 Maxk;
        struct _TOS *pTos;
        f32 **ppJumpFactor;

        Maxk = This->Maxk;
        ppJumpFactor = This->ppJumpFactor;
        pTos = This->BaseBlock.Object.pTos;

        if (This->pAllMeas != NULL) {
            heap_Free(pTos->hHeap, This->pAllMeas);
            heap_Free(pTos->hHeap, This->ppMeas);
            heap_Free(pTos->hHeap, This->pIndexPitch);
            heap_Free(pTos->hHeap, This->pOutDummy);
            for (i = 0; i < Maxk; i++) {
                heap_Free(pTos->hHeap, ppJumpFactor[i]);
            }
            heap_Free(pTos->hHeap, This->ppJumpFactor);
        }
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    }
    default:
        return 0;
    }
    return 1;
}

/* Initializes the cost factors for transitions between pitch states. */
static void InitJumpFactors(PitchDP *This)
{
    s32 k;
    s32 Maxk = This->Maxk;
    f32 *pIndexPitch = This->pIndexPitch;
    f32 **ppJumpFactor = This->ppJumpFactor;
    f32 MaxJump = This->MaxPitchChange * This->FrameDuration;

    for (k = 0; k < Maxk; k++) {
        s32 j;
        f32 score;
        ppJumpFactor[k][k] = 1.0f;
        for (j = k - 1; j >= 0; j--) {
            score = 1.0f -
                (pIndexPitch[k] - pIndexPitch[j]) / MaxJump *
                ((pIndexPitch[k] - pIndexPitch[j]) / MaxJump);

            ppJumpFactor[k][j] = score;
            if (ppJumpFactor[k][j] < 0.0f) {
                ppJumpFactor[k][j] = 0.0f;
                break;
            }
        }

        for (j = k + 1; j < Maxk; j++) {
            score = 1.0f -
                (pIndexPitch[j] - pIndexPitch[k]) / MaxJump *
                ((pIndexPitch[j] - pIndexPitch[k]) / MaxJump);

            ppJumpFactor[k][j] = score;
            if (ppJumpFactor[k][j] < 0.0f) {
                ppJumpFactor[k][j] = 0.0f;
                break;
            }
        }
    }
}

/* Loads pitch tracking settings and allocates its working buffers. */
static unsigned char InitPitchDP(struct _BLOCK *_This)
{
    PitchDP *This = (PitchDP *)_This;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    u32 i;
    u16 MaximumPitch;
    u8 NumberOfOctaves;
    u16 PointsPerOctave;
    u16 j;
    u16 MinimumPitch;

    PointsPerOctave = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x01, 50);
    NumberOfOctaves = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x0A, 5);
    MaximumPitch = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x02, 500);
    MinimumPitch = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x03, 50);
    This->BaseBlock.InputQueues->Size = NumberOfOctaves * PointsPerOctave * sizeof(f32);
    This->BaseBlock.OutputQueues->Size = sizeof(f32);
    This->Beam = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x06, 10);
    This->Neighbours = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x07, 10);
    This->MaxPitchChange = (u16)_tosGetProfileU32(&This->BaseBlock.Object, 0x08, 2000);
    This->BufferLength = (u8)_tosGetProfileU32(&This->BaseBlock.Object, 0x04, 15);
    This->BufferOverlap = (u8)_tosGetProfileU32(&This->BaseBlock.Object, 0x05, 6);
    This->LineLength = This->BufferLength + This->BufferOverlap * 2;
    This->FrameDuration = _tosGetProfileFloat(&This->BaseBlock.Object, 0x09, 0.01f);

    This->BufferID = 1.0f;
    This->NbrToFlush = 0;
    This->FlushCounter = 0;
    This->FlushFlag = 0;
    This->FirstFlush = 1;
    This->FirstDP = 1;
    j = This->BufferLength + This->BufferOverlap;
    This->NbrOfTotalFrames = j;
    This->NbrOfFreshFrames = j;
    This->NbrOfFreshFrames_AllreadyFilledIn = 0;
    This->NbrOfOldFrames_AllreadyFilledIn = 0;

    This->Maxk = (u16)(
        PointsPerOctave *
        logf_check((f32)MaximumPitch / (f32)MinimumPitch) /
        logf_check(2.0f));

    This->pAllMeas = heap_Calloc(pTos->hHeap, This->Maxk * This->LineLength, sizeof(PitchDPCell));
    This->ppMeas = heap_Calloc(pTos->hHeap, This->Maxk, sizeof(PitchDPCell *));
    This->pIndexPitch = heap_Calloc(pTos->hHeap, This->Maxk, sizeof(f32));
    This->ppJumpFactor = heap_Calloc(pTos->hHeap, This->Maxk, sizeof(f32 *));
    This->pOutDummy = heap_Calloc(
        pTos->hHeap, This->BufferLength + This->BufferOverlap * 2, sizeof(f32));
    if (This->pAllMeas == NULL || This->ppMeas == NULL ||
        This->ppJumpFactor == NULL || This->pIndexPitch == NULL ||
        This->pOutDummy == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 2);
        return 1;
    }

    for (i = 0; i < This->Maxk; i++) {
        This->ppMeas[i] = This->pAllMeas + This->LineLength * i;
        This->pIndexPitch[i] = MinimumPitch * powf(2.0f, (f32)i / (f32)PointsPerOctave);
        This->ppJumpFactor[i] = heap_Calloc(pTos->hHeap, This->Maxk, sizeof(f32));
        if (This->ppJumpFactor[i] == NULL) {
            _tosErrorLog(&This->BaseBlock.Object, 2);
            return 1;
        }
    }

    InitJumpFactors(This);
    return 0;
}

/* Registers the pitch tracker with the scheduler. */
struct _BLOCK *ConstructPitchDP(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 1, 1, ProcessPitchDP, InitPitchDP, ControlPitchDP, sizeof(PitchDP));
}
