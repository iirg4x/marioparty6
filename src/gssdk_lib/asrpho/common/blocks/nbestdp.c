/* N-best dynamic-programming search block for speech recognition. */
#include "types.h"
#include <string.h>

#include <stdlib.h>

#include "gssdk/heap.h"
#include "gssdk/langdata.h"
#include "gssdk/mqueue.h"
#include "gssdk/secondpass.h"
#include "gssdk/tos.h"
#include "gssdk/engine.h"

typedef struct _BACKTRACE _BACKTRACE;
typedef struct _MINBACKTRACE _MINBACKTRACE;
typedef struct _ACTIVEDPSTATE _ACTIVEDPSTATE;
typedef struct _DUMMY1NODE _DUMMY1NODE;
typedef struct _DUMMY2NODE _DUMMY2NODE;
typedef struct _WORDFOLLOWER _WORDFOLLOWER;
typedef struct _BACKTRACESTACK _BACKTRACESTACK;

/* A scored reference to the preceding word and its parallel alternatives. */
struct _MINBACKTRACE {
    /* Preceding word on this recognition path. */
    _BACKTRACE *Previous;
    /* Next alternative path for the same endpoint. */
    _MINBACKTRACE *NextInParallel;
    /* Cumulative path score at this word boundary. */
    unsigned long Score;
};

/* A word-boundary backtrace referenced by active states and other backtraces. */
struct _BACKTRACE {
    /* Head of this boundary's parallel alternatives. */
    _MINBACKTRACE MinBt;
    /* References that keep the backtrace alive. */
    unsigned short Count;
    /* Frame at the word exit and the corresponding dictionary state. */
    unsigned short ExitTime;
    struct _DICTSTATE *State;
};

/* A linked reference to a state that can follow a word boundary. */
struct _WORDFOLLOWER {
    /* Next follower in the list and its active state. */
    _WORDFOLLOWER *pNext;
    _ACTIVEDPSTATE *pActiveState;
};

/* One active dictionary path in the current search frame. */
struct _ACTIVEDPSTATE {
    /* Link in the active-state list. */
    _ACTIVEDPSTATE *pNext;
    /* Accumulated path score relative to the frame baseline. */
    signed short Score;
    /* Marks the first dictionary state of a word. */
    unsigned char BeginOfWord;
    /* Current dictionary state and the word boundaries leading to it. */
    struct _DICTSTATE *pStateInDict;
    _BACKTRACE *pBacktrace;
    /* A word-start state refers to its followers or its list slot.
     * Other states refer to their predecessor. */
    union {
        _WORDFOLLOWER *pWordFollowers;
        _ACTIVEDPSTATE *pPredecessor;
        _ACTIVEDPSTATE **ppFirstState;
    } PoF;
    /* Slot for the word-pair endpoint and the next linked active state. */
    _ACTIVEDPSTATE **ppWordPairEnd;
    _ACTIVEDPSTATE *pFollower;
};

/* Intermediate path record used to carry a backtrace across a word transition. */
struct _DUMMY1NODE {
    /* Link in the intermediate-state list. */
    _DUMMY1NODE *pNext;
    /* Accumulated path score relative to the frame baseline. */
    signed short Score;
    /* Dictionary state and backtrace carried across the transition. */
    struct _DICTSTATE *pStateInDict;
    _BACKTRACE *pBacktrace;
    /* Predecessor state or the endpoint slot shared with the path. */
    union {
        _ACTIVEDPSTATE *pPredecessor;
        _ACTIVEDPSTATE **ppWordPairEnd;
    } PoF;
};

/* Word-end path record with the list of states that can follow it. */
struct _DUMMY2NODE {
    /* Link in the word-end list. */
    _DUMMY2NODE *pNext;
    /* Accumulated path score relative to the frame baseline. */
    signed short Score;
    /* Backtrace retained by this word-end path. */
    _BACKTRACE *pBacktrace;
    /* Ended word and the states reached from its boundary. */
    long WordID;
    _WORDFOLLOWER *pWordFollowers;
};

/* Work stack used while releasing referenced backtrace paths. */
struct _BACKTRACESTACK {
    /* Adjacent stack slots. */
    _BACKTRACESTACK *Up;
    _BACKTRACESTACK *Down;
    /* Backtrace awaiting cleanup in this slot. */
    _BACKTRACE *ToDo;
};

/* Controls whether the search checks the end-of-speech stop criterion. */
enum _ENDOFSPEECH { ENDOFSPEECH_None = 0x0, ENDOFSPEECH_StopCriterium = 0x1 };
typedef struct _NBESTDYNPROG _NBESTDYNPROG;
/* TinyOS block that accumulates frame scores and builds N-best word paths. */
struct _NBESTDYNPROG {
    /* Base block used for dispatch, configuration and allocation. */
    struct _BLOCK BaseBlock;
    /* Distance calculator, its frame operations and shared cache. */
    void *pDistCalc;
    signed short (*CalculateDistance)(void *, unsigned int);
    void (*InitCalculateDistances)(void *, signed short *, unsigned char *, unsigned char, int);
    signed short *(*GetpCachedDistances_DISTBLK)(void *);
    signed short *pCachedDistances_DISTBLK;
    /* Number of phoneme distances reset in the cache each frame. */
    unsigned int NbrPhenomes;
    /* Consecutive frames with the same eligible terminal word. */
    unsigned int uCountSilence;
    /* Vocabulary size and number of bytes in its menu mask. */
    unsigned long uNbrOfWords;
    unsigned long uNbrOfWordsBit;
    /* Dictionary state count copied from the context. */
    unsigned long uLastState;
    /* Per-word dictionary offsets and the dictionary state table. */
    unsigned long *pBeginOfWords;
    struct _DICTSTATE *pTheStateDict;
    /* First slot of the backtrace cleanup stack. */
    _BACKTRACESTACK *TopOfBacktraceStack;
    /* Recognized paths and garbage model scores for this search. */
    struct _DYNPROG_RESULT Result;
    /* Number and heads of live search paths and intermediate records. */
    unsigned int uNbrOfActiveStates;
    _ACTIVEDPSTATE *pDynProg;
    _DUMMY1NODE *pDynProgDummy1;
    _DUMMY2NODE *pDynProgDummy2;
    /* Context syntax table and the selected-word bit mask. */
    unsigned long *pSyntax;
    unsigned char *pucMenuMasks;
    /* Ranges into the menu-filtered list of successor words. */
    long **ppNext;
    long *pNextList;
    /* Word-follower sentinel and the lookup tables for word and word-pair ends. */
    _ACTIVEDPSTATE *DummyWordFollower;
    _DUMMY2NODE **ppWordEnds;
    _ACTIVEDPSTATE **ppWordPairEnds;
    /* Transition count before menu filtering. */
    unsigned long uNbrOfEdgesInSyntax;
    /* Per-word start, end and menu flags from the context. */
    unsigned short *pWordProps;
    /* The game's code never reads this field. */
    int OwnsWordsProps;
    /* Distance-cache indices for the two garbage models. */
    unsigned int SingleWordGarbagePhenom;
    unsigned int SentenceGarbagePhenom;
    /* Stop-criterion mode and processed frame count. */
    enum _ENDOFSPEECH EndOfSpeech;
    unsigned short u16FrameCounter;
    /* Accumulator selected for the context's optional-silence mode. */
    int (*DynamicProgramming)(_NBESTDYNPROG *);
    /* Maximum number of recognized paths to return. */
    unsigned long MaxNBest;
    /* Count and indices of words allowed to start a path. */
    unsigned long NbrOfStart;
    long *pStartWords;
    /* Stop threshold in frames and its unscaled profile value. */
    unsigned long uCountSilenceThreshold;
    unsigned long uOriCountSilenceThreshold;
    /* Terminal word from the previous stop-criterion check. */
    long OldWord;
    /* Target active-state count for adaptive pruning. */
    unsigned long Nset;
    /* Enables threshold adjustment after the initial search frames. */
    unsigned char PruneAdaptFlag;
    /* Current pruning threshold and its upper limit. */
    signed short CurrentPruningT;
    signed short ClipMaxTreshold;
    /* The game's code never reads this field. */
    float InitTreshFactor;
    /* Scaled pruning estimate and adjustment coefficient. */
    unsigned long Theta;
    unsigned long Alfa;
    /* Recent state-count times threshold and squared state-count samples. */
    unsigned long NiTi[5];
    unsigned long NiNi[5];
    /* Next slot in the pruning sample ring. */
    unsigned char PruneIndex;
    /* Best active score and the state that produced it. */
    signed short sMin;
    _ACTIVEDPSTATE *pStsMin;
    /* Profile offset added to the pruning threshold. */
    signed short sMinClip;
    /* Sum of the per-frame normalization scores. */
    unsigned long AccumulatedScore;
    /* Extra path cost applied at each word boundary. */
    signed short *pWordPenalties;
};

/* Scores one frame of recognizer input. */
static void NBestDynProgProcess(struct _BLOCK *_This, void *pInputs);
/* Advances active paths for contexts without optional silence. */
static int DynProgAccNBestNoOptSil(_NBESTDYNPROG *This);
/* Advances active paths for contexts with optional silence. */
static int DynProgAccNBestOptSil(_NBESTDYNPROG *This);
/* Allocates and links a new backtrace stack slot. */
static void ExtendBacktraceStack(_NBESTDYNPROG *This, _BACKTRACESTACK *pEndFree);
/* Releases a backtrace and predecessors whose final reference it holds. */
static void CleanUpParBTProc(_NBESTDYNPROG *This, _BACKTRACE *FirstPtr);

/* Returns discarded backtrace records and their children to the block pools. */
static void CollectDummyMemory(_NBESTDYNPROG *This)
{
    struct _TOS *systemContext = This->BaseBlock.Object.pTos;
    _DUMMY1NODE *record = This->pDynProgDummy1;
    _DUMMY2NODE *recordWithChildren;
    _DUMMY1NODE *currentReference;
    _WORDFOLLOWER *child;
    struct _FASTALLO_FREE_NODE **freeReferences;
    _DUMMY2NODE *currentRecord;
    struct _FASTALLO_FREE_NODE **childFreeList;
    struct _FASTALLO_FREE_NODE **freeRecords;

    freeReferences = &systemContext->FastAllocHeap.pFreeNodes[sizeof(_DUMMY1NODE) / 4];
    while (record != NULL) {
        currentReference = record;

        if (record->pBacktrace != NULL) {
            if (--record->pBacktrace->Count == 0) {
                CleanUpParBTProc(This, record->pBacktrace);
            }
        }
        record = record->pNext;
        ((struct _FASTALLO_FREE_NODE *)currentReference)->pNext = *freeReferences;
        *freeReferences = (struct _FASTALLO_FREE_NODE *)currentReference;
    }

    recordWithChildren = This->pDynProgDummy2;
    childFreeList = &systemContext->FastAllocHeap.pFreeNodes[sizeof(_WORDFOLLOWER) / 4];
    freeRecords = &systemContext->FastAllocHeap.pFreeNodes[sizeof(_DUMMY2NODE) / 4];
    while (recordWithChildren != NULL) {
        currentRecord = recordWithChildren;
        child = recordWithChildren->pWordFollowers;

        if (recordWithChildren->pBacktrace != NULL) {
            if (--recordWithChildren->pBacktrace->Count == 0) {
                CleanUpParBTProc(This, recordWithChildren->pBacktrace);
            }
        }
        while (child != NULL) {
            _WORDFOLLOWER *currentChild = child;
            child = child->pNext;
            ((struct _FASTALLO_FREE_NODE *)currentChild)->pNext = *childFreeList;
            *childFreeList = (struct _FASTALLO_FREE_NODE *)currentChild;
        }
        recordWithChildren = recordWithChildren->pNext;
        ((struct _FASTALLO_FREE_NODE *)currentRecord)->pNext = *freeRecords;
        *freeRecords = (struct _FASTALLO_FREE_NODE *)currentRecord;
    }
    This->pDynProgDummy1 = NULL;
    This->pDynProgDummy2 = NULL;
}

/* Recycles completed paths and their child nodes before dynamic programming. */
static void CollectMemory(_NBESTDYNPROG *This)
{
    struct _TOS *systemContext = This->BaseBlock.Object.pTos;
    _ACTIVEDPSTATE *record = This->pDynProg;
    _ACTIVEDPSTATE *currentRecord;
    struct _FASTALLO_FREE_NODE **freeRecords =
        &systemContext->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
    struct _DYNPROG_RESULT_NODE *recycleRecord;
    struct _DYNPROG_RESULT_NODE *currentRecycleRecord;
    struct _DYNPROG_RESULT_NODE *child;

    while (record != NULL) {
        currentRecord = record;
        if (record->pBacktrace != NULL) {
            if (--record->pBacktrace->Count == 0) {
                CleanUpParBTProc(This, record->pBacktrace);
            }
        }
        record = record->pNext;
        ((struct _FASTALLO_FREE_NODE *)currentRecord)->pNext = *freeRecords;
        *freeRecords = (struct _FASTALLO_FREE_NODE *)currentRecord;
    }
    This->uNbrOfActiveStates = 0;
    This->pDynProg = NULL;
    CollectDummyMemory(This);

    recycleRecord = This->Result.pResult;
    while (recycleRecord != NULL) {
        currentRecycleRecord = recycleRecord;
        child = recycleRecord->pInfo;
        while (child != NULL) {
            struct _DYNPROG_RESULT_NODE *currentChild = child;
            struct _FASTALLO_FREE_NODE **freeNodes =
                &systemContext->FastAllocHeap.pFreeNodes[sizeof(struct _DYNPROG_RESULT_NODE) / 4];
            child = child->pNext;
            ((struct _FASTALLO_FREE_NODE *)currentChild)->pNext = *freeNodes;
            *freeNodes = (struct _FASTALLO_FREE_NODE *)currentChild;
        }
        recycleRecord = recycleRecord->pNext;
        {
            struct _FASTALLO_FREE_NODE **freeNodes =
                &systemContext->FastAllocHeap.pFreeNodes[sizeof(struct _DYNPROG_RESULT_NODE) / 4];
            ((struct _FASTALLO_FREE_NODE *)currentRecycleRecord)->pNext = *freeNodes;
            *freeNodes = (struct _FASTALLO_FREE_NODE *)currentRecycleRecord;
        }
    }
    This->Result.pResult = NULL;
}

