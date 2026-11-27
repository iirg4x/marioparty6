/* Decodes isolated words by propagating and pruning tokens through a word tree. */
#include "gssdk/blocks.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "types.h"
#include "gssdk/tos.h"
#include "gssdk/mqueue.h"
#include <string.h>
#include <stdlib.h>

/* Circular list of active nodes in the current and next decoding frames. */
struct _WHEEL {
    struct _ISOWORD_NODE **pHead; /* Base of the allocated node-pointer array. */
    struct _ISOWORD_NODE **pFirstFrom; /* First active node to read for the current frame. */
    struct _ISOWORD_NODE **pFirstTo; /* First active node inserted for the next frame. */
    struct _ISOWORD_NODE **pTail; /* Last node-pointer entry before wrapping. */
    unsigned long WheelSize; /* Number of node-pointer entries allocated in the wheel. */
};

/* Counts active nodes in each score band. */
struct _HISTO {
    unsigned long *pHead; /* Count for each relative-score bin. */
    unsigned long NrBins; /* Number of relative-score bins allocated. */
};

/* Best score for each speech unit and count of updated entries. */
struct _BESTPHOSCORE {
    signed short *pHead; /* Best scores for the active-node pruning pass. */
    unsigned long Size; /* Number of score entries allocated. */
    long NrUpdated; /* Number of distinct score entries updated in this frame. */
};

/* State of the isolated-word token decoder. */
struct _ISOWORDTOKENDP {
    struct _BLOCK BaseBlock; /* Scheduler block and input queues. */
    void *pDistCalc; /* Object passed to the distance callbacks. */
    /* Prepares distances for the input frame. */
    void (*InitCalculateDistances)(void *, s16 *, u8 *, u8, int);
    s16 *(*GetpCachedDistances_DISTBLK)(void *); /* Returns the current distance cache. */
    s16 *pCachedDistances_DISTBLK; /* The game's code never reads this field. */
    s16 (*CalculateDistance)(void *, unsigned int); /* Calculates an uncached distance. */
    s32 NbrPhenomes; /* Number of entries in the distance cache. */
    const u32 *pLngSpeechUnitLex; /* Offsets in u16 units followed by the state IDs. */
    u16 LngNbrSpeechUnits; /* Number of speech units in the language lexicon. */
    struct _WHEEL Wheel; /* Current-frame and next-frame active nodes. */
    s16 BPthr; /* Back-pruning score threshold for active paths. */
    s16 BestScore; /* Best path score carried into the next frame. */
    s32 s32MaxNrActPho; /* Current upper bound on the active node count. */
    s32 s32Accuracy; /* Configured upper bound on the active node count. */
    s32 s32ReducedAccuracy; /* Upper bound on active nodes when decoding in silence. */
    struct _HISTO Histogram; /* Active-node counts by relative score. */
    struct _BESTPHOSCORE BestPhoScore; /* Score array used to prune active nodes. */
    struct _ISOWORD_NODE *pBestNode; /* Node carrying the best score for the frame. */
    s32 s32AccumulatedScore; /* Sum of the previous frame minima. */
    struct _ISOWORD_NODE *pNodeHead; /* Root node for the current word tree. */
    s32 NrNodes; /* The game's code never reads this field. */
    s32 NrExit; /* The game's code never reads this field. */
    s32 NrWordEndsInTree; /* The game's code never reads this field. */
    s16 BeamW; /* Largest relative score included in the beam. */
    u32 TSCount; /* Frame threshold for reporting terminal silence. */
    u32 OriTSCount; /* Terminal-silence duration before conversion to frames. */
    s32 MaxNBest; /* Maximum number of ranked word results retained. */
    u8 TSEnable; /* Enables reporting terminal silence from the frame process. */
    u32 uNrFrSinceLastSpeech; /* Number of frames since the best node last had a successor. */
    struct _DYNPROG_RESULT Result; /* Ranked results and garbage-model scores. */
    u32 FrameCounter; /* Number of processed frames in the current utterance. */
    u32 SingleWordGarbagePhenom; /* Distance index for the single-word garbage model. */
    u32 SentenceGarbagePhenom; /* Distance index for the sentence garbage model. */
    u16 *pUserWordLngSuLex; /* User-word speech-unit state IDs. */
    u32 *pFirstPhenomOfUwSu; /* Offsets of user-word speech units in their state-ID array. */
};

/* Scores and advances the active tokens for one input frame. */
static void IsoWordTokenDpProcess(struct _BLOCK *_This, void *pInputs, int NbrInputs);
/* Allocates or expands the circular node wheel. */
static u8 ExtendNodeWheel(struct _ISOWORDTOKENDP *This, struct _ISOWORD_NODE ***ppTo,
                          struct _ISOWORD_NODE ***ppFrom);
/* Resets the decoder and prepares its initial tokens. */
static u8 InitNewUtterance(struct _ISOWORDTOKENDP *This);
/* Sets the maximum number of results and replaces the result array. */
static u8 SetMaxNBest(struct _ISOWORDTOKENDP *This, unsigned int MaxNBest)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    This->MaxNBest = MaxNBest;
    if (This->Result.pResult != NULL) {
        heap_Free(pTos->hHeap, This->Result.pResult);
    }
    This->Result.pResult = heap_Alloc(pTos->hHeap, MaxNBest * sizeof(struct _WORDRESULT));
    if (This->Result.pResult == NULL) {
        return 1;
    } else {
        return 0;
    }
}

/* Initializes the isolated-word decoder from the block's profile. */
static u8 InitIsoWordTokenDpProcess(struct _BLOCK *_This)
{
    struct _ISOWORDTOKENDP *This = (struct _ISOWORDTOKENDP *) _This;
    u32 inputSize;

    if (SetMaxNBest(This, _tosGetProfileU32(&_This->Object, 0x07, 10)) != 0) {
        return 1;
    }
    This->TSCount = _tosGetProfileU32(&_This->Object, 0x06, 30);
    This->OriTSCount = This->TSCount * 10;
    inputSize = (u16) _tosGetProfileU32(&_This->Object, 0x0F, 4);
    _This->InputQueues[1].Size = inputSize;
    _This->InputQueues[0].Size = 1;
    return 0;
}