/* Resets the search state and rebuilds its initial Viterbi paths. */
static void InitViterbi(_NBESTDYNPROG *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _ACTIVEDPSTATE *pDp1 = NULL;
    int i;
    unsigned int St;
    _BACKTRACE *NewBacktrace;
    unsigned long Word;
    _ACTIVEDPSTATE *pPrev = NULL;
    unsigned int BtCount;
    if (This->TopOfBacktraceStack == NULL) {
        ExtendBacktraceStack(This, NULL);
    }
    CollectMemory(This);
    This->AccumulatedScore = 0;
    This->uCountSilence = 0;
    This->OldWord = 0;
    This->u16FrameCounter = 0xFFFF;
    This->sMin = 0;
    This->pStsMin = NULL;
    for (St = 0; St <= This->uNbrOfWords; St++) {
        This->ppWordEnds[St] = NULL;
    }
    for (St = 0; St < This->NbrOfStart + This->uNbrOfEdgesInSyntax; St++) {
        This->ppWordPairEnds[St] = NULL;
    }

    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
    NewBacktrace->State = NULL;
    NewBacktrace->MinBt.NextInParallel = NULL;
    NewBacktrace->MinBt.Previous = NULL;
    BtCount = 0;
    for (Word = 0; Word < This->NbrOfStart; Word++) {
        long WordNbr = This->pStartWords[Word];
        if ((This->pucMenuMasks[WordNbr >> 3] & (0x80 >> (WordNbr & 7))) != 0) {
            unsigned long *PtrBeg = This->pBeginOfWords + (This->pBeginOfWords)[WordNbr];
            unsigned long *PtrBegEnd = This->pBeginOfWords + (This->pBeginOfWords)[WordNbr + 1];

            while (PtrBeg < PtrBegEnd) {
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp1)
                pDp1->pStateInDict = This->pTheStateDict + *PtrBeg;
                pDp1->Score = 0;
                pDp1->pBacktrace = NewBacktrace;
                pDp1->pFollower = NULL;
                pDp1->PoF.ppFirstState = &This->DummyWordFollower;
                pDp1->BeginOfWord = 1;
                pDp1->ppWordPairEnd = &This->ppWordPairEnds[Word];
                pDp1->pNext = pPrev;
                pPrev = pDp1;
                BtCount++;
                PtrBeg++;
            }
        }
    }
    This->pDynProg = pDp1;
    NewBacktrace->Count = BtCount;
    This->uNbrOfActiveStates = BtCount;
    This->CurrentPruningT = This->ClipMaxTreshold;
    This->PruneAdaptFlag = 0;
    for (i = 0; i < 5; i++) {
        This->NiTi[i] = 0;
        This->NiNi[i] = 0;
    }
    This->PruneIndex = 0;
}

/* Rebuilds syntax ranges using the enabled words in the menu mask. */
static void ChangeMenuIndexedSyntax(_NBESTDYNPROG *This)
{
    long *Ptr;
    long **NextPtr;
    unsigned long *SynPtr;
    unsigned long *SynBasePtr;
    u32 k;
    const u8 *pucMenuMasks;
    u32 NbrWords;

    SynBasePtr = This->pSyntax;
    Ptr = This->pNextList;
    NextPtr = This->ppNext;
    SynPtr = SynBasePtr;
    pucMenuMasks = This->pucMenuMasks;
    NbrWords = This->uNbrOfWords;
    for (k = 0; k < NbrWords; k++) {
        u32 BeginIndex;
        u32 EndIndex;
        u32 index;

        *NextPtr = Ptr;
        NextPtr++;
        BeginIndex = *SynPtr;
        EndIndex = *++SynPtr;

        for (index = BeginIndex; index < EndIndex; index++) {
            long WordNbr = SynBasePtr[index];
            u32 bitMask = 0x80 >> (WordNbr & 7);

            if (bitMask & pucMenuMasks[WordNbr >> 3]) {
                *Ptr++ = WordNbr;
            }
        }
    }
    *NextPtr = Ptr;
}

/* Loads the profile defaults used by the N-best dynamic-programming block. */
static u8 InitNBestDynProgProcess(struct _BLOCK *_This)
{
    _NBESTDYNPROG *This = (_NBESTDYNPROG *)_This;
    float Alfa;
    This->EndOfSpeech = ENDOFSPEECH_None;
    This->uCountSilenceThreshold = _tosGetProfileU32(&This->BaseBlock.Object, 6, 50);
    This->uOriCountSilenceThreshold = This->uCountSilenceThreshold * 10;
    This->MaxNBest = _tosGetProfileU32(&This->BaseBlock.Object, 7, 10);
    This->Nset = _tosGetProfileU32(&This->BaseBlock.Object, 9, 300);
    Alfa = _tosGetProfileFloat(&This->BaseBlock.Object, 0xA, 0.2f);
    This->Alfa = (u32)(Alfa * 128.0f);
    This->ClipMaxTreshold = _tosGetProfileU32(&This->BaseBlock.Object, 0xB, 5500);
    This->sMinClip = _tosGetProfileU32(&This->BaseBlock.Object, 0xE, 0);
    This->Result.pResult = NULL;
    This->BaseBlock.InputQueues[1].Size = _tosGetProfileU32(&This->BaseBlock.Object, 0xF, 4);
    This->BaseBlock.InputQueues[0].Size = 1;
    return 0;
}

/* Sets up the dictionary, enabled syntax and work tables for a recognition context. */
static u8 InitDynprogContext(_NBESTDYNPROG *This, struct _DYNPROG_CTXINFO *pCtx)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    u32 NbrOfStart = 0;
    u32 i;
    u32 Word;
    long StartWord = 0;
    unsigned long uMaxNbrOfEdgesInSyntax;
    u16 *pProps;

    if (pCtx->iOptionalSilence != 0) {
        This->DynamicProgramming = DynProgAccNBestOptSil;
    } else {
        This->DynamicProgramming = DynProgAccNBestNoOptSil;
    }
    This->uNbrOfWords = pCtx->uNbrOfWords;
    This->uNbrOfWordsBit = ((This->uNbrOfWords - 1) >> 3) + 1;
    pProps = pCtx->pWordProps;
    for (i = 0; i < This->uNbrOfWords; i++) {
        if ((*pProps & 1) != 0) {
            NbrOfStart++;
        }
        pProps++;
    }
    This->NbrOfStart = NbrOfStart;
    This->uLastState = pCtx->uNrStatesInDict;
    This->pStartWords = heap_Alloc(pTos->hHeap, This->NbrOfStart * sizeof(*This->pStartWords));
    This->pWordPenalties = pCtx->pWordPenalties;
    This->pBeginOfWords = pCtx->pBeginOfWords;
    This->pTheStateDict = pCtx->pStateInDict;
    if (This->pStartWords == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x71);
    }

    This->pSyntax = pCtx->pSyntax;
    This->pWordProps = pCtx->pWordProps;
    for (Word = 0; Word < This->uNbrOfWords; Word++) {
        if ((This->pWordProps[Word] & 1) != 0) {
            This->pStartWords[StartWord++] = Word;
        }
    }
    This->ppNext = heap_Alloc(pTos->hHeap, (This->uNbrOfWords + 1) * sizeof(*This->ppNext));
    This->pucMenuMasks = pCtx->pucMenuMasks;
    uMaxNbrOfEdgesInSyntax = This->pSyntax[This->uNbrOfWords] - This->pSyntax[0];
    This->uNbrOfEdgesInSyntax = uMaxNbrOfEdgesInSyntax;
    This->pNextList = heap_Alloc(
        pTos->hHeap, (uMaxNbrOfEdgesInSyntax + 1) * sizeof(*This->pNextList));
    if (This->ppNext == NULL || This->pNextList == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x71);
    }

    ChangeMenuIndexedSyntax(This);
    This->ppWordEnds = heap_Alloc(pTos->hHeap, (This->uNbrOfWords + 1) * sizeof(*This->ppWordEnds));
    if (This->ppWordEnds == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x71);
    }
    This->ppWordPairEnds = heap_Alloc(pTos->hHeap,
        (This->NbrOfStart + This->uNbrOfEdgesInSyntax) * sizeof(*This->ppWordPairEnds));
    if (This->ppWordPairEnds == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 0x71);
    }
    This->pCachedDistances_DISTBLK = This->GetpCachedDistances_DISTBLK(This->pDistCalc);
    This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))NBestDynProgProcess;
    return 0;
}

/* Adds a stack node, taking one from the context allocator or its chunk pool. */
static void ExtendBacktraceStack(_NBESTDYNPROG *This, _BACKTRACESTACK *pEndFree)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _BACKTRACESTACK *pExtra;
    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACESTACK) / 4, &pExtra)
    if (pEndFree != NULL) {
        pEndFree->Down = pExtra;
        pExtra->Up = pEndFree;
    } else {
        This->TopOfBacktraceStack = pExtra;
        pExtra->Up = NULL;
    }
    pExtra->Down = NULL;
}

/* Frees the arrays allocated for the current recognition context. */
static void FreeDynprog(_NBESTDYNPROG *This)
{
    struct _TOS *context = This->BaseBlock.Object.pTos;

    if (This->pStartWords != NULL) {
        heap_Free(context->hHeap, This->pStartWords);
        This->pStartWords = NULL;
    }
    if (This->ppNext != NULL) {
        heap_Free(context->hHeap, This->ppNext);
        This->ppNext = NULL;
    }
    if (This->pNextList != NULL) {
        heap_Free(context->hHeap, This->pNextList);
        This->pNextList = NULL;
    }
    if (This->ppWordEnds != NULL) {
        heap_Free(context->hHeap, This->ppWordEnds);
        This->ppWordEnds = NULL;
    }
    if (This->ppWordPairEnds != NULL) {
        heap_Free(context->hHeap, This->ppWordPairEnds);
        This->ppWordPairEnds = NULL;
    }
    if (This->Result.GarbageSize != 0) {
        heap_Free(context->hHeap, This->Result.pSingleWordGarbage);
        heap_Free(context->hHeap, This->Result.pSentenceGarbage);
        This->Result.GarbageSize = 0;
    }
}

/* Releases a backtrace and walks predecessors whose last reference was dropped. */
static void CleanUpParBTProc(_NBESTDYNPROG *This, _BACKTRACE *FirstPtr)
{
    struct _TOS *systemContext = This->BaseBlock.Object.pTos;
    _BACKTRACESTACK *stack = This->TopOfBacktraceStack;

    stack->ToDo = FirstPtr;
    if (stack->Down == NULL) {
        ExtendBacktraceStack(This, stack);
    }
    stack = stack->Down;
    while (stack != This->TopOfBacktraceStack) {
        _MINBACKTRACE *referenceEntry;
        _BACKTRACE *firstReference;

        firstReference = stack->Up->ToDo;
        stack = stack->Up;
        referenceEntry = &firstReference->MinBt;
        while (referenceEntry != NULL) {
            _BACKTRACE *item = referenceEntry->Previous;
            _MINBACKTRACE *currentReference;

            if (item != NULL) {
                item->Count--;
                if (item->Count == 0) {
                    stack->ToDo = item;
                    if (stack->Down == NULL) {
                        ExtendBacktraceStack(This, stack);
                    }
                    stack = stack->Down;
                }
            }
            currentReference = referenceEntry;
            referenceEntry = currentReference->NextInParallel;
            if (currentReference != &firstReference->MinBt) {
                struct _FASTALLO_FREE_NODE **recycledReferenceFreeList =
                    &systemContext->FastAllocHeap.pFreeNodes[sizeof(_MINBACKTRACE) / 4];

                ((struct _FASTALLO_FREE_NODE *)currentReference)->pNext =
                    *recycledReferenceFreeList;
                *recycledReferenceFreeList = (struct _FASTALLO_FREE_NODE *)currentReference;
            }
        }
        {
            struct _FASTALLO_FREE_NODE **firstReferenceFreeList =
                &systemContext->FastAllocHeap.pFreeNodes[sizeof(_BACKTRACE) / 4];

            ((struct _FASTALLO_FREE_NODE *)firstReference)->pNext = *firstReferenceFreeList;
            *firstReferenceFreeList = (struct _FASTALLO_FREE_NODE *)firstReference;
        }
    }
}

/* Adjusts the pruning threshold as recognition progresses. */
static void PrunDynProg(_NBESTDYNPROG *This)
{
    if (This->PruneAdaptFlag != 0) {
        u32 scoreTotal = This->NiTi[0];
        u32 countTotal = This->NiNi[0] + 1;
        s32 remaining;
        s32 scoreDelta;
        s32 nextScore;

        scoreTotal += This->NiTi[1];
        countTotal += This->NiNi[1];
        scoreTotal += This->NiTi[2];
        countTotal += This->NiNi[2];
        scoreTotal += This->NiTi[3];
        countTotal += This->NiNi[3];
        scoreTotal += This->NiTi[4];
        countTotal += This->NiNi[4];

        /* Scale the average without overflowing the shifted total. */
        if (scoreTotal > 0x01FFFFFF) {
            This->Theta = scoreTotal / (countTotal >> 7);
        } else {
            This->Theta = (scoreTotal << 7) / countTotal;
        }
        remaining = This->Nset - This->uNbrOfActiveStates;
        scoreDelta = (This->Alfa * This->Theta) * remaining;
        /* Saturate the signed correction before converting it to score units. */
        if (remaining > 0) {
            if (scoreDelta < 0) {
                scoreDelta = 0x1FFFC000;
            } else if (scoreDelta > 0x1FFFC000) {
                scoreDelta = 0x1FFFC000;
            }
        } else if (scoreDelta > 0) {
            scoreDelta = (s32)0xE0004000;
        } else if (scoreDelta < (s32)0xE0004000) {
            scoreDelta = (s32)0xE0004000;
        }
        {
            s16 oldScore = This->CurrentPruningT;

            nextScore = (scoreDelta >> 14) + oldScore;
            if (nextScore > This->ClipMaxTreshold) {
                This->CurrentPruningT = This->ClipMaxTreshold;
            } else {
                This->CurrentPruningT = (s16)nextScore;
            }
            if (This->CurrentPruningT < (s16)(oldScore / 3)) {
                This->CurrentPruningT = oldScore / 3;
            }
        }
    } else if (This->u16FrameCounter > 10 || This->uNbrOfActiveStates > (This->Nset >> 2)) {
        This->PruneAdaptFlag = 1;
    }
    if (This->PruneAdaptFlag != 0) {
        u32 bucket = This->PruneIndex;
        unsigned long *scoreBuckets = &This->NiTi[0];
        unsigned long *countBuckets = &This->NiNi[0];

        scoreBuckets[bucket] = (This->uNbrOfActiveStates * (u16)This->CurrentPruningT) >> 3;
        countBuckets[bucket] = (This->uNbrOfActiveStates * This->uNbrOfActiveStates) >> 3;
        bucket++;
        if (bucket == 5) {
            bucket = 0;
        }
        This->PruneIndex = bucket;
    }
}

/* Resolve the score tie through the dictionary word. */
static void UpdateNewMinIfScoreEqual(_NBESTDYNPROG *This, _ACTIVEDPSTATE *pDp1)
{
    struct _DICTSTATE *pTempDict;
    long WordID;

    pTempDict = pDp1->pStateInDict;
    while ((pTempDict->StateIndex & 1) == 0)
        ++pTempDict;
    WordID = pTempDict[1].StateIndex + ((unsigned long)pTempDict[2].StateIndex << 0x10);
    if (This->pWordProps[WordID] & 2)
        This->pStsMin = pDp1;
}

/* Advances the search without inserting optional silence. */
static int DynProgAccNBestNoOptSil(_NBESTDYNPROG *This)
{
    struct _TOS *pTos;
    _ACTIVEDPSTATE *pDp1;
    _DUMMY1NODE *pDp1Dummy1;
    _DUMMY2NODE *pDp1Dummy2;
    _ACTIVEDPSTATE *pDp2;
    _DUMMY1NODE *pDp2Dummy1;
    _DUMMY2NODE *pDp2Dummy2;
    _ACTIVEDPSTATE *pPrev;
    _DUMMY1NODE *pPrevDummy1;
    _DUMMY2NODE *pPrevDummy2;
    unsigned int uNewActiveStates;
    int sNewMin;
    int sTresh;
    signed short *pCachedDistances_DISTBLK;

    pTos = This->BaseBlock.Object.pTos;
    pCachedDistances_DISTBLK = This->pCachedDistances_DISTBLK;
    sNewMin = 0x7fff;
    PrunDynProg(This);
    uNewActiveStates = This->uNbrOfActiveStates;
    sTresh = This->CurrentPruningT + This->sMinClip;

    pPrev = NULL;
    pPrevDummy1 = NULL;
    pDp1 = This->pDynProg;
    while (pDp1 != NULL) {
        int sScore;
        _BACKTRACE *pBPtr;
        struct _DICTSTATE *pDict;
        unsigned int StateIndex;
        unsigned int FromPhenom;

        sScore = pDp1->Score;
        pBPtr = pDp1->pBacktrace;
        pDict = pDp1->pStateInDict;
        StateIndex = pDict->StateIndex;
        FromPhenom = StateIndex >> 2;

        if ((StateIndex & 1) == 0) {
            _ACTIVEDPSTATE *pDpOld;
            unsigned int Phenom;
            int NewScore;
            StateIndex = (++pDict)->StateIndex;
            Phenom = StateIndex >> 2;
            pDpOld = pDp1->pFollower;
            if (pDpOld == NULL) {
                NewScore = sScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                         ? (pCachedDistances_DISTBLK[Phenom] =
                                                This->CalculateDistance(This->pDistCalc, Phenom))
                                         : pCachedDistances_DISTBLK[Phenom]);
                if (NewScore <= sTresh) {
                    if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pBPtr;
                        pBPtr->Count++;
                        NewBacktrace->State = pDp1->pStateInDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        pBPtr = NewBacktrace;
                    }
                    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp2)
                    pDp2->Score = NewScore;
                    pDp2->pStateInDict = pDict;
                    pDp2->pBacktrace = pBPtr;
                    pDp2->pFollower = NULL;
                    pDp2->PoF.pPredecessor = pDp1;
                    pBPtr->Count++;
                    pDp2->pNext = This->pDynProg;
                    This->pDynProg = pDp2;
                    if (pPrev == NULL) {
                        pPrev = pDp2;
                    }
                    pDp2->BeginOfWord = 0;
                    pBPtr = pDp1->pBacktrace;
                    pDp2->ppWordPairEnd = pDp1->ppWordPairEnd;
                    pDp1->pFollower = pDp2;
                    ++uNewActiveStates;
                    if (NewScore <= sNewMin) {
                        if (NewScore == sNewMin) {
                            UpdateNewMinIfScoreEqual(This, pDp2);
                        } else {
                            This->pStsMin = pDp2;
                        }
                        sNewMin = NewScore;
                    }
                }
            }

            if (pDp1->BeginOfWord == 0) {
                _ACTIVEDPSTATE *pPred = pDp1->PoF.pPredecessor;
                if (pPred != NULL && pPred->Score < sScore) {
                    _BACKTRACE *FirstPtr;
                    sScore = pPred->Score;
                    if ((pPred->pStateInDict->StateIndex & 2) != 0) {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pPred->pBacktrace;
                        pPred->pBacktrace->Count++;
                        NewBacktrace->State = pPred->pStateInDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        FirstPtr = NewBacktrace;
                    } else {
                        FirstPtr = pPred->pBacktrace;
                    }
                    pDp1->pBacktrace = FirstPtr;
                    FirstPtr->Count++;
                    if (--pBPtr->Count == 0)
                        CleanUpParBTProc(This, pBPtr);
                }
            }

            sScore += (pCachedDistances_DISTBLK[FromPhenom] == 0x7FFF
                           ? (pCachedDistances_DISTBLK[FromPhenom] =
                                  This->CalculateDistance(This->pDistCalc, FromPhenom))
                           : pCachedDistances_DISTBLK[FromPhenom]);
            pDp1->Score = sScore;
            if (sScore > sTresh) {
                if (pDp1->BeginOfWord != 0) {
                    *pDp1->PoF.ppFirstState = NULL;
                } else if (pDp1->PoF.pPredecessor != NULL) {
                    goto PruneStateWithPredecessor;
                }
                if (--pDp1->pBacktrace->Count == 0)
                    CleanUpParBTProc(This, pDp1->pBacktrace);
                if (pDp1->pFollower != NULL)
                    pDp1->pFollower->PoF.pPredecessor = NULL;
                {
                    _ACTIVEDPSTATE *NextDp;
                    NextDp = pDp1->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                    }
                    if (pPrev != NULL) {
                        pPrev->pNext = NextDp;
                    } else {
                        This->pDynProg = NextDp;
                    }
                    pDp1 = NextDp;
                    --uNewActiveStates;
                    continue;
                }