/* Counts the nodes in the tree down to the specified number of levels. */
static s32 CalcNrNodes(struct _ISOWORD_NODE *pNode, int NrLevels)
{
    u32 NrFol;
    s32 NrNodes;
    u32 i;
    struct _ISOWORD_NODE *pFol;

    NrNodes = 1;
    NrFol = pNode->NrFol;
    if (NrLevels - 1 > 0) {
        NrLevels--;
        pFol = pNode->u.pFirstFol;
        i = 0;
        while (i < NrFol) {
            NrNodes += CalcNrNodes(pFol, NrLevels);
            pFol++;
            i++;
        }
    }
    return NrNodes;
}

/* Quantizes the pruning beam width and allocates bins for active node counts. */
static int ChangeBeamW(struct _ISOWORDTOKENDP *This, s16 BeamW)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    unsigned long NrBins = BeamW / 32 + 1;

    if (NrBins > 10000) {
        NrBins = 10000;
    }
    This->BeamW = NrBins * 32 - 1;
    if (NrBins > This->Histogram.NrBins) {
        if (This->Histogram.pHead != NULL) {
            heap_Free(pTos->hHeap, This->Histogram.pHead);
        }
        This->Histogram.pHead = heap_Alloc(pTos->hHeap, NrBins * sizeof(*This->Histogram.pHead));
        if (This->Histogram.pHead == NULL) {
            return 1;
        }
        This->Histogram.NrBins = NrBins;
    }
    return 0;
}

/* Sets the tree root and prepares the token wheel and score storage. */
static u8 InitIsoWordTokenDpContext(struct _ISOWORDTOKENDP *This, struct _ISOWORD_NODE *pTree,
                                    const struct ISOWORD_USERWORD_LNG *pUserWordLng)
{
    struct _TOS *pTos;

    pTos = This->BaseBlock.Object.pTos;
    This->pNodeHead = pTree;
    if (pUserWordLng != NULL) {
        This->pUserWordLngSuLex = pUserWordLng->pUserWordLngSpeechUnitLex;
        This->pFirstPhenomOfUwSu = pUserWordLng->pFirstPhenomOfUserWordSpeechUnit;
    }
    This->s32ReducedAccuracy = CalcNrNodes(pTree, 4);
    This->BaseBlock.Process = IsoWordTokenDpProcess;
    This->s32Accuracy = _tosGetProfileU32(&This->BaseBlock.Object, 0x09, 3000);
    if (ChangeBeamW(This, _tosGetProfileU32(&This->BaseBlock.Object, 0x0B, 5000)) != 0) {
        return 1;
    }
    if (ExtendNodeWheel(This, NULL, NULL) != 0) {
        return 1;
    }
    This->BestPhoScore.pHead = heap_Calloc(pTos->hHeap, 300, sizeof(*This->BestPhoScore.pHead));
    if (This->BestPhoScore.pHead == NULL) {
        return 1;
    }
    This->BestPhoScore.Size = 300;
    if (InitNewUtterance(This) != 0) {
        return 1;
    }
    if (SetMaxNBest(This, This->MaxNBest) != 0) {
        return 1;
    }
    This->pCachedDistances_DISTBLK = This->GetpCachedDistances_DISTBLK(This->pDistCalc);
    return 0;
}

/* Returns the tokens used in this utterance to the free list. */
static void FreeUsedTokens(struct _ISOWORDTOKENDP *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    struct _ISOWORD_NODE **pFromP = This->Wheel.pFirstFrom;

    while (pFromP != This->Wheel.pFirstTo) {
        struct _FASTALLO_FREE_NODE **pTemp =
            &pTos->FastAllocHeap.pFreeNodes[((*pFromP)->pToken->u8LastState * 2 + 7) / 4U];
        ((struct _FASTALLO_FREE_NODE *) (*pFromP)->pToken)->pNext = *pTemp;
        *pTemp = (struct _FASTALLO_FREE_NODE *) (*pFromP)->pToken;
        (*pFromP)->pToken = NULL;
        if (pFromP < This->Wheel.pTail) {
            pFromP++;
        } else {
            pFromP = This->Wheel.pHead;
        }
    }
}

/* Releases the decoder's tokens and working arrays. */
static void FreeIsoWordTokenDp(struct _ISOWORDTOKENDP *This)
{
    struct _TOS *pTos;

    pTos = This->BaseBlock.Object.pTos;
    if (This->Wheel.pHead != NULL) {
        FreeUsedTokens(This);
        heap_Free(pTos->hHeap, This->Histogram.pHead);
        This->Histogram.pHead = NULL;
        This->Histogram.NrBins = 0;
        heap_Free(pTos->hHeap, This->BestPhoScore.pHead);
        This->BestPhoScore.pHead = NULL;
        This->BestPhoScore.Size = 0;
        heap_Free(pTos->hHeap, This->Wheel.pHead);
        This->Wheel.pHead = NULL;
        This->Wheel.WheelSize = 0;
    }
    if (This->Result.GarbageSize != 0) {
        heap_Free(pTos->hHeap, This->Result.pSingleWordGarbage);
        heap_Free(pTos->hHeap, This->Result.pSentenceGarbage);
        This->Result.GarbageSize = 0;
        This->Result.pSentenceGarbage = NULL;
        This->Result.pSingleWordGarbage = NULL;
    }
    if (This->Result.pResult != NULL) {
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->Result.pResult);
        This->Result.pResult = NULL;
    }
}