PruneStateWithPredecessor:
                if (pDp1->pFollower == NULL) {
                    if (--pDp1->pBacktrace->Count == 0)
                        CleanUpParBTProc(This, pDp1->pBacktrace);
                    pDp1->PoF.pPredecessor->pFollower = NULL;
                    {
                        _ACTIVEDPSTATE *NextDp = pDp1->pNext;
                        {
                            struct _FASTALLO_FREE_NODE **pTemp =
                                &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                            ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                            *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                        }
                        if (pPrev != NULL) {
                            pPrev->pNext = NextDp;
                        } else {
                            This->pDynProg = NextDp;
                        }
                        pDp1 = NextDp;
                        --uNewActiveStates;
                        continue;
                    }
                }
                if (sScore > 0x77FF) {
                    pDp1->Score = 0x77ff;
                }
                pPrev = pDp1;
                pDp1 = pDp1->pNext;
                continue;
            } else if (sScore <= sNewMin) {
                if (sScore == sNewMin) {
                    UpdateNewMinIfScoreEqual(This, pDp1);
                } else {
                    This->pStsMin = pDp1;
                }
                sNewMin = sScore;
            }
            pPrev = pDp1;
            pDp1 = pDp1->pNext;
            continue;
        } else {
            int NewScore;
            _DUMMY1NODE *pDpOld = (_DUMMY1NODE *)*pDp1->ppWordPairEnd;
            if (pDpOld != NULL) {
                if (sScore < pDpOld->Score) {
                    _BACKTRACE *FirstPtr = pDpOld->pBacktrace;
                    pDpOld->Score = sScore;
                    pDpOld->pStateInDict = pDict;
                    pDpOld->pBacktrace = pBPtr;
                    pBPtr->Count++;
                    if (FirstPtr != NULL && --FirstPtr->Count == 0)
                        CleanUpParBTProc(This, FirstPtr);
                }
            } else if (sScore <= sTresh) {
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY1NODE) / 4, &pDp2Dummy1)
                pDp2Dummy1->Score = sScore;
                pDp2Dummy1->pStateInDict = pDict;
                pDp2Dummy1->pBacktrace = pBPtr;
                pDp2Dummy1->PoF.pPredecessor = pDp1;
                pBPtr->Count++;
                pDp2Dummy1->pNext = This->pDynProgDummy1;
                This->pDynProgDummy1 = pDp2Dummy1;
                if (pPrevDummy1 == NULL) {
                    pPrevDummy1 = pDp2Dummy1;
                }
                pDp2Dummy1->PoF.ppWordPairEnd = pDp1->ppWordPairEnd;
                *pDp1->ppWordPairEnd = (_ACTIVEDPSTATE *)pDp2Dummy1;
            }

            if (pDp1->BeginOfWord == 0) {
                _ACTIVEDPSTATE *pPred = pDp1->PoF.pPredecessor;
                if (pPred != NULL && pPred->Score < sScore) {
                    _BACKTRACE *FirstPtr;
                    sScore = pPred->Score;
                    if ((pPred->pStateInDict->StateIndex & 2) != 0) {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pPred->pBacktrace;
                        pPred->pBacktrace->Count++;
                        NewBacktrace->State = pPred->pStateInDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        FirstPtr = NewBacktrace;
                    } else {
                        FirstPtr = pPred->pBacktrace;
                    }
                    pDp1->pBacktrace = FirstPtr;
                    FirstPtr->Count++;
                    if (--pBPtr->Count == 0)
                        CleanUpParBTProc(This, pBPtr);
                }
            }

            sScore += (pCachedDistances_DISTBLK[FromPhenom] == 0x7FFF
                           ? (pCachedDistances_DISTBLK[FromPhenom] =
                                  This->CalculateDistance(This->pDistCalc, FromPhenom))
                           : pCachedDistances_DISTBLK[FromPhenom]);
            pDp1->Score = sScore;
            if (sScore <= sNewMin) {
                if (sScore == sNewMin) {
                    UpdateNewMinIfScoreEqual(This, pDp1);
                } else {
                    This->pStsMin = pDp1;
                }
                sNewMin = sScore;
            }

            if (pDp1->Score > sTresh) {
                if (pDp1->pFollower == NULL) {
                    _ACTIVEDPSTATE *NextDp;
                    if (--pDp1->pBacktrace->Count == 0)
                        CleanUpParBTProc(This, pDp1->pBacktrace);
                    if (pDp1->BeginOfWord != 0) {
                        *pDp1->PoF.ppFirstState = NULL;
                    } else if (pDp1->PoF.pPredecessor != NULL) {
                        pDp1->PoF.pPredecessor->pFollower = NULL;
                    }
                    NextDp = pDp1->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                    }
                    if (pPrev != NULL) {
                        pPrev->pNext = NextDp;
                    } else {
                        This->pDynProg = NextDp;
                    }
                    pDp1 = NextDp;
                    --uNewActiveStates;
                    continue;
                }
                if (pDp1->Score > 0x77ff) {
                    pDp1->Score = 0x77ff;
                }
                pPrev = pDp1;
                pDp1 = pDp1->pNext;
                continue;
            }
        }
        pPrev = pDp1;
        pDp1 = pDp1->pNext;
    }

    pPrevDummy1 = NULL;
    pPrevDummy2 = NULL;
    pDp1Dummy1 = This->pDynProgDummy1;
    while (pDp1Dummy1 != NULL) {
        int sScore;
        _BACKTRACE *pBPtr;
        long cPrevWord;
        struct _DICTSTATE *pDict;

        sScore = pDp1Dummy1->Score;
        if (sScore <= sTresh) {
            _DUMMY2NODE *pDpOld;
            pBPtr = pDp1Dummy1->pBacktrace;
            pDict = pDp1Dummy1->pStateInDict;
            cPrevWord = pDict[1].StateIndex + ((unsigned long)pDict[2].StateIndex << 0x10);
            pDpOld = This->ppWordEnds[cPrevWord];
            if (pDpOld != NULL) {
                if (sScore < pDpOld->Score) {
                    _BACKTRACE *FirstPtr = pDpOld->pBacktrace;
                    pDpOld->Score = sScore;
                    if (FirstPtr != NULL) {
                        _MINBACKTRACE *FirstMinPtr;
                        _BACKTRACE *pBackPrev;
                        unsigned long u32BackScore;
                        _MINBACKTRACE *NewBacktrace2;
                        _MINBACKTRACE *pBPtr2;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_MINBACKTRACE) / 4,
                            &NewBacktrace2)
                        NewBacktrace2->Previous = pBPtr;
                        pBPtr->Count++;
                        NewBacktrace2->NextInParallel = NULL;
                        NewBacktrace2->Score = sScore + This->AccumulatedScore;
                        pBPtr2 = NewBacktrace2;
                        FirstMinPtr = pDpOld->pBacktrace->MinBt.NextInParallel;
                        pDpOld->pBacktrace->MinBt.NextInParallel = pBPtr2;
                        pBPtr2->NextInParallel = FirstMinPtr;
                        u32BackScore = pDpOld->pBacktrace->MinBt.Score;
                        pBackPrev = pDpOld->pBacktrace->MinBt.Previous;
                        pDpOld->pBacktrace->MinBt.Score = pBPtr2->Score;
                        pDpOld->pBacktrace->MinBt.Previous = pBPtr2->Previous;
                        pBPtr2->Score = u32BackScore;
                        pBPtr2->Previous = pBackPrev;
                    } else {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pBPtr;
                        pBPtr->Count++;
                        NewBacktrace->State = pDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        FirstPtr = NewBacktrace;
                        pDpOld->pBacktrace = FirstPtr;
                        FirstPtr->Count++;
                    }
                } else {
                    _MINBACKTRACE *FirstMinPtr;
                    _MINBACKTRACE *NewBacktrace2;
                    _MINBACKTRACE *pBPtr2;
                    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_MINBACKTRACE) / 4, &NewBacktrace2)
                    NewBacktrace2->Previous = pBPtr;
                    pBPtr->Count++;
                    NewBacktrace2->NextInParallel = NULL;
                    NewBacktrace2->Score = sScore + This->AccumulatedScore;
                    pBPtr2 = NewBacktrace2;
                    FirstMinPtr = pDpOld->pBacktrace->MinBt.NextInParallel;
                    pDpOld->pBacktrace->MinBt.NextInParallel = pBPtr2;
                    pBPtr2->NextInParallel = FirstMinPtr;
                }
            } else {
                _BACKTRACE *NewBacktrace;
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                NewBacktrace->MinBt.Previous = pBPtr;
                pBPtr->Count++;
                NewBacktrace->State = pDict;
                NewBacktrace->Count = 0;
                NewBacktrace->MinBt.NextInParallel = NULL;
                NewBacktrace->ExitTime = This->u16FrameCounter;
                NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                pBPtr = NewBacktrace;
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY2NODE) / 4, &pDp2Dummy2)
                pDp2Dummy2->Score = sScore;
                pDp2Dummy2->WordID =
                    pDict[1].StateIndex + ((unsigned long) pDict[2].StateIndex << 0x10);
                pDp2Dummy2->pBacktrace = pBPtr;
                pDp2Dummy2->pWordFollowers = NULL;
                pBPtr->Count++;
                pDp2Dummy2->pNext = This->pDynProgDummy2;
                This->pDynProgDummy2 = pDp2Dummy2;
                if (pPrevDummy2 == NULL) {
                    pPrevDummy2 = pDp2Dummy2;
                }
                This->ppWordEnds[cPrevWord] = pDp2Dummy2;
            }
            if (--pDp1Dummy1->pBacktrace->Count == 0)
                CleanUpParBTProc(This, pDp1Dummy1->pBacktrace);
            pDp1Dummy1->Score = 0x7fff;
            pPrevDummy1 = pDp1Dummy1;
            pDp1Dummy1->pBacktrace = NULL;
            pDp1Dummy1 = pDp1Dummy1->pNext;
        } else {
            _DUMMY1NODE *NextDp;
            if (pDp1Dummy1->pBacktrace != NULL && --pDp1Dummy1->pBacktrace->Count == 0)
                CleanUpParBTProc(This, pDp1Dummy1->pBacktrace);
            *pDp1Dummy1->PoF.ppWordPairEnd = NULL;
            NextDp = pDp1Dummy1->pNext;
            {
                struct _FASTALLO_FREE_NODE **pTemp =
                    &pTos->FastAllocHeap.pFreeNodes[sizeof(_DUMMY1NODE) / 4];
                ((struct _FASTALLO_FREE_NODE *)pDp1Dummy1)->pNext = *pTemp;
                *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1Dummy1;
            }
            if (pPrevDummy1 != NULL) {
                pPrevDummy1->pNext = NextDp;
            } else {
                This->pDynProgDummy1 = NextDp;
            }
            pDp1Dummy1 = NextDp;
        }
    }

    pPrevDummy2 = NULL;
    pDp1Dummy2 = This->pDynProgDummy2;
    while (pDp1Dummy2 != NULL) {
        int sScore;
        long cPrevWord;
        _WORDFOLLOWER *pWordFollowers;
        _WORDFOLLOWER *pPrevWordFoll;
        _BACKTRACE *pBPtr;
        long *Ptr;
        long *PtrEnd;
        long cNextWord;
        int sPenalScore;
        unsigned long *PtrBeg;
        unsigned long *PtrBegEnd;
        struct _DICTSTATE *pDict;
        _ACTIVEDPSTATE *pDpOld;
        unsigned int Phenom;
        int NewScore;
        int ReuseNode;

        sScore = pDp1Dummy2->Score;
        cPrevWord = pDp1Dummy2->WordID;
        pWordFollowers = pDp1Dummy2->pWordFollowers;
        pPrevWordFoll = NULL;
        if (sScore <= sTresh) {
            pBPtr = pDp1Dummy2->pBacktrace;
            Ptr = This->ppNext[cPrevWord];
            PtrEnd = This->ppNext[cPrevWord + 1];
            while (Ptr < PtrEnd) {
                cNextWord = *Ptr;
                sPenalScore = sScore + This->pWordPenalties[cNextWord];
                PtrBeg = This->pBeginOfWords + This->pBeginOfWords[cNextWord];
                PtrBegEnd = This->pBeginOfWords + This->pBeginOfWords[cNextWord + 1];
                do {
                    pDict = This->pTheStateDict + *PtrBeg++;
                    pDpOld = NULL;
                    ReuseNode = 0;
                    Phenom = pDict->StateIndex >> 2;
                    NewScore =
                        sPenalScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                           ? (pCachedDistances_DISTBLK[Phenom] =
                                                  This->CalculateDistance(This->pDistCalc, Phenom))
                                           : pCachedDistances_DISTBLK[Phenom]);
                    if (pWordFollowers != NULL) {
                        if (pWordFollowers->pActiveState != NULL) {
                            if (pWordFollowers->pActiveState->pStateInDict == pDict)
                                pDpOld = pWordFollowers->pActiveState;
                        } else {
                            ReuseNode = 1;
                        }
                    }

                    if (pDpOld != NULL) {
                        if (NewScore < pDpOld->Score) {
                            pDpOld->Score = NewScore;
                            if (--pDpOld->pBacktrace->Count == 0)
                                CleanUpParBTProc(This, pDpOld->pBacktrace);
                            pDpOld->pBacktrace = pBPtr;
                            ++pBPtr->Count;
                            if (NewScore <= sNewMin) {
                                if (NewScore == sNewMin) {
                                    UpdateNewMinIfScoreEqual(This, pDpOld);
                                } else {
                                    This->pStsMin = pDpOld;
                                }
                                sNewMin = NewScore;
                            }
                        }
                        pPrevWordFoll = pWordFollowers;
                        pWordFollowers = pWordFollowers->pNext;
                    } else if (NewScore <= sTresh) {
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp2)
                        pDp2->Score = NewScore;
                        pDp2->pStateInDict = pDict;
                        pDp2->pBacktrace = pBPtr;
                        pDp2->pFollower = NULL;
                        pDp2->PoF.pPredecessor = NULL;
                        ++pBPtr->Count;
                        pDp2->pNext = This->pDynProg;
                        This->pDynProg = pDp2;
                        if (pPrev == NULL) {
                            pPrev = pDp2;
                        }
                        pDp2->BeginOfWord = 0;
                        ++uNewActiveStates;

                        if (ReuseNode == 0) {
                            if (pPrevWordFoll != NULL) {
                                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_WORDFOLLOWER) / 4,
                                    (void *)&pPrevWordFoll->pNext)
                                pPrevWordFoll->pNext->pNext = pWordFollowers;
                                pWordFollowers = pPrevWordFoll->pNext;
                            } else {
                                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_WORDFOLLOWER) / 4,
                                    (void *)&pDp1Dummy2->pWordFollowers)
                                pDp1Dummy2->pWordFollowers->pNext = pWordFollowers;
                                pWordFollowers = pDp1Dummy2->pWordFollowers;
                            }
                        }
                        pDp2->PoF.ppFirstState = &pWordFollowers->pActiveState;
                        pDp2->BeginOfWord = 1;
                        pWordFollowers->pActiveState = pDp2;
                        pDp2->ppWordPairEnd = This->ppWordPairEnds +
                            This->NbrOfStart + (Ptr - This->pNextList);
                        if (NewScore <= sNewMin) {
                            if (NewScore == sNewMin) {
                                UpdateNewMinIfScoreEqual(This, pDp2);
                            } else {
                                This->pStsMin = pDp2;
                            }
                            sNewMin = NewScore;
                        }
                        pPrevWordFoll = pWordFollowers;
                        pWordFollowers = pWordFollowers->pNext;
                    } else if (ReuseNode != 0 && pWordFollowers != NULL) {
                        _WORDFOLLOWER *pToDelete = pWordFollowers;
                        if (pPrevWordFoll != NULL) {
                            pPrevWordFoll->pNext = pWordFollowers->pNext;
                        } else {
                            pDp1Dummy2->pWordFollowers = pWordFollowers->pNext;
                        }
                        pWordFollowers = pWordFollowers->pNext;
                        {
                            struct _FASTALLO_FREE_NODE **pTemp =
                                &pTos->FastAllocHeap.pFreeNodes[sizeof(_WORDFOLLOWER) / 4];
                            ((struct _FASTALLO_FREE_NODE *)pToDelete)->pNext = *pTemp;
                            *pTemp = (struct _FASTALLO_FREE_NODE *)pToDelete;
                        }
                    }
                } while (PtrBeg < PtrBegEnd);
                ++Ptr;
            }
            if (--pBPtr->Count == 0)
                CleanUpParBTProc(This, pBPtr);
            pDp1Dummy2->Score = 0x7fff;
            pPrevDummy2 = pDp1Dummy2;
            pDp1Dummy2->pBacktrace = NULL;
            pDp1Dummy2 = pDp1Dummy2->pNext;
        } else {
            while (pWordFollowers != NULL) {
                if (pWordFollowers->pActiveState == NULL) {
                    _WORDFOLLOWER *pToDelete = pWordFollowers;
                    if (pPrevWordFoll != NULL) {
                        pPrevWordFoll->pNext = pWordFollowers->pNext;
                    } else {
                        pDp1Dummy2->pWordFollowers = pWordFollowers->pNext;
                    }
                    pWordFollowers = pWordFollowers->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_WORDFOLLOWER) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pToDelete)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pToDelete;
                    }
                } else {
                    pPrevWordFoll = pWordFollowers;
                    pWordFollowers = pWordFollowers->pNext;
                }
            }
            if (pDp1Dummy2->pWordFollowers == NULL) {
                This->ppWordEnds[cPrevWord] = NULL;
                {
                    _DUMMY2NODE *NextDp;
                    NextDp = pDp1Dummy2->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_DUMMY2NODE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1Dummy2)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1Dummy2;
                    }
                    if (pPrevDummy2 != NULL) {
                        pPrevDummy2->pNext = NextDp;
                    } else {
                        This->pDynProgDummy2 = NextDp;
                    }
                    pDp1Dummy2 = NextDp;
                }
                continue;
            }
            pPrevDummy2 = pDp1Dummy2;
            pDp1Dummy2 = pDp1Dummy2->pNext;
        }
    }

    This->uNbrOfActiveStates = uNewActiveStates;
    This->sMin = sNewMin;
    return 0;
}