/* Resets the decoder and prepares the initial tokens for a new utterance. */
static u8 InitNewUtterance(struct _ISOWORDTOKENDP *This)
{
    struct _TOS *pTos;
    const u32 *pLngSpeechUnitLex;
    const u16 *pCurStateID;
    const u16 *pCurStateIDEnd;
    struct _ISOWORD_NODE **pFromP;
    u32 uLastState;
    struct _ISOWORD_NODE **pDummyP;

    pTos = This->BaseBlock.Object.pTos;
    pLngSpeechUnitLex = This->pLngSpeechUnitLex;
    pCurStateID = (const u16 *) pLngSpeechUnitLex + pLngSpeechUnitLex[This->pNodeHead->PhoID];
    pCurStateIDEnd =
        (const u16 *) pLngSpeechUnitLex + pLngSpeechUnitLex[This->pNodeHead->PhoID + 1];
    uLastState = pCurStateIDEnd - pCurStateID - 3;
    FreeUsedTokens(This);
    This->FrameCounter = -1;
    pFromP = This->Wheel.pFirstFrom;
    *pFromP = This->pNodeHead;
    FASTALLO_TAKE(&pTos->FastAllocHeap, (uLastState * 2 + 7) / 4,
                  &This->pNodeHead->pToken)
    This->pNodeHead->pToken->u8LastState = uLastState;
    This->pNodeHead->pToken->pScore[0] = 0;
    This->pNodeHead->pToken->u8LastActState = 0;
    pFromP = pFromP < This->Wheel.pTail ? pFromP + 1 : This->Wheel.pHead;
    if (pFromP == This->Wheel.pFirstFrom && ExtendNodeWheel(This, &pDummyP, &pFromP) != 0) {
        return 1;
    }
    This->Wheel.pFirstTo = pFromP;
    This->BestScore = 0;
    This->s32AccumulatedScore = 0;
    This->uNrFrSinceLastSpeech = 0;
    This->pBestNode = This->pNodeHead;
    This->BPthr = This->BeamW;
    This->s32MaxNrActPho = This->s32Accuracy;
    return 0;
}

/* Allocates or expands the node wheel and updates the caller's cursors. */
static u8 ExtendNodeWheel(struct _ISOWORDTOKENDP *This, struct _ISOWORD_NODE ***ppTo,
                          struct _ISOWORD_NODE ***ppFrom)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    unsigned long NewSize;
    s32 FirstFrom;
    s32 FirstTo;
    s32 Index;
    struct _ISOWORD_NODE **pNodeWheel;
    struct _ISOWORD_NODE **pNodeWheel2;
    struct _WHEEL *pWheel;
    struct _ISOWORD_NODE **pNew;

    pWheel = &This->Wheel;
    if (pWheel->pHead == NULL) {
        NewSize = 256;
        pWheel->pHead = heap_Alloc(pTos->hHeap, NewSize * sizeof(void *));
        if (pWheel->pHead == NULL) {
            return 1;
        }
        pWheel->pTail = pWheel->pHead + 255;
        pWheel->WheelSize = NewSize;
        pWheel->pFirstFrom = pWheel->pHead;
        pWheel->pFirstTo = pWheel->pFirstFrom;
    } else {
        pNodeWheel = pWheel->pHead;
        NewSize = pWheel->WheelSize + 200;
        FirstFrom = (pWheel->pFirstFrom - pNodeWheel);
        FirstTo = (pWheel->pFirstTo - pNodeWheel);
        Index = (s32) (*ppFrom - pNodeWheel);
        pNew = heap_Alloc(pTos->hHeap, NewSize * sizeof(void *));
        if (pNew == NULL) {
            return 1;
        }
        memcpy(pNew, pWheel->pHead, pWheel->WheelSize * sizeof(void *));
        heap_Free(pTos->hHeap, pWheel->pHead);
        pWheel->pHead = pNew;
        if (pNodeWheel != pWheel->pHead) {
            pWheel->pFirstFrom = pWheel->pHead + FirstFrom;
            pWheel->pFirstTo = pWheel->pHead + FirstTo;
            *ppFrom = pWheel->pHead + Index;
            *ppTo = *ppFrom;
        }
        pWheel->pTail = pWheel->pHead + NewSize - 1;
        pWheel->WheelSize = NewSize;

        if (Index < 200) {
            pNodeWheel = pWheel->pHead;
            pNodeWheel2 = pWheel->pTail - 199;
            while (pNodeWheel < *ppTo) {
                *pNodeWheel2++ = *pNodeWheel++;
            }
            if (pWheel->pFirstFrom < *ppTo) {
                pWheel->pFirstFrom += NewSize - 200;
            }
            if (pWheel->pFirstTo < *ppTo) {
                pWheel->pFirstTo += NewSize - 200;
            }
            *ppTo += NewSize - 200;
        } else {
            pNodeWheel = pWheel->pHead;
            pNodeWheel2 = pWheel->pTail - 199;
            while (pNodeWheel2 <= pWheel->pTail) {
                *pNodeWheel2++ = *pNodeWheel++;
            }
            pNodeWheel = pWheel->pHead + 200;
            pNodeWheel2 = pWheel->pHead;
            while (pNodeWheel < *ppTo) {
                *pNodeWheel2++ = *pNodeWheel++;
            }
            if (pWheel->pFirstFrom < pWheel->pHead + 200) {
                pWheel->pFirstFrom += NewSize - 200;
            } else if (pWheel->pFirstFrom < *ppTo) {
                pWheel->pFirstFrom -= 200;
            }
            if (pWheel->pFirstTo < pWheel->pHead + 200) {
                pWheel->pFirstTo += NewSize - 200;
            } else if (pWheel->pFirstTo < *ppTo) {
                pWheel->pFirstTo -= 200;
            }
            *ppTo -= 200;
        }
    }
    return 0;
}