/* Expands one frame of the active-state lists with the silence shortcut. */
static int DynProgAccNBestOptSil(_NBESTDYNPROG *This)
{
    struct _TOS *pTos;
    _ACTIVEDPSTATE *pDp1;
    _DUMMY1NODE *pDp1Dummy1;
    _DUMMY2NODE *pDp1Dummy2;
    _ACTIVEDPSTATE *pDp2;
    _DUMMY1NODE *pDp2Dummy1;
    _DUMMY2NODE *pDp2Dummy2;
    _ACTIVEDPSTATE *pPrev;
    _DUMMY1NODE *pPrevDummy1;
    _DUMMY2NODE *pPrevDummy2;
    unsigned int uNewActiveStates;
    int sNewMin;
    int sTresh;
    short *pCachedDistances_DISTBLK;
    int sScore;
    _BACKTRACE *pBPtr;
    struct _DICTSTATE *pDict;
    unsigned int StateIndex;
    unsigned int FromPhenom;
    unsigned int Phenom;

    sNewMin = 0x7FFF;
    pTos = This->BaseBlock.Object.pTos;
    pCachedDistances_DISTBLK = This->pCachedDistances_DISTBLK;
    PrunDynProg(This);
    uNewActiveStates = This->uNbrOfActiveStates;
    sTresh = This->CurrentPruningT + This->sMinClip;
    pPrev = NULL;
    pDp1 = This->pDynProg;
    pPrevDummy1 = NULL;
    while (pDp1 != NULL) {
        pDict = pDp1->pStateInDict;
        sScore = pDp1->Score;
        StateIndex = pDict->StateIndex;
        pBPtr = pDp1->pBacktrace;
        FromPhenom = StateIndex >> 2;
        if ((StateIndex & 1) == 0) {
            StateIndex = (++pDict)->StateIndex;
            Phenom = StateIndex >> 2;
            if ((StateIndex & 1) == 0) {
                _ACTIVEDPSTATE *pDpOld = pDp1->pFollower;
                if (pDpOld == NULL) {
                    int NewScore =
                        sScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                      ? (pCachedDistances_DISTBLK[Phenom] =
                                             This->CalculateDistance(This->pDistCalc, Phenom))
                                      : pCachedDistances_DISTBLK[Phenom]);
                    if (NewScore <= sTresh) {
                        if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                            _BACKTRACE *NewBacktrace;
                            FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                &NewBacktrace)
                            NewBacktrace->MinBt.Previous = pBPtr;
                            ++pBPtr->Count;
                            NewBacktrace->State = pDp1->pStateInDict;
                            NewBacktrace->Count = 0;
                            NewBacktrace->MinBt.NextInParallel = NULL;
                            NewBacktrace->ExitTime = This->u16FrameCounter;
                            NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                            pBPtr = NewBacktrace;
                        }
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp2)
                        pDp2->Score = NewScore;
                        pDp2->pStateInDict = pDict;
                        pDp2->pBacktrace = pBPtr;
                        pDp2->pFollower = NULL;
                        pDp2->PoF.pPredecessor = pDp1;
                        ++pBPtr->Count;
                        pDp2->pNext = This->pDynProg;
                        This->pDynProg = pDp2;
                        if (pPrev == NULL)
                            pPrev = pDp2;
                        pDp2->BeginOfWord = 0;
                        pBPtr = pDp1->pBacktrace;
                        pDp2->ppWordPairEnd = pDp1->ppWordPairEnd;
                        pDp1->pFollower = pDp2;
                        ++uNewActiveStates;
                        if (NewScore <= sNewMin) {
                            if (NewScore == sNewMin) {
                                UpdateNewMinIfScoreEqual(This, pDp2);
                            } else {
                                This->pStsMin = pDp2;
                            }
                            sNewMin = NewScore;
                        }
                    }
                }
                if (pDp1->BeginOfWord == 0) {
                    _ACTIVEDPSTATE *pPred = pDp1->PoF.pPredecessor;
                    if (pPred != NULL && pPred->Score < sScore) {
                        _BACKTRACE *FirstPtr;
                        sScore = pPred->Score;
                        if ((pPred->pStateInDict->StateIndex & 2) != 0) {
                            _BACKTRACE *NewBacktrace;
                            FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                &NewBacktrace)
                            NewBacktrace->MinBt.Previous = pPred->pBacktrace;
                            ++pPred->pBacktrace->Count;
                            NewBacktrace->State = pPred->pStateInDict;
                            NewBacktrace->Count = 0;
                            NewBacktrace->MinBt.NextInParallel = NULL;
                            NewBacktrace->ExitTime = This->u16FrameCounter;
                            NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                            FirstPtr = NewBacktrace;
                        } else {
                            FirstPtr = pPred->pBacktrace;
                        }
                        pDp1->pBacktrace = FirstPtr;
                        ++FirstPtr->Count;
                        if (--pBPtr->Count == 0)
                            CleanUpParBTProc(This, pBPtr);
                    }
                }

                sScore += (pCachedDistances_DISTBLK[FromPhenom] == 0x7FFF
                               ? (pCachedDistances_DISTBLK[FromPhenom] =
                                      This->CalculateDistance(This->pDistCalc, FromPhenom))
                               : pCachedDistances_DISTBLK[FromPhenom]);
                pDp1->Score = sScore;
                if (sScore > sTresh) {
                    if (pDp1->BeginOfWord != 0) {
                        *pDp1->PoF.ppFirstState = NULL;
                    } else if (pDp1->PoF.pPredecessor != NULL) {
                        goto prune_with_predecessor;
                    }
                    {
                        _ACTIVEDPSTATE *NextDp;
                        if (--pDp1->pBacktrace->Count == 0)
                            CleanUpParBTProc(This, pDp1->pBacktrace);
                        if (pDp1->pFollower != NULL)
                            pDp1->pFollower->PoF.pPredecessor = NULL;
                        NextDp = pDp1->pNext;
                        {
                            struct _FASTALLO_FREE_NODE **pTemp =
                                &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                            ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                            *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                        }
                        if (pPrev != NULL) {
                            pPrev->pNext = NextDp;
                        } else {
                            This->pDynProg = NextDp;
                        }
                        pDp1 = NextDp;
                        --uNewActiveStates;
                        continue;
                    }
prune_with_predecessor:
                    if (pDp1->pFollower == NULL) {
                        _ACTIVEDPSTATE *NextDp;
                        if (--pDp1->pBacktrace->Count == 0)
                            CleanUpParBTProc(This, pDp1->pBacktrace);
                        pDp1->PoF.pPredecessor->pFollower = NULL;
                        NextDp = pDp1->pNext;
                        {
                            struct _FASTALLO_FREE_NODE **pTemp =
                                &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                            ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                            *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                        }
                        if (pPrev != NULL)
                            pPrev->pNext = NextDp;
                        else
                            This->pDynProg = NextDp;
                        pDp1 = NextDp;
                        --uNewActiveStates;
                    } else {
                        if (sScore > 0x77FF) {
                            pDp1->Score = 0x77FF;
                        }
                        pPrev = pDp1;
                        pDp1 = pDp1->pNext;
                    }
                } else {
                    if (sScore <= sNewMin) {
                        if (sScore == sNewMin) {
                            UpdateNewMinIfScoreEqual(This, pDp1);
                        } else {
                            This->pStsMin = pDp1;
                        }
                        sNewMin = sScore;
                    }
                    pPrev = pDp1;
                    pDp1 = pDp1->pNext;
                }
            } else {
                int NewSilTransScore =
                    sScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                  ? (pCachedDistances_DISTBLK[Phenom] =
                                         This->CalculateDistance(This->pDistCalc, Phenom))
                                  : pCachedDistances_DISTBLK[Phenom]);
                _ACTIVEDPSTATE *pDpOld = *pDp1->ppWordPairEnd;

                if (pDpOld != NULL) {
                    int SilScore = pDpOld->Score;
                    int NewSilLoopScore =
                        SilScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                        ? (pCachedDistances_DISTBLK[Phenom] =
                                               This->CalculateDistance(This->pDistCalc, Phenom))
                                        : pCachedDistances_DISTBLK[Phenom]);
                    _BACKTRACE *pSilBacktrace = pDpOld->pBacktrace;
                    _DUMMY1NODE *pDumDpOld = (_DUMMY1NODE *)pDpOld->pFollower;
                    if (pDumDpOld != NULL) {
                        if (pDumDpOld->Score == 0x7FFF) {
                            pDpOld->Score = NewSilLoopScore;
                            pDumDpOld->Score = SilScore;
                            pDumDpOld->pStateInDict = pDict;
                            pDumDpOld->pBacktrace = pSilBacktrace;
                            ++pSilBacktrace->Count;
                            if (NewSilLoopScore <= sNewMin) {
                                if (NewSilLoopScore == sNewMin) {
                                    UpdateNewMinIfScoreEqual(This, pDpOld);
                                } else {
                                    This->pStsMin = pDpOld;
                                }
                                sNewMin = NewSilLoopScore;
                            }
                        }
                    } else if (SilScore <= sTresh) {
                        _BACKTRACE *pSaveBPtr = pBPtr;
                        pBPtr = pSilBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY1NODE) / 4, &pDp2Dummy1)
                        pDp2Dummy1->Score = SilScore;
                        pDp2Dummy1->pStateInDict = pDict;
                        pDp2Dummy1->pBacktrace = pBPtr;
                        pDp2Dummy1->PoF.pPredecessor = pDpOld;
                        ++pBPtr->Count;
                        pDp2Dummy1->pNext = This->pDynProgDummy1;
                        This->pDynProgDummy1 = pDp2Dummy1;
                        if (pPrevDummy1 == NULL)
                            pPrevDummy1 = pDp2Dummy1;
                        pDumDpOld = pDp2Dummy1;
                        pDpOld->pFollower = (_ACTIVEDPSTATE *)pDumDpOld;
                        pDpOld->Score = NewSilLoopScore;
                        pBPtr = pSaveBPtr;
                        if (NewSilLoopScore <= sNewMin) {
                            if (NewSilLoopScore == sNewMin) {
                                UpdateNewMinIfScoreEqual(This, pDpOld);
                            } else {
                                This->pStsMin = pDpOld;
                            }
                            sNewMin = NewSilLoopScore;
                        }
                    }
                    if (NewSilTransScore < pDpOld->Score) {
                        _BACKTRACE *FirstPtr;
                        pDpOld->Score = NewSilTransScore;
                        if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                            _BACKTRACE *NewBacktrace;
                            FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                &NewBacktrace)
                            NewBacktrace->MinBt.Previous = pBPtr;
                            ++pBPtr->Count;
                            NewBacktrace->State = pDp1->pStateInDict;
                            NewBacktrace->Count = 0;
                            NewBacktrace->MinBt.NextInParallel = NULL;
                            NewBacktrace->ExitTime = This->u16FrameCounter;
                            NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                            pBPtr = NewBacktrace;
                        }
                        FirstPtr = pDpOld->pBacktrace;
                        pDpOld->pBacktrace = pBPtr;
                        ++pBPtr->Count;
                        if (--FirstPtr->Count == 0)
                            CleanUpParBTProc(This, FirstPtr);
                        pBPtr = pDp1->pBacktrace;
                        if (NewSilTransScore <= sNewMin) {
                            if (NewSilTransScore == sNewMin) {
                                UpdateNewMinIfScoreEqual(This, pDpOld);
                            } else {
                                This->pStsMin = pDpOld;
                            }
                            sNewMin = NewSilTransScore;
                        }
                    }
                    if (pDumDpOld != NULL) {
                        if (sScore < pDumDpOld->Score) {
                            _BACKTRACE *FirstPtr;
                            pDumDpOld->pStateInDict = pDict;
                            pDumDpOld->Score = sScore;
                            if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                                _BACKTRACE *NewBacktrace;
                                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                    &NewBacktrace)
                                NewBacktrace->MinBt.Previous = pBPtr;
                                ++pBPtr->Count;
                                NewBacktrace->State = pDp1->pStateInDict;
                                NewBacktrace->Count = 0;
                                NewBacktrace->MinBt.NextInParallel = NULL;
                                NewBacktrace->ExitTime = This->u16FrameCounter;
                                NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                                pBPtr = NewBacktrace;
                            }
                            FirstPtr = pDumDpOld->pBacktrace;
                            pDumDpOld->pBacktrace = pBPtr;
                            ++pBPtr->Count;
                            if (FirstPtr != NULL && --FirstPtr->Count == 0)
                                CleanUpParBTProc(This, FirstPtr);
                            pBPtr = pDp1->pBacktrace;
                        }
                    } else if (sScore <= sTresh) {
                        if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                            _BACKTRACE *NewBacktrace;
                            FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                &NewBacktrace)
                            NewBacktrace->MinBt.Previous = pBPtr;
                            ++pBPtr->Count;
                            NewBacktrace->State = pDp1->pStateInDict;
                            NewBacktrace->Count = 0;
                            NewBacktrace->MinBt.NextInParallel = NULL;
                            NewBacktrace->ExitTime = This->u16FrameCounter;
                            NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                            pBPtr = NewBacktrace;
                        }
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY1NODE) / 4, &pDp2Dummy1)
                        pDp2Dummy1->Score = sScore;
                        pDp2Dummy1->pStateInDict = pDict;
                        pDp2Dummy1->pBacktrace = pBPtr;
                        pDp2Dummy1->PoF.pPredecessor = pDpOld;
                        ++pBPtr->Count;
                        pDp2Dummy1->pNext = This->pDynProgDummy1;
                        This->pDynProgDummy1 = pDp2Dummy1;
                        if (pPrevDummy1 == NULL)
                            pPrevDummy1 = pDp2Dummy1;
                        pDpOld->pFollower = (_ACTIVEDPSTATE *)pDp2Dummy1;
                        pBPtr = pDp1->pBacktrace;
                    }
                } else {
                    _ACTIVEDPSTATE *pSilDp1;
                    if ((pDp1->pStateInDict->StateIndex & 2) != 0) {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pBPtr;
                        ++pBPtr->Count;
                        NewBacktrace->State = pDp1->pStateInDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        pBPtr = NewBacktrace;
                    }
                    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp2)
                    pDp2->Score = NewSilTransScore;
                    pDp2->pStateInDict = pDict;
                    pDp2->pBacktrace = pBPtr;
                    pDp2->pFollower = NULL;
                    pDp2->PoF.pPredecessor = NULL;
                    ++pBPtr->Count;
                    pDp2->pNext = This->pDynProg;
                    This->pDynProg = pDp2;
                    if (pPrev == NULL)
                        pPrev = pDp2;
                    pDp2->BeginOfWord = 0;
                    ++uNewActiveStates;
                    pDp1->pFollower = pDp2;
                    pDp2->ppWordPairEnd = pDp1->ppWordPairEnd;
                    *pDp1->ppWordPairEnd = pDp2;
                    if (NewSilTransScore <= sNewMin) {
                        if (NewSilTransScore == sNewMin) {
                            UpdateNewMinIfScoreEqual(This, pDp2);
                        } else {
                            This->pStsMin = pDp2;
                        }
                        sNewMin = NewSilTransScore;
                    }
                    pSilDp1 = pDp2;
                    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY1NODE) / 4, &pDp2Dummy1)
                    pDp2Dummy1->Score = sScore;
                    pDp2Dummy1->pStateInDict = pDict;
                    pDp2Dummy1->pBacktrace = pBPtr;
                    pDp2Dummy1->PoF.pPredecessor = pSilDp1;
                    ++pBPtr->Count;
                    pDp2Dummy1->pNext = This->pDynProgDummy1;
                    This->pDynProgDummy1 = pDp2Dummy1;
                    if (pPrevDummy1 == NULL)
                        pPrevDummy1 = pDp2Dummy1;
                    pSilDp1->pFollower = (_ACTIVEDPSTATE *)pDp2Dummy1;
                    pBPtr = pDp1->pBacktrace;
                }

                if (pDp1->BeginOfWord == 0) {
                    _ACTIVEDPSTATE *pPred = pDp1->PoF.pPredecessor;
                    if (pPred != NULL && pPred->Score < sScore) {
                        _BACKTRACE *FirstPtr;
                        sScore = pPred->Score;
                        if ((pPred->pStateInDict->StateIndex & 2) != 0) {
                            _BACKTRACE *NewBacktrace;
                            FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4,
                                &NewBacktrace)
                            NewBacktrace->MinBt.Previous = pPred->pBacktrace;
                            ++pPred->pBacktrace->Count;
                            NewBacktrace->State = pPred->pStateInDict;
                            NewBacktrace->Count = 0;
                            NewBacktrace->MinBt.NextInParallel = NULL;
                            NewBacktrace->ExitTime = This->u16FrameCounter;
                            NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                            FirstPtr = NewBacktrace;
                        } else {
                            FirstPtr = pPred->pBacktrace;
                        }
                        pDp1->pBacktrace = FirstPtr;
                        ++FirstPtr->Count;
                        if (--pBPtr->Count == 0)
                            CleanUpParBTProc(This, pBPtr);
                    }
                }
                sScore += (pCachedDistances_DISTBLK[FromPhenom] == 0x7FFF
                               ? (pCachedDistances_DISTBLK[FromPhenom] =
                                      This->CalculateDistance(This->pDistCalc, FromPhenom))
                               : pCachedDistances_DISTBLK[FromPhenom]);
                pDp1->Score = sScore;
                if (sScore > sTresh) {
                    _ACTIVEDPSTATE *NextDp;
                    if (--pDp1->pBacktrace->Count == 0)
                        CleanUpParBTProc(This, pDp1->pBacktrace);
                    if (pDp1->BeginOfWord != 0) {
                        *pDp1->PoF.ppFirstState = NULL;
                    } else if (pDp1->PoF.pPredecessor != NULL) {
                        pDp1->PoF.pPredecessor->pFollower = NULL;
                    }
                    NextDp = pDp1->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                    }
                    if (pPrev != NULL)
                        pPrev->pNext = NextDp;
                    else
                        This->pDynProg = NextDp;
                    pDp1 = NextDp;
                    --uNewActiveStates;
                } else {
                    if (sScore <= sNewMin) {
                        if (sScore == sNewMin) {
                            UpdateNewMinIfScoreEqual(This, pDp1);
                        } else {
                            This->pStsMin = pDp1;
                        }
                        sNewMin = sScore;
                    }
                    pPrev = pDp1;
                    pDp1 = pDp1->pNext;
                }
            }
        } else {
            int LocSilScore = (pCachedDistances_DISTBLK[FromPhenom] == 0x7FFF
                                   ? (pCachedDistances_DISTBLK[FromPhenom] =
                                          This->CalculateDistance(This->pDistCalc, FromPhenom))
                                   : pCachedDistances_DISTBLK[FromPhenom]);
            _DUMMY1NODE *pDpOld = (_DUMMY1NODE *)pDp1->pFollower;
            if (pDpOld != NULL) {
                if (pDpOld->Score == 0x7FFF) {
                    pDpOld->Score = sScore;
                    sScore += LocSilScore;
                    pDpOld->pStateInDict = pDict;
                    pDpOld->pBacktrace = pBPtr;
                    ++pBPtr->Count;
                    pDp1->Score = sScore;
                    if (sScore <= sNewMin) {
                        if (sScore == sNewMin) {
                            UpdateNewMinIfScoreEqual(This, pDp1);
                        } else {
                            This->pStsMin = pDp1;
                        }
                        sNewMin = sScore;
                    }
                }
            } else if (sScore <= sTresh) {
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY1NODE) / 4, &pDp2Dummy1)
                pDp2Dummy1->Score = sScore;
                pDp2Dummy1->pStateInDict = pDict;
                pDp2Dummy1->pBacktrace = pBPtr;
                pDp2Dummy1->PoF.pPredecessor = pDp1;
                ++pBPtr->Count;
                pDp2Dummy1->pNext = This->pDynProgDummy1;
                This->pDynProgDummy1 = pDp2Dummy1;
                if (pPrevDummy1 == NULL)
                    pPrevDummy1 = pDp2Dummy1;
                sScore += LocSilScore;
                pDp1->pFollower = (_ACTIVEDPSTATE *)pDp2Dummy1;
                pDp1->Score = sScore;
                if (sScore <= sNewMin) {
                    if (sScore == sNewMin) {
                        UpdateNewMinIfScoreEqual(This, pDp1);
                    } else {
                        This->pStsMin = pDp1;
                    }
                    sNewMin = sScore;
                }
            }
            if (sScore > sTresh) {
                if (pDp1->pFollower == NULL) {
                    _ACTIVEDPSTATE *NextDp;
                    if (--pDp1->pBacktrace->Count == 0)
                        CleanUpParBTProc(This, pDp1->pBacktrace);
                    *pDp1->ppWordPairEnd = NULL;
                    NextDp = pDp1->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1;
                    }
                    if (pPrev != NULL)
                        pPrev->pNext = NextDp;
                    else
                        This->pDynProg = NextDp;
                    pDp1 = NextDp;
                    --uNewActiveStates;
                    continue;
                }
                if (sScore > 0x77FF)
                    pDp1->Score = 0x77FF;
                pPrev = pDp1;
                pDp1 = pDp1->pNext;
                continue;
            }
            pPrev = pDp1;
            pDp1 = pDp1->pNext;
        }
    }
    pDp1Dummy1 = This->pDynProgDummy1;
    pPrevDummy1 = NULL;
    pPrevDummy2 = NULL;
    while (pDp1Dummy1 != NULL) {
        int sScore;
        _BACKTRACE *pBPtr;
        long cPrevWord;
        struct _DICTSTATE *pDict;

        sScore = pDp1Dummy1->Score;
        if (sScore <= sTresh) {
            _DUMMY2NODE *pDpOld;
            pBPtr = pDp1Dummy1->pBacktrace;
            pDict = pDp1Dummy1->pStateInDict;
            cPrevWord = pDict[1].StateIndex + ((unsigned long)pDict[2].StateIndex << 0x10);
            pDpOld = This->ppWordEnds[cPrevWord];
            if (pDpOld != NULL) {
                if (sScore < pDpOld->Score) {
                    _BACKTRACE *FirstPtr = pDpOld->pBacktrace;
                    pDpOld->Score = sScore;
                    if (FirstPtr != NULL) {
                        _MINBACKTRACE *FirstMinPtr;
                        unsigned long u32BackScore;
                        _BACKTRACE *pBackPrev;
                        _MINBACKTRACE *NewBacktrace2;
                        _MINBACKTRACE *pBPtr2;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_MINBACKTRACE) / 4,
                            &NewBacktrace2)
                        NewBacktrace2->Previous = pBPtr;
                        pBPtr->Count++;
                        NewBacktrace2->NextInParallel = NULL;
                        NewBacktrace2->Score = sScore + This->AccumulatedScore;
                        pBPtr2 = NewBacktrace2;
                        FirstMinPtr = pDpOld->pBacktrace->MinBt.NextInParallel;
                        pDpOld->pBacktrace->MinBt.NextInParallel = pBPtr2;
                        pBPtr2->NextInParallel = FirstMinPtr;
                        u32BackScore = pDpOld->pBacktrace->MinBt.Score;
                        pBackPrev = pDpOld->pBacktrace->MinBt.Previous;
                        pDpOld->pBacktrace->MinBt.Score = pBPtr2->Score;
                        pDpOld->pBacktrace->MinBt.Previous = pBPtr2->Previous;
                        pBPtr2->Score = u32BackScore;
                        pBPtr2->Previous = pBackPrev;
                    } else {
                        _BACKTRACE *NewBacktrace;
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                        NewBacktrace->MinBt.Previous = pBPtr;
                        pBPtr->Count++;
                        NewBacktrace->State = pDict;
                        NewBacktrace->Count = 0;
                        NewBacktrace->MinBt.NextInParallel = NULL;
                        NewBacktrace->ExitTime = This->u16FrameCounter;
                        NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                        FirstPtr = NewBacktrace;
                        pDpOld->pBacktrace = FirstPtr;
                        FirstPtr->Count++;
                    }
                } else {
                    _MINBACKTRACE *FirstMinPtr;
                    _MINBACKTRACE *NewBacktrace2;
                    _MINBACKTRACE *pBPtr2;
                    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_MINBACKTRACE) / 4, &NewBacktrace2)
                    NewBacktrace2->Previous = pBPtr;
                    pBPtr->Count++;
                    NewBacktrace2->NextInParallel = NULL;
                    NewBacktrace2->Score = sScore + This->AccumulatedScore;
                    pBPtr2 = NewBacktrace2;
                    FirstMinPtr = pDpOld->pBacktrace->MinBt.NextInParallel;
                    pDpOld->pBacktrace->MinBt.NextInParallel = pBPtr2;
                    pBPtr2->NextInParallel = FirstMinPtr;
                }
            } else {
                _BACKTRACE *NewBacktrace;
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)
                NewBacktrace->MinBt.Previous = pBPtr;
                pBPtr->Count++;
                NewBacktrace->State = pDict;
                NewBacktrace->Count = 0;
                NewBacktrace->MinBt.NextInParallel = NULL;
                NewBacktrace->ExitTime = This->u16FrameCounter;
                NewBacktrace->MinBt.Score = sScore + This->AccumulatedScore;
                pBPtr = NewBacktrace;
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_DUMMY2NODE) / 4, &pDp2Dummy2)
                pDp2Dummy2->Score = sScore;
                pDp2Dummy2->WordID =
                    pDict[1].StateIndex + ((unsigned long) pDict[2].StateIndex << 0x10);
                pDp2Dummy2->pBacktrace = pBPtr;
                pDp2Dummy2->pWordFollowers = NULL;
                pBPtr->Count++;
                pDp2Dummy2->pNext = This->pDynProgDummy2;
                This->pDynProgDummy2 = pDp2Dummy2;
                if (pPrevDummy2 == NULL) {
                    pPrevDummy2 = pDp2Dummy2;
                }
                This->ppWordEnds[cPrevWord] = pDp2Dummy2;
            }
            if (--pDp1Dummy1->pBacktrace->Count == 0)
                CleanUpParBTProc(This, pDp1Dummy1->pBacktrace);
            pDp1Dummy1->Score = 0x7fff;
            pPrevDummy1 = pDp1Dummy1;
            pDp1Dummy1->pBacktrace = NULL;
            pDp1Dummy1 = pDp1Dummy1->pNext;
        } else {
            _DUMMY1NODE *NextDp;
            if (pDp1Dummy1->pBacktrace != NULL && --pDp1Dummy1->pBacktrace->Count == 0)
                CleanUpParBTProc(This, pDp1Dummy1->pBacktrace);
            pDp1Dummy1->PoF.pPredecessor->pFollower = NULL;
            NextDp = pDp1Dummy1->pNext;
            {
                struct _FASTALLO_FREE_NODE **pTemp =
                    &pTos->FastAllocHeap.pFreeNodes[sizeof(_DUMMY1NODE) / 4];
                ((struct _FASTALLO_FREE_NODE *)pDp1Dummy1)->pNext = *pTemp;
                *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1Dummy1;
            }
            if (pPrevDummy1 != NULL) {
                pPrevDummy1->pNext = NextDp;
            } else {
                This->pDynProgDummy1 = NextDp;
            }
            pDp1Dummy1 = NextDp;
        }
    }

    pPrevDummy2 = NULL;
    pDp1Dummy2 = This->pDynProgDummy2;
    while (pDp1Dummy2 != NULL) {
        int sScore;
        long cPrevWord;
        _WORDFOLLOWER *pWordFollowers;
        _WORDFOLLOWER *pPrevWordFoll;
        _BACKTRACE *pBPtr;
        long *Ptr;
        long *PtrEnd;
        long cNextWord;
        int sPenalScore;
        unsigned long *PtrBeg;
        unsigned long *PtrBegEnd;
        struct _DICTSTATE *pDict;
        _ACTIVEDPSTATE *pDpOld;
        unsigned int Phenom;
        int NewScore;
        int ReuseNode;

        sScore = pDp1Dummy2->Score;
        cPrevWord = pDp1Dummy2->WordID;
        pWordFollowers = pDp1Dummy2->pWordFollowers;
        pPrevWordFoll = NULL;
        if (sScore <= sTresh) {
            pBPtr = pDp1Dummy2->pBacktrace;
            Ptr = This->ppNext[cPrevWord];
            PtrEnd = This->ppNext[cPrevWord + 1];
            while (Ptr < PtrEnd) {
                cNextWord = *Ptr;
                sPenalScore = sScore + This->pWordPenalties[cNextWord];
                PtrBeg = This->pBeginOfWords + This->pBeginOfWords[cNextWord];
                PtrBegEnd = This->pBeginOfWords + This->pBeginOfWords[cNextWord + 1];
                do {
                    pDict = This->pTheStateDict + *PtrBeg++;
                    pDpOld = NULL;
                    Phenom = pDict->StateIndex >> 2;
                    NewScore =
                        sPenalScore + (pCachedDistances_DISTBLK[Phenom] == 0x7FFF
                                           ? (pCachedDistances_DISTBLK[Phenom] =
                                                  This->CalculateDistance(This->pDistCalc, Phenom))
                                           : pCachedDistances_DISTBLK[Phenom]);
                    ReuseNode = 0;
                    if (pWordFollowers != NULL) {
                        if (pWordFollowers->pActiveState != NULL) {
                            if (pWordFollowers->pActiveState->pStateInDict == pDict)
                                pDpOld = pWordFollowers->pActiveState;
                        } else {
                            ReuseNode = 1;
                        }
                    }

                    if (pDpOld != NULL) {
                        if (NewScore < pDpOld->Score) {
                            pDpOld->Score = NewScore;
                            if (--pDpOld->pBacktrace->Count == 0)
                                CleanUpParBTProc(This, pDpOld->pBacktrace);
                            pDpOld->pBacktrace = pBPtr;
                            ++pBPtr->Count;
                            if (NewScore <= sNewMin) {
                                if (NewScore == sNewMin) {
                                    UpdateNewMinIfScoreEqual(This, pDpOld);
                                } else {
                                    This->pStsMin = pDpOld;
                                }
                                sNewMin = NewScore;
                            }
                        }
                        pPrevWordFoll = pWordFollowers;
                        pWordFollowers = pWordFollowers->pNext;
                    } else if (NewScore <= sTresh) {
                        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_ACTIVEDPSTATE) / 4, &pDp2)
                        pDp2->Score = NewScore;
                        pDp2->pStateInDict = pDict;
                        pDp2->pBacktrace = pBPtr;
                        pDp2->pFollower = NULL;
                        pDp2->PoF.pPredecessor = NULL;
                        ++pBPtr->Count;
                        pDp2->pNext = This->pDynProg;
                        This->pDynProg = pDp2;
                        if (pPrev == NULL) {
                            pPrev = pDp2;
                        }
                        pDp2->BeginOfWord = 0;
                        ++uNewActiveStates;

                        if (ReuseNode == 0) {
                            if (pPrevWordFoll != NULL) {
                                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_WORDFOLLOWER) / 4,
                                    (void *)&pPrevWordFoll->pNext)
                                pPrevWordFoll->pNext->pNext = pWordFollowers;
                                pWordFollowers = pPrevWordFoll->pNext;
                            } else {
                                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_WORDFOLLOWER) / 4,
                                    (void *)&pDp1Dummy2->pWordFollowers)
                                pDp1Dummy2->pWordFollowers->pNext = pWordFollowers;
                                pWordFollowers = pDp1Dummy2->pWordFollowers;
                            }
                        }
                        pDp2->PoF.ppFirstState = &pWordFollowers->pActiveState;
                        pDp2->BeginOfWord = 1;
                        pWordFollowers->pActiveState = pDp2;
                        pDp2->ppWordPairEnd = This->ppWordPairEnds +
                            This->NbrOfStart + (Ptr - This->pNextList);
                        if (NewScore <= sNewMin) {
                            if (NewScore == sNewMin) {
                                UpdateNewMinIfScoreEqual(This, pDp2);
                            } else {
                                This->pStsMin = pDp2;
                            }
                            sNewMin = NewScore;
                        }
                        pPrevWordFoll = pWordFollowers;
                        pWordFollowers = pWordFollowers->pNext;
                    } else if (ReuseNode != 0 && pWordFollowers != NULL) {
                        _WORDFOLLOWER *pToDelete = pWordFollowers;
                        if (pPrevWordFoll != NULL)
                            pPrevWordFoll->pNext = pWordFollowers->pNext;
                        else
                            pDp1Dummy2->pWordFollowers = pWordFollowers->pNext;
                        pWordFollowers = pWordFollowers->pNext;
                        {
                            struct _FASTALLO_FREE_NODE **pTemp =
                                &pTos->FastAllocHeap.pFreeNodes[sizeof(_WORDFOLLOWER) / 4];
                            ((struct _FASTALLO_FREE_NODE *)pToDelete)->pNext = *pTemp;
                            *pTemp = (struct _FASTALLO_FREE_NODE *)pToDelete;
                        }
                    }
                } while (PtrBeg < PtrBegEnd);
                ++Ptr;
            }
            if (--pBPtr->Count == 0)
                CleanUpParBTProc(This, pBPtr);
            pDp1Dummy2->Score = 0x7fff;
            pPrevDummy2 = pDp1Dummy2;
            pDp1Dummy2->pBacktrace = NULL;
            pDp1Dummy2 = pDp1Dummy2->pNext;
        } else {
            while (pWordFollowers != NULL) {
                if (pWordFollowers->pActiveState == NULL) {
                    _WORDFOLLOWER *pToDelete = pWordFollowers;
                    if (pPrevWordFoll != NULL) {
                        pPrevWordFoll->pNext = pWordFollowers->pNext;
                    } else {
                        pDp1Dummy2->pWordFollowers = pWordFollowers->pNext;
                    }
                    pWordFollowers = pWordFollowers->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_WORDFOLLOWER) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pToDelete)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pToDelete;
                    }
                } else {
                    pPrevWordFoll = pWordFollowers;
                    pWordFollowers = pWordFollowers->pNext;
                }
            }
            if (pDp1Dummy2->pWordFollowers == NULL) {
                This->ppWordEnds[cPrevWord] = NULL;
                {
                    _DUMMY2NODE *NextDp;
                    NextDp = pDp1Dummy2->pNext;
                    {
                        struct _FASTALLO_FREE_NODE **pTemp =
                            &pTos->FastAllocHeap.pFreeNodes[sizeof(_DUMMY2NODE) / 4];
                        ((struct _FASTALLO_FREE_NODE *)pDp1Dummy2)->pNext = *pTemp;
                        *pTemp = (struct _FASTALLO_FREE_NODE *)pDp1Dummy2;
                    }
                    if (pPrevDummy2 != NULL) {
                        pPrevDummy2->pNext = NextDp;
                    } else {
                        This->pDynProgDummy2 = NextDp;
                    }
                    pDp1Dummy2 = NextDp;
                }
                continue;
            }
            pPrevDummy2 = pDp1Dummy2;
            pDp1Dummy2 = pDp1Dummy2->pNext;
        }
    }

    This->uNbrOfActiveStates = uNewActiveStates;
    This->sMin = sNewMin;
    return 0;
}

/* Copies a full backtrace and its parallel alternatives. */
static _BACKTRACE *CopyParBT(_NBESTDYNPROG *This, _BACKTRACE *ThisPtr)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _BACKTRACE *SecondPtr;
    _BACKTRACE *NewBacktrace;
    _MINBACKTRACE *PrevPtr;
    _MINBACKTRACE *Second2Ptr;
    FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewBacktrace)

    SecondPtr = ThisPtr;
    NewBacktrace->State = SecondPtr->State;
    NewBacktrace->Count = 1;
    NewBacktrace->MinBt.Score = SecondPtr->MinBt.Score;
    NewBacktrace->ExitTime = SecondPtr->ExitTime;
    NewBacktrace->MinBt.Previous = SecondPtr->MinBt.Previous;
    NewBacktrace->MinBt.Previous->Count++;
    NewBacktrace->MinBt.NextInParallel = NULL;
    PrevPtr = &NewBacktrace->MinBt;
    Second2Ptr = SecondPtr->MinBt.NextInParallel;
    while (Second2Ptr != NULL) {
        _MINBACKTRACE *NewBacktrace2;
        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_MINBACKTRACE) / 4, &NewBacktrace2)

        NewBacktrace2->Score = Second2Ptr->Score;
        NewBacktrace2->Previous = Second2Ptr->Previous;
        NewBacktrace2->Previous->Count++;
        NewBacktrace2->NextInParallel = NULL;
        if (PrevPtr != NULL) {
            PrevPtr->NextInParallel = NewBacktrace2;
        }
        PrevPtr = NewBacktrace2;
        Second2Ptr = Second2Ptr->NextInParallel;
    }
    NewBacktrace->Count = 0;
    return NewBacktrace;
}