/* Tightens the phoneme-relative pruning threshold when too many tokens survive. */
u8 RePruneToPho(struct _ISOWORDTOKENDP *This, struct _ISOWORD_NODE ***ppToP, s32 *pNrToCreated,
                s32 NrToPredicted, int BestScoreCurFr)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    s16 *pScore;
    s16 *pLastScore;
    s16 *pPhoScore;
    struct _ISOWORD_NODE **pToP;
    struct _ISOWORD_NODE *pCurNode;
    struct _ISOWORD_NODE **pToP2;
    struct _ISOWORD_TOKEN *pCurToken;
    u32 TargetNrToCreated;
    u32 i;
    u32 *Histo;
    u32 Accum = 0;
    u32 PrevAccum = 0;
    u32 LastBinIx;
    u32 NrToCreatedCopy = *pNrToCreated;
    u8 Reduced = 0;
    struct _HISTO *pHisto = &This->Histogram;
    struct _WHEEL *pWheel = &This->Wheel;
    struct _BESTPHOSCORE *pBestPhoScore = &This->BestPhoScore;
    f32 Offset;
    f32 Resolution;
    f32 MaxBinScore;
    f32 TotResolutionIncr;
    int BPthr;

    TargetNrToCreated = (0.95 * This->s32MaxNrActPho * *pNrToCreated) / NrToPredicted;
    if (TargetNrToCreated < 0.5 * *pNrToCreated) {
        TargetNrToCreated = 0.5 * *pNrToCreated;
        if (TargetNrToCreated < 1) {
            TargetNrToCreated = 1;
        }
        Reduced = 1;
    }
    LastBinIx = (u32) (This->BPthr - BestScoreCurFr) / 32;
    Histo = pHisto->pHead;
    if (LastBinIx + 1 > pHisto->NrBins) {
        u32 *pNew;

        pHisto->NrBins = LastBinIx + 1;
        pNew = heap_Alloc(pTos->hHeap, pHisto->NrBins * sizeof(u32));
        if (pNew == NULL) {
            return 1;
        }
        heap_Free(pTos->hHeap, pHisto->pHead);
        Histo = pNew;
        pHisto->pHead = Histo;
    }
    memset(Histo, 0, (LastBinIx + 1) * sizeof(u32));
    if (NrToCreatedCopy > pBestPhoScore->Size) {
        s16 *pNew = heap_Alloc(pTos->hHeap, NrToCreatedCopy * sizeof(s16));
        if (pNew == NULL) {
            return 1;
        }
        memcpy(pNew, pBestPhoScore->pHead, pBestPhoScore->Size * sizeof(s16));
        heap_Free(pTos->hHeap, pBestPhoScore->pHead);
        pBestPhoScore->pHead = pNew;
        pBestPhoScore->Size = NrToCreatedCopy;
    }
    if (pBestPhoScore->NrUpdated != 0) {
        pPhoScore = pBestPhoScore->pHead + pBestPhoScore->NrUpdated;
        pScore = pBestPhoScore->pHead;
        while (pScore < pPhoScore) {
            int RelScore = *pScore - BestScoreCurFr;
            Histo[RelScore >> 5]++;
            pScore++;
        }
        if (pWheel->pTail - pWheel->pFirstTo >= pBestPhoScore->NrUpdated) {
            pToP = pWheel->pFirstTo + pBestPhoScore->NrUpdated;
        } else {
            pToP =
                pWheel->pHead + pBestPhoScore->NrUpdated - (pWheel->pTail - pWheel->pFirstTo) - 1;
        }
    } else {
        pPhoScore = pBestPhoScore->pHead;
        pToP = pWheel->pFirstTo;
    }
    while (pToP != *ppToP) {
        s16 PhoScore;
        int RelScore;
        pCurNode = *pToP;
        pCurToken = pCurNode->pToken;
        pScore = pCurToken->pScore;
        pLastScore = pScore + pCurToken->u8LastActState;
        PhoScore = *pScore++;
        while (pScore <= pLastScore) {
            if (*pScore < PhoScore) {
                PhoScore = *pScore;
            }
            pScore++;
        }
        RelScore = PhoScore - BestScoreCurFr;
        Histo[RelScore >> 5]++;
        *pPhoScore++ = PhoScore;
        pToP = pToP < pWheel->pTail ? pToP + 1 : pWheel->pHead;
    }
    i = 0;
    while (i <= LastBinIx && Accum < TargetNrToCreated) {
        PrevAccum = Accum;
        Accum += Histo[i];
        i++;
    }
    Offset = 0.0f;
    Resolution = 32.0f;
    TotResolutionIncr = 1.0f;
    while (Histo[i - 1] > 0.2 * TargetNrToCreated && Resolution > 1.0f) {
        TotResolutionIncr *= 100.0f;
        Offset += Resolution * (i - 1);
        MaxBinScore = Offset + Resolution;
        Resolution /= 100.0f;
        if (pHisto->NrBins < 100) {
            u32 *pNew = heap_Alloc(pTos->hHeap, 100 * sizeof(*Histo));
            if (pNew == NULL) {
                return 1;
            }
            heap_Free(pTos->hHeap, pHisto->pHead);
            pHisto->pHead = pNew;
            Histo = pNew;
            pHisto->NrBins = 100;
        }
        memset(Histo, 0, 100 * sizeof(*Histo));
        pScore = pBestPhoScore->pHead;
        pLastScore = pScore + NrToCreatedCopy;
        while (pScore < pLastScore) {
            int RelScore = *pScore - BestScoreCurFr;
            if (RelScore >= Offset && RelScore < MaxBinScore) {
                Histo[(s16) (TotResolutionIncr * (RelScore - Offset)) >> 5]++;
            }
            pScore++;
        }
        i = 0;
        Accum = PrevAccum;
        while (i < 100 && Accum < TargetNrToCreated) {
            PrevAccum = Accum;
            Accum += Histo[i];
            i++;
        }
    }
    BPthr =
        BestScoreCurFr +
        (s32) (Offset + Resolution * (i - (f32) (Accum - TargetNrToCreated) / (Accum - PrevAccum)));
    if (BPthr < (-33792 > BestScoreCurFr ? -33792 : BestScoreCurFr)) {
        BPthr = -33792 > BestScoreCurFr + 1 ? -33792 : BestScoreCurFr + 1;
    }
    This->BPthr = BPthr;
    pToP = pWheel->pFirstTo;
    pToP2 = pToP;
    pPhoScore = pBestPhoScore->pHead;
    while (pToP != *ppToP) {
        if (*pPhoScore < BPthr) {
            *pToP2 = *pToP;
            pToP2 = pToP2 < pWheel->pTail ? pToP2 + 1 : pWheel->pHead;
        } else {
            struct _FASTALLO_FREE_NODE **pTemp;
            pCurNode = *pToP;
            pTemp = &pTos->FastAllocHeap
                         .pFreeNodes[((pCurNode->pToken->u8LastState * 2 + 7) & ~3U) / 4];
            ((struct _FASTALLO_FREE_NODE *) pCurNode->pToken)->pNext = *pTemp;
            *pTemp = (struct _FASTALLO_FREE_NODE *) pCurNode->pToken;
            pCurNode->pToken = NULL;
            (*pNrToCreated)--;
        }
        pToP = pToP < pWheel->pTail ? pToP + 1 : pWheel->pHead;
        pPhoScore++;
    }
    if (pWheel->pTail - pWheel->pFirstTo >= *pNrToCreated) {
        *ppToP = pWheel->pFirstTo + *pNrToCreated;
    } else {
        *ppToP = pWheel->pHead + *pNrToCreated - (pWheel->pTail - pWheel->pFirstTo) - 1;
    }
    if (Reduced || *pNrToCreated > TargetNrToCreated / 0.95) {
        pPhoScore = pBestPhoScore->pHead;
        pLastScore = pPhoScore + NrToCreatedCopy;
        pScore = pPhoScore;
        while (pScore < pLastScore) {
            if (*pScore < BPthr) {
                *pPhoScore++ = *pScore;
            }
            pScore++;
        }
        pBestPhoScore->NrUpdated = *pNrToCreated;
    } else {
        pBestPhoScore->NrUpdated = 0;
    }
    return 0;
}