/* Moves the lowest-scoring alternative to the head of a backtrace. */
static _BACKTRACE *BestBTAhead(_BACKTRACE *ThisPtr)
{
    unsigned long BestScore;
    _MINBACKTRACE *thisPtr;
    _MINBACKTRACE *pBest;

    BestScore = ThisPtr->MinBt.Score;
    pBest = NULL;
    thisPtr = ThisPtr->MinBt.NextInParallel;
    while (thisPtr != NULL) {
        if (thisPtr->Score < BestScore) {
            pBest = thisPtr;
            BestScore = thisPtr->Score;
        }
        thisPtr = thisPtr->NextInParallel;
    }
    if (pBest != NULL) {
        unsigned long u32Score = pBest->Score;
        _BACKTRACE *pPrev;
        pBest->Score = ThisPtr->MinBt.Score;
        ThisPtr->MinBt.Score = u32Score;
        pPrev = pBest->Previous;
        pBest->Previous = ThisPtr->MinBt.Previous;
        ThisPtr->MinBt.Previous = pPrev;
    }
    return ThisPtr;
}

/* Moves the lowest-scoring alternative to the head of a minimal backtrace. */
static _MINBACKTRACE *BestMinBTAhead(_MINBACKTRACE *ThisPtr)
{
    unsigned long BestScore;
    _MINBACKTRACE *thisPtr;
    _MINBACKTRACE *pBest;

    BestScore = ThisPtr->Score;
    pBest = NULL;
    thisPtr = ThisPtr->NextInParallel;
    while (thisPtr != NULL) {
        if (thisPtr->Score < BestScore) {
            pBest = thisPtr;
            BestScore = thisPtr->Score;
        }
        thisPtr = thisPtr->NextInParallel;
    }
    if (pBest != NULL) {
        unsigned long u32Score = pBest->Score;
        _BACKTRACE *pPrev;
        pBest->Score = ThisPtr->Score;
        ThisPtr->Score = u32Score;
        pPrev = pBest->Previous;
        pBest->Previous = ThisPtr->Previous;
        ThisPtr->Previous = pPrev;
    }
    return ThisPtr;
}

/* Reconstructs the recognized word chain represented by a backtrace. */
static struct _DYNPROG_RESULT_NODE *BacktraceWordsParBT(_NBESTDYNPROG *This,
                                                      _ACTIVEDPSTATE *pDPState)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    struct _DYNPROG_RESULT_NODE *pRes;
    struct _DYNPROG_RESULT_NODE *pToAdd = NULL;
    _BACKTRACE *ThisPtr = pDPState->pBacktrace;
    _BACKTRACE *PrevPtr;
    _BACKTRACE *NewPtr;
    _BACKTRACE *LastPtr = NULL;
    _BACKTRACESTACK *StackPtr = This->TopOfBacktraceStack;
    signed short DiffScore = 0;

    if (ThisPtr != NULL) {
        struct _DYNPROG_RESULT_NODE *pWord;
        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(struct _DYNPROG_RESULT_NODE) / 4, &pWord)
        pWord->StateNbr = pDPState->pStateInDict - This->pTheStateDict;
        pWord->pNext = NULL;
        pWord->pInfo = 0;
        pToAdd = pWord;
        pWord->ExitTime = This->u16FrameCounter;
        pWord->Score = pDPState->Score + This->AccumulatedScore;
    }
    while (ThisPtr != NULL) {
        if (ThisPtr->State != 0) {
            PrevPtr = ThisPtr->MinBt.Previous;
            if (PrevPtr != NULL &&
                (PrevPtr->ExitTime != ThisPtr->ExitTime || PrevPtr->State != ThisPtr->State)) {
                struct _DYNPROG_RESULT_NODE *pNewWord;
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(struct _DYNPROG_RESULT_NODE) / 4,
                              &pNewWord)
                pNewWord->StateNbr = ThisPtr->State - This->pTheStateDict;
                pNewWord->pNext = pToAdd;
                pNewWord->pInfo = 0;
                pNewWord->ExitTime = ThisPtr->ExitTime;
                pNewWord->Score = ThisPtr->MinBt.Score;
                pToAdd = pNewWord;
            }
            if (ThisPtr == pDPState->pBacktrace) {
                NewPtr = ThisPtr;
            } else if (ThisPtr->Count <= 1) {
                NewPtr = ThisPtr;
            } else {
                NewPtr = CopyParBT(This, ThisPtr);
            }
            StackPtr->ToDo = NewPtr;
            if (StackPtr->Down == NULL) {
                ExtendBacktraceStack(This, StackPtr);
            }
            StackPtr = StackPtr->Down;
            if (NewPtr != ThisPtr && LastPtr != NULL) {
                NewPtr->Count++;
                if (--LastPtr->MinBt.Previous->Count == 0)
                    CleanUpParBTProc(This, LastPtr->MinBt.Previous);
                LastPtr->MinBt.Previous = NewPtr;
            }
            LastPtr = NewPtr;
            ThisPtr = PrevPtr;
        } else {
            ThisPtr = NULL;
        }
    }
    if (pToAdd != NULL) {
        FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(struct _DYNPROG_RESULT_NODE) / 4, &pRes)
        pRes->pInfo = pToAdd;
        pRes->Score = pDPState->Score + This->AccumulatedScore;
    } else pRes = NULL;
    NewPtr = NULL;
    while (StackPtr != This->TopOfBacktraceStack) {
        LastPtr = StackPtr->Up->ToDo;
        StackPtr = StackPtr->Up;
        if (NewPtr == NULL) {
            if (LastPtr->MinBt.NextInParallel != NULL) {
                _MINBACKTRACE *New2Ptr =
                    BestMinBTAhead(LastPtr->MinBt.NextInParallel);
                FASTALLO_TAKE(&pTos->FastAllocHeap, sizeof(_BACKTRACE) / 4, &NewPtr)
                NewPtr->MinBt.Previous = New2Ptr->Previous;
                NewPtr->MinBt.NextInParallel = New2Ptr->NextInParallel;
                NewPtr->MinBt.Score = New2Ptr->Score;
                NewPtr->State = LastPtr->State;
                NewPtr->ExitTime = LastPtr->ExitTime;
                NewPtr->Count = 0;
                {
                    struct _FASTALLO_FREE_NODE **pTemp =
                        &pTos->FastAllocHeap.pFreeNodes[sizeof(_MINBACKTRACE) / 4];
                    ((struct _FASTALLO_FREE_NODE *)New2Ptr)->pNext = *pTemp;
                    *pTemp = (struct _FASTALLO_FREE_NODE *)New2Ptr;
                }
                DiffScore = NewPtr->MinBt.Score - LastPtr->MinBt.Score;
                LastPtr->MinBt.NextInParallel = NULL;
            }
        } else {
            unsigned long u32OldScore;
            ThisPtr = CopyParBT(This, LastPtr);
            NewPtr->Count++;
            if (--ThisPtr->MinBt.Previous->Count == 0)
                CleanUpParBTProc(This, ThisPtr->MinBt.Previous);
            ThisPtr->MinBt.Previous = NewPtr;
            u32OldScore = ThisPtr->MinBt.Score;
            ThisPtr->MinBt.Score += DiffScore;
            NewPtr = BestBTAhead(ThisPtr);
            DiffScore = NewPtr->MinBt.Score - u32OldScore;
        }
    }
    if (NewPtr != NULL) {
        NewPtr->Count++;
        if (--pDPState->pBacktrace->Count == 0) {
            CleanUpParBTProc(This, pDPState->pBacktrace);
        }
        pDPState->pBacktrace = NewPtr;
        pDPState->Score += DiffScore;
    } else {
        if (pDPState->pBacktrace != NULL) {
            if (--pDPState->pBacktrace->Count == 0)
                CleanUpParBTProc(This, pDPState->pBacktrace);
            pDPState->pBacktrace = NULL;
        }
        pDPState->Score = 0x7FFF;
    }
    return pRes;
}

/* Orders active states by increasing score. */
static int ScoreCompare(const void *A, const void *B)
{
    s16 firstScore = (*(_ACTIVEDPSTATE *const *)A)->Score;
    s16 secondScore = (*(_ACTIVEDPSTATE *const *)B)->Score;

    if (firstScore > secondScore) {
        return 1;
    }
    if (firstScore < secondScore) {
        return -1;
    }
    return 0;
}

/* Removes invalid word paths, then sorts the retained backtraces by score. */
static unsigned long FindBestBacktraces(_NBESTDYNPROG *This, _ACTIVEDPSTATE ***ppActDps)
{
    _ACTIVEDPSTATE *pPrev = NULL;
    u32 cActiveWords = 0;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _ACTIVEDPSTATE *pToDelete;
    struct _FASTALLO_FREE_NODE **pTemp =
        &pTos->FastAllocHeap.pFreeNodes[sizeof(_ACTIVEDPSTATE) / 4];
    _ACTIVEDPSTATE *pDp1 = This->pDynProg;

    while (pDp1 != NULL) {
        pToDelete = NULL;

        if ((pDp1->pStateInDict[0].StateIndex & 1) != 0) {
            long Word = pDp1->pStateInDict[1].StateIndex +
                        ((u32)pDp1->pStateInDict[2].StateIndex << 16);

            if ((This->pWordProps[Word] & 2) != 0) {
                cActiveWords++;
            } else {
                pToDelete = pDp1;
            }
        } else {
            pToDelete = pDp1;
        }

        if (pToDelete != NULL) {
            if (--pToDelete->pBacktrace->Count == 0) {
                CleanUpParBTProc(This, pToDelete->pBacktrace);
            }
            pDp1 = pToDelete->pNext;
            if (pPrev != NULL) {
                pPrev->pNext = pDp1;
            } else {
                This->pDynProg = pDp1;
            }
            ((struct _FASTALLO_FREE_NODE *)pToDelete)->pNext = *pTemp;
            *pTemp = (struct _FASTALLO_FREE_NODE *)pToDelete;
        } else {
            pPrev = pDp1;
            pDp1 = pDp1->pNext;
        }
    }
    CollectDummyMemory(This);
    if (cActiveWords != 0) {
        _ACTIVEDPSTATE **Ptr;

        *ppActDps = heap_Alloc(pTos->hHeap, cActiveWords * sizeof(**ppActDps));
        pDp1 = This->pDynProg;
        Ptr = *ppActDps;
        while (pDp1 != NULL) {
            *Ptr++ = pDp1;
            pDp1 = pDp1->pNext;
        }
        qsort(*ppActDps, cActiveWords, sizeof(**ppActDps), ScoreCompare);
    } else {
        *ppActDps = NULL;
    }
    return cActiveWords;
}

/* Builds the best recognised backtrace list and normalizes its scores. */
static u8 FindBestRecognised(_NBESTDYNPROG *This)
{
    _ACTIVEDPSTATE **pActDps;
    unsigned long cActiveWords;
    unsigned char RetVal = 0;
    struct _DYNPROG_RESULT_NODE *pPrevRet;
    unsigned long St;
    struct _DYNPROG_RESULT_NODE *pRet;
    short NewScore;
    _ACTIVEDPSTATE *pRescoredDp;
    unsigned int i;
    short MinScore;
    struct _DYNPROG_RESULT_NODE *pToAdd;
    unsigned long j;
    cActiveWords = FindBestBacktraces(This, &pActDps);

    if (cActiveWords != 0) {
        MinScore = pActDps[0]->Score;
        St = 1;

        for (i = 0; cActiveWords > i; i++) {
            pActDps[i]->Score = pActDps[i]->Score - MinScore;
        }

        pRet = BacktraceWordsParBT(This, pActDps[0]);
        pPrevRet = pRet;
        pToAdd = pRet;
        while (pToAdd != NULL && St < This->MaxNBest) {
            j = 1;

            pRescoredDp = pActDps[0];
            NewScore = pRescoredDp->Score;
            while (j < cActiveWords && pActDps[j]->Score < NewScore) {
                pActDps[j - 1] = pActDps[j];
                j++;
            }
            pActDps[j - 1] = pRescoredDp;
            pToAdd = BacktraceWordsParBT(This, pActDps[0]);
            pPrevRet->pNext = pToAdd;
            pPrevRet = pToAdd;
            St++;
        }
        if (pPrevRet != NULL) {
            pPrevRet->pNext = NULL;
        }
        This->Result.pResult = pRet;
        heap_Free(This->BaseBlock.Object.pTos->hHeap, pActDps);
        if (pRet != NULL) {
            RetVal = 1;
        }
    } else {
        This->Result.pResult = NULL;
    }
    return RetVal;
}

/* Tests for terminal silence by tracking consecutive frames with the same terminal token. */
static u8 CheckTSCondition(_NBESTDYNPROG *This)
{
    _ACTIVEDPSTATE *pStMin = This->pStsMin;
    struct _DICTSTATE *pDict;
    long Word;

    if (pStMin == NULL) {
        return 0;
    }
    pDict = pStMin->pStateInDict;
    while ((pDict->StateIndex & 1) == 0) {
        pDict++;
    }
    Word = pDict[1].StateIndex + ((unsigned long)pDict[2].StateIndex << 16);
    if ((This->pWordProps[Word] & 2) != 0 && Word == (s32)This->OldWord) {
        This->uCountSilence++;
    } else {
        This->uCountSilence = 0;
    }
    This->OldWord = Word;
    if (This->uCountSilence > This->uCountSilenceThreshold) {
        return 1;
    }
    return 0;
}

/* Grows the arrays of running single-word and sentence garbage scores. */
static void ExpandGarbageArrays(_NBESTDYNPROG *This)
{
    unsigned long *pNewSingleWordGarbage =
        heap_Alloc(This->BaseBlock.Object.pTos->hHeap,
                   (This->Result.GarbageSize + 0x97) * sizeof(*pNewSingleWordGarbage));
    unsigned long *pNewSentenceGarbage;

    if (pNewSingleWordGarbage == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x71);
        return;
    }
    pNewSentenceGarbage =
        heap_Alloc(This->BaseBlock.Object.pTos->hHeap,
                   (This->Result.GarbageSize + 0x97) * sizeof(*pNewSentenceGarbage));
    if (pNewSentenceGarbage == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x71);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, pNewSingleWordGarbage);
        return;
    }
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

/* Ignores frame dispatch while recognition is idle. */
static void IdleProcess(void)
{
}

/* Processes one recognition input and updates the best-path score arrays. */
static void NBestDynProgProcess(struct _BLOCK *_This, void *pInputs)
{
    _NBESTDYNPROG *This = (_NBESTDYNPROG *)_This;
    /* Trace state for this processing call. */
    int dbg_bSubModActive;
    void *dbg_pInst;
    u8 *pLabels;
    u8 RetVal;
    s16 *pCachedDistances_DISTBLK = This->pCachedDistances_DISTBLK;
    unsigned int i;
    unsigned int NbrPhenoms;
    int sOldMin;
    u8 Speech;
    u8 *pSpeech;

    pLabels = ((void **)pInputs)[1];
    if (pLabels == NULL) {
        return;
    }

    NbrPhenoms = This->NbrPhenomes;
    sOldMin = This->sMin;
    Speech = 1;
    pSpeech = ((void **)pInputs)[0];
    if (pSpeech != NULL) {
        Speech = *pSpeech;
    }

    This->AccumulatedScore += sOldMin;
    for (i = 0; i < NbrPhenoms; i++) {
        pCachedDistances_DISTBLK[i] = 0x7FFF;
    }
    This->InitCalculateDistances(This->pDistCalc, pCachedDistances_DISTBLK, pLabels, Speech,
                                 sOldMin);
    ++This->u16FrameCounter;
    if (This->DynamicProgramming(This) != 0) {
        _tosErrorLog(&This->BaseBlock.Object, 0x77);
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))IdleProcess;
        RetVal = _tosControl(&This->BaseBlock.Object, 0xB, 0, 0);
        if (RetVal != 0) {
            return;
        }
    }
    if (This->u16FrameCounter >= This->Result.GarbageSize) {
        ExpandGarbageArrays(This);
    }
    This->Result.pSingleWordGarbage[This->u16FrameCounter + 1] =
        This->Result.pSingleWordGarbage[This->u16FrameCounter] +
        (pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom] == 0x7FFF
             ? (pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom] =
                    This->CalculateDistance(This->pDistCalc, This->SingleWordGarbagePhenom))
             : pCachedDistances_DISTBLK[This->SingleWordGarbagePhenom]) +
        sOldMin;
    This->Result.pSentenceGarbage[This->u16FrameCounter + 1] =
        This->Result.pSentenceGarbage[This->u16FrameCounter] +
        (pCachedDistances_DISTBLK[This->SentenceGarbagePhenom] == 0x7FFF
             ? (pCachedDistances_DISTBLK[This->SentenceGarbagePhenom] =
                    This->CalculateDistance(This->pDistCalc, This->SentenceGarbagePhenom))
             : pCachedDistances_DISTBLK[This->SentenceGarbagePhenom]) +
        sOldMin;
    if (This->EndOfSpeech != ENDOFSPEECH_None && CheckTSCondition(This)) {
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))IdleProcess;
        _tosControl(&This->BaseBlock.Object, 8, This->u16FrameCounter, 0);
    }
}

/* Returns the backtrace stack nodes to the block allocator. */
static void FreeBacktraceStack(_NBESTDYNPROG *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    _BACKTRACESTACK *pStack = This->TopOfBacktraceStack;
    _BACKTRACESTACK *pToFree;
    while (pStack != NULL) {
        pToFree = pStack;
        pStack = pStack->Down;
        {
            struct _FASTALLO_FREE_NODE **pTemp =
                &pTos->FastAllocHeap.pFreeNodes[sizeof(_BACKTRACESTACK) / 4];
            ((struct _FASTALLO_FREE_NODE *)pToFree)->pNext = *pTemp;
            *pTemp = (struct _FASTALLO_FREE_NODE *)pToFree;
        }
    }
    This->TopOfBacktraceStack = NULL;
}

/* Loads the language's phoneme count and garbage phonemes for N-best decoding. */
static inline u8 InitDynprogLanguage(_NBESTDYNPROG *This, struct _LANGUAGE *pLang)
{
    unsigned int NbrPhenomes = pLang->langGetNbrPhenome(pLang);

    This->NbrPhenomes = NbrPhenomes;
    This->SingleWordGarbagePhenom = (u16)pLang->langGetSingleWordGarbagePhenome(pLang);
    This->SentenceGarbagePhenom = (u16)pLang->langGetSentenceGarbagePhenome(pLang);
    return 0;
}

/* Applies control messages that configure or reset the N-best search. */
static unsigned char ControlNBestDynProgProcess(
    struct _BLOCK *_This, unsigned char ControlId, unsigned long CParam1, unsigned long CParam2)
{
    _NBESTDYNPROG *This = (_NBESTDYNPROG *)_This;
    struct _DISTCALCFUNCS *pFuncs;

    switch (ControlId) {
    case 0x25:
        return qQueueControl(This->BaseBlock.InputQueues->pQ, 7,
                             This->BaseBlock.InputQueues->ReadPos, CParam1);
    case 0x26:
        return qQueueControl(This->BaseBlock.InputQueues->pQ, 6,
                             This->BaseBlock.InputQueues->ReadPos, 0);
    case 4:
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))IdleProcess;
        if (FindBestRecognised(This) != 0) {
            return !_tosControl(&This->BaseBlock.Object, 9, (u32)&This->Result.pResult, 0);
        }
        return !_tosControl(&This->BaseBlock.Object, 0xA, 0, 0);
    case 3:
        InitViterbi(This);
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))NBestDynProgProcess;
        break;
    case 5:
        This->BaseBlock.Process = (void (*)(struct _BLOCK *, void *, int))IdleProcess;
        break;
    case 0x27:
        break;
    case 0x22:
        InitDynprogLanguage(This, (struct _LANGUAGE *)CParam1);
        break;
    case 0x23:
        FreeDynprog(This);
        if (InitDynprogContext(This, (void *)CParam1) != 0) {
            return 0;
        }
        break;
    case 0x24: {
        pFuncs = (struct _DISTCALCFUNCS *)CParam1;
        This->InitCalculateDistances = pFuncs->InitCalculateDistances;
        This->CalculateDistance = pFuncs->CalculateDistance;
        This->GetpCachedDistances_DISTBLK = pFuncs->GetpCachedDistances;
        This->pDistCalc = pFuncs->pThis;
        break;
    }
    case 0x1C: {
        unsigned long *pWordID = (unsigned long *)CParam1;
        long NbrWords = CParam2;
        long i;
        const unsigned short *pWordProps = This->pWordProps;
        unsigned char *pucMenuMasks = This->pucMenuMasks;
        if (pWordID == NULL) {
            if (NbrWords != 0) {
                memset(pucMenuMasks, 0xFF, This->uNbrOfWordsBit - 1);
                pucMenuMasks[This->uNbrOfWordsBit - 1] =
                    0xFF << ((8 - (This->uNbrOfWords & 7)) & 7);
            } else {
                memset(pucMenuMasks, 0, This->uNbrOfWordsBit);
            }
        } else {
            if (NbrWords > 0) {
                memset(pucMenuMasks, 0, This->uNbrOfWordsBit);
            } else {
                NbrWords = -NbrWords;
            }
            for (i = 0; i < NbrWords; i++) {
                pucMenuMasks[pWordID[i] >> 3] |= 0x80 >> (pWordID[i] & 7);
            }
        }
        NbrWords = This->uNbrOfWords;
        for (i = 0; i < NbrWords; i++) {
            if ((pWordProps[i] & 0x80) == 0) {
                pucMenuMasks[i >> 3] |= 0x80 >> (i & 7);
            }
        }
        ChangeMenuIndexedSyntax(This);
        InitViterbi(This);
        break;
    }
    case 0x1E: {
        unsigned long *pWordID = (unsigned long *)CParam1;
        unsigned long NbrWords = -CParam2;
        unsigned long i;
        const unsigned short *pWordProps = This->pWordProps;
        unsigned char *pucMenuMasks = This->pucMenuMasks;
        if (pWordID == NULL) {
            memset(This->pucMenuMasks, 0, This->uNbrOfWordsBit);
        } else {
            for (i = 0; i < NbrWords; i++) {
                unsigned char Mask = ~(0x80 >> (pWordID[i] & 7));
                This->pucMenuMasks[pWordID[i] >> 3] &= Mask;
            }
        }
        NbrWords = This->uNbrOfWords;
        for (i = 0; i < NbrWords; i++) {
            if ((pWordProps[i] & 0x80) == 0) {
                pucMenuMasks[i >> 3] |= 0x80 >> (i & 7);
            }
        }
        ChangeMenuIndexedSyntax(This);
        InitViterbi(This);
        break;
    }
    case 0x1D:
        *(void **)CParam1 = This->pucMenuMasks;
        *(u32 *)CParam2 = This->uNbrOfWordsBit;
        break;
    case 0x1A: {
        struct _DYNPROG_PARAM RecParam;
        RecParam = *(struct _DYNPROG_PARAM *)CParam1;
        if (This->Nset != RecParam.Accuracy) {
            This->Nset = RecParam.Accuracy;
        }
        This->uOriCountSilenceThreshold = RecParam.TSCount;
        This->uCountSilenceThreshold = This->uOriCountSilenceThreshold / 10;
        This->MaxNBest = RecParam.MaxNBest;
        if (RecParam.TSEnable) {
            This->EndOfSpeech = ENDOFSPEECH_StopCriterium;
        } else {
            This->EndOfSpeech = ENDOFSPEECH_None;
        }
        break;
    }
    case 0x1B: {
        struct _DYNPROG_PARAM *pRecParam = (struct _DYNPROG_PARAM *)CParam1;
        pRecParam->Accuracy = This->Nset;
        pRecParam->TSCount = This->uOriCountSilenceThreshold;
        pRecParam->MaxNBest = This->MaxNBest;
        pRecParam->TSEnable = This->EndOfSpeech == ENDOFSPEECH_None ? 0 : 1;
        break;
    }
    case 0xFF:
        CollectMemory(This);
        FreeDynprog(This);
        FreeBacktraceStack(This);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Constructs the N-best recognition block and installs its callbacks. */
struct _BLOCK *ConstructNBestDynProg(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(
        pTos, BlockID, 2, 0,
        (void (*)(struct _BLOCK *, void *, int))NBestDynProgProcess,
        InitNBestDynProgProcess,
        ControlNBestDynProgProcess,
        sizeof(_NBESTDYNPROG));
}