/* Advances paths and prunes them against the back-pruning score threshold. */
static u8 FrameDPacoust(struct _ISOWORDTOKENDP *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    struct _ISOWORD_NODE **pFromP;
    struct _ISOWORD_NODE **pToP;
    struct _ISOWORD_NODE **pWheelFirstTo;
    struct _ISOWORD_NODE **pWheelTail;
    struct _ISOWORD_TOKEN *pCurToken;
    struct _ISOWORD_TOKEN *pFolToken;
    struct _ISOWORD_NODE *pCurNode;
    struct _ISOWORD_NODE *pFolNode;
    const u16 *pCurStateID;
    const u16 *pCurStateIDEnd;
    int BestScore;
    int tempd;
    s16 *pCurScore;
    unsigned int CurState;
    unsigned int CurActivated;
    long TotNrFrom;
    long NrToCreated;
    long NrFromTreated;
    long NrToPredicted;
    long TestNr;
    struct _WHEEL *pWheel = &This->Wheel;
    s16 *pCachedDistances_DISTBLK = This->GetpCachedDistances_DISTBLK(This->pDistCalc);
    const u32 *pLngSpeechUnitLex = This->pLngSpeechUnitLex;
    u32 *pFirstPhenomOfUwSu = This->pFirstPhenomOfUwSu;
    u16 *pUserWordLngSuLex = This->pUserWordLngSuLex;
    u16 LngNbrSpeechUnits = This->LngNbrSpeechUnits;
    u16 UserWordSuID;
    int BPthr = This->BeamW < This->BPthr + 100 ? This->BeamW : This->BPthr + 100;

    This->BPthr = BPthr;
    This->BestPhoScore.NrUpdated = 0;
    pWheelFirstTo = pWheel->pFirstTo;
    pWheelTail = pWheel->pTail;
    BestScore = 0x7BFF;
    if (pWheelFirstTo > pWheel->pFirstFrom)
        TotNrFrom = pWheelFirstTo - pWheel->pFirstFrom;
    else
        TotNrFrom = pWheel->WheelSize - (pWheel->pFirstFrom - pWheelFirstTo);
    NrFromTreated = 0;
    NrToCreated = 0;
    TestNr = This->s32MaxNrActPho < 300 ? This->s32MaxNrActPho : 300;
    pFromP = pWheel->pFirstFrom;
    pToP = pWheelFirstTo;
    while (pFromP != pWheelFirstTo) {
        NrFromTreated++;
        pCurNode = *pFromP;
        pCurToken = pCurNode->pToken;
        CurState = pCurToken->u8LastActState;
        CurActivated = 0;
        if (pCurNode->PhoID < LngNbrSpeechUnits) {
            pCurStateID =
                (const u16 *) pLngSpeechUnitLex + pLngSpeechUnitLex[pCurNode->PhoID] + CurState + 2;
            pCurStateIDEnd =
                (const u16 *) pLngSpeechUnitLex + pLngSpeechUnitLex[pCurNode->PhoID + 1] - 1;
        } else {
            UserWordSuID = pCurNode->PhoID - LngNbrSpeechUnits - 1;
            pCurStateID = pUserWordLngSuLex + pFirstPhenomOfUwSu[UserWordSuID] + CurState;
            pCurStateIDEnd = pUserWordLngSuLex + pFirstPhenomOfUwSu[UserWordSuID + 1] - 1;
        }
        pCurScore = pCurToken->pScore + CurState;
        if (pCurStateID == pCurStateIDEnd) {
            if (pCurNode->NrFol) {
                struct _ISOWORD_NODE *pFolNodeEnd;
                pFolNode = pCurNode->u.pFirstFol;
                pFolNodeEnd = pFolNode + pCurNode->NrFol;
                do {
                    unsigned int Phenom;
                    if (pFolNode->PhoID < LngNbrSpeechUnits)
                        Phenom = ((
                            const u16 *) pLngSpeechUnitLex)[pLngSpeechUnitLex[pFolNode->PhoID] + 2];
                    else {
                        UserWordSuID = pFolNode->PhoID - LngNbrSpeechUnits - 1;
                        Phenom = pUserWordLngSuLex[pFirstPhenomOfUwSu[UserWordSuID]];
                    }
                    tempd =
                        *pCurScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                          ? (pCachedDistances_DISTBLK[Phenom] =
                                                 This->CalculateDistance(This->pDistCalc, Phenom))
                                          : pCachedDistances_DISTBLK[Phenom]);
                    if (tempd < BPthr) {
                        pFolToken = pFolNode->pToken;
                        if (pFolToken) {
                            if (tempd < pFolToken->pScore[0]) {
                                pFolToken->pScore[0] = tempd;
                                if (tempd < BestScore) {
                                    BestScore = tempd;
                                    This->pBestNode = pFolNode;
                                }
                            }
                        } else {
                            unsigned int LastStateOfPho;
                            if (pFolNode->PhoID < LngNbrSpeechUnits)
                                LastStateOfPho = pLngSpeechUnitLex[pFolNode->PhoID + 1] -
                                                 pLngSpeechUnitLex[pFolNode->PhoID] - 3;
                            else {
                                UserWordSuID = pFolNode->PhoID - LngNbrSpeechUnits - 1;
                                LastStateOfPho = pFirstPhenomOfUwSu[UserWordSuID + 1] -
                                                 pFirstPhenomOfUwSu[UserWordSuID] - 1;
                            }
                            FASTALLO_TAKE(&pTos->FastAllocHeap,
                                          (LastStateOfPho * 2 + 7) / 4, &pFolToken)
                            pFolToken->u8LastState = LastStateOfPho;
                            pFolNode->pToken = pFolToken;
                            *pToP = pFolNode;
                            pFolToken->pScore[0] = tempd;
                            pFolToken->u8LastActState = 0;
                            if (tempd < BestScore) {
                                BestScore = tempd;
                                This->pBestNode = pFolNode;
                            }
                            NrToCreated++;
                            pToP = pToP < pWheelTail ? pToP + 1 : pWheel->pHead;
                            if (pToP == pFromP) {
                                /* This growth failure returns success to the caller. */
                                if (ExtendNodeWheel(This, &pToP, &pFromP))
                                    return 0;
                                pWheelTail = pWheel->pTail;
                                pWheelFirstTo = pWheel->pFirstTo;
                            }
                            if (NrToCreated >= TestNr) {
                                NrToPredicted = NrToCreated * TotNrFrom / NrFromTreated;
                                if (NrToPredicted >= This->s32MaxNrActPho) {
                                    if (RePruneToPho(This, &pToP, &NrToCreated, NrToPredicted,
                                                     BestScore))
                                        return 1;
                                    BPthr = This->BPthr;
                                }
                                TestNr = NrToCreated + 200 < 300 ? 300 : NrToCreated + 200;
                                TestNr =
                                    This->s32MaxNrActPho < TestNr ? This->s32MaxNrActPho : TestNr;
                            }
                        }
                    }
                    pFolNode++;
                } while (pFolNode < pFolNodeEnd);
            }
        } else {
            unsigned int Phenom = pCurStateID[1];
            s16 distance = pCachedDistances_DISTBLK[Phenom];
            if (distance == 0x7FFF) {
                distance = This->CalculateDistance(This->pDistCalc, Phenom);
                pCachedDistances_DISTBLK[Phenom] = distance;
            }
            tempd = *pCurScore + distance;
            if (tempd < BPthr) {
                *pToP = pCurNode;
                pCurToken->u8LastActState++;
                CurActivated = 1;
                if (tempd < BestScore) {
                    BestScore = tempd;
                    This->pBestNode = pCurNode;
                }
                NrToCreated++;
                pToP = pToP < pWheelTail ? pToP + 1 : pWheel->pHead;
                if (pToP == pFromP) {
                    if (ExtendNodeWheel(This, &pToP, &pFromP))
                        return 1;
                    pWheelTail = pWheel->pTail;
                    pWheelFirstTo = pWheel->pFirstTo;
                }
                pCurScore[1] = tempd;
            }
        }
        while (CurState) {
            unsigned int Phenom = *pCurStateID;
            tempd = (pCurScore[-1] < *pCurScore ? pCurScore[-1] : *pCurScore) +
                    (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                         ? (pCachedDistances_DISTBLK[Phenom] =
                                This->CalculateDistance(This->pDistCalc, Phenom))
                         : pCachedDistances_DISTBLK[Phenom]);
            if (tempd < BPthr) {
                if (!CurActivated) {
                    *pToP = pCurNode;
                    pCurToken->u8LastActState = CurState;
                    CurActivated = 1;
                    NrToCreated++;
                    pToP = pToP < pWheelTail ? pToP + 1 : pWheel->pHead;
                    if (pToP == pFromP) {
                        if (ExtendNodeWheel(This, &pToP, &pFromP))
                            return 1;
                        pWheelTail = pWheel->pTail;
                        pWheelFirstTo = pWheel->pFirstTo;
                    }
                }
                *pCurScore = tempd;
                if (tempd < BestScore) {
                    BestScore = tempd;
                    This->pBestNode = pCurNode;
                }
            } else
                *pCurScore = 0x7BFF;
            pCurScore--;
            pCurStateID--;
            CurState--;
        }
        tempd = *pCurScore + (pCachedDistances_DISTBLK[*pCurStateID] == 0x7FFF
                                  ? (pCachedDistances_DISTBLK[*pCurStateID] =
                                         This->CalculateDistance(This->pDistCalc, *pCurStateID))
                                  : pCachedDistances_DISTBLK[*pCurStateID]);
        if (tempd < BPthr) {
            if (!CurActivated) {
                *pToP = pCurNode;
                pCurToken->u8LastActState = 0;
                NrToCreated++;
                pToP = pToP < pWheelTail ? pToP + 1 : pWheel->pHead;
                if (pToP == pFromP) {
                    if (ExtendNodeWheel(This, &pToP, &pFromP))
                        return 1;
                    pWheelTail = pWheel->pTail;
                    pWheelFirstTo = pWheel->pFirstTo;
                }
            }
            *pCurScore = tempd;
            if (tempd < BestScore) {
                BestScore = tempd;
                This->pBestNode = pCurNode;
            }
        } else if (CurActivated)
            *pCurScore = 0x7BFF;
        else {
            struct _FASTALLO_FREE_NODE **pTemp =
                &pTos->FastAllocHeap.pFreeNodes[(pCurNode->pToken->u8LastState * 2 + 7) / 4U];
            ((struct _FASTALLO_FREE_NODE *) pCurNode->pToken)->pNext = *pTemp;
            *pTemp = (struct _FASTALLO_FREE_NODE *) pCurNode->pToken;
            pCurNode->pToken = NULL;
        }
        if (NrToCreated >= TestNr) {
            NrToPredicted = NrToCreated * TotNrFrom / NrFromTreated;
            if (NrToPredicted >= This->s32MaxNrActPho) {
                if (RePruneToPho(This, &pToP, &NrToCreated, NrToPredicted, BestScore))
                    return 1;
                BPthr = This->BPthr;
            }
            TestNr = NrToCreated + 200 < 300 ? 300 : NrToCreated + 200;
            TestNr = This->s32MaxNrActPho < TestNr ? This->s32MaxNrActPho : TestNr;
        }
        pFromP = pFromP < pWheelTail ? pFromP + 1 : pWheel->pHead;
    }
    pWheel->pFirstFrom = pFromP;
    pWheel->pFirstTo = pToP;
    This->BestScore = BestScore;
    return 0;
}

/* Grows the working arrays used to score garbage paths. */
static u8 ExpandGarbageArrays(struct _ISOWORDTOKENDP *This)
{
    u8 bRetVal;
    unsigned long *pNewSingleWordGarbage;
    unsigned long *pNewSentenceGarbage;

    bRetVal = 0;
    pNewSingleWordGarbage =
        heap_Alloc(This->BaseBlock.Object.pTos->hHeap,
                   (This->Result.GarbageSize + 0x97) * sizeof(*This->Result.pSingleWordGarbage));
    if (pNewSingleWordGarbage == NULL) {
        bRetVal = 1;
    } else {
        pNewSentenceGarbage =
            heap_Alloc(This->BaseBlock.Object.pTos->hHeap,
                       (This->Result.GarbageSize + 0x97) * sizeof(*This->Result.pSentenceGarbage));
        if (pNewSentenceGarbage == NULL) {
            bRetVal = 1;
        } else {
            if (This->Result.GarbageSize != 0) {
                memcpy(pNewSingleWordGarbage, This->Result.pSingleWordGarbage,
                       (This->Result.GarbageSize + 1) * sizeof(*pNewSingleWordGarbage));
                heap_Free(This->BaseBlock.Object.pTos->hHeap, This->Result.pSingleWordGarbage);
                memcpy(pNewSentenceGarbage, This->Result.pSentenceGarbage,
                       (This->Result.GarbageSize + 1) * sizeof(*pNewSentenceGarbage));
                heap_Free(This->BaseBlock.Object.pTos->hHeap, This->Result.pSentenceGarbage);
            } else {
                pNewSingleWordGarbage[0] = 0;
                pNewSentenceGarbage[0] = 0;
            }
            This->Result.pSingleWordGarbage = pNewSingleWordGarbage;
            This->Result.pSentenceGarbage = pNewSentenceGarbage;
            This->Result.GarbageSize += 0x96;
        }
    }
    return bRetVal;
}

/* Leaves input frames unprocessed while isolated-word recognition is idle. */
static void IdleProcess(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
}

/* Scores and advances the active tokens for one input frame. */
static void IsoWordTokenDpProcess(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    struct _ISOWORDTOKENDP *This = (struct _ISOWORDTOKENDP *) _This;
    u8 *pLabels = ((void **) pInputs)[1];
    u32 NbrPhenoms = This->NbrPhenomes;

    if (pLabels != NULL) {
        s16 *pCachedDistances_DISTBLK = This->GetpCachedDistances_DISTBLK(This->pDistCalc);
        s16 sOldMin = This->BestScore;
        u8 *pSpeech = ((void **) pInputs)[0];
        u8 Speech = 1;
        u32 i;

        This->FrameCounter++;
        if (pSpeech != NULL) {
            Speech = *pSpeech;
        }
        This->s32AccumulatedScore += sOldMin;
        for (i = 0; i < NbrPhenoms; i++) {
            pCachedDistances_DISTBLK[i] = 0x7FFF;
        }
        This->InitCalculateDistances(This->pDistCalc, pCachedDistances_DISTBLK, pLabels, Speech,
                                     sOldMin);
        if (Speech == 0 && This->pBestNode == This->pNodeHead) {
            This->s32MaxNrActPho = This->s32Accuracy < This->s32ReducedAccuracy
                                       ? This->s32Accuracy
                                       : This->s32ReducedAccuracy;
        } else if (This->s32MaxNrActPho == This->s32ReducedAccuracy) {
            This->s32MaxNrActPho = This->s32Accuracy;
            This->BPthr = This->BeamW;
        }
        if (FrameDPacoust(This) == 0 &&
            (This->FrameCounter < This->Result.GarbageSize || ExpandGarbageArrays(This) == 0)) {
            This->Result.pSingleWordGarbage[This->FrameCounter + 1] =
                This->Result.pSingleWordGarbage[This->FrameCounter] +
                (pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom] == 0x7FFF
                     ? (pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom] =
                            This->CalculateDistance(This->pDistCalc, This->SingleWordGarbagePhenom))
                     : pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom]) +
                sOldMin;

            This->Result.pSentenceGarbage[This->FrameCounter + 1] =
                This->Result.pSentenceGarbage[This->FrameCounter] +
                (pCachedDistances_DISTBLK[This->SentenceGarbagePhenom] == 0x7FFF
                     ? (pCachedDistances_DISTBLK[This->SentenceGarbagePhenom] =
                            This->CalculateDistance(This->pDistCalc, This->SentenceGarbagePhenom))
                     : pCachedDistances_DISTBLK[This->SentenceGarbagePhenom]) +
                sOldMin;

            if (This->pBestNode != NULL && This->pBestNode->NrFol == 0) {
                This->uNrFrSinceLastSpeech++;
            } else {
                This->uNrFrSinceLastSpeech = 0;
            }
            if (This->TSEnable != 0 && This->uNrFrSinceLastSpeech >= This->TSCount) {
                This->BaseBlock.Process = IdleProcess;
                _tosControl(&This->BaseBlock.Object, 0x08, This->FrameCounter, 0);
            }
        }
    }
}

/* Orders isolated-word recognition results by score. */
static int ScoreCompare(const void *elem1, const void *elem2)
{
    const struct _WORDRESULT *pElem1 = elem1;
    const struct _WORDRESULT *pElem2 = elem2;

    if (pElem1->Score < pElem2->Score) {
        return -1;
    }
    if (pElem1->Score > pElem2->Score) {
        return 1;
    }
    return 0;
}

/* Collects the best-scoring isolated-word results. */
static u32 GetNBest(struct _ISOWORDTOKENDP *This, u32 N)
{
    struct _ISOWORD_NODE **pFromP;
    long score;
    struct _ISOWORD_NODE *pCurNode;
    long WordID;
    unsigned long MaxScoreIx;
    unsigned long FreeIx;
    long MaxScore;
    unsigned long i;
    struct _WHEEL *pWheel;
    struct _WORDRESULT *pResult;

    pWheel = &This->Wheel;
    MaxScore = -0x8400;
    MaxScoreIx = 0;
    FreeIx = 0;
    pResult = (struct _WORDRESULT *) This->Result.pResult;
    pFromP = pWheel->pFirstFrom;
    for (; pFromP != pWheel->pFirstTo;) {
        pCurNode = *pFromP;
        if (pCurNode->NrFol == 0 && pCurNode != This->pNodeHead) {
            if (pCurNode->pToken->u8LastActState == pCurNode->pToken->u8LastState) {
                WordID = pCurNode->u.WordID;
                score = pCurNode->pToken->pScore[pCurNode->pToken->u8LastActState] +
                    This->s32AccumulatedScore;
                if (FreeIx < N) {
                    pResult[FreeIx].Score = score;
                    pResult[FreeIx].WordID = WordID;
                    FreeIx++;
                } else {
                    if (MaxScore == -0x8400) {
                        for (i = 0; i < N; i++) {
                            if (pResult[i].Score > MaxScore) {
                                MaxScore = pResult[i].Score;
                                MaxScoreIx = i;
                            }
                        }
                    }
                    if (score < MaxScore) {
                        pResult[MaxScoreIx].Score = score;
                        MaxScore = -0x8400;
                        pResult[MaxScoreIx].WordID = WordID;
                    }
                }
            }
        }
        pFromP = pFromP < pWheel->pTail ? pFromP + 1 : pWheel->pHead;
    }
    if (FreeIx != 0) {
        qsort(pResult, FreeIx, sizeof(*pResult), ScoreCompare);
    }
    return FreeIx;
}

/* Reads the speech-unit lexicon and distance indices for the two garbage models. */
static u8 InitIsoWordTokenDpLanguage(struct _ISOWORDTOKENDP *This, struct _LANGUAGE *pLang)
{
    unsigned int NbrPhenomes = pLang->langGetNbrPhenome(pLang);
    This->NbrPhenomes = NbrPhenomes;
    This->SingleWordGarbagePhenom = (u16) pLang->langGetSingleWordGarbagePhenome(pLang);
    This->SentenceGarbagePhenom = (u16) pLang->langGetSentenceGarbagePhenome(pLang);
    This->pLngSpeechUnitLex = pLang->langGetpSpeechUnit(pLang);
    This->LngNbrSpeechUnits = pLang->langGetNbrSpeechUnit(pLang);
    return 0;
}

/* Handles scheduler setup, queue control and teardown for isolated-word recognition. */
static u8 ControlIsoWordTokenDp(struct _BLOCK *_This, u8 ControlId, u32 CParam1, u32 CParam2)
{
    struct _ISOWORDTOKENDP *This = (struct _ISOWORDTOKENDP *) _This;

    switch (ControlId) {
    case 0x25: {
        struct _INPUTQUEUE *port = This->BaseBlock.InputQueues;

        return qQueueControl(port->pQ, 0x07, port->ReadPos, CParam1);
    }
    case 0x26: {
        struct _INPUTQUEUE *port = This->BaseBlock.InputQueues;

        return qQueueControl(port->pQ, 0x06, port->ReadPos, 0);
    }
    case 0x04: {
        unsigned int RealN = GetNBest(This, This->MaxNBest);

        if (InitNewUtterance(This) != 0) {
            return 0;
        }
        This->BaseBlock.Process = IdleProcess;
        if (RealN != 0) {
            return !_tosControl(&This->BaseBlock.Object, 0x09, (u32) &This->Result, RealN);
        }
        return !_tosControl(&This->BaseBlock.Object, 0x0A, 0, 0);
    }
    case 0x03:
        This->BaseBlock.Process = IsoWordTokenDpProcess;
        break;
    case 0x05:
        This->BaseBlock.Process = IdleProcess;
        break;
    case 0x22:
        InitIsoWordTokenDpLanguage(This, (struct _LANGUAGE *) CParam1);
        break;
    case 0x23:
        FreeIsoWordTokenDp(This);
        if (InitIsoWordTokenDpContext(This, (struct _ISOWORD_NODE *) CParam1,
                                      (const struct ISOWORD_USERWORD_LNG *) CParam2) != 0) {
            return 0;
        }
        break;
    case 0x24: {
        const struct _DISTCALCFUNCS *pFuncs = (const struct _DISTCALCFUNCS *) CParam1;

        This->InitCalculateDistances = pFuncs->InitCalculateDistances;
        This->CalculateDistance = pFuncs->CalculateDistance;
        This->GetpCachedDistances_DISTBLK = pFuncs->GetpCachedDistances;
        This->pDistCalc = pFuncs->pThis;
        break;
    }
    case 0x27:
        FreeIsoWordTokenDp(This);
        break;
    case 0x1A: {
        struct _DYNPROG_PARAM *pRecParam = (struct _DYNPROG_PARAM *) CParam1;
        This->s32Accuracy = pRecParam->Accuracy;
        if (SetMaxNBest(This, pRecParam->MaxNBest) != 0) {
            return 0;
        }
        This->TSEnable = pRecParam->TSEnable;
        This->OriTSCount = pRecParam->TSCount;
        This->TSCount = This->OriTSCount / 10;
        break;
    }
    case 0x1B: {
        struct _DYNPROG_PARAM *pRecParam = (struct _DYNPROG_PARAM *) CParam1;

        pRecParam->Accuracy = This->s32Accuracy;
        pRecParam->TSCount = This->OriTSCount;
        pRecParam->MaxNBest = This->MaxNBest;
        pRecParam->TSEnable = This->TSEnable;
        break;
    }
    case 0xFF:
        FreeIsoWordTokenDp(This);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    case 0x1C:
    case 0x1D:
    case 0x1E:
        break;
    default:
        return 0;
    }
    return 1;
}

/* Constructs a TOS block for isolated-word recognition. */
struct _BLOCK *ConstructIsoWordTokenDp(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 2, 0, IsoWordTokenDpProcess,
                                 InitIsoWordTokenDpProcess, ControlIsoWordTokenDp,
                                 sizeof(struct _ISOWORDTOKENDP));
}
